#!/bin/bash
# Test for automatic creation of dynafile directories
# note that we use the "'${RSYSLOG_DYNNAME}'.spool" directory, because it is handled by diag.sh
# in any case, so we do not need to add any extra new test dir.
# added 2009-11-30 by Rgerhards
# This file is part of the rsyslog project, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
# set spool locations and switch queue to disk-only mode
$WorkDirectory '$RSYSLOG_DYNNAME'.spool
$MainMsgQueueFilename mainq
$MainMsgQueueType disk

$template dynfile,"'$RSYSLOG_DYNNAME'.logdir/'$RSYSLOG_OUT_LOG'"
*.* ?dynfile
'
startup
injectmsg  0 1 # a single message is sufficient
shutdown_when_empty # shut down rsyslogd when done processing messages
wait_shutdown
if [ ! -e $RSYSLOG_DYNNAME.logdir/$RSYSLOG_OUT_LOG ]
then
	echo "$RSYSLOG_DYNNAME.logdir or logfile not created!"
	error_exit 1
fi
exit_test
