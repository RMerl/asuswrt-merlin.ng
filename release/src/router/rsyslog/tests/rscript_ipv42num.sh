#!/bin/bash
# add 2017-02-09 by Jan Gerhards, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

set $!ip!v1 = ip42num("0.0.0.0");
set $!ip!v2 = ip42num("0.0.0.1");
set $!ip!v3 = ip42num("0.0.1.0");
set $!ip!v4 = ip42num("0.1.0.0");
set $!ip!v5 = ip42num("1.0.0.0");
set $!ip!v6 = ip42num("0.0.0.135");
set $!ip!v7 = ip42num("1.1.1.1");
set $!ip!v8 = ip42num("225.33.1.10");
set $!ip!v9 = ip42num("172.0.0.1");
set $!ip!v10 = ip42num("255.255.255.255");
set $!ip!v11 = ip42num("1.0.3.45         ");
set $!ip!v12 = ip42num("      0.0.0.1");
set $!ip!v13 = ip42num("    0.0.0.1   ");

set $!ip!e1 = ip42num("a");
set $!ip!e2 = ip42num("");
set $!ip!e3 = ip42num("123.4.6.*");
set $!ip!e4 = ip42num("172.0.0.1.");
set $!ip!e5 = ip42num("172.0.0..1");
set $!ip!e6 = ip42num(".172.0.0.1");
set $!ip!e7 = ip42num(".17 2.0.0.1");


template(name="outfmt" type="string" string="%!ip%\n")
local4.* action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup
tcpflood -m1 -y
shutdown_when_empty
wait_shutdown
echo '{ "v1": 0, "v2": 1, "v3": 256, "v4": 65536, "v5": 16777216, "v6": 135, "v7": 16843009, "v8": 3777036554, "v9": 2885681153, "v10": 4294967295, "v11": 16778029, "v12": 1, "v13": 1, "e1": -1, "e2": -1, "e3": -1, "e4": -1, "e5": -1, "e6": -1, "e7": -1 }' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid function output detected, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit 1
fi;
exit_test

