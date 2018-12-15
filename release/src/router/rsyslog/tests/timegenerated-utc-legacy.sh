#!/bin/bash
# addd 2016-03-22 by RGerhards, released under ASL 2.0

# NOTE: faketime does NOT properly support subseconds,
# so we must ensure we do not use them. Actually, what we
# see is uninitialized data value in tv_usec, which goes
# away as soon as we do not run under faketime control.
# FOR THE SAME REASON, there is NO VALGRIND EQUIVALENT
# of this test, as valgrind would abort with reports
# of faketime.
. $srcdir/diag.sh init
. $srcdir/faketime_common.sh

export TZ=TEST+02:00

generate_conf
add_conf '
$ModLoad ../plugins/imtcp/.libs/imtcp
$InputTCPServerRun '$TCPFLOOD_PORT'

template(name="outfmt" type="string"
	 string="%timegenerated:::date-utc%\n")
:msg, contains, "msgnum:" action(type="omfile" template="outfmt"
			         file=`echo $RSYSLOG_OUT_LOG`)
'

echo "***SUBTEST: check 2016-03-01"
rm -f $RSYSLOG_OUT_LOG	# do cleanup of previous subtest
FAKETIME='2016-03-01 12:00:00' startup
tcpflood -m1
shutdown_when_empty
wait_shutdown
echo "Mar  1 14:00:00" | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid timestamps generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  exit 1
fi;

exit_test
