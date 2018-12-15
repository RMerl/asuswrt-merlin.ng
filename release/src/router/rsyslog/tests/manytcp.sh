#!/bin/bash
# test many concurrent tcp connections

uname
if [ `uname` = "FreeBSD" ] ; then
   echo "This test currently does not work on FreeBSD."
   exit 77
fi

echo \[manytcp.sh\]: test concurrent tcp connections

uname
if [ `uname` = "SunOS" ] ; then
   echo "Solaris: FIX ME"
   exit 77
fi

. $srcdir/diag.sh init
generate_conf
add_conf '
$ModLoad ../plugins/imtcp/.libs/imtcp
$MainMsgQueueTimeoutShutdown 10000
$MaxOpenFiles 2000
$InputTCPMaxSessions 1100
$InputTCPServerRun '$TCPFLOOD_PORT'

$template outfmt,"%msg:F,58:2%\n"
template(name="dynfile" type="string" string=`echo $RSYSLOG_OUT_LOG`) # trick to use relative path names!
:msg, contains, "msgnum:" ?dynfile;outfmt
'
startup
# the config file specifies exactly 1100 connections
tcpflood -c-1100 -m40000
# the sleep below is needed to prevent too-early termination of the tcp listener
sleep 1
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown	# we need to wait until rsyslogd is finished!
seq_check 0 39999
exit_test
