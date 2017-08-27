#! /usr/bin/env python
#
# Example Python module for prepaid usage using MySQL

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
#
# Copyright 2002 Miguel A.L. Paraz <mparaz@mparaz.com>
# Copyright 2002 Imperium Technology, Inc.
#
# $Id$

import radiusd
import MySQLdb

# Configuration
configDb = 'python'                     # Database name
configHost = 'localhost'                # Database host
configUser = 'python'                   # Database user and password
configPasswd = 'python'         

# xxx Database 

# Globals
dbHandle = None

def log(level, s):
  """Log function."""
  radiusd.radlog(level, 'prepaid.py: ' + s)

def instantiate(p):
  """Module Instantiation.  0 for success, -1 for failure.
  p is a dummy variable here."""
  global dbHandle

  p = p

  try:
    dbHandle = MySQLdb.connect(db=configDb, host=configHost,
                               user=configUser, passwd=configPasswd)

  except MySQLdb.OperationalError, e:
    # Report the error and return -1 for failure.
    # xxx A more advanced module would retry the database.
    log(radiusd.L_ERR, str(e))
    return -1

  log(radiusd.L_INFO, 'db connection: ' + str(dbHandle))

  return 0


def authorize(authData):
  """Authorization and authentication are done in one step."""

  # Extract the data we need.
  userName = None
  userPasswd = None
  
  for t in authData:
    if t[0] == 'User-Name':
      userName = t[1]
    elif t[0] == 'Password':
      userPasswd = t[1]

  # Build and log the SQL statement
  # radiusd puts double quotes (") around the string representation of
  # the RADIUS packet.
  sql = 'select passwd, maxseconds from users where username = ' +  userName
  
  log(radiusd.L_DBG, sql)

  # Get a cursor
  # xxx Or should this be one cursor all throughout?
  try:
    dbCursor = dbHandle.cursor()
  except MySQLdb.OperationalError, e:
    log(radiusd.L_ERR, str(e))
    return radiusd.RLM_MODULE_FAIL

  # Execute the SQL statement
  try:
    dbCursor.execute(sql)
  except MySQLdb.OperationalError, e:
    log(radiusd.L_ERR, str(e))
    dbCursor.close()
    return radiusd.RLM_MODULE_FAIL
  
  # Get the result. (passwd, maxseconds)
  result = dbCursor.fetchone()
  if not result:
    # User not found
    log(radiusd.L_INFO, 'user not found: ' + userName)
    dbCursor.close()
    return radiusd.RLM_MODULE_NOTFOUND


 
  # Compare passwords
  # Ignore the quotes around userPasswd.
  if result[0] != userPasswd[1:-1]:
    log(radiusd.L_DBG, 'user password mismatch: ' + userName)
    return radiusd.RLM_MODULE_REJECT

  maxSeconds = result[1]

  # Compute their session limit
  
  # Build and log the SQL statement
  sql = 'select sum(seconds) from sessions where username = ' + userName

  log(radiusd.L_DBG, sql)
  
  # Execute the SQL statement
  try:
    dbCursor.execute(sql)
  except MySQLdb.OperationalError, e:
    log(radiusd.L_ERR, str(e))
    dbCursor.close()
    return radiusd.RLM_MODULE_FAIL
  
  # Get the result. (sum,)
  result = dbCursor.fetchone()
  if (not result) or (not result[0]):
    # No usage yet
    secondsUsed = 0
  else:
    secondsUsed = result[0]

  # Done with cursor
  dbCursor.close()

  # Note that MySQL returns the result of SUM() as a float.
  sessionTimeout = maxSeconds - int(secondsUsed)
  
  if sessionTimeout <= 0:
    # No more time, reject outright
    log(radiusd.L_INFO, 'user out of time: ' + userName)    
    return radiusd.RLM_MODULE_REJECT

  # Log the success
  log(radiusd.L_DBG, 'user accepted: %s, %d seconds' %
      (userName, sessionTimeout))

  # We are adding to the RADIUS packet
  # Note that the session timeout integer must be converted to string.
  # We need to set an Auth-Type.

  return (radiusd.RLM_MODULE_UPDATED,
          (('Session-Timeout', str(sessionTimeout)),),
          (('Auth-Type', 'python'),))
  # If you want to use different operators 
  # you can do
  # return (radiusd.RLM_MODULE_UPDATED,
  #         radiusd.resolve(
  #            'Session-Timeout := %s' % str(sessionTimeout),
  #            'Some-other-option -= Value',
  #         ),
  #         radiusd.resolve(
  #            'Auth-Type := python'
  #         )
  #        ) 
  # Edit operators you need in OP_TRY in radiusd.py

def authenticate(p):
  p = p
  return radiusd.RLM_MODULE_OK


def preacct(p):
  p = p
  return radiusd.RLM_MODULE_OK


def accounting(acctData):
  """Accounting."""
  # Extract the data we need.

  userName = None
  acctSessionTime = None
  acctStatusType = None

  # xxx A dict would make this nice.
  for t in acctData:
    if t[0] == 'User-Name':
      userName = t[1]
    elif t[0] == 'Acct-Session-Time':
      acctSessionTime = t[1]
    elif t[0] == 'Acct-Status-Type':
      acctStatusType = t[1]


  # We will not deal with Start for now.
  # We may later, for simultaneous checks and the like.
  if acctStatusType == 'Start':
    return radiusd.RLM_MODULE_OK
  
  # Build and log the SQL statement
  # radiusd puts double quotes (") around the string representation of
  # the RADIUS packet.
  #
  # xxx This is simplistic as it does not record the time, etc.
  # 
  sql = 'insert into sessions (username, seconds) values (%s, %d)' % \
        (userName, int(acctSessionTime))
  
  log(radiusd.L_DBG, sql)

  # Get a cursor
  # xxx Or should this be one cursor all throughout?
  try:
    dbCursor = dbHandle.cursor()
  except MySQLdb.OperationalError, e:
    log(radiusd.L_ERR, str(e))
    return radiusd.RLM_MODULE_FAIL

  # Execute the SQL statement
  try:
    dbCursor.execute(sql)
  except MySQLdb.OperationalError, e:
    log(radiusd.L_ERR, str(e))
    dbCursor.close()
    return radiusd.RLM_MODULE_FAIL

  
  return radiusd.RLM_MODULE_OK


def detach():
  """Detach and clean up."""
  # Shut down the database connection.
  global dbHandle
  log(radiusd.L_DBG, 'closing database handle: ' + str(dbHandle))
  dbHandle.close()

  return radiusd.RLM_MODULE_OK



# Test the modules 
if __name__ == '__main__':
  instantiate(None)
  print authorize((('User-Name', '"map"'), ('User-Password', '"abc"')))
