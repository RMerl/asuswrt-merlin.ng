#!/bin/bash
# This is part of the rsyslog testbench, licensed under ASL 2.0
. $srcdir/diag.sh init
uname
if [ `uname` = "FreeBSD" ] ; then
   echo "This test currently does not work on FreeBSD."
   exit 77
fi

grep "\.el6\." <<< $(uname -a)
if [ "$?" == "0" ]; then
	echo "CentOS 6 detected, adding valgrind suppressions"
	export RS_TEST_VALGRIND_EXTRA_OPTS="--suppressions=${srcdir}/imfile-basic-vgthread.supp"
fi

generate_conf
add_conf '
$ModLoad ../plugins/imfile/.libs/imfile
$InputFileName ./'$RSYSLOG_DYNNAME'.input
$InputFileTag file:
$InputFileStateFile stat-file1
$InputFileSeverity error
$InputFileFacility local7
$InputFileMaxLinesAtOnce 100000
$InputRunFileMonitor

$template outfmt,"%msg:F,58:2%\n"
:msg, contains, "msgnum:" ./'$RSYSLOG_OUT_LOG';outfmt
'

# generate input file first. Note that rsyslog processes it as
# soon as it start up (so the file should exist at that point).
./inputfilegen -m 50000 > $RSYSLOG_DYNNAME.input
ls -l $RSYSLOG_DYNNAME.input

startup_vgthread
shutdown_when_empty
wait_shutdown_vg
check_exit_vg

seq_check 0 49999
exit_test
