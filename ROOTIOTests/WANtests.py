import subprocess, threading, os, sys, cx_Oracle, random

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
    

    
def upload(site):
    PING      = 0
    SITE1     = site
    PANDAID   = 1  
    FILENAME  = 'd'
    COPYTIME  = 0
    WALLTIME  = 0
    CPUTIME  = 0
    ZIPSIZE   = 0
    ROOTREADS = 0
    VERSION   = 'd'
    
    JOBTYPE   = '10% default cache'
    PARAM1    = '10'
    PARAM2    = '30'
    PARAM3    = '0'    
    
    #print  JOBTYPE, PARAM1, PARAM2, PARAM3
    
    if os.environ.has_key('PandaID'): PANDAID = int(os.environ['PandaID'])
    
    if os.environ.has_key('MACHTYPE'): VERSION = os.environ['MACHTYPE']
    else: 
        if os.environ.has_key('HOSTTYPE'): VERSION = os.environ['HOSTTYPE']
    
    print SITE1,' -> ',SITE     
    if not os.path.isfile(site+'.log'):
        print "log file for site",site,"missing"
        return 
    with open(site+'.log', 'r') as f:
        lines=f.readlines()
        skipNext=False
        for l in lines:
            if l.startswith('Command exited with non-zero status'):
                skipNext=True
            if '=' not in l: continue 
            if skipNext==True:
                skipNext=False
                print 'skipping line:', l,' due to non-zero exit status.'
                continue
            # print 'executing: ', l.strip()
            try:
                exec(l)
            except:
                pass
    
    print FILENAME, COPYTIME, WALLTIME, CPUTIME, ZIPSIZE, ROOTREADS
    
    try:
        connection = cx_Oracle.Connection(line)
        cursor = cx_Oracle.Cursor(connection)
        print 'Sending...'
        all=[JOBTYPE, PARAM1, PARAM2, PARAM3, SITE, SITE1, PANDAID, FILENAME, PING, COPYTIME, WALLTIME, CPUTIME, ZIPSIZE, ROOTREADS]
        print all
        cursor.callproc('ATLAS_WANHCTEST.INSERTWANRESULT', all)
        connection.commit()
        print '---------------------------------------------------------------'
        print
    except cx_Oracle.DatabaseError, exc:
        error, = exc.args
        print "WANtests.py Oracle-Error-Code:", error.code
        print "WANtests.py Oracle-Error-Message:", error.message    

# getting oracle conn string
line='ATLAS_WANHCTEST/WANhctest_3@(DESCRIPTION=(ADDRESS=(PROTOCOL=TCP)(HOST=intr1-v.cern.ch)(PORT=10121))(ADDRESS=(PROTOCOL=TCP)(HOST=intr2-v.cern.ch)(PORT=10121))(LOAD_BALANCE=yes)(CONNECT_DATA=(SERVER=DEDICATED)(SERVICE_NAME=intr.cern.ch)(FAILOVER_MODE=(TYPE=SELECT)(METHOD=BASIC)(RETRIES=200)(DELAY=15))))'

sites=[];

SITE      = 'd'
if os.environ.has_key('PANDA_SITE_NAME'):
    SITE=os.environ['PANDA_SITE_NAME']

siteenv=''
print 'RUNNING AT: ',SITE
print 'Geting site list ...'

try:
    connection = cx_Oracle.Connection(line)
    cursor = cx_Oracle.Cursor(connection)
    print 'Connection established.'

    cursor.execute("SELECT siteid, name, server, url, filepath, protocol, debug, ping, accessprobability FROM ATLAS_WANHCTEST.site WHERE role='both' or role='server'")
    res = cursor.fetchall()
    for r in res:
        sites.append([r[0],r[1],r[2],r[3],r[4],r[5],r[6],r[7],r[8]])
        print r[1],r[2],r[3],r[4],r[5],r[6],r[7],r[8]
    print 'got', len(sites), 'sites.'
    print

    cursor.execute("SELECT setenv FROM ATLAS_WANHCTEST.site WHERE name='"+SITE+"'")
    res = cursor.fetchall()
    cursor.close()
    for r in res:
        siteenv=r[0]
    print 'got environment modification.'
    
except cx_Oracle.DatabaseError, exc:
    error, = exc.args
    print "WANtests.py - problem in establishing connection to db"
    print "WANtests.py Oracle-Error-Code:", error.code
    print "WANtests.py Oracle-Error-Message:", error.message

print 'creating scripts to execute'

for s in sites:

    with open('exec'+s[1]+'.sh', 'w') as f:

        f.write( siteenv + """\n\n""")

        fn=s[5]+s[2]+s[3]+s[4]
        logfile = s[1] + '.log'
        
        #staggering start
        f.write("sleep "+str(random.randint(0,300))+"\n");
        #  ping
        noPortSitename=(s[2].split(':'))[0] 
        if (s[7]==1 and SITE!='ANALY_HU_ATLAS_Tier2'):
            f.write("""ping -c 5 -n -q -W 10 """+ noPortSitename +"""  | awk \'{if ( index($1,"PING")>0 ) r1=$2;  if ( index($1,"rtt")>0 ) {split($4,a,"/"); print "PING=", a[2], "#" , $NF } }\' >> """+ logfile +""" 2>&1 \n""");
        else:
            f.write(""" echo \'PING= 0.0 # ms\' >> """+ logfile +""" 2>&1 \n""");
        # turning debug on/off
        f.write(s[6] + """\n""")
       
        # COPY
        if (s[5]=='root://'):
            f.write('`which time`  -f "COPYTIME=%e" --append -o '+ logfile +' xrdcp -np -f ' + fn + """ /dev/null  2>&1 \n""")
        if (s[5]=='dcap://'):
            f.write('`which time`  -f "COPYTIME=%e" --append -o '+ logfile +' dccp ' + fn + """ /dev/null  2>&1 \n""")
        
        # remote read
        #f.write('root -l -q -b "read.C++(\\"' + fn + '\\",\\"physics\\", 100, 30 )" >> '+ logfile +' 2>&1 \n')
        f.write('./readDirect '+ fn + ' physics 10 30 >> '+ logfile +' 2>&1 \n')

        fd=f.fileno();
    f.close()

    os.chmod('exec'+s[1]+'.sh', 0755);


comms=[]
for s in sites:
    if (random.random()>s[8]): continue
    comm=Command("source exec"+s[1]+".sh")
    comms.append(comm)


for w in range(46):
    print '-----------\nstep: ', w
    
    for c in comms:
        c.run(1790)
        
    comm4 = Command("sleep 1800")    
    comm4.run(1802, True)    
    
    for s in sites:
        # results upload
        upload(s[1])
        mvcomm='mv '+ s[1]+'.log '+s[1]+'_'+str(w)+'.log'
        mvc=Command(mvcomm)
        mvc.run(10,True)
    
    print 'done.'
