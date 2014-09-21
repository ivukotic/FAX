#!/usr/bin/env python

# this code is responsible for filling up of SSB costMatrix
# it takes list of active xrootd doors from AGIS, copy a file from each of them
# uploads MB/s results using ActiveMQ to a machine in CERN
# it also uploads into GAE datastore
# it runs trace and uploads results to a MongoDB

import subprocess, threading, os, sys, random, math
import stomp, logging, datetime, time, ConfigParser

import urllib,urllib2
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
    def __init__(self, cmd,sn):
        self.cmd = cmd
        self.cn=sn
        tv=random.randint(0,15*60)
        print sn,"will wait", tv,"seconds to start"
        self.next=time.time()+tv
        self.process = None
        self.thread = None
        self.counter=0
        
    def run(self, timeout, foreground=False):
        def target():
            print 'command started', self.cn, self.counter
            self.next=time.time()+24*3600 # this prevents it from restarting in case it's still running
            self.process = subprocess.Popen(self.cmd, shell=True)
            if (foreground): self.process.communicate()
        
        self.thread = threading.Thread(target=target)
        self.thread.start()
        
        self.thread.join(timeout)
        if self.thread.is_alive():
            print 'Terminating process'
            try:
                self.process.terminate()
                print 'command killed', self.cn
            except OSError as e:
                print >>sys.stderr, "Forced termination failed:", e
            self.thread.join()
        return #self.process.returncode
        
    
def upload(SITE_FROMLOG, SITE_TO):
    SITE_FROM = SITE_FROMLOG.replace('.log','')
    
    print 'Upload:',SITE_FROM,' -> ',SITE_TO     
    if not os.path.isfile(SITE_FROMLOG):
        print "log file for site",site,"missing"
        return 
        
    res=rate=0
    traceIsOn=False
    hcount=0            
    hops=[]
    with open(SITE_FROMLOG, 'r') as f:
        lines=f.readlines()
        
        for l in lines:
            l=l.strip()
            
            if l.count('COPYTIME=')>0:
                res=l.replace('COPYTIME=','')
                if res!='' and res>0:
                    rate=100/float(res)
                else:
                    rate=0
                    
            if l.count('EXITSTATUS')>0:
                res=l.replace('EXITSTATUS=','')
                
            if l.startswith('traceroute'): 
                traceIsOn=True
                continue
            if traceIsOn:
                hcount+=1
                w=l.split()
                if len(w)==5 or len(w)==4 or len(w)==2:
                    hn=0
                    try:
                        hn=int(w[0])
                    except:
                        print "Unexpected error in parsing hop number:", sys.exc_info()[0]
                    if hn!=hcount:
                        print 'missing hop in a traceroute'
                        continue
                    sip=w[1].split('.')
                    ip=delay=0
                    if w[1]=="*":
                        pass
                    elif len(sip)!=4:
                        print 'could not parse IP address: ', w[1]
                        continue
                    else:
                        try:
                            ip=int(sip[0]) * 16777216 + int(sip[1]) * 65536 + int(sip[2]) * 256 + int(sip[3])
                        except ValueError:
                            print "Unexpected error in parsing ip: ", w[1], sys.exc_info()[0]
                            continue
                    if len(w)>2:
                        try:
                            delay=float(w[2])
                        except ValueError:
                            print "Unexpected error in parsing delay:",w[2], sys.exc_info()[0]
                            continue
                    hops.append([ip,delay])
                else:
                    print 'unexpected line in the traceroute log.', l
                    continue
                    
            
        if res=='0':
            #print '--------------------------------- Uploading result ---------------------------------'
            #print rate    
            ts=datetime.datetime.utcnow()
            ts=ts.replace(microsecond=0)
            toSend='site_from: '+ SITE_FROM + '\nsite_to: '+SITE_TO+'\nmetricName: FAXprobe4\nrate: '+str(rate)+'\ntimestamp: '+ts.isoformat(' ')+'\n'
            #print toSend
            send (toSend)

            print '-------------------------------- Writing to GAE -------------------------------------------'
            data = dict(source=SITE_FROM, destination=SITE_TO, rate=rate)
            u = urllib2.urlopen('http://waniotest.appspot.com/wancost', urllib.urlencode(data))
            #print u.read()
        else:
            print 'non 0 exit code. will not upload result. ' 
            if SITE_TO=='CERN-PROD':
                print "full log:", lines
            
        if len(hops)>1:
            print '-------------------------------- Writing to MongoDB -------------------------------------------'
    
            data = simplejson.dumps({"hops":hops,"rate":rate,"from":SITE_FROM,"to":SITE_TO})
            print data
            try:
                r = urllib2.Request('http://db.mwt2.org:8080/trace', data, {'Content-Type': 'application/json'})
                f = urllib2.urlopen(r)
                response = f.read()
                f.close()
                print response
            except:
                print "Unexpected error:", sys.exc_info()[0]
        
