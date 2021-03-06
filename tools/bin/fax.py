import logging
import urllib2

from threading import Thread
import Queue

try: import simplejson as json
except ImportError: import json

import rucio
import rucio.client
import rucio.common.config as conf

rrc=rucio.client.replicaclient.ReplicaClient()
rq=Queue.Queue()

class endpoint:
    name=''
    host=''
    port=1094
    def __init__(self, na, ho, dd):
        self.name=na
        ho=ho.replace("root://","")
        self.host=ho.split(":")[0]
        if ho.count(":"):
            self.port=ho.split(":")[1]
        self.ddms=dd
    def prnt(self):
        logging.debug('name: %s \thost: %s:%s' % (self.name, self.host, self.port ))
        for i in range(len(self.ddms)):
            logging.debug(self.ddms[i])

class faxfile:
    def __init__(self, sc, na, si, ad):
        self.scope=sc
        self.name=na
        self.size=si
        self.adler32=ad
        self.attempts=0
        self.reps=[]   # this contains ddm endpoints
        self.endpoints=[] # what is this
        self.PNFS=[] # this contains full pnfs
        self.expectedRates=[]
        
    def findReplicas(self):
        reps=[]
        try:
            reps=rrc.list_replicas([{'scope': self.scope, 'name': self.name}], schemes=['root'])
        except:
            logging.error("Could not get replicas from rucio.")
            
        for r in reps:
            for key, value in r['rses'].iteritems():
                if len(value)==0:
                    logging.warning("Site %s has no fax endpoint!" % key);
                    continue 
                if len(value)>1:
                    logging.warning("Site %s has multiple copies of the same file!" % key);
                self.reps.append(key)
                self.endpoints.append('')
                self.PNFS.append(value[0])
                self.expectedRates.append(0)
                
    def getPNFS(self, retry):
        #first creates a dictionary of all the valid PNFSes and rates.
        allValid={}
        for i in range(len(self.reps)):
            if self.expectedRates[i]>0:
                allValid[self.expectedRates[i]]=self.PNFS[i]
        # sort rates desc
        so=sorted(allValid.keys(),reverse=True)
        if len(so)==0: 
            logging.warning('No accessible replica. This file will be skipped.')
            return None
        retry=retry%len(so)
        return allValid[so[retry]]
        
    def prnt(self):
        logging.debug( 'file: %s:%s  size:%.3f \t attempts:%i' % (self.scope, self.name, self.size/1024/1024, self.attempts))
        for i in range(len(self.reps)):
            logging.debug('replica: %i \t endpoint: %s \t ddm: %s \t PNFS: %s \t ExpectedRate: %.3f' % (i, self.endpoints[i], self.reps[i], self.PNFS[i],self.expectedRates[i]))
            
# =========================================================================================

def getFAXendpoints():
    logging.debug('---------------getting FAX endpoints from AGISrepeater. ---------------')
    endpoints={}
    try:
        req = urllib2.Request("http://atlas-agis-api.cern.ch/request/service/query/get_se_services/?json&state=ACTIVE&flavour=XROOTD", None)
        opener = urllib2.build_opener()
        f = opener.open(req)
        res = json.load(f)
        for s in res:
            #logging.debug( s["name"]+'  '+s["rc_site"]+'  '+s["endpoint"])
            endpoints[s["rc_site"]] = endpoint(s["rc_site"],s["endpoint"], s["protocols"].keys())
        # print res
        logging.debug('Done.')
    except:
        logging.error("Could not get FAX endpoints from AGIS. Exiting...")
        logging.error("Unexpected error:%s" % str(sys.exc_info()[0]))
        sys.exit(1)
    
    #for s in sites:  endpoints[s].prnt()
    return endpoints
    
    
    
def getCostMatrix(destinationSite):
    logging.debug('---------------- Getting cost matrix values. ---------------')
    costs={}
    if not destinationSite: 
        return costs
    try:
        req = urllib2.Request("http://waniotest.appspot.com/wancostget?destination="+destinationSite, None)
        opener = urllib2.build_opener()
        f = opener.open(req)
        res=json.load(f)
        for s in res:
            logging.debug( "source: %s \t %s MB/s" %(s["site"], s["cost"]))
            costs[s["site"]] = float(s["cost"])
        logging.debug('Done.')
        return costs
    except:
        logging.error("Could not get cost matrix values. Exiting...")
        logging.error("Unexpected error:%s" % str(sys.exc_info()[0]))
        sys.exit(1)
        
def getScope(DS):
    AllScopes = rucio.client.scopeclient.ScopeClient().list_scopes()
    
    # find the scope of the dataset we are dealing with
    if DS.count(':')>0:
        spl=DS.split(':')
        scope=spl[0]
        DS=spl[1]
    else:
        w=DS.split('.')
        if w[0].startswith('user') or w[0].startswith('group'):
            scope=w[0]+'.'+w[1]
        else:
            scope=w[0]
    
    if scope not in AllScopes:
        logging.error('could not determine scope of this dataset. ')
        return None,DS
    
    return scope,DS
    
def getFiles(scope, DS, NonRoot=True):
    logging.debug('---------------- Getting files in this dataset. ---------------')
    collFiles=[]
    cont=rucio.client.didclient.DIDClient().list_content(scope,DS)
    for f in cont:
        if f['type']=='DATASET':
            collFiles+=( getFiles(f['scope'],f['name'], NonRoot) )
        else:
            if f['name'].count('.root')==0 and not NonRoot : continue
            collFiles.append(faxfile(f['scope'],f['name'],f['bytes'],f['adler32']))
    logging.debug('Done.')
    return collFiles
    
def replicaFinder():
    while True:
        f=rq.get()
        f.findReplicas()
        rq.task_done()
    
def getReplicas(Files):
    logging.debug('finding file replicas.')
    
    for i in range(10):
        t = Thread(target=replicaFinder)
        t.daemon = True
        t.start()     
    for f in Files:
        rq.put(f)
        
    rq.join()

    logging.debug('Done.')