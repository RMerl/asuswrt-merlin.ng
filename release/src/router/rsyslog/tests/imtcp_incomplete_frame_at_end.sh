#!/bin/bash
# Copyright (C) 2016 by Rainer Gerhardds
# This file is part of the rsyslog project, released  under ASL 2.0

. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

template(name="outfmt" type="list") {
	property(name="msg")
	constant(value="\n")
}
:msg, contains, "lastmsg" action(type="omfile" template="outfmt"
			         file=`echo $RSYSLOG_OUT_LOG`)
'
startup
echo -n "<165>1 2003-08-24T05:14:15.000003-07:00 192.0.2.1 tcpflood 8710 - - lastmsg" >$RSYSLOG_DYNNAME.tmp
tcpflood -I $RSYSLOG_DYNNAME.tmp
rm $RSYSLOG_DYNNAME.tmp
./msleep 500
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown       # and wait for it to terminate
echo "lastmsg" | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "lastmsg was not properly recorded, file content:"
  cat $RSYSLOG_OUT_LOG
  exit 1
fi;
exit_test
