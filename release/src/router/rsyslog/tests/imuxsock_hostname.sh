#!/bin/bash
echo \[imuxsock_hostname.sh\]: test set hostname
./syslog_caller -fsyslog_inject-l -m0 > /dev/null 2>&1
no_liblogging_stdlog=$?
if [ $no_liblogging_stdlog -ne 0 ];then
  echo "liblogging-stdlog not available - skipping test"
  exit 77
fi
. $srcdir/diag.sh init
generate_conf
add_conf '
global(localHostName="rsyslog-testbench-hostname")

module(load="../plugins/imuxsock/.libs/imuxsock" sysSock.use="off")
input(type="imuxsock" Socket="'$RSYSLOG_DYNNAME'-testbench_socket")

template(name="outfmt" type="string" string="%hostname:%\n")
local1.*    action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup
# the message itself is irrelevant. The only important thing is
# there is one
./syslog_caller -m1 -C "uxsock:$RSYSLOG_DYNNAME-testbench_socket"
# the sleep below is needed to prevent too-early termination of rsyslogd
./msleep 100
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown	# we need to wait until rsyslogd is finished!
cmp $RSYSLOG_OUT_LOG $srcdir/resultdata/imuxsock_hostname.log
if [ ! $? -eq 0 ]; then
  echo "imuxsock_hostname.sh failed"
  exit 1
fi;
exit_test
