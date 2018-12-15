#!/bin/bash
# added 2011-02-28 by Rgerhards
# This file is part of the rsyslog project, released  under GPLv3
echo ===============================================================================
echo \[imtcp-tls-basic.sh\]: testing imtcp in TLS mode - basic test
. $srcdir/diag.sh init
generate_conf
add_conf '
$ModLoad ../plugins/imtcp/.libs/imtcp
$MainMsgQueueTimeoutShutdown 10000

$DefaultNetstreamDriver gtls

# certificate files - just CA for a client
$IncludeConfig '$RSYSLOG_DYNNAME'.rsyslog.conf.tlscert
$InputTCPServerStreamDriverMode 1
$InputTCPServerStreamDriverAuthMode anon
$InputTCPServerRun '$TCPFLOOD_PORT'

$template outfmt,"%msg:F,58:2%\n"
$OMFileFlushOnTXEnd off
$OMFileFlushInterval 2
$OMFileAsyncWriting on
$OMFileIOBufferSize 16k
:msg, contains, "msgnum:" action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
echo \$DefaultNetstreamDriverCAFile $srcdir/tls-certs/ca.pem     >$RSYSLOG_DYNNAME.rsyslog.conf.tlscert
echo \$DefaultNetstreamDriverCertFile $srcdir/tls-certs/cert.pem >>$RSYSLOG_DYNNAME.rsyslog.conf.tlscert
echo \$DefaultNetstreamDriverKeyFile $srcdir/tls-certs/key.pem   >>$RSYSLOG_DYNNAME.rsyslog.conf.tlscert
startup
tcpflood -p'$TCPFLOOD_PORT' -m50000 -Ttls -Z$srcdir/tls-certs/cert.pem -z$srcdir/tls-certs/key.pem
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown
seq_check 0 49999
exit_test
