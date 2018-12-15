#!/bin/bash
# add 2018-05-17 by Pascal Withopf, released under ASL 2.0
. $srcdir/diag.sh init
. $srcdir/diag.sh check-inotify
generate_conf
add_conf '
module(load="../plugins/imfile/.libs/imfile")

input(type="imfile" freshStartTail="on" Tag="pro"
	File="'$RSYSLOG_DYNNAME'.input.*")

template(name="outfmt" type="string" string="%msg%\n")

:syslogtag, contains, "pro" action(type="omfile" File=`echo $RSYSLOG_OUT_LOG`
	template="outfmt")
'
startup

echo '{ "id": "jinqiao1"}' > $RSYSLOG_DYNNAME.input.a
./msleep 2000
echo '{ "id": "jinqiao2"}' >> $RSYSLOG_DYNNAME.input.a

shutdown_when_empty
wait_shutdown

EXPECTED='{ "id": "jinqiao1"}
{ "id": "jinqiao2"}'
cmp_exact $RSYSLOG_OUT_LOG
exit_test
