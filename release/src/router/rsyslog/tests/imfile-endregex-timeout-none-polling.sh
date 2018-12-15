#!/bin/bash
# This is part of the rsyslog testbench, licensed under ASL 2.0
echo ======================================================================
if [ `uname` = "SunOS" ] ; then
   echo "Solaris: FIX ME"
   exit 77
fi

. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imfile/.libs/imfile"
       mode="polling"
       pollingInterval="2"
      )
input(type="imfile"
      File="./'$RSYSLOG_DYNNAME'.input"
      Tag="file:"
      PersistStateInterval="1"
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
./msleep 5000 # wait 5 seconds - this shall cause a timeout
echo ' msgnum:2
 msgnum:3' >> $RSYSLOG_DYNNAME.input
# the next line terminates our test. It is NOT written to the output file,
# as imfile waits whether or not there is a follow-up line that it needs
# to combine.
echo 'END OF TEST' >> $RSYSLOG_DYNNAME.input
./msleep 200

shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown    # we need to wait until rsyslogd is finished!

echo 'HEADER msgnum:0\\n msgnum:1\\n msgnum:2\\n msgnum:3' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid multiline message generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  exit 1
fi;

exit_test
