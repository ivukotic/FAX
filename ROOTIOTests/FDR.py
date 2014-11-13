#!/usr/bin/env python
import subprocess, threading, os, sys, cx_Oracle

class Command(object):
    def __init__(self, cmd):
        self.cmd = cmd
        self.process = None
    
    def run(self, timeout, foreground=False):
        def target():
            print 'command started: ', self.cmd
            self.process = subprocess.Popen(self.cmd, shell=True)
            if (foreground): self.process.communicate()
            # print 'command finished'
        
        thread = threading.Thread(target=target)
        thread.start()
        
        thread.join(timeout)
        if thread.is_alive():
            print 'Terminating process'
            self.process.terminate()
            thread.join()
        return self.process.returncode


line='ATLAS_HCLOUDTEST/HCLOUDTEST_2014@(DESCRIPTION=(ADDRESS=(PROTOCOL=TCP)(HOST=intr1-v.cern.ch)(PORT=10121))(ADDRESS=(PROTOCOL=TCP)(HOST=intr2-v.cern.ch)(PORT=10121))(LOAD_BALANCE=yes)(CONNECT_DATA=(SERVER=DEDICATED)(SERVICE_NAME=intr.cern.ch)(FAILOVER_MODE=(TYPE=SELECT)(METHOD=BASIC)(RETRIES=200)(DELAY=15))))'

SITE='d'
PANDAID = 1  

if os.environ.has_key('PANDA_SITE_NAME'):
    SITE=os.environ['PANDA_SITE_NAME']

if os.environ.has_key('PandaID'):
    PANDAID = int(os.environ['PandaID'])

print 'SITE: ',SITE, "\tPANDAID: ",PANDAID



try:
    connection = cx_Oracle.Connection(line)
    cursor = cx_Oracle.Cursor(connection)
    print 'connection established.'
    cursor.execute("select testid, name, testtype, server, address, files, timeout from FDR_TESTS,FDR_SERVERS where status='Active' and client='"+SITE+"'  and jobs>started and server=site")
    res = cursor.fetchall()
    cursor.close()
    if len(res)==0: 
        print 'nothing to do. Exiting.'        
        sys.exit(0)
    r=res[0]
    testid=r[0]
    print 'testID:   ',testid
    name=r[1]
    print 'name:     ',name
    testtype=r[2]
    print 'testtype: ',testtype 
    server=r[3]
    print 'server:   ',server
    address=r[4]
    print 'address:  ',address
    nfiles=r[5]
    print 'files:    ',nfiles
    timeout=r[6]
    print 'timeout:  ',timeout

# update column 'started'
    cursor = cx_Oracle.Cursor(connection)
    print 'updateing column started.'
    cursor.execute("update FDR_TESTS set started=started+1 where testid="+str(testid))
    cursor.close()

# load all the files
    filenames={}
    cursor = cx_Oracle.Cursor(connection)
    print 'loading files...'
    cursor.execute("select filename,dataset,tree, bytes from ( select filename, dataset, tree, bytes from FDR_FILES,FDR_DS where  dataset=dsname and active=1 order by dbms_random.value )  where rownum <= "+str(nfiles))
    res = cursor.fetchall()
    cursor.close()
    connection.commit()
    path="root://"+address
    for r in res:
        fn=r[0].replace('SITENAME',server)
        ds=r[1].replace('SITENAME',server)
        filenames[path+ds+fn]=[r[2],r[3]]

# create scripts to execute
    if testtype=='FAXcopy':
         f=open('toExecute.sh', 'w')
         for fn in filenames.keys():
             f.write('`which time` -f "COPYTIME=%e" --append -o logfile xrdcp -f -np '+fn+""" /dev/null 2>&1 \n""")
             f.write(' echo BYTES=' +str(filenames[fn][1])+' >> logfile \n')
         f.close()
         os.chmod('toExecute.sh', 0755);
    
     # execute them      
         comm=Command('source toExecute.sh')
         comm.run(timeout,True);

     # parsing log file
         f=open('logfile','r')
         result=f.readlines()
         f.close()
         print result
         sumtime=0
         sumbytes=0
         succ=0
         problems=0
         skip=False
         for r in result:
             if r.count('Command exited with non-zero status')!=0:
                 problems=problems+1 
                 skip=True
                 continue
             w=r.split('=')
             if len(w)!=2: continue
             if w[0]=='COPYTIME' and skip==False:
                 sumtime=sumtime+float(w[1])
                 succ=succ+1
             if w[0]=='BYTES' and skip==False:
                 sumbytes+=float(w[1]) 
             if w[0]=='BYTES' and skip==True:
                 skip=False
         report=''
         mbps=0
         evps=0
         if succ!=nfiles:
             report=str(problems)+' copies failed. '+str(succ)+' were OK. '
         else:
             report='OK. '
         if sumtime>0:
             mbps=sumbytes/1024/1024/sumtime     
         print testid, sumtime,'sec\t', PANDAID, report, mbps,'MB/s'

     # uploading result
         cursor = cx_Oracle.Cursor(connection)
         cursor.callproc('FDR_INSERT_FULL_RESULT',(testid,sumtime,mbps,evps,PANDAID,report))
         cursor.close()
         print  'result uploaded.'
         connection.commit()

    if testtype=='read10pc':
         f=open('toExecute.sh', 'w')
         for fn in filenames:
             f.write('./readDirect '+fn+' '+filenames[fn][0]+' 10 30 >> logfile  2>&1 \n')
         f.close()
         os.chmod('toExecute.sh', 0755);

     # execute them      
         comm=Command('source toExecute.sh')
         comm.run(timeout,True);

      # parsing log file
         f=open('logfile','r')
         result=f.readlines()
         f.close()
         sum=0
         succ=0
         rootbytesread=0
         eventsread=0  
         for r in result:
             print r
             w=r.split('=')
             if len(w)!=2: continue
             if w[0]=='WALLTIME': 
                 sum=sum+float(w[1])
                 succ=succ+1
             if w[0]=='ROOTBYTESREAD':
                 rootbytesread+=float(w[1])
             if w[0]=='EVENTS':
                 eventsread+=float(w[1])
         report=''
         mbps=0
         evps=0
         if succ!=nfiles:
             report='Some reads failed. '+str(succ)+' were OK. '
         else:
             report='OK '
         if sum!=0:
             mbps=rootbytesread/1024/1024/sum
             evps=eventsread/sum
         print report, mbps,'MB/s\t', evps,'ev/s'

     # uploading result
         cursor = cx_Oracle.Cursor(connection)
         cursor.callproc('FDR_INSERT_FULL_RESULT',(testid,sum,mbps,evps,PANDAID,report))
         cursor.close()
         print  'result uploaded.'
         connection.commit()
    

    print 'FDR => DONE.'
except cx_Oracle.DatabaseError, exc:
    print exc
    error, = exc.args
    print "FDR.py - problem in establishing connection to db"
    print "FDR.py Oracle-Error-Code:", error.code
    print "FDR.py Oracle-Error-Message:", error.message
                               
