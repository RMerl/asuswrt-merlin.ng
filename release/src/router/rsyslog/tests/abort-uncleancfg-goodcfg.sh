#!/bin/bash
# Copyright 2015-01-29 by Tim Eifler
# This file is part of the rsyslog project, released  under ASL 2.0
echo ===============================================================================
echo \[abort-uncleancfg-goodcfg.sh\]: testing abort on unclean configuration
echo "testing a good Configuration verification run"
. $srcdir/diag.sh init
generate_conf
add_conf '
$AbortOnUncleanConfig on

$ModLoad ../plugins/imtcp/.libs/imtcp
$MainMsgQueueTimeoutShutdown 10000
$InputTCPServerRun '$TCPFLOOD_PORT'

$template outfmt,"%msg:F,58:2%\n"
template(name="dynfile" type="string" string=`echo $RSYSLOG_OUT_LOG`) # trick to use relative path names!
:msg, contains, "msgnum:" ?dynfile;outfmt
'
startup
tcpflood -m10 -i1 
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown

if [ ! -e  $RSYSLOG_OUT_LOG ]
then
        echo "error: expected file does not exist"
	error_exit 1
fi
exit_test
