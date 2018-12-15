#!/bin/bash
# added 2012-09-14 by rgerhards
# This file is part of the rsyslog project, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
$template outfmt,"%msg:F,58:2%\n"
if $msg contains "msgnum" then action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup
injectmsg  0 5000
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown 
seq_check  0 4999
exit_test
