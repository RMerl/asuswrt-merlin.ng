#!/bin/bash
# This file is part of the rsyslog project, released under GPLv3
echo ===============================================================================
echo \[libdbi-asyn.sh\]: asyn test for libdbi functionality
. $srcdir/diag.sh init
generate_conf
add_conf '
$ModLoad ../plugins/omlibdbi/.libs/omlibdbi

$ActionQueueType LinkedList
$ActionQueueTimeoutEnqueue 2000

$ActionLibdbiDriver mysql
$ActionLibdbiHost 127.0.0.1
$ActionLibdbiUserName rsyslog
$ActionLibdbiPassword testbench
$ActionLibdbiDBName Syslog
:msg, contains, "msgnum:" :omlibdbi:
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
