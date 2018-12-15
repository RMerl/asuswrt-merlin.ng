#!/bin/bash
# This file is part of the rsyslog project, released under ASL 2.0

# Similar to the 'omprog-feedback.sh' test, with multiple worker threads
# on high load, and a given error rate (percentage of failed messages, i.e.
# confirmed as failed by the program). Note: the action retry interval
# (1 second) causes a very low throughput; we need to set a very low error
# rate to avoid the test lasting too much.

NUMBER_OF_MESSAGES=10000  # number of logs to send
ERROR_RATE_PERCENT=1      # percentage of logs to be retried

export command_line=`echo $srcdir/testsuites/omprog-feedback-mt-bin.sh $ERROR_RATE_PERCENT`

. $srcdir/diag.sh init

uname
if [ `uname` = "SunOS" ] ; then
    # On Solaris, this test causes rsyslog to hang for unknown reasons
    echo "Solaris: FIX ME"
    exit 77
fi

generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
module(load="../plugins/omprog/.libs/omprog")

input(type="imtcp" port="'$TCPFLOOD_PORT'")

template(name="outfmt" type="string" string="%msg%\n")

main_queue(
    queue.timeoutShutdown="30000"  # long shutdown timeout for the main queue
)

:msg, contains, "msgnum:" {
    action(
        type="omprog"
        binary=`echo $command_line`
        template="outfmt"
        name="omprog_action"
        confirmMessages="on"
        queue.type="LinkedList"  # use a dedicated queue
        queue.workerThreads="10"  # ...with multiple workers
        queue.size="10000"  # ...high capacity (default is 1000)
        queue.timeoutShutdown="30000"  # ...and a long shutdown timeout
        action.resumeInterval="1"  # retry interval: 1 second
    )
}
'
startup
tcpflood -m$NUMBER_OF_MESSAGES
shutdown_when_empty
wait_shutdown

# Note: we use awk here to remove leading spaces returned by wc on FreeBSD
line_count=$(wc -l < ${RSYSLOG_OUT_LOG} | awk '{print $1}')
if [[ $line_count != $NUMBER_OF_MESSAGES ]]; then
    echo "unexpected line count in omprog script output: $line_count (expected: $NUMBER_OF_MESSAGES)"
    error_exit 1
fi

exit_test
