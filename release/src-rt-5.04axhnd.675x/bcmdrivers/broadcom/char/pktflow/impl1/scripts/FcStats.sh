#!/bin/sh
#
# FcStats.sh
#

LOOP_TIME=10
NR_RUNS=0
NR_REPEATS=1
DUMP_CLEAR=0
DUMP_DMESG=0
DUMP_FLOWS=1
DUMP_MISC=1
FTYPE=all

show_help () {
    echo "Syntax: $0 [-f flowtype] [-r repeat_count] [-t interval_time_sec] [-c] [-d] [-nfd] [-nm]"
    echo "Options:"
    echo -e "\t-f flowtype: ucast|mcast default=all"
    echo -e "\t-r repeat_count: Number of times data is collected. Default one loop."
    echo -e "\t-t interval_time_sec: Sleep time between two intervals. Default 10 sec."
    echo -e "\t-c: Clear driver dumps wherever applicable. Default is not clear."
    echo -e "\t-d: dmesg -c. Default no dmesg"
    echo -e "\t-nfd: noflowdump. Do not dump flows."
    echo -e "\t-nm: nomiscdump. Do not dump flows."
    exit
}

# Help option
if [[ $# -eq 1 ]] && [[ $1 == "--help" ]]; then
    show_help
    exit 0
fi

while [[ $# -gt 0 ]]
do
key="$1"

case $key in
        -f|--flowtype)
        FTYPE="$2"
        shift # past argument
        shift # past value
        ;;
        -r|--repeatcount)
        NR_REPEATS="$2"
        shift # past argument
        shift # past value
        ;;
        -c|--clear)
        shift # past argument
        DUMP_CLEAR=1
        ;;
        -t|--timeinterval)
        LOOP_TIME="$2"
        shift # past argument
        shift # past value
        ;;
        -d|--dmesg)
        shift # past argument
        DUMP_DMESG=1
        ;;
        -nfd|--noflowdump)
        shift # past argument
        DUMP_FLOWS=0
        ;;
        -nm|--nomisc)
        shift # past argument
        DUMP_MISC=0
        ;;
        -h|--help)
    show_help
    exit 0
    ;;
        *)    # unknown option
    echo "Unknown Option $1"
    show_help
    exit 0;
esac
done

if ! [ "$LOOP_TIME" -eq "$LOOP_TIME" ] 2>/dev/null; then
    echo "Time interval between repeat should be number in sec."
    show_help
    exit 0;
fi

if ! [ "$NR_REPEATS" -eq "$NR_REPEATS" ] 2>/dev/null; then
    echo "Repeat count should be a number"
    show_help
    exit 0;
fi

dump_cmd() {
    echo "================================="
    echo $1
    echo "================================="
    $1
    echo -e "\n\n"
}

fc_stat_dump () {
    ### FC stats dump

    # Dumping blog stats before fcctl status otherwise output gets messed up
    dump_cmd "blogctl stats";
    dump_cmd "fcctl status";
    dump_cmd "cat /proc/fcache/stats/errors";
    dump_cmd "cat /proc/fcache/stats/evict";
    dump_cmd "cat /proc/fcache/stats/fhw";
    dump_cmd "cat /proc/fcache/stats/flow_bmap";
    dump_cmd "cat /proc/fcache/stats/notify";
    dump_cmd "cat /proc/fcache/stats/path";
    dump_cmd "cat /proc/fcache/stats/path_usage";
    dump_cmd "cat /proc/fcache/stats/query";
    dump_cmd "cat /proc/fcache/stats/slow_path";
    dump_cmd "cat /proc/fcache/stats/vtdev";
    if [[ "$DUMP_MISC" -eq 1 ]]; then
        dump_cmd "cat /proc/fcache/misc/evt_list_info";
        dump_cmd "cat /proc/fcache/misc/host_netdev";
        dump_cmd "cat /proc/fcache/misc/slice_info";
        dump_cmd "cat /proc/fcache/misc/fdblist";
        dump_cmd "cat /proc/fcache/misc/npelist";
        dump_cmd "cat /proc/fcache/misc/host_dev_mac";
    fi
    if [ "$FTYPE" != "ucast" ]; then
        if [[ "$DUMP_MISC" -eq 1 ]]; then
            dump_cmd "cat /proc/fcache/misc/mcastlist";
            dump_cmd "cat /proc/fcache/misc/mcast_group_info";
            dump_cmd "cat /proc/fcache/misc/mcastdnatlist";
            dump_cmd "cat /proc/fcache/misc/rtpseqlist";
        fi
    fi
    if [[ "$DUMP_FLOWS" -eq 1 ]]; then
        dump_cmd "cat /proc/fcache/brlist";
        dump_cmd "cat /proc/fcache/l2list";
        dump_cmd "cat /proc/fcache/nflist";
    fi
    ###
}

loop_commands () {
    NR_RUNS=`expr $NR_RUNS + 1`

    if [[ $NR_REPEATS != 1 ]]; then
        echo "========================================"
        echo " Iteration $NR_RUNS"
        echo "========================================"
    fi    
    fc_stat_dump

    if [[ "$DUMP_DMESG" -eq 1 ]]; then
        dmesg -c
    fi

    if [[ $NR_REPEATS -gt $NR_RUNS ]]; then
        sleep $LOOP_TIME
    fi
}

while [[ $NR_REPEATS -gt $NR_RUNS ]]; do
    loop_commands
done
