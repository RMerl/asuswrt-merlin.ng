#!/bin/bash
# test many concurrent tcp connections
echo ====================================================================================
echo TEST: \[manyptcp.sh\]: test imptcp with large connection count
. $srcdir/diag.sh init
generate_conf
add_conf '
$ModLoad ../plugins/imptcp/.libs/imptcp
$MainMsgQueueTimeoutShutdown 10000
$MaxOpenFiles 2000
$InputPTCPServerRun '$TCPFLOOD_PORT'

$template outfmt,"%msg:F,58:2%\n"
template(name="dynfile" type="string" string=`echo $RSYSLOG_OUT_LOG`) # trick to use relative path names!
:msg, contains, "msgnum:" ?dynfile;outfmt
'
startup
# the config file specifies exactly 1100 connections
tcpflood -c1000 -m40000
# the sleep below is needed to prevent too-early termination of the tcp listener
sleep 1
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown	# we need to wait until rsyslogd is finished!
seq_check 0 39999
exit_test
