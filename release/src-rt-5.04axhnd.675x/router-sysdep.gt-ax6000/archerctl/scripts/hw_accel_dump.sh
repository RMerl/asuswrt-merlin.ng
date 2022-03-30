#!/bin/sh
#
# hw_accel_dump.sh
#

LOOP_TIME=10
NR_RUNS=0
NR_REPEATS=1
DUMP_CLEAR=0
DUMP_DMESG=0
DUMP_FLOW=1
MAX_PRINT="--all"
# By default disable Wifi dumps and let user specify
RADIOS=""

show_help () {
    echo "Syntax: $0 [-r repeat_count] [-t interval_time_sec] [-c] [-d] [-nfd] [-mpr] [-radio]"
    echo "Options:"
    echo -e "\t-r repeat_count: Number of iterations of data collection. Default one loop."
    echo -e "\t-t interval_time_sec: Sleep time between two intervals. Default 10 sec."
    echo -e "\t-c: Clear driver dumps wherever applicable. Default is not clear."
    echo -e "\t-d: dmesg -c. Default no dmesg"
    echo -e "\t-nfd: noflowdump. Do not dump flows."
    echo -e "\t-mpr: maxprint. Provide max flows to dump in decimal"
    echo -e "\t-radio: Specify radio index [0-2], Default none"
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
        DUMP_FLOW=0
        ;;
        -mpr|--maxprint)
        MAX_PRINT="--max $2"
        shift # past argument
        shift # past value
        ;;
        -radio)
        RADIOS="$2"
        shift # past argument
        shift # past value
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
    #echo "================================="
    echo "*** $1 ***"
    #echo "================================="
    $1
    echo -e "\n\n"
}

wifi_dump () {
    # dump wifi data path stats
    for RADIO_IDX in ${RADIOS}
    do
        dump_cmd "wl -i wl$RADIO_IDX dpstats"
        dump_cmd "dhd -i wl$RADIO_IDX dump"
    done
    ##
}

archer_flow_dump () {
    ###
    
    if [[ "$DUMP_FLOW" -eq 1 ]]; then
        #dump_cmd "archerctl flows $MAX_PRINT"
        dump_cmd "archerctl ucast_l3 $MAX_PRINT"
        dump_cmd "archerctl ucast_l2 $MAX_PRINT"
        dump_cmd "archerctl mcast $MAX_PRINT"
    fi
    ###
}
archer_dump () {

    ##
    dump_cmd "archerctl status"
    dump_cmd "archerctl host"
    dump_cmd "archerctl stats"

    ##
}

stats_dump () {

    ## Wifi dump
    wifi_dump

    ## archer dumps
    archer_dump

    ## flow-dump
    archer_flow_dump
    ###
}

dump_params () {
    echo "LOOP_TIME     = $LOOP_TIME"
    echo "NR_RUNS       = $NR_RUNS"
    echo "NR_REPEATS    = $NR_REPEATS"
    echo "DUMP_CLEAR    = $DUMP_CLEAR"
    echo "DUMP_DMESG    = $DUMP_DMESG"
    echo "DUMP_FLOW     = $DUMP_FLOW"
    echo "MAX_PRINT     = $MAX_PRINT"
    ###
}

# For script param debugging; Uncomment below
#dump_params

loop_commands () {
    NR_RUNS=`expr $NR_RUNS + 1`

    if [[ $NR_REPEATS != 1 ]]; then
        #echo "========================================"
        echo " === Iteration $NR_RUNS  ==="
        #echo "========================================"
    fi
    
    ## Dump Stats/counters    
    stats_dump

    if [[ "$DUMP_DMESG" -eq 1 ]]; then
        dmesg -c
    fi

    #if [[ "$DUMP_CLEAR" -eq 1 ]]; then
        # Add code here to issue command to clear counters
    #fi

    if [[ $NR_REPEATS -gt $NR_RUNS ]]; then
        sleep $LOOP_TIME
    fi
}

while [[ $NR_REPEATS -gt $NR_RUNS ]]; do
    loop_commands
done
