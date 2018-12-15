#!/bin/bash
# This is part of the rsyslog testbench, licensed under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imfile/.libs/imfile" mode="polling" pollingInterval="1")

input(type="imfile"
      File="./'$RSYSLOG_DYNNAME'.input"
      Tag="file:"
      PersistStateInterval="1"
      readTimeout="3"
      startmsg.regex="^[^ ]")
template(name="outfmt" type="list") {
  constant(value="HEADER ")
  property(name="msg" format="json")
  constant(value="\n")
}
if $msg contains "msgnum:" then
 action(
   type="omfile"
   file=`echo $RSYSLOG_OUT_LOG`
   template="outfmt"
 )
'
startup

# we need to sleep a bit between writes to give imfile a chance
# to pick up the data (IN MULTIPLE ITERATIONS!)
echo 'msgnum:0
 msgnum:1' > $RSYSLOG_DYNNAME.input
./msleep 8000
echo ' msgnum:2
 msgnum:3' >> $RSYSLOG_DYNNAME.input

# we now do a stop and restart of rsyslog. This checks that everything
# works across restarts.
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown    # we need to wait until rsyslogd is finished!

#echo DROPPING YOU TO BASH!
#bash

startup

# new data
echo ' msgnum:4' >> $RSYSLOG_DYNNAME.input
./msleep 8000
echo ' msgnum:5
 msgnum:6' >> $RSYSLOG_DYNNAME.input
./msleep 8000

# the next line terminates our test. It is NOT written to the output file,
# as imfile waits whether or not there is a follow-up line that it needs
# to combine.
#echo 'END OF TEST' >> $RSYSLOG_DYNNAME.input
#./msleep 2000

shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown    # we need to wait until rsyslogd is finished!

printf 'HEADER msgnum:0\\\\n msgnum:1
HEADER  msgnum:2\\\\n msgnum:3\\\\n msgnum:4
HEADER  msgnum:5\\\\n msgnum:6\n' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid multiline message generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit 1
fi;

exit_test
