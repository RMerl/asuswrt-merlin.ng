#!/bin/bash
CURRENT_DIR="$(dirname $0)"
source "$CURRENT_DIR"/shared.sh || exit 1
CERTS_ARGS="-t $(dirname $0)/certs.pem -a $(dirname $0)/client.pem"

rm -rf "$LOG_FILE"
rm -rf "$GREP_FILE"
rm -rf "$DB_FILE"

trap cleanup EXIT
trap cleanup SIGINT

stop_obuspa
sleep 1
start_obuspa $CERTS_ARGS
techo
techo "TEST SSL"

obuspa_cmd -c dump mdelta
NO_LEAK_STRING=$(obuspa_cmd -c dump mdelta)

add_client 1 2 3 4 5
verify_client 1 2 3 4 5

add_local_agent_mtp 1 2 3 4 5 || fail
add_local_agent_controller 1 2 3 4 5 || fail
add_local_agent_controller_mtp 1 2 3 4 5 || fail

configure_client "1" "mqtt.eclipse.org" "8883" "/usp/endpoint/#" "/usp/controller"
obuspa_cmd -c set "Device.MQTT.Client.1.TransportProtocol" "TLS"

configure_client "2" "test.mosquitto.org" "8883" "/usp/endpoint/#" "/usp/controller"
obuspa_cmd -c set "Device.MQTT.Client.2.TransportProtocol" "TLS"

configure_client "3" "test.mosquitto.org" "1883" "/usp/endpoint/#" "/usp/controller"

configure_client "4" "mqtt.eclipse.org" "8883" "/usp/endpoint/#" "/usp/controller"
obuspa_cmd -c set "Device.MQTT.Client.4.TransportProtocol" "TLS"

configure_client "5" "test.mosquitto.org" "8884" "/usp/endpoint/#" "/usp/controller"
obuspa_cmd -c set "Device.MQTT.Client.5.TransportProtocol" "TLS"

obuspa_cmd -c get Device.MQTT.Client.

# Finally, enable the all of the clients
enable_client 1 2 3 4 5
sleep 2

# Verify we have all 5 up and running, and saw now errors
[[ 5 == $(grep '\-\-> Running' -- "$GREP_FILE" | wc -l) ]] || fail "Not correct running clients"
grep -v -i 'Error' -- "$GREP_FILE" || fail "Found an error."

# Log and wipe grep file
cat "$GREP_FILE" >> $LOG_FILE
echo "" > "$GREP_FILE"


disable_client 1 2 3 4 5
obuspa_cmd -c del Device.LocalAgent.MTP.1.
obuspa_cmd -c del Device.LocalAgent.MTP.2.
obuspa_cmd -c del Device.LocalAgent.MTP.3.
obuspa_cmd -c del Device.LocalAgent.MTP.4.
obuspa_cmd -c del Device.LocalAgent.MTP.5.
obuspa_cmd -c del Device.LocalAgent.Controller.1.
obuspa_cmd -c del Device.LocalAgent.Controller.2.
obuspa_cmd -c del Device.LocalAgent.Controller.3.
obuspa_cmd -c del Device.LocalAgent.Controller.4.
obuspa_cmd -c del Device.LocalAgent.Controller.5.
del_client 1 2 3 4 5

mdelta_string="$(obuspa_cmd -c dump mdelta)"

if [[ "$mdelta_string" != "$NO_LEAK_STRING" ]]; then
    echo "$mdelta_string"
    fail "Memory leak"
fi

techo "Passed"
stop_obuspa
grep '0 memory allocations' -- $GREP_FILE || fail "Memory leaks"
