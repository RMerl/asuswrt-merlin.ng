#!/bin/bash
# This test checks if an empty includeConfig directory causes problems. It
# should not, as this is a valid situation that by default exists on many
# distros.
. $srcdir/diag.sh init
generate_conf
add_conf "\$IncludeConfig ${srcdir}/testsuites/incltest.d/*.conf-not-there
"
add_conf '$template outfmt,"%msg:F,58:2%\n"
:msg, contains, "msgnum:" action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")'
startup
# 100 messages are enough - the question is if the include is read ;)
injectmsg 0 100
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown
seq_check 0 99
exit_test
