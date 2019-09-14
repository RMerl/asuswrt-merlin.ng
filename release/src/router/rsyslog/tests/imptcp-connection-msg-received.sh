#!/bin/bash
# addd 2017-03-31 by Pascal Withopf, released under ASL 2.0

. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imptcp/.libs/imptcp")
input(type="imptcp" port="'$TCPFLOOD_PORT'" notifyonconnectionclose="on" notifyonconnectionopen="on")

:msg, contains, "msgnum:" {
	action(type="omfile" file=`echo $RSYSLOG2_OUT_LOG`)
}

action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`)

'
startup
tcpflood -m1 -M"\"<129>Mar 10 01:00:00 172.20.245.8 tag: msgnum:1\""
shutdown_when_empty
wait_shutdown

grep "imptcp: connection established"  $RSYSLOG_OUT_LOG > /dev/null
if [ $? -ne 0 ]; then
	echo
	echo "FAIL: expected error message not found.  $RSYSLOG_OUT_LOG is:"
	cat $RSYSLOG_OUT_LOG
	error_exit 1
fi

grep "imptcp: session on socket.* closed"  $RSYSLOG_OUT_LOG > /dev/null
if [ $? -ne 0 ]; then
	echo
	echo "FAIL: expected error message not found.  $RSYSLOG_OUT_LOG is:"
	cat $RSYSLOG_OUT_LOG
	error_exit 1
fi

exit_test
