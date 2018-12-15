#!/bin/bash
# Test for the pipe output action.
# will create a fifo in the current directory, write to it and
# then do the usual sequence checks.
# added 2009-11-05 by RGerhards
echo ===============================================================================
echo \[pipeaction.sh\]: testing pipe output action

uname
if [ `uname` = "SunOS" ] ; then
   echo "Solaris: FIX ME"
   exit 77
fi


# create the pipe and start a background process that copies data from 
# it to the "regular" work file
. $srcdir/diag.sh init
generate_conf
add_conf '
$MainMsgQueueTimeoutShutdown 10000

# set spool locations and switch queue to disk-only mode
$WorkDirectory '$RSYSLOG_DYNNAME'.spool
$MainMsgQueueFilename mainq
$MainMsgQueueType disk

$template outfmt,"%msg:F,58:2%\n"
# with pipes, we do not need to use absolute path names, so
# we can simply refer to our working pipe via the usual relative
# path name
:msg, contains, "msgnum:" |rsyslog-testbench-fifo;outfmt
'
rm -f rsyslog-testbench-fifo
mkfifo rsyslog-testbench-fifo
cp rsyslog-testbench-fifo  $RSYSLOG_OUT_LOG &
CPPROCESS=$!
echo background cp process id is $CPPROCESS

# now do the usual run
startup
# 20000 messages should be enough
#tcpflood -m20000
injectmsg 0 20000
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown

# wait for the cp process to finish, do pipe-specific cleanup
echo waiting for background cp to terminate...
wait $CPPROCESS
rm -f rsyslog-testbench-fifo
echo background cp has terminated, continue test...

# and continue the usual checks
seq_check 0 19999
exit_test
