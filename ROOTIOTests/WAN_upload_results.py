#!/usr/bin/env python
import os
import sys
import cx_Oracle

line='ATLAS_ATHENAIOPERF/Leptir32@(DESCRIPTION=(ADDRESS=(PROTOCOL=TCP)(HOST=intr1-v.cern.ch)(PORT=10121))(ADDRESS=(PROTOCOL=TCP)(HOST=intr2-v.cern.ch)(PORT=10121))(LOAD_BALANCE=yes)(CONNECT_DATA=(SERVER=DEDICATED)(SERVICE_NAME=intr.cern.ch)(FAILOVER_MODE=(TYPE=SELECT)(METHOD=BASIC)(RETRIES=200)(DELAY=15))))'

l=len(sys.argv)
if l>1:
    print 'uploading information from ', sys.argv[1]
    
    PING      = 0
    SITE      = 'd'
    SITE1     = (sys.argv[1].split('.'))[0]
    PANDAID   = 1  
    FILENAME  = 'd'
    COPYTIME  = 0
    WALLTIME  = 0
    ZIPSIZE   = 0
    ROOTREADS = 0
    VERSION   = 'd'




    JOBTYPE   = '100% default cache'
    PARAM1    = '100'
    PARAM2    = '30'
    PARAM3    = '0'    
    
    print  JOBTYPE, PARAM1, PARAM2, PARAM3
    
    if os.environ.has_key('PANDA_SITE_NAME'):
        SITE=os.environ['PANDA_SITE_NAME']
    if os.environ.has_key('PandaID'):
        PANDAID = int(os.environ['PandaID'])


    if os.environ.has_key('MACHTYPE'): 
        VERSION = os.environ['MACHTYPE']
    else:
        if os.environ.has_key('HOSTTYPE'): 
            VERSION = os.environ['HOSTTYPE']

    print 'SITE: ',SITE,'\tSITE1: ',SITE1,"\tVERSION: ",VERSION,"\tPANDAID: ",PANDAID     

    with open(sys.argv[1], 'r') as f:
        lines=f.readlines()
        for l in lines:
            if '=' not in l: continue 
            # print 'executing: ', l.strip()
            try:
                exec(l)
            except:
                pass

    print FILENAME, COPYTIME, WALLTIME, ZIPSIZE, ROOTREADS
     
    try:
        connection = cx_Oracle.Connection(line)
        cursor = cx_Oracle.Cursor(connection)
        print 'Connection established.'
        all=[JOBTYPE, PARAM1, PARAM2, PARAM3, SITE, SITE1, PANDAID, FILENAME, PING, COPYTIME, WALLTIME, ZIPSIZE, ROOTREADS]
        print all
        cursor.callproc('INSERTWANRESULT', all)
        connection.commit()
        print 'Uploaded'
        print
    except cx_Oracle.DatabaseError, exc:
        error, = exc.args
        print "WANtests.py Oracle-Error-Code:", error.code
        print "WANtests.py Oracle-Error-Message:", error.message
     
