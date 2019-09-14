#!/bin/bash
# this test checks that old (v1, pre 8.34.0) imfile state files are
# properly read in. It is based on imfile-readmode2-with-persists.sh,
# where the first part before the shutdown is removed, and an old state
# file is populated. Note that in contrast to the original test the
# initial set of lines from the input file is missing - this is
# exactly what shall happen.
# This is part of the rsyslog testbench, licensed under ASL 2.0
# added 2018-03-29 by rgerhards
. $srcdir/diag.sh init
. $srcdir/diag.sh check-inotify
generate_conf
add_conf '
global(workDirectory="'${RSYSLOG_DYNNAME}'.spool")
module(load="../plugins/imfile/.libs/imfile")

input(type="imfile"
      File="./'$RSYSLOG_DYNNAME'.input"
      Tag="file:"
      ReadMode="2")

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

# do mock-up setup
echo 'msgnum:0
 msgnum:1' > $RSYSLOG_DYNNAME.input
echo 'msgnum:2' >> $RSYSLOG_DYNNAME.input

# we need to patch the state file to match the current inode number
inode=$(ls -i $RSYSLOG_DYNNAME.input|awk '{print $1}')
leninode=${#inode}
newline="+inode:2:${leninode}:${inode}:"

sed s/+inode:2:7:4464465:/${newline}/ <$srcdir/testsuites/imfile-old-state-file_imfile-state_.-rsyslog.input > ${RSYSLOG_DYNNAME}.spool/imfile-state\:.-$RSYSLOG_DYNNAME.input
printf "info: new input file: $(ls -i $RSYSLOG_DYNNAME.input)\n"
printf "info: new inode line: ${newline}\n"
printf "info: patched state file:\n"
cat ${RSYSLOG_DYNNAME}.spool/imfile-state\:.-$RSYSLOG_DYNNAME.input

startup

echo 'msgnum:3
 msgnum:4' >> $RSYSLOG_DYNNAME.input
echo 'msgnum:5' >> $RSYSLOG_DYNNAME.input

shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown    # we need to wait until rsyslogd is finished!

NUMLINES=$(grep -c HEADER  $RSYSLOG_OUT_LOG 2>/dev/null)

if [ -z $NUMLINES ]; then
  echo "ERROR: expecting at least a match for HEADER, maybe  $RSYSLOG_OUT_LOG wasn't even written?"
  cat $RSYSLOG_OUT_LOG
  error_exit 1
else
  # note: we expect only 2 headers as the first file part if NOT processed!
  if [ ! $NUMLINES -eq 2 ]; then
    echo "ERROR: expecting 2 headers, got $NUMLINES"
    cat $RSYSLOG_OUT_LOG
    error_exit 1
  fi
fi

## check if all the data we expect to get in the file is there

for i in {2..4}; do
  grep msgnum:$i  $RSYSLOG_OUT_LOG > /dev/null 2>&1
  if [ ! $? -eq 0 ]; then
    echo "ERROR: expecting the string 'msgnum:$i', it's not there"
    cat $RSYSLOG_OUT_LOG
    error_exit 1
  fi
done

exit_test
