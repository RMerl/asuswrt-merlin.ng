#!/bin/sh
#
# rdp_hw_accel_dump.sh
#

###################################################
###  This script relies on RGEN and PLAT variables
###################################################

LOOP_TIME=10
NR_RUNS=0
NR_REPEATS=1
DUMP_CLEAR=0
DUMP_DMESG=0
DUMP_FLOW=1
DUMP_CFG=1
DUMP_PKTRNR=1
MAX_PRINT="max_prints:-1"
RADIOS="0 1 2"

show_help () {
    echo "Syntax: $0 [-r repeat_count] [-t interval_time_sec] [-c] [-d] [-nfd] [-ncfg] [-npr] [-mpr]"
    echo "Options:"
    echo -e "\t-r repeat_count: Number of iterations driver?s data collection. Default one loop."
    echo -e "\t-t interval_time_sec: Sleep time between two intervals. Default 10 sec."
    echo -e "\t-c: Clear driver dumps wherever applicable. Default is not clear."
    echo -e "\t-d: dmesg -c. Default no dmesg"
    echo -e "\t-nfd: noflowdump. Do not dump flows."
    echo -e "\t-ncfg: nocfgdump. Do not dump RDPA configuration."
    echo -e "\t-npr: nopktrnrdump. Do not dump pktrunner stats."
    echo -e "\t-mpr: maxprint. Provide max_prints value in decimal"
    echo -e "\t-radio: Specify radio index [0-2], Default all"
    exit
}

if [ -z ${RGEN+x} ]; then echo "ERROR !! RGEN is not set"; exit 0; fi
#if [ -z ${PLAT+x} ]; then echo "ERROR !! PLAT is not set"; exit 0; fi

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
        -nm|--nomisc)
        shift # past argument
        DUMP_MISC=0
        ;;
        -ncfg|--nocfgdump)
        shift # past argument
        DUMP_CFG=0
        ;;
        -npr|--nopktrnrdump)
        shift # past argument
        DUMP_PKTRNR=0
        ;;
        -mpr|--maxprint)
        MAX_PRINT="max_prints:$2"
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
    echo "================================="
    echo $1
    echo "================================="
    $1
    echo -e "\n\n"
}

rdpa_cfg_dump () {

    dump_cmd "bs /Bdmf/e system children $MAX_PRINT";
    ###
}

rdpa_nonzero_dump () {
    dump_cmd "bs /Bdmf/nZstats";
}

rdpa_flow_dump () {
    ## Check if dump enabled
    if [ $DUMP_FLOW != 1 ]; then
        return
    fi


    local RDPA_INTEREST_OBJ="ucast l2_ucast mcast iptv wlan_mcast ip_class l2_class"
    local RDPA_CURRENT_OBJ=`bs /Bdmf/types`

    for RDPA_EXAM_OBJ in ${RDPA_INTEREST_OBJ}
    do
        RDPA_OBJ=`echo "  ${RDPA_CURRENT_OBJ}  " | grep -m1 -o "${RDPA_EXAM_OBJ}"`
        if [ "${RDPA_OBJ}" != "" ] ; then
            dump_cmd "bs /Bdmf/e $RDPA_EXAM_OBJ $MAX_PRINT"
        fi
    done
}

pktrnr_dump () {
    ## Check if dump enabled
    if [ $DUMP_PKTRNR != 1 ]; then
        return
    fi

    dump_cmd "cat /proc/pktrunner/accel0/stats"
    #dump_cmd "cat /proc/pktrunner/accel0/flows/L2"
    #dump_cmd "cat /proc/pktrunner/accel0/flows/L3"
    #dump_cmd "cat /proc/pktrunner/accel0/flows/mcast"
}

dhd_dump () {

    if [ $RGEN = 30 ] ; then

        dump_cmd "bs /Driver/rdd/pddc"
        dump_cmd "bs /Driver/rdd/pdfrc"

        for RADIO_IDX in ${RADIOS}
        do
            dump_cmd "dhd -i wl$RADIO_IDX dump"
        done

    elif [ $RGEN -gt 30 ] ; then

        dump_cmd "bs /Driver/rdd/gdfl"

        for RADIO_IDX in ${RADIOS}
        do
            dump_cmd "bs /Driver/rdd/pdgc $RADIO_IDX"
            dump_cmd "bs /Driver/rdd/pddc $RADIO_IDX"
            dump_cmd "bs /Driver/rdd/pdfrc $RADIO_IDX"
            dump_cmd "dhd -i wl$RADIO_IDX dump"
        done
    fi
}

wfd_dump () {

    # WFD Stats or flctl
    local WFD_INTEREST_OBJ="stats flctl"
    local WFD_CURRENT_OBJ=`ls /proc/wfd 2>/dev/null`

    for WFD_EXAM_OBJ in ${WFD_INTEREST_OBJ}
    do
        WFD_OBJ=`echo "  ${WFD_CURRENT_OBJ}  " | grep -m1 -o "${WFD_EXAM_OBJ}"`
        if [ "${WFD_OBJ}" != "" ] ; then
            dump_cmd "cat /proc/wfd/$WFD_OBJ"
        fi
    done
    ##


}

wifi_dump () {
    # WFD dumps
    wfd_dump
    # DHD offload dumps
    dhd_dump
    # dump wifi data path stats
    for RADIO_IDX in ${RADIOS}
    do
        dump_cmd "wl -i wl$RADIO_IDX dpstats"
    done
    ##
}

rnr_other_dump () {

    if [ $RGEN -gt 30 ] ; then
        dump_cmd "bs /Driver/qm/Print_non_empty_queues"
        dump_cmd "bs /Driver/cpur sar"
        dump_cmd "bs /Driver/System/Sanity"
        dump_cmd "bs /Driver/Fpm/Debug_get"
        dump_cmd "bs /Driver/Sbpm/Debug_get"
        dump_cmd "bs /Driver/qm debug"
        dump_cmd "bs /Driver/dsptchr Get flldes_bufavail"
        dump_cmd "bs /Driver/dsptchr Get congestion_congstn_status"
    fi
    if [ $RGEN -gt 60 ] ; then
        dump_cmd "bs /Driver/cnpl/bufmng"
    fi
    ##
}

stats_dump () {

    ## pkt_runner dumps
    pktrnr_dump

    ## nonzero rdpa dumps
    rdpa_nonzero_dump

    ## Wifi dump
    wifi_dump

    ## other runner dumps
    rnr_other_dump

    ## flow-dump
    rdpa_flow_dump
    ###
}

loop_commands () {
    NR_RUNS=`expr $NR_RUNS + 1`

    if [[ $NR_REPEATS != 1 ]]; then
        echo "========================================"
        echo " Iteration $NR_RUNS"
        echo "========================================"
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

## Dump the configuration
if [[ "$DUMP_CFG" -eq 1 ]]; then
    rdpa_cfg_dump
fi

while [[ $NR_REPEATS -gt $NR_RUNS ]]; do
    loop_commands
done
