#!/bin/bash
# add 2016-11-22 by Jan Gerhards, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="outfmt" type="string" string="%msg%\n")
template(name="filename" type="string" string="'${RSYSLOG_DYNNAME}'.%syslogtag%.log")

module(load="../plugins/mmanon/.libs/mmanon")
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'" ruleset="testing")

ruleset(name="testing") {
	action(type="mmanon" ipv4.mode="random" ipv4.bits="32")
	action(type="omfile" dynafile="filename" template="outfmt")
}'

echo 'Since this test tests randomization, there is a theoretical possibility of it failing even if rsyslog works correctly. Therefore, if the test unexpectedly fails try restarting it.'
startup
tcpflood -m1 -M "\"<129>Mar 10 01:00:00 172.20.245.8 file1 1.1.1.8
<129>Mar 10 01:00:00 172.20.245.8 file2 0.0.0.0
<129>Mar 10 01:00:00 172.20.245.8 file3 172.0.234.255
<129>Mar 10 01:00:00 172.20.245.8 file4 111.1.1.8.
<129>Mar 10 01:00:00 172.20.245.8 file5 172.0.234.255\""

shutdown_when_empty
wait_shutdown
echo ' 1.1.1.8' | cmp - ${RSYSLOG_DYNNAME}.file1.log >/dev/null
if [ ! $? -eq 1 ]; then
  echo "invalidly equal ip-address generated, ${RSYSLOG_DYNNAME}.file1.log is:"
  cat ${RSYSLOG_DYNNAME}.file1.log
  error_exit  1
fi;

echo ' 0.0.0.0' | cmp - ${RSYSLOG_DYNNAME}.file2.log >/dev/null
if [ ! $? -eq 1 ]; then
  echo "invalidly equal ip-address generated, ${RSYSLOG_DYNNAME}.file2.log is:"
  cat ${RSYSLOG_DYNNAME}.file2.log
  error_exit  1
fi;

echo ' 172.0.234.255' | cmp - ${RSYSLOG_DYNNAME}.file3.log  >/dev/null
if [ ! $? -eq 1 ]; then
  echo "invalidly equal ip-address generated, ${RSYSLOG_DYNNAME}.file3.log is:"
  cat ${RSYSLOG_DYNNAME}.file3.log
  error_exit  1
fi;

echo ' 111.1.1.8.' | cmp - ${RSYSLOG_DYNNAME}.file4.log >/dev/null
if [ ! $? -eq 1 ]; then
  echo "invalidly equal ip-address generated, ${RSYSLOG_DYNNAME}.file4.log is:"
  cat ${RSYSLOG_DYNNAME}.file4.log
  error_exit  1
fi;

cmp ${RSYSLOG_DYNNAME}.file3.log ${RSYSLOG_DYNNAME}.file5.log >/dev/null
if [ ! $? -eq 1 ]; then
  echo "invalidly equal ip-addresses generated, ${RSYSLOG_DYNNAME}.file3.log and ${RSYSLOG_DYNNAME}.file5.log are:"
  cat ${RSYSLOG_DYNNAME}.file3.log
  cat ${RSYSLOG_DYNNAME}.file5.log
  error_exit  1
fi;

rm -f ${RSYSLOG_DYNNAME}.*.log
exit_test
