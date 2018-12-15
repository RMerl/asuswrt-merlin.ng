#!/bin/bash
# added 2013-12-10 by Rgerhards
# This file is part of the rsyslog project, released under ASL 2.0
# the first test is redundant, but we keep it when we later make the port
# configurable.
if [ `./omrelp_dflt_port` -lt 1024 ]; then
    if [ "$EUID" -ne 0 ]; then
	echo need to be root to run this test - skipping
        exit 77
    fi
fi

if [ `./omrelp_dflt_port` -ne 13515 ]; then
    echo this test needs configure option --enable-omrelp-default-port=13515 to work
    exit 77
fi
exit

# uncomment for debugging support:
. $srcdir/diag.sh init
# start up the instances
#export RSYSLOG_DEBUG="debug nostdout noprintmutexaction"
export RSYSLOG_DEBUGLOG="log"
generate_conf
export PORT_RCVR="$(get_free_port)"
add_conf '
module(load="../plugins/imrelp/.libs/imrelp")
# then SENDER sends to this port (not tcpflood!)
input(type="imrelp" port="'$PORT_RCVR'")

$template outfmt,"%msg:F,58:2%\n"
:msg, contains, "msgnum:" action(type="omfile" file="'$RSYSLOG_OUT_LOG'" template="outfmt")
'
startup
export RSYSLOG_DEBUGLOG="log2"
#valgrind="valgrind"
generate_conf 2
export TCPFLOOD_PORT="$(get_free_port)"
add_conf '
module(load="../plugins/omrelp/.libs/omrelp")

action(type="omrelp" protocol="tcp" target="127.0.0.1" port="'$PORT_RCVR'")
' 2
startup 2
# may be needed by TLS (once we do it): sleep 30

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
