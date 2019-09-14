#!/bin/bash
# This is part of the rsyslog testbench, licensed under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
set $!backslash = "a \\ \"b\" c / d"; # '/' must not be escaped!

template(name="json" type="list" option.json="on") {
        constant(value="{")
        constant(value="\"backslash\":\"")     
		property(name="$!backslash")
        constant(value="\"}\n")
}

:msg, contains, "msgnum:" action(type="omfile" template="json"
			         file=`echo $RSYSLOG_OUT_LOG`)
'
startup
injectmsg  0 1
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown    # we need to wait until rsyslogd is finished!

printf '{"backslash":"a \\\\ \\"b\\" c / d"}\n' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid JSON generated, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit 1
fi;

exit_test
