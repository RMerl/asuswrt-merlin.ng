#!/bin/bash
# added 2016-12-11 by rgerhards
# This file is part of the rsyslog project, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="outfmt" type="list") {
	property(name="msg" field.delimiter="58" field.number="2")
	constant(value="\n")
}

ruleset(name="rs") {
	action(type="omfile" file="./'"${RSYSLOG2_OUT_LOG}"'" template="outfmt")
}

if $msg contains "msgnum" then
	call_indirect "does-not-exist";
else
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`)
'
startup
injectmsg  0 5
shutdown_when_empty
wait_shutdown 
grep "error.*does-not-exist"  $RSYSLOG_OUT_LOG > /dev/null
if [ $? -ne 0 ]; then
	echo
	echo "FAIL: expected error message not found.  $RSYSLOG_OUT_LOG is:"
	cat $RSYSLOG_OUT_LOG
	error_exit 1
fi
exit_test
