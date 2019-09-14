#!/bin/bash
# test many concurrent tcp connections

uname
if [ `uname` = "FreeBSD" ] ; then
   echo "This test currently does not work on FreeBSD."
   exit 77
fi

echo \[manytcp-too-few-tls.sh\]: test concurrent tcp connections
. $srcdir/diag.sh init
generate_conf
add_conf '
$ModLoad ../plugins/imtcp/.libs/imtcp
$MainMsgQueueTimeoutShutdown 10000
$MaxOpenFiles 200
$InputTCPMaxSessions 1100
global(
	defaultNetstreamDriverCAFile=`echo $srcdir/testsuites/x.509/ca.pem`
	defaultNetstreamDriverCertFile=`echo $srcdir/testsuites/x.509/client-cert.pem`
	defaultNetstreamDriverKeyFile=`echo $srcdir/testsuites/x.509/client-key.pem`
	defaultNetstreamDriver="gtls"
)

$InputTCPServerStreamDriverMode 1
$InputTCPServerStreamDriverAuthMode anon
$InputTCPServerRun '$TCPFLOOD_PORT'

$template outfmt,"%msg:F,58:2%\n"
template(name="dynfile" type="string" string=`echo $RSYSLOG_OUT_LOG`) # trick to use relative path names!
:msg, contains, "msgnum:" ?dynfile;outfmt
'
startup_vg
# the config file specifies exactly 1100 connections
tcpflood -c1000 -m40000
# the sleep below is needed to prevent too-early termination of the tcp listener
sleep 1
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown_vg	# we need to wait until rsyslogd is finished!
check_exit_vg
# we do not do a seq check, as of the design of this test some messages
# will be lost. So there is no point in checking if all were received. The
# point is that we look at the valgrind result, to make sure we do not
# have a mem leak in those error cases (we had in the past, thus the test
# to prevent that in the future).
exit_test
