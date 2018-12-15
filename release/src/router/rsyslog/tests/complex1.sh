#!/bin/bash
# This is a rather complex test that runs a number of features together.
#
# added 2010-03-16 by Rgerhards
#
# This file is part of the rsyslog project, released under ASL 2.0
. $srcdir/diag.sh init
skip_platform "SunOS"  "This test currently does not work on all flavors of Solaris."
export RSYSLOG_PORT2="$(get_free_port)"
export RSYSLOG_PORT3="$(get_free_port)"
generate_conf
add_conf '
$MaxMessageSize 10k

$MainMsgQueueTimeoutEnqueue 5000

$ModLoad ../plugins/imtcp/.libs/imtcp
$MainMsgQueueTimeoutShutdown 10000

$template outfmt,"%msg:F,58:3%,%msg:F,58:4%,%msg:F,58:5%\n"
$template dynfile,"'$RSYSLOG_DYNNAME'.out.%inputname%.%msg:F,58:2%.log.Z"

## RULESET with listener
$Ruleset R13514
# queue params:
$ActionQueueTimeoutShutdown 60000
$ActionQueueTimeoutEnqueue 5000
$ActionQueueSize 5000
$ActionQueueSaveOnShutdown on
$ActionQueueHighWaterMark 4900
$ActionQueueLowWaterMark 3500
$ActionQueueType FixedArray
$ActionQueueWorkerThreads 1
# action params:
$OMFileFlushOnTXEnd off
$OMFileZipLevel 6
#$OMFileIOBufferSize 256k
$DynaFileCacheSize 4
$omfileFlushInterval 1
*.* ?dynfile;outfmt
# listener
$InputTCPServerInputName '$TCPFLOOD_PORT'
$InputTCPServerBindRuleset R13514
$InputTCPServerRun '$TCPFLOOD_PORT'


## RULESET with listener
$Ruleset R_PORT2
# queue params:
$ActionQueueTimeoutShutdown 60000
$ActionQueueTimeoutEnqueue 5000
$ActionQueueSize 5000
$ActionQueueSaveOnShutdown on
$ActionQueueHighWaterMark 4900
$ActionQueueLowWaterMark 3500
$ActionQueueType FixedArray
$ActionQueueWorkerThreads 1
# action params:
$OMFileFlushOnTXEnd off
$OMFileZipLevel 6
$OMFileIOBufferSize 256k
$DynaFileCacheSize 4
$omfileFlushInterval 1
*.* ?dynfile;outfmt
# listener
$InputTCPServerInputName '$RSYSLOG_PORT2'
$InputTCPServerBindRuleset R_PORT2
$InputTCPServerRun '$RSYSLOG_PORT2'



## RULESET with listener
$Ruleset R_PORT3
# queue params:
$ActionQueueTimeoutShutdown 60000
$ActionQueueTimeoutEnqueue 5000
$ActionQueueSize 5000
$ActionQueueSaveOnShutdown on
$ActionQueueHighWaterMark 4900
$ActionQueueLowWaterMark 3500
$ActionQueueType FixedArray
$ActionQueueWorkerThreads 1
# action params:
$OMFileFlushOnTXEnd off
$OMFileZipLevel 6
$OMFileIOBufferSize 256k
$DynaFileCacheSize 4
$omfileFlushInterval 1
*.* ?dynfile;outfmt
# listener
$InputTCPServerInputName '$RSYSLOG_PORT3'
$InputTCPServerBindRuleset R_PORT3
$InputTCPServerRun '$RSYSLOG_PORT3'
'
# uncomment for debugging support:
#export RSYSLOG_DEBUG="debug nostdout"
#export RSYSLOG_DEBUGLOG="log"
startup
# send 40,000 messages of 400 bytes plus header max, via three dest ports
export TCPFLOOD_PORT="$TCPFLOOD_PORT:$RSYSLOG_PORT2:$RSYSLOG_PORT3"
tcpflood -m40000 -rd400 -P129 -f5 -n3 -c15 -i1
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown       # and wait for it to terminate
ls $RSYSLOG_DYNNAME.out.*.log.Z
gunzip $RSYSLOG_DYNNAME.out.*.log.Z
cat $RSYSLOG_DYNNAME.out.*.log > $RSYSLOG_OUT_LOG
seq_check 1 40000 -E
exit_test
