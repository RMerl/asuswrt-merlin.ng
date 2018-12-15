#!/bin/bash
# This file is part of the rsyslog project, released under GPLv3
echo ===============================================================================
echo \[mysql-asyn.sh\]: asyn test for mysql functionality
. $srcdir/diag.sh init
generate_conf
add_conf '
$ModLoad ../plugins/ommysql/.libs/ommysql
$ActionQueueType LinkedList
$ActionQueueTimeoutEnqueue 10000 # 10 second to make sure we do not loose due to action q full
:msg, contains, "msgnum:" :ommysql:127.0.0.1,Syslog,rsyslog,testbench;
'
mysql --user=rsyslog --password=testbench < testsuites/mysql-truncate.sql
startup
injectmsg  0 50000
shutdown_when_empty
wait_shutdown 
# note "-s" is requried to suppress the select "field header"
mysql -s --user=rsyslog --password=testbench < testsuites/mysql-select-msg.sql > $RSYSLOG_OUT_LOG
seq_check  0 49999
exit_test
