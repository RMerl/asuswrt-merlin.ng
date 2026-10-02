#!/bin/bash
# SHARED, To be sourced only
DB_FILE="/tmp/usp.db"
GREP_FILE="/tmp/obuspa-test.grep"
LOG_FILE="/tmp/obuspa-test.log"

function cleanup() {
    stop_obuspa
    rm -rf "$DB_FILE"
    rm -rf "$GREP_FILE"
}

function techo() {
    echo "******* $*"
}

function start_obuspa() {
    ./obuspa -m -f "$DB_FILE" -v4 $* 2>&1 | tee -a $GREP_FILE &
    sleep 1
}

function obuspa_cmd() {
    ./obuspa -f "$DB_FILE" -v4 $* 2>&1 | tee -a $GREP_FILE
}

function stop_obuspa() {
    if pgrep obuspa; then
        obuspa_cmd -c stop
        sleep 1
    fi
    pkill obuspa
}

function fail() {
    techo FAILED "$*" >&2
    exit 1
}

function add_client() {
    while [[ -n $1 ]]; do
        obuspa_cmd -c add Device.MQTT.Client.$1 || fail "Couldn't add client $1"
        shift
    done
}

function del_client() {
    while [[ -n $1 ]]; do
        obuspa_cmd -c del Device.MQTT.Client.$1 || fail "Couldn't delete client $1"
        shift
    done
}

function verify_client() {
    local output
    while [[ -n $1 ]]; do
        output="$(obuspa_cmd -c get Device.MQTT. || fail)"
        echo "$output" | grep -q "Device.MQTT.Client.$1.BrokerPort => 1883" || fail "Client $1 wasn't added"
        shift
    done
}

function verify_no_client() {
    local output
    while [[ -n $1 ]]; do
        output="$(obuspa_cmd -c get Device.MQTT. || fail)"
        echo $output | grep "Device.MQTT.Client.$1.BrokerPort => 1883" && fail "Client $1 wasn't deleted"
        shift
    done
}

function configure_client() {
    local index=$1
    local host=$2
    local port=$3
    local response_topic="$4"
    local topic="$5"

    obuspa_cmd -c set "Device.MQTT.Client.$index.BrokerAddress" "$host" || fail
    obuspa_cmd -c set "Device.MQTT.Client.$index.BrokerPort" "$port" || fail
}

function enable_client() {
    while [[ -n $1 ]]; do
        obuspa_cmd -c set "Device.MQTT.Client.$1.Enable" "true"
        shift
    done
}

function disable_client() {
    while [[ -n $1 ]]; do
        obuspa_cmd -c set "Device.MQTT.Client.$1.Enable" "false"
        shift
    done
}

function set_parameter_client_reconnect() {
    local sp=$1
    local spv=$2
    local response_topic="$3"
    local topic="$4"
    obuspa_cmd -c set "Device.MQTT.Client.1.$sp" "$spv"
    sleep 2
    grep '\-\-> Running' -- $GREP_FILE || fail "Failed to set the Given Parameter"
    cat $GREP_FILE >> $LOG_FILE
    echo "" > $GREP_FILE
    output="$(obuspa_cmd -c get Device.MQTT.Client.1.$sp)"
    echo $output | grep "Device.MQTT.Client.1.$sp => $spv" || fail "Failed to set the $sp to $spv for Client.1"
}

function set_parameter_client() {
    local sp=$1
    local spv=$2
    local response_topic="$3"
    local topic="$4"
    obuspa_cmd -c set "Device.MQTT.Client.1.$sp" "$spv"
    cat $GREP_FILE >> $LOG_FILE
    echo "" > $GREP_FILE
    output="$(obuspa_cmd -c get Device.MQTT.Client.1.$sp)"
    echo $output | grep "Device.MQTT.Client.1.$sp => $spv" || fail "Failed to set the $sp to $spv for Client.1"
}

function add_local_agent_mtp() {
    while [[ -n $1 ]]; do
        # Setup the LocalAgent.MTP.$1
        obuspa_cmd -c add "Device.LocalAgent.MTP.$1" || fail "MTP 1 failed to be added"
        obuspa_cmd -c set "Device.LocalAgent.MTP.$1.Alias" "cpe-$1"
        obuspa_cmd -c set "Device.LocalAgent.MTP.$1.Enable" "true"
        obuspa_cmd -c set "Device.LocalAgent.MTP.$1.Protocol" "MQTT"
        obuspa_cmd -c set "Device.LocalAgent.MTP.$1.MQTT.Reference" "Device.MQTT.Client.$1" || fail
        obuspa_cmd -c set "Device.LocalAgent.MTP.$1.MQTT.ResponseTopicConfigured" "/usp/endpoint/#"
        shift
    done
}

function add_local_agent_controller() {
    while [[ -n $1 ]]; do
        # Setup the LocalAgent.Controller.$1
        obuspa_cmd -c add "Device.LocalAgent.Controller.$1" || fail "Controller $1 failed to be added"
        obuspa_cmd -c set "Device.LocalAgent.Controller.$1.Enable" "true"
        obuspa_cmd -c set "Device.LocalAgent.Controller.$1.Alias" "cpe-$1"
        obuspa_cmd -c set "Device.LocalAgent.Controller.$1.PeriodicNotifInterval" "86400"
        obuspa_cmd -c set "Device.LocalAgent.Controller.$1.PeriodicNotifTime" "0001-01-01T00:00:00Z"
        obuspa_cmd -c set "Device.LocalAgent.Controller.$1.USPRetryMinimumWaitInterval" "5"
        obuspa_cmd -c set "Device.LocalAgent.Controller.$1.USPRetryIntervalMultiplier" "2000"
        obuspa_cmd -c set "Device.LocalAgent.Controller.$1.EndpointID" "self::usp-controller-$1"
        shift
    done
}

function add_local_agent_controller_mtp(){
    while [[ -n $1 ]]; do
        # Setup the Controller.$1.MTP.$1
        obuspa_cmd -c add "Device.LocalAgent.Controller.$1.MTP.$1" || fail "Controller $1 MTP $1 failed to be added"
        obuspa_cmd -c set "Device.LocalAgent.Controller.$1.MTP.$1.Enable" "true"
        obuspa_cmd -c set "Device.LocalAgent.Controller.$1.MTP.$1.Alias" "cpe-$1"
        obuspa_cmd -c set "Device.LocalAgent.Controller.$1.MTP.$1.Protocol" "MQTT"
        obuspa_cmd -c set "Device.LocalAgent.Controller.$1.MTP.$1.MQTT.Topic" "/usp/controller"
        obuspa_cmd -c set "Device.LocalAgent.Controller.$1.MTP.$1.MQTT.Reference" "Device.MQTT.Client.$1"
        obuspa_cmd -c set "Device.LocalAgent.Controller.$1.MTP.$1.Protocol" "MQTT"
        shift
    done
}
