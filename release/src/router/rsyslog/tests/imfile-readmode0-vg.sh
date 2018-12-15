#!/bin/bash
# Tests for processing of partial lines in read mode 0
# This is part of the rsyslog testbench, licensed under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imfile/.libs/imfile")
input(type="imfile" File="./'$RSYSLOG_DYNNAME'.input" Tag="file:" ReadMode="0")

template(name="outfmt" type="list") {
	constant(value="HEADER ")
	property(name="msg" format="json")
	constant(value="\n")
}

if $msg contains "msgnum:" then
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup_vg

printf 'msgnum:0
 msgnum:1' > $RSYSLOG_DYNNAME.input
printf '\nmsgnum:2' >> $RSYSLOG_DYNNAME.input

# sleep a little to give rsyslog a chance to process unterminated linet 
./msleep 500

# write some more lines (see https://github.com/rsyslog/rsyslog/issues/144)
printf 'msgnum:3
 msgnum:4' >> $RSYSLOG_DYNNAME.input
printf '\nmsgnum:5' >> $RSYSLOG_DYNNAME.input # this one shouldn't be written to the output file because of missing LF

shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown_vg    # we need to wait until rsyslogd is finished!
check_exit_vg

## check if we have the correct number of messages
NUMLINES=$(grep -c HEADER  $RSYSLOG_OUT_LOG 2>/dev/null)

if [ -z $NUMLINES ]; then
  echo "ERROR: expecting at least a match for HEADER, maybe  $RSYSLOG_OUT_LOG wasn't even written?"
  cat $RSYSLOG_OUT_LOG
  error_exit 1
else
  if [ ! $NUMLINES -eq 4 ]; then
    echo "ERROR: expecting 4 headers, got $NUMLINES"
    cat $RSYSLOG_OUT_LOG
    error_exit 1
  fi
fi

## check if all the data we expect to get in the file is there
for i in {1..4}; do
  grep msgnum:$i  $RSYSLOG_OUT_LOG > /dev/null 2>&1
  if [ ! $? -eq 0 ]; then
    echo "ERROR: expecting the string 'msgnum:$i', it's not there"
    cat $RSYSLOG_OUT_LOG
    error_exit 1
  fi
done


exit_test
