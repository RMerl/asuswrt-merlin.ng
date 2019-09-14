#!/bin/bash
# add 2018-05-17 by Pascal Withopf, released under ASL 2.0
. $srcdir/diag.sh init
. $srcdir/diag.sh check-inotify
generate_conf
add_conf '
module(load="../plugins/imfile/.libs/imfile")

input(type="imfile" freshStartTail="on" Tag="pro"
	File="'$RSYSLOG_DYNNAME'.input")

template(name="outfmt" type="string" string="%msg%\n")

:syslogtag, contains, "pro" action(type="omfile" File=`echo $RSYSLOG_OUT_LOG`
	template="outfmt")
'

echo '{ "id": "jinqiao1"}' > $RSYSLOG_DYNNAME.input
startup
./msleep 2000
echo '{ "id": "jinqiao2"}' >> $RSYSLOG_DYNNAME.input

shutdown_when_empty
wait_shutdown

echo '{ "id": "jinqiao2"}' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid response generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit  1
fi;

exit_test
