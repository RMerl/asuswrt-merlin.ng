#!/bin/sh
export PATH=/tmp/sbin:/tmp/bin:/bin:/usr/bin:/sbin:/usr/sbin

UPGRADE_CHECK_ENABLED=`/usr/sbin/nvram get upgrade_check`
if [ $UPGRADE_CHECK_ENABLED -gt 0 ]; then
    
    # verbosity
    VERBOSITY=`/usr/sbin/nvram get NC_Verbosity`
    
    #use the same time out here
    CHECK_TIMEOUT=`/usr/sbin/nvram get NC_check_timeout`
    
    UPGRADE_CHECK_URL=`/usr/sbin/nvram get hsz_upgrade_url`
    
    if [ $VERBOSITY -gt 4 ]; then /bin/logger "check firmware upgrade: call with timeout: $CHECK_TIMEOUT"; fi
    
    MY_PID=$$
    
    (/bin/sleep $CHECK_TIMEOUT; /bin/kill -1 $MY_PID) &
    CONTROL_PID=$!
    
    /bin/wget $UPGRADE_CHECK_URL -O /tmp/hsz_firmware & 
    CHECK_PID=$!
    
    if [ $VERBOSITY -gt 4 ]; then
     trap "/bin/logger check firmware upgrade: TIMEOUT, stop check; /bin/kill $CHECK_PID; /bin/kill $MY_PID " 1
    else
     trap "/bin/kill $CHECK_PID ; /bin/kill $MY_PID " 1
    fi
    
    wait $CHECK_PID
    #kill the controlling process, 
    #we can do that as it must still exist, becuase on timeout we would have been killed already by the trap above via control
    kill $CONTROL_PID

fi
