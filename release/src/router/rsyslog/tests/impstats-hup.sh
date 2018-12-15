#!/bin/bash
# test if HUP works for impstats
# This file is part of the rsyslog project, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/impstats/.libs/impstats"
	log.file=`echo $RSYSLOG_OUT_LOG`
	interval="1" ruleset="stats")

ruleset(name="stats") {
	stop # nothing to do here
}
'
startup
./msleep 2000
mv  $RSYSLOG_OUT_LOG ${RSYSLOG2_OUT_LOG}
issue_HUP
./msleep 2000
shutdown_when_empty
wait_shutdown
echo checking pre-HUP file
content_check 'global: origin=dynstats' ${RSYSLOG2_OUT_LOG}
echo checking post-HUP file
content_check 'global: origin=dynstats' $RSYSLOG_OUT_LOG
exit_test
