#!/bin/bash
# added 2015-09-29 by singh.janmejay
# This file is part of the rsyslog project, released under ASL 2.0
echo ===============================================================================
echo \[rscript_re_match.sh\]: test re_match rscript-fn
. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="outfmt" type="string" string="*Matched*\n")

module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")'
add_conf "
if (re_match(\$msg, '.* ([0-9]+)$')) then {"
add_conf '
	 action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
}
'
startup
tcpflood -m 1 -I $srcdir/testsuites/date_time_msg
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown
content_check "*Matched*"
exit_test
