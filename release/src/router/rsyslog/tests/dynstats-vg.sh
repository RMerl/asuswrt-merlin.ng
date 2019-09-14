#!/bin/bash
# added 2015-11-13 by singh.janmejay
# This file is part of the rsyslog project, released under ASL 2.0

uname
if [ `uname` = "FreeBSD" ] ; then
   echo "This test currently does not work on FreeBSD."
   exit 77
fi

echo ===============================================================================
echo \[dynstats-vg.sh\]: test for gathering stats over dynamic metric names with valgrind
. $srcdir/diag.sh init
generate_conf
add_conf '
ruleset(name="stats") {
  action(type="omfile" file="'${RSYSLOG_DYNNAME}'.out.stats.log")
}

module(load="../plugins/impstats/.libs/impstats" interval="1" severity="7" resetCounters="on" Ruleset="stats" bracketing="on")

template(name="outfmt" type="string" string="%msg% %$.increment_successful%\n")

dyn_stats(name="msg_stats")

set $.msg_prefix = field($msg, 32, 1);

set $.increment_successful = dyn_inc("msg_stats", $.msg_prefix);

action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup_vg
. $srcdir/diag.sh wait-for-stats-flush ${RSYSLOG_DYNNAME}.out.stats.log
. $srcdir/diag.sh injectmsg-litteral $srcdir/testsuites/dynstats_input
wait_queueempty
content_check "foo 001 0"
content_check "bar 002 0"
content_check "baz 003 0"
content_check "foo 004 0"
content_check "baz 005 0"
content_check "foo 006 0"
rst_msleep 1100 # wait for stats flush
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown_vg
check_exit_vg
custom_content_check 'bar=1' "${RSYSLOG_DYNNAME}.out.stats.log"
. $srcdir/diag.sh first-column-sum-check 's/.*foo=\([0-9]\+\)/\1/g' 'foo=' "${RSYSLOG_DYNNAME}.out.stats.log" 3
. $srcdir/diag.sh first-column-sum-check 's/.*bar=\([0-9]\+\)/\1/g' 'bar=' "${RSYSLOG_DYNNAME}.out.stats.log" 1
. $srcdir/diag.sh first-column-sum-check 's/.*baz=\([0-9]\+\)/\1/g' 'baz=' "${RSYSLOG_DYNNAME}.out.stats.log" 2
exit_test
