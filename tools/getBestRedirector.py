#!/usr/bin/env python
import subprocess, threading, os, sys, time 
import urllib2
from  datetime import datetime
from datetime import timedelta
import math

try: import simplejson as json
except ImportError: import json

#trying to resolve localredirector

import socket
debug=0

try:
	socket.gethostbyname('localredirector')
	if debug: print 'localredirector exist. using it...'
        print 'export LOCALREDIRECTOR=localredirector'
	sys.exit()
except (IOError, socket.gaierror, socket.error):
	if debug: print 'localredirector does not resolve.'


#geting coordinates 

try:
    req = urllib2.Request("http://freegeoip.net/json/", None)
    opener = urllib2.build_opener()
    f = opener.open(req)
    res=json.load(f)
    if debug: print res
    lon=res['longitude']
    lat=res['latitude']
except:
    print "Can't determine client coordinates. Setting local redirector to glrd.usatlas.org .", sys.exc_info()[0]
    print 'export LOCALREDIRECTOR=glrd.usatlas.org'
    sys.exit()

# geting FAX endpoints information from SSB


class site:
        name=''
        direct=0
        directfails=timedelta(0)
        def __init__(self,na):
                self.name=na
	def coo(self, lo, la):
		self.longitude=lo
		self.latitude=la
        def prn(self):
                print self.name, "\tlong:",self.longitude,"\t lat:",self.latitude, "\tdirect: ",self.directfails,"/",self.direct


#direct
url="http://dashb-atlas-ssb.cern.ch/dashboard/request.py/getplotdata?time=1&dateFrom=&dateTo=&sites=all&clouds=all&batch=1&columnid=10083"
try:
        response=urllib2.Request(url,None)
        opener = urllib2.build_opener()
        f = opener.open(response)
        data = json.load(f)
        data=data["csvdata"]
except urllib2.HTTPError:
	print "Can't connect ot SSB. Setting localRedirector to glrd.usatlas.org ."
        print 'export LOCALREDIRECTOR=glrd.usatlas.org'
	sys.exit()
except:
    	print "Unexpected error:", sys.exc_info()[0]
	print 'export LOCALREDIRECTOR=glrd.usatlas.org'
	sys.exit()


Sites=dict()

curtime=datetime.now()
cuttime=curtime-timedelta(0,5*3600)

for si in data:
        n=si['VOName']
        if n not in Sites:
                s=site(n)
                Sites[n]=s
        st= datetime(*(time.strptime(si["Time"], '%Y-%m-%dT%H:%M:%S')[0:6])) #datetime.strptime(si["Time"],'%Y-%m-%dT%H:%M:%S')
	et= datetime(*(time.strptime(si["EndTime"], '%Y-%m-%dT%H:%M:%S')[0:6])) #datetime.strptime(si["EndTime"],'%Y-%m-%dT%H:%M:%S')
        if et<cuttime: continue  # throws away too early measurements
        if st<cuttime: st=cuttime # cuts to exact cutoff time
        if et>curtime: # current state
                et=curtime
                Sites[n].direct=si['COLOR']
        if si['COLOR']==3: Sites[n].directfails+= (et-st)

try:
    req = urllib2.Request("http://atlas-agis-api.cern.ch/request/site/query/list/?json", None)
    opener = urllib2.build_opener()
    f = opener.open(req)
    res=json.load(f)
    for s in res:
	na=s["name"]
	if na in Sites:
        	Sites[na].coo(s["longitude"],s["latitude"])
		if debug>2: print  s["name"],s["rc_site"], s["latitude"], s["longitude"]
except urllib2.HTTPError:
        print "Can't connect ot AGIS to get coordinates of endpoints. Setting localRedirector to glrd.usatlas.org ."
        print 'export LOCALREDIRECTOR=glrd.usatlas.org'
        sys.exit()
except:
    print "Unexpected error:", sys.exc_info()[0]
    print "Can't connect ot AGIS to get coordinates of endpoints. Setting localRedirector to glrd.usatlas.org ."
    print 'export LOCALREDIRECTOR=glrd.usatlas.org'
    sys.exit()


# calculating distances to "green" endpoints
mindist=40000
minsite=''

for s in Sites:
	if not Sites[s].direct==5: continue

	dlon = math.radians(lon - Sites[s].longitude)
	dlat = math.radians(lat - Sites[s].latitude)
	a = pow(math.sin(dlat/2),2) + math.cos(math.radians(Sites[s].latitude)) * math.cos(math.radians(lat)) * pow(math.sin(dlon/2),2)
	c = 2 * math.atan2( math.sqrt(a), math.sqrt(1-a) )
	d = 6373 * c # 6373 is the radius of the Earth in km
	if debug>1:
		Sites[s].prn()
		print d,"km"
	if d<mindist:
		mindist=d
		minsite=Sites[s].name

if debug: print minsite, mindist,"km"

#finding out endpoint address

try:
    req = urllib2.Request("http://atlas-agis-api.cern.ch/request/service/query/get_se_services/?json&state=ACTIVE&flavour=XROOTD&rc_site="+minsite, None)
    opener = urllib2.build_opener()
    f = opener.open(req)
    res=json.load(f)
    if debug: print "found corresponding endpoint: ",res[0]["endpoint"]
    print 'export LOCALREDIRECTOR='+res[0]["endpoint"]
except urllib2.HTTPError:
    print "Can't connect ot AGIS to get coordinates of endpoints. Setting localRedirector to glrd.usatlas.org ."
    print 'export LOCALREDIRECTOR=glrd.usatlas.org'
except:
    print "Unexpected error:", sys.exc_info()[0]
    print "Can't connect ot AGIS to get coordinates of endpoints. Setting localRedirector to glrd.usatlas.org ."
    print 'export LOCALREDIRECTOR=glrd.usatlas.org'

