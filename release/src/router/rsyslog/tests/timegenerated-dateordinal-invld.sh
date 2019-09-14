#!/bin/bash
# test many concurrent tcp connections
# addd 2016-03-02 by RGerhards, released under ASL 2.0
# the key point of this test is that we do not abort and
# instead provide the defined return value (0)
# requires faketime
echo \[timegenerated-dateordinal-invld\]: check invalid dates with ordinal format
. $srcdir/diag.sh init

. $srcdir/faketime_common.sh

export TZ=UTC+00:00

generate_conf
add_conf '
$ModLoad ../plugins/imtcp/.libs/imtcp
$InputTCPServerRun '$TCPFLOOD_PORT'

template(name="outfmt" type="string"
	 string="%timegenerated:::date-ordinal%\n")
:msg, contains, "msgnum:" action(type="omfile" template="outfmt"
			         file=`echo $RSYSLOG_OUT_LOG`)
'


echo "***SUBTEST: check 1800-01-01"
rm -f $RSYSLOG_OUT_LOG	# do cleanup of previous subtest
FAKETIME='1800-01-01 00:00:00' startup
tcpflood -m1
shutdown_when_empty
wait_shutdown
echo "001" | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid timestamps generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  date -d @`cat $RSYSLOG_OUT_LOG`
  exit 1
fi;


echo "***SUBTEST: check 1960-01-01"
rm -f $RSYSLOG_OUT_LOG	# do cleanup of previous subtest
FAKETIME='1960-01-01 00:00:00' startup
tcpflood -m1
shutdown_when_empty
wait_shutdown
echo "001" | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid timestamps generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  date -d @`cat $RSYSLOG_OUT_LOG`
  exit 1
fi;


echo "***SUBTEST: check 2101-01-01"
rm -f $RSYSLOG_OUT_LOG	# do cleanup of previous subtest
FAKETIME='2101-01-01 00:00:00' startup
tcpflood -m1
shutdown_when_empty
wait_shutdown
echo "001" | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid timestamps generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  date -d @`cat $RSYSLOG_OUT_LOG`
  exit 1
fi;


echo "***SUBTEST: check 2500-01-01"
rm -f $RSYSLOG_OUT_LOG	# do cleanup of previous subtest
FAKETIME='2500-01-01 00:00:00' startup
tcpflood -m1
shutdown_when_empty
wait_shutdown
echo "001" | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid timestamps generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  date -d @`cat $RSYSLOG_OUT_LOG`
  exit 1
fi;


exit_test
