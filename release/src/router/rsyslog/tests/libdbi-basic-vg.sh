#!/bin/bash
# This file is part of the rsyslog project, released under GPLv3
# this test is currently not included in the testbench as libdbi
# itself seems to have a memory leak
echo ===============================================================================
echo \[libdbi-basic.sh\]: basic test for libdbi-basic functionality via mysql
. $srcdir/diag.sh init
generate_conf
add_conf '
$ModLoad ../plugins/omlibdbi/.libs/omlibdbi
$ActionLibdbiDriver mysql
$ActionLibdbiHost 127.0.0.1
$ActionLibdbiUserName rsyslog
$ActionLibdbiPassword testbench
$ActionLibdbiDBName Syslog
:msg, contains, "msgnum:" :omlibdbi:
'
mysql --user=rsyslog --password=testbench < testsuites/mysql-truncate.sql
startup_vg_noleak
injectmsg  0 5000
shutdown_when_empty
wait_shutdown_vg
check_exit_vg
# note "-s" is requried to suppress the select "field header"
mysql -s --user=rsyslog --password=testbench < testsuites/mysql-select-msg.sql > $RSYSLOG_OUT_LOG
seq_check  0 4999
exit_test
