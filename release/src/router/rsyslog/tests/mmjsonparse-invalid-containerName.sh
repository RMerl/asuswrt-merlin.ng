#!/bin/bash
# add 2018-04-13 by Pascal Withopf, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/impstats/.libs/impstats" interval="300"
	resetCounters="on" format="cee" ruleset="fooruleset" log.syslog="on")
module(load="../plugins/mmjsonparse/.libs/mmjsonparse")

action(name="fooname" type="mmjsonparse" container="foobar")

action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`)
'
startup
shutdown_when_empty
wait_shutdown

grep "mmjsonparse: invalid container name 'foobar', name must start with"  $RSYSLOG_OUT_LOG > /dev/null
if [ $? -ne 0 ]; then
        echo "FAIL: expected error message not found.  $RSYSLOG_OUT_LOG is:"
        cat $RSYSLOG_OUT_LOG
        error_exit 1
fi

grep "impstats: ruleset 'fooruleset' not found - using default ruleset instead"  $RSYSLOG_OUT_LOG > /dev/null
if [ $? -ne 0 ]; then
        echo "FAIL: expected error message not found.  $RSYSLOG_OUT_LOG is:"
        cat $RSYSLOG_OUT_LOG
        error_exit 1
fi

exit_test
