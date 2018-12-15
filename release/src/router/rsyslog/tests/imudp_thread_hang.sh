#!/bin/bash
# the whole point of this test is just to check that imudp
# does not block rsyslog termination. This test was introduced
# after we had a regression where imudp's worker threads were
# not properly terminated.
# Copyright 2014 by Rainer Gerhards, licensed under ASL 2.0
echo \[imudp_thread_hang\]: a situation where imudp caused a hang
. $srcdir/diag.sh init
generate_conf
add_conf '
$WorkDirectory '$RSYSLOG_DYNNAME'.spool

module(load="../plugins/imudp/.libs/imudp" threads="3")
input(type="imudp" Address="127.0.0.1" Port="20514")
'
startup
./msleep 1000
. $srcdir/diag.sh shutdown-immediate
wait_shutdown
exit_test
