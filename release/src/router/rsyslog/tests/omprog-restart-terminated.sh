#!/bin/bash
# This file is part of the rsyslog project, released under ASL 2.0

# This test checks that omprog restarts the external program when it
# terminates prematurely, and that it does so without leaking file
# descriptors. Two cases are checked: termination of the program when
# omprog is going to write to the pipe (to send a message to the
# program), and when omprog is going to read from the pipe (when it
# is expecting the program to confirm the last message).
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
child_lsof=$(lsof -a -d 0-65535 -p $(get_child_pid) | awk '$4 != "255r" { print $4 " " $9 }')

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

if [[ "$start_fd_count" != "$end_fd_count" ]]; then
    echo "file descriptor leak: started with $start_fd_count open files, ended with $end_fd_count"
    error_exit 1
fi

# Check that the child process does not inherit open fds from rsyslog
# (apart from the pipes), and that stderr is redirected to /dev/null.
# Ignore fd 255, which bash opens for internal use.

EXPECTED_CHILD_LSOF="FD NAME
0r pipe
1w pipe
2w /dev/null"

# On Solaris, lsof gives this alternate output:
EXPECTED_CHILD_LSOF_2="FD NAME
0u (fifofs)
1u (fifofs)
2w "

if [[ "$child_lsof" != "$EXPECTED_CHILD_LSOF" && "$child_lsof" != "$EXPECTED_CHILD_LSOF_2" ]]; then
    echo "unexpected open files for child process:"
    echo "$child_lsof"
    error_exit 1
fi

exit_test
