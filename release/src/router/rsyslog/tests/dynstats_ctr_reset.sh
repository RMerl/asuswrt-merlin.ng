#!/bin/bash
# added 2015-11-16 by singh.janmejay
# This file is part of the rsyslog project, released under ASL 2.0

uname
if [ `uname` = "FreeBSD" ] ; then
   echo "This test currently does not work on FreeBSD."
   exit 77
fi

echo ===============================================================================
echo \[dynstats_ctr_reset.sh\]: test to ensure correctness of stats-ctr reset
. $srcdir/diag.sh init
generate_conf
add_conf '
ruleset(name="stats") {
  action(type="omfile" file="'${RSYSLOG_DYNNAME}'.out.stats.log")
}

module(load="../plugins/impstats/.libs/impstats" interval="1" severity="7" resetCounters="on" Ruleset="stats" bracketing="on")

template(name="outfmt" type="string" string="%msg%\n")

dyn_stats(name="msg_stats_resettable_on" resettable="on")
dyn_stats(name="msg_stats_resettable_off" resettable="off")
dyn_stats(name="msg_stats_resettable_default")

set $.msg_prefix = field($msg, 32, 1);

set $.x = dyn_inc("msg_stats_resettable_on", $.msg_prefix);
set $.y = dyn_inc("msg_stats_resettable_off", $.msg_prefix);
set $.z = dyn_inc("msg_stats_resettable_default", $.msg_prefix);

action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup
. $srcdir/diag.sh injectmsg-litteral $srcdir/testsuites/dynstats_input_1
. $srcdir/diag.sh injectmsg-litteral $srcdir/testsuites/dynstats_input_2
wait_queueempty
sleep 1
. $srcdir/diag.sh injectmsg-litteral $srcdir/testsuites/dynstats_input_3
wait_queueempty
sleep 1
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown
content_check "foo 006"
custom_content_check 'bar=1' "${RSYSLOG_DYNNAME}.out.stats.log"
. $srcdir/diag.sh first-column-sum-check 's/.*foo=\([0-9]\+\)/\1/g' 'msg_stats_resettable_on.\+foo=' "${RSYSLOG_DYNNAME}.out.stats.log" 3
. $srcdir/diag.sh first-column-sum-check 's/.*bar=\([0-9]\+\)/\1/g' 'msg_stats_resettable_on.\+bar=' "${RSYSLOG_DYNNAME}.out.stats.log" 1
. $srcdir/diag.sh first-column-sum-check 's/.*baz=\([0-9]\+\)/\1/g' 'msg_stats_resettable_on.\+baz=' "${RSYSLOG_DYNNAME}.out.stats.log" 2
. $srcdir/diag.sh assert-first-column-sum-greater-than 's/.*foo=\([0-9]\+\)/\1/g' 'msg_stats_resettable_off.\+foo=' "${RSYSLOG_DYNNAME}.out.stats.log" 3
. $srcdir/diag.sh assert-first-column-sum-greater-than 's/.*bar=\([0-9]\+\)/\1/g' 'msg_stats_resettable_off.\+bar=' "${RSYSLOG_DYNNAME}.out.stats.log" 1
. $srcdir/diag.sh assert-first-column-sum-greater-than 's/.*baz=\([0-9]\+\)/\1/g' 'msg_stats_resettable_off.\+baz=' "${RSYSLOG_DYNNAME}.out.stats.log" 2
. $srcdir/diag.sh first-column-sum-check 's/.*foo=\([0-9]\+\)/\1/g' 'msg_stats_resettable_default.\+foo=' "${RSYSLOG_DYNNAME}.out.stats.log" 3
. $srcdir/diag.sh first-column-sum-check 's/.*bar=\([0-9]\+\)/\1/g' 'msg_stats_resettable_default.\+bar=' "${RSYSLOG_DYNNAME}.out.stats.log" 1
. $srcdir/diag.sh first-column-sum-check 's/.*baz=\([0-9]\+\)/\1/g' 'msg_stats_resettable_default.\+baz=' "${RSYSLOG_DYNNAME}.out.stats.log" 2
exit_test
