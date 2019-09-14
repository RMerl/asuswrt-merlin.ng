#!/bin/bash
# Test concurrency of message variables
# Added 2015-11-03 by rgerhards
# This file is part of the rsyslog project, released  under ASL 2.0
echo ===============================================================================
echo \[localvar-concurrency.sh\]: testing concurrency of local variables

uname
if [ `uname` = "SunOS" ] ; then
   echo "This test currently does not work on all flavors of Solaris."
   exit 77
fi

. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

template(name="outfmt" type="string" string="%$.tree!here!nbr%\n")

if $msg contains "msgnum:" then {
	set $.tree!here!nbr = field($msg, 58, 2);
	action(type="omfile" file=`echo $RSYSLOG2_OUT_LOG` template="outfmt"
	       queue.type="linkedList")

	set $.tree!here!save = $.tree!here!nbr;
	set $.tree!here!nbr = "";
	set $.tree!here!nbr = $.tree!here!save;
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
}
'
startup
sleep 1
tcpflood -m500000
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown
seq_check 0 499999
exit_test
