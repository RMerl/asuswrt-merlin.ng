#!/bin/bash
# added 2014-01-17 by rgerhards
# This file is part of the rsyslog project, released under ASL 2.0
echo ===============================================================================
echo \[rscript_ne.sh\]: testing rainerscript NE statement
. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="outfmt" type="list") {
	property(name="$!usr!msgnum")
	constant(value="\n")
}

if $msg contains "msgnum" then {
	set $!usr!msgnum = field($msg, 58, 2);
	if $!usr!msgnum != "00005000" and
	   $!usr!msgnum != "00005001" and
	   $!usr!msgnum != "00005002" then
		set $!usr!write = 0;
	else
		set $!usr!write = 1;
	if $!usr!write == 1 then
		action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
}
'
startup
injectmsg  0 8000
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown 
seq_check  5000 5002
exit_test
