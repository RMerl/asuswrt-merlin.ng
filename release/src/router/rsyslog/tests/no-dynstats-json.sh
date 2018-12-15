#!/bin/bash
# added 2016-03-10 by singh.janmejay
# This file is part of the rsyslog project, released under ASL 2.0
echo ===============================================================================
echo \[no-dynstats-json.sh\]: test for verifying stats are reported correctly in json format in absence of any dynstats buckets being configured
. $srcdir/diag.sh init
generate_conf
add_conf '
ruleset(name="stats") {
  action(type="omfile" file="'${RSYSLOG_DYNNAME}'.out.stats.log")
}

module(load="../plugins/impstats/.libs/impstats" interval="1" severity="7" resetCounters="on" Ruleset="stats" bracketing="on" format="json")

action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`)
'
startup
. $srcdir/diag.sh wait-for-stats-flush ${RSYSLOG_DYNNAME}.out.stats.log
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown
custom_content_check '{ "name": "global", "origin": "dynstats", "values": { } }' "${RSYSLOG_DYNNAME}.out.stats.log"
exit_test
