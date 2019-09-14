#!/bin/bash
# add 2018-06-27 by Pascal Withopf, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imudp/.libs/imudp")
input(type="imudp" port="'$TCPFLOOD_PORT'" ruleset="ruleset1")

global(localHostname="localhost")

template(name="outfmt" type="string" string="%PRI%,%syslogfacility-text%,%syslogseverity-text%,%timestamp%,%programname%,%syslogtag%,%msg%\n")

ruleset(name="ruleset1") {
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`
	       template="outfmt")
}

'
startup
tcpflood -m1 -T "udp" -M "\"<175>Oct 16 23:47:31 #001 MSWinEventLog 0#011Security#01119023582#011Fri Oct 16 16:30:44 2009#011592#011Security#011rgabcde#011User#011Success Audit#011XSXSXSN01#011Detailed Tracking#011#0112572#01119013885\""
shutdown_when_empty
wait_shutdown

echo '175,local5,debug,Oct 16 23:47:31,#001,#001, MSWinEventLog 0#011Security#01119023582#011Fri Oct 16 16:30:44 2009#011592#011Security#011rgabcde#011User#011Success Audit#011XSXSXSN01#011Detailed Tracking#011#0112572#01119013885' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid response generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit  1
fi;

exit_test
