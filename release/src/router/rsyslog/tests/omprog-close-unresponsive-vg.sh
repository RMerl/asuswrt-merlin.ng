#!/bin/bash
# This file is part of the rsyslog project, released under ASL 2.0

# Same test than 'omprog-close-unresponsive.sh', but checking for memory
# problems using valgrind. Note it is not necessary to repeat the
# rest of checks (this simplifies the maintenance of the tests).

. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/omprog/.libs/omprog")

template(name="outfmt" type="string" string="%msg%\n")

main_queue(
    queue.timeoutShutdown="60000"  # give time to omprog to wait for the child
)

:msg, contains, "msgnum:" {
    action(
        type="omprog"
        binary=`echo $srcdir/testsuites/omprog-close-unresponsive-bin.sh`
        template="outfmt"
        name="omprog_action"
        queue.type="Direct"  # the default; facilitates sync with the child process
        confirmMessages="on"  # facilitates sync with the child process
        signalOnClose="on"
        closeTimeout="1000"  # ms
        #killUnresponsive="on"  # default value: the value of signalOnClose
    )
}
'
startup_vg
injectmsg 0 10
shutdown_when_empty
wait_shutdown_vg
check_exit_vg
exit_test
