#!/bin/sh
#
# WlGetDriverStats.sh <ucast|mcast> <WiFi interface name> <NrRetries> <enable|disable>
#
# Copyright (C) 2024, Broadcom. All Rights Reserved.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
#
# <<Broadcom-WL-IPTag/Dual>>
#
# $Id$

WLCMD=wl
DHDCMD=dhd
CEVENTCCMD=ceventc
IFNAME="-e"
PRIM_IFNAME="-p"
DUMPPATH=/tmp
DUMPDIR=${DUMPPATH}/$(date +"%d:%m:%Y_%H:%M")
LOOP_TIME=10
NR_RUNS=1
HOSTAPD_CONF=/tmp/wlX_hapd.conf
HOSTAPD_CONF_1=/tmp/wlX.1_hapd.conf
WPASUPP_CONF=/tmp/wlX_wpa_supplicant.conf
NR_REPEATS=1
DUMP_CLEAR=1
MODE="auto"
ENABLE_SOCRAM_DUMP="disable"
VERSION=1
FCSTATS=/bin/FcStats.sh
HWACCELSTATS=/bin/hw_accel_dump.sh

show_help () {
    echo "Two input formats are supported by this script."
    echo "First one is for backward compatibility."
    echo "Second one to make it more user friendly, and developer friendly for future expansion."
    echo -e "\nFirst input format:"
    echo "Syntax: $0 <WiFi interface name> <NrRetries> <SOCRAMDUMP: enable|disable>"
    echo "  or  : $0 <WiFi interface name> [<Driver mode: auto|nic|dhd>] <NrRetries> <SOCRAMDUMP: enable|disable>"
    echo "Example 1: $0 wl1 (infinite loop)"
    echo "Example 2: $0 wl1 dhd (for FD driver; infinite loop)"
    echo "Example 3: $0 eth5 auto 12 enable (for any driver; 12 loops - 10s between loops; dump socram also)"
    echo "Try \`$0 --help' for more information."

    echo -e "\nSecond input format:"
    echo "Syntax: $0 [-i ifname] [-m mode] [-r repeat_count] [-t interval_time_sec] [-c] [-s enable|disable]"
    echo "Options:"
    echo -e "\t-i ifname: Name of the WLAN driver interface. Default is wl0."
    echo -e "\t-m mode: Valid driver modes: auto|nic|dhd. Default is auto."
    echo -e "\t-r repeater_count: Number of iterations driver?s data collection. Default infinite loop."
    echo -e "\t-t interval_time_sec: Sleep time between two intervals. Default 10 sec."
    echo -e "\t-c: Clear wlan driver dumps wherever applicable. Default is not clear."
    echo -e "\t-s enable|disable: SOCRAMDUMP enable or disable. Default is disable."
    exit
}

