#!/bin/bash
# added 2016-04-13 by singh.janmejay
# This file is part of the rsyslog project, released under ASL 2.0

uname
if [ `uname` = "FreeBSD" ] ; then
   echo "This test currently does not work on FreeBSD."
   exit 77
fi

echo ===============================================================================
echo \[dynstats_prevent_premature_eviction.sh\]: test for ensuring metrics are not evicted before unused-ttl
. $srcdir/diag.sh init
generate_conf
add_conf '
ruleset(name="stats") {
  action(type="omfile" file="'${RSYSLOG_DYNNAME}'.out.stats.log")
}

module(load="../plugins/impstats/.libs/impstats" interval="4" severity="7" resetCounters="on" Ruleset="stats" bracketing="on")

template(name="outfmt" type="string" string="%msg% %$.increment_successful%\n")

dyn_stats(name="msg_stats" unusedMetricLife="1" resettable="off")

set $.msg_prefix = field($msg, 32, 1);

if (re_match($.msg_prefix, "foo|bar|baz|quux|corge|grault")) then {
  set $.increment_successful = dyn_inc("msg_stats", $.msg_prefix);
} else {
  set $.increment_successful = -1;
}

action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup
. $srcdir/diag.sh wait-for-stats-flush ${RSYSLOG_DYNNAME}.out.stats.log
rst_msleep 1000
. $srcdir/diag.sh injectmsg-litteral $srcdir/testsuites/dynstats_input_1
rst_msleep 4000
. $srcdir/diag.sh injectmsg-litteral $srcdir/testsuites/dynstats_input_2
rst_msleep 4000
. $srcdir/diag.sh injectmsg-litteral $srcdir/testsuites/dynstats_input_3
wait_queueempty
. $srcdir/diag.sh wait-for-stats-flush ${RSYSLOG_DYNNAME}.out.stats.log
content_check "foo 001 0"
content_check "foo 006 0"
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown
 # because dyn-accumulators for existing metrics were posted-to under a second, they should not have been evicted
custom_content_check 'baz=2' "${RSYSLOG_DYNNAME}.out.stats.log"
custom_content_check 'bar=1' "${RSYSLOG_DYNNAME}.out.stats.log"
custom_content_check 'foo=3' "${RSYSLOG_DYNNAME}.out.stats.log"
# sum is high because accumulators were never reset, and we expect them to last specific number of cycles(when we posted before ttl expiry)
. $srcdir/diag.sh first-column-sum-check 's/.*foo=\([0-9]\+\)/\1/g' 'foo=' "${RSYSLOG_DYNNAME}.out.stats.log" 6
. $srcdir/diag.sh first-column-sum-check 's/.*bar=\([0-9]\+\)/\1/g' 'bar=' "${RSYSLOG_DYNNAME}.out.stats.log" 1
. $srcdir/diag.sh first-column-sum-check 's/.*baz=\([0-9]\+\)/\1/g' 'baz=' "${RSYSLOG_DYNNAME}.out.stats.log" 3
. $srcdir/diag.sh first-column-sum-check 's/.*new_metric_add=\([0-9]\+\)/\1/g' 'new_metric_add=' "${RSYSLOG_DYNNAME}.out.stats.log" 3
. $srcdir/diag.sh first-column-sum-check 's/.*ops_overflow=\([0-9]\+\)/\1/g' 'ops_overflow=' "${RSYSLOG_DYNNAME}.out.stats.log" 0
. $srcdir/diag.sh first-column-sum-check 's/.*no_metric=\([0-9]\+\)/\1/g' 'no_metric=' "${RSYSLOG_DYNNAME}.out.stats.log" 0
exit_test
