#!/bin/bash
# add 2017-04-28 by Pascal Withopf, released under ASL 2.0

. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imfile/.libs/imfile" mode="polling" pollingInterval="1")
input(type="imfile" File="./'$RSYSLOG_DYNNAME'.input" Tag="tag1" ruleset="ruleset1")

template(name="tmpl1" type="string" string="%msg%\n")
ruleset(name="ruleset1") {
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="tmpl1")
}
action(type="omfile" file=`echo $RSYSLOG2_OUT_LOG`)
'
startup
./msleep 3000

echo 'testmessage1
testmessage2
testmessage3' > $RSYSLOG_DYNNAME.input

./msleep 2000
rm ./'$RSYSLOG_DYNNAME'.input
./msleep 3000
shutdown_when_empty
wait_shutdown

grep "file.*$RSYSLOG_DYNNAME.input.*No such file or directory" ${RSYSLOG2_OUT_LOG} > /dev/null
if [ $? -ne 0 ]; then
        echo "FAIL: expected error message from missing input file not found. ${RSYSLOG2_OUT_LOG} is:"
        cat ${RSYSLOG2_OUT_LOG}
        error_exit 1
fi

if [ `grep "No such file or directory" ${RSYSLOG2_OUT_LOG} | wc -l` -ne 1 ]; then
	echo "FAIL: expected error message is put out multiple times. ${RSYSLOG2_OUT_LOG} is:"
	cat ${RSYSLOG2_OUT_LOG}
	error_exit 1
fi

echo 'testmessage1
testmessage2
testmessage3' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid response generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit  1
fi;

exit_test
