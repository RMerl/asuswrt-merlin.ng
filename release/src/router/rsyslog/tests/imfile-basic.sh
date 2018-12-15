#!/bin/bash
# This is part of the rsyslog testbench, licensed under GPLv3
echo [imfile-basic.sh]
. $srcdir/diag.sh init
generate_conf
add_conf '
$ModLoad ../plugins/imfile/.libs/imfile
$InputFileName ./'$RSYSLOG_DYNNAME'.input
$InputFileTag file:
$InputFileStateFile stat-file1
$InputFileSeverity error
$InputFileFacility local7
$InputFileMaxLinesAtOnce 100000
$InputRunFileMonitor

$template outfmt,"%msg:F,58:2%\n"
:msg, contains, "msgnum:" action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
# generate input file first. Note that rsyslog processes it as
# soon as it start up (so the file should exist at that point).
./inputfilegen -m 50000 > $RSYSLOG_DYNNAME.input
ls -l $RSYSLOG_DYNNAME.input
startup
# sleep a little to give rsyslog a chance to begin processing
sleep 1
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown	# we need to wait until rsyslogd is finished!
seq_check 0 49999
exit_test
