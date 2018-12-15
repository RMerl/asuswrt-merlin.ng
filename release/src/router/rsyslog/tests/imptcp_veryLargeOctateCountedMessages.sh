#!/bin/bash
# Test imptcp with poller not processing any messages
# test imptcp with very large messages while poller driven processing is disabled
# added 2015-10-17 by singh.janmejay
# This file is part of the rsyslog project, released  under GPLv3
. $srcdir/diag.sh init
generate_conf
add_conf '$MaxMessageSize 10k
template(name="outfmt" type="string" string="%msg:F,58:2%,%msg:F,58:3%,%msg:F,58:4%\n")

module(load="../plugins/imptcp/.libs/imptcp" threads="32" processOnPoller="off")
input(type="imptcp" port="'$TCPFLOOD_PORT'")

if (prifilt("local0.*")) then {
   action(type="omfile" file="'$RSYSLOG_OUT_LOG'" template="outfmt")
}
'
export RS_REDIR="2>/dev/null"
startup 1
tcpflood -c1 -m20000 -r -d100000 -P129 -O
shutdown_when_empty
wait_shutdown
seq_check 0 19999 -E -T
exit_test
