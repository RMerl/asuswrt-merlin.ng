#!/bin/bash
# add 2016-11-22 by Pascal Withopf, released under ASL 2.0

. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

template(name="dynafile" type="string" string=`echo $RSYSLOG_OUT_LOG`)
template(name="outfmt" type="string" string="-%msg%-\n")

:msg, contains, "msgnum:" {
	action(type="omfile" template="outfmt" file=`echo $RSYSLOG2_OUT_LOG` dynafile="dynafile")
}
action(type="omfile" file="'${RSYSLOG_DYNNAME}'.errorfile") 
'
startup
tcpflood -m1 -M "\"<129>Mar 10 01:00:00 172.20.245.8 tag: msgnum:1\""
shutdown_when_empty
wait_shutdown

grep "will use dynafile" ${RSYSLOG_DYNNAME}.errorfile > /dev/null
if [ $? -ne 0 ]; then
	echo
	echo "FAIL: expected error message not found. ${RSYSLOG_DYNNAME}.errorfile is:"
	cat ${RSYSLOG_DYNNAME}.errorfile
	error_exit 1
fi

echo '- msgnum:1-' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "unexpected content in  $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit  1
fi;

if [ -f ${RSYSLOG2_OUT_LOG} ]; then
  echo "file exists, but should not: ${RSYSLOG2_OUT_LOG}; content:"
  cat ${RSYSLOG2_OUT_LOG}
  error_exit  1
fi;


exit_test
