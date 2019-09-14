#!/bin/bash
# addd 2016-06-16 by RGerhards, released under ASL 2.0

messages=20000 # how many messages to inject?
# Note: we need to inject a somewhat larger nubmer of messages in order
# to ensure that we receive some messages in the actual output file,
# as batching can (validly) cause a larger loss in the non-writable
# file

. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

template(name="outfmt" type="string" string="%msg:F,58:2%\n")
:msg, contains, "msgnum:" {
	action(type="omfile" template="outfmt" file=`echo $RSYSLOG2_OUT_LOG`)
	action(type="omfile" template="outfmt" file=`echo $RSYSLOG_OUT_LOG`
	       action.execOnlyWhenPreviousIsSuspended="on"
	      )
}
'
touch ${RSYSLOG2_OUT_LOG}
chmod 0400 ${RSYSLOG2_OUT_LOG}
ls -l rsyslog.ou*
startup
injectmsg 0 $messages
shutdown_when_empty
wait_shutdown
# we know that the output file is missing some messages, but it
# MUST have some more, and these be in sequence. So we now read
# the first message number and calculate based on it what must be
# present in the output file.
presort
let firstnum=$((10#`$RS_HEADCMD -n1 $RSYSLOG_DYNNAME.presort`))
echo "info: first message expected to be number $firstnum, using that value."
seq_check $firstnum $(($messages-1))
exit_test
