#!/bin/bash
# This file is part of the rsyslog project, released under GPLv3
echo ===============================================================================
echo \[failover-basic.sh\]: basic test for failover functionality
. $srcdir/diag.sh init
generate_conf
add_conf '
$template outfmt,"%msg:F,58:2%\n"
# note: the target server shall not be available!
:msg, contains, "msgnum:" @@127.0.0.1:13514
$ActionExecOnlyWhenPreviousIsSuspended on
& ./'"${RSYSLOG_OUT_LOG}"';outfmt
'
startup
injectmsg  0 5000
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown 
seq_check  0 4999
exit_test
