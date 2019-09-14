#!/bin/bash
# added 2016-03-31 by singh.janmejay
# This file is part of the rsyslog project, released under ASL 2.0

uname
if [ `uname` = "FreeBSD" ] ; then
   echo "This test currently does not work on FreeBSD."
   exit 77
fi

echo ===============================================================================
echo \[json_object_looping-vg.sh\]: basic test for looping over json object / associative-array with valgrind
. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="garply" type="string" string="garply: %$.garply%\n")
template(name="corge" type="string" string="corge: key: %$.corge!key% val: %$.corge!value%\n")
template(name="prefixed_corge" type="string" string="prefixed_corge: %$.corge%\n")
template(name="quux" type="string" string="quux: %$.quux%\n")
template(name="modified" type="string" string="new: %$!foo!str4% deleted: %$!foo!str3%\n")

module(load="../plugins/mmjsonparse/.libs/mmjsonparse")
module(load="../plugins/imptcp/.libs/imptcp")
input(type="imptcp" port="'$TCPFLOOD_PORT'")

action(type="mmjsonparse")
set $.garply = "";

ruleset(name="prefixed_writer" queue.type="linkedlist" queue.workerthreads="5") {
  action(type="omfile" file="'$RSYSLOG_DYNNAME'.out.prefixed.log" template="prefixed_corge" queue.type="linkedlist")
}

foreach ($.quux in $!foo) do {
  if ($.quux!key == "str2") then {
    set $.quux!random_key = $.quux!key;
		unset $!foo!str3; #because it is deep copied, the foreach loop will still see str3, but the "modified" action in the bottom will not
		set $!foo!str4 = "jkl3";
	}
  action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="quux")
  foreach ($.corge in $.quux!value) do {
    action(type="omfile" file="'$RSYSLOG_DYNNAME'.out.async.log" template="corge" queue.type="linkedlist" action.copyMsg="on")
    call prefixed_writer

    foreach ($.grault in $.corge!value) do {
      if ($.garply != "") then
        set $.garply = $.garply & ", ";
      set $.garply = $.garply & $.grault!key & "=" & $.grault!value;
    }
  }
}
action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="garply")
action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="modified")
'
startup_vg
tcpflood -m 1 -I $srcdir/testsuites/json_object_input
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown_vg
check_exit_vg
content_check 'quux: { "key": "str1", "value": "abc0" }'
content_check 'quux: { "key": "str2", "value": "def1", "random_key": "str2" }'
content_check 'quux: { "key": "str3", "value": "ghi2" }'
assert_content_missing 'quux: { "key": "str4", "value": "jkl3" }'
content_check 'new: jkl3'
assert_content_missing 'deleted: ghi2'
content_check 'quux: { "key": "obj", "value": { "bar": { "k1": "important_msg", "k2": "other_msg" } } }'
custom_content_check 'corge: key: bar val: { "k1": "important_msg", "k2": "other_msg" }' $RSYSLOG_DYNNAME.out.async.log
custom_content_check 'prefixed_corge: { "key": "bar", "value": { "k1": "important_msg", "k2": "other_msg" } }' $RSYSLOG_DYNNAME.out.prefixed.log
content_check 'garply: k1=important_msg, k2=other_msg'
exit_test
