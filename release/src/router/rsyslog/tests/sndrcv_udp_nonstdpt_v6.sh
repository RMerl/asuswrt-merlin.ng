#!/bin/bash
# added 2014-11-05 by Rgerhards
# This file is part of the rsyslog project, released  under ASL 2.0
echo ===============================================================================
echo \[sndrcv_udp_nonstdpt_v6.sh\]: testing sending and receiving via udp

# uncomment for debugging support:
. $srcdir/diag.sh init
# start up the instances
#export RSYSLOG_DEBUG="debug nostdout noprintmutexaction"
export RSYSLOG_DEBUGLOG="log"
generate_conf
export PORT_RCVR="$(get_free_port)"
add_conf '
module(load="../plugins/imudp/.libs/imudp")
# then SENDER sends to this port (not tcpflood!)
input(type="imudp" address="127.0.0.1" port=`echo $PORT_RCVR`)

template(name="outfmt" type="string" string="%msg:F,58:2%\n")
:msg, contains, "msgnum:" action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup
export RSYSLOG_DEBUGLOG="log2"
#valgrind="valgrind"
generate_conf 2
export TCPFLOOD_PORT="$(get_free_port)" # TODO: move to diag.sh
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
# this listener is for message generation by the test framework!
input(type="imtcp" port=`echo $TCPFLOOD_PORT`)

action(type="omfwd"
       target="127.0.0.1" port=`echo $PORT_RCVR`
       protocol="udp" udp.sendDelay="1")
' 2
startup 2
# may be needed by TLS (once we do it): sleep 30

# now inject the messages into instance 2. It will connect to instance 1,
# and that instance will record the data.
tcpflood -m500 -i1
sleep 5 # make sure all data is received in input buffers
# shut down sender when everything is sent, receiver continues to run concurrently
# may be needed by TLS (once we do it): sleep 60
shutdown_when_empty 2
wait_shutdown 2
# now it is time to stop the receiver as well
shutdown_when_empty
wait_shutdown

# may be needed by TLS (once we do it): sleep 60
# do the final check
seq_check 1 500

unset PORT_RCVR # TODO: move to exit_test()?
exit_test
