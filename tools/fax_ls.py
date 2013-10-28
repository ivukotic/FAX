#!/usr/bin/env python

import os, sys
import json
import urllib2 as urllib
import cPickle as pickle
import optparse
import fnmatch
import commands,time,os

# get xrootd path for a given dset
# from dq2.filecatalog.lfc.lfcconventions import to_native_ldn
# to_native_ldn(dsetName, "", "/atlas/dq2")

PathPrefix = "/atlas/dq2"
timeout = 10  # 10 second

#get date in format YYYYmmdd
from datetime import date
TODAY = date.today().strftime("%Y%m%d")
pickleFile = "FAX_servers-" + TODAY + ".pickle"

usage = """
     %prog [options] dsetListFile
  or
     %prog [options] dsetNamePattern
  or
     %prog [options] dsetNamePattern1,dsetNamePattern2[,more name patterns]
"""
 
optParser = optparse.OptionParser(usage=usage,conflict_handler="resolve")
optParser.add_option('-v',action='store_const',const=True,dest='verbose',
                  default=False, help='Verbose')
optParser.add_option('-r','--readPickle', action='store',dest='pickleFile',
                  type='string', default='',
                  help='Read from pickle filename to get xrootd servers, \notherwise get from ATLAS-AGIS, and later save into file '+pickleFile)
optParser.add_option('-p', action='store_const', const=True, dest='pfnGen',
                  default=False, help='Generate xrootd PFN (Physical File Name) list')
optParser.add_option('-o','--outPFNFile', action='store',dest='outPFNFile',
                  type='string', default='',
                  help='Use with option -p, write pfn list into a file instead of print to screen')

# parse options and args
options, args = optParser.parse_args()
if options.verbose:
    print options
    print "args=",args
    print

if len(args) == 0:
    optParser.print_help()
    optParser.exit(1)


readPickle = False
if options.pickleFile != '':
  if os.path.isfile(options.pickleFile):
    readPickle = True
    pickleFile = options.pickleFile

outPFN = None
if options.pfnGen:
   status,output = commands.getstatusoutput("which xrd")
   if status != 0:
      sys.exit("Warning!! Command \"xrd\" not found, pls set ROOT env first")
   outPFNFile = options.outPFNFile
   if len(outPFNFile)>0:
      outPFN = open(outPFNFile, 'w')

# bothRepList={}

#-------------------------------------
def getFax_xrds(pickleFile,readPickle):
#-------------------------------------
  if readPickle:
     FaxDDMs = pickle.load( open( pickleFile, "rb" ) )
     return FaxDDMs

  sites={}
  try:
     lines = urllib.urlopen("http://atlas-agis-api-0.cern.ch/request/service/query/get_se_services/?json&state=ACTIVE&flavour=XROOTD", None).read()
     res = json.loads(lines)
     for s in res:
        sites[s["rc_site"].upper()] = {"xrdServer":s["endpoint"],
                                       "redirector":s["redirector"]["endpoint"]}
  except:
     print "Unexpected error:", sys.exc_info()[0]    


  FaxDDMs={}
  try:
     lines = urllib.urlopen("http://atlas-agis-api-0.cern.ch/request/ddmendpoint/query/list/?json&state=ACTIVE", None).read()
     res = json.loads(lines)
     for s in res:
        mySite = s["rc_site"]
        if mySite in sites:
           FaxDDMs[s["name"]] = sites[mySite]["xrdServer"]
  except:
     print "Unexpected error:", sys.exc_info()[0]    

  pickle.dump( FaxDDMs, open( pickleFile, "wb" ) )
  return FaxDDMs


#print 'Geting data from AGIS ...'

FaxDDMs = getFax_xrds(pickleFile,readPickle)

#----------------------------
def matchFax(bothRep, FaxDDMs):
#----------------------------
   completeFax = []
   completeRep = bothRep["COMPLETE"]
   incompFax   = []
   incompRep   = bothRep["INCOMPLETE"]

   for rep in completeRep:
      if rep in FaxDDMs:
         completeFax += [rep]

   for rep in incompRep:
      if rep in FaxDDMs:
         incompFax += [rep]

   return {"COMPLETE":completeFax, "INCOMPLETE":incompFax}

#-------------------------
def getFaxSpeed(xrdServer):
#-------------------------
   if options.verbose:
      print "Come to getFaxSpeed=", xrdServer
   speed = timeout  # default period if no right response received from xrd server
   if os.path.isfile("/usr/bin/timeout"):
      timeout_cmd = "/usr/bin/timeout %ds" % timeout
   else:
      timeout_cmd = ""
   xrdCmd = "%s xrd %s <<EOF\nexit\nEOF" % (timeout_cmd, xrdServer.lstrip("root://"))
   start = time.time()
   output = os.popen(xrdCmd).read()
   end = time.time()
   if output.find("root://") < 0:
      return speed
   speed = end - start
   if options.verbose:
      print "::getFaxSpeed::, speed=",speed
   return speed

#-----------------------
def getFastestFax(sites):
#-----------------------
   if options.verbose:
      print "Come to getFastestFax for sites=", sites
   bestSpeed = timeout
   if len(sites) == 0:
      return []
   bestSite = sites[0]
   if options.verbose:
      print "::getFastestFax::, 1st site=",bestSite
   for site in sites:
      siteSpeed = getFaxSpeed(site)
      if options.verbose:
         print "::getFastestFax:: site=",site, ";bestSite=",bestSite, ";siteSpeed=",siteSpeed, ";bestSpeed=",bestSpeed
      if siteSpeed < bestSpeed:
         bestSite = site
         bestSpeed = siteSpeed
   return bestSite

