#!/bin/bash
# Test if rsyslog survives sending truely random data to it...
#
# added 2010-04-01 by Rgerhards
# This file is part of the rsyslog project, released  under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
# The random data will generate TCP framing error messages. We will
# not clutter the test output with them. So we disable error messages
# to stderr.
$ErrorMessagesToStderr off

$ModLoad ../plugins/imtcp/.libs/imtcp
$MainMsgQueueTimeoutShutdown 10000
$InputTCPServerRun '$TCPFLOOD_PORT'

$template outfmt,"%rawmsg%\n"
template(name="dynfile" type="string" string=`echo $RSYSLOG_OUT_LOG`) # trick to use relative path names!
*.* /dev/null
'
startup
# generate random data
./randomgen -f $RSYSLOG_DYNNAME.random.data -s 100000
ls -l $RSYSLOG_DYNNAME.random.data
tcpflood -B -I $RSYSLOG_DYNNAME.random.data -c5 -C10
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown       # and wait for it to terminate
# we do not check anything yet, the point is if rsyslog survived ;)
# TODO: check for exit message, but we'll notice an abort anyhow, so not that important
exit_test
