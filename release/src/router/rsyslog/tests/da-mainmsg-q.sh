#!/bin/bash
# Test for DA mode on the main message queue
# This test checks if DA mode operates correctly. To do so,
# it uses a small in-memory queue size, so that DA mode is initiated
# rather soon, and disk spooling used. There is some uncertainty (based
# on machine speeds), but in general the test should work rather well.
# We add a few messages after the initial run, just so that we can
# check everything recovers from DA mode correctly.
# added 2009-04-22 by Rgerhards
# This file is part of the rsyslog project, released  under GPLv3
echo ===============================================================================
echo "[da-mainmsg-q.sh]: testing main message queue in DA mode (going to disk)"
. $srcdir/diag.sh init
generate_conf
add_conf '
$ModLoad ../plugins/imtcp/.libs/imtcp
$MainMsgQueueTimeoutShutdown 10000
$InputTCPServerRun '$TCPFLOOD_PORT'

# set spool locations and switch queue to disk assisted mode
$WorkDirectory '$RSYSLOG_DYNNAME'.spool
$MainMsgQueueSize 200 # this *should* trigger moving on to DA mode...
# note: we must set QueueSize sufficiently high, so that 70% (light delay mark)
# is high enough above HighWatermark!
$MainMsgQueueHighWatermark 80
$MainMsgQueueLowWatermark 40
$MainMsgQueueFilename mainq
$MainMsgQueueType linkedlist

$template outfmt,"%msg:F,58:2%\n"
template(name="dynfile" type="string" string=`echo $RSYSLOG_OUT_LOG`) # trick to use relative path names!
:msg, contains, "msgnum:" ?dynfile;outfmt
'
startup

# part1: send first 50 messages (in memory, only)
#tcpflood 127.0.0.1 '$TCPFLOOD_PORT' 1 50
injectmsg 0 50
wait_queueempty # let queue drain for this test case

# part 2: send bunch of messages. This should trigger DA mode
#injectmsg 50 20000
injectmsg 50 2000
ls -l ${RSYSLOG_DYNNAME}.spool	 # for manual review

# send another handful
injectmsg 2050 50
#sleep 1 # we need this so that rsyslogd can receive all outstanding messages

# clean up and check test result
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown
seq_check  0 2099
exit_test
