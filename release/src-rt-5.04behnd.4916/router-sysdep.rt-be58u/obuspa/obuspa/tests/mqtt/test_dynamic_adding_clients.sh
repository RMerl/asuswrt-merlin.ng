#!/bin/bash
# Dynamically adding and removing MQTT clients
CURRENT_DIR="$(dirname $0)"
source "$CURRENT_DIR"/shared.sh || exit 1

rm -rf "$LOG_FILE"
rm -rf "$GREP_FILE"
rm -rf "$DB_FILE"

trap cleanup EXIT
trap cleanup SIGINT

function verify_status() {
    local instance="$1"
    local status="$2"
    local output=""

    output=$(obuspa_cmd -c get "Device.MQTT.Client.$instance.Status")
    if grep -q "$status" <<< $output; then
        techo "Status of $instance is $status"
    else
        fail "Status of $instance is not $status is: ($output)"
    fi
}

stop_obuspa
sleep 1
start_obuspa
techo
techo "TEST 1"
techo "Adding MQTT.Client.1, Client.2"

add_client 1
verify_client 1

add_client 2
verify_client 1 2

del_client 1
verify_no_client 1
verify_client 2

verify_client 2
del_client 2
verify_no_client 2

add_client 1 2 3 4 5
verify_client 1 2 3 4 5

add_local_agent_mtp 1 2
add_local_agent_controller 1 2
add_local_agent_controller_mtp 1 2

configure_client "1" "mqtt.eclipse.org" "1883" "/usp/endpoint/#" "/usp/controller"
enable_client 1
sleep 1

# Verify enable works by checking the logs
# NB escaped characters in this grep
grep '\-\-> Running' -- "$GREP_FILE" || fail "Failed to enable client"
cat "$GREP_FILE" >> $LOG_FILE
echo "" > "$GREP_FILE"

# Verify the status
verify_status 1 "Running"

disable_client 1
sleep 1
grep '\-\-> Idle' -- $GREP_FILE || fail "Failed to disable client"
cat "$GREP_FILE" >> $LOG_FILE
echo "" > "$GREP_FILE"

verify_status 1 "Disabled"

configure_client "2" "doesnt-exist" "1883" "/usp/endpoint/#" "/usp/controller"
enable_client 2
sleep 1

verify_status 2 "Error_BrokerUnreachable"

techo "Passed"

stop_obuspa
grep '0 memory allocations' -- $GREP_FILE || fail "Memory leaks"


