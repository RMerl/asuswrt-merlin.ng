#!/bin/bash
# add 2017-12-01 by Rainer Gerhards, released under ASL 2.0
. $srcdir/diag.sh init
rsyslog_testbench_test_url_access http://testbench.rsyslog.com/testbench/echo-get.php
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
module(load="../plugins/fmhttp/.libs/fmhttp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

# for debugging the test itself:
#template(name="outfmt" type="string" string="%$!%:  :%$.%:  %rawmsg%\n")
template(name="outfmt" type="string" string="%$!%\n")

if $msg contains "msgnum:" then {
	set $.url = "http://testbench.rsyslog.com/testbench/echo-get.php?content=" & ltrim($msg);
	set $!reply = http_request($.url);
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
}
'
startup_vg
tcpflood -m10
shutdown_when_empty
wait_shutdown_vg
check_exit_vg
echo '{ "reply": "msgnum:00000000:" }
{ "reply": "msgnum:00000001:" }
{ "reply": "msgnum:00000002:" }
{ "reply": "msgnum:00000003:" }
{ "reply": "msgnum:00000004:" }
{ "reply": "msgnum:00000005:" }
{ "reply": "msgnum:00000006:" }
{ "reply": "msgnum:00000007:" }
{ "reply": "msgnum:00000008:" }
{ "reply": "msgnum:00000009:" }' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid function output detected, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit 1
fi;
exit_test

