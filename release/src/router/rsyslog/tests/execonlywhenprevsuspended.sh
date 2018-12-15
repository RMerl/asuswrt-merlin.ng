#!/bin/bash
# we test the execonly if previous is suspended directive. This is the
# most basic test which soley tests a singel case but no dependencies within
# the ruleset.
# rgerhards, 2010-06-23
echo =====================================================================================
echo \[execonlywhenprevsuspended.sh\]: test execonly...suspended functionality simple case
. $srcdir/diag.sh init
generate_conf
add_conf '
main_queue(queue.workerthreads="1") 

# omtesting provides the ability to cause "SUSPENDED" action state
$ModLoad ../plugins/omtesting/.libs/omtesting

$MainMsgQueueTimeoutShutdown 100000
$template outfmt,"%msg:F,58:2%\n"

:msg, contains, "msgnum:" :omtesting:fail 2 0
$ActionExecOnlyWhenPreviousIsSuspended on
&			   ./'"${RSYSLOG_OUT_LOG}"';outfmt
'
startup
injectmsg 0 1000
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown
seq_check 1 999
exit_test
