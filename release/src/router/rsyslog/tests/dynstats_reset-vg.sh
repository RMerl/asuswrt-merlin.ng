#!/bin/bash
# added 2015-11-13 by singh.janmejay
# This file is part of the rsyslog project, released under ASL 2.0

uname
if [ `uname` = "FreeBSD" ] ; then
   echo "This test currently does not work on FreeBSD."
   exit 77
fi

echo ===============================================================================
echo \[dynstats_reset-vg.sh\]: test for gathering stats with a known-dyn-metrics reset in-between
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
startup_vg
. $srcdir/diag.sh wait-for-stats-flush ${RSYSLOG_DYNNAME}.out.stats.log
. $srcdir/diag.sh injectmsg-litteral $srcdir/testsuites/dynstats_input_1
rst_msleep 8100 #two seconds for unused-metrics to be kept under observation, another two them to be cleared off
. $srcdir/diag.sh injectmsg-litteral $srcdir/testsuites/dynstats_input_2
rst_msleep 8100
. $srcdir/diag.sh injectmsg-litteral $srcdir/testsuites/dynstats_input_3
rst_msleep 8100
wait_queueempty
content_check "foo 001 0"
content_check "bar 002 0"
content_check "baz 003 0"
content_check "foo 004 0"
content_check "baz 005 0"
content_check "foo 006 0"
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown_vg
check_exit_vg
 # because dyn-metrics would be reset before it can accumulate and report high counts, sleep between msg-injection ensures that
custom_assert_content_missing 'baz=2' "${RSYSLOG_DYNNAME}.out.stats.log"
custom_assert_content_missing 'foo=2' "${RSYSLOG_DYNNAME}.out.stats.log"
custom_assert_content_missing 'foo=3' "${RSYSLOG_DYNNAME}.out.stats.log"
# but actual reported stats (aggregate) should match
. $srcdir/diag.sh first-column-sum-check 's/.*foo=\([0-9]\+\)/\1/g' 'foo=' "${RSYSLOG_DYNNAME}.out.stats.log" 3
. $srcdir/diag.sh first-column-sum-check 's/.*bar=\([0-9]\+\)/\1/g' 'bar=' "${RSYSLOG_DYNNAME}.out.stats.log" 1
. $srcdir/diag.sh first-column-sum-check 's/.*baz=\([0-9]\+\)/\1/g' 'baz=' "${RSYSLOG_DYNNAME}.out.stats.log" 2
exit_test
