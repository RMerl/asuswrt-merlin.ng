#!/bin/bash
# added 2016-12-11 by rgerhards
# This file is part of the rsyslog project, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="outfmt" type="list") {
	property(name="msg" field.delimiter="58" field.number="2")
	constant(value="\n")
}

ruleset(name="rs") {
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
}

set $.var = "rs";

if $msg contains "msgnum" then call_indirect $.var;
'
startup
injectmsg  0 100
shutdown_when_empty
wait_shutdown 
seq_check  0 99
exit_test
