#!/bin/bash
# This file is part of the rsyslog project, released under ASL 2.0

# This test tests the feedback feature of omprog (confirmMessages=on),
# by checking that omprog restarts the program if it does not send the
# feedback before the configured 'confirmTimeout'. Also tests the
# keep-alive feature.

. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/omprog/.libs/omprog")

template(name="outfmt" type="string" string="%msg%\n")

:msg, contains, "msgnum:" {
    action(
        type="omprog"
        binary=`echo $srcdir/testsuites/omprog-feedback-timeout-bin.sh`
        template="outfmt"
        name="omprog_action"
        queue.type="Direct"  # the default; facilitates sync with the child process
        confirmMessages="on"
        confirmTimeout="2000"  # 2 seconds
        reportFailures="on"
    )
}
'
startup
injectmsg 0 10
shutdown_when_empty
wait_shutdown

EXPECTED="Starting
<= OK
=> msgnum:00000000:
<= OK
=> msgnum:00000001:
<= OK
=> msgnum:00000002:
<= OK
=> msgnum:00000003:
<= OK
=> msgnum:00000004:
<= (timeout)
Starting
<= OK
=> msgnum:00000004:
<= OK
=> msgnum:00000005:
<= OK
=> msgnum:00000006:
<= OK
=> msgnum:00000007:
<= ........OK
=> msgnum:00000008:
<= OK
=> msgnum:00000009:
<= OK"

cmp_exact $RSYSLOG_OUT_LOG

exit_test
