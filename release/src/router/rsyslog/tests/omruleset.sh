#!/bin/bash
# Basic test for omruleset. What we do is have the main queue forward 
# all messages to a secondary ruleset via omruleset, which then does
# the actual file write. We check if all messages arrive at the file, 
# what implies that omruleset works. No filters or special queue modes
# are used, so the message is re-enqueued into the main message queue.
# We inject just 5,000 message because we may otherwise run into
# queue full conditions (as we use the same queue) and that
# would result in longer execution time. In any case, 5000 messages
# are well enough to test what we want to test.
# added 2009-11-02 by rgerhards
# This file is part of the rsyslog project, released under GPLv3
echo ===============================================================================
echo \[omruleset.sh\]: basic test for omruleset functionality
. $srcdir/diag.sh init
generate_conf
add_conf '
$ModLoad ../plugins/omruleset/.libs/omruleset
$ModLoad ../plugins/imtcp/.libs/imtcp
$InputTCPServerRun '$TCPFLOOD_PORT'

$ruleset rsinclude
$template outfmt,"%msg:F,58:2%\n"
template(name="dynfile" type="string" string=`echo $RSYSLOG_OUT_LOG`) # trick to use relative path names!
:msg, contains, "msgnum:" ?dynfile;outfmt

$ruleset RSYSLOG_DefaultRuleset
$ActionOmrulesetRulesetName rsinclude
*.* :omruleset:
'
startup
injectmsg  0 5000
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown 
seq_check  0 4999
exit_test
