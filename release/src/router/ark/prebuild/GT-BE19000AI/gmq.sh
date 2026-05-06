#!/bin/sh

INI_FILE=/usr/ark/gmq.conf
SRV_CMD=mosquitto 
ret=0

# Check if SRV_CMD exists and is executable
if ! type $SRV_CMD >/dev/null 2>&1; then
    echo "Error: $SRV_CMD not found!"
    exit 1
fi

start() {
    echo "Starting GMQ..."
    
    if [ "$(status)" = "1" ]; then
        echo "Server is already running."
        return 0
    fi

    $SRV_CMD -d -c "$INI_FILE"

    if [ "$(status)" = "1" ]; then
        echo "Server stated successfully."
        return 0
    else
        echo "Failed to run server!"
        return 1
    fi
}

stop() {
    echo "Stopping GMQ..."

    if [ "$(status)" = "0" ]; then
        echo "Server is not running."
        return 0
    fi

    pid_file=$(grep '^pid_file ' "$INI_FILE" | head -n1 | sed 's/^pid_file *//')
    if [ -n "$pid_file" ] && [ -f "$pid_file" ]; then
        pid=$(cat "$pid_file" 2>/dev/null)
        if [ -n "$pid" ] && [ -d "/proc/$pid" ]; then
            kill "$pid"
            sleep 1
            rm -f "$pid_file"
        fi
    fi

    if [ "$(status)" = "1" ]; then
        echo "Failed to stop server!"
        return 1
    else
        echo "Server stop successfully."
        return 0
    fi
}

status() {
    pid_file=$(grep '^pid_file ' "$INI_FILE" | head -n1 | sed 's/^pid_file *//')
    if [ -n "$pid_file" ] && [ -f "$pid_file" ]; then
        pid=$(cat "$pid_file" 2>/dev/null)
        [ -n "$pid" -a -d "/proc/$pid" ] && echo "1" || echo "0"
    else
        echo "0"
    fi
}

# Check if an argument is provided
if [ $# -eq 0 ]; then
    echo "Usage: $0 {start|stop}"
    exit 1
fi

# cd "${0%/*}"

# Call the appropriate function based on the argument
case "$1" in
    start)
        start
        ret=$?
        ;;
    stop)
        stop
        ret=$?
        ;;
    status)
        result=$(status)
        echo $result
        ret=0
        ;;
    *)
        ret=1
        echo "Invalid argument. Usage: $0 {start|stop}"
        ;;
esac

# cd - > /dev/null
exit $ret
