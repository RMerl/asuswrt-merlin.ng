#!/bin/bash
# This file is part of the rsyslog project, released  under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

template(name="outfmt" type="string"
	 string="%msg:F,58:2%,%msg:F,58:3%,%msg:F,58:4%\n")
:msg, contains, "msgnum:" action(type="omfile" template="outfmt"
				 zipLevel="6" ioBufferSize="256k" flushOnTXEnd="off"
			         file=`echo $RSYSLOG_OUT_LOG`)
'
startup
tcpflood -m2500 -P129
tcpflood -i2500 -m2500 -P129
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown       # and wait for it to terminate
gzip_seq_check 0 4999
exit_test
