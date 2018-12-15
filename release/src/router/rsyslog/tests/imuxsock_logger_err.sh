#!/bin/bash
# this is primarily a safeguard to ensure the imuxsock tests basically work
# added 2014-12-04 by Rainer Gerhards, licensed under ASL 2.0
echo \[imuxsock_logger.sh\]: test imuxsock
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imuxsock/.libs/imuxsock" sysSock.use="off")
input(type="imuxsock" Socket="'$RSYSLOG_DYNNAME'-testbench_socket")

template(name="outfmt" type="string" string="%msg:%\n")
*.notice      action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup
# send a message with trailing LF
logger -d -u $RSYSLOG_DYNNAME-testbench_socket "wrong message"
# the sleep below is needed to prevent too-early termination of rsyslogd
./msleep 100
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown	# we need to wait until rsyslogd is finished!
cmp $RSYSLOG_OUT_LOG $srcdir/resultdata/imuxsock_logger.log
if [ $? -eq 0 ]; then
  echo "imuxsock_logger_err.sh did not report an error where it should!"
  exit 1
else
  echo "all well, we saw the error that we wanted to have"
fi;
exit_test
