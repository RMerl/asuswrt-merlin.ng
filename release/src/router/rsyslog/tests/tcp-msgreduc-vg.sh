#!/bin/bash
# check if valgrind violations occur. Correct output is not checked.
# added 2011-03-01 by Rgerhards
# This file is part of the rsyslog project, released  under GPLv3

uname
if [ `uname` = "FreeBSD" ] ; then
   echo "This test currently does not work on FreeBSD."
   exit 77
fi

echo ===============================================================================
echo \[tcp-msgreduc-vg.sh\]: testing msg reduction via UDP
. $srcdir/diag.sh init
generate_conf
add_conf '
$ModLoad ../plugins/imtcp/.libs/imtcp
$InputTCPServerRun '$TCPFLOOD_PORT'
$RepeatedMsgReduction on

$template outfmt,"%msg:F,58:2%\n"
*.*       action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup_vg
tcpflood -t 127.0.0.1 -m 4 -r -M "\"<133>2011-03-01T11:22:12Z host tag msgh ...\""
tcpflood -t 127.0.0.1 -m 1 -r -M "\"<133>2011-03-01T11:22:12Z host tag msgh ...x\""
# we need to give rsyslog a little time to settle the receiver
./msleep 1500
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown_vg
check_exit_vg
exit_test
