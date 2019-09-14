#!/bin/bash
# tests 'config.enabled="on"' -- default value is implicitely check
# in all testbench tests and does not need its individual test
# (actually it is here tested via template() and action() as well...
# added 2018-01-22 by Rainer Gerhards; Released under ASL 2.0
. $srcdir/diag.sh init
export DO_STOP=on
generate_conf
add_conf '
template(name="outfmt" type="string" string="%msg:F,58:2%\n")

if $msg contains "msgnum:" then {
	if $msg contains "msgnum:00000000" then {
		include(text="stop" config.enabled=`echo $DO_STOP`)
	}
	action(type="omfile" template="outfmt" file=`echo $RSYSLOG_OUT_LOG`)
}
'
startup
injectmsg 0 10
shutdown_when_empty
wait_shutdown
seq_check 1 9
exit_test
