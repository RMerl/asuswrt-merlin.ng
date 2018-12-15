#!/bin/bash
# detect queue corruption based on invalid property bag ordering.
# Note: this mimics an issue actually seen in practice.
# Triggering condition: "json" property (message variables) are present
# and "structured-data" property is also present. Caused rsyslog to
# thrash the queue file, getting messages stuck in it and loosing all
# after the initial problem occurence.
# add 2017-02-08 by Rainer Gerhards, released under ASL 2.0

uname
if [ `uname` = "SunOS" ] ; then
   echo "This test currently does not work on all flavors of Solaris."
   exit 77
fi

. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'" ruleset="rs")


template(name="outfmt" type="string" string="%msg:F,58:2%\n")

ruleset(name="rs2" queue.type="disk" queue.filename="rs2_q"
	queue.spoolDirectory="'${RSYSLOG_DYNNAME}'.spool") {
	set $!tmp=$msg;
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
}
ruleset(name="rs") {
	set $!tmp=$msg;
	call rs2
}
'
startup
tcpflood -m1000 -y
shutdown_when_empty
wait_shutdown
seq_check 0 999

exit_test
