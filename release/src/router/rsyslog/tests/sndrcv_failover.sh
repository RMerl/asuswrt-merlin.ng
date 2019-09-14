#!/bin/bash
# This tests failover capabilities. Data is sent to a local port, where
# no process shall listen. Then it fails over to a second instance, then to
# a file. The second instance is started. So all data should be received
# there and none be logged to the file.
# This builds on the basic sndrcv.sh test, but adds a first, failing,
# location to the conf file.
# added 2011-06-20 by Rgerhards
# This file is part of the rsyslog project, released under ASL 2.0
. $srcdir/diag.sh init

# uncomment for debugging support:
# start up the instances
#export RSYSLOG_DEBUG="debug nostdout noprintmutexaction"
export DEAD_PORT=4  # a port unassigned by IANA and very unlikely to be used
export RSYSLOG_DEBUGLOG="log"
generate_conf
export PORT_RCVR="$(get_free_port)"
add_conf '
$ModLoad ../plugins/imtcp/.libs/imtcp
# then SENDER sends to this port (not tcpflood!)
$InputTCPServerRun '$PORT_RCVR'

$template outfmt,"%msg:F,58:2%\n"
$template dynfile,"'$RSYSLOG_OUT_LOG'" # trick to use relative path names!
:msg, contains, "msgnum:" ?dynfile;outfmt
'
startup
export RSYSLOG_DEBUGLOG="log2"
#valgrind="valgrind"
generate_conf 2
export TCPFLOOD_PORT="$(get_free_port)" # TODO: move to diag.sh
add_conf '
$ModLoad ../plugins/imtcp/.libs/imtcp
# this listener is for message generation by the test framework!
$InputTCPServerRun '$TCPFLOOD_PORT'

*.*	@@127.0.0.1:'$DEAD_PORT' # this must be DEAD
$ActionExecOnlyWhenPreviousIsSuspended on
&	@@127.0.0.1:'$PORT_RCVR'
&	./'${RSYSLOG_DYNNAME}'.empty
$ActionExecOnlyWhenPreviousIsSuspended off
' 2
startup 2
# may be needed by TLS (once we do it): sleep 30

# now inject the messages into instance 2. It will connect to instance 1,
# and that instance will record the data.
tcpflood -m50000 -i1
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
seq_check 1 50000

unset PORT_RCVR # TODO: move to exit_test()?

ls -l ${RSYSLOG_DYNNAME}.empty
if [[ -s ${RSYSLOG_DYNNAME}.empty ]] ; then
  echo "FAIL: ${RSYSLOG_DYNNAME}.empty has data. Failover handling failed. Data is written"
  echo "      even though the previous action (in a failover chain!) properly"
  echo "      worked."
  error_exit 1
else
  echo "${RSYSLOG_DYNNAME}.empty is empty - OK"
fi ;
exit_test
