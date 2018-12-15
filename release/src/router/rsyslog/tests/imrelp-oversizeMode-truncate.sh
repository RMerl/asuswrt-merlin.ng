#!/bin/bash
# add 2018-04-19 by PascalWithopf, released under ASL 2.0
. $srcdir/diag.sh init
./have_relpSrvSetOversizeMode
if [ $? -eq 1 ]; then
  echo "imrelp parameter oversizeMode not available. Test stopped"
  exit 77
fi;
generate_conf
add_conf '
module(load="../plugins/imrelp/.libs/imrelp")
global(maxMessageSize="150" oversizemsg.input.mode="accept")


input(type="imrelp" port="'$TCPFLOOD_PORT'" maxdatasize="200" oversizeMode="truncate")

template(name="outfmt" type="string" string="%msg%\n")
:msg, contains, "msgnum:" action(type="omfile" template="outfmt"
				 file=`echo $RSYSLOG_OUT_LOG`)
'
startup
tcpflood -Trelp-plain -p'$TCPFLOOD_PORT' -m1 -d 240
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown

# We need the ^-sign to symbolize the beginning and the $-sign to symbolize the end
# because otherwise we won't know if it was truncated at the right length.
grep "^ msgnum:00000000:240:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX$"  $RSYSLOG_OUT_LOG > /dev/null
if [ $? -ne 0 ]; then
        echo
        echo "FAIL: expected message not found.  $RSYSLOG_OUT_LOG is:"
        cat $RSYSLOG_OUT_LOG
        error_exit 1
fi

exit_test
