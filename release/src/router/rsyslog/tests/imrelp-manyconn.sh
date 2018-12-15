#!/bin/bash
# adddd 2016-06-08 by RGerhards, released under ASL 2.0
. $srcdir/diag.sh init
uname
if [ `uname` = "FreeBSD" ] ; then
   echo "This test currently does not work on FreeBSD."
   exit 77
fi

generate_conf
add_conf '
module(load="../plugins/imrelp/.libs/imrelp")
input(type="imrelp" port="'$TCPFLOOD_PORT'")

template(name="outfmt" type="string" string="%msg:F,58:2%\n")
:msg, contains, "msgnum:" action(type="omfile" template="outfmt"
			         file=`echo $RSYSLOG_OUT_LOG`)
'
startup
tcpflood -Trelp-plain -c-2000 -p'$TCPFLOOD_PORT' -m100000
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown
seq_check 0 99999
exit_test
