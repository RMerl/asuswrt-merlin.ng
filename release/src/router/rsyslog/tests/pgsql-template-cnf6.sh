#!/bin/bash
# This file is part of the rsyslog project, released under ASL 2.0

. $srcdir/diag.sh init

psql -h localhost -U postgres -f ${srcdir}/testsuites/pgsql-basic.sql

generate_conf
add_conf '
template(name="pgtemplate" type="list" option.sql="on") {
	constant(value="INSERT INTO SystemEvents (SysLogTag) values ('"'"'")
	property(name="msg")
	constant(value="'"'"')")
}

module(load="../plugins/ompgsql/.libs/ompgsql")
if $msg contains "msgnum" then {
	action(type="ompgsql" server="127.0.0.1"
		db="syslogtest" user="postgres" pass="testbench"
		template="pgtemplate")
}'

startup
injectmsg  0 5000
shutdown_when_empty
wait_shutdown


psql -h localhost -U postgres -d syslogtest -f ${srcdir}/testsuites/pgsql-select-syslogtag.sql -t -A > $RSYSLOG_OUT_LOG 


seq_check  0 4999

echo cleaning up test database
psql -h localhost -U postgres -c 'DROP DATABASE IF EXISTS syslogtest;'

exit_test
