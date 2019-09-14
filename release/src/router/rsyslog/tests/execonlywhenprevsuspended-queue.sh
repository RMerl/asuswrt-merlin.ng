#!/bin/bash
# rgerhards, 2015-05-27
echo =====================================================================================
echo \[execonlywhenprevsuspended-queue.sh\]: test execonly...suspended functionality with action on its own queue

uname
if [ `uname` = "SunOS" ] ; then
   echo "This test currently does not work on all flavors of Solaris."
   exit 77
fi

. $srcdir/diag.sh init
generate_conf
add_conf '
main_queue(queue.workerthreads="1") 

# omtesting provides the ability to cause "SUSPENDED" action state
module(load="../plugins/omtesting/.libs/omtesting")

$MainMsgQueueTimeoutShutdown 100000
template(name="outfmt" type="string" string="%msg:F,58:2%\n")

:msg, contains, "msgnum:" {
	:omtesting:fail 2 0 # omtesting has only legacy params!
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt"
	       queue.type="linkedList"
	       action.ExecOnlyWhenPreviousIsSuspended="on"
	      )
}
'
startup
injectmsg 0 1000
shutdown_when_empty
wait_shutdown
seq_check 1 999
exit_test
