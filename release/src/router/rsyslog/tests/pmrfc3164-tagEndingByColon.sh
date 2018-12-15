#!/bin/bash
# add 2016-11-22 by Pascal Withopf, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'" ruleset="customparser")
parser(name="custom.rfc3164" type="pmrfc3164" force.tagEndingByColon="on")
template(name="outfmt" type="string" string="-%syslogtag%-%msg%-\n")

ruleset(name="customparser" parser="custom.rfc3164") {
	:syslogtag, contains, "tag" action(type="omfile" template="outfmt" file=`echo $RSYSLOG_OUT_LOG`)
}
'
startup
tcpflood -m1 -M "\"<129>Mar 10 01:00:00 Hostname1 tag1: msgnum:1\""
tcpflood -m1 -M "\"<129>Mar 10 01:00:00 Hostname2 tag2:  msgnum:2\""
tcpflood -m1 -M "\"<129>Mar 10 01:00:00 Hostname3 tag3 msgnum:3\""
tcpflood -m1 -M "\"<129>Mar 10 01:00:00 Hostname4 tag4 :\""
tcpflood -m1 -M "\"<129>Mar 10 01:00:00 Hostname5 tag5:msgnum:5\""
shutdown_when_empty
wait_shutdown
echo '-tag1:- msgnum:1-
-tag2:-  msgnum:2-
-tag5:-msgnum:5-' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid response generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit  1
fi;

exit_test
