#!/bin/bash
# This test tests tcp forwarding with assigned template. To do so, a simple
# tcp listener service is started.
# added 2012-10-30 by Rgerhards. Released under ASL 2.0

# create the pipe and start a background process that copies data from 
# it to the "regular" work file
. $srcdir/diag.sh init
generate_conf
add_conf '
$MainMsgQueueTimeoutShutdown 10000
template(name="outfmt" type="string" string="%msg:F,58:2%\n")

if $msg contains "msgnum:" then
	action(type="omfwd" template="outfmt"
	       target="127.0.0.1" port="'$TCPFLOOD_PORT'" protocol="tcp")
'
./minitcpsrv -t127.0.0.1 -p$TCPFLOOD_PORT -f $RSYSLOG_OUT_LOG &
BGPROCESS=$!
echo background minitcpsrvr process id is $BGPROCESS

# now do the usual run
startup
# 10000 messages should be enough
injectmsg 0 10000
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown

# note: minitcpsrvr shuts down automatically if the connection is closed!
# (we still leave the code here in in case we need it later)
#echo shutting down minitcpsrv...
#kill $BGPROCESS
#wait $BGPROCESS
#echo background process has terminated, continue test...

# and continue the usual checks
seq_check 0 9999
exit_test
