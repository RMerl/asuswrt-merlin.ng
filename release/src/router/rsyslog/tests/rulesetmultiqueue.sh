#!/bin/bash
# Test for disk-only queue mode
# This tests defines three rulesets, each one with its own queue. Then, it
# sends data to them and checks the outcome. Note that we do need to
# use some custom code as the test driver framework does not (yet?)
# support multi-output-file operations.
# added 2009-10-30 by Rgerhards
# This file is part of the rsyslog project, released  under ASL 2.0
uname
if [ `uname` = "SunOS" ] ; then
   echo "This test currently does not work on all flavors of Solaris."
   exit 77
fi

. $srcdir/diag.sh init
export RSYSLOG_PORT2="$(get_free_port)"
export RSYSLOG_PORT3="$(get_free_port)"
generate_conf
add_conf '
$ModLoad ../plugins/imtcp/.libs/imtcp
$MainMsgQueueTimeoutShutdown 10000

# general definition
$template outfmt,"%msg:F,58:2%\n"

# create the individual rulesets
$ruleset file1
$RulesetCreateMainQueue on
$template dynfile1,"'$RSYSLOG_OUT_LOG'1.log" # trick to use relative path names!
:msg, contains, "msgnum:" ?dynfile1;outfmt

$ruleset file2
$RulesetCreateMainQueue on
$template dynfile2,"'$RSYSLOG_OUT_LOG'2.log" # trick to use relative path names!
:msg, contains, "msgnum:" ?dynfile2;outfmt

$ruleset file3
$RulesetCreateMainQueue on
$template dynfile3,"'$RSYSLOG_OUT_LOG'3.log" # trick to use relative path names!
:msg, contains, "msgnum:" ?dynfile3;outfmt

# start listeners and bind them to rulesets
$InputTCPServerBindRuleset file1
$InputTCPServerRun '$TCPFLOOD_PORT'

$InputTCPServerBindRuleset file2
$InputTCPServerRun '$RSYSLOG_PORT2'

$InputTCPServerBindRuleset file3
$InputTCPServerRun '$RSYSLOG_PORT3'
'
rm -f ${RSYSLOG_OUT_LOG}1.log ${RSYSLOG_OUT_LOG}2.log ${RSYSLOG_OUT_LOG}3.log
startup
# now fill the three files (a bit sequentially, but they should
# still get their share of concurrency - to increase the chance
# we use three connections per set).
tcpflood -c3 -p'$TCPFLOOD_PORT' -m20000 -i0
tcpflood -c3 -p'$RSYSLOG_PORT2' -m20000 -i20000
tcpflood -c3 -p'$RSYSLOG_PORT3' -m20000 -i40000

# in this version of the imdiag, we do not have the capability to poll
# all queues for emptyness. So we do a sleep in the hopes that this will
# sufficiently drain the queues. This is race, but the best we currently
# can do... - rgerhards, 2009-11-05
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown
# now consolidate all logs into a single one so that we can use the
# regular check logic
cat ${RSYSLOG_OUT_LOG}1.log ${RSYSLOG_OUT_LOG}2.log ${RSYSLOG_OUT_LOG}3.log > $RSYSLOG_OUT_LOG
seq_check 0 59999
rm -f ${RSYSLOG_OUT_LOG}1.log ${RSYSLOG_OUT_LOG}2.log ${RSYSLOG_OUT_LOG}3.log
exit_test
