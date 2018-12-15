#!/bin/bash
# Added 2018-01-17 by Rainer Gerhards, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

template(name="outfmt" type="list"){
    property(name="jsonmesg")
    constant(value="\n")
}
local4.* action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'

startup_vg
tcpflood -m1 -y
shutdown_when_empty
wait_shutdown_vg
check_exit_vg
export EXPECTED='"msg": "msgnum:00000000:", '
. $srcdir/diag.sh grep-check
exit_test
