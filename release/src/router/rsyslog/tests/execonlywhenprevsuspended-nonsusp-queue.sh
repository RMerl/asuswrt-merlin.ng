#!/bin/bash
# check if execonly...suspended works when the first action is *not*
# suspended --> file1 must be created, file 2 not
# rgerhards, 2015-05-27
echo =====================================================================================
echo \[execonlywhenprevsuspended-nonsusp-queue\]: test execonly...suspended functionality with non-suspended action and queue

. $srcdir/diag.sh init
generate_conf
add_conf '
main_queue(queue.workerthreads="1") 

# omtesting provides the ability to cause "SUSPENDED" action state
module(load="../plugins/omtesting/.libs/omtesting")

$MainMsgQueueTimeoutShutdown 100000
template(name="outfmt" type="string" string="%msg:F,58:2%\n")

:msg, contains, "msgnum:" {
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG`
	       template="outfmt" name="ok")
	action(type="omfile" file=`echo $RSYSLOG2_OUT_LOG`
	       template="outfmt" name="susp"
	       queue.type="linkedList"
	       action.ExecOnlyWhenPreviousIsSuspended="on"
	      )
}
'
startup
injectmsg 0 1000
shutdown_when_empty
wait_shutdown
ls *.out.log
seq_check 0 999
if [ -e ${RSYSLOG2_OUT_LOG} ]; then
    echo "error: \"suspended\" file exists, first 10 lines:"
    $RS_HEADCMD ${RSYSLOG2_OUT_LOG}
    exit 1
fi
exit_test
