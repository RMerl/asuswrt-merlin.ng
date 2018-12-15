#!/bin/bash
# add 2017-02-09 by Jan Gerhards, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

set $.n = "-1";
set $!ip!v1 = 1 + $.n;

template(name="outfmt" type="string" string="%!ip%\n")
local4.* action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup
tcpflood -m1
shutdown_when_empty
wait_shutdown
echo '{ "v1": 0 }' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid function output detected, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit 1
fi;
exit_test
