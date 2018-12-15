#!/bin/bash
# Test for "stop" statement
# This file is part of the rsyslog project, released  under ASL 2.0
echo ===============================================================================
echo \[stop.sh\]: testing stop statement
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

if $msg contains "00000001" then
	stop

template(name="outfmt" type="string" string="%msg:F,58:2%\n")
if $msg contains "msgnum:" then
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup
sleep 1
tcpflood -m10 -i1
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown
seq_check 2 10
exit_test