def main():

    maxParallel=6
    currParallel=0
            
    if len(sys.argv)==3:
        # print 'uploading results from file ', sys.argv[1]
        # print 'I was told the site name is ', sys.argv[2]
        upload(sys.argv[1],sys.argv[2])
        return
    if len(sys.argv)==2:
        #print 'uploading traceroute from file ', sys.argv[1]
        uploadTrace(sys.argv[1])
        return

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
        if SITE=='GRIF': SITE=res[0]["atlas_site"]
        print "at SITE:", SITE    
    except:
        print "Unexpected error:", sys.exc_info()[0]
        



    sites=[]; # each site contains [name, host, redirector]
    print 'Geting site list from AGIS...' 

    try:
        req = urllib2.Request("http://atlas-agis-api.cern.ch/request/service/query/get_se_services/?json&state=ACTIVE&flavour=XROOTD", None)
        opener = urllib2.build_opener()
        f = opener.open(req)
        res=simplejson.load(f)
        for s in res:
            # print  s["rc_site"], s["endpoint"], s["redirector"]["endpoint"]
            sname=s["rc_site"]
            if sname=='GRIF': sname=s["name"]
            si=site(sname, s["endpoint"], s["redirector"]["endpoint"])
            si.prnt()
            sites.append(si)
    except:
        print "Unexpected error:", sys.exc_info()[0] 
    



    for s in sites:

        with open('exec'+s.name+'.sh', 'w') as f:
        
            fn=s.host+"//atlas/rucio/user/ivukotic:user.ivukotic.xrootd."+s.name.lower()+"-100M"    
            logfile = s.name + '.log'
        
            f.write( """#!/bin/bash\n""")
            f.write('echo "--------------------------------------"\n ')
            if SITE.count("CERN")>0:
                f.write('xrdcp -np -d 2 -f ' + fn + ' tmp.root  &>'+logfile+' \n')
            else:
                f.write('`which time`  -f "COPYTIME=%e\\nEXITSTATUS=%x" -o '+ logfile +' xrdcp -np ' + fn + """ - > /dev/null  2>&1 \n""")
            servname=s.host.replace('root://','')
            servname=servname.split(':')[0]
            f.write('traceroute -w 3 -q 1 -n '+servname+' >> '+logfile+" \n")
            f.write('python costMatrix.py '+logfile+" "+SITE+"\n")
            f.write('rm '+logfile+"\n")

        f.close()
        os.chmod('exec'+s.name+'.sh', 0755);
        
    comms=[]
    for s in sites:
        comm=Command("source ./exec"+s.name+".sh",s.name)
        comms.append(comm)
    
    for w in range(86100):
        time.sleep(1)
        ct=time.time()
        for c in comms:
            if c.process is not None:
                if c.process.poll() is not None: 
                    c.process.wait()
                    print 'command finished', c.cn, c.counter
                    currParallel-=1
                    c.counter+=1
                    c.process=None
                    c.next=time.time()+15*60
            if ct>c.next:
                if currParallel>maxParallel:
                    print 'Already having',currParallel, "streams. Delaying start of",c.cn
                    continue
                c.run(3600)
                currParallel+=1
        
    print 'stopping.'
    
    
if __name__ == "__main__":
    main()
