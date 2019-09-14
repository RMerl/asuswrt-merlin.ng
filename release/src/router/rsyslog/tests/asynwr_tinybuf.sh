#!/bin/bash
# This tests async writing with a very small output buffer (1 byte!),
# so it stresses output buffer handling. This also means operations will
# be somewhat slow, so we send only a small amounts of data.
#
# added 2010-03-09 by Rgerhards
#
# This file is part of the rsyslog project, released  under GPLv3
echo ===============================================================================
echo TEST: \[asynwr_tinybuf.sh\]: test async file writing with 1-byte buffer
. $srcdir/diag.sh init
generate_conf
add_conf '
$ModLoad ../plugins/imtcp/.libs/imtcp
$MainMsgQueueTimeoutShutdown 10000
$InputTCPServerRun '$TCPFLOOD_PORT'

$template outfmt,"%msg:F,58:2%\n"
template(name="dynfile" type="string" string=`echo $RSYSLOG_OUT_LOG`) # trick to use relative path names!
$OMFileFlushOnTXEnd off
$OMFileFlushInterval 2
$OMFileIOBufferSize 1
$OMFileAsyncWriting on
:msg, contains, "msgnum:" ?dynfile;outfmt
'
# uncomment for debugging support:
#export RSYSLOG_DEBUG="debug nostdout noprintmutexaction"
#export RSYSLOG_DEBUGLOG="log"
startup
# send 1000 messages, fairly enough to trigger problems
tcpflood -m1000
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown # shut down rsyslogd when done processing messages
seq_check 0 999
exit_test
