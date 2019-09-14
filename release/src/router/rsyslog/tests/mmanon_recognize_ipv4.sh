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
	action(type="mmanon" mode="zero" ipv4.bits="32")
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
}'

startup
tcpflood -m1 -M "\"<129>Mar 10 01:00:00 172.20.245.8 tag: asdfghjk
<129>Mar 10 01:00:00 172.20.245.8 tag: before 172.9.6.4
<129>Mar 10 01:00:00 172.20.245.8 tag: 75.123.123.0 after
<129>Mar 10 01:00:00 172.20.245.8 tag: before 181.23.1.4 after
<129>Mar 10 01:00:00 172.20.245.8 tag: nothingnothingnothing
<129>Mar 10 01:00:00 172.20.245.8 tag: before 181.23.1.4 after 172.1.3.4
<129>Mar 10 01:00:00 172.20.245.8 tag: 1.1.1.8
<129>Mar 10 01:00:00 172.20.245.8 tag: 1.1.1.8
<129>Mar 10 01:00:00 172.20.245.8 tag: 1.1.1.9
<129>Mar 10 01:00:00 172.20.245.8 tag: 0.0.0.0
<129>Mar 10 01:00:00 172.20.245.8 tag: 1.2.3.4.5.6.7.8.76
<129>Mar 10 01:00:00 172.20.245.8 tag: 172.0.234.255
<129>Mar 10 01:00:00 172.20.245.8 tag: 1.0.0.0
<129>Mar 10 01:00:00 172.20.245.8 tag: 1.225.225.225
<129>Mar 10 01:00:00 172.20.245.8 tag: 172.0.234.255
<129>Mar 10 01:00:00 172.20.245.8 tag: 3.4.5.6
<129>Mar 10 01:00:00 172.20.245.8 tag: 256.0.0.0
<129>Mar 10 01:00:00 172.20.245.8 tag: 1....1....1....8
<129>Mar 10 01:00:00 172.20.245.8 tag: 1..1..1..8
<129>Mar 10 01:00:00 172.20.245.8 tag: 1..1.1.8
<129>Mar 10 01:00:00 172.20.245.8 tag: 1.1..1.8
<129>Mar 10 01:00:00 172.20.245.8 tag: 1.1.1..8
<129>Mar 10 01:00:00 172.20.245.8 tag: 1111.1.1.8.1
<129>Mar 10 01:00:00 172.20.245.8 tag: 111.1.1.8.1
<129>Mar 10 01:00:00 172.20.245.8 tag: 111.1.1.8.
<129>Mar 10 01:00:00 172.20.245.8 tag: textnoblank1.1.1.9stillnoblank\""

shutdown_when_empty
wait_shutdown
echo ' asdfghjk
 before 0.0.0.0
 0.0.0.0 after
 before 0.0.0.0 after
 nothingnothingnothing
 before 0.0.0.0 after 0.0.0.0
 0.0.0.0
 0.0.0.0
 0.0.0.0
 0.0.0.0
 0.0.0.0.0.0.0.0.76
 0.0.0.0
 0.0.0.0
 0.0.0.0
 0.0.0.0
 0.0.0.0
 20.0.0.0
 1....1....1....8
 1..1..1..8
 1..1.1.8
 1.1..1.8
 1.1.1..8
 10.0.0.0.1
 0.0.0.0.1
 0.0.0.0.
 textnoblank0.0.0.0stillnoblank' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid response generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit  1
fi;

exit_test
