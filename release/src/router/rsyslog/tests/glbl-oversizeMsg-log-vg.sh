#!/bin/bash
# add 2018-05-03 by PascalWithopf, released under ASL 2.0
. $srcdir/diag.sh init
./have_relpSrvSetOversizeMode
if [ $? -eq 1 ]; then
  echo "imrelp parameter oversizeMode not available. Test stopped"
  exit 77
fi;
generate_conf
add_conf '
module(load="../plugins/imrelp/.libs/imrelp")
global(maxMessageSize="230"
	oversizemsg.errorfile=`echo $RSYSLOG2_OUT_LOG`)


input(type="imrelp" port="'$TCPFLOOD_PORT'" maxdatasize="300")

template(name="outfmt" type="string" string="%rawmsg%\n")
action(type="omfile" template="outfmt"
				 file=`echo $RSYSLOG_OUT_LOG`)
'
# TODO: add tcpflood option to specific EXACT test message size!
startup_vg
tcpflood -Trelp-plain -p'$TCPFLOOD_PORT' -m1 -d 240
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown_vg
check_exit_vg
grep "rawmsg.*<167>Mar  1 01:00:00 172.20.245.8 tag msgnum:00000000:240:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX.*input.*imrelp" ${RSYSLOG2_OUT_LOG} > /dev/null
if [ $? -ne 0 ]; then
        echo
        echo "FAIL: expected message not found. ${RSYSLOG2_OUT_LOG} is:"
        cat ${RSYSLOG2_OUT_LOG}
        error_exit 1
fi


exit_test
