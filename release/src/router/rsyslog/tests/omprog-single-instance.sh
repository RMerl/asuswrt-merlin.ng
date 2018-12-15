#!/bin/bash
# This file is part of the rsyslog project, released under ASL 2.0

# This test tests the omprog 'forceSingleInstance' flag by checking
# that only one instance of the program is started when multiple
# workers are in effect.

NUMBER_OF_MESSAGES=10000  # number of logs to send

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
        binary=`echo $srcdir/testsuites/omprog-single-instance-bin.sh`
        template="outfmt"
        name="omprog_action"
        confirmMessages="on"
        forceSingleInstance="on"
        queue.type="LinkedList"  # use a dedicated queue
        queue.workerThreads="10"  # ...with multiple workers
        queue.size="10000"  # ...high capacity (default is 1000)
        queue.timeoutShutdown="30000"  # ...and a long shutdown timeout
    )
}
'
startup
tcpflood -m$NUMBER_OF_MESSAGES
shutdown_when_empty
wait_shutdown

EXPECTED_LINE_LENGTH=25    # example line: 'Received msgnum:00009880:'
line_num=0
while IFS= read -r line; do
    let "line_num++"
    if [[ $line_num == 1 ]]; then
        if [[ "$line" != "Starting" ]]; then
            echo "unexpected first line in output: $line"
            error_exit 1
        fi
    elif [[ $line_num == $(($NUMBER_OF_MESSAGES + 2)) ]]; then
        if [[ "$line" != "Terminating" ]]; then
            echo "unexpected last line in output: $line"
            error_exit 1
        fi
    elif [[ ${#line} != $EXPECTED_LINE_LENGTH ]]; then
        echo "unexpected line in output (line $line_num): $line"
        error_exit 1
    fi
done < $RSYSLOG_OUT_LOG

if (( $line_num != $(($NUMBER_OF_MESSAGES + 2)) )); then
    echo "unexpected line count in output: $line_num (expected: $(($NUMBER_OF_MESSAGES + 2)))"
    error_exit 1
fi

exit_test
