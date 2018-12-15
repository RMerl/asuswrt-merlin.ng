#!/bin/bash
# addd 2016-03-24 by RGerhards, released under ASL 2.0

uname
if [ `uname` = "SunOS" ] ; then
   echo "Solaris: FIX ME"
   exit 77
fi

. $srcdir/privdrop_common.sh
rsyslog_testbench_setup_testuser

. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="outfmt" type="list") {
	property(name="msg" compressSpace="on")
	constant(value="\n")
}
action(type="omfile" template="outfmt" file=`echo $RSYSLOG_OUT_LOG`)
'
add_conf "\$PrivDropToUser ${TESTBENCH_TESTUSER[username]}"
startup
shutdown_when_empty
wait_shutdown
grep "userid.*${TESTBENCH_TESTUSER[uid]}" < $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "message indicating drop to user \"${TESTBENCH_TESTUSER[username]}\" (#${TESTBENCH_TESTUSER[uid]}) is missing:"
  cat $RSYSLOG_OUT_LOG
  exit 1
fi;

exit_test
