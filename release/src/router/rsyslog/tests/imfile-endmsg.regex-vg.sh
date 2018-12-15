#!/bin/bash
# This is part of the rsyslog testbench, licensed under ASL 2.0
# This test tests imfile endmsg.regex.
USE_VALGRIND=true
echo ======================================================================
echo [imfile-endmsg.regex.sh]
. $srcdir/diag.sh check-inotify
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imfile/.libs/imfile")

input(type="imfile"
      File="./'$RSYSLOG_DYNNAME'.crio.input"
      Tag="file:" addMetadata="on" escapelf="on"
      endmsg.regex="^[^ ]+ (stdout|stderr) F ")

template(name="outfmt" type="list") {
  property(name="$!all-json-plain")
  constant(value=" ")
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
if [ "x${USE_VALGRIND:-false}" == "xtrue" ] ; then
	startup_vg
else
	startup
fi
if [ -n "${USE_GDB:-}" ] ; then
	echo attach gdb here
	sleep 54321 || :
fi

# write the beginning of the file
echo 'date stdout P msgnum:0' > $RSYSLOG_DYNNAME.crio.input
echo 'date stdout F msgnum:1' >> $RSYSLOG_DYNNAME.crio.input
echo 'date stdout F msgnum:2' >> $RSYSLOG_DYNNAME.crio.input

# sleep a little to give rsyslog a chance to begin processing
if [ -n "${USE_GDB:-}" ] ; then
	sleep 54321 || :
else
	sleep 1
fi

echo 'date stdout P msgnum:3' >> $RSYSLOG_DYNNAME.crio.input
echo 'date stdout P msgnum:4' >> $RSYSLOG_DYNNAME.crio.input
echo 'date stdout P msgnum:5' >> $RSYSLOG_DYNNAME.crio.input

# give it time to finish
if [ -n "${USE_GDB:-}" ] ; then
	sleep 54321 || :
else
	sleep 1
fi

echo 'date stdout F msgnum:6' >> $RSYSLOG_DYNNAME.crio.input
echo 'date stdout P msgnum:7' >> $RSYSLOG_DYNNAME.crio.input

shutdown_when_empty # shut down rsyslogd when done processing messages
if [ "x${USE_VALGRIND:-false}" == "xtrue" ] ; then
	wait_shutdown_vg
	check_exit_vg
else
	wait_shutdown    # we need to wait until rsyslogd is finished!
fi

# give it time to write the output file
sleep 1

## check if we have the correct number of messages

NUMLINES=$(wc -l $RSYSLOG_OUT_LOG | awk '{print $1}' 2>/dev/null)

rc=0
if [ ! $NUMLINES -eq 3 ]; then
  echo "ERROR: expecting 3 lines, got $NUMLINES"
  rc=1
fi

## check if all the data we expect to get in the file is there

for string in "metadata.*filename.*crio[.]input.*msgnum:0.*msgnum:1" \
	"metadata.*filename.*crio[.]input.*msgnum:2" \
	"metadata.*filename.*crio[.]input.*msgnum:3.*msgnum:4.*msgnum:5.*msgnum:6" ; do
	if grep "$string" $RSYSLOG_OUT_LOG > /dev/null 2>&1 ; then
		: # ok
	else
		echo "Error: the expected string '$string' not found"
		rc=1
	fi
done

if [ $rc -ne 0 ]; then
    cat $RSYSLOG_OUT_LOG
    exit 1
fi

## if we got here, all is good :)

exit_test
