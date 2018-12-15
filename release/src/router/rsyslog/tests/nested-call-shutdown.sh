#!/bin/bash
# addd 2017-10-18 by RGerhards, released under ASL 2.0

. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/omtesting/.libs/omtesting")
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

template(name="outfmt" type="string" string="%msg:F,58:2%\n")

ruleset(name="rs3" queue.type="linkedList") {
	action(type="omfile" template="outfmt" file=`echo $RSYSLOG_OUT_LOG`)
}

ruleset(name="rs2" queue.type="linkedList") {
  call rs3
}

ruleset(name="rs1" queue.type="linkedList") {
  call rs2
  :omtesting:sleep 0 1000
}

if $msg contains "msgnum:" then call rs1
'
startup
#tcpflood -p'$TCPFLOOD_PORT' -m10000
injectmsg 0 1000
. $srcdir/diag.sh shutdown-immediate
wait_shutdown
# wo do not check reception - the main point is that we do not abort. The actual
# message count is unknown (as the point is to shut down while still in processing).
exit_test
