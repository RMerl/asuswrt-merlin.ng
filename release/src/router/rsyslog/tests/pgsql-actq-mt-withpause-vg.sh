#!/bin/bash
# This file is part of the rsyslog project, released under ASL 2.0

. $srcdir/diag.sh init

psql -h localhost -U postgres -f testsuites/pgsql-basic.sql

generate_conf
add_conf '
module(load="../plugins/ompgsql/.libs/ompgsql")
if $msg contains "msgnum" then {
	action(type="ompgsql" server="127.0.0.1"
		db="syslogtest" user="postgres" pass="testbench"
		queue.size="10000" queue.type="linkedList"
		queue.workerthreads="5"
		queue.workerthreadMinimumMessages="500"
		queue.timeoutWorkerthreadShutdown="1000"
		queue.timeoutEnqueue="10000"
	)
}'
startup_vg
injectmsg  0 50000
wait_queueempty
echo waiting for worker threads to timeout
./msleep 3000
injectmsg  50000 50000
wait_queueempty
echo waiting for worker threads to timeout
./msleep 2000
injectmsg  100000 50000
shutdown_when_empty
wait_shutdown_vg
check_exit_vg

psql -h localhost -U postgres -d syslogtest -f testsuites/pgsql-select-msg.sql -t -A > $RSYSLOG_OUT_LOG
seq_check  0 149999
echo cleaning up test database
psql -h localhost -U postgres -c 'DROP DATABASE IF EXISTS syslogtest;'

exit_test
