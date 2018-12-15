#!/bin/bash
# Test imptcp with poller not processing any messages
# added 2015-10-16 by singh.janmejay
# This file is part of the rsyslog project, released  under GPLv3
echo ====================================================================================
echo TEST: \[imptcp_nonProcessingPoller.sh\]: test imptcp with poller driven processing disabled
. $srcdir/diag.sh init
generate_conf
add_conf '
$MaxMessageSize 10k
template(name="outfmt" type="string" string="%msg:F,58:2%,%msg:F,58:3%,%msg:F,58:4%\n")

module(load="../plugins/imptcp/.libs/imptcp" threads="32" processOnPoller="off")
input(type="imptcp" port="'$TCPFLOOD_PORT'")

if (prifilt("local0.*")) then {
   action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
}
'
startup
tcpflood -c1 -m20000 -r -d10000 -P129 -O
sleep 2 # due to large messages, we need this time for the tcp receiver to settle...
shutdown_when_empty
wait_shutdown
seq_check 0 19999 -E
exit_test
