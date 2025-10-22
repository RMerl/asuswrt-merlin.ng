#!/bin/sh

[ -z "$1" ] && exit

QUEUES="0 1 2 3 4 5 6 7"

queue2w()
{
    local qid=$1

    # default
    if [ ${qid} -eq 0 ]; then
        printf "1"

    # pass
    elif [ ${qid} -eq 1 ]; then
        printf "63"

    # extreme
    elif [ ${qid} -eq 2 ]; then
        printf "63"

    # high
    elif [ ${qid} -eq 3 ]; then
        printf "32"

    # medium
    elif [ ${qid} -eq 4 ]; then
        printf "16"

    # low
    elif [ ${qid} -eq 5 ]; then
        printf "1"

    # l2l
    elif [ ${qid} -eq 6 ]; then
        printf "1"
    elif [ ${qid} -eq 7 ]; then
        printf "1"
    fi
}

create()
{
    [ -z "$1" ] && exit
    [ -z "$2" ] && exit

    local type=$1
    local ifname=$2

    # ETH
    if [ ${type} -eq 0 ]; then
        tmctl porttminit --devtype 0 --if ${ifname} --flag 0x200 --numqueues 8
        for qid in ${QUEUES}; do
        tmctl setqcfg --devtype 0 --if ${ifname} --qid ${qid} --weight $(queue2w ${qid}) --schedmode 2 --shapingrate 0
        done

    # Service queue
    elif [ ${type} -eq 4 ]; then
        tmctl porttminit --devtype 4 --if ${ifname} --flag 0x200 --numqueues 8
        for qid in ${QUEUES}; do
            tmctl setqcfg --devtype 4 --if ${ifname} --qid ${qid} --weight $(queue2w ${qid}) --schedmode 2 --shapingrate 0
        done
    fi

}

delete()
{
    [ -z "$1" ] && exit
    [ -z "$2" ] && exit

    local type=$1
    local ifname=$2

    # ETH
    if [ ${type} -eq 0 ]; then
        tmctl porttminit --devtype 0 --if ${ifname} --flag 0x0 --numqueues 8
        for qid in ${QUEUES}; do
            tmctl setqcfg --devtype 0 --if ${ifname} --qid ${qid} --priority ${qid} --weight 0 --schedmode 1 --shapingrate 0
        done

    # Service queue
    elif [ ${type} -eq 4 ]; then
        tmctl porttmuninit --devtype 4 --if ${ifname}
    fi

}

# get_traffic <DIRECTION> <DEVTYPE> <INTERFACE NAME> <FILE>
# function get_traffic()
get_traffic()
{
    [ -z "$1" ] && exit
    [ -z "$2" ] && exit
    [ -z "$3" ] && exit
    [ -z "$4" ] && exit

    local d=$1
    local type=$2
    local ifname=$3
    local file=$4
	
    if [ ! -d "/sys/class/net/${ifname}" ]; then
        exit
    fi

    for qid in ${QUEUES}; do
        tmctl getqstats --devtype ${type} --if ${ifname} --qid ${qid} | grep txBytes | awk "{print \"set-txbytes ${d} ${qid} \" \$2}"  > ${file}
    done

}

# function set_limit()
set_limit()
{
    [ -z "$1" ] && exit
    [ -z "$2" ] && exit
    [ -z "$3" ] && exit
    [ -z "$4" ] && exit
    
    local type=$1
    local ifname=$2
    local qid=$3
    local kbps=$4

    if [ ! -d "/sys/class/net/${ifname}" ]; then
        exit
    fi

    tmctl setqcfg --devtype ${type} --if ${ifname} --qid ${qid} --priority 0 --weight $(queue2w ${qid}) --schedmode 2 --shapingrate ${kbps}
}

clear()
{
    fc_flush
}

case $1 in
    create)
        shift;
        create $@
        ;;
    get-traffic)
        shift;
        get_traffic $@
        ;;
    set-limit)
        shift;
        set_limit $@
        ;;
    delete)
        shift;
        delete $@
        ;;
    clear)
        clear
        ;;
    *)
        ;;
esac


