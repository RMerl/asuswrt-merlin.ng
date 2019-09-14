#!/bin/bash
# This is part of the rsyslog testbench, licensed under GPLv3
. $srcdir/diag.sh init
. $srcdir/diag.sh check-inotify-only
generate_conf
add_conf '
global(workDirectory="'${RSYSLOG_DYNNAME}'.spool")
module(load="../plugins/imfile/.libs/imfile" mode="inotify" PollingInterval="1")

/* Filter out busy debug output */
global( debug.whitelist="off"
	debug.files=["rainerscript.c", "ratelimit.c", "ruleset.c", "main Q", "msg.c", "../action.c"])

input(type="imfile" File="./'$RSYSLOG_DYNNAME'.input"
	Tag="file:" Severity="error" Facility="local7" addMetadata="on")

template(name="outfmt" type="string" string="%msg:F,58:2%\n")
if $msg contains "msgnum:" then
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'

# generate small input file - state file must be inode only
./inputfilegen -m 1 > $RSYSLOG_DYNNAME.input
ls -li $RSYSLOG_DYNNAME.input

echo "STEP 1 - small input"
startup
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown	# we need to wait until rsyslogd is finished!

echo "STEP 2 - still small input"
# add a bit to input file, but state file must still be inode only
./inputfilegen -m 1 -i1 >> $RSYSLOG_DYNNAME.input
ls -li $RSYSLOG_DYNNAME.input*
if [ $(ls ${RSYSLOG_DYNNAME}.spool/* | wc -l) -ne 1 ]; then
	echo FAIL: more than one state file in work directory:
	ls -l ${RSYSLOG_DYNNAME}.spool
	error_exit 1
fi

startup
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown	# we need to wait until rsyslogd is finished!

echo "STEP 3 - larger input, hash shall be used"
./inputfilegen -m 998 -i 2 >> $RSYSLOG_DYNNAME.input
ls -li $RSYSLOG_DYNNAME.input*
echo ls ${RSYSLOG_DYNNAME}.spool:
ls -l ${RSYSLOG_DYNNAME}.spool

startup
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown	# we need to wait until rsyslogd is finished!

if [ $(ls ${RSYSLOG_DYNNAME}.spool/* | wc -l) -ne 1 ]; then
	echo FAIL: more than one state file in work directory:
	ls -l ${RSYSLOG_DYNNAME}.spool
	error_exit 1
fi


echo "STEP 4 - append to larger input, hash state file must now be found"
./inputfilegen -m 1000 -i 1000 >> $RSYSLOG_DYNNAME.input
ls -li $RSYSLOG_DYNNAME.input*
echo ls ${RSYSLOG_DYNNAME}.spool:
ls -l ${RSYSLOG_DYNNAME}.spool

startup
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown	# we need to wait until rsyslogd is finished!

if [ $(ls ${RSYSLOG_DYNNAME}.spool/* | wc -l) -ne 1 ]; then
	echo FAIL: more than one state file in work directory:
	ls -l ${RSYSLOG_DYNNAME}.spool
	error_exit 1
fi

seq_check 0 1999
exit_test
