#!/bin/bash
# added 2018-04-16 by Rainer Gerhards, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="outfmt" type="string" string="%$.msgnum%\n")

module(load="../plugins/mmjsonparse/.libs/mmjsonparse")
module(load="../plugins/imptcp/.libs/imptcp")
input(type="imptcp" port="'$TCPFLOOD_PORT'")

action(type="mmjsonparse" cookie="@cim:" container="$.")
if $parsesuccess == "OK" then {
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
}
'
startup
tcpflood -m 5000 -j "@cim: "
shutdown_when_empty
wait_shutdown
seq_check  0 4999
exit_test

