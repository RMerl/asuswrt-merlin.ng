#!/bin/bash
# basic test for looping over json object and unsetting it while inside the loop-body
# added 2016-03-31 by singh.janmejay
# This file is part of the rsyslog project, released under ASL 2.0
. $srcdir/diag.sh init

uname
if [ `uname` = "FreeBSD" ] ; then
   echo "This test currently does not work on FreeBSD."
   exit 77
fi
generate_conf
add_conf '
template(name="corge" type="string" string="corge: key: %$.corge!key% val: %$.corge!value%\n")
template(name="quux" type="string" string="quux: %$.quux%\n")
template(name="post_suicide_foo" type="string" string="post_suicide_foo: '
add_conf "'%\$!foo%'"
add_conf '\n")

module(load="../plugins/mmjsonparse/.libs/mmjsonparse")
module(load="../plugins/imptcp/.libs/imptcp")
input(type="imptcp" port="'$TCPFLOOD_PORT'")

action(type="mmjsonparse")
set $.garply = "";

foreach ($.quux in $!foo) do {
  if ($.quux!key == "str2") then {
    set $.quux!random_key = $.quux!key;
		unset $!foo; #because it is deep copied, the foreach loop will continue to work, but the action to print "post_sucide_foo" will not see $!foo
	}
  action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="quux")
  foreach ($.corge in $.quux!value) do {
    action(type="omfile" file="'$RSYSLOG_DYNNAME'.out.async.log" template="corge" queue.type="linkedlist" action.copyMsg="on")
  }
}
action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="post_suicide_foo")
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
content_check 'quux: { "key": "obj", "value": { "bar": { "k1": "important_msg", "k2": "other_msg" } } }'
custom_content_check 'corge: key: bar val: { "k1": "important_msg", "k2": "other_msg" }' $RSYSLOG_DYNNAME.out.async.log
content_check "post_suicide_foo: ''"
exit_test