#------------------------------------
def matchFastestFax(bothRep, FaxDDMs):
#------------------------------------
   if options.verbose:
      print "Come to matchFastestFax"
   completeXrd = []
   completeRep = bothRep["COMPLETE"]
   incompXrd   = []
   incompRep   = bothRep["INCOMPLETE"]

   for rep in completeRep:
      rep = rep.lstrip()
      if options.verbose:
         print "::matchFastestFax::, Complete rep=",rep
      if rep in FaxDDMs:
         completeXrd += [FaxDDMs[rep]]

   if options.pfnGen:
      fastestCompXrd = getFastestFax(completeXrd)
   else:
      fastestCompXrd = completeXrd

   for rep in incompRep:
      if rep in FaxDDMs:
         incompXrd += [FaxDDMs[rep]]

   if options.pfnGen:
      fastestIncompXrd = getFastestFax(incompXrd)
   else:
      fastestIncompXrd = incompXrd

   return {"COMPLETE":fastestCompXrd, "INCOMPLETE":fastestIncompXrd}


dsetListName=args[0]
try:
  listFile = open(dsetListName)
  dsetNames = listFile.readlines()
  listFile.close()
except:
  sys.exc_clear()
  # print "dsetListName=",dsetListName
  dsetNames = dsetListName.split(',')
  pass

# print "dsetNames=",dsetNames

old_argv = sys.argv
sys.argv = []
try:
  from dq2.filecatalog.lfc.lfcconventions import to_native_ldn
except:
  sys.exit("Warning!! Command \"DQ2Client\" not set, pls set it first")

from dq2.clientapi.cli.Ls import Ls
myLs = Ls()

from cStringIO import StringIO
mystdout = StringIO()
old_stdout = sys.stdout
# sys.stdout = mystdout

dsetsInFax={}
for dsPattern in dsetNames:
  DS = dsPattern.strip()
  # print >> old_stdout, "DS=",DS
  sys.stdout = mystdout
  sys.argv = ["", "-r", DS]
  myLs.execute()
  sys.stdout = old_stdout
  mystdout.reset()
  dsets = []
  dsetName = None
  blockRep = False
  for line in mystdout:
    if line.startswith('Multiple'): break
    line=line.strip()
    if options.verbose:
       print "line=",line
    if len(line) == 0: continue
    if fnmatch.fnmatch(line.strip(':'),DS):
       # process last dset
       if dsetName:
          if blockRep:
             bothRepList[repType] = repList
          bothFaxRep = matchFastestFax(bothRepList,FaxDDMs)
          dsets += [[dsetName,bothFaxRep]]
       dsetName = line.strip(":")
       if options.verbose: print "Matching dsetName=",dsetName
       blockRep = False
       bothRepList = {"COMPLETE":[],"INCOMPLETE":[]}
       repList = []
       continue
    if line.find("COMPLETE:") >= 0:
       if blockRep:
          bothRepList[repType] = repList
       repList = line.split(':')[1]
       if len(repList) > 0:
          repList = repList.strip().split(',')
       else:
          repList = []
       blockRep = True
       repType = "COMPLETE"
       if line.startswith("INCOMPLETE:"):
          repType = "INCOMPLETE"
       continue
    if blockRep:
       # to make it work for old version of DQ2Cleint
       #  which return each site per line
       if options.verbose:
          print "Within blockRep, line=",line, ", repType=",repType
       repList += [line]

  if dsetName:
     if blockRep:
        bothRepList[repType] = repList
     bothFaxRep = matchFastestFax(bothRepList,FaxDDMs)
     dsets += [[dsetName,bothFaxRep]]

  mystdout.reset()
  mystdout.truncate()

  for dsetName,bothRepList in dsets:
      print "\ndsetName=",dsetName, "\n FAX Replica list="
      compList = bothRepList['COMPLETE']
      print "\tCOMPLETE:",compList
      print "\tINCOMPLETE:",bothRepList['INCOMPLETE']
      # print "compList=",compList
      if len(compList) > 0 and options.pfnGen:
         dsetPath = to_native_ldn(dsetName, "", PathPrefix)
         # print "dsetPath=",dsetPath
         sys.stdout = mystdout
         sys.argv = ["", "-f", dsetName]
         myLs.execute()
         sys.stdout = old_stdout
         mystdout.reset()

         filenames=[]
         for line in mystdout:
            if line[0] != '[':
               continue
            filename = line[3:].split()[0]
            filenames += [filename]
         filenames.sort()

         stdout_pfn = old_stdout
         if outPFN != None:
            stdout_pfn = outPFN
         for filename in filenames:
            if filename.find(".root") >= 0:
               path_filename = compList + '/' + dsetPath + filename
               print >>stdout_pfn, path_filename

         mystdout.reset()
         mystdout.truncate()
         sys.stdout = old_stdout

      # print "dsetName=",dsetName, "\n Complete replicas=",bothRepList["Complete"], "\n InComplete replicas=",bothRepList["InComplete"]


if outPFN != None:
   outPFN.close()

sys.stdout = old_stdout
sys.argv = old_argv

# for d  in dsets.keys():
#    print d,'\tcomplete replicas:',dsets[d][1],'\tincomplete:',dsets[d][0]
