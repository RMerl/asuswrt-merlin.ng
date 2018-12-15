#!/bin/bash
# we test the execonly if previous is suspended directive.
# This test checks if multiple backup actions can be defined.
# rgerhards, 2010-06-24
echo ===============================================================================
echo \[execonlywhenprevsuspended4.sh\]: test execonly..suspended multi backup action
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
# note that $ActionExecOnlyWhenPreviousIsSuspended on is still active!
& ./'"${RSYSLOG2_OUT_LOG}"';outfmt
'
startup
injectmsg 0 1000
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown
seq_check 1 999
if [[ -s ${RSYSLOG2_OUT_LOG} ]] ; then
   echo failure: second output file has data where it should be empty
   exit 1
fi ;
exit_test
