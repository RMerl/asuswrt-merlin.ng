#!/bin/bash
# Test setting client paramters in MQTT
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
techo "Verifying dynamic parameter change"

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
techo "Set various connection parameters for a client OBUSPA"
set_parameter_client "CleanSession" "false"
set_parameter_client "CleanSession" "true"
set_parameter_client "CleanStart" "false"
set_parameter_client "CleanStart" "true"
set_parameter_client_reconnect "KeepAliveTime" "75"
set_parameter_client_reconnect "KeepAliveTime" "65"
set_parameter_client_reconnect "KeepAliveTime" "90"
set_parameter_client_reconnect "KeepAliveTime"  "10"
set_parameter_client_reconnect "KeepAliveTime" "60"
set_parameter_client_reconnect "Username" "obuspa_user"
set_parameter_client_reconnect "Password" "obuspa_password"
set_parameter_client_reconnect "ProtocolVersion" "3.1.1"
set_parameter_client_reconnect "ProtocolVersion" "3.1"
set_parameter_client_reconnect "ProtocolVersion" "5.0"
set_parameter_client_reconnect "Name" "indigoc1"
set_parameter_client_reconnect "ClientID" "obuspacid"
set_parameter_client_reconnect "TransportProtocol" "WebSocket"
set_parameter_client_reconnect "TransportProtocol" "TCP/IP"

techo "Passed"

stop_obuspa
grep '0 memory allocations' -- $GREP_FILE || fail "Memory leaks"
