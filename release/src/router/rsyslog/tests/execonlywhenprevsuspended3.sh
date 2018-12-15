#!/bin/bash
# we test the execonly if previous is suspended directive.
# This test checks if, within the same rule, one action can be set
# to emit only if the previous was suspended while the next action
# always sends data.
# rgerhards, 2010-06-24
echo ===============================================================================
echo \[execonlywhenprevsuspended3.sh\]: test execonly...suspended functionality
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
# note that we MUST re-set PrevSusp, else it will remain active
# for all other actions as well (this tells us how bad the current
# config language is...). -- rgerhards, 2010-06-24
$ActionExecOnlyWhenPreviousIsSuspended off
& ./'"${RSYSLOG2_OUT_LOG}"';outfmt
'
startup
injectmsg 0 1000
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown
echo check file 1
seq_check 1 999
echo check file 2
seq_check2 0 999
exit_test
