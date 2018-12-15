#!/bin/bash
# This file is part of the rsyslog project, released under ASL 2.0

# This test tests the feedback feature of omprog (confirmMessages=on),
# by checking that omprog re-sends to the external program the messages
# it has failed to process.

. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/omprog/.libs/omprog")

template(name="outfmt" type="string" string="%msg%\n")

:msg, contains, "msgnum:" {
    action(
        type="omprog"
        binary=`echo $srcdir/testsuites/omprog-feedback-bin.sh`
        template="outfmt"
        name="omprog_action"
        queue.type="Direct"  # the default; facilitates sync with the child process
        confirmMessages="on"
        reportFailures="on"
        action.resumeInterval="1"  # retry interval: 1 second
#       action.resumeRetryCount="0" # the default; no need to increase since
                                    # the action resumes immediately
    )
}
'
startup
injectmsg 0 10
shutdown_when_empty
wait_shutdown

EXPECTED="<= OK
=> msgnum:00000000:
<= OK
=> msgnum:00000001:
<= OK
=> msgnum:00000002:
<= OK
=> msgnum:00000003:
<= OK
=> msgnum:00000004:
<= Error: could not process log message
=> msgnum:00000004:
<= Error: could not process log message
=> msgnum:00000004:
<= OK
=> msgnum:00000005:
<= OK
=> msgnum:00000006:
<= OK
=> msgnum:00000007:
<= Error: could not process log message
=> msgnum:00000007:
<= Error: could not process log message
=> msgnum:00000007:
<= OK
=> msgnum:00000008:
<= OK
=> msgnum:00000009:
<= OK"

cmp_exact $RSYSLOG_OUT_LOG

exit_test
