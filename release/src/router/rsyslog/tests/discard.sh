#!/bin/bash
# Test for discard functionality
# This test checks if discard works. It is not a perfect test but
# will find at least segfaults and obviously not discarded messages.
# added 2009-07-30 by Rgerhards
# This file is part of the rsyslog project, released  under GPLv3
# uncomment for debugging support:
echo ===============================================================================
echo \[discard.sh\]: testing discard functionality
. $srcdir/diag.sh init
generate_conf
add_conf '
$ModLoad ../plugins/imtcp/.libs/imtcp
$MainMsgQueueTimeoutShutdown 10000
$InputTCPServerRun '$TCPFLOOD_PORT'

:msg, contains, "00000001" ~

$template outfmt,"%msg:F,58:2%\n"
template(name="dynfile" type="string" string=`echo $RSYSLOG_OUT_LOG`) # trick to use relative path names!
:msg, contains, "msgnum:" ?dynfile;outfmt
'
startup
# 20000 messages should be enough - the disk test is slow enough ;)
sleep 4
tcpflood -m10 -i1
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown
seq_check 2 10
exit_test
