#!/bin/sh
action=$1

PREFIX=/usr
PATH=${PREFIX}/bin:${PREFIX}/sbin:${PATH}
LD_LIBRARY_PATH=${PREFIX}/lib:${LD_LIBRARY_PATH}

export PATH
export LD_LIBRARY_PATH

TMP_ROOT=/tmp

gtnet_stop() {
    echo "stopping gtnet..."
}

gtnet_start() {
    echo "starting gtnet..."
}

gdd_stop() {
    echo "stopping gdd..."
    # GDD begin
    gdrctl stop
    # GDD end
}

gdd_start() {
    echo "starting gdd..."
    # GDD begin
    gdrctl start > /dev/null 2>&1
    # GDD end
}

ark_stop() {
    echo "stopping ark..."
    # ARK begin
    arkctl ark off
    # ARK end
}

gweb_stop() {
    echo "stopping gweb..."
    # GWEB begin
    gws stop
    # GWEB end
}

gweb_start() {
    echo "starting gweb..."
    # GWEB begin
    gws start
    # GWEB end
}

gweb_boot() {
    echo "booting gweb..."
    # GWEB begin
    gws boot
    # GWEB end
}

boot() {


    gmq start > /dev/null 2>&1

    gbusd.sh start

    # GLOGER begin
    gloger.sh start
    # GLOGER end

    gweb_boot

    gdd_start
    gtnet_start > /dev/null 2>&1
}

start() {
    boot
}

stop() {
    gtnet_stop
    gdd_stop
    ark_stop
    gweb_stop
    
    # GLOGER begin
    gloger.sh stop
    # GLOGER end

    gbusd.sh stop
    gmq stop

    

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