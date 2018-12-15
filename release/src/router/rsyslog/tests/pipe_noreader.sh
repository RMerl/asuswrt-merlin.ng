#!/bin/bash
# This is test driver for a pipe that has no reader. This mimics a usual
# real-world scenario, the /dev/xconsole pipe. Some versions of rsyslog
# were known to hang or loop on this pipe, thus we added this scenario
# as a permanent testcase. For some details, please see bug tracker
# http://bugzilla.adiscon.com/show_bug.cgi?id=186
#
# IMPORTANT: we do NOT check any result message set. The whole point in
# this test is to verify that we do NOT run into an eternal loop. As such,
# the test is "PASS", if rsyslogd terminates. If it does not terminate, we
# obviously do not cause "FAIL", but processing will hang, which should be
# a good-enough indication of failure.
#
# added 2010-04-26 by Rgerhards
# This file is part of the rsyslog project, released  under ASL 2.0
# uncomment for debugging support:
#export RSYSLOG_DEBUG="debug nostdout noprintmutexaction"
#export RSYSLOG_DEBUGLOG="log"
. $srcdir/diag.sh init
generate_conf
add_conf '
$ModLoad ../plugins/imtcp/.libs/imtcp
$MainMsgQueueTimeoutShutdown 10000
$InputTCPServerRun '$TCPFLOOD_PORT'

$template outfmt,"%msg:F,58:2%\n"
:msg, contains, "msgnum:" |./'$RSYSLOG_DYNNAME'.pipe
'
mkfifo ./$RSYSLOG_DYNNAME.pipe
startup
# we need to emit ~ 128K of data according to bug report
tcpflood -m1000 -d500
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown       # and wait for it to terminate
# NO need to check seqno -- see header comment
echo we did not loop, so the test is sucessfull
exit_test
