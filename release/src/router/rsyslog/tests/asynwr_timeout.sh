#!/bin/bash
# This test writes to the output buffers, let's the output
# write timeout (and write data) and then continue. The conf file
# has a 2 second timeout, so we wait 4 seconds to be on the save side.
#
# added 2010-03-09 by Rgerhards
# This file is part of the rsyslog project, released  under ASL 2.0
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
$OMFileIOBufferSize 10k
$OMFileAsyncWriting on
:msg, contains, "msgnum:" ?dynfile;outfmt
'
startup
# send 35555 messages, make sure file size is not a multiple of
# 10K, the buffer size!
tcpflood -m 35555
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown       # and wait for it to terminate
seq_check 0 35554
exit_test
