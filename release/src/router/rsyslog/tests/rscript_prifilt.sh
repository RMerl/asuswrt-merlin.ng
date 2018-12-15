#!/bin/bash
# added 2012-09-20 by rgerhards
# This file is part of the rsyslog project, released under ASL 2.0
echo ===============================================================================
echo \[rscript_prifilt.sh\]: testing rainerscript prifield\(\) function
. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="outfmt" type="list") {
	property(name="msg" field.delimiter="58" field.number="2")
	constant(value="\n")
}

/* tcpflood uses local4.=debug, we use a bit more generic filter */
if prifilt("local4.*") then
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup
injectmsg  0 5000
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown 
seq_check  0 4999
exit_test
