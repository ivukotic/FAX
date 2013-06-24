#!/usr/bin/env python
import cx_Oracle
import sys
import os

line               = 'ATLAS_ATHENAIOPERF/Sophia2010@(DESCRIPTION=(ADDRESS=(PROTOCOL=TCP)(HOST=intr1-v.cern.ch)(PORT=10121))(ADDRESS=(PROTOCOL=TCP)(HOST=intr2-v.cern.ch)(PORT=10121))(LOAD_BALANCE=yes)(CONNECT_DATA=(SERVER=DEDICATED)(SERVICE_NAME=intr.cern.ch)(FAILOVER_MODE=(TYPE=SELECT)(METHOD=BASIC)(RETRIES=200)(DELAY=15))))'

try:
    connection     = cx_Oracle.Connection(line)
    cursor         = cx_Oracle.Cursor(connection)
    print 'Connection established.'

except cx_Oracle.DatabaseError, exc:
    error,         = exc.args
    print "problem in establishing connection to db"
    print "Oracle-Error-Code:", error.code
    print "Oracle-Error-Message:", error.message
except:
    print "Unexpected error (establishConnection 1st step):", sys.exc_info()[0]

try:    
    comm       = "delete from gridjob"
    cursor.execute(comm)
    connection.commit()
    print 'Database cleared.'
except cx_Oracle.DatabaseError, exc:
    error,     = exc.args
    print "problem in deleting stuff"
    print "Oracle-Error-Code:", error.code
    print "Oracle-Error-Message:", error.message
except:
    print "Unexpected error:", sys.exc_info()[0]    

try:
    connection.close() 
    print 'Connection closed.'
except cx_Oracle.DatabaseError, exc:
    error, = exc.args
    print "problem in closing connection to db"
    print "Oracle-Error-Code:", error.code
    print "Oracle-Error-Message:", error.message
except:
    print "Unexpected error (closeConnection):", sys.exc_info()[0]
