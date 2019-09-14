#!/bin/bash
# added 2015-03-02 by singh.janmejay
# test to assert attempt to iterate upon a non-array json-object fails gracefully
# This file is part of the rsyslog project, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="garply" type="string" string="garply: %$.garply%\n")
template(name="grault" type="string" string="grault: %$.grault%\n")
template(name="prefixed_grault" type="string" string="prefixed_grault: %$.grault%\n")
template(name="quux" type="string" string="quux: %$.quux%\n")

module(load="../plugins/mmjsonparse/.libs/mmjsonparse")
module(load="../plugins/imptcp/.libs/imptcp")
input(type="imptcp" port="'$TCPFLOOD_PORT'")

action(type="mmjsonparse")
set $.garply = "";

ruleset(name="prefixed_writer" queue.type="linkedlist" queue.workerthreads="5") {
  action(type="omfile" file="'$RSYSLOG_DYNNAME'.out.prefixed.log" template="prefixed_grault" queue.type="linkedlist")
}

foreach ($.quux in $!foo) do {
  action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="quux")
  foreach ($.corge in $.quux!bar) do {
     reset $.grault = $.corge;
     action(type="omfile" file="'$RSYSLOG_DYNNAME'.out.async.log" template="grault" queue.type="linkedlist" action.copyMsg="on")
     call prefixed_writer
     if ($.garply != "") then
         set $.garply = $.garply & ", ";
     reset $.garply = $.garply & $.grault!baz;
  }
}
action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="garply")
'
startup
tcpflood -m 1 -I $srcdir/testsuites/json_nonarray_input
shutdown_when_empty
wait_shutdown
assert_content_missing 'quux'
exit_test
