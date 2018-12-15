#!/bin/bash
# This file is part of the rsyslog project, released  under ASL 2.0
# rgerhards, 2013-11-22
echo ===============================================================================
echo \[rfc5424parser.sh\]: testing mmpstrucdata
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")

template(name="outfmt" type="string" string="%msg:F,58:2%\n")

input(type="imtcp" port="'$TCPFLOOD_PORT'")

if $msg contains "msgnum" then
	action(type="omfile" template="outfmt" file=`echo $RSYSLOG_OUT_LOG`)
'
startup
sleep 1
tcpflood -m100 -y
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown
seq_check 0 99
exit_test
