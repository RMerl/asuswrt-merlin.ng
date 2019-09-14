#!/bin/bash
# addd 2016-10-06 by RGerhards, released under ASL 2.0
. $srcdir/diag.sh init
. $srcdir/diag.sh check-inotify
generate_conf
add_conf '
module(load="../plugins/imfile/.libs/imfile")

input(type="imfile" File="./'$RSYSLOG_DYNNAME'.input" Tag="file:" reopenOnTruncate="on")

template(name="outfmt" type="string" string="%msg:F,58:2%\n")
if $msg contains "msgnum:" then
	action( type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'

# write the beginning of the file
echo 'msgnum:0
msgnum:1' > $RSYSLOG_DYNNAME.input

startup

wait_queueempty # wait for message to be processed

# truncate and write some more lines (see https://github.com/rsyslog/rsyslog/issues/1090)
echo 'msgnum:2' > $RSYSLOG_DYNNAME.input
wait_queueempty # wait for message to be processed

echo 'msgnum:3
msgnum:4' >> $RSYSLOG_DYNNAME.input

shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown

seq_check 0 4
exit_test
