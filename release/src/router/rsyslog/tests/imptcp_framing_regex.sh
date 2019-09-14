#!/bin/bash
# This file is part of the rsyslog project, released  under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
$EscapeControlCharactersOnReceive off
module(load="../plugins/imptcp/.libs/imptcp")
input(type="imptcp" port="'$TCPFLOOD_PORT'" ruleset="remote"
	framing.delimiter.regex="^<[0-9]{2}>(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)")

template(name="outfmt" type="string" string="NEWMSG: %rawmsg%\n")
ruleset(name="remote") {
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
}
'
startup
tcpflood -B -I ${srcdir}/testsuites/imptcp_framing_regex.testdata
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown       # and wait for it to terminate
echo 'NEWMSG: <33>Mar  1 01:00:00 172.20.245.8 tag test1
NEWMSG: <33>Mar  1 01:00:00 172.20.245.8 tag test xml-ish
<test/>
NEWMSG: <33>Mar  1 01:00:00 172.20.245.8 tag test2
NEWMSG: <33>Mar  1 01:00:00 172.20.245.8 tag multi
line1
NEWMSG: <33>Mar  1 01:00:00 172.20.245.8 tag multi
 l
 i
 n

e2
NEWMSG: <33>Mar  1 01:00:00 172.20.245.8 tag test3
NEWMSG: <33>Mar  1 01:00:00 172.20.245.8 tag multi
line3
NEWMSG: <33>Mar  1 01:00:00 172.20.245.8 tag test4' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid response generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit  1
fi;
exit_test
