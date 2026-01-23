#!/bin/sh

CONF_PATH=/usr/ark/gst.ini
action=$1

usage_prompt() {
    echo "GST service control script"
    echo "Usage: $0 <start|stop|status|get>"
}

if [ $# -lt 1 ]; then
    usage_prompt
    exit 1
fi

status() {
    pid=$(cat $(gini get -i $CONF_PATH -k pid_file) 2>/dev/null)
    [ -n "$pid" -a -d "/proc/$pid" ] && echo 1 || echo 0
}

start() {
    echo "Starting GST..."

    if [ "$(status)" = "1" ]; then
        echo "Service is already running."
        return 0
    fi

    gst start -d -i "$CONF_PATH"

    if [ "$(status)" = "1" ]; then
        echo "Service started successfully."
        return 0
    else
        echo "Failed to start service!"
        return 1
    fi
}

stop() {
    echo "Stopping GST..."

    if [ "$(status)" = "0" ]; then
        echo "Service is not running."
        return 0
    fi

    gst stop -i "$CONF_PATH"

    # if [ "$(status)" = "1" ]; then
    #     echo "Failed to unload module!"
    #     return 1
    # else
    #     echo "Service stopped successfully."
    #     return 0
    # fi
}

get() {
    if [ "$(status)" = "0" ]; then
        echo "Service is not running."
        return 1
    fi

    gst get -i "$CONF_PATH" -g "$1"
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
    get)
        get "${2:-host}"
        exit $?
        ;;
    *)
        echo "Invalid action: $action"
        usage_prompt
        exit 1
        ;;
esac
