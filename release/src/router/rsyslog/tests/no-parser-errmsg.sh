#!/bin/bash
# add 2017-03-06 by Rainer Gerhards, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'" ruleset="ruleset")
template(name="test" type="string" string="tag: %syslogtag%, pri: %pri%, syslogfacility: %syslogfacility%, syslogseverity: %syslogseverity% msg: %msg%\n")
ruleset(name="ruleset" parser="rsyslog.rfc5424") {
	action(type="omfile" file=`echo $RSYSLOG2_OUT_LOG` template="test")
}
action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`)
'
startup
tcpflood -m1
shutdown_when_empty
wait_shutdown
grep 'one message could not be processed by any parser'  $RSYSLOG_OUT_LOG > /dev/null
if [ $? -ne 0 ]; then
  echo "invalid response generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit  1
fi;

exit_test
