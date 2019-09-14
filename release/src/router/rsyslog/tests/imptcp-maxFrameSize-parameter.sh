#!/bin/bash
# addd 2017-03-01 by RGerhards, released under ASL 2.0

. $srcdir/diag.sh init
generate_conf
add_conf '
$MaxMessageSize 12800
global(processInternalMessages="on")
module(load="../plugins/imptcp/.libs/imptcp")
input(type="imptcp" port="'$TCPFLOOD_PORT'" maxframesize="100")

action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`)

'
startup
tcpflood -m1 -M "\"10005 <120> 2011-03-01T11:22:12Z host tag: this is a way too long message\""
shutdown_when_empty
wait_shutdown

grep "Framing Error.*change to octet stuffing"  $RSYSLOG_OUT_LOG > /dev/null
if [ $? -ne 0 ]; then
        echo
        echo "FAIL: expected error message from imptcp truncation not found.  $RSYSLOG_OUT_LOG is:"
        cat $RSYSLOG_OUT_LOG
        error_exit 1
fi

exit_test
