#!/bin/bash
# a very basic test for omjournal. Right now, we have no
# reliable way of verifying that data was actually written
# to the journal, but at least we check that rsyslog does
# not abort when trying to use omjournal. Not high tech,
# but better than nothing.
# addd 2016-03-16 by RGerhards, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/omjournal/.libs/omjournal")

action(type="omjournal")
'
startup
./msleep 500
shutdown_when_empty
wait_shutdown
# if we reach this, we have at least not aborted
exit_test
