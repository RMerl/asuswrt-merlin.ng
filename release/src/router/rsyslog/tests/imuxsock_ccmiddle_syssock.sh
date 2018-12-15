#!/bin/bash
echo \[imuxsock_ccmiddle_syssock.sh\]: test trailing LF handling in imuxsock

uname
if [ `uname` = "SunOS" ] ; then
   echo "Solaris: FIX ME"
   exit 77
fi

./syslog_caller -fsyslog_inject-l -m0 > /dev/null 2>&1
no_liblogging_stdlog=$?
if [ $no_liblogging_stdlog -ne 0 ];then
  echo "liblogging-stdlog not available - skipping test"
  exit 77
fi
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imuxsock/.libs/imuxsock"
       SysSock.name="'$RSYSLOG_DYNNAME'-testbench_socket")

template(name="outfmt" type="string" string="%msg:%\n")
local1.*    action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup
# send a message with trailing LF
./syslog_caller -fsyslog_inject-c -m1 -C "uxsock:$RSYSLOG_DYNNAME-testbench_socket"
# the sleep below is needed to prevent too-early termination of rsyslogd
./msleep 100
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown	# we need to wait until rsyslogd is finished!
cmp $RSYSLOG_OUT_LOG $srcdir/resultdata/imuxsock_ccmiddle.log
if [ ! $? -eq 0 ]; then
  echo "imuxsock_ccmiddle_syssock.sh failed"
  echo "contents of $RSYSLOG_OUT_LOG:"
  echo \"`cat $RSYSLOG_OUT_LOG`\"
  exit 1
fi;
exit_test
