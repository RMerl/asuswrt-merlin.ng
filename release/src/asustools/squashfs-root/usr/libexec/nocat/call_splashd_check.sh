#!/bin/sh
export PATH=/tmp/sbin:/tmp/bin:/bin:/usr/bin:/sbin:/usr/sbin

SPLASHD_ENABLED=`/usr/sbin/nvram get NC_enable`
if [ $SPLASHD_ENABLED -gt 0 ]; then
    
    # verbosity
    VERBOSITY=`/usr/sbin/nvram get NC_Verbosity`
    
    CHECK_TIMEOUT=`/usr/sbin/nvram get NC_check_timeout`
    
    if [ $VERBOSITY -gt 4 ]; then /bin/logger "check splashd: call with timeout: $CHECK_TIMEOUT"; fi
    
    MY_PID=$$
    
    (/bin/sleep $CHECK_TIMEOUT; /bin/kill -1 $MY_PID) &
    CONTROL_PID=$!
    
    /usr/libexec/nocat/check_splashd.sh & 
    CHECK_PID=$!
    
    if [ $VERBOSITY -gt 4 ]; then
     trap "/bin/logger check splashd: TIMEOUT, stop check and splashd ; /bin/kill $CHECK_PID ; /bin/killall splashd; /bin/sleep 15; /bin/logger check splashd: restart splashd ; /usr/sbin/splashd >> /tmp/services.out 2>&1 & ; /bin/kill $MY_PID " 1
    else
     trap "/bin/kill $CHECK_PID ; /bin/killall splashd; /bin/sleep 15; /usr/sbin/splashd >> /tmp/services.out 2>&1 & ; /bin/kill $MY_PID " 1
    fi
    
    wait $CHECK_PID
    #kill the controlling process, 
    #we can do that as it must still exist, becuase on timeout we would have been killed already by the trap above via control
    kill $CONTROL_PID

fi
