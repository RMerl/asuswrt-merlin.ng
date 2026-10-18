#!/bin/sh

GBUS_CMD=gbus
GBUSD_INI_PATH=/usr/ark/gbusd.ini
action=$1

usage_prompt() {
    echo "GBUSD module control script"
    echo "Usage: $0 <start|stop|status>"
}

if [ $# -ne 1 ]; then
    usage_prompt
    exit 1
fi

status() {
    pid=$(cat $(gini get -i $GBUSD_INI_PATH -k pid_file) 2>/dev/null)
    [ -n "$pid" -a -d "/proc/$pid" ] && echo 1 || echo 0
}

start() {
    echo "Starting GBUSD..."

    if [ "$(status)" = "1" ]; then
        echo "Module is already loaded."
        return 0
    fi

    $GBUS_CMD start
    gbusd -d -i "$GBUSD_INI_PATH"

    if [ "$(status)" = "1" ]; then
        echo "Module loaded successfully."
        return 0
    else
        echo "Failed to load module!"
        return 1
    fi
}

stop() {
    echo "Stopping GBUSD..."

    if [ "$(status)" = "0" ]; then
        echo "Module is not loaded."
        return 0
    fi

    gbusd -s -i "$GBUSD_INI_PATH"
    $GBUS_CMD stop

    if [ "$(status)" = "1" ]; then
        echo "Failed to unload module!"
        return 1
    else
        echo "Module unloaded successfully."
        return 0
    fi
}

case $action in
    status)
        result=$(status)
        echo $result
        exit 0
        ;;
    start)
        start
        exit $?
        ;;
    stop)
        stop
        exit $?
        ;;
    *)
        echo "Invalid action: $action"
        usage_prompt
        exit 1
        ;;
esac
