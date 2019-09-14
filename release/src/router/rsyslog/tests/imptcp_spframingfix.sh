#!/bin/bash
# This file is part of the rsyslog project, released  under ASL 2.0
echo ====================================================================================
echo TEST: \[imptcp_spframingfix.sh\]: test imptcp in regard to Cisco ASA framing fix
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imptcp/.libs/imptcp")
input(type="imptcp" port="'$TCPFLOOD_PORT'" ruleset="remote" framingfix.cisco.asa="on")

template(name="outfmt" type="string" string="%rawmsg:6:7%\n")
ruleset(name="remote") {
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
}
'
startup
tcpflood -B -I ${srcdir}/testsuites/spframingfix.testdata
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown       # and wait for it to terminate
seq_check 0 19
exit_test
