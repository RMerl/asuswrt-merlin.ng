#!/bin/bash
# test for omruleset. What we do is have the main queue forward 
# all messages to a secondary ruleset via omruleset, which then does
# the actual file write. We check if all messages arrive at the file, 
# what implies that omruleset works. No filters or special queue modes
# are used, but the ruleset uses its own queue. So we can also inject
# more messages without running into troubles.
# added 2009-11-02 by rgerhards
# This file is part of the rsyslog project, released under GPLv3
echo ===============================================================================
echo \[omruleset-queue.sh\]: test for omruleset functionality with a ruleset queue

uname
if [ `uname` = "SunOS" ] ; then
   echo "This test currently does not work on all flavors of Solaris."
   exit 77
fi

. $srcdir/diag.sh init
generate_conf
add_conf '
$ModLoad ../plugins/omruleset/.libs/omruleset
$ModLoad ../plugins/imtcp/.libs/imtcp
$InputTCPServerRun '$TCPFLOOD_PORT'

$ruleset rsinclude
# create ruleset main queue with default parameters
$RulesetCreateMainQueue on
# make sure we do not terminate too early!
$MainMsgQueueTimeoutShutdown 10000
$template outfmt,"%msg:F,58:2%\n"
template(name="dynfile" type="string" string=`echo $RSYSLOG_OUT_LOG`) # trick to use relative path names!
:msg, contains, "msgnum:" ?dynfile;outfmt

$ruleset RSYSLOG_DefaultRuleset
$ActionOmrulesetRulesetName rsinclude
*.* :omruleset:
'
startup
injectmsg  0 20000
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown 
seq_check 0 19999
exit_test
