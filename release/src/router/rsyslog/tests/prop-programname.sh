#!/bin/bash
# addd 2017-01142 by RGerhards, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

template(name="outfmt" type="string" string="%syslogtag%,%programname%\n")
local0.* action(type="omfile" template="outfmt"
	        file=`echo $RSYSLOG_OUT_LOG`)
'
startup
tcpflood -m 1 -M "\"<133>2011-03-01T11:22:12Z host tag/with/slashes msgh ...x\""
tcpflood -m1
shutdown_when_empty
wait_shutdown
EXPECTED="tag/with/slashes,tag"
cmp_exact $RSYSLOG_OUT_LOG
exit_test
