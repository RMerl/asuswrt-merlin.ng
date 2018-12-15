#!/bin/bash
# This file is part of the rsyslog project, released under GPLv3

uname
if [ `uname` = "FreeBSD" ] ; then
   echo "This test currently does not work on FreeBSD."
   exit 77
fi

echo ===============================================================================
echo \[failover-no-basic.sh\]: basic test for failover functionality - no failover
. $srcdir/diag.sh init
generate_conf
add_conf '
$RepeatedMsgReduction off

# second action should never execute
:msg, contains, "msgnum:" /dev/null
$ActionExecOnlyWhenPreviousIsSuspended on
& ./'"${RSYSLOG_OUT_LOG}"'
'
startup_vg
injectmsg  0 5000
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown_vg
check_exit_vg
# now we need our custom logic to see if the result file is empty
# (what it should be!)
cmp $RSYSLOG_OUT_LOG /dev/null
if [ $? -eq 1 ]
then
	echo "ERROR, output file not empty"
	exit 1
fi
exit_test
