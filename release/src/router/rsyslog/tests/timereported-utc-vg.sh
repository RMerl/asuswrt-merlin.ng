#!/bin/bash
# addd 2016-03-22 by RGerhards, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
$ModLoad ../plugins/imtcp/.libs/imtcp
$InputTCPServerRun '$TCPFLOOD_PORT'

template(name="outfmt" type="string"
	 string="%timereported:::date-rfc3339,date-utc%\n")
:msg, contains, "msgnum:" action(type="omfile" template="outfmt"
			         file=`echo $RSYSLOG_OUT_LOG`)
'

echo "*** SUBTEST 2003 ****"
rm -f $RSYSLOG_OUT_LOG	# do cleanup of previous subtest
startup_vg
tcpflood -m1 -M"\"<165>1 2003-08-24T05:14:15.000003-07:00 192.0.2.1 tcpflood 8710 - - msgnum:0000000\""
shutdown_when_empty
wait_shutdown_vg
echo "2003-08-24T12:14:15.000003+00:00" | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid timestamps generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  exit 1
fi;

echo "*** SUBTEST 2016 ****"
rm -f $RSYSLOG_OUT_LOG	# do cleanup of previous subtest
startup_vg
tcpflood -m1 -M"\"<165>1 2016-03-01T12:00:00-02:00 192.0.2.1 tcpflood 8710 - - msgnum:0000000\""
shutdown_when_empty
wait_shutdown_vg
echo "2016-03-01T14:00:00.000000+00:00" | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid timestamps generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  exit 1
fi;

echo "*** SUBTEST 2016 (already in UTC) ****"
rm -f $RSYSLOG_OUT_LOG	# do cleanup of previous subtest
startup_vg
tcpflood -m1 -M"\"<165>1 2016-03-01T12:00:00Z 192.0.2.1 tcpflood 8710 - - msgnum:0000000\""
shutdown_when_empty
wait_shutdown_vg
echo "2016-03-01T12:00:00.000000+00:00" | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid timestamps generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  exit 1
fi;

exit_test