dump_hapd () {
   for i in 0 1 2 3
   do
       HAPDFILE=${HOSTAPD_CONF//X/$i}
       if test -f $HAPDFILE; then
           echo "------- Dumping $HAPDFILE ----------"
           cat $HAPDFILE
       fi

       HAPDFILE_1=${HOSTAPD_CONF_1//X/$i}
       if test -f $HAPDFILE_1; then
           echo "------- Dumping $HAPDFILE_1 ----------"
           cat $HAPDFILE_1
       fi
   done
}

dump_wpasupp () {
   for i in 0 1 2 3
   do
       WPASUPPFILE=${WPASUPP_CONF//X/$i}
       if test -f $WPASUPPFILE; then
           echo "------- Dumping $WPASUPPFILE ----------"
           cat $WPASUPPFILE
       fi
   done
}

# $1 Heading
# $2 cmd to execute
display_cmd_op(){
    cmd="$2"
    echo "---------------------------------------------"
    echo -e "$1"
    if [ -n "$3" ] && [ "$DUMP_CLEAR" -eq 1 ] && [ $NR_RUNS -ne 0 ]; then
        echo "This Dump/Stats was cleared after last display"
    fi
    echo "---------------------------------------------"
    $cmd |
    while IFS= read -r line
    do
        echo -e "\t""$line"
    done
    if [ -n "$3" ] && [ "$DUMP_CLEAR" -eq 1 ]; then
        echo "CLEAR: $WLCMD $IFNAME $3 $4"
        $WLCMD $IFNAME $3 $4
    fi
    echo -e ""
}

# Help option
if [[ $# -eq 1 ]] && [[ $1 == "--help" ]]; then
    show_help
    exit 0
fi

OPT=$1
if [[ ${OPT:0:1} == - ]]; then
    VERSION=2
	IFNAME="-i wl0"
    while [[ $# -gt 0 ]]
    do
    key="$1"

    case $key in
            -a|-i|--interface)
            IFNAME="-i $2"
            PRIM_IFNAME=$IFNAME
            shift # past argument
            shift # past value
            ;;
            -m|--mode)
            MODE="$2"
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
            -s|--socramdump)
            ENABLE_SOCRAM_DUMP="$2"
            shift # past argument
            shift # past value
            ;;
            -t|--timeinterval)
            LOOP_TIME="$2"
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

else

    VERSION=1

    if [ $# -eq 4 ]; then
        MODE=$2
        NR_REPEATS=$3
        ENABLE_SOCRAM_DUMP=$4
    elif [ $# -eq 3 ]; then
        if [[ $2 != "nic" ]] && [[ $2 != "dhd" ]] && [[ $2 != "auto" ]]; then
            NR_REPEATS=$2
            ENABLE_SOCRAM_DUMP=$3
        else
            MODE=$2
            NR_REPEATS=$3
        fi
    elif [ $# -eq 2 ]; then
        if [[ $2 != "nic" ]] && [[ $2 != "dhd" ]] && [[ $2 != "auto" ]]; then
            NR_REPEATS=$2
        else
            MODE=$2
        fi
    elif [ $# -eq 1 ]; then
        IFNAME="-i $1"
    fi
fi # end of input argument parsing

echo "============================="
echo "Input info::::"
echo "IFNAME: $IFNAME"
echo "Mode: $MODE"
echo "NR_REPEATS: $NR_REPEATS"
echo "ENABLE_SOCRAM_DUMP: $ENABLE_SOCRAM_DUMP"
echo "LOOP_TIME: $LOOP_TIME"
echo "DUMP_CLEAR: $DUMP_CLEAR"
echo "============================="

if ! [ "$NR_REPEATS" -eq "$NR_REPEATS" ] 2>/dev/null; then
    echo "Repeat count should be a number"
    show_help
    exit 0;
fi

if [[ $ENABLE_SOCRAM_DUMP != "enable" ]] && [[ $ENABLE_SOCRAM_DUMP != "disable" ]] ; then
    echo "Valid input for srom dump are enable/disable"
    exit 0;
fi

if [ $MODE = "auto" ]; then
    fwid=$($WLCMD $IFNAME ver | grep -c FWID)
    if [ $fwid = "1" ]; then
        MODE="dhd"
    else
        MODE="nic"
    fi
fi

if [[ $MODE != "nic" ]] && [[ $MODE != "dhd" ]]; then
    echo "UNKNOWN driver mode!"
    show_help
    exit 0
fi

SOCRAMDUMPFILE=${DUMPDIR}/socram_${IFNAME}

# Overwrite msglevel
if [[ $IFNAME == $PRIM_IFNAME ]]; then
    WLMSGLVL=`$WLCMD $IFNAME msglevel | cut -d ' ' -f1`
    $WLCMD $IFNAME msglevel 0
    if [[ $MODE == "dhd" ]]; then
        echo -n $IFNAME DHD version = ; echo $($DHDCMD $IFNAME version)
        DHDMSGLVL=`$DHDCMD $IFNAME msglevel | cut -d ' ' -f1`
        $DHDCMD $IFNAME msglevel 0
    fi
fi

driver_init () {
    if [ $($WLCMD $IFNAME isup) -ne 1 ]; then
    echo -e "Interface $IFNAME is not up"
    exit 0
    fi
    $WLCMD $IFNAME bs_data -noreset > /dev/NULL 2>&1
    $WLCMD $IFNAME dpstats c: a: m: p: n: b: > /dev/NULL 2>&1
    $WLCMD $IFNAME rx_report -noidle > /dev/NULL 2>&1
}

driver_info () {
    echo "============================="
    echo "$PRIM_IFNAME Driver info"
    echo "============================="

    display_cmd_op "WLVERSION: $WLCMD $PRIM_IFNAME ver" "$WLCMD $PRIM_IFNAME ver"
    display_cmd_op "REVINFO: $WLCMD $PRIM_IFNAME revinfo" "$WLCMD $PRIM_IFNAME revinfo"
    display_cmd_op "DHDVERSION: $DHDCMD $PRIM_IFNAME version" "$DHDCMD $PRIM_IFNAME version"
    display_cmd_op "WLCAP: $WLCMD $PRIM_IFNAME cap" "$WLCMD $PRIM_IFNAME cap"
    display_cmd_op "UPTIME (sec): [System uptime] [sum of how much time each core spent idle] " "cat /proc/uptime"
}

wl_stats () {
    echo "============================================="
    echo "WL Statistics for $IFNAME"
    echo "============================================="

    items="status assoclist authe_sta_list 6g_rate 5g_rate 2g_rate nrate rate ratedump
        if_counters smfstats phy_ed_thresh macmode mac txpwr_adj_est bss keep_ap_up "

    for x in $items; do
        display_cmd_op "$WLCMD $IFNAME $x" "$WLCMD $IFNAME $x"
    done

    echo "============================================="
    echo "WL Statistics for $PRIM_IFNAME"
    echo "============================================="

    if [[ $MODE == "dhd" ]]; then
    items="chanim_stats memuse chan_info dfs_status_all noise phy_tempsense band escanresults escan wme_counters counters
        wme_clear_counters reset_cnts spect oper_mode interference interference_override"
    else
    items="chanim_stats memuse chan_info dfs_status_all noise phy_tempsense band scanresults scan wme_counters counters
        wme_clear_counters reset_cnts spect oper_mode interference interference_override"
    fi

    for x in $items; do
        display_cmd_op "$WLCMD $IFNAME $x" "$WLCMD $IFNAME $x"
    done

    echo "============================================="
    echo "STA_INFO for $IFNAME"
    echo "============================================="

    display_cmd_op "$WLCMD $IFNAME sta_info all" "$WLCMD $IFNAME sta_info all"

    if [[ $IFNAME == $PRIM_IFNAME ]]; then
        # Only query on main
        display_cmd_op "$WLCMD $IFNAME mlo info" "$WLCMD $IFNAME mlo info"
        $WLCMD $IFNAME mlo_unit > /dev/NULL 2>&1
        if [ $? -eq 0 ] ; then
            for x in $($WLCMD $IFNAME assoclist);
            do
                if [ $x != assoclist ] ; then
                    display_cmd_op "$WLCMD $IFNAME mlo scb_stats $x" "$WLCMD $IFNAME mlo scb_stats $x"
                fi
            done
        fi
    else
        display_cmd_op "$WLCMD $IFNAME mlo scb_stats $x" "$WLCMD $IFNAME mlo scb_stats $x"
    fi

    echo "============================================="
    echo "WL datapath for $PRIM_IFNAME"
    echo "============================================="

    display_cmd_op "BSDATA: $WLCMD $IFNAME bs_data -noreset" "$WLCMD $IFNAME bs_data -noreset"
    display_cmd_op "DPSTATS: $WLCMD $IFNAME dpstats" "$WLCMD $IFNAME dpstats c: a: m: p: n:"
    display_cmd_op "RXREPORT: $WLCMD $IFNAME rx_report" "$WLCMD $IFNAME rx_report -noidle"
    display_cmd_op "TAF ATOS DUMP: $WLCMD $PRIM_IFNAME dump taf -atos" "$WLCMD $PRIM_IFNAME dump taf -atos"

    echo "============================================="
    echo "WL dump for $PRIM_IFNAME"
    echo "============================================="

    items="txfifo txq perf_stats wlc murx ratelinkmem"

    for x in $items; do
        display_cmd_op "$WLCMD $IFNAME dump $x" "$WLCMD $IFNAME dump $x"
    done

    echo "============================================="
    echo "WL dump with clear for $PRIM_IFNAME"
    echo "============================================="

    items="ampdu amsdu txbf mutx msched umsched twt smfstats d11cnts bsscfg scb "

    for x in $items; do
        display_cmd_op "$WLCMD $IFNAME dump $x" "$WLCMD $IFNAME dump $x" "dump_clear" "$x"
    done

    display_cmd_op "CEVENT: $CEVENTCCMD $IFNAME dump" "$CEVENTCCMD $IFNAME dump"
    ## Flush ceventc log
    # $CEVENTCCMD $IFNAME flush > /dev/NULL 2>&1
}

dhd_stats () {
    echo "================================="
    echo "DHD Statistics for $IFNAME"
    echo "================================="
    display_cmd_op "DHDDUMP: $DHDCMD $IFNAME DHD dump" "$DHDCMD $IFNAME dump"
    if [ $ENABLE_SOCRAM_DUMP = "enable" ] ; then
    mkdir $DUMPDIR
    display_cmd_op "SOCRAMDUMP: Uploading DUMP to $1" "$DHDCMD $IFNAME upload $1"
    fi
    $DHDCMD $IFNAME cons mu
}

Dump_Archer_CPU_affinify () {
    target_kthread="archer awl recycle kthrd skb enet-kthrd"
    grep_patten="";

    for kthread_name in $target_kthread ;
    do
        grep_patten="$kthread_name\\|$grep_patten"
    done

    grep_patten=${grep_patten:0:-2};
    grep_cmd="grep '${grep_patten}'"

    for pid in `ps | ${grep_cmd} | grep '\[' | cut -f 2-4 -d ' ' | grep -o '[0-9]\+'` ;
    do
        comm_name=`cat /proc/${pid}/comm`;
        pad_pid="       $pid";
        echo "### ${pad_pid:-5} : $comm_name ";
        taskset -c -p  ${pid};
    done

    wlaffinity show
    echo "";
}

Archer_accel_dump () {
    ## Dump Archer ,PKTFWD

    echo "================================="
    echo "Archer HW_ACCEL"
    echo "================================="

    Dump_Archer_CPU_affinify

    # msg to printk ...
    dmesg -c
    archer status
    archer stats
    dmesg -c

    cat /proc/pktfwd_wl/stats
    dmesg -c

    # Dump partial flow to avoid too much print ..
    archer flows --max 64
    dmesg -c

    # Service Queue stats
    # archer sq stats
    # dmesg -c
}

Runner_accel_dump () {
    ## Dump Runner ,WFD

    echo "================================="
    echo "Runner HW_ACCEL"
    echo "================================="

    local BSCMD_MAX_PRINT=64
    local WLIF_NO=""

    # Only partial platform support bs command with Radio index
    # WLIF_NO=`echo "$IFNAME" | grep -m1 -o -e "wl[0-9]\+[\.]\?" | grep -m1 -o -e "[0-9]\+"`

    #    system           RDPA system
    #        filter           Ingress Filters
    #        mcast            Multicast Flow Manager
    #        policer          Traffic Policer
    #        dscp_to_pbit     DSCP to PBIT mapping table
    #        egress_tm        Hierarchical Traffic Scheduler
    #        ucast            Unicast Flow Manager
    #        l2_ucast         L2 Unicast Flow Manager
    #        cpu              CPU Interface
    #        dhd_helper       DHD Helper channel bundle
    #        ingress_class    Ingress classification
    #        port             Physical or Virtual Interface

    local RDPA_INTEREST_OBJ="system cpu ucast l2_ucast mcast iptv wlan_mcast ip_class l2_class dhd_helper"
    local RDPA_CURRENT_OBJ=`bs /b/types`

    ## config
    RDPA_INTEREST_OBJ="ucast l2_ucast mcast iptv wlan_mcast ip_class l2_class dhd_helper"
    for RDPA_EXAM_OBJ in ${RDPA_INTEREST_OBJ}
    do
        RDPA_OBJ=`echo "  ${RDPA_CURRENT_OBJ}  " | grep -m1 -o "${RDPA_EXAM_OBJ}"`
        if [ "${RDPA_OBJ}" != "" ] ; then
            BS_CMD="bs /b/e $RDPA_EXAM_OBJ class:config max_prints:${BSCMD_MAX_PRINT}"
            display_cmd_op "${BS_CMD}" "${BS_CMD}"
        fi
    done

    ## nzstat
    RDPA_INTEREST_OBJ="system cpu ucast l2_ucast mcast iptv wlan_mcast ip_class l2_class dhd_helper"

    for RDPA_EXAM_OBJ in ${RDPA_INTEREST_OBJ}
    do
        RDPA_OBJ=`echo "  ${RDPA_CURRENT_OBJ}  " | grep -m1 -o "${RDPA_EXAM_OBJ}"`
        if [ "${RDPA_OBJ}" != "" ] ; then
            BS_CMD="bs /b/e $RDPA_EXAM_OBJ class:nzstat max_prints:${BSCMD_MAX_PRINT}"
            display_cmd_op "${BS_CMD}" "${BS_CMD}"
        fi
    done

    ## Dump drop count
    display_cmd_op "bs /d/rdd/gdfl" "bs /d/rdd/gdfl"
    display_cmd_op "bs /d/rdd/pdgc ${WLIF_NO}" "bs /d/rdd/pdgc ${WLIF_NO}"

    if [[ $MODE == "dhd" ]]; then
        display_cmd_op "bs /d/rdd/pddc ${WLIF_NO}" "bs /d/rdd/pddc ${WLIF_NO}"
        display_cmd_op "bs /d/rdd/pdfrc ${WLIF_NO}" "bs /d/rdd/pdfrc ${WLIF_NO}"
    fi

    display_cmd_op "bs /d/qm/Print_non_empty_queues" "bs /d/qm/Print_non_empty_queues"
    display_cmd_op "bs /d/cpur sar" "bs /d/cpur sar"

    ## non zero statistic
    display_cmd_op "bs /b/z" "bs /b/z"
}

probe_module_ins () {
    local MODULE_NAME=$1
    local PROBED=`grep -m1 -o -e "^${MODULE_NAME}" /proc/modules`

    if [ "${PROBED}" != "" ] ; then
        echo "1"
    else
        echo "0"
    fi
}

probe_hw_accelerator () {
    local HW_ACCEL=""

    if [ "`probe_module_ins pktrunner`" -eq 1 ] ; then
        HW_ACCEL="RUNNER"
    elif [ "`probe_module_ins archer`"  -eq 1 ] ; then
        HW_ACCEL="ARCHER"
    else
        HW_ACCEL="UNKNOWN"
    fi

    echo "${HW_ACCEL}"
}

host_side_dump () {
    ### host side dump
    echo "================================="
    echo "HOST Statistics"
    echo "================================="

    if [ -f $FCSTATS ]; then
        $FCSTATS -d
    else
        HW_ACCEL=`probe_hw_accelerator`

        FcStats.sh

        if [ "$HW_ACCEL" != "ARCHER" ] ; then
            ## For WFD
            display_cmd_op "cat /proc/wfd/flctl" "cat /proc/wfd/flctl"
            display_cmd_op "cat /proc/wfd/stats" "cat /proc/wfd/stats"
        fi

        cat /proc/fcache/misc/mcastlist;
    fi

    cat /proc/net/igmp_snooping;
    bpm status
    dmesg -c

    ## For HW Accelerator

    if [ -f $HWACCELSTATS ]; then
        $HWACCELSTATS
    else
        if [ "$HW_ACCEL" == "ARCHER" ] ; then
            Archer_accel_dump
        elif  [ "$HW_ACCEL" == "RUNNER" ] ; then
            Runner_accel_dump
        fi
    fi

    ###
}

beacon_info () {
    echo "================================="
    echo "beacon content for $IFNAME"
    echo "================================="

    display_cmd_op "BEACON INFO: wl $IFNAME beacon_info" "$WLCMD $IFNAME beacon_info"
}

loop_commands () {
    NR_RUNS=`expr $NR_RUNS + 1`
    if [[ "$VERSION" -eq 2 ]]; then
        echo -e "\n==== Collecting data for run $NR_RUNS. Interval: $LOOP_TIME sec ===="
        if [[ "$DUMP_CLEAR" -eq 1 ]]; then
            echo "==== Previous WL dumps are cleared ===="
        fi
    fi
    sleep $LOOP_TIME
    echo "================================="
    echo "CPU Stats"
    echo "================================="
    mpstat -P ALL
    echo "================================="
    echo "Process status"
    echo "================================="
    ps -ax
    if [ $? != 0 ]; then
        ps
    fi
    top -n 1
    echo "================================="
    echo "Memory status"
    echo "================================="
    cat /proc/meminfo
    echo "================================="
    echo "Statistics for $IFNAME run $NR_RUNS/$NR_REPEATS @$LOOP_TIME s"
    echo "================================="
    wl_stats
    if [[ $MODE == "dhd" ]]; then
    dhd_stats "${SOCRAMDUMPFILE}_${NR_RUNS}"
    fi

    host_side_dump
    dmesg -c
}

if [[ $IFNAME == $PRIM_IFNAME ]]; then
    if [ $MODE == "dhd" ]; then
        $DHDCMD $IFNAME version > /dev/NULL 2>&1
        if [ $? -ne 0 ] ; then
            echo -e "Bad Interface $IFNAME $?"
            show_help
            exit 0
        fi
    fi

    $WLCMD $IFNAME ver > /dev/NULL 2>&1
    if [ $? -ne 0 ] ; then
        echo "########### Dongle trap detected on $IFNAME ################"
        exit 0
    fi
fi

#driver_init
driver_info
#display_cmd_op "IFCONFIG: ifconfig -a" "ifconfig -a"
$WLCMD $IFNAME cevent 1 > /dev/NULL 2>&1
echo "================================="
echo "Statistics for $IFNAME first run"
echo "================================="

wl_stats
dump_hapd

if [[ $MODE == "dhd" ]]; then
    dhd_dconpoll=`nvram get dconpoll`
    $DHDCMD $IFNAME dconpoll 250
    dhd_stats "${SOCRAMDUMPFILE}_${NR_RUNS}"
    dmesg -c
    if [[ "$dhd_dconpoll" == "" ]]; then
        $DHDCMD $IFNAME dconpoll 0
    else
		$DHDCMD $IFNAME dconpoll $dhd_dconpoll
    fi
fi

if [[ $NR_REPEATS -eq 0 ]]; then
    while true; do
        loop_commands
    done
else
    while [[ $NR_REPEATS -gt $NR_RUNS ]]; do
        loop_commands
    done
fi

beacon_info
dump_wpasupp

# restore msglevel
if [[ $IFNAME == $PRIM_IFNAME ]]; then
    $WLCMD $IFNAME msglevel $WLMSGLVL
    if [ $MODE == "dhd" ]; then
        $DHDCMD $IFNAME msglevel $DHDMSGLVL
    fi
fi
