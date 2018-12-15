#!/bin/bash
# This file is part of the rsyslog project, released under ASL 2.0

# This test tests the 'output' setting of omprog when the feedback
# feature is not used (confirmMessages=off).
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/omprog/.libs/omprog")

template(name="outfmt" type="string" string="%msg%\n")

:msg, contains, "msgnum:" {
    action(
        type="omprog"
	    binary=`echo $srcdir/testsuites/omprog-output-capture-bin.sh`
        template="outfmt"
        name="omprog_action"
        output=`echo $RSYSLOG_OUT_LOG`
        fileCreateMode="0644"  # default is 0600
    )
}
'
startup
injectmsg 0 10
shutdown_when_empty
wait_shutdown

EXPECTED="[stdout] Starting
[stderr] Starting
[stdout] Received msgnum:00000000:
[stderr] Received msgnum:00000000:
[stdout] Received msgnum:00000001:
[stderr] Received msgnum:00000001:
[stdout] Received msgnum:00000002:
[stderr] Received msgnum:00000002:
[stdout] Received msgnum:00000003:
[stderr] Received msgnum:00000003:
[stdout] Received msgnum:00000004:
[stderr] Received msgnum:00000004:
[stdout] Received msgnum:00000005:
[stderr] Received msgnum:00000005:
[stdout] Received msgnum:00000006:
[stderr] Received msgnum:00000006:
[stdout] Received msgnum:00000007:
[stderr] Received msgnum:00000007:
[stdout] Received msgnum:00000008:
[stderr] Received msgnum:00000008:
[stdout] Received msgnum:00000009:
[stderr] Received msgnum:00000009:
[stdout] Terminating normally
[stderr] Terminating normally"

cmp_exact $RSYSLOG_OUT_LOG

exit_test
