#!/bin/bash
# add 2016-12-08 by Pascal Withopf, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
module(load="../plugins/pmnull/.libs/pmnull")
input(type="imtcp" port="'$TCPFLOOD_PORT'" ruleset="ruleset")
parser(name="custom.pmnull" type="pmnull" tag="mytag" syslogfacility="3" syslogseverity="1")
template(name="test" type="string" string="tag: %syslogtag%, pri: %pri%, syslogfacility: %syslogfacility%, syslogseverity: %syslogseverity% msg: %msg%\n")
ruleset(name="ruleset" parser=["custom.pmnull", "rsyslog.pmnull"]) {
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="test")
}
'
startup
tcpflood -m1 -M "\"<189>16261: May 28 16:09:56.185: %SYS-5-CONFIG_I: Configured from console by adminsepp on vty0 (10.23.214.226)\""
shutdown_when_empty
wait_shutdown
echo 'tag: mytag, pri: 25, syslogfacility: 3, syslogseverity: 1 msg: <189>16261: May 28 16:09:56.185: %SYS-5-CONFIG_I: Configured from console by adminsepp on vty0 (10.23.214.226)' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid response generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit  1
fi;

exit_test
