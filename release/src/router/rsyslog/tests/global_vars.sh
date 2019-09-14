#!/bin/bash
# Test for global variables
# added 2013-11-18 by rgerhards
# This file is part of the rsyslog project, released  under ASL 2.0
echo ===============================================================================
echo \[global_vars.sh\]: testing global variable support
. $srcdir/diag.sh init
generate_conf
add_conf '
$MainMsgQueueTimeoutShutdown 10000

module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

template(name="outfmt" type="string" string="%$/msgnum%\n")
template(name="dynfile" type="string" string=`echo $RSYSLOG_OUT_LOG`) /* trick to use relative path names! */

if $/msgnum == "" then
	set $/msgnum = 0;

if $msg contains "msgnum:" then {
	action(type="omfile" dynaFile="dynfile" template="outfmt")
	set $/msgnum = $/msgnum + 1;
}
'
startup

# 40000 messages should be enough
injectmsg  0 40000

shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown 
seq_check 0 39999
exit_test
