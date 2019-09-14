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
	action(type="mmanon" ipv4.bits="33" ipv4.mode="simple" ipv4.replacechar="*")
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
}
action(type="omfile" file=`echo $RSYSLOG2_OUT_LOG`)'

startup
tcpflood -m1 -M "\"<129>Mar 10 01:00:00 172.20.245.8 tag: asdfghjk
<129>Mar 10 01:00:00 172.20.245.8 tag: before 172.9.6.4
<129>Mar 10 01:00:00 172.20.245.8 tag: 75.123.123.0 after
<129>Mar 10 01:00:00 172.20.245.8 tag: before 181.23.1.4 after
<129>Mar 10 01:00:00 172.20.245.8 tag: nothingnothingnothing
<129>Mar 10 01:00:00 172.20.245.8 tag: before 181.23.1.4 after 172.1.3.45
<129>Mar 10 01:00:00 172.20.245.8 tag: 1.1.1.8
<129>Mar 10 01:00:00 172.20.245.8 tag: 1.12.1.8
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
<129>Mar 10 01:00:00 172.20.245.8 tag: textnoblank1.1.31.9stillnoblank\""

shutdown_when_empty
wait_shutdown
echo ' asdfghjk
 before ***.*.*.*
 **.***.***.* after
 before ***.**.*.* after
 nothingnothingnothing
 before ***.**.*.* after ***.*.*.**
 *.*.*.*
 *.**.*.*
 *.*.*.*
 *.*.*.*
 *.*.*.*.*.*.*.*.76
 ***.*.***.***
 *.*.*.*
 *.***.***.***
 ***.*.***.***
 *.*.*.*
 ***.*.*.*
 1....1....1....8
 1..1..1..8
 1..1.1.8
 1.1..1.8
 1.1.1..8
 ****.*.*.*.1
 ***.*.*.*.1
 ***.*.*.*.
 textnoblank*.*.**.*stillnoblank' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid response generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit  1
fi;

grep 'invalid number of ipv4.bits (33), corrected to 32' ${RSYSLOG2_OUT_LOG} > /dev/null
if [ $? -ne 0 ]; then
  echo "invalid response generated, ${RSYSLOG2_OUT_LOG} is:"
  cat ${RSYSLOG2_OUT_LOG}
  error_exit  1
fi;

exit_test
