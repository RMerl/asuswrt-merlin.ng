#!/bin/bash
# Copyright 2014-11-20 by Rainer Gerhards
# This file is part of the rsyslog project, released  under ASL 2.0
. $srcdir/diag.sh init
export TCPFLOOD_PORT2="$(get_free_port)"
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
$MainMsgQueueTimeoutShutdown 10000

input(type="imtcp" port="'$TCPFLOOD_PORT'" ruleset="real")
input(type="imtcp" port="'$TCPFLOOD_PORT2'" ruleset="empty")

$template outfmt,"%msg:F,58:2%\n"
template(name="dynfile" type="string" string=`echo $RSYSLOG_OUT_LOG`) # trick to use relative path names!

ruleset(name="empty") {
}

ruleset(name="real") {
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
}
'
startup
tcpflood -p$TCPFLOOD_PORT2 -m5000 -i0 # these should NOT show up
tcpflood -p$TCPFLOOD_PORT -m10000 -i5000
tcpflood -p$TCPFLOOD_PORT2 -m500 -i15000 # these should NOT show up
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown
seq_check 5000 14999
exit_test
