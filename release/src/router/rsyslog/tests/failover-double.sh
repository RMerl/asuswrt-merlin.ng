#!/bin/bash
# This file is part of the rsyslog project, released under ASL 2.0
. $srcdir/diag.sh init
export DEAD_PORT=4  # a port unassigned by IANA and very unlikely to be used
generate_conf
add_conf '
$template outfmt,"%msg:F,58:2%\n"

:msg, contains, "msgnum:" @@127.0.0.1:'$DEAD_PORT'
$ActionExecOnlyWhenPreviousIsSuspended on
&	@@127.0.0.1:1234
&	./'"${RSYSLOG_OUT_LOG}"';outfmt
$ActionExecOnlyWhenPreviousIsSuspended off
'
startup
injectmsg  0 5000
shutdown_when_empty
wait_shutdown 
seq_check  0 4999
exit_test
