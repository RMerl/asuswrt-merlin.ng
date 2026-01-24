#!/bin/sh
BINDIR=/usr/sbin
LIBDIR=/tmp/ark
export PATH=${BINDIR}:${PATH}
export LD_LIBRARY_PATH=${LIBDIR}:${LD_LIBRARY_PATH}
LOGGER_INI_PATH=/usr/ark/gloger.ini
action=$1

usage_prompt() {
    echo "LOGGER service control script"
    echo "Usage: $0 <start|stop|status>"
}

if [ $# -ne 1 ]; then
    usage_prompt
    exit 1
fi

status() {
    pid=$(cat $(gini get -i $LOGGER_INI_PATH -k pid_file) 2>/dev/null)
    [ -n "$pid" -a -d "/proc/$pid" ] && echo 1 || echo 0
}

start() {
    echo "Starting LOGGER..."

    if [ "$(status)" = "1" ]; then
        echo "Module is already loaded."
        return 0
    fi

    gloger -d -i "$LOGGER_INI_PATH" 

    if [ "$(status)" = "1" ]; then
        echo "Module loaded successfully."
        return 0
    else
        echo "Failed to load module!"
        return 1
    fi
}

stop() {
    echo "Stopping LOGGER..."

    if [ "$(status)" = "0" ]; then
        echo "Module is not loaded."
        return 0
    fi

    gloger -s -i "$LOGGER_INI_PATH"

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
