#!/usr/bin/env python
import subprocess, threading, os, sys
import urllib2

try: import simplejson as json
except ImportError: import json

#geting coordinates 

try:
    req = urllib2.Request("http://freegeoip.net/json/", None)
    opener = urllib2.build_opener()
    f = opener.open(req)
    res=json.load(f)
    print res
    lon=res['longitude']
    lat=res['latitude']
except:
    print "Unexpected error:", sys.exc_info()[0]

# geting FAX topology information



sites=[]; # each site contains [name, host, redirector]
redirectors=[]

class site:
    name=''
    fullname=''
    host=''
    redirector=''
    direct=0
    upstream=0
    downstream=0
    comm1=''

    def __init__(self, fn, na, ho, re):
        if na=='GRIF':
            na=fn
        self.fullname=fn
        self.lname=na.lower()
        self.name=na
        self.host=ho
        self.redirector=re
        self.rucio=0
	self.lon=0
	self.lat=0

    def prnt(self, what):
        if (what>=0 and self.redirector!=what): return
        print '------------------------------------\nfullname:',self.fullname
        print 'redirector:', self.redirector, '\tname:', self.name, '\thost:', self.host
        print 'responds:', self.direct, '\t rucio:', self.rucio, '\t upstream:', self.upstream, '\t downstream:', self.downstream
        print 'lon: ', self.lon, '\t lat:', self.lat

class redirector:
    def __init__(self, name, address):
        self.name=name
        self.address=address
        self.upstream=False
        self.downstream=False
        self.status=0
    def prnt(self):
        print 'redirector: ', self.name, '\taddress: ', self.address, '\t upstream:', self.upstream, '\t downstream:', self.downstream, '\t status:', self.status

print 'Geting site list from AGIS...'



try:
    req = urllib2.Request("http://atlas-agis-api-0.cern.ch/request/service/query/get_se_services/?json&state=ACTIVE&flavour=XROOTD", None)
    opener = urllib2.build_opener()
    f = opener.open(req)
    res=json.load(f)
    for s in res:
        print  s["name"],s["rc_site"], s["endpoint"], s["redirector"]["endpoint"]
        si=site(s["name"],s["rc_site"], s["endpoint"], s["redirector"]["endpoint"])
        sites.append(si)
except:
    print "Unexpected error:", sys.exc_info()[0]


print 'Geting redirector list from AGIS...'
try:
    req = urllib2.Request("http://atlas-agis-api.cern.ch/request/service/query/get_redirector_services/?json&state=ACTIVE", None)
    opener = urllib2.build_opener()
    f = opener.open(req)
    res=json.load(f)
    for s in res:
        print s["name"], s["endpoint"]
        redirectors.append(redirector(s["name"],s["endpoint"]))
except:
    print "Unexpected error:", sys.exc_info()[0]


