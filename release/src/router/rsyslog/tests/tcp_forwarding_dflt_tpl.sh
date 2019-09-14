#!/bin/bash
# This test tests tcp forwarding with assigned default template.
# added 2015-05-30 by rgerhards. Released under ASL 2.0

# create the pipe and start a background process that copies data from 
# it to the "regular" work file
. $srcdir/diag.sh init
generate_conf
add_conf '
$MainMsgQueueTimeoutShutdown 10000

template(name="outfmt" type="string" string="%msg:F,58:2%\n")

#this is what we want to test: setting the default template
module(load="builtin:omfwd" template="outfmt")

if $msg contains "msgnum:" then
	action(type="omfwd" target="127.0.0.1" port="'$TCPFLOOD_PORT'" protocol="tcp")
'
./minitcpsrv -t127.0.0.1 -p$TCPFLOOD_PORT -f $RSYSLOG_OUT_LOG &
BGPROCESS=$!
echo background minitcpsrv process id is $BGPROCESS

# now do the usual run
startup
# 10000 messages should be enough
injectmsg 0 10000
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown

# note: minitcpsrv shuts down automatically if the connection is closed!
# (we still leave the code here in in case we need it later)
#echo shutting down minitcpsrv...
#kill $BGPROCESS
#wait $BGPROCESS
#echo background process has terminated, continue test...

# and continue the usual checks
seq_check 0 9999
exit_test
