#!/bin/bash
# This file is part of the rsyslog project, released under ASL 2.0

# Same test than 'omprog-restart-terminated.sh', but checking for memory
# problems using valgrind. Note it is not necessary to repeat the
# rest of checks (this simplifies the maintenance of the tests).

. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/omprog/.libs/omprog")

template(name="outfmt" type="string" string="%msg%\n")

:msg, contains, "msgnum:" {
    action(
        type="omprog"
        binary="'$RSYSLOG_DYNNAME'.omprog-restart-terminated-bin.sh"
        template="outfmt"
        name="omprog_action"
        queue.type="Direct"  # the default; facilitates sync with the child process
        confirmMessages="on"  # facilitates sync with the child process
        action.resumeRetryCount="3"
        action.resumeInterval="1"
        action.reportSuspensionContinuation="on"
        signalOnClose="off"
    )
}
'

# We need a test-specific program name, as the test needs to signal the child process
cp -f $srcdir/testsuites/omprog-restart-terminated-bin.sh $RSYSLOG_DYNNAME.omprog-restart-terminated-bin.sh

# On Solaris 10, the output of ps is truncated for long process names; use /usr/ucb/ps instead:
if [[ `uname` = "SunOS" && `uname -r` = "5.10" ]]; then
    function get_child_pid {
        echo $(/usr/ucb/ps -awwx | grep "$RSYSLOG_DYNNAME.[o]mprog-restart-terminated-bin.sh" | awk '{ print $1 }')
    }
else
    function get_child_pid {
        echo $(ps -ef | grep "$RSYSLOG_DYNNAME.[o]mprog-restart-terminated-bin.sh" | awk '{ print $2 }')
    }
fi

startup_vg
injectmsg 0 1
injectmsg 1 1
injectmsg 2 1
wait_queueempty

kill -s USR1 $(get_child_pid)
./msleep 100

injectmsg 3 1
injectmsg 4 1
wait_queueempty

kill -s TERM $(get_child_pid)
./msleep 100

injectmsg 5 1
injectmsg 6 1
injectmsg 7 1
wait_queueempty

kill -s USR1 $(get_child_pid)
./msleep 100

injectmsg 8 1
injectmsg 9 1

shutdown_when_empty
wait_shutdown_vg
check_exit_vg
exit_test
