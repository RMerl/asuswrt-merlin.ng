#!/bin/bash
# add 2016-11-22 by Pascal Withopf, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'" ruleset="customparser")
parser(name="custom.rfc3164" type="pmrfc3164" permit.AtSignsInHostname="off")
template(name="outfmt" type="string" string="-%hostname%-%syslogtag%-%msg%-\n")

ruleset(name="customparser" parser="custom.rfc3164") {
	:syslogtag, contains, "tag" action(type="omfile" template="outfmt" file=`echo $RSYSLOG_OUT_LOG`)
}
'
startup
tcpflood -m1 -M "\"<129>Mar 10 01:00:00 Hostname1 tag: msgnum:1\""
tcpflood -m1 -M "\"<129>Mar 10 01:00:00 Hostn@me2 tag:  msgnum:2\""
tcpflood -m1 -M "\"<129>Mar 10 01:00:00 Hostname3 tag:msgnum:3\""
tcpflood -m1 -M "\"<129>Mar 10 01:00:00 Hos@name4 tag4:\""
shutdown_when_empty
wait_shutdown
echo '-Hostname1-tag:- msgnum:1-
-Hostname3-tag:-msgnum:3-' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid response generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit  1
fi;

exit_test
