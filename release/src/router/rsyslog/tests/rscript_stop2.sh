#!/bin/bash
# added 2012-09-20 by rgerhards
# This file is part of the rsyslog project, released under ASL 2.0
echo ===============================================================================
echo \[rscript_stop2.sh\]: testing rainerscript STOP statement, alternate method
. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="outfmt" type="list") {
	property(name="$!usr!msgnum")
	constant(value="\n")
}

if not ($msg contains "msgnum") then
	stop

set $!usr!msgnum = field($msg, 58, 2);
if cnum($!usr!msgnum) >= 5000 then
	stop
/* We could use yet another method, but we like to have the action statement
 * without a filter in rsyslog.conf top level hierarchy - so this test, as
 * a side-effect, also tests this ability.
 */
action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup
injectmsg  0 8000
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown 
seq_check  0 4999
exit_test
