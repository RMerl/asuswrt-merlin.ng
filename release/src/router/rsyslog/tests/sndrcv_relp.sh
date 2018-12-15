#!/bin/bash
# added 2013-12-10 by Rgerhards
# This file is part of the rsyslog project, released under ASL 2.0
#. $srcdir/sndrcv_drvr.sh sndrcv_relp 50000


. $srcdir/diag.sh init
########## receiver ##########
#export RSYSLOG_DEBUG="debug nostdout noprintmutexaction"
#export RSYSLOG_DEBUGLOG="log"
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
printf "#### RECEIVER STARTED\n\n"

########## sender ##########
#export RSYSLOG_DEBUGLOG="log2"
generate_conf 2
add_conf '
module(load="../plugins/omrelp/.libs/omrelp")

action(type="omrelp" name="omrelp" target="127.0.0.1" port="'$PORT_RCVR'")
' 2
startup 2
# may be needed by TLS (once we do it): sleep 30
printf "#### SENDER STARTED\n\n"

# now inject the messages into instance 2. It will connect to instance 1,
# and that instance will record the data.
injectmsg 1 50000

shutdown_when_empty 2
wait_shutdown 2
# now it is time to stop the receiver as well
shutdown_when_empty
wait_shutdown

seq_check 1 50000
exit_test
