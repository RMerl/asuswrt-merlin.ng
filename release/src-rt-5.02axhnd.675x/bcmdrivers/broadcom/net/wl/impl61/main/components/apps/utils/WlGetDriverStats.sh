#!/bin/sh
#
# WlGetDriverStats.sh <WiFi interface name> <Driver mode nic|dhd>
#
# Copyright (C) 2019, Broadcom. All Rights Reserved.
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
# <<Broadcom-WL-IPTag/Proprietary,Open:.*>>
#

WLCMD=wl
DHDCMD=dhd
IFNAME=$1
DUMPPATH=/tmp
DUMPDIR=${DUMPPATH}/$(date +"%d:%m:%Y_%H:%M")
SOCRAMDUMPFILE=${DUMPDIR}/socram_${IFNAME}
MODE=$2
LOOP_TIME=5

show_help () {
	echo "Syntax: $0 <WiFi interface name> <Driver mode: nic|dhd> <NrRetries> <SOCRAMDUMP: enable|disable>"
	echo "Example 1: $0 wl1 nic"
	echo "Example 2: $0 eth5 dhd 12 enable"
	echo "Try \`$0 --help' for more information."
	exit
}

# $1 Heading
# $2 cmd to execute
display_cmd_op(){
    cmd="$2"
    echo "---------------------------------"
    echo -e "$1"
    echo "---------------------------------"
    $cmd |
    while IFS= read -r line
    do
	echo -e "\t""$line"
    done
}

