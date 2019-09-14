#!/bin/bash
# rgerhards, 2013-12-05
echo =====================================================================================
echo \[execonlywhenprevsuspended_multiwrkr.sh\]: test execonly...suspended functionality multiworker case
. $srcdir/diag.sh init
generate_conf
add_conf '
# omtesting provides the ability to cause "SUSPENDED" action state
$ModLoad ../plugins/omtesting/.libs/omtesting

$MainMsgQueueTimeoutShutdown 100000
$template outfmt,"%msg:F,58:2%\n"

:msg, contains, "msgnum:" :omtesting:fail 2 0
$ActionExecOnlyWhenPreviousIsSuspended on
&			   ./'"${RSYSLOG_OUT_LOG}"';outfmt
'
startup
# we initially send only 10 messages. It has shown that if we send more,
# we cannot really control which are the first two messages imdiag sees,
# and so we do not know for sure which numbers are skipped. So we inject
# those 10 to get past that point.
injectmsg 0 10
./msleep 500
injectmsg 10 990
shutdown_when_empty
wait_shutdown
seq_check 1 999
exit_test
