#!/bin/bash
# the goal here is to detect memleaks when structured data is not
# correctly parsed.
# This file is part of the rsyslog project, released  under ASL 2.0
# rgerhards, 2015-04-30
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/mmpstrucdata/.libs/mmpstrucdata")
module(load="../plugins/imtcp/.libs/imtcp")

template(name="outfmt" type="string" string="SD:%$!RFC5424-SD%\n")

input(type="imtcp" port="'$TCPFLOOD_PORT'")

action(type="mmpstrucdata" sd_name.lowercase="off")

action(type="omfile" template="outfmt" file=`echo $RSYSLOG_OUT_LOG`)
'

uname
if [ `uname` = "FreeBSD" ] ; then
   echo "This test currently does not work on FreeBSD."
   exit 77
fi

startup
tcpflood -m100 -M "\"<161>1 2003-03-01T01:00:00.000Z mymachine.example.com tcpflood - tag [tcpflood@32473 eventID=\\\"1011\\\"] valid structured data\""
shutdown_when_empty
wait_shutdown
content_check_with_count eventID 100
exit_test
