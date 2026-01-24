#!/bin/sh
. /usr/ark/gar.env
action=$1

TMP_ROOT=/tmp


ark_stop() {
    echo "stopping ark..."
    # ARK begin
    arkctl ark off
    # ARK end
}

gweb_stop() {
    echo "stopping gweb..."
}

gweb_start() {
    echo "starting gweb..."
}

gweb_boot() {
    echo "booting gweb..."
}

boot() {


    # GMQ begin
    gmq start > /dev/null 2>&1
    # GMQ end

    # GBUS begin
    gbusd.sh start
    # GBUS end

    # GLOGER begin
    gloger.sh start
    # GLOGER end

    gweb_boot

    # GDM begin
    gdm.sh start > /dev/null 2>&1
    # GDM end

    # GST begin
    gst.sh start
    # GST end



}

start() {
    boot
}

stop() {



    # GST begin
    gst.sh stop
    # GST end
    
    # GDM begin
    gdm.sh stop
    # GDM end

    ark_stop
    gweb_stop

    # GLOGER begin
    gloger.sh stop
    # GLOGER end

    # GBUS begin
    gbusd.sh stop
    # GBUS end

    # GMQ begin
    gmq stop
    # GMQ end



    rmmod aqua > /dev/null 2>&1
    rmmod gbus > /dev/null 2>&1
    rmmod gtools > /dev/null 2>&1
}

usage_prompt() {
    echo "Usage: $0 {start|stop|status}"
    echo "  start   - Start the GBUSD module"
    echo "  stop    - Stop the GBUSD module"
}

if [ $# -ne 1 ]; then
    usage_prompt
    exit 1
fi

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