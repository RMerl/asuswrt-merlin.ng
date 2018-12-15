#!/bin/sh
#
# WlGetDriverStats.sh <WiFi interface name> <Driver mode nic|dhd>
#
# Copyright (C) 2018, Broadcom. All Rights Reserved.
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
CEVENTCCMD=ceventc
IFNAME=$1
DUMPPATH=/tmp
DUMPDIR=${DUMPPATH}/$(date +"%d:%m:%Y_%H:%M")
SOCRAMDUMPFILE=${DUMPDIR}/socram_${IFNAME}
MODE=$2
LOOP_TIME=5

show_help () {
	echo "Syntax: $0 <WiFi interface name> <Driver mode: nic|dhd> <NrRetries> <SOCRAMDUMP: enable|disable>"
	echo "Example 1: $0 wl1 nic"
	echo "Example 2: $0 eth5 dhd enable"
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
	if [[ $MODE == "dhd" ]]; then
		display_cmd_op "DHDVERSION: dhd -i $IFNAME version" "$DHDCMD -i $IFNAME version"
	fi
    display_cmd_op "WLCAP: wl -i $IFNAME cap" "$WLCMD -i $IFNAME cap"
    display_cmd_op "UPTIME: System uptime" "uptime"
}

wl_stats () {
    echo "================================="
    echo "WL Statistics for $IFNAME"
    echo "================================="

    display_cmd_op "STATUS: wl -i $IFNAME status" "$WLCMD -i $IFNAME status"
    display_cmd_op "ASSOCLIST: wl -i $IFNAME assoclist" "$WLCMD -i $IFNAME assoclist"
    for x in $($WLCMD -i $IFNAME assoclist | cut -d ' ' -f 2);
    do
	display_cmd_op "STAINFO: stainfo for $x" "$WLCMD -i $IFNAME sta_info $x"
    done
    for x in $($WLCMD -i $IFNAME assoclist | cut -d ' ' -f 2);
    do
	display_cmd_op "RSSI: rssi for $x" "$WLCMD -i $IFNAME rssi $x"
    done
    display_cmd_op "AUTH STA LIST: wl -i $IFNAME authe_sta_list" "$WLCMD -i $IFNAME authe_sta_list"
    display_cmd_op "5G RATE: wl -i $IFNAME 5g_rate" "$WLCMD -i $IFNAME 5g_rate"
    display_cmd_op "2G RATE: wl -i $IFNAME 2g_rate" "$WLCMD -i $IFNAME 2g_rate"
    display_cmd_op "NRATE: wl -i $IFNAME nrate" "$WLCMD -i $IFNAME nrate"
    display_cmd_op "11H SPECT: wl -i $IFNAME spect" "$WLCMD -i $IFNAME spect"
    display_cmd_op "CHANIMSTATS: wl -i $IFNAME chanim_stats" "$WLCMD -i $IFNAME chanim_stats"
    display_cmd_op "INTERFERENCE: wl -i $IFNAME interference" "$WLCMD -i $IFNAME interference"
    display_cmd_op "wl -i $IFNAME interference_override" "$WLCMD -i $IFNAME interference_override"
    display_cmd_op "BSDATA: wl -i $IFNAME bs_data" "$WLCMD -i $IFNAME bs_data"
    display_cmd_op "PKTQSTATS: wl -i $IFNAME pktqstats" "$WLCMD -i $IFNAME pktq_stats"
    display_cmd_op "AMPDU DUMP: wl -i $IFNAME dump ampdu" "$WLCMD -i $IFNAME dump ampdu"
    display_cmd_op "AMSDU DUMP: wl -i $IFNAME dump amsdu" "$WLCMD -i $IFNAME dump amsdu"
    display_cmd_op "WME_COUNTERS: wl -i $IFNAME wme counters" "$WLCMD -i $IFNAME wme_counters"
    display_cmd_op "COUNTERS: wl -i $IFNAME counters" "$WLCMD -i $IFNAME counters"
    display_cmd_op "COUNTERS: wl -i $IFNAME memuse" "$WLCMD -i $IFNAME memuse"
    display_cmd_op "COUNTERS: wl -i $IFNAME chan_info" "$WLCMD -i $IFNAME chan_info"
    display_cmd_op "COUNTERS: wl -i $IFNAME dfs_status_all" "$WLCMD -i $IFNAME dfs_status_all"
    display_cmd_op "COUNTERS: wl -i $IFNAME noise" "$WLCMD -i $IFNAME noise"
    display_cmd_op "COUNTERS: wl -i $IFNAME phy_rssi_ant" "$WLCMD -i $IFNAME phy_rssi_ant"
    display_cmd_op "COUNTERS: wl -i $IFNAME phy_tempsense" "$WLCMD -i $IFNAME phy_tempsense"
    display_cmd_op "COUNTERS: wl -i $IFNAME rate" "$WLCMD -i $IFNAME rate"
    display_cmd_op "COUNTERS: wl -i $IFNAME band" "$WLCMD -i $IFNAME band"
    display_cmd_op "COUNTERS: wl -i $IFNAME scanresults" "$WLCMD -i $IFNAME scanresults"
    display_cmd_op "WLC DUMP: wl -i $IFNAME dump wlc" "$WLCMD -i $IFNAME dump wlc"
    display_cmd_op "BSS DUMP: wl -i $IFNAME dump bsscfg" "$WLCMD -i $IFNAME dump bsscfg"
    display_cmd_op "SCB DUMP: wl -i $IFNAME dump scb" "$WLCMD -i $IFNAME dump scb"
    display_cmd_op "SMF DUMP: wl -i $IFNAME dump smfstats" "$WLCMD -i $IFNAME dump smfstats"
    display_cmd_op "WME DUMP: wl -i $IFNAME dump wme" "$WLCMD -i $IFNAME dump wme"
    display_cmd_op "TXQ DUMP: wl -i $IFNAME dump txq" "$WLCMD -i $IFNAME dump txq"
    display_cmd_op "DMA DUMP: wl -i $IFNAME dump dma [-t tx -f all]" "$WLCMD -i $IFNAME dump dma [-t tx -f all]"
    display_cmd_op "AQM DUMP: wl -i $IFNAME dump dma [-t aqm -f all]" "$WLCMD -i $IFNAME dump dma [-t aqm -f all]"
    display_cmd_op "RSSI DUMP: wl -i $IFNAME dump rssi" "$WLCMD -i $IFNAME dump rssi"
    display_cmd_op "MACFLTR DUMP: wl -i $IFNAME dump macfltr" "$WLCMD -i $IFNAME dump macfltr"
    display_cmd_op "RATELINK DUMP: wl -i $IFNAME dump ratelinkmem" "$WLCMD -i $IFNAME dump ratelinkmem"
    display_cmd_op "PHYACI DUMP: wl -i $IFNAME dump phyaci" "$WLCMD -i $IFNAME dump phyaci"
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

if [[ $MODE == "dhd" ]]; then
	$DHDCMD -i $IFNAME version > /dev/NULL 2>&1
	if [ $? -ne 0 ] ; then
		echo -e "Bad Interface $IFNAME $?"
		show_help
		exit 0
	fi
fi

$WLCMD -i $IFNAME ver > /dev/NULL 2>&1
if [ $? -ne 0 ] ; then
if [[ $MODE == "dhd" ]]; then
	echo "########### Dongle trap detected on $IFNAME ################"
else
	echo -e "Bad Interface $IFNAME $?"
fi
	exit 0
fi

ceventc_dump() {
	echo "================================="
	echo "CEVENT DUMP for $IFNAME"
	echo "================================="
	display_cmd_op "CEVENTCDUMP: ceventc -i $IFNAME dump" "$CEVENTCCMD -i $IFNAME dump"
}

#driver_init
#driver_info
#display_cmd_op "IFCONFIG: ifconfig -a" "ifconfig -a"
NR_RUNS=0
echo "================================="
echo "Statistics for $IFNAME first run"
echo "================================="
wl_stats
if [[ $MODE == "dhd" ]]; then
    dhd_stats "${SOCRAMDUMPFILE}_${NR_RUNS}"
fi
ceventc_dump

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
	ceventc_dump
	echo $NR_REPEATS
done
