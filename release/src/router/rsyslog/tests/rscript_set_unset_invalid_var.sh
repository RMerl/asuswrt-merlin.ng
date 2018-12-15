#!/bin/bash
# Check that invalid variable names are detected.
# Copyright 2017-01-24 by Rainer Gerhards
# This file is part of the rsyslog project, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="json" type="string" string="%$!%\n")
ruleset(name="rcvr" queue.type="LinkedList") {
	set $@timestamp="test";
	unset $@timestamp2;
	action(type="omfile" file=`echo $RSYSLOG2_OUT_LOG`)
}

action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`)
 
'
startup
injectmsg  0 10
shutdown_when_empty
wait_shutdown

grep "@timestamp"  $RSYSLOG_OUT_LOG > /dev/null
if [ ! $? -eq 0 ]; then
  echo "expected error message on \"@timestamp\" not found, output is:"
  echo "------------------------------------------------------------"
  cat $RSYSLOG_OUT_LOG
  echo "------------------------------------------------------------"
  error_exit 1
fi;

grep "@timestamp2"  $RSYSLOG_OUT_LOG > /dev/null
if [ ! $? -eq 0 ]; then
  echo "expected error message on \"@timestamp2\" not found, output is:"
  echo "------------------------------------------------------------"
  cat $RSYSLOG_OUT_LOG
  echo "------------------------------------------------------------"
  error_exit 1
fi;

exit_test
