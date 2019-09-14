#!/bin/bash
# This is part of the rsyslog testbench, licensed under ASL 2.0
# This test mimics the test imfile-readmode2.sh, but works via
# endmsg.regex. It's kind of a base test for the regex functionality.
echo ======================================================================
# Check if inotify header exist
echo [imfile-discard-truncated-line.sh]
. $srcdir/diag.sh check-inotify
. $srcdir/diag.sh init
generate_conf
add_conf '
$MaxMessageSize 128
module(load="../plugins/imfile/.libs/imfile")
input(type="imfile"
      File="./'$RSYSLOG_DYNNAME'.input"
      discardTruncatedMsg="on"
      msgDiscardingError="off"
      Tag="file:"
      startmsg.regex="^[^ ]"
      ruleset="ruleset")
template(name="outfmt" type="list") {
  constant(value="HEADER ")
  property(name="msg" format="json")
  constant(value="\n")
}
ruleset(name="ruleset") {
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
}
action(type="omfile" file=`echo $RSYSLOG2_OUT_LOG` template="outfmt")
'
startup

# write the beginning of the file
echo 'msgnum:0
msgnum:1
msgnum:2 aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
 msgnum:3 bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb
 msgnum:4 cccccccccccccccccccccccccccccccccccccccccccc
 msgnum:5 dddddddddddddddddddddddddddddddddddddddddddd
msgnum:6 eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
 msgnum:7 ffffffffffffffffffffffffffffffffffffffffffff
 msgnum:8 gggggggggggggggggggggggggggggggggggggggggggg
msgnum:9' > $RSYSLOG_DYNNAME.input
# the next line terminates our test. It is NOT written to the output file,
# as imfile waits whether or not there is a follow-up line that it needs
# to combine.
echo 'END OF TEST' >> $RSYSLOG_DYNNAME.input
# sleep a little to give rsyslog a chance to begin processing
./msleep 500
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown    # we need to wait until rsyslogd is finished!

printf 'HEADER msgnum:0
HEADER msgnum:1
HEADER msgnum:2 aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\\\\n msgnum:3 bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\\\\n msgnum:4 ccccccc
HEADER msgnum:6 eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\\\\n msgnum:7 ffffffffffffffffffffffffffffffffffffffffffff\\\\n msgnum:8 ggggggg
HEADER msgnum:9\n' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid multiline message generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  exit 1
fi;

grep "imfile error:.*rest of message will not be processed" ${RSYSLOG2_OUT_LOG} > /dev/null
if [ $? -eq 0 ]; then
        echo
        echo "FAIL: expected error message from missing input file not found. ${RSYSLOG2_OUT_LOG} is:"
        cat ${RSYSLOG2_OUT_LOG}
        error_exit 1
fi

exit_test
