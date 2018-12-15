#!/bin/bash
# This file is part of the rsyslog project, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

template(type="string" name="outfmt" string="%rawmsg-after-pri%\n")
if $syslogfacility-text == "local0" then
    action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup
tcpflood -m1 -P 129
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown       # and wait for it to terminate
NUMLINES=$(grep -c "^Mar  1 01:00:00 172.20.245.8 tag msgnum:00000000:$"  $RSYSLOG_OUT_LOG 2>/dev/null)

if [ -z $NUMLINES ]; then
  echo "ERROR: output file seems not to exist"
  ls -l $RSYSLOG_OUT_LOG
  cat $RSYSLOG_OUT_LOG
  error_exit 1
else
  if [ ! $NUMLINES -eq 1 ]; then
    echo "ERROR: output format does not match expectation"
    cat $RSYSLOG_OUT_LOG
    error_exit 1
  fi
fi
exit_test
