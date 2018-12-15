#!/bin/bash
# add 2016-11-22 by Jan Gerhards, released under ASL 2.0

. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="outfmt" type="string" string="%msg%\n")

module(load="../plugins/mmanon/.libs/mmanon")
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'" ruleset="testing")

ruleset(name="testing") {
	action(type="mmanon" ipv4.enable="on" ipv4.mode="zero" ipv4.bits="32" ipv6.bits="128" ipv6.anonmode="zero" ipv6.enable="on")
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
}'

startup
tcpflood -m1 -M "\"<129>Mar 10 01:00:00 172.20.245.8 tag: FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF
<129>Mar 10 01:00:00 172.20.245.8 tag: 1.1.1.8 space 61:34:ad::7:F
<129>Mar 10 01:00:00 172.20.245.8 tag: 111.1.1.8
<129>Mar 10 01:00:00 172.20.245.8 tag: abf:3:002::500F:ce 1.1.1.9\""

shutdown_when_empty
wait_shutdown
echo ' 0:0:0:0:0:0:0:0
 0.0.0.0 space 0:0:0:0:0:0:0:0
 0.0.0.0
 0:0:0:0:0:0:0:0 0.0.0.0' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid response generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit  1
fi;

exit_test
