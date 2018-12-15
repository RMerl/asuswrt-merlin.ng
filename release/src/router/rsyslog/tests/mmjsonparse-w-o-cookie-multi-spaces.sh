#!/bin/bash
# addd 2016-03-22 by RGerhards, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="outfmt" type="string" string="%$!msgnum%\n")

module(load="../plugins/mmjsonparse/.libs/mmjsonparse")
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

action(type="mmjsonparse" cookie="")
if $parsesuccess == "OK" then {
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
}
'
rm -f $RSYSLOG_OUT_LOG	# do cleanup of previous subtest
startup
tcpflood -m 5000 "-j \"      \""
shutdown_when_empty
wait_shutdown
seq_check  0 4999
exit_test
