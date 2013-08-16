#!/usr/bin/env python

# this code is responsible for filling up of SSB costMatrix
# it takes list of active xrootd doors from AGIS, copy a file from each of them
# uploads MB/s results using ActiveMQ to a machine in CERN

import subprocess, threading, os, sys, random
import stomp, logging, datetime, ConfigParser

import urllib2
import json as simplejson

        


def send (message):
    # all of the ActiveMQ configuration 
    logging.basicConfig()

    config = ConfigParser.ConfigParser()
    config.read("neet.cfg")
    HOST = config.get("Connection", "HOST")
    PORT = int(config.get("Connection", "PORT"))
    USER = config.get("Connection", "USER")
    PASS = config.get("Connection", "PASS")
    QUEUE = config.get("Connection", "QUEUE")
    
    """ Send message by stomp protocols.  @param message: the message being sent"""  
    conn = stomp.Connection([(HOST,PORT)],USER,PASS)
    conn.start()
    conn.connect()
    
    conn.send(message,destination=QUEUE, ack='auto')
    
    try:
       conn.disconnect()
    
    except Exception:
        'Exception on disconnect'
    



# getting all sites details from AGIS


class site:    
    name=''
    host=''
    redirector=''
     
    def __init__(self, na, ho, re):
        self.name=na
        self.host=ho
        self.redirector=re
    
    def prnt(self):
        print 'name:', self.name,'\tredirector:', self.redirector,  '\thost:', self.host
    




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
    

    
def upload(SITE_FROMLOG, SITE_TO):
    SITE_FROM = SITE_FROMLOG.replace('.log','')
    
    print SITE_FROM,' -> ',SITE_TO     
    if not os.path.isfile(SITE_FROMLOG):
        print "log file for site",site,"missing"
        return 
    with open(SITE_FROMLOG, 'r') as f:
        lines=f.readlines()
        if len(lines)>0:
            res=lines[0]
            if res.count('COPYTIME=')>0:
                res=res.replace('COPYTIME=','')
                print '--------------------------------- Uploading result ---------------------------------'
                rate=100/float(res)
                print rate    
                ts=datetime.datetime.utcnow()
                ts=ts.replace(microsecond=0)
                toSend='site_from: '+ SITE_FROM + '\nsite_to: '+SITE_TO+'\nmetricName: FAXprobe4\nrate: '+str(rate)+'\ntimestamp: '+ts.isoformat(' ')+'\n'
                print toSend
                send (toSend) 

def main():
        
    QUEUE = ''    
    SITE = ''
    if os.environ.has_key('PANDA_SITE_NAME'):
        QUEUE=os.environ['PANDA_SITE_NAME']
    else:
        print 'ERROR. PANDA_SITE_NAME is not defined. Exiting.'
        sys.exit(0)
    print 'RUNNING in QUEUE: ',QUEUE
    
    #finding which site is this
    try:
        req = urllib2.Request("http://atlas-agis-api.cern.ch/request/pandaqueue/query/list/?json&panda_resource="+QUEUE, None)
        opener = urllib2.build_opener()
        f = opener.open(req)
        res = simplejson.load(f)
        if len(res)==0:
            print 'could not get site name from queue name. exiting.'
            sys.exit(0)
        SITE=res[0]["rc_site"]
        print "at SITE:", SITE    
    except:
        print "Unexpected error:", sys.exc_info()[0]
        
        

    if len(sys.argv)>1:
        print 'uploading results from file ', sys.argv[1]
        upload(sys.argv[1],SITE)
        return




    sites=[]; # each site contains [name, host, redirector]
    print 'Geting site list from AGIS...' 


    try:
        req = urllib2.Request("http://atlas-agis-api.cern.ch/request/service/query/get_se_services/?json&state=ACTIVE&flavour=XROOTD", None)
        opener = urllib2.build_opener()
        f = opener.open(req)
        res=simplejson.load(f)
        for s in res:
            # print  s["rc_site"], s["endpoint"], s["redirector"]["endpoint"]
            si=site(s["rc_site"], s["endpoint"], s["redirector"]["endpoint"])
            si.prnt()
            sites.append(si)
    except:
        print "Unexpected error:", sys.exc_info()[0] 
    



    for s in sites:

        with open('exec'+s.name+'.sh', 'w') as f:
        
            fn=s.host+"//atlas/dq2/user/"
            fn+="HironoriIto/user.HironoriIto.xrootd."+s.name.lower()+"/user.HironoriIto.xrootd."+s.name.lower()+"-100M"
            #ilijav/HCtest/user.ilijav.HCtest.1/group.test.hc.NTUP_SMWZ.root"
            logfile = s.name + '.log'
        
        
            f.write( """#!/bin/bash\n""")
            #staggering start
            f.write("sleep "+str(random.randint(0,300))+"\n");
            f.write("""for (( ; ; ))\n""")
            f.write("""do\n""")
            f.write(""" echo "--------------------------------------"\n """)
            f.write('   `which time`  -f "COPYTIME=%e" -o '+ logfile +' xrdcp -np -f ' + fn + """ /dev/null  2>&1 \n""")
            f.write('   python costMatrix.py '+logfile+"\n")
            f.write("   sleep 600\n");
            f.write("""done\n""")

        f.close()
        os.chmod('exec'+s.name+'.sh', 0755);

    comms=[]
    for s in sites:
        comm=Command("source exec"+s.name+".sh")
        comms.append(comm)
    
    for c in comms:	
        c.run(47*1800)
    print 'jobs started'
    
    comm4 = Command("sleep 84500") 
    print 'now wating'
    comm4.run(84510,True)
    print 'stopping.'
    
    
if __name__ == "__main__":
    main()
