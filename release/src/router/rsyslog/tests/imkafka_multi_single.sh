#!/bin/bash
# added 2018-08-29 by alorbach
# This file is part of the rsyslog project, released under ASL 2.0
export TESTMESSAGES=100000
export TESTMESSAGESFULL=100000

# Generate random topic name
export RANDTOPIC=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 8 | head -n 1)

echo Init Testbench 
. $srcdir/diag.sh init

# Check for kafkacat
check_command_available kafkacat

# enable the EXTRA_EXITCHECK only if really needed - otherwise spams the test log
# too much
#export EXTRA_EXITCHECK=dumpkafkalogs
echo ===============================================================================
echo Create kafka/zookeeper instance and $RANDTOPIC topic
. $srcdir/diag.sh download-kafka
. $srcdir/diag.sh stop-zookeeper
. $srcdir/diag.sh stop-kafka

echo Create kafka/zookeeper instance and topics
. $srcdir/diag.sh start-zookeeper
. $srcdir/diag.sh start-kafka

echo Give Kafka some time to sync...
sleep 5

# create new topic
. $srcdir/diag.sh create-kafka-topic $RANDTOPIC '.dep_wrk' '22181'

echo Give Kafka some time to process topic create ...
sleep 5

# --- Create imkafka receiver config
export RSYSLOG_DEBUGLOG="log"
generate_conf
add_conf '
main_queue(queue.timeoutactioncompletion="60000" queue.timeoutshutdown="60000")

module(load="../plugins/imkafka/.libs/imkafka")
/* Polls messages from kafka server!*/
input(	type="imkafka" 
	topic="'$RANDTOPIC'" 
	broker="localhost:29092"
	consumergroup="default1"
	confParam=[ "compression.codec=none",
		"session.timeout.ms=10000",
		"socket.timeout.ms=5000",
		"socket.keepalive.enable=true",
		"reconnect.backoff.jitter.ms=1000",
		"enable.partition.eof=false" ]
	)

input(	type="imkafka" 
	topic="'$RANDTOPIC'" 
	broker="localhost:29092"
	consumergroup="default2"
	confParam=[ "compression.codec=none",
		"session.timeout.ms=10000",
		"socket.timeout.ms=5000",
		"socket.keepalive.enable=true",
		"reconnect.backoff.jitter.ms=1000",
		"enable.partition.eof=false" ]
	)

input(	type="imkafka" 
	topic="'$RANDTOPIC'" 
	broker="localhost:29092"
	consumergroup="default3"
	confParam=[ "compression.codec=none",
		"session.timeout.ms=10000",
		"socket.timeout.ms=5000",
		"socket.keepalive.enable=true",
		"reconnect.backoff.jitter.ms=1000",
		"enable.partition.eof=false" ]
	)

input(	type="imkafka" 
	topic="'$RANDTOPIC'" 
	broker="localhost:29092"
	consumergroup="default4"
	confParam=[ "compression.codec=none",
		"session.timeout.ms=10000",
		"socket.timeout.ms=5000",
		"socket.keepalive.enable=true",
		"reconnect.backoff.jitter.ms=1000",
		"enable.partition.eof=false" ]
	)

input(	type="imkafka" 
	topic="'$RANDTOPIC'" 
	broker="localhost:29092"
	consumergroup="default5"
	confParam=[ "compression.codec=none",
		"session.timeout.ms=10000",
		"socket.timeout.ms=5000",
		"socket.keepalive.enable=true",
		"reconnect.backoff.jitter.ms=1000",
		"enable.partition.eof=false" ]
	)

input(	type="imkafka" 
	topic="'$RANDTOPIC'" 
	broker="localhost:29092"
	consumergroup="default6"
	confParam=[ "compression.codec=none",
		"session.timeout.ms=10000",
		"socket.timeout.ms=5000",
		"socket.keepalive.enable=true",
		"reconnect.backoff.jitter.ms=1000",
		"enable.partition.eof=false" ]
	)

input(	type="imkafka" 
	topic="'$RANDTOPIC'" 
	broker="localhost:29092"
	consumergroup="default7"
	confParam=[ "compression.codec=none",
		"session.timeout.ms=10000",
		"socket.timeout.ms=5000",
		"socket.keepalive.enable=true",
		"reconnect.backoff.jitter.ms=1000",
		"enable.partition.eof=false" ]
	)

input(	type="imkafka" 
	topic="'$RANDTOPIC'" 
	broker="localhost:29092"
	consumergroup="default8"
	confParam=[ "compression.codec=none",
		"session.timeout.ms=10000",
		"socket.timeout.ms=5000",
		"socket.keepalive.enable=true",
		"reconnect.backoff.jitter.ms=1000",
		"enable.partition.eof=false" ]
	)

template(name="outfmt" type="string" string="%msg:F,58:2%\n")

if ($msg contains "msgnum:") then {
	action( type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt" )
}
'

# Start imkafka receiver config
echo Starting receiver instance [imkafka]
startup
# --- 

# Messure Starttime
TIMESTART=$(date +%s.%N)

# --- Fill Kafka Server with messages
# Can properly be done in a better way?!
for i in {00000001..00100000}
do
	echo " msgnum:$i" >> $RSYSLOG_OUT_LOG.in
done

echo Inject messages into kafka
cat $RSYSLOG_OUT_LOG.in | kafkacat -P -b localhost:29092 -t $RANDTOPIC
# --- 

echo Give imkafka some time to start...
sleep 5

echo Stopping sender instance [omkafka]
shutdown_when_empty
wait_shutdown

# Messure Endtime
TIMEEND=$(date +%s.%N)
TIMEDIFF=$(echo "$TIMEEND - $TIMESTART" | bc)
echo "*** imkafka time to process all data: $TIMEDIFF seconds!"

# Delete topic to remove old traces before
. $srcdir/diag.sh delete-kafka-topic $RANDTOPIC '.dep_wrk' '22181'

echo \[sndrcv_kafka.sh\]: stop kafka instances
. $srcdir/diag.sh stop-kafka
. $srcdir/diag.sh stop-zookeeper

# Do the final sequence check
seq_check 1 $TESTMESSAGESFULL -d

echo success
exit_test
