#!/bin/bash
# This is test case from practice, with the version we introduced it, it
# caused a deadlock during processing (when the a stream was purged from the
# dynafile cache).
# We added this as a standard test in the hopes that iw will help
# detect such things in the future.
#
# The problem that originally caused this test to fail was:
# We write to files asynchronously (with the async writer thread). There is
# no signaling done when the file stream is closed. That can lead to the writer
# process hanging in memory, that in turn leads to the main thread waiting on a
# condition that never occurs (because it would need to be signalled by the
# async writer). Even worse, in that case, the async writer was signalled invalid
# in such a way that when it received a wakeup, it thought it shall not terminate,
# but received a spurios wakeup due to timeout and no data to write. In that case
# it (correctly) concluded that it would not need to timeout until a new buffer write
# was done (in which case it would receive a wakeup). As such, it went into an eternal
# wait. However, the invalid signaling did not take into account that it did not
# signal the async writer to shut down. So the main thread went into a condition
# wait - and thus we had a deadlock. That situation occured only under very specific
# cirumstances. As far as the analysis goes, the following need to happen:
# 1. buffers on that file are being flushed
# 2. no new data arrives
# 3. the inactivity timeout has not yet expired
# 4. *then* (and only then) the stream is closed or destructed
# In that, 1 to 4 are prequisites for the deadlock which will happen in 4. However,
# for it to happen, we also need the right "timing". There is a race between the
# main thread and the async writer thread. The deadlock will only happen under
# the "right" circumstances, which basically means it will not happen always.
# In order to create this case as reliable as possible, I have used
# the "$OMFileFlushOnTXEnd on" directive
# inside my test case. It makes sure that #1 above happens. The test uses a dynafile
# cache size of 4, and the load generator generates data for 5 different dynafiles.
# So over time, we will hit a spot where 4 dynafiles are open and the 5th file name
# is generated. As such, one file needs to be discarded. Thanks to FlushOnTXEnd, we
# now likely have #2 in place and thanks to the load pattern generated, we most
# probably have #3 in place. During the dynafile cache displacement of the oldest
# entry, #4 is generated. At this point, we have the deadlock we are testing for.
# Note that this deadlock does not necessarily lead to a total lockup of rsyslogd.
# Parts of it continue to operate. But in our test setup, this means data is
# received and placed into the main queue. Once it's high water mark is hit, data 
# is still being enqueued, but at a slow rate. So if one is patient enough, the load
# generator will be able to finish. However, rsyslogd will never process the data
# it received because it is locked in the deadlock caused by #4 above.
# Note that "$OMFileFlushOnTXEnd on" is not causing this behaviour. We just use it
# to (quite) reliably cause the failure condition. The failure described above
# (in version 4.6.1) was also present when the setting was set to "off", but its
# occurence was very much less probable - because the perquisites are then much
# harder to hit. without it, the test may need to run for several hours before
# we hit all failure conditions.
#
# added 2010-03-17 by Rgerhards
# This file is part of the rsyslog project, released under ASL 2.0
. $srcdir/diag.sh init
generate_conf
add_conf '
$ModLoad ../plugins/imtcp/.libs/imtcp
$MainMsgQueueTimeoutShutdown 10000
$InputTCPServerRun '$TCPFLOOD_PORT'

$template outfmt,"%msg:F,58:3%,%msg:F,58:4%,%msg:F,58:5%\n"
$template dynfile,"'$RSYSLOG_DYNNAME'.%msg:F,58:2%.log" # use multiple dynafiles

$OMFileFlushOnTXEnd on
$OMFileFlushInterval 10
$OMFileIOBufferSize 10k
$OMFileAsyncWriting on
$DynaFileCacheSize 4
local0.* ?dynfile;outfmt
'
# uncomment for debugging support:
#export RSYSLOG_DEBUG="debug nostdout noprintmutexaction"
#export RSYSLOG_DEBUGLOG="log"
startup
# send 20000 messages, each close to 2K (non-randomized!), so that we can fill
# the buffers and hopefully run into the "deadlock".
tcpflood -m20000 -d1800 -P129 -i1 -f5
shutdown_when_empty
wait_shutdown
cat $RSYSLOG_DYNNAME.*.log > $RSYSLOG_OUT_LOG
seq_check 1 20000 -E
rm -f $RSYSLOG_DYNNAME.*.log
exit_test
