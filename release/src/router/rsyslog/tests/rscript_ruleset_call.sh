#!/bin/bash
# added 2012-10-29 by rgerhards
# This file is part of the rsyslog project, released under ASL 2.0
echo ===============================================================================
echo \[rscript_ruleset_call.sh\]: testing rainerscript ruleset\(\) and call statement
. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="outfmt" type="list") {
	property(name="msg" field.delimiter="58" field.number="2")
	constant(value="\n")
}


# we deliberately include continue/stop to make sure we have more than
# one statement. This catches grammar erorrs
ruleset(name="rs2") {
	continue
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
	stop
}

# this time we make sure a single statement is properly supported
ruleset(name="rs1") {
	call rs2
}

if $msg contains "msgnum" then call rs1
'
startup
injectmsg  0 5000
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown 
seq_check  0 4999
exit_test
