import logging
import urllib2

try: import simplejson as json
except ImportError: import json

class site:
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
    def toString(self):
        ret='name: %s \thost: %s:%s\n' % (self.name, self.host, self.port )
        for i in range(len(self.ddms)):
            ret += self.ddms[i] + '\n'
        return ret


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
            endpoints[s["rc_site"]] = site(s["rc_site"],s["endpoint"])
        # print res
        logging.debug('Done.')
    except:
        logging.error("Could not get FAX endpoints from AGIS. Exiting...")
        logging.error("Unexpected error:%s" % str(sys.exc_info()[0]))
        sys.exit(1)
    
    #for s in sites:  logging.debug(endpoints[s].toString())
    return endpoints