#!/usr/bin/env python
import os
import sys
import urllib
import urllib2

l=len(sys.argv)
if l==1:
    print 'Need at least a name for the test.'
    sys.exit(2)


SITE='d'
VERSION = 'd'
PANDAID = 1  
JOBTYPE                = sys.argv[1]
PARAM1        ='0'
PARAM2        ='0'
PARAM5        ='d'
if l==3: PARAM1        = sys.argv[2]
if l==4: PARAM2        = sys.argv[3]
if l==5: PARAM2        = sys.argv[4]
if l==6: PARAM5        = sys.argv[5]
FILESYSTEM    = 'd'
FILENAME      = 'd'
CPUTIME       = 0
WALLTIME      = 0
HDDTIME       = 0
HDDREADS      = 0
CACHESIZE     = 0
VMEM          = 0
RSS           = 0
USEDSWAP      = 0
EVENTS        = 0
ZIPSIZE       = 0
TOTALSIZE     = 0
SUBJOBS       = 0
CPUSTATUS     = 0
ETHERNETIN    = 0
CPUTYPE       = 'd'
ETHERNETOUT   = 0
ROOTREADS     = 0
ROOTBYTESREAD = 0  
EVENTSREAD    = 0
BRANCHESREAD       = 0
HT            = 0
TOTALCORES    = 0
ROOTVERSION   = 0
ROOTBRANCH    = 'd'

    
# if os.environ.has_key('OSG_GRID'):
#     if os.environ.has_key('OSG_SITE_NAME'):
#         SITE=os.environ['OSG_SITE_NAME']
#     else:
#         print 'OSG_GRID site but not OSG_SITE_NAME'
#         
# if os.environ.has_key('VO_ATLAS_SW_DIR'):
#     if os.environ.has_key('SITE_NAME'):
#         SITE=os.environ['SITE_NAME']
#     else:
#         print 'VO_ATLAS_SW_DIR site but no SITE_NAME'
        

# finding CPU details
with open('/proc/cpuinfo','r') as cpui:
    lines=cpui.readlines()
    CPUTYPE=''
    sib=0
    cores=0
    for l in lines:
        if l.startswith("processor"):
            w=l.split(':')
            TOTALCORES=int(w[1])+1
        if l.startswith("model name"):
            w=l.split(':')
            CPUTYPE=w[1].strip()
        if l.startswith("siblings"):
            w=l.split(':')
            sib=int(w[1])
        if l.startswith("cpu cores"):
            w=l.split(':')
            cores=int(w[1])            
    
    if sib!=cores:
        HT=1
        print 'HT is ON'
        

print 'CPUTYPE: ', CPUTYPE

with open('/proc/swaps','r') as swaps:
    lines=swaps.readlines()
    priority=9999;
    for l in lines:
        if l.startswith("Filename"):
            continue
        else:
            w=l.strip().split('\t')
            tp=int(w[3])
            if tp<priority:
                priority=tp
                USEDSWAP=int(w[2])

with open('info.txt', 'r') as f:
    lines=f.readlines()
    for l in lines:
        if '=' not in l: continue 
        # print 'executing: ', l.strip()
        try:
          exec(l)
        except:
          pass


if os.environ.has_key('PANDA_SITE_NAME'):
    SITE=os.environ['PANDA_SITE_NAME']

if os.environ.has_key('COPY_TOOL'):
    FILESYSTEM=os.environ['COPY_TOOL']

if os.environ.has_key('MACHTYPE'): 
    VERSION = os.environ['MACHTYPE']
else:
    if os.environ.has_key('HOSTTYPE'): 
        VERSION = os.environ['HOSTTYPE']

if os.environ.has_key('PandaID'):
    PANDAID = int(os.environ['PandaID'])

print 'SITE: ',SITE,"\tVERSION: ",VERSION,"\tPANDAID: ",PANDAID

print "create xml"

from xml.dom.minidom import Document
from xml.dom.ext import PrettyPrint

doc = Document()

pro = doc.createElement("project")
pro.setAttribute("name",JOBTYPE)
pro.setAttribute("parameter1",PARAM1)
doc.appendChild(pro)

sit = doc.createElement("site")
sit.setAttribute("name", SITE)
sit.setAttribute("storage",FILESYSTEM)
pro.appendChild(sit)

sto = doc.createElement("storage")
sto.setAttribute("name", FILESYSTEM)
sto.setAttribute("serverversion","unknown")
pro.appendChild(sto)

res = doc.createElement("result")
res.setAttribute("cputime", str(CPUTIME))
res.setAttribute("walltime", str(WALLTIME))
res.setAttribute("filename", str(FILENAME))
res.setAttribute("rss", str(RSS))
res.setAttribute("vmem", str(VMEM))
res.setAttribute("uswap", str(USEDSWAP))
res.setAttribute("pandaid", str(PANDAID))
pro.appendChild(res)

roo = doc.createElement("root")
roo.setAttribute("events", str(EVENTS))
roo.setAttribute("reads",str(ROOTREADS))
roo.setAttribute("ttc",str(CACHESIZE))
roo.setAttribute("totalsize", str(TOTALSIZE))
roo.setAttribute("zipsize",str(ZIPSIZE))
roo.setAttribute("bytesread", str(ROOTBYTESREAD))
roo.setAttribute("eventsread", str(EVENTSREAD))
roo.setAttribute("branchesread", str(BRANCHESREAD))
rve = doc.createElement("rootversion")
rve.setAttribute("svnversion", str(ROOTVERSION))
rve.setAttribute("name", ROOTBRANCH)
roo.appendChild(rve)
pro.appendChild(roo)

won = doc.createElement("wn")
won.setAttribute("load", str(CPUSTATUS))
won.setAttribute("networkin",str(ETHERNETIN))
won.setAttribute("networkout", str(ETHERNETOUT))
cpu = doc.createElement("cpu")
cpu.setAttribute("name", CPUTYPE)
cpu.setAttribute("ht", str(HT))
cpu.setAttribute("cores", str(TOTALCORES))
won.appendChild(cpu)
pro.appendChild(won)

if (HDDTIME!=0):
    sme = doc.createElement("storagemeasurements")
    sme.setAttribute("reads",str(HDDREADS))
    sme.setAttribute("time", str(HDDTIME))
    sme.setAttribute("merged",str(0))
    sme.setAttribute("vector", str(0))
    pro.appendChild(sme)


print doc.toprettyxml(indent="  ")

PrettyPrint(doc, open(JOBTYPE+".xml", "w"))

url = 'http://ivukotic.web.cern.ch/ivukotic/HC/addResult.asp'
values = {'result' : doc.toxml() }

data = urllib.urlencode(values)
req = urllib2.Request(url, data)
response = urllib2.urlopen(req)
the_page = response.read()
print the_page

