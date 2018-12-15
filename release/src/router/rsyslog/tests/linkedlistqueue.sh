#!/bin/bash
# Test for Linkedlist queue mode
# added 2009-05-20 by rgerhards
# This file is part of the rsyslog project, released  under GPLv3
. $srcdir/diag.sh init
generate_conf
add_conf '
$ModLoad ../plugins/imtcp/.libs/imtcp
$MainMsgQueueTimeoutShutdown 10000
$InputTCPServerRun '$TCPFLOOD_PORT'

$ErrorMessagesToStderr off

# set spool locations and switch queue to disk-only mode
$MainMsgQueueType LinkedList

$template outfmt,"%msg:F,58:2%\n"
template(name="dynfile" type="string" string=`echo $RSYSLOG_OUT_LOG`) # trick to use relative path names!
:msg, contains, "msgnum:" ?dynfile;outfmt
'
startup
injectmsg  0 40000
shutdown_when_empty
wait_shutdown 
seq_check 0 39999
exit_test
