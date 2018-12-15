#!/bin/bash
# A test that checks for memory leaks
# created based on real world case:
# https://github.com/rsyslog/rsyslog/issues/1376
# Copyright 2017-01-24 by Rainer Gerhards
# This file is part of the rsyslog project, released under ASL 2.0

uname
if [ `uname` = "FreeBSD" ] ; then
   echo "This test currently does not work on FreeBSD."
   exit 77
fi

. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'" ruleset="rcvr")

template(name="json" type="string" string="%$!%\n")
template(name="ts" type="string" string="%timestamp:::date-rfc3339%")
ruleset(name="rcvr" queue.type="LinkedList") {
	set $.index="unknown";
	set $.type="unknown";
	set $.interval=$$now & ":" & $$hour;
	set $!host_forwarded=$hostname;
	set $!host_received=$$myhostname;
	set $!time_received=$timegenerated;
	set $!@timestamp=exec_template("ts");
	action( type="omfile"
		file=`echo $RSYSLOG_OUT_LOG`
		template="json"
	)
}'
startup_vg
tcpflood -m5000
shutdown_when_empty
wait_shutdown_vg
check_exit_vg
# note: we check only the valgrind result, we are not really interested
# in the output data (non-standard format in any way...)
exit_test
