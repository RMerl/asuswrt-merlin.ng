#!/bin/bash
# check if CEE properties are properly saved & restored to/from disk queue
# added 2012-09-19 by rgerhards
# This file is part of the rsyslog project, released under ASL 2.0
echo ===============================================================================
echo \[cee_diskqueue.sh\]: CEE and diskqueue test

uname
if [ `uname` = "SunOS" ] ; then
   echo "This test currently does not work on all flavors of Solaris."
   exit 77
fi

. $srcdir/diag.sh init
generate_conf
add_conf '
global(workDirectory="/tmp")
template(name="outfmt" type="string" string="%$!usr!msg:F,58:2%\n")

set $!usr!msg = $msg;
if $msg contains '
add_conf "'msgnum' "
add_conf 'then
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt"
	       queue.type="disk" queue.filename="rsyslog-act1")
'
startup
injectmsg  0 5000
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown 
seq_check  0 4999
exit_test
