#!/bin/bash
# This tests a memory leak we have seen when processing internal error
# message with the settings used in this test. We use imfile as it is
# easist to reproduce this way. Note that we are only interested in
# whether or not we have a leak, not any other functionality. Most
# importantly, we do not care if the error message appears or not. This
# is because it is not so easy to pick it up from the system log and other
# tests already cover this szenario.
# add 2017-05-10 by Rainer Gerhards, released under ASL 2.0

uname
if [ `uname` = "FreeBSD" ] ; then
   echo "This test currently does not work on FreeBSD."
   exit 77
fi

. $srcdir/diag.sh init
generate_conf
add_conf '
global(processInternalMessages="off")
$RepeatedMsgReduction on # keep this on because many distros have set it

module(load="../plugins/imfile/.libs/imfile") # mode="polling" pollingInterval="1")
input(type="imfile" File="./'$RSYSLOG_DYNNAME'.input" Tag="tag1" ruleset="ruleset1")

template(name="tmpl1" type="string" string="%msg%\n")
ruleset(name="ruleset1") {
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="tmpl1")
}
action(type="omfile" file=`echo $RSYSLOG2_OUT_LOG`)
'
startup_vg_waitpid_only
./msleep 500 # wait a bit so that the error message can be emitted
. $srcdir/diag.sh shutdown-immediate
wait_shutdown_vg

exit_test
