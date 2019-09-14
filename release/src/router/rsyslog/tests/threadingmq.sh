#!/bin/bash
# test many concurrent tcp connections
# we send 100,000 messages in the hopes that his puts at least a little bit
# of pressure on the threading subsystem. To really prove it, we would need to
# push messages for several minutes, but that takes too long during the 
# automatted tests (hint: do this manually after suspect changes). Thankfully,
# in practice many threading bugs result in an abort rather quickly and these
# should be covered by this test here.
# rgerhards, 2009-06-26
echo \[threadingmq.sh\]: main queue concurrency
. $srcdir/diag.sh init
generate_conf
add_conf '
$MainMsgQueueTimeoutShutdown 1
#$MainMsgQueueTimeoutShutdown 100000

$MainMsgQueueWorkerThreadMinimumMessages 10
$MainMsgQueueWorkerThreads 5

$template outfmt,"%msg:F,58:2%\n"
template(name="dynfile" type="string" string=`echo $RSYSLOG_OUT_LOG`) # trick to use relative path names!
# write quickly to the output file:
$OMFileFlushOnTXEnd off
$OMFileIOBufferSize 256k 
:msg, contains, "msgnum:" ?dynfile;outfmt
'
startup
injectmsg 0 100000
shutdown_when_empty # shut down rsyslogd when done processing messages
# we give an extra seconds for things to settle, especially
# important on slower test machines
./msleep 5000
wait_shutdown
seq_check 0 99999
exit_test
