#!/bin/bash
# added 2018-09-13 by PascalWithopf
# This file is part of the rsyslog project, released under ASL 2.0
. $srcdir/diag.sh init
# uncomment for debugging support:
#export RSYSLOG_DEBUG="debug nostdout noprintmutexaction"
export RSYSLOG_DEBUGLOG="log"

generate_conf
export PORT_RCVR="$(get_free_port)"
add_conf '
module(load="../plugins/imrelp/.libs/imrelp")
# then SENDER sends to this port (not tcpflood!)
input(type="imrelp" port="'$PORT_RCVR'" tls="on"
		tls.cacert="tls-certs/ca.pem"
		tls.mycert="tls-certs/cert.pem"
		tls.myprivkey="tls-certs/key.pem"
		tls.authmode="certvalid"
		tls.permittedpeer="rsyslog")

$template outfmt,"%msg:F,58:2%\n"
:msg, contains, "msgnum:" action(type="omfile" file="'$RSYSLOG_OUT_LOG'" template="outfmt")
'
startup

export RSYSLOG_DEBUGLOG="log2"
#valgrind="valgrind"
generate_conf 2
add_conf '
module(load="../plugins/omrelp/.libs/omrelp")

:msg, contains, "msgnum:" action(type="omrelp"
		target="127.0.0.1" port="'$PORT_RCVR'" tls="on"
		tls.cacert="tls-certs/ca.pem"
		tls.mycert="tls-certs/cert.pem"
		tls.myprivkey="tls-certs/key.pem"
		tls.authmode="certvalid"
		tls.permittedpeer="rsyslog")
action(type="omfile" file="'$RSYSLOG_DYNNAME.errmsgs'")
' 2
startup 2

grep "omrelp error: invalid authmode" $RSYSLOG_DYNNAME.errmsgs > /dev/null
if [ $? -eq 0 ]; then
        echo "SKIP: librelp does not support "certvalid" auth mode"
	# mini-cleanup to not leave dangling processes
	. $srcdir/diag.sh shutdown-immediate 2
	. $srcdir/diag.sh shutdown-immediate
	rm $RSYSLOG_DYNNAME* &> /dev/null
	exit 77
fi

# now inject the messages into instance 2. It will connect to instance 1,
# and that instance will record the data.
injectmsg 1 50000

# shut down sender
shutdown_when_empty 2
wait_shutdown 2
# now it is time to stop the receiver as well
shutdown_when_empty
wait_shutdown

seq_check 1 50000
exit_test
