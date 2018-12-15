#!/bin/bash
# This file is part of the rsyslog project, released under ASL 2.0

# Similar to the 'omprog-output-capture.sh' test, with multiple worker
# threads on high load. Checks that the lines concurrently emmitted to
# stdout/stderr by the various program instances are not intermingled in
# the output file (i.e., are captured atomically by omprog) when 1) the
# lines are less than PIPE_BUF bytes long and 2) the program writes the
# lines in line-buffered mode. In this test, the 'stdbuf' utility of GNU
# Coreutils is used to force line buffering in a Python program (see
# 'omprog-output-capture-mt-bin.py' for alternatives).
. $srcdir/diag.sh init
uname
if [ `uname` = "SunOS" ] ; then
   echo "This test currently does not work on all flavors of Solaris (problems with Python?)."
   exit 77
fi

NUMBER_OF_MESSAGES=20000   # number of logs to send

if [[ "$(uname)" == "Linux" ]]; then
    LINE_LENGTH=4095   # 4KB minus 1 byte (for the newline char)
else
    LINE_LENGTH=511   # 512 minus 1 byte (for the newline char)
fi

export command_line="/usr/bin/stdbuf -oL -eL $srcdir/testsuites/omprog-output-capture-mt-bin.py $LINE_LENGTH"

check_command_available stdbuf
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
module(load="../plugins/omprog/.libs/omprog")

input(type="imtcp" port="'$TCPFLOOD_PORT'")

template(name="outfmt" type="string" string="%msg%\n")

main_queue(
    queue.timeoutShutdown="60000"  # long shutdown timeout for the main queue
)

:msg, contains, "msgnum:" {
    action(
        type="omprog"
        binary=`echo $command_line`
        template="outfmt"
        name="omprog_action"
        queue.type="LinkedList"  # use a dedicated queue
        queue.workerThreads="10"  # ...with many workers
        queue.timeoutShutdown="60000"  # ...and a long shutdown timeout
        closeTimeout="10000"  # wait enough for program to terminate
        output=`echo $RSYSLOG_OUT_LOG`
        fileCreateMode="0644"
    )
}
'
startup
tcpflood -m$NUMBER_OF_MESSAGES

# Issue some HUP signals to cause the output file to be reopened during
# writing (not a complete test of this feature, but at least we check it
# doesn't break the output).
issue_HUP
./msleep 1000
issue_HUP
./msleep 1000
issue_HUP

shutdown_when_empty
wait_shutdown

line_num=0
while IFS= read -r line; do
    let "line_num++"
    if [[ ${#line} != $LINE_LENGTH ]]; then
        echo "intermingled line in captured output: line: $line_num, length: ${#line} (expected: $LINE_LENGTH)"
        echo "$line"
        error_exit 1
    fi
done < $RSYSLOG_OUT_LOG

if (( $line_num != $(($NUMBER_OF_MESSAGES * 2)) )); then
    echo "unexpected number of lines in captured output: $line_num (expected: $(($NUMBER_OF_MESSAGES * 2)))"
    error_exit 1
fi

exit_test
