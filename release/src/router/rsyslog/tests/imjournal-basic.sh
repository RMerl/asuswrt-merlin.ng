#!/bin/bash
# This test injects a message and checks if it is received by
# imjournal. We use a special test string which we do not expect
# to be present in the regular log stream. So we do not expect that
# any other journal content matches our test message. We skip the 
# test in case message does not make it even to journal which may 
# sometimes happen in some environments.
# addd 2017-10-25 by RGerhards, released under ASL 2.0

. $srcdir/diag.sh init
. $srcdir/diag.sh require-journalctl
generate_conf
add_conf '
module(load="../plugins/imjournal/.libs/imjournal" IgnorePreviousMessages="on"
	RateLimit.Burst="1000000")

template(name="outfmt" type="string" string="%msg%\n")
action(type="omfile" template="outfmt" file="'$RSYSLOG_OUT_LOG'")
'
startup
set -x
TESTMSG="TestBenCH-RSYSLog imjournal This is a test message - $(date +%s) - $RSYSLOG_DYNNAME"

./journal_print "$TESTMSG"
journal_write_state=$?
echo "state: $journal_write_state writing: $TESTMSG"
set +x

# we must not terminate the test until we have terminated rsyslog
./msleep 500
echo after sleep, shutting down rsyslog
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown

if [ $journal_write_state -ne 0 ]; then
	echo "SKIP: failed to put test into journal."
	exit 77
fi
journalctl -a | fgrep -qF "$TESTMSG"
if [ $? -ne 0 ]; then
	echo "SKIP: cannot read journal."
	exit 77
fi

echo "INFO: $(wc -l < $RSYSLOG_OUT_LOG) lines in $RSYSLOG_OUT_LOG"

fgrep -qF "$TESTMSG" < $RSYSLOG_OUT_LOG 
if [ $? -ne 0 ]; then
  echo "FAIL:  $RSYSLOG_OUT_LOG content (tail -n200):"
  tail -n200 $RSYSLOG_OUT_LOG
  echo "======="
  echo "searching journal for testbench messages:"
  journalctl -a | grep -qf "TestBenCH-RSYSLog imjournal"
  echo "======="
  echo "NOTE: showing only last 200 lines, may be insufficient on busy systems!"
  echo "last entries from journal:"
  journalctl -an 200
  echo "======="
  echo "NOTE: showing only last 200 lines, may be insufficient on busy systems!"
  echo "======="
  echo "FAIL: imjournal test message could not be found!"
  echo "Expected message content was:"
  echo "$TESTMSG"
  error_exit 1
fi;
exit_test
