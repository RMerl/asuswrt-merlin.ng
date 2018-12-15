#!/bin/bash
# This file is part of the rsyslog project, released  under ASL 2.0

uname
if [ `uname` = "FreeBSD" ] ; then
   echo "This test currently does not work on FreeBSD."
   exit 77
fi

. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

template(name="outfmt" type="string"
	 string="%msg:F,58:2%\n")
:msg, contains, "msgnum:" action(type="omfile" template="outfmt"
				 zipLevel="6" ioBufferSize="256k"
				 flushOnTXEnd="off" flushInterval="1"
				 asyncWriting="on"
			         file=`echo $RSYSLOG_OUT_LOG`)
'
startup
tcpflood -m2500 -P129
./msleep 2500
gzip_seq_check 0 2499
tcpflood -i2500 -m2500 -P129
shutdown_when_empty
wait_shutdown
gzip_seq_check 0 4999
exit_test
