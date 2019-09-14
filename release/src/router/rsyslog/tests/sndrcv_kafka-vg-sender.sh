#!/bin/bash
# added 2017-05-03 by alorbach
# This file is part of the rsyslog project, released under ASL 2.0
export TESTMESSAGES=1000
export TESTMESSAGESFULL=1000
# enable the EXTRA_EXITCHECK only if really needed - otherwise spams the test log
# too much
# export EXTRA_EXITCHECK=dumpkafkalogs
echo Check and Stop previous instances of kafka/zookeeper 
. $srcdir/diag.sh download-kafka
. $srcdir/diag.sh stop-zookeeper
. $srcdir/diag.sh stop-kafka

echo Init Testbench
. $srcdir/diag.sh init

echo Create kafka/zookeeper instance and topics
. $srcdir/diag.sh start-zookeeper
. $srcdir/diag.sh start-kafka
. $srcdir/diag.sh create-kafka-topic 'static' '.dep_wrk' '22181'

echo Give Kafka some time to process topic create ...
sleep 5

echo Starting receiver instance [imkafka]
export RSYSLOG_DEBUGLOG="log"
generate_conf
add_conf '
module(load="../plugins/imkafka/.libs/imkafka")
/* Polls messages from kafka server!*/
input(	type="imkafka"
	topic="static"
	broker="localhost:29092"
	consumergroup="default"
	confParam=[ "compression.codec=none",
		"socket.timeout.ms=5000",
		"socket.keepalive.enable=true"]
	)

template(name="outfmt" type="string" string="%msg:F,58:2%\n")

if ($msg contains "msgnum:") then {
	action( type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt" )
}
'
startup

echo Starting sender instance [omkafka]
export RSYSLOG_DEBUGLOG="log2"
generate_conf 2
add_conf '
main_queue(queue.timeoutactioncompletion="10000" queue.timeoutshutdown="60000")
$imdiagInjectDelayMode full

module(load="../plugins/omkafka/.libs/omkafka")

template(name="outfmt" type="string" string="%msg%\n")

action(	name="kafka-fwd"
	type="omkafka"
	topic="static"
	broker="localhost:29092"
	template="outfmt"
	confParam=[	"compression.codec=none",
			"socket.timeout.ms=5000",
			"socket.keepalive.enable=true",
			"reconnect.backoff.jitter.ms=1000",
			"queue.buffering.max.messages=20000",
			"message.send.max.retries=1"]
	topicConfParam=["message.timeout.ms=5000"]
	partitions.auto="on"
	resubmitOnFailure="on"
	keepFailedMessages="on"
	failedMsgFile="'$RSYSLOG_OUT_LOG'-failed-'$RANDTOPIC'.data"
	action.resumeInterval="2"
	action.resumeRetryCount="10"
	queue.saveonshutdown="on"
	)
' 2
startup_vg 2

echo Inject messages into rsyslog sender instance
injectmsg 1 $TESTMESSAGES

echo Sleep to give rsyslog instances time to process data ...
sleep 5

echo Stopping sender instance [omkafka]
shutdown_when_empty 2
wait_shutdown_vg 2
check_exit_vg 2

# TODO: REMOVE (2018-09-13 rgerhards)
#echo Sleep to give rsyslog receiver time to receive data ...
#sleep 20

echo Stopping receiver instance [imkafka]
kafka_wait_group_coordinator
shutdown_when_empty
wait_shutdown

echo delete kafka topics
. $srcdir/diag.sh delete-kafka-topic 'static' '.dep_wrk' '22181'

# Do the final sequence check
seq_check 1 $TESTMESSAGESFULL -d

echo stop kafka instance
. $srcdir/diag.sh stop-kafka

# STOP ZOOKEEPER in any case
. $srcdir/diag.sh stop-zookeeper

echo success
exit_test