# Help option
if [[ $# -eq 1 ]] && [[ $1 == "--help" ]]; then
	show_help
	exit 0
fi

if [ $# -eq 0 ]; then
	show_help
	exit 0
fi

if [ $# -ge 3 ]; then
    NR_REPEATS=$3
else
    NR_REPEATS=0
fi

if [[ $MODE != "nic" ]] && [[ $MODE != "dhd" ]]; then
    echo "UNKNOWN driver mode!"
    show_help
    exit 0
fi

if [ $# -eq 4 ]; then
    if [[ $4 == "enable" ]] && [[ $MODE == "dhd" ]]; then
	ENABLEDUMP=1
    else
	if [ $MODE == "nic" ]; then
	    echo "SOCRAMDUMP not possible in NIC mode"
	fi
	ENABLEDUMP=0
    fi
fi

driver_init () {
    if [ $($WLCMD -i $IFNAME isup) -ne 1 ]; then
	echo -e "Interface $IFNAME is not up"
	exit 0
    fi
    $WLCMD -i $IFNAME bs_data > /dev/NULL 2>&1
    $WLCMD -i $IFNAME pktq_stats > /dev/NULL 2>&1
}

driver_info () {
    echo "============================="
    echo "$IFNAME Driver info"
    echo "============================="

    display_cmd_op "WLVERSION: wl -i $IFNAME ver" "$WLCMD -i $IFNAME ver"
    display_cmd_op "REVINFO: wl -i $IFNAME revinfo" "$WLCMD -i $IFNAME revinfo"
    display_cmd_op "DHDVERSION: dhd -i $IFNAME version" "$DHDCMD -i $IFNAME version"
    display_cmd_op "WLCAP: wl -i $IFNAME cap" "$WLCMD -i $IFNAME cap"
    display_cmd_op "UPTIME: System uptime" "uptime"
}

wl_stats () {
    echo "================================="
    echo "WL Statistics for $IFNAME"
    echo "================================="

    display_cmd_op "STATUS: wl -i $IFNAME status" "$WLCMD -i $IFNAME status"
    display_cmd_op "ASSOCLIST: wl -i $IFNAME assoclist" "$WLCMD -i $IFNAME assoclist"
    for x in $($WLCMD -i $IFNAME assoclist);
    do
	if [ $x != assoclist ] ; then
		display_cmd_op "STAINFO: stainfo for $x" "$WLCMD -i $IFNAME sta_info $x"
	fi
    done
    for x in $($WLCMD -i $IFNAME assoclist);
    do
	if [ $x != assoclist ] ; then
		display_cmd_op "RSSI: rssi for $x" "$WLCMD -i $IFNAME rssi $x"
	fi
    done
    display_cmd_op "AUTH STA LIST: wl -i $IFNAME authe_sta_list" "$WLCMD -i $IFNAME authe_sta_list"
    display_cmd_op "5G RATE: wl -i $IFNAME 5g_rate" "$WLCMD -i $IFNAME 5g_rate"
    display_cmd_op "2G RATE: wl -i $IFNAME 2g_rate" "$WLCMD -i $IFNAME 2g_rate"
    display_cmd_op "NRATE: wl -i $IFNAME nrate" "$WLCMD -i $IFNAME nrate"
    display_cmd_op "11H SPECT: wl -i $IFNAME spect" "$WLCMD -i $IFNAME spect"
    display_cmd_op "PHY_ED_THRESH: wl -i $IFNAME phy_ed_thresh" "$WLCMD -i $IFNAME phy_ed_thresh"
    display_cmd_op "CHANIMSTATS: wl -i $IFNAME chanim_stats" "$WLCMD -i $IFNAME chanim_stats"
    display_cmd_op "INTERFERENCE: wl -i $IFNAME interference" "$WLCMD -i $IFNAME interference"
    display_cmd_op "wl -i $IFNAME interference_override" "$WLCMD -i $IFNAME interference_override"
    display_cmd_op "BSDATA: wl -i $IFNAME bs_data" "$WLCMD -i $IFNAME bs_data"
    display_cmd_op "PKTQSTATS: wl -i $IFNAME pktqstats" "$WLCMD -i $IFNAME pktq_stats"
    display_cmd_op "TXQ DUMP: wl -i $IFNAME dump txq" "$WLCMD -i $IFNAME dump txq"
    display_cmd_op "PERFSTATS DUMP: wl -i $IFNAME dump perf_stats" "$WLCMD -i $IFNAME dump perf_stats"
    display_cmd_op "AMPDU DUMP: wl -i $IFNAME dump ampdu" "$WLCMD -i $IFNAME dump ampdu"
    display_cmd_op "AMSDU DUMP: wl -i $IFNAME dump amsdu" "$WLCMD -i $IFNAME dump amsdu"
    display_cmd_op "WLC DUMP: wl -i $IFNAME dump wlc" "$WLCMD -i $IFNAME dump wlc"
    display_cmd_op "TXBF DUMP: wl -i $IFNAME dump txbf" "$WLCMD -i $IFNAME dump txbf"
    display_cmd_op "MUTX DUMP: wl -i $IFNAME dump mutx" "$WLCMD -i $IFNAME dump mutx"
    display_cmd_op "MURX DUMP: wl -i $IFNAME dump murx" "$WLCMD -i $IFNAME dump murx"
    display_cmd_op "MSCHED DUMP: wl -i $IFNAME dump msched" "$WLCMD -i $IFNAME dump msched"
    display_cmd_op "UMSCHED DUMP: wl -i $IFNAME dump umsched" "$WLCMD -i $IFNAME dump umsched"
    display_cmd_op "D11CNTS DUMP: wl -i $IFNAME dump d11cnts" "$WLCMD -i $IFNAME dump d11cnts"
    display_cmd_op "WME COUNTERS: wl -i $IFNAME wme counters" "$WLCMD -i $IFNAME wme_counters"
    display_cmd_op "INTERFACE COUNTERS: wl -i $IFNAME if_counters" "$WLCMD -i $IFNAME if_counters"
    display_cmd_op "COUNTERS: wl -i $IFNAME counters" "$WLCMD -i $IFNAME counters"
    display_cmd_op "MEMORY USAGE: wl -i $IFNAME memuse" "$WLCMD -i $IFNAME memuse"
    display_cmd_op "CHANNEL INFO: wl -i $IFNAME chan_info" "$WLCMD -i $IFNAME chan_info"
    display_cmd_op "DFS STATUS: wl -i $IFNAME dfs_status_all" "$WLCMD -i $IFNAME dfs_status_all"
    display_cmd_op "NOISE: wl -i $IFNAME noise" "$WLCMD -i $IFNAME noise"
    display_cmd_op "RSSI ANTENNA: wl -i $IFNAME phy_rssi_ant" "$WLCMD -i $IFNAME phy_rssi_ant"
    display_cmd_op "TEMPERATURE SENSOR: wl -i $IFNAME phy_tempsense" "$WLCMD -i $IFNAME phy_tempsense"
    display_cmd_op "RATE: wl -i $IFNAME rate" "$WLCMD -i $IFNAME rate"
    display_cmd_op "BAND: wl -i $IFNAME band" "$WLCMD -i $IFNAME band"
    display_cmd_op "SCAN RESULTS: wl -i $IFNAME scanresults" "$WLCMD -i $IFNAME scanresults"
}

dhd_stats () {
    echo "================================="
    echo "DHD Statistics for $IFNAME"
    echo "================================="
    display_cmd_op "DHDDUMP: dhd -i $IFNAME DHD dump" "$DHDCMD -i $IFNAME dump"
    if [ $ENABLEDUMP -ne 0 ] ; then
	mkdir $DUMPDIR
	display_cmd_op "SOCRAMDUMP: Uploading DUMP to $1" "$DHDCMD -i $IFNAME upload $1"
    fi
    $DHDCMD -i $IFNAME cons mu
}

if [ $MODE == "dhd" ]; then
$DHDCMD -i $IFNAME version > /dev/NULL 2>&1
if [ $? -ne 0 ] ; then
	echo -e "Bad Interface $IFNAME $?"
	show_help
	exit 0
fi
fi

$WLCMD -i $IFNAME ver > /dev/NULL 2>&1
if [ $? -ne 0 ] ; then
	echo "########### Dongle trap detected on $IFNAME ################"
	exit 0
fi

#driver_init
driver_info
#display_cmd_op "IFCONFIG: ifconfig -a" "ifconfig -a"
NR_RUNS=0
echo "================================="
echo "Statistics for $IFNAME first run"
echo "================================="
wl_stats
if [[ $MODE == "dhd" ]]; then
    dhd_stats "${SOCRAMDUMPFILE}_${NR_RUNS}"
fi

while [[ $NR_REPEATS -gt $NR_RUNS ]]; do
	sleep $LOOP_TIME
	NR_RUNS=`expr $NR_RUNS + 1`
	echo "================================="
	echo "Statistics for $IFNAME run $NR_RUNS/$NR_REPEATS @$LOOP_TIME s"
	echo "================================="
	wl_stats
	if [[ $MODE == "dhd" ]]; then
	    dhd_stats "${SOCRAMDUMPFILE}_${NR_RUNS}"
	fi
	echo $NR_REPEATS
done
