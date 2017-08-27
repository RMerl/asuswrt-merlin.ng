#!/bin/sh
export PATH=/tmp/sbin:/tmp/bin:/bin:/usr/bin:/sbin:/usr/sbin

SPLASHD_ENABLED=`/usr/sbin/nvram get NC_enable`
if [ $SPLASHD_ENABLED -gt 0 ]; then
    
    # verbosity
    VERBOSITY=`/usr/sbin/nvram get NC_Verbosity`
    
    # prevent two checks at the same time
    CHECK_SPLASHD_COUNT=`/bin/ps -elf | /bin/grep -c check_splashd`
    CHECK_SPLASHD_TXT=`/bin/ps -elf | /bin/grep check_splashd`
    
    if [ $CHECK_SPLASHD_COUNT -gt 4 ]; then
        if [ $VERBOSITY -gt 4 ]; then /bin/logger "check splashd: runs already, count:$CHECK_SPLASHD_COUNT, do nothing"; fi
    else
        # IPTABLES 
        # must have at least NoCat and NoCat_Capture chain
        # nat table should perfrom forward
        if [ $VERBOSITY -gt 4 ]; then /bin/logger "check splashd: check iptables"; fi
        
        TEST_NOCAT_IPTABLES_RESULT=`/usr/sbin/iptables -t filter -n -L NoCat 2>&1 | /bin/grep '^iptables' `
        TEST_FORWARD_NOCAT_IPTABLES_RESULT=`/usr/sbin/iptables -t filter -n -L FORWARD | /bin/grep -c 'NoCat ' `
        TEST_NOCAT_CAPTURE_IPTABLES_RESULT=`/usr/sbin/iptables -t nat -n -L NoCat_Capture 2>&1 | /bin/grep '^iptables' `
        TEST_NAT_NOCAT_CAPTURE_IPTABLES_RESULT=`/usr/sbin/iptables -t nat -n -L PREROUTING | /bin/grep -c 'NoCat_Capture ' `
        
        if [ -z "$TEST_NOCAT_IPTABLES_RESULT" -a $TEST_FORWARD_NOCAT_IPTABLES_RESULT -eq 1 -a -z "$TEST_NOCAT_CAPTURE_IPTABLES_RESULT" -a $TEST_NAT_NOCAT_CAPTURE_IPTABLES_RESULT -eq 1 ]; then
            
            if [ $VERBOSITY -gt 0 ]; then /bin/logger "check splashd: iptables ok"; fi
            
            # check redirect to HTTP welcome page
            GATEWAY_IP=`/usr/sbin/nvram get lan_ipaddr`
            GATEWAY_PORT=`/usr/sbin/nvram get NC_GatewayPort`
            
            if [ $VERBOSITY -gt 4 ]; then /bin/logger "check splashd: redirect on $GATEWAY_IP:$GATEWAY_PORT"; fi
            
            TEST_RESULT=`/bin/wget http://$GATEWAY_IP:$GATEWAY_PORT/test -O- 2>&1 | /bin/sed 's/^wget: not an http/ /' | /bin/sed 's/^wget: too many redirections/ /' | /bin/grep '^wget' `
            
            if [ -z "$TEST_RESULT" ]; then
              if [ $VERBOSITY -gt 0 ]; then /bin/logger "check splashd: redirect ok"; fi
            else
              # redirect error
              if [ $VERBOSITY -gt 0 ]; then /bin/logger "check splashd: redirect error $TEST_RESULT, stop splashd"; fi
              /bin/killall splashd
              /bin/sleep 10
              if [ $VERBOSITY -gt 0 ]; then /bin/logger "check splashd: restart splashd"; fi
              /usr/sbin/splashd >> /tmp/services.out 2>&1 &
            fi
        else
          # iptables error
          if [ $VERBOSITY -gt 0 ]; then /bin/logger "check splashd: iptables error: $TEST_NOCAT_IPTABLES_RESULT,$TEST_FORWARD_NOCAT_IPTABLES_RESULT,$TEST_NOCAT_CAPTURE_IPTABLES_RESULT,$TEST_NAT_NOCAT_CAPTURE_IPTABLES_RESULT, stop splashd"; fi
          /bin/killall splashd
          /bin/sleep 10
          if [ $VERBOSITY -gt 0 ]; then /bin/logger "check splashd: restart splashd"; fi
          /usr/sbin/splashd >> /tmp/services.out 2>&1 &
        fi
    fi
fi
