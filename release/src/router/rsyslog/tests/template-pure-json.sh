#!/bin/bash
# added 2018-02-10 by Rainer Gerhards; Released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="outfmt" type="list" option.jsonf="on") {
	 property(outname="message" name="msg" format="jsonf")
	 constant(outname="@version" value="1" format="jsonf")
}

local4.* action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup
injectmsg 0 1
shutdown_when_empty
wait_shutdown
EXPECTED='{"message":" msgnum:00000000:", "@version": "1"}'
cmp_exact $RSYSLOG_OUT_LOG
exit_test
