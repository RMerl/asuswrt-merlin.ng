#!/bin/bash
# added 2017-09-29 by Rgerhards
# This file is part of the rsyslog project, released under ASL 2.0
. $srcdir/diag.sh init
echo testing sending and receiving via relp w/ rebind interval
# uncomment for debugging support:
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
export TCPFLOOD_PORT="$(get_free_port)" # TODO: move to diag.sh
add_conf '
module(load="../plugins/omrelp/.libs/omrelp")

# We know that a rebind interval of 1 is NOT what you would normally expect in
# production. However, this stresses the code the most and we have seen that
# some problems do not reliably occur if we use higher rebind intervals. Thus
# we consider it to be a good, actually required, setting.
action(type="omrelp" protocol="tcp" target="127.0.0.1" port="'$PORT_RCVR'" rebindinterval="1")
' 2
startup 2
# may be needed by TLS (once we do it): sleep 30

# now inject the messages into instance 2. It will connect to instance 1,
# and that instance will record the data.
injectmsg 1 1000

# shut down sender
shutdown_when_empty 2
wait_shutdown 2
# now it is time to stop the receiver as well
shutdown_when_empty
wait_shutdown

seq_check 1 1000
exit_test
