#!/bin/bash
# add 2018-04-26 by Pascal Withopf, released under ASL 2.0
echo [imrelp-maxDataSize-error.sh]
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imrelp/.libs/imrelp")

global(
	maxMessageSize="300"
)

input(type="imrelp" port="'$TCPFLOOD_PORT'" maxDataSize="250")

action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`)
'
startup
./msleep 2000

shutdown_when_empty
wait_shutdown

grep "error: maxDataSize.*smaller than global parameter maxMessageSize"  $RSYSLOG_OUT_LOG > /dev/null
if [ $? -ne 0 ]; then
        echo
        echo "FAIL: expected error message not found.  $RSYSLOG_OUT_LOG is:"
        cat $RSYSLOG_OUT_LOG
        error_exit 1
fi

exit_test
