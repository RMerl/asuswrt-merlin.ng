#!/bin/bash
# add 2018-06-27 by Pascal Withopf, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imudp/.libs/imudp")
input(type="imudp" port="'$TCPFLOOD_PORT'" ruleset="ruleset1")


template(name="outfmt" type="string" string="%PRI%,%syslogfacility-text%,%syslogseverity-text%,%timestamp:::date-rfc3164-buggyday%,%hostname%,%programname%,%syslogtag%,%msg%\n")

ruleset(name="ruleset1") {
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`
	       template="outfmt")
}

'
startup
tcpflood -m1 -T "udp" -M "\"<38> Mar  7 19:06:53 example tag: testmessage (only date actually tested)\""
tcpflood -m1 -T "udp" -M "\"<38> Mar 17 19:06:53 example tag: testmessage (only date actually tested)\""
shutdown_when_empty
wait_shutdown

echo '38,auth,info,Mar 07 19:06:53,example,tag,tag:, testmessage (only date actually tested)
38,auth,info,Mar 17 19:06:53,example,tag,tag:, testmessage (only date actually tested)' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid response generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit  1
fi;

exit_test
