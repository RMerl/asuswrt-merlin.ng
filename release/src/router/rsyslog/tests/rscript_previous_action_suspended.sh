#!/bin/bash
# Added 2017-12-09 by Rainer Gerhards, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
module(load="../plugins/omtesting/.libs/omtesting")
input(type="imtcp" port="'$TCPFLOOD_PORT'")
template(name="outfmt" type="string" string="%msg:F,58:2%\n")

ruleset(name="output_writer") {
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
}

:msg, contains, "msgnum:" {
	:omtesting:fail 2 0
	if previous_action_suspended() then
		call output_writer
}
'

startup
injectmsg 0 10
shutdown_when_empty
wait_shutdown
seq_check 1 9
exit_test
