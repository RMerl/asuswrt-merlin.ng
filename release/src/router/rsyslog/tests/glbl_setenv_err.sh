#!/bin/bash
# This is part of the rsyslog testbench, licensed under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
# env var is missing equal sign and MUST trigger parsing error!
global(environment="http_proxy ERROR")

action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`)
'
startup
injectmsg  0 1
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown    # we need to wait until rsyslogd is finished!

grep "http_proxy ERROR" < $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo 
  echo "MESSAGE INDICATING ERROR ON ENVIRONMENT VARIABLE IS MISSING:"
  echo 
  cat $RSYSLOG_OUT_LOG
  error_exit 1
fi;

exit_test
