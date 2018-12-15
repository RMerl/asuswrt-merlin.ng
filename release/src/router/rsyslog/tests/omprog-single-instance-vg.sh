#!/bin/bash
# This file is part of the rsyslog project, released under ASL 2.0

# Same test than 'omprog-force-single-instance.sh', but checking for
# memory problems using valgrind. Note it is not necessary to repeat
# the rest of checks (this simplifies the maintenance of the tests).

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
    queue.timeoutShutdown="30000"
)

:msg, contains, "msgnum:" {
    action(
        type="omprog"
        binary=`echo $srcdir/testsuites/omprog-single-instance-bin.sh`
        template="outfmt"
        name="omprog_action"
        confirmMessages="on"
        forceSingleInstance="on"
        queue.type="LinkedList"
        queue.workerThreads="10"
        queue.size="10000"
        queue.timeoutShutdown="30000"
    )
}
'
startup_vg
tcpflood -m$NUMBER_OF_MESSAGES
shutdown_when_empty
wait_shutdown_vg
check_exit_vg
exit_test
