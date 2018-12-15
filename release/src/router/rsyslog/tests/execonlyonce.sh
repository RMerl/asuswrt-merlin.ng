#!/bin/bash
# Test for the $ActionExecOnlyOnceEveryInterval directive.
# We inject a couple of messages quickly during the interval,
# then wait until the interval expires, then quickly inject
# another set. After that, it is checked if exactly two messages
# have arrived.
# The once interval must be set to 3 seconds in the config file.
# added 2009-11-12 by Rgerhards
# This file is part of the rsyslog project, released  under GPLv3
echo ===============================================================================
echo \[execonlyonce.sh\]: test for the $ActionExecOnlyOnceEveryInterval directive
. $srcdir/diag.sh init
generate_conf
add_conf '
$ModLoad ../plugins/imtcp/.libs/imtcp
$MainMsgQueueTimeoutShutdown 10000
$InputTCPServerRun '$TCPFLOOD_PORT'

$template outfmt,"%msg:F,58:2%\n"
template(name="dynfile" type="string" string=`echo $RSYSLOG_OUT_LOG`) # trick to use relative path names!
$ActionExecOnlyOnceEveryInterval 3
:msg, contains, "msgnum:" ?dynfile;outfmt
'
startup
tcpflood -m10 -i1
# now wait until the interval definitely expires
sleep 4 # one more than the once inerval!
# and inject another couple of messages
tcpflood -m10 -i100
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown

# now we need your custom logic to see if the result is equal to the
# expected result
cmp $RSYSLOG_OUT_LOG testsuites/execonlyonce.data
if [ $? -eq 1 ]
then
	echo "ERROR, output not as expected"
	exit 1
fi
exit_test
