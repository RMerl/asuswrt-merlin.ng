#!/bin/bash
# This file is part of the rsyslog project, released under ASL 2.0
echo ===============================================================================
echo \[mysql-act-mt.sh\]: test for mysql with multithread actionq
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/ommysql/.libs/ommysql")

:msg, contains, "msgnum:" {
	action(type="ommysql" server="127.0.0.1"
	db="Syslog" uid="rsyslog" pwd="testbench"
	queue.size="10000" queue.type="linkedList"
	queue.workerthreads="5"
	queue.workerthreadMinimumMessages="500"
	queue.timeoutWorkerthreadShutdown="1000"
	queue.timeoutEnqueue="10000"
	)
} 
'
mysql --user=rsyslog --password=testbench < testsuites/mysql-truncate.sql
startup
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
wait_shutdown 
# note "-s" is requried to suppress the select "field header"
mysql -s --user=rsyslog --password=testbench < testsuites/mysql-select-msg.sql > $RSYSLOG_OUT_LOG
seq_check  0 149999
exit_test
