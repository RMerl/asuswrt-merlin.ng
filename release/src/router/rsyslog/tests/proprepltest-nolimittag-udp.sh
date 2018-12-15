#!/bin/bash
# add 2018-06-27 by Pascal Withopf, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imudp/.libs/imudp")
input(type="imudp" port="'$TCPFLOOD_PORT'")

template(name="outfmt" type="string" string="+%syslogtag%+\n")

:pri, contains, "167" action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`
				   template="outfmt")


'
startup
tcpflood -m1 -T "udp" -M "\"<167>Mar  6 16:57:54 172.20.245.8 TAG: Rest of message...\""
tcpflood -m1 -T "udp" -M "\"<167>Mar  6 16:57:54 172.20.245.8 0 Rest of message...\""
tcpflood -m1 -T "udp" -M "\"<167>Mar  6 16:57:54 172.20.245.8 01234567890123456789012345678901 Rest of message...\""
tcpflood -m1 -T "udp" -M "\"<167>Mar  6 16:57:54 172.20.245.8 01234567890123456789012345678901-toolong Rest of message...\""
shutdown_when_empty
wait_shutdown

echo '+TAG:+
+0+
+01234567890123456789012345678901+
+01234567890123456789012345678901-toolong+' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid response generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit  1
fi;

exit_test
