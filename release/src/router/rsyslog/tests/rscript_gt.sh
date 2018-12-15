#!/bin/bash
# added 2014-01-17 by rgerhards
# This file is part of the rsyslog project, released under ASL 2.0
echo ===============================================================================
echo \[rscript_gt.sh\]: testing rainerscript GT statement
. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="outfmt" type="list") {
	property(name="$!usr!msgnum")
	constant(value="\n")
}

if $msg contains "msgnum" then {
	set $!usr!msgnum = field($msg, 58, 2);
	if $!usr!msgnum > "00004999" then
		action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
}
'
startup
injectmsg  0 8000
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown 
seq_check  5000 7999
exit_test
