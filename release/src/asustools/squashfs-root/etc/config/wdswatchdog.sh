#!/bin/sh

WDS_WATCHDOG_INTERVAL_SEC=$(nvram get wds_watchdog_interval_sec)
WDS_WATCHDOG_IPS=$(nvram get wds_watchdog_ips)

while sleep $WDS_WATCHDOG_INTERVAL_SEC
do
  for ip in $WDS_WATCHDOG_IPS
  do
    if ping -c 1 $ip > /tmp/null
    then
      echo "$ip ok"
    else
      echo "$ip dropped one"
      sleep 10
      if ! ping -c 1 $ip > /tmp/null
      then
        echo "$ip dropped two"
        sleep 10
        if ! ping -c 1 $ip > /tmp/null
        then
    	    echo "$ip dropped three, Restarting Router"
    	    /usr/sbin/nvram commit
    	    /sbin/reboot &
        fi
      fi
    fi
  done
done 2>&1
