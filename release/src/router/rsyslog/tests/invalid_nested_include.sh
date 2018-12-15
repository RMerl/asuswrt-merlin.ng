#!/bin/bash
# Note: this test tests if we die when recursively include the same
# file ever again. This is a user error, but we should detect it.
# This file is part of the rsyslog project, released  under ASL 2.0
. $srcdir/diag.sh init
generate_conf
echo '$IncludeConfig '${RSYSLOG_DYNNAME}'work-nested.conf
' > ${RSYSLOG_DYNNAME}work-nested.conf
add_conf '
$IncludeConfig '${RSYSLOG_DYNNAME}'work-nested.conf
template(name="outfmt" type="string" string="%msg%\n")
if $msg contains "error" then
	action(type="omfile" template="outfmt" file=`echo $RSYSLOG_OUT_LOG`)
'
startup
shutdown_when_empty
wait_shutdown
grep ${RSYSLOG_DYNNAME}work-nested.conf $RSYSLOG_OUT_LOG
if [ $? -ne 0 ]; then
	echo "FAIL:  $RSYSLOG_OUT_LOG does not contain expected error message on"
	echo "recursive include file ${RSYSLOG_DYNNAME}work-nested.conf."
	echo "content is:"
	echo "......................................................................"
	cat $RSYSLOG_OUT_LOG
	echo "......................................................................"
	error_exit
fi
exit_test
