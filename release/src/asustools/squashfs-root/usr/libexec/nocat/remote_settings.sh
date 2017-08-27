#!/bin/sh
export PATH=/tmp/sbin:/tmp/bin:/bin:/usr/bin:/sbin:/usr/sbin

#use the remote settings url and target script
REMOTE_SETTINGS_URL=$1
SCRIPT=$2

# verbosity
VERBOSITY=`/usr/sbin/nvram get NC_Verbosity`

#use the same time out here
CHECK_TIMEOUT=`/usr/sbin/nvram get NC_check_timeout`

if [ $VERBOSITY -gt 0 ]; then /bin/logger "settings:download from: $REMOTE_SETTINGS_URL with timeout: $CHECK_TIMEOUT"; fi

MY_PID=$$

(/bin/sleep $CHECK_TIMEOUT; /bin/kill -1 $MY_PID) &
CONTROL_PID=$!

/bin/wget $REMOTE_SETTINGS_URL -O- 2>&1 > $SCRIPT & 
CHECK_PID=$!

if [ $VERBOSITY -gt 1 ]; then
 trap "/bin/logger settings:TIMEOUT, stop download; /bin/kill $CHECK_PID; /bin/kill $MY_PID " 1
else
 trap "/bin/kill $CHECK_PID ; /bin/kill $MY_PID " 1
fi

wait $CHECK_PID
#kill the controlling process, 
#we can do that as it must still exist, becuase on timeout we would have been killed already by the trap above via control
kill $CONTROL_PID

