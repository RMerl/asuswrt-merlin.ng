#!/bin/bash
# addd 2018-04-03 by RGerhards, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
action(type="omfile" file=" ")
action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`)
'
startup
shutdown_when_empty
wait_shutdown

grep "only of whitespace"  $RSYSLOG_OUT_LOG > /dev/null
if [ $? -ne 0 ]; then
	echo
	echo "FAIL: expected error message not found.  $RSYSLOG_OUT_LOG is:"
	cat $RSYSLOG_OUT_LOG
	error_exit 1
fi

exit_test
