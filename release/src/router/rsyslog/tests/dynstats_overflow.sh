#!/bin/bash
# added 2015-11-13 by singh.janmejay
# This file is part of the rsyslog project, released under ASL 2.0
. $srcdir/diag.sh init

uname
if [ `uname` = "FreeBSD" ] ; then
   echo "This test currently does not work on FreeBSD."
   exit 77
fi

generate_conf
add_conf '
ruleset(name="stats") {
  action(type="omfile" file="'${RSYSLOG_DYNNAME}'.out.stats.log")
}

module(load="../plugins/impstats/.libs/impstats" interval="2" severity="7" resetCounters="on" Ruleset="stats" bracketing="on")

template(name="outfmt" type="string" string="%msg% %$.increment_successful%\n")

dyn_stats(name="msg_stats" unusedMetricLife="1" maxCardinality="3")

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
. $srcdir/diag.sh block-stats-flush
. $srcdir/diag.sh injectmsg-litteral $srcdir/testsuites/dynstats_input_more_0
. $srcdir/diag.sh injectmsg-litteral $srcdir/testsuites/dynstats_input_more_1
wait_queueempty
. $srcdir/diag.sh allow-single-stats-flush-after-block-and-wait-for-it

. $srcdir/diag.sh first-column-sum-check 's/.*foo=\([0-9]\+\)/\1/g' 'foo=' "${RSYSLOG_DYNNAME}.out.stats.log" 5
. $srcdir/diag.sh first-column-sum-check 's/.*bar=\([0-9]\+\)/\1/g' 'bar=' "${RSYSLOG_DYNNAME}.out.stats.log" 1
. $srcdir/diag.sh first-column-sum-check 's/.*baz=\([0-9]\+\)/\1/g' 'baz=' "${RSYSLOG_DYNNAME}.out.stats.log" 2

custom_assert_content_missing 'quux' "${RSYSLOG_DYNNAME}.out.stats.log"
custom_assert_content_missing 'corge' "${RSYSLOG_DYNNAME}.out.stats.log"
custom_assert_content_missing 'grault' "${RSYSLOG_DYNNAME}.out.stats.log"

. $srcdir/diag.sh first-column-sum-check 's/.*new_metric_add=\([0-9]\+\)/\1/g' 'new_metric_add=' "${RSYSLOG_DYNNAME}.out.stats.log" 3
. $srcdir/diag.sh first-column-sum-check 's/.*ops_overflow=\([0-9]\+\)/\1/g' 'ops_overflow=' "${RSYSLOG_DYNNAME}.out.stats.log" 5
. $srcdir/diag.sh first-column-sum-check 's/.*no_metric=\([0-9]\+\)/\1/g' 'no_metric=' "${RSYSLOG_DYNNAME}.out.stats.log" 0

#ttl-expiry(2*ttl in worst case, ttl + delta in best) so metric-names reset should have happened
. $srcdir/diag.sh allow-single-stats-flush-after-block-and-wait-for-it
. $srcdir/diag.sh await-stats-flush-after-block

. $srcdir/diag.sh wait-for-stats-flush ${RSYSLOG_DYNNAME}.out.stats.log

. $srcdir/diag.sh first-column-sum-check 's/.*metrics_purged=\([0-9]\+\)/\1/g' 'metrics_purged=' "${RSYSLOG_DYNNAME}.out.stats.log" 3

rm ${RSYSLOG_DYNNAME}.out.stats.log
issue_HUP #reopen stats file
. $srcdir/diag.sh wait-for-stats-flush ${RSYSLOG_DYNNAME}.out.stats.log
. $srcdir/diag.sh block-stats-flush
. $srcdir/diag.sh injectmsg-litteral $srcdir/testsuites/dynstats_input_more_2
wait_queueempty
. $srcdir/diag.sh allow-single-stats-flush-after-block-and-wait-for-it

content_check "foo 001 0"
content_check "bar 002 0"
content_check "baz 003 0"
content_check "foo 004 0"
content_check "baz 005 0"
content_check "foo 006 0"
content_check "quux 007 -6"
content_check "corge 008 -6"
content_check "quux 009 -6"
content_check "foo 010 0"
content_check "corge 011 -6"
content_check "grault 012 -6"
content_check "foo 013 0"
content_check "corge 014 0"
content_check "grault 015 0"
content_check "quux 016 0"
content_check "foo 017 -6"
content_check "corge 018 0"

. $srcdir/diag.sh first-column-sum-check 's/.*corge=\([0-9]\+\)/\1/g' 'corge=' "${RSYSLOG_DYNNAME}.out.stats.log" 2
. $srcdir/diag.sh first-column-sum-check 's/.*grault=\([0-9]\+\)/\1/g' 'grault=' "${RSYSLOG_DYNNAME}.out.stats.log" 1
. $srcdir/diag.sh first-column-sum-check 's/.*quux=\([0-9]\+\)/\1/g' 'quux=' "${RSYSLOG_DYNNAME}.out.stats.log" 1

. $srcdir/diag.sh first-column-sum-check 's/.*new_metric_add=\([0-9]\+\)/\1/g' 'new_metric_add=' "${RSYSLOG_DYNNAME}.out.stats.log" 3
. $srcdir/diag.sh first-column-sum-check 's/.*ops_overflow=\([0-9]\+\)/\1/g' 'ops_overflow=' "${RSYSLOG_DYNNAME}.out.stats.log" 1
. $srcdir/diag.sh first-column-sum-check 's/.*no_metric=\([0-9]\+\)/\1/g' 'no_metric=' "${RSYSLOG_DYNNAME}.out.stats.log" 0

. $srcdir/diag.sh allow-single-stats-flush-after-block-and-wait-for-it
. $srcdir/diag.sh await-stats-flush-after-block

echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown

. $srcdir/diag.sh first-column-sum-check 's/.*metrics_purged=\([0-9]\+\)/\1/g' 'metrics_purged=' "${RSYSLOG_DYNNAME}.out.stats.log" 3

custom_assert_content_missing 'foo' "${RSYSLOG_DYNNAME}.out.stats.log"
exit_test
