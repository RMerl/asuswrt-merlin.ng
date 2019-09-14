#!/bin/bash
. $srcdir/diag.sh init
generate_conf
env|grep src
echo ac_top_srcdir: $ac_top_srcdir
echo FULL ENV:
env
echo FS info:
set -x
pwd
echo "srcdir: $srcdir"
ls -l ${srcdir}/testsuites/incltest.d/
ls -l ${srcdir}/testsuites
find ../../../.. -name incltest.d
set +x
add_conf "\$IncludeConfig ${srcdir}/testsuites/incltest.d/
"
startup
# 100 messages are enough - the question is if the include is read ;)
injectmsg 0 100
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown
seq_check 0 99
exit_test
