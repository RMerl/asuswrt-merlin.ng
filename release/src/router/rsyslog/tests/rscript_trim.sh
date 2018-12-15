#!/bin/bash
# add 2017-08-14 by Jan Gerhards, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

set $!str!l1 = ltrim("");
set $!str!l2 = ltrim("test");
set $!str!l3 = ltrim("   test");
set $!str!l4 = ltrim("test   ");
set $!str!l5 = ltrim("   test   ");
set $!str!l6 = ltrim(" test");
set $!str!l7 = ltrim("test ");
set $!str!l8 = ltrim(" ");
set $!str!l9 = ltrim("te st");
set $!str!l10 = ltrim(" te st");
set $!str!l11 = ltrim(" a");
set $!str!l12 = ltrim("a ");

set $!str!r1 = rtrim("");
set $!str!r2 = rtrim("test");
set $!str!r3 = rtrim("   test");
set $!str!r4 = rtrim("test   ");
set $!str!r5 = rtrim("   test   ");
set $!str!r6 = rtrim(" test");
set $!str!r7 = rtrim("test ");
set $!str!r8 = rtrim(" ");
set $!str!r9 = rtrim("te st");
set $!str!r10 = rtrim("te st ");
set $!str!r11 = rtrim(" a");
set $!str!r12 = rtrim("a ");


set $!str!b1 = ltrim(" ");
set $!str!b1 = rtrim($!str!b1);

set $!str!b2 = ltrim(" test ");
set $!str!b2 = rtrim($!str!b2);

set $!str!b3 = ltrim("   test      ");
set $!str!b3 = rtrim($!str!b3);

set $!str!b4 = ltrim("te st");
set $!str!b4 = rtrim($!str!b4);

set $!str!b5 = rtrim(" ");
set $!str!b5 = ltrim($!str!b5);

set $!str!b6 = rtrim(" test ");
set $!str!b6 = ltrim($!str!b6);

set $!str!b7 = rtrim("   test      ");
set $!str!b7 = ltrim($!str!b7);

set $!str!b8 = rtrim("te st");
set $!str!b8 = ltrim($!str!b8);

set $!str!b9 = rtrim(ltrim("test"));
set $!str!b10 = rtrim(ltrim("te st"));
set $!str!b11 = rtrim(ltrim(" test"));
set $!str!b12 = rtrim(ltrim("test "));
set $!str!b13 = rtrim(ltrim(" test "));
set $!str!b14 = rtrim(ltrim(" te st "));

set $!str!b15 = ltrim(rtrim("test"));
set $!str!b16 = ltrim(rtrim("te st"));
set $!str!b17 = ltrim(rtrim(" test"));
set $!str!b18 = ltrim(rtrim("test "));
set $!str!b19 = ltrim(rtrim(" test "));
set $!str!b20 = ltrim(rtrim(" te st "));

template(name="outfmt" type="string" string="%!str%\n")
local4.* action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup
tcpflood -m1 -y
shutdown_when_empty
wait_shutdown
echo '{ "l1": "", "l2": "test", "l3": "test", "l4": "test   ", "l5": "test   ", "l6": "test", "l7": "test ", "l8": "", "l9": "te st", "l10": "te st", "l11": "a", "l12": "a ", "r1": "", "r2": "test", "r3": "   test", "r4": "test", "r5": "   test", "r6": " test", "r7": "test", "r8": "", "r9": "te st", "r10": "te st", "r11": " a", "r12": "a", "b1": "", "b2": "test", "b3": "test", "b4": "te st", "b5": "", "b6": "test", "b7": "test", "b8": "te st", "b9": "test", "b10": "te st", "b11": "test", "b12": "test", "b13": "test", "b14": "te st", "b15": "test", "b16": "te st", "b17": "test", "b18": "test", "b19": "test", "b20": "te st" }' | cmp - $RSYSLOG_OUT_LOG
if [ ! $? -eq 0 ]; then
  echo "invalid function output detected, $RSYSLOG_OUT_LOG is:"
  cat $RSYSLOG_OUT_LOG
  error_exit 1
fi;
exit_test

