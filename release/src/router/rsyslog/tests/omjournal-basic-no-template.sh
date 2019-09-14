#!/bin/bash
# a basic test for omjournal.
# addd 2016-03-18 by RGerhards, released under ASL 2.0
. $srcdir/diag.sh init
. $srcdir/diag.sh require-journalctl
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
module(load="../plugins/omjournal/.libs/omjournal")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

action(type="omjournal")
'
startup
tcpflood -m1 -M "\"<133>2011-03-01T11:22:12Z host tag msgh RsysLoG-TESTBENCH $COOKIE\""
./msleep 500
shutdown_when_empty
wait_shutdown
# if we reach this, we have at least not aborted
journalctl -r -t rsyslogd:  |grep "RsysLoG-TESTBENCH $COOKIE"
if [ $? -ne 1 ]; then
	echo "error: cookie $COOKIE not found. Head of journal:"
	journalctrl -r -t rsyslogd: | head
	exit 1
fi
exit_test
