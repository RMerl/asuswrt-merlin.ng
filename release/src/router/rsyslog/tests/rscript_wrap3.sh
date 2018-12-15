#!/bin/bash
# added 2014-11-03 by singh.janmejay
# This file is part of the rsyslog project, released under ASL 2.0
echo ===============================================================================
echo \[rscript_wrap3.sh\]: a test for wrap\(3\) script-function
. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="outfmt" type="string" string="%$.replaced_msg%\n")

module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

set $.replaced_msg = wrap("foo says" & $msg, "bc" & "def" & "bc", "ES" & "C");

action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup
tcpflood -m 1 -I $srcdir/testsuites/wrap3_input
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown 
content_check "bcdefbcfoo says a abcESCdefb has ESCbcdefbc"
exit_test
