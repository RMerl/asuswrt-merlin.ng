#!/bin/bash
. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="outfmt" type="string" string="%msg:F,58:2%\n")

if `echo $DO_WORK` == "on" and $msg contains "msgnum:" then
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
export DO_WORK=on
startup_vg
injectmsg 0 1000
#tcpflood -m10
shutdown_when_empty
wait_shutdown_vg
seq_check  0 999
exit_test
