#!/bin/bash
# Test for a startup with a bad qi file. This tests simply tests
# if the rsyslog engine survives (we had segfaults in this situation
# in the past).
# added 2009-10-21 by RGerhards
# This file is part of the rsyslog project, released  under GPLv3
# uncomment for debugging support:
echo ===============================================================================
echo \[badqi.sh\]: test startup with invalid .qi file
. $srcdir/diag.sh init
generate_conf
add_conf '
$ModLoad ../plugins/imtcp/.libs/imtcp
$MainMsgQueueTimeoutShutdown 10000
$InputTCPServerRun '$TCPFLOOD_PORT'

$template outfmt,"%msg:F,58:2%\n"
template(name="dynfile" type="string" string=`echo $RSYSLOG_OUT_LOG`) # trick to use relative path names!
# instruct to use bad .qi file
$WorkDirectory bad_qi
$ActionQueueType LinkedList
$ActionQueueFileName dbq
:msg, contains, "msgnum:" ?dynfile;outfmt
'
startup
# we just inject a handful of messages so that we have something to wait for...
tcpflood -m20
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown  # wait for process to terminate
seq_check 0 19
exit_test
