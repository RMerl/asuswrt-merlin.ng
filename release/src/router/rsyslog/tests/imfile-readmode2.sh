#!/bin/bash
# This is part of the rsyslog testbench, licensed under ASL 2.0
#export RSYSLOG_DEBUG="debug nologfuncflow noprintmutexaction stdout"
#export RSYSLOG_DEBUGLOG="log"

. $srcdir/diag.sh init
. $srcdir/diag.sh check-inotify
generate_conf
add_conf '
global( debug.whitelist="on"
	debug.files=["imfile.c"])

module(load="../plugins/imfile/.libs/imfile")

input(type="imfile" File="./'$RSYSLOG_DYNNAME'.input" Tag="file:" ReadMode="2")
#input(type="imfile" File="./'$RSYSLOG_DYNNAME'.input.*" Tag="file:" ReadMode="2")

template(name="outfmt" type="list") {
  constant(value="HEADER ")
  property(name="msg" format="json")
  constant(value="\n")
}

if $msg contains "msgnum:" then
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'

# write the beginning of the file
echo 'msgnum:0
 msgnum:1' > $RSYSLOG_DYNNAME.input
echo 'msgnum:2' >> $RSYSLOG_DYNNAME.input

startup
# sleep a little to give rsyslog a chance to begin processing
rst_msleep 500

# write some more lines (see https://github.com/rsyslog/rsyslog/issues/144)
echo 'msgnum:3
 msgnum:4' >> $RSYSLOG_DYNNAME.input
echo 'msgnum:5' >> $RSYSLOG_DYNNAME.input # this one shouldn't be written to the output file because of ReadMode 2

# give it time to finish

shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown    # we need to wait until rsyslogd is finished!

## check if we have the correct number of messages
NUMLINES=$(grep -c HEADER $RSYSLOG_OUT_LOG 2>/dev/null)
if [ -z $NUMLINES ]; then
  echo "ERROR: expecting at least a match for HEADER, maybe  $RSYSLOG_OUT_LOG wasn't even written?"
  cat $RSYSLOG_OUT_LOG
  error_exit 1
else
  if [ ! $NUMLINES -eq 3 ]; then
    echo "ERROR: expecting 3 headers, got $NUMLINES"
    cat $RSYSLOG_OUT_LOG
    error_exit 1
  fi
fi

## check if all the data we expect to get in the file is there

for i in {1..4}; do
  grep msgnum:$i $RSYSLOG_OUT_LOG > /dev/null 2>&1
  if [ ! $? -eq 0 ]; then
    echo "ERROR: expecting the string 'msgnum:$i', it's not there"
    cat $RSYSLOG_OUT_LOG
    error_exit 1
  fi
done

## if we got here, all is good :)

exit_test
