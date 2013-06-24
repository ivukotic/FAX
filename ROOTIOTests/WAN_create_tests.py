#!/usr/bin/env python
import os
import sys
import cx_Oracle

line='ATLAS_ATHENAIOPERF/Leptir32@(DESCRIPTION=(ADDRESS=(PROTOCOL=TCP)(HOST=intr1-v.cern.ch)(PORT=10121))(ADDRESS=(PROTOCOL=TCP)(HOST=intr2-v.cern.ch)(PORT=10121))(LOAD_BALANCE=yes)(CONNECT_DATA=(SERVER=DEDICATED)(SERVICE_NAME=intr.cern.ch)(FAILOVER_MODE=(TYPE=SELECT)(METHOD=BASIC)(RETRIES=200)(DELAY=15))))'
sites=[];

print 'Geting site list ...'

try:
    connection = cx_Oracle.Connection(line)
    cursor = cx_Oracle.Cursor(connection)
    print 'Connection established.'

    cursor.execute("SELECT siteid, name, server, url, filepath, protocol, debug FROM site WHERE role='both' or role='server'")
    res = cursor.fetchall()
    cursor.close()
    for r in res:
        sites.append([r[0],r[1],r[2],r[3],r[4],r[5],r[6]])
        print r[1],r[2],r[3],r[4],r[5],r[6]
    print 'got', len(sites), 'sites.'
    print
except cx_Oracle.DatabaseError, exc:
    error, = exc.args
    print "WANtests.py - problem in establishing connection to db"
    print "WANtests.py Oracle-Error-Code:", error.code
    print "WANtests.py Oracle-Error-Message:", error.message

print 'creating scripts to execute'
    
    
with open('toExecute.sh', 'w') as f:
    for s in sites:
        fn=s[5]+s[2]+s[3]+s[4]
        logfile = s[1] + '.log'
        
        #  ping 
        f.write("""./timeout3.sh -t 30 ping -c 5 -n -q """+ s[2] +"""  | awk \'{if ( index($1,"PING")>0 ) r1=$2;  if ( index($1,"rtt")>0 ) {split($4,a,"/"); print "PING=", a[2], "#" , $NF } }\' >> """+ logfile +""" 2>&1 \n""");
        
        # xrdcp
        f.write('./timeout3.sh -t 3600 `which time`  -f "COPYTIME=%e" --append -o '+ logfile +' xrdcp -np -f ' + fn + """ \dev\\null  2>&1 \n""")
        
        # turning debug on/off
        f.write(s[6] + """\n""")
        
        # remote read
        f.write('./timeout3.sh -t 3600 root -l -q -b "read.C++(\\"' + fn + '\\",\\"physics\\", 100, 30 )" >> '+ logfile +' 2>&1 \n')
        
        # results upload
        f.write('python WANtests.py '+ logfile +' \n\n')
    f.close()
    

