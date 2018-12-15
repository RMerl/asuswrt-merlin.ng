#!/bin/bash
# add 2018-04-19 by Pascal Withopf, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
$AbortOnUncleanConfig on
$LocalHostName wtpshutdownall
$PreserveFQDN on

global(
	workDirectory="'${RSYSLOG_DYNNAME}'.spool"
)

module(load="../plugins/mmjsonparse/.libs/mmjsonparse")
module(load="../plugins/impstats/.libs/impstats" interval="300"
	resetCounters="on" format="cee" ruleset="metrics-impstat" log.syslog="on")

ruleset(name="metrics-impstat" queue.type="Direct"){
	action(type="omfile" file="'$RSYSLOG_DYNNAME'.spool/stats.log")
}
'
startup
shutdown_when_empty
wait_shutdown

# This test only checks that rsyslog does not abort
# so we don't need to check for output.

exit_test
