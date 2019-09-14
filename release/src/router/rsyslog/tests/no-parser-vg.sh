#!/bin/bash
# add 2017-03-06 by Rainer Gerhards, released under ASL 2.0

uname
if [ `uname` = "FreeBSD" ] ; then
   echo "This test currently does not work on FreeBSD."
   exit 77
fi

. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'" ruleset="ruleset")
ruleset(name="ruleset" parser="rsyslog.rfc5424") {
	action(type="omfile" file=`echo $RSYSLOG2_OUT_LOG`)
}
'
startup_vg
tcpflood -m10
shutdown_when_empty
wait_shutdown_vg
# note: we just check the valgrind output, the log file itself does not
# interest us

exit_test
