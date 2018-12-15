#!/bin/bash
# add 2017-11-06 by PascalWithopf, released under ASL 2.0

. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
module(load="../plugins/mmexternal/.libs/mmexternal")
input(type="imtcp" port="'$TCPFLOOD_PORT'")
set $!x = "a";

template(name="outfmt" type="string" string="-%$!%-\n")

if $msg contains "msgnum:" then {
	action(type="mmexternal" interface.input="fulljson"'
add_conf "
		binary=\"${srcdir}/testsuites/mmexternal-SegFault-mm-python.py\")"
add_conf '
	action(type="omfile" template="outfmt" file=`echo $RSYSLOG_OUT_LOG`)
}
'
startup_vg
tcpflood -m1 -M "\"<129>Mar 10 01:00:00 172.20.245.8 tag:msgnum:1\""
shutdown_when_empty
wait_shutdown_vg
check_exit_vg

echo '-{ "x": "a", "sometag": "somevalue" }-' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid response generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit  1
fi;

exit_test
