#!/bin/bash
# added 2014-11-17 by singh.janmejay
# This file is part of the rsyslog project, released under ASL 2.0
echo ===============================================================================
echo \[mmnormalize_regex_disabled.sh\]: test for mmnormalize regex field_type with allow_regex disabled
. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="hosts_and_ports" type="string" string="host and port list: %$!hps%\n")

template(name="paths" type="string" string="%$!fragments% %$!user%\n")
template(name="numbers" type="string" string="nos: %$!some_nos%\n")

module(load="../plugins/mmnormalize/.libs/mmnormalize" allowRegex="off")
module(load="../plugins/imptcp/.libs/imptcp")
input(type="imptcp" port="'$TCPFLOOD_PORT'")

action(type="mmnormalize" rulebase=`echo $srcdir/testsuites/mmnormalize_regex.rulebase`)
action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="hosts_and_ports")
'
startup
tcpflood -m 1 -I $srcdir/testsuites/regex_input
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown 
assert_content_missing '192' #several ips in input are 192.168.1.0/24
exit_test
