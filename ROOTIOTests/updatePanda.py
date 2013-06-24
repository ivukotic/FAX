import os
import sys
import cx_Oracle

line='ATLAS_HCLOUDTEST/HCLOUDTEST_2012_2@(DESCRIPTION=(ADDRESS=(PROTOCOL=TCP)(HOST=intr1-v.cern.ch)(PORT=10121))(ADDRESS=(PROTOCOL=TCP)(HOST=intr2-v.cern.ch)(PORT=10121))(LOAD_BALANCE=yes)(CONNECT_DATA=(SERVER=DEDICATED)(SERVICE_NAME=intr.cern.ch)(FAILOVER_MODE=(TYPE=SELECT)(METHOD=BASIC)(RETRIES=200)(DELAY=15))))'

try:
    connection = cx_Oracle.Connection(line)
    cursor = cx_Oracle.Cursor(connection)
    print 'Connection established.'
    ids=[]
    cursor.execute("SELECT UNIQUE pandaid FROM result WHERE not exists (Select pandaid from panda where result.pandaid=panda.pandaid)")
    
    res = cursor.fetchall()
    cursor.close()
    for r in res:
        ids.append(r[0])
    print 'got', len(ids), 'new pandaIDs'
    print
except cx_Oracle.DatabaseError, exc:
    error, = exc.args
    print "uploader.py - problem in establishing connection to db"
    print "uploader.py Oracle-Error-Code:", error.code
    print "uploader.py Oracle-Error-Message:", error.message

print 'leaving only first 200 in the list.'
del ids[2000:]


from pandatools import Client
status,jobSpec = Client.getFullJobStatus(ids,False)
print 'got back from panda:', len(jobSpec)

try:
    connection = cx_Oracle.Connection(line)
    cursor = cx_Oracle.Cursor(connection)
    print 'Connection established.'
    for i in jobSpec:
        if i is None:
            print "problem..."
            continue 
        ti=i.pilotTiming.split('|')
        if len(ti)!=5:
            print i.PandaID,i.pilotTiming,ti
            cursor.execute("DELETE FROM result where pandaid=:pid",{'pid':i.PandaID});
            continue
        cursor.execute("INSERT INTO panda (pandaid, stagein, stageout, exec, setup, status, cmtconfig, atlasrelease, modificationhost) VALUES (:pid,:sin,:sout,:exe,:setu,:sta,:cmt,:rel,:hos)",{'pid':i.PandaID, 'sin':ti[1], 'sout':ti[3], 'exe':ti[2], 'setu':ti[4], 'sta':i.transExitCode, 'cmt':i.cmtConfig, 'rel':i.AtlasRelease, 'hos':i.modificationHost })
        print i.PandaID
    cursor.close()
    
    connection.commit()
    print 'new PandaIDs populated.'
    print
except cx_Oracle.DatabaseError, exc:
    error, = exc.args
    print "uploader.py - problem in establishing connection to db"
    print "uploader.py Oracle-Error-Code:", error.code
    print "uploader.py Oracle-Error-Message:", error.message

