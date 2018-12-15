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
module(load="../plugins/imjournal/.libs/imjournal" StateFile="imjournal.state"
	RateLimit.Burst="1000000")

template(name="outfmt" type="string" string="%msg%\n")
action(type="omfile" template="outfmt" file=`echo $RSYSLOG_OUT_LOG`)
'
TESTMSG="TestBenCH-RSYSLog imjournal This is a test message - $(date +%s)"
./journal_print "$TESTMSG"
if [ $? -ne 0 ]; then
        echo "SKIP: failed to put test into journal."
        exit 77
fi
journalctl -an 200 | fgrep -qF "$TESTMSG"
if [ $? -ne 0 ]; then
        echo "SKIP: cannot read journal."
        exit 77
fi
# do first run to process all the stuff already in journal db
startup
./msleep 500
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown
#now do a second which should NOT capture testmsg again
startup
./msleep 500
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown
COUNT= cat $RSYSLOG_OUT_LOG | fgrep "$TESTMSG" | wc -l
if [ $COUNT -ne 1 ]; then
  echo "FAIL: message found $COUNT times (expected 1)"
  echo " $RSYSLOG_OUT_LOG content (tail -n200):"
  tail -n200 $RSYSLOG_OUT_LOG
  echo "======="
  echo "last entries from journal:"
  journalctl -an 200
  echo "======="
  echo "NOTE: last 200 lines may be insufficient on busy systems!"
  echo "======="
  echo "FAIL: imjournal test message could not be found!"
  echo "Expected message content was:"
  echo "$TESTMSG"
  error_exit 1
fi;
exit_test
