#!/bin/bash
# added 2014-11-11 by singh.janmejay
# This file is part of the rsyslog project, released under ASL 2.0
echo ===============================================================================
echo \[json_array_subscripting.sh\]: basic test for json array subscripting
. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="outfmt" type="string" string="msg: %$!foo[1]% | %$.quux% | %$.corge% | %$.grault% | %$!foo[3]!bar[1]!baz%\n")

module(load="../plugins/mmjsonparse/.libs/mmjsonparse")
module(load="../plugins/imptcp/.libs/imptcp")
input(type="imptcp" port="'$TCPFLOOD_PORT'")

action(type="mmjsonparse")
set $.quux = $!foo[2];
set $.corge = $!foo[3]!bar[0]!baz;
set $.grault = $!foo[3]!bar[1];
action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup
tcpflood -m 1 -I $srcdir/testsuites/json_array_input
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown 
content_check 'msg: def1 | ghi2 | important_msg | { "baz": "other_msg" } | other_msg'
exit_test
