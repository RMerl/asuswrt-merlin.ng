#!/bin/bash
# addd 2016-03-22 by RGerhards, released under ASL 2.0

. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

template(name="outfmt" type="string" string="%msg:::compressSPACE%\n")
:msg, contains, "msgnum:" action(type="omfile" template="outfmt"
			         file=`echo $RSYSLOG_OUT_LOG`)
'

startup
# we need to generate a file, because otherwise our multiple spaces
# do not survive the execution pathes through the shell
echo "<165>1 2003-08-24T05:14:15.000003-07:00 192.0.2.1 tcpflood 8710 - - msgnum:0000000 test   test     test" >$RSYSLOG_DYNNAME.tmp
tcpflood -I $RSYSLOG_DYNNAME.tmp
rm $RSYSLOG_DYNNAME.tmp
#tcpflood -m1 -M"\"<165>1 2003-08-24T05:14:15.000003-07:00 192.0.2.1 tcpflood 8710 - - msgnum:0000000 test   test     test\""
shutdown_when_empty
wait_shutdown
echo "msgnum:0000000 test test test" | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid message recorded, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  exit 1
fi;

exit_test
