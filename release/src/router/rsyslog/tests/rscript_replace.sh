#!/bin/bash
# added 2014-10-31 by singh.janmejay
# This file is part of the rsyslog project, released under ASL 2.0
echo ===============================================================================
echo \[rscript_replace.sh\]: test for replace script-function
. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="outfmt" type="string" string="%$.replaced_msg%\n")

module(load="../plugins/imptcp/.libs/imptcp")
input(type="imptcp" port="'$TCPFLOOD_PORT'")

template(name="date_time" type="list") {
  property(name="msg" regex.Expression="Thu .+ 2014" regex.Type="ERE" regex.Match="0")
}

set $.replaced_msg = replace("date time: " & exec_template("date_time"), "O" & "ct", replace("october", "o", "0"));

action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup
tcpflood -m 1 -I $srcdir/testsuites/date_time_msg
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown 
content_check  "date time: Thu 0ct0ber 30 13:20:18 IST 2014"
exit_test
