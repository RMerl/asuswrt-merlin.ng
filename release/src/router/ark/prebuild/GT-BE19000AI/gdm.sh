#!/bin/sh

GDM_CMD=gdm
GDM_INI_PATH=/usr/ark/gdm.ini
MQTT_PUB=mosquitto_pub
MQTT_HOST="127.0.0.1"
MQTT_PORT=3881
action=$1

usage_prompt() {
    echo "GDM control script"
    echo "Usage: $0 start"
    echo "       $0 stop"
    echo "       $0 status"
    echo "       $0 get [output file]"
    echo "       $0 set <host id> <username>"
    echo "       $0 query <host id>"
    echo "       $0 setwan <WANs>"
    echo "       $0 getitfs"
    echo "       $0 datasync"
}

if [ $# -lt 1 ]; then
    usage_prompt
    exit 1
fi

do_status() {
    pid=$(cat $(gini get -i $GDM_INI_PATH -k pid_file) 2>/dev/null)
    [ -n "$pid" -a -d "/proc/$pid" ] && echo 1 || echo 0
}

do_start() {
    echo "Starting GDM..."

    if [ "$(do_status)" = "1" ]; then
        echo "Module is already loaded."
        return 0
    fi

    $GDM_CMD -d

    if [ "$(do_status)" = "1" ]; then
        echo "Module loaded successfully."
        return 0
    else
        echo "Failed to load module!"
        return 1
    fi
}

do_stop() {
    echo "Stopping GDM..."

    if [ "$(do_status)" = "0" ]; then
        echo "Module is not loaded."
        return 0
    fi

    $GDM_CMD --stop

    i=1
    while [ $i -le 3 ]; do
        if [ "$(do_status)" = "0" ]; then
            echo "Module unloaded successfully."
            return 0
        fi
        sleep 1
        i=$((i + 1))
    done
    echo "Failed to unload module!"
    return 1
}

do_get() {
    if [ -n "$1" ]; then
        OPTIONS="$OPTIONS -O $1"
    fi

    $GDM_CMD -A get $OPTIONS
}

do_set() {
    if [ -z "$1" ] || [ -z "$2" ]; then
        echo "Host ID and username are required for set action."
        return 1
    fi

    $GDM_CMD -A set -H "$1" -U "$2"
}

do_query() {
    if [ -z "$1" ]; then
        echo "Host ID or NIC ID is required for query action."
        return 1
    fi

    $GDM_CMD -A query -H "$1"
}

do_cmd() {
    if [ "$(do_status)" = "0" ]; then
        echo "Module is not loaded. Cannot send command."
        return 1
    fi

    if [ $# -lt 1 ]; then
        echo "At least method is required for do command."
        return 1
    fi

    ctlcmd=$1
    shift
    $GDM_CMD -A srvctl -C $ctlcmd -P ''$*''
}

setenv() {
    export PATH=$PATH:$bindir
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$libdir
}

setenv

case $action in
    status)
        result=$(do_status)
        echo $result
        exit 0
        ;;
    start)
        do_start
        exit $?
        ;;
    stop)
        do_stop
        exit $?
        ;;
    get)
        do_get $2
        exit $?
        ;;
    set)
        do_set $2 $3
        exit $?
        ;;
    query)
        do_query $2
        exit $?
        ;;
    setwan|getitfs|datasync)
        do_cmd $@
        exit $?
        ;;
    *)
        echo "Invalid action: $action"
        usage_prompt
        exit 1
        ;;
esac
