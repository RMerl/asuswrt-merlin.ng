#!/bin/bash
# This file is part of the rsyslog project, released under GPLv3

. $srcdir/diag.sh init

psql -h localhost -U postgres -f testsuites/pgsql-basic.sql

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

startup_vg
injectmsg  0 5000
shutdown_when_empty
wait_shutdown_vg
check_exit_vg

# we actually put the message in the SysLogTag field, so we know it doesn't use the default
# template, like in pgsql-basic
psql -h localhost -U postgres -d syslogtest -f testsuites/pgsql-select-syslogtag.sql -t -A > $RSYSLOG_OUT_LOG

seq_check  0 4999

echo cleaning up test database
psql -h localhost -U postgres -c 'DROP DATABASE IF EXISTS syslogtest;'

exit_test
