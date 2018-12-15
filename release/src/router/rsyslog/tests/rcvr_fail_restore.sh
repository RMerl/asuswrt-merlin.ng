#!/bin/bash
# Copyright (C) 2011 by Rainer Gerhards
# This file is part of the rsyslog project, released  under GPLv3
. $srcdir/diag.sh init

uname
if [ `uname` = "FreeBSD" ] ; then
   echo "This test currently does not work on FreeBSD."
   exit 77
fi

#
# STEP1: start both instances and send 1000 messages.
# Note: receiver is instance 1, sender instance 2.
#
# start up the instances. Note that the envrionment settings can be changed to
# set instance-specific debugging parameters!
#export RSYSLOG_DEBUG="debug nostdout"
#export RSYSLOG_DEBUGLOG="log2"
echo starting receiver
generate_conf
export PORT_RCVR="$(get_free_port)"
add_conf '
$ModLoad ../plugins/imtcp/.libs/imtcp
# then SENDER sends to this port (not tcpflood!)
$InputTCPServerRun '$PORT_RCVR'

$template outfmt,"%msg:F,58:2%\n"
:msg, contains, "msgnum:" ./'$RSYSLOG_OUT_LOG';outfmt
'
startup
#export RSYSLOG_DEBUG="debug nostdout"
#export RSYSLOG_DEBUGLOG="log"
#valgrind="valgrind"
echo starting sender
generate_conf 2
export TCPFLOOD_PORT="$(get_free_port)"
add_conf '
$ModLoad ../plugins/imtcp/.libs/imtcp
# this listener is for message generation by the test framework!
$InputTCPServerRun '$TCPFLOOD_PORT'

$WorkDirectory '$RSYSLOG_DYNNAME'.spool
$MainMsgQueueSize 2000
$MainMsgQueueLowWaterMark 800
$MainMsgQueueHighWaterMark 1000
$MainMsgQueueDequeueBatchSize 1
$MainMsgQueueMaxFileSize 1g
$MainMsgQueueWorkerThreads 1
$MainMsgQueueFileName mainq

# we use the shortest resume interval a) to let the test not run too long 
# and b) make sure some retries happen before the reconnect
$ActionResumeInterval 1
$ActionSendResendLastMsgOnReconnect on
$ActionResumeRetryCount -1
*.*	@@127.0.0.1:'$PORT_RCVR'
' 2
startup 2
# re-set params so that new instances do not thrash it...
#unset RSYSLOG_DEBUG
#unset RSYSLOG_DEBUGLOG

# now inject the messages into instance 2. It will connect to instance 1,
# and that instance will record the data.
injectmsg2  1 1000
wait_queueempty
./msleep 1000 # let things settle down a bit

#
# Step 2: shutdown receiver, then send some more data, which then
# needs to go into the queue.
#
echo step 2

shutdown_when_empty
wait_shutdown

injectmsg2  1001 10000
./msleep 3000 # make sure some retries happen (retry interval is set to 3 second)
get_mainqueuesize 2
ls -l ${RSYSLOG_DYNNAME}.spool

#
# Step 3: restart receiver, wait that the sender drains its queue
#
echo step 3
#export RSYSLOG_DEBUGLOG="log2"
startup
echo waiting for sender to drain queue [may need a short while]
wait_queueempty 2
ls -l ${RSYSLOG_DYNNAME}.spool
OLDFILESIZE=$(stat -c%s ${RSYSLOG_DYNNAME}.spool/mainq.00000001)
echo file size to expect is $OLDFILESIZE


#
# Step 4: send new data. Queue files are not permitted to grow now
# (but one file continous to exist).
#
echo step 4
injectmsg2  11001 10
wait_queueempty 2

# at this point, the queue file shall not have grown. Note
# that we MUST NOT shut down the instance right now, because it
# would clean up the queue files! So we need to do our checks
# first (here!).
ls -l ${RSYSLOG_DYNNAME}.spool
NEWFILESIZE=$(stat -c%s ${RSYSLOG_DYNNAME}.spool/mainq.00000001)
if [ $NEWFILESIZE != $OLDFILESIZE ]
then
   echo file sizes do not match, expected $OLDFILESIZE, actual $NEWFILESIZE
   echo this means that data has been written to the queue file where it
   echo no longer should be written.
   # abort will happen below, because we must ensure proper system shutdown
   # HOWEVER, during actual testing it may be useful to do an exit here (so
   # that e.g. the debug log is pointed right at the correct spot).
   # exit 1
fi

#
# We now do an extra test (so this is two in one ;)) to see if the DA
# queue can be reactivated after its initial shutdown. In essence, we 
# redo steps 2 and 3.
#
# Step 5: stop receiver again, then send some more data, which then
# needs to go into the queue.
#
echo step 5
echo "*** done primary test *** now checking if DA can be restarted"
shutdown_when_empty
wait_shutdown

injectmsg2  11011 10000
sleep 1 # we need to wait, otherwise we may be so fast that the receiver
# comes up before we have finally suspended the action
get_mainqueuesize 2
ls -l ${RSYSLOG_DYNNAME}.spool

#
# Step 6: restart receiver, wait that the sender drains its queue
#
echo step 6
startup
echo waiting for sender to drain queue [may need a short while]
wait_queueempty 2
ls -l ${RSYSLOG_DYNNAME}.spool

#
# Queue file size checks done. Now it is time to terminate the system
# and see if everything could be received (the usual check, done here
# for completeness, more or less as a bonus).
#
shutdown_when_empty 2
wait_shutdown 2

# now it is time to stop the receiver as well
shutdown_when_empty
wait_shutdown

# now abort test if we need to (due to filesize predicate)
if [ $NEWFILESIZE != $OLDFILESIZE ]
then
   exit 1
fi
# do the final check
seq_check 1 21010 -m 100
exit_test
