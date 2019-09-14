#!/bin/bash
# Test for "stop" statement
# This file is part of the rsyslog project, released  under ASL 2.0
echo ===============================================================================
echo \[stop-msgvar.sh\]: testing stop statement together with message variables
. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="outfmt" type="string" string="%$!nbr%\n")

module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

if $msg contains "msgnum:" then {
	set $!nbr = field($msg, 58, 2);
	if cnum($!nbr) < 100 then
		stop
	/* check is intentionally more complex than needed! */
	else if not (cnum($!nbr) > 999) then {
		action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
	}
}
'
startup
sleep 1
tcpflood -m2000 -i1
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown
seq_check 100 999
exit_test
