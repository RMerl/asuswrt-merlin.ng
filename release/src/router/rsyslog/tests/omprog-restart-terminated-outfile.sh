#!/bin/bash
# This file is part of the rsyslog project, released under ASL 2.0

# Similar to the 'omprog-restart-terminated.sh' test, using the 'output'
# parameter. Checks that no file descriptors are leaked across restarts
# of the program when stderr is being captured to a file.

. $srcdir/diag.sh init
check_command_available lsof

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
        output=`echo $RSYSLOG2_OUT_LOG`
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

startup
injectmsg 0 1
wait_queueempty

. $srcdir/diag.sh getpid
start_fd_count=$(lsof -p $pid | wc -l)

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
wait_queueempty

end_fd_count=$(lsof -p $pid | wc -l)

shutdown_when_empty
wait_shutdown

EXPECTED="Starting
Received msgnum:00000000:
Received msgnum:00000001:
Received msgnum:00000002:
Received SIGUSR1, will terminate after the next message
Received msgnum:00000003:
Terminating without confirming the last message
Starting
Received msgnum:00000003:
Received msgnum:00000004:
Received SIGTERM, terminating
Starting
Received msgnum:00000005:
Received msgnum:00000006:
Received msgnum:00000007:
Received SIGUSR1, will terminate after the next message
Received msgnum:00000008:
Terminating without confirming the last message
Starting
Received msgnum:00000008:
Received msgnum:00000009:
Terminating normally"

cmp_exact $RSYSLOG_OUT_LOG

EXPECTED="[stderr] Starting
[stderr] Received msgnum:00000000:
[stderr] Received msgnum:00000001:
[stderr] Received msgnum:00000002:
[stderr] Received SIGUSR1, will terminate after the next message
[stderr] Received msgnum:00000003:
[stderr] Terminating without confirming the last message
[stderr] Starting
[stderr] Received msgnum:00000003:
[stderr] Received msgnum:00000004:
[stderr] Received SIGTERM, terminating
[stderr] Starting
[stderr] Received msgnum:00000005:
[stderr] Received msgnum:00000006:
[stderr] Received msgnum:00000007:
[stderr] Received SIGUSR1, will terminate after the next message
[stderr] Received msgnum:00000008:
[stderr] Terminating without confirming the last message
[stderr] Starting
[stderr] Received msgnum:00000008:
[stderr] Received msgnum:00000009:
[stderr] Terminating normally"

cmp_exact $RSYSLOG2_OUT_LOG

if [[ "$start_fd_count" != "$end_fd_count" ]]; then
    echo "file descriptor leak: started with $start_fd_count open files, ended with $end_fd_count"
    error_exit 1
fi

exit_test
