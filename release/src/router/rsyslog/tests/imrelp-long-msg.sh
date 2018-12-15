#!/bin/bash
# adddd 2018-04-16 by PascalWithopf, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
global(maxMessageSize="214800")
module(load="../plugins/imrelp/.libs/imrelp")
input(type="imrelp" port="'$TCPFLOOD_PORT'" maxdatasize="214800")

template(name="outfmt" type="string" string="%msg:F,58:2%\n")
:msg, contains, "msgnum:" action(type="omfile" template="outfmt"
				 file=`echo $RSYSLOG_OUT_LOG`)
'
startup
tcpflood -Trelp-plain -p'$TCPFLOOD_PORT' -m2 -d 204800
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown
seq_check 0 1
exit_test
