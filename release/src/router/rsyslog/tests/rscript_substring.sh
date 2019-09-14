#!/bin/bash
# add 2017-12-10 by Jan Gerhards, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

set $!str!var1 = substring("", 0, 0);
set $!str!var2 = substring("test", 0, 4);
set $!str!var3 = substring("test", 1, 2);
set $!str!var4 = substring("test", 4, 2);
set $!str!var5 = substring("test", 0, 5);
set $!str!var6 = substring("test", 0, 6);
set $!str!var7 = substring("test", 3, 4);
set $!str!var8 = substring("test", 1, 0);

template(name="outfmt" type="string" string="%!str%\n")
local4.* action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup
tcpflood -m1 -y
shutdown_when_empty
wait_shutdown
echo '{ "var1": "", "var2": "test", "var3": "es", "var4": "", "var5": "test", "var6": "test", "var7": "t", "var8": "" }' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid function output detected, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit 1
fi;
exit_test

