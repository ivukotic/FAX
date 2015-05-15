import logging
import urllib2

try: import simplejson as json
except ImportError: import json

import rucio
import rucio.client

class endpoint:
    name=''
    host=''
    port=1094
    def __init__(self, na, ho):
        self.name=na
        ho=ho.replace("root://","")
        self.host=ho.split(":")[0]
        if ho.count(":"):
            self.port=ho.split(":")[1]
        self.ddms=[]
    def prnt(self):
        logging.debug('name: %s \thost: %s:%s' % (self.name, self.host, self.port ))
        for i in range(len(self.ddms)):
            logging.debug(self.ddms[i])

class dsfile:
    def __init__(self, sc, na, si, ad):
        self.scope=sc
        self.name=na
        self.size=si
        self.adler32=ad
        self.attempts=0
        self.reps=[]
        self.sites=[]
        self.areps=[]
        self.arepsPNFS=[]
        self.aExpectedRates=[]
    def prnt(self):
        logging.debug( 'file: %s:%s  size:%.3f \t attempts:%i' % (self.scope, self.name, self.size/1024/1024, self.attempts))
        for i in range(len(self.areps)):
            logging.debug('replica: %i \t ddm: %s \t PNFS: %s \t ExpectedRate: %.3f' % (i, self.reps[i], self.arepsPNFS[i],self.aExpectedRates[i]))
            
# =========================================================================================

def getFAXendpoints():
    logging.debug('---------------getting FAX endpoints from AGISrepeater. ---------------')
    endpoints={}
    try:
        req = urllib2.Request("http://atlas-agis-api.cern.ch/request/service/query/get_se_services/?json&state=ACTIVE&flavour=XROOTD", None)
        opener = urllib2.build_opener()
        f = opener.open(req)
        res=json.load(f)
        for s in res:
            #logging.debug( s["name"]+'  '+s["rc_site"]+'  '+s["endpoint"])
            endpoints[s["rc_site"]] = endpoint(s["rc_site"],s["endpoint"])
        # print res
        logging.debug('Done.')
    except:
        logging.error("Could not get FAX endpoints from AGIS. Exiting...")
        logging.error("Unexpected error:%s" % str(sys.exc_info()[0]))
        sys.exit(1)
    
    #for s in sites:  endpoints[s].prnt()
    return endpoints
    
    
def getCostMatrix(destinationSite):
    costs={}
    if not destinationSite: 
        return costs
    try:
        logging.debug('---------------- Getting cost matrix values. ---------------')
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
        return None
    
def getFiles(scope, DS):
    collFiles=[]
    cont=rucio.client.didclient.DIDClient().list_content(scope,DS)
    for f in cont:
        if f['type']=='DATASET':
            collFiles+=( getFiles(f['scope'],f['name']) )
        else:
            if f['name'].count('.root')==0 and not opts.NonRoot : continue
            collFiles.append(dsfile(f['scope'],f['name'],f['bytes'],f['adler32']))
    return collFiles