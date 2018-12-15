#!/bin/bash
# added 2015-11-17 by singh.janmejay
# This file is part of the rsyslog project, released under ASL 2.0

uname
if [ `uname` = "FreeBSD" ] ; then
   echo "This test currently does not work on FreeBSD."
   exit 77
fi

echo ===============================================================================
echo \[dynstats_nometric.sh\]: test for dyn-stats meta-metric behavior with zero-length metric name
. $srcdir/diag.sh init
generate_conf
add_conf '
ruleset(name="stats") {
  action(type="omfile" file="'${RSYSLOG_DYNNAME}'.out.stats.log")
}

module(load="../plugins/impstats/.libs/impstats" interval="1" severity="7" resetCounters="on" Ruleset="stats" bracketing="on")

template(name="outfmt" type="string" string="%msg% %$.increment_successful%\n")

dyn_stats(name="msg_stats")

set $.msg_prefix = field($msg, 32, 2);

set $.increment_successful = dyn_inc("msg_stats", $.msg_prefix);

action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup
. $srcdir/diag.sh wait-for-stats-flush ${RSYSLOG_DYNNAME}.out.stats.log
wait_queueempty
rm $srcdir/${RSYSLOG_DYNNAME}.out.stats.log
issue_HUP #reopen stats file
. $srcdir/diag.sh injectmsg-litteral $srcdir/testsuites/dynstats_empty_input
wait_queueempty
rst_msleep 1100 # wait for stats flush
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown
. $srcdir/diag.sh first-column-sum-check 's/.*no_metric=\([0-9]\+\)/\1/g' 'no_metric=' "${RSYSLOG_DYNNAME}.out.stats.log" 5
custom_assert_content_missing 'foo' "${RSYSLOG_DYNNAME}.out.stats.log"
custom_assert_content_missing 'bar' "${RSYSLOG_DYNNAME}.out.stats.log"
custom_assert_content_missing 'baz' "${RSYSLOG_DYNNAME}.out.stats.log"
custom_assert_content_missing 'corge' "${RSYSLOG_DYNNAME}.out.stats.log"
custom_content_check 'quux=1' "${RSYSLOG_DYNNAME}.out.stats.log"
custom_content_check 'grault=1' "${RSYSLOG_DYNNAME}.out.stats.log"
exit_test
