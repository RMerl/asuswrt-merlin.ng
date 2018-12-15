#!/bin/bash
# This file is part of the rsyslog project, released under GPLv3

. $srcdir/diag.sh init

psql -h localhost -U postgres -f testsuites/pgsql-basic.sql

generate_conf
add_conf '
module(load="../plugins/ompgsql/.libs/ompgsql")
if $msg contains "msgnum" then {
	action(type="ompgsql" server="127.0.0.1"
		db="syslogtest" user="postgres" pass="testbench")
}'
startup
injectmsg  0 5000
shutdown_when_empty
wait_shutdown


psql -h localhost -U postgres -d syslogtest -f testsuites/pgsql-select-msg.sql -t -A > $RSYSLOG_OUT_LOG

seq_check  0 4999

echo cleaning up test database
psql -h localhost -U postgres -c 'DROP DATABASE IF EXISTS syslogtest;'

exit_test
