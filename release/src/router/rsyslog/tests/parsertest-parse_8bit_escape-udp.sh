#!/bin/bash
# add 2018-06-28 by Pascal Withopf, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imudp/.libs/imudp")
input(type="imudp" port="'$TCPFLOOD_PORT'" ruleset="ruleset1")

$Escape8BitCharactersOnReceive on

template(name="outfmt" type="string" string="%PRI%,%syslogfacility-text%,%syslogseverity-text%,%timestamp%,%hostname%,%programname%,%syslogtag%,%msg%\n")

ruleset(name="ruleset1") {
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`
	       template="outfmt")
}

'
startup
tcpflood -m1 -T "udp" -M "\"<6>AUG 10 22:18:24 host tag This msg contains 8-bit European chars: äöü\""
shutdown_when_empty
wait_shutdown

echo '6,kern,info,Aug 10 22:18:24,host,tag,tag, This msg contains 8-bit European chars: #303#244#303#266#303#274' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid response generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit  1
fi;

exit_test
