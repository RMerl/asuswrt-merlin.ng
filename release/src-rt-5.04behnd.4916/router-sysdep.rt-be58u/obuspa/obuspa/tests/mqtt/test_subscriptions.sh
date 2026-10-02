#!/bin/bash
CURRENT_DIR="$(dirname $0)"
source "$CURRENT_DIR"/shared.sh || exit 1

rm -rf "$LOG_FILE"
rm -rf "$GREP_FILE"
rm -rf "$DB_FILE"

trap cleanup EXIT
trap cleanup SIGINT

stop_obuspa
sleep 1
start_obuspa -m
obuspa_cmd -c dump mdelta
NO_LEAK_STRING=$(obuspa_cmd -c dump mdelta)
techo
techo "TEST START STOP"

add_client 1
verify_client 1

add_local_agent_mtp 1
add_local_agent_controller 1
add_local_agent_controller_mtp 1

configure_client "1" "mqtt.eclipse.org" "1883" "/usp/endpoint/#" "/usp/controller"
enable_client 1
sleep 1

# Verify enable works by checking the logs
# NB escaped characters in this grep
grep '\-\-> Running' -- $GREP_FILE || fail "Failed to enable client"
cat $GREP_FILE >> $LOG_FILE
echo "" > $GREP_FILE

techo "Setup finished."

obuspa_cmd -c get Device.

obuspa_cmd -c add Device.MQTT.Client.1.Subscription
obuspa_cmd -c add Device.MQTT.Client.1.Subscription
obuspa_cmd -c add Device.MQTT.Client.1.Subscription
obuspa_cmd -c add Device.MQTT.Client.1.Subscription
obuspa_cmd -c add Device.MQTT.Client.1.Subscription
obuspa_cmd -c get Device.MQTT.Client.1.Subscription.
obuspa_cmd -c del Device.MQTT.Client.1.Subscription.5
obuspa_cmd -c add Device.MQTT.Client.1.Subscription

obuspa_cmd -c set Device.MQTT.Client.1.Subscription.1.Topic "/usp/topic1" || fail
obuspa_cmd -c set Device.MQTT.Client.1.Subscription.2.Topic "/usp/topic2" || fail
obuspa_cmd -c set Device.MQTT.Client.1.Subscription.3.Topic "/usp/topic3" || fail
obuspa_cmd -c set Device.MQTT.Client.1.Subscription.4.Topic "/usp/topic4" || fail
obuspa_cmd -c set Device.MQTT.Client.1.Subscription.6.Topic "/usp/topic5" || fail


obuspa_cmd -c set Device.MQTT.Client.1.Subscription.3.Enable true
sleep 1
obuspa_cmd -c set Device.MQTT.Client.1.Subscription.3.Topic "/usp/newtopic"
sleep 1
grep 'Resubscribing' -- $GREP_FILE || fail "Failed to switch topic"
cat $GREP_FILE >> $LOG_FILE
echo "" > $GREP_FILE

obuspa_cmd -c set Device.MQTT.Client.1.Subscription.3.Enable false
sleep 1
grep 'UNSUBACK' -- $GREP_FILE || fail "Failed to unsubscribe"
cat $GREP_FILE >> $LOG_FILE; echo "" > $GREP_FILE

obuspa_cmd -c set Device.MQTT.Client.1.Subscription.3.Enable true
sleep 1
grep 'd SUBACK' -- $GREP_FILE || fail "Failed to unsubscribe"
cat $GREP_FILE >> $LOG_FILE; echo "" > $GREP_FILE


obuspa_cmd -c add Device.MQTT.Client
obuspa_cmd -c add Device.MQTT.Client
obuspa_cmd -c add Device.MQTT.Client
obuspa_cmd -c add Device.MQTT.Client
obuspa_cmd -c del Device.MQTT.Client.3
obuspa_cmd -c add Device.MQTT.Client || fail

obuspa_cmd -c set Device.MQTT.Client.1.Subscription.3.Enable false

disable_client 1
obuspa_cmd -c del Device.LocalAgent.MTP.1.
obuspa_cmd -c del Device.LocalAgent.Controller.1.

stop_obuspa
grep '0 memory allocations' -- $GREP_FILE || fail "Memory leaks seen."
