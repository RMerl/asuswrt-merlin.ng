#!/bin/bash
# This file is part of the rsyslog project, released  under ASL 2.0
# Copyright (C) 2014 Rainer Gerhards -- 2014-11-14
echo ====================================================================================
echo TEST: \[imptcp_conndrop-vg.sh\]: test imptcp with random connection drops
. $srcdir/diag.sh init
generate_conf
add_conf '
$MaxMessageSize 10k

$ModLoad ../plugins/imptcp/.libs/imptcp
$MainMsgQueueTimeoutShutdown 10000
$InputPTCPServerRun '$TCPFLOOD_PORT'

$template outfmt,"%msg:F,58:2%,%msg:F,58:3%,%msg:F,58:4%\n"
template(name="dynfile" type="string" string=`echo $RSYSLOG_OUT_LOG`) # trick to use relative path names!
$OMFileFlushOnTXEnd off
$OMFileFlushInterval 2
$OMFileIOBufferSize 256k
local0.* ?dynfile;outfmt
'
startup_vg
# 100 byte messages to gain more practical data use
tcpflood -c20 -m50000 -r -d100 -P129 -D
sleep 10 # due to large messages, we need this time for the tcp receiver to settle...
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown_vg    # and wait for it to terminate
check_exit_vg
seq_check 0 49999 -E
exit_test
