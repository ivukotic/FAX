#!/usr/bin/env python
import subprocess, threading, os, sys, cx_Oracle

class Command(object):
    def __init__(self, cmd):
        self.cmd = cmd
        self.process = None
    
    def run(self, timeout, foreground=False):
        def target():
            print 'command started: ', self.cmd
            self.process = subprocess.Popen(self.cmd, shell=True)
            if (foreground): self.process.communicate()
            # print 'command finished'
        
        thread = threading.Thread(target=target)
        thread.start()
        
        thread.join(timeout)
        if thread.is_alive():
            print 'Terminating process'
            self.process.terminate()
            thread.join()
        return self.process.returncode


line='ATLAS_HCLOUDTEST/HCLOUDTEST_2012_2@(DESCRIPTION=(ADDRESS=(PROTOCOL=TCP)(HOST=intr1-v.cern.ch)(PORT=10121))(ADDRESS=(PROTOCOL=TCP)(HOST=intr2-v.cern.ch)(PORT=10121))(LOAD_BALANCE=yes)(CONNECT_DATA=(SERVER=DEDICATED)(SERVICE_NAME=intr.cern.ch)(FAILOVER_MODE=(TYPE=SELECT)(METHOD=BASIC)(RETRIES=200)(DELAY=15))))'

SITE='d'
PANDAID = 1  

if os.environ.has_key('PANDA_SITE_NAME'):
    SITE=os.environ['PANDA_SITE_NAME']

if os.environ.has_key('PandaID'):
    PANDAID = int(os.environ['PandaID'])

print 'CandC => SITE: ',SITE, "\tPANDAID: ",PANDAID



try:
    connection = cx_Oracle.Connection(line)
    cursor = cx_Oracle.Cursor(connection)
    print 'CandC => connection established.'
    cursor.execute("select candc.commandid, command from candc, candcresult where candc.commandid=candcresult.commandid and (candc.site='"+SITE+"' or candc.site='ALL') and not candcresult.site='"+SITE+"' union select commandid, command from candc where not exists( select * from candcresult where candcresult.commandid=candc.commandid )")
    res = cursor.fetchall()
    cursor.close()
    id=0;
    for r in res:
        id=r[0]
        comm=Command(r[1] + ' >& c.out')
        print 'commandid:', id
        print 'command  :', r[1]
        print 'pandaID  :', PANDAID
        comm.run(1800,True);
        
        with open('c.out', 'r') as f:
            result=f.read(4000)
        
        cursor1 = cx_Oracle.Cursor(connection)
        inscom="INSERT INTO candcresult  (commandid, site, pandaid, result) VALUES ("+str(id)+",'"+SITE+"',"+str(PANDAID)+",'"+result.strip()+"')"
        cursor1.execute(inscom)
        cursor1.close()
        connection.commit()
    print 'CandC => DONE.'
except cx_Oracle.DatabaseError, exc:
    error, = exc.args
    print "sleeper.py - problem in establishing connection to db"
    print "sleeper.py Oracle-Error-Code:", error.code
    print "sleeper.py Oracle-Error-Message:", error.message
