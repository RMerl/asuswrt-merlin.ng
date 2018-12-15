#!/bin/bash
# added 2014-10-31 by singh.janmejay
# This file is part of the rsyslog project, released under ASL 2.0
echo ===============================================================================
echo \[mmnormalize_variable.sh\]: basic test for mmnormalize module variable-support
. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="outfmt" type="string" string="h:%$!hr% m:%$!min% s:%$!sec%\n")

module(load="../plugins/mmnormalize/.libs/mmnormalize")
module(load="../plugins/imptcp/.libs/imptcp")
input(type="imptcp" port="'$TCPFLOOD_PORT'")

template(name="time_fragment" type="list") {
  property(name="msg" regex.Expression="[0-9]{2}:[0-9]{2}:[0-9]{2} [A-Z]+" regex.Type="ERE" regex.Match="0")
}

set $.time_frag = exec_template("time_fragment");

action(type="mmnormalize" rulebase=`echo $srcdir/testsuites/mmnormalize_variable.rulebase` variable="$.time_frag")
action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup
tcpflood -m 1 -I $srcdir/testsuites/date_time_msg
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown 
content_check  "h:13 m:20 s:18"
exit_test
