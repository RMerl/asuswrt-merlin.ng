#!/bin/bash
# addd 2017-03-01 by RGerhards, released under ASL 2.0

. $srcdir/diag.sh init
generate_conf
add_conf '
$MaxMessageSize 128
global(processInternalMessages="on"
	oversizemsg.input.mode="accept")
module(load="../plugins/imptcp/.libs/imptcp")
input(type="imptcp" port="'$TCPFLOOD_PORT'" ruleset="ruleset1")

template(name="templ1" type="string" string="%rawmsg%\n")
ruleset(name="ruleset1") {
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="templ1")
}

'
startup
tcpflood -m2 -M "\"41 <120> 2011-03-01T11:22:12Z host msgnum:1\""
tcpflood -m1 -M "\"214000000000 <120> 2011-03-01T11:22:12Z host msgnum:1\""
tcpflood -m1 -M "\"41 <120> 2011-03-01T11:22:12Z host msgnum:1\""
tcpflood -m1 -M "\"214000000000 <120> 2011-03-01T11:22:12Z host msgnum:1\""
tcpflood -m1 -M "\"41 <120> 2011-03-01T11:22:12Z host msgnum:1\""
tcpflood -m1 -M "\"2000000010 <120> 2011-03-01T11:22:12Z host msgnum:1\""
tcpflood -m1 -M "\"4000000000 <120> 2011-03-01T11:22:12Z host msgnum:1\""
tcpflood -m1 -M "\"0 <120> 2011-03-01T11:22:12Z host msgnum:1\""
shutdown_when_empty
wait_shutdown

echo '<120> 2011-03-01T11:22:12Z host msgnum:1
<120> 2011-03-01T11:22:12Z host msgnum:1
214000000000<120> 2011-03-01T11:22:12Z host msgnum:1
<120> 2011-03-01T11:22:12Z host msgnum:1
214000000000<120> 2011-03-01T11:22:12Z host msgnum:1
<120> 2011-03-01T11:22:12Z host msgnum:1
2000000010<120> 2011-03-01T11:22:12Z host msgnum:1
4000000000<120> 2011-03-01T11:22:12Z host msgnum:1
<120> 2011-03-01T11:22:12Z host msgnum:1' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid response generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit  1
fi;

exit_test
