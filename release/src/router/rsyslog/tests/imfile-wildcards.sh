#!/bin/bash
# This is part of the rsyslog testbench, licensed under GPLv3
export IMFILEINPUTFILES="10"
. $srcdir/diag.sh init
. $srcdir/diag.sh check-inotify
generate_conf
add_conf '
# comment out if you need more debug info:
	global( debug.whitelist="on"
		debug.files=["imfile.c"])

module(load="../plugins/imfile/.libs/imfile"
       mode="inotify" normalizePath="off")

input(type="imfile" File="./'$RSYSLOG_DYNNAME'.input.*.log" Tag="file:"
	Severity="error" Facility="local7" addMetadata="on")

input(type="imfile" File="/does/not/exist/*.log" Tag="file:"
	Severity="error" Facility="local7" addMetadata="on")

template(name="outfmt" type="list") {
	constant(value="HEADER ")
	property(name="msg" format="json")
	constant(value=", filename: ")
	property(name="$!metadata!filename")
	constant(value=", fileoffset: ")
	property(name="$!metadata!fileoffset")
	constant(value="\n")
}

if $msg contains "msgnum:" then
	action( type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
# generate input files first. Note that rsyslog processes it as
# soon as it start up (so the file should exist at that point).

imfilebefore=$RSYSLOG_DYNNAME.input.1.log
./inputfilegen -m 1 > $imfilebefore

# Start rsyslog now before adding more files
startup

for i in `seq 2 $IMFILEINPUTFILES`;
do
	cp $imfilebefore $RSYSLOG_DYNNAME.input.$i.log
	imfilebefore=$RSYSLOG_DYNNAME.input.$i.log
	# Wait little for correct timing
	./msleep 50
done
./inputfilegen -m 3 > $RSYSLOG_DYNNAME.input.$((IMFILEINPUTFILES + 1)).log
ls -l $RSYSLOG_DYNNAME.input.*

shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown	# we need to wait until rsyslogd is finished!

printf 'HEADER msgnum:00000000:, filename: ./'$RSYSLOG_DYNNAME'.input.1.log, fileoffset: 0
HEADER msgnum:00000000:, filename: ./'$RSYSLOG_DYNNAME'.input.2.log, fileoffset: 0
HEADER msgnum:00000000:, filename: ./'$RSYSLOG_DYNNAME'.input.3.log, fileoffset: 0
HEADER msgnum:00000000:, filename: ./'$RSYSLOG_DYNNAME'.input.4.log, fileoffset: 0
HEADER msgnum:00000000:, filename: ./'$RSYSLOG_DYNNAME'.input.5.log, fileoffset: 0
HEADER msgnum:00000000:, filename: ./'$RSYSLOG_DYNNAME'.input.6.log, fileoffset: 0
HEADER msgnum:00000000:, filename: ./'$RSYSLOG_DYNNAME'.input.7.log, fileoffset: 0
HEADER msgnum:00000000:, filename: ./'$RSYSLOG_DYNNAME'.input.8.log, fileoffset: 0
HEADER msgnum:00000000:, filename: ./'$RSYSLOG_DYNNAME'.input.9.log, fileoffset: 0
HEADER msgnum:00000000:, filename: ./'$RSYSLOG_DYNNAME'.input.10.log, fileoffset: 0
HEADER msgnum:00000000:, filename: ./'$RSYSLOG_DYNNAME'.input.11.log, fileoffset: 0
HEADER msgnum:00000001:, filename: ./'$RSYSLOG_DYNNAME'.input.11.log, fileoffset: 17
HEADER msgnum:00000002:, filename: ./'$RSYSLOG_DYNNAME'.input.11.log, fileoffset: 34\n' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid output generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  exit 1
fi;

exit_test
