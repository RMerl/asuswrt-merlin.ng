#!/bin/bash
# Added 2017-12-09 by Rainer Gerhards, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")
template(name="outfmt" type="string" string="%$!%\n")

local4.* {
	set $.ret = parse_json("{ \"c1\":\"data\" }", "\$!parsed");
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
}
'

startup
tcpflood -m1
shutdown_when_empty
wait_shutdown

# Our fixed and calculated expected results
EXPECTED='{ "parsed": { "c1": "data" } }'
echo $EXPECTED | cmp - $RSYSLOG_OUT_LOG
if [[ $? -ne 0 ]]; then
  printf "Invalid function output detected!\n"
  printf "expected:\n$EXPECTED\n"
  printf "rsyslog.out is:\n"
  cat $RSYSLOG_OUT_LOG
  error_exit 1
fi;

exit_test
