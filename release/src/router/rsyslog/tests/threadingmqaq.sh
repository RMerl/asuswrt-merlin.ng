#!/bin/bash
# test many concurrent tcp connections
# we send 100,000 messages in the hopes that his puts at least a little bit
# of pressure on the threading subsystem. To really prove it, we would need to
# push messages for several minutes, but that takes too long during the 
# automatted tests (hint: do this manually after suspect changes). Thankfully,
# in practice many threading bugs result in an abort rather quickly and these
# should be covered by this test here.
# rgerhards, 2009-06-26

uname
if [ `uname` = "SunOS" ] ; then
   echo "This test currently does not work on all flavors of Solaris."
   exit 77
fi

. $srcdir/diag.sh init
generate_conf
add_conf '
$MainMsgQueueTimeoutShutdown 10000

$MainMsgQueueWorkerThreadMinimumMessages 10
$MainMsgQueueWorkerThreads 5

$template outfmt,"%msg:F,58:2%\n"
template(name="dynfile" type="string" string=`echo $RSYSLOG_OUT_LOG`) # trick to use relative path names!
# write quickly to the output file:
$OMFileFlushOnTXEnd off
$OMFileIOBufferSize 256k 
# This time, also run the action queue detached
$ActionQueueWorkerThreadMinimumMessages 10
$ActionQueueWorkerThreads 5
$ActionQueueTimeoutEnqueue 500
$ActionQueueType LinkedList
:msg, contains, "msgnum:" ?dynfile;outfmt
'
startup
#tcpflood -c2 -m100000
#shutdown_when_empty # shut down rsyslogd when done processing messages
injectmsg 0 100000
# we need to sleep a bit on some environments, as imdiag can not correctly
# diagnose when the action queues are empty...
sleep 3
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown
seq_check 0 99999
exit_test
