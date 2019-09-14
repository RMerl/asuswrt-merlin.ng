#!/bin/bash
# add 2017-06-12 by Pascal Withopf, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf 'module(load="../plugins/imtcp/.libs/imtcp")
module(load="../plugins/pmnormalize/.libs/pmnormalize")

input(type="imtcp" port="'$TCPFLOOD_PORT'" ruleset="ruleset")
parser(name="custom.pmnormalize" type="pmnormalize" rule=[
	"rule=:<%pri:number%> %fromhost-ip:ipv4% %fromhost:word% %hostname:word% %syslogtag:char-to:\\x3a%: %msg:rest%",
	"rule=:<%pri:number%> %fromhost:word% %hostname:word% %fromhost-ip:ipv4% %syslogtag:char-to:\\x3a%: %msg:rest%"
	])

template(name="test" type="string" string="host: %hostname%, fromhost: %fromhost%, ip: %fromhost-ip%, tag: %syslogtag%, pri: %pri%, syslogfacility: %syslogfacility%, syslogseverity: %syslogseverity% msg: %msg%\n")

ruleset(name="ruleset" parser="custom.pmnormalize") {
	action(type="omfile" file="'$RSYSLOG_OUT_LOG'" template="test")
}
'
startup_vg

tcpflood -m1 -M "\"<189> 127.0.0.1 ubuntu ubuntu tag1: this is a test message\""
tcpflood -m1 -M "\"<112> 255.255.255.255 debian debian tag2: this is a test message\""
tcpflood -m1 -M "\"<177> centos centos 192.168.0.9 tag3: this is a test message\""
shutdown_when_empty
wait_shutdown_vg
check_exit_vg

EXPECTED='host: ubuntu, fromhost: ubuntu, ip: 127.0.0.1, tag: tag1, pri: 189, syslogfacility: 23, syslogseverity: 5 msg: this is a test message
host: debian, fromhost: debian, ip: 255.255.255.255, tag: tag2, pri: 112, syslogfacility: 14, syslogseverity: 0 msg: this is a test message
host: centos, fromhost: centos, ip: 192.168.0.9, tag: tag3, pri: 177, syslogfacility: 22, syslogseverity: 1 msg: this is a test message'
cmp_exact $RSYSLOG_OUT_LOG
exit_test
