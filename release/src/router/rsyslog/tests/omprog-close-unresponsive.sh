#!/bin/bash
# This file is part of the rsyslog project, released under ASL 2.0

# This test checks that omprog sends a TERM signal to the external
# program when signalOnClose=on, closes the pipe, and kills the
# child if unresponsive.

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
        binary="'$RSYSLOG_DYNNAME.'omprog-close-unresponsive-bin.sh"
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
cp -f $srcdir/testsuites/omprog-close-unresponsive-bin.sh $RSYSLOG_DYNNAME.omprog-close-unresponsive-bin.sh
startup
injectmsg 0 10
shutdown_when_empty
wait_shutdown
. $srcdir/diag.sh ensure-no-process-exists $RSYSLOG_DYNNAME.omprog-close-unresponsive-bin.sh

EXPECTED="Starting
Received msgnum:00000000:
Received msgnum:00000001:
Received msgnum:00000002:
Received msgnum:00000003:
Received msgnum:00000004:
Received msgnum:00000005:
Received msgnum:00000006:
Received msgnum:00000007:
Received msgnum:00000008:
Received msgnum:00000009:
Received SIGTERM
Terminating unresponsively"
cmp_exact $RSYSLOG_OUT_LOG

exit_test
