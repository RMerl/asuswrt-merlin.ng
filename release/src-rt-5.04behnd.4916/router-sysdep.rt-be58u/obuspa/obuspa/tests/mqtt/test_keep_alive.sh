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
start_obuspa
techo
techo "TEST 2"
techo "Verifying dynamic keep alive change"

add_client 1
verify_client 1

add_local_agent_mtp 1
add_local_agent_controller 1
add_local_agent_controller_mtp 1

configure_client "1" "mqtt.eclipse.org" "1883" "/usp/endpoint/#" "/usp/controller"
enable_client 1
sleep 5

# Verify enable works by checking the logs
# NB escaped characters in this grep
grep '\-\-> Running' -- $GREP_FILE || fail "Failed to enable client"
cat $GREP_FILE >> $LOG_FILE
echo "" > $GREP_FILE

techo "Setup finished."

# Client now enabled. Now change the keepalive.
obuspa_cmd -c set "Device.MQTT.Client.1.KeepAliveTime" "55"
sleep 5
grep '\-\-> Running' -- $GREP_FILE || fail "Failed to change keep alive"
cat $GREP_FILE >> $LOG_FILE
echo "" > $GREP_FILE

obuspa_cmd -c set "Device.MQTT.Client.1.KeepAliveTime" "5"
sleep 5
grep '\-\-> Running' -- $GREP_FILE || fail "Failed to change keep alive"
cat $GREP_FILE >> $LOG_FILE
echo "" > $GREP_FILE


# Client now enabled. Now change the keepalive.
obuspa_cmd -c set "Device.MQTT.Client.1.KeepAliveTime" "60"
sleep 5
grep '\-\-> Running' -- $GREP_FILE || fail "Failed to change keep alive"
cat $GREP_FILE >> $LOG_FILE
echo "" > $GREP_FILE

obuspa_cmd -c set "Device.MQTT.Client.1.KeepAliveTime" "10"
sleep 5
grep '\-\-> Running' -- $GREP_FILE || fail "Failed to change keep alive"
cat $GREP_FILE >> $LOG_FILE
echo "" > $GREP_FILE

