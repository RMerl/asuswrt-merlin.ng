#!/bin/bash
# Test for multiple ports in imtcp
# This test checks if multiple tcp listener ports are correctly
# handled by imtcp
# added 2009-05-22 by Rgerhards
# This file is part of the rsyslog project, released under ASL 2.0
. $srcdir/diag.sh init
export TCPFLOOD_PORT2="$(get_free_port)"
export TCPFLOOD_PORT3="$(get_free_port)"
generate_conf
add_conf '
$ModLoad ../plugins/imtcp/.libs/imtcp
$MainMsgQueueTimeoutShutdown 10000
$InputTCPServerRun '$TCPFLOOD_PORT'
$InputTCPServerRun '$TCPFLOOD_PORT2'
$InputTCPServerRun '$TCPFLOOD_PORT3'

$template outfmt,"%msg:F,58:2%\n"
:msg, contains, "msgnum:" action(type="omfile" file="'$RSYSLOG_OUT_LOG'" template="outfmt")
'
startup
tcpflood -p'$TCPFLOOD_PORT' -m10000
tcpflood -p'$TCPFLOOD_PORT2' -i10000 -m10000
tcpflood -p'$TCPFLOOD_PORT3' -i20000 -m10000
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown
seq_check 0 29999
exit_test
