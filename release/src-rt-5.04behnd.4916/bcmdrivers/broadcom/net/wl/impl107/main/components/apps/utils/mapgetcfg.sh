#!/bin/sh
#
# mapgetcfg.sh [loop-count] [loop-interval]
# By default loop-count=1 loop-interval=0 ==> Run the script once
#
# Copyright (C) 2025, Broadcom. All Rights Reserved.
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
# $Id: mapgetcfg.sh 855352 2025-05-06 11:33:14Z $

######################################################
#
# Functions
#
######################################################

# Wrapper function to print the command and execute it
exec_cmd() {
	echo "#" $*
	$*
	echo
	echo
}

do_WlGetDriverCfg() {
	ifname=$1
	band=$2
	fwid=$(wl -i $ifname ver | grep -c FWID)
	if [ $fwid = "1" ]
	then
		mode="dhd"
	else
		mode="nic"
	fi
	exec_cmd WlGetDriverCfg.sh $ifname $band $mode
}

# Get List of BSS in all radios
get_BSSList() {
	ifnames=`nvram get lan_ifnames; nvram get lan1_ifnames`
	for ifname in $ifnames
	do
		# Check if it is a wireless interface or not
		wl -i $ifname ver >& /dev/null
		if [ $? -eq 0 ]
		then
			echo -n "$ifname "
		fi
	done
}

# Get List of Bridges in the device
get_BridgeList() {
	brctl show | grep -v "bridge name" | cut -f1
}

# Get List of primary interface names of all radios
get_RadioList() {
	radionames=`nvram get wl0_ifname; nvram get wl1_ifname; nvram get wl2_ifname`
	echo $radionames
}

# Get the frequency band 2 / 5 / 6
get_Frequency() {
	radio=$1
	band=`wl -i $radio band`
	if [ "$band" == "a" ]
	then
		echo 5
	elif [ "$band" == "b" ]
	then
		echo 2
	elif [ "$band" == "6g" ]
	then
		echo 6
	elif [ "$band" == "auto" ]
	then
		echo 2
	fi
}

######################################################
#
# Script starts here
#
######################################################

if [ ! -z $2 ]
then
	loop_count=$1
	sleep_time=$2
else
	echo "Usage : $0 [loop-count] [loop-interval]"
	echo "    Ex : $0 5 2  # Run all commands 5 times with 2 seconds gap"
	echo "    Using Default : Run all commands once"
	loop_count=1
	sleep_time=0
fi

bridgelist=`get_BridgeList`
radiolist=`get_RadioList`
bsslist=`get_BSSList`

for radio in $radiolist
do
	band=`get_Frequency $radio`
	do_WlGetDriverCfg $radio $band
done

exec_cmd ifconfig

cnt=1
while [ $cnt -le $loop_count ]
do
	echo "============================== Iteration : $cnt =============================="
	exec_cmd date
	exec_cmd ps
	exec_cmd wb_cli -m info
	exec_cmd wb_cli -s info
	exec_cmd wb_cli -m logs
	exec_cmd bsd -l
	exec_cmd bsd -s
	exec_cmd i5ctl -m dm
	exec_cmd i5ctl dm
	exec_cmd i5ctl -m dme
	exec_cmd i5ctl dme
	exec_cmd i5ctl -m keys
	exec_cmd i5ctl keys
	exec_cmd i5ctl -m dumpSockets
	exec_cmd i5ctl dumpSockets
	exec_cmd i5ctl -m dumpMsgs
	exec_cmd i5ctl dumpMsgs
	exec_cmd brctl show
	exec_cmd ceventc dump
	exec_cmd ebtables -t broute --list
	exec_cmd nft -a list ruleset
	exec_cmd wl --all mlo info
	exec_cmd wl --all mlo_unit
	exec_cmd wl --all mld_unit
	exec_cmd cat /tmp/map_agt_hapd.conf
	exec_cmd cat /tmp/map_agt_wpa_supplicant.conf
	exec_cmd cat /tmp/map_ctrl_hapd.conf
	exec_cmd netstat -tulpn
	exec_cmd netstat -apeen
	exec_cmd ip -d link

	# archer stats is printed from the firmware directly. So the output gets mixed with
	# previous outputs. Add additional sleep to avoid it. The command output will not
	# be captured if the script is run from a telnet window
	exec_cmd sleep 2
	exec_cmd archer stats

	for bridge in $bridgelist
	do
		exec_cmd brctl showstp $bridge
		exec_cmd brctl showmacs $bridge
	done

	for radio in $radiolist
	do
		exec_cmd wl -i $radio band
		exec_cmd wl -i $radio chanspec
		exec_cmd wl -i $radio keep_ap_up
		exec_cmd wl -i $radio dfs_status_all
		exec_cmd wl -i $radio chan_info
		exec_cmd wl -i $radio chanim_stats
		exec_cmd wl -i $radio counters
		exec_cmd wl -i $radio pktq_stats C: A:
		exec_cmd wl -i $radio dump ptrack
		exec_cmd wl -i $radio ptclear
		exec_cmd wl -i $radio cap
		exec_cmd wl -i $radio dump wlc
		exec_cmd wl -i $radio dump amt
		exec_cmd cat /tmp/${radio}_wpa_supplicant.conf
		exec_cmd wpa_cli -i $radio -i$radio scan_results
		exec_cmd wpa_cli -i $radio list_network
		exec_cmd wpa_cli -i $radio status
		exec_cmd acs_cli2 -i $radio dump acs_record
	done

	for bss in $bsslist
	do
		exec_cmd wl -i $bss status
		exec_cmd wl -i $bss assoclist
		exec_cmd wl -i $bss sta_info all
		exec_cmd wl -i $bss bs_data -noreset
		exec_cmd wl -i $bss macmode
		exec_cmd wl -i $bss mac
		exec_cmd wl -i $bss sta_monitor
		exec_cmd wl -i $bss apsta
		exec_cmd wl -i $bss ap
		exec_cmd wl -i $bss map
		exec_cmd wl -i $bss split_assoc_req
		exec_cmd wl -i $bss rrm
		exec_cmd wl -i $bss rrm_nbr_list
		exec_cmd wl -i $bss dump mlo
		exec_cmd wl -i $bss dump swkeys
		for sta in `wl -i $bss sta_monitor`
		do
			exec_cmd wl -i $bss sta_monitor stats $sta
		done
		exec_cmd hostapd_cli -i $bss all_sta
		exec_cmd cat /tmp/${bss}_hapd.conf
		for sta in `wl -i $bss assoclist`
		do
			exec_cmd wl -i $bss mlo scb_stats $sta
		done
	done
	exec_cmd sleep $sleep_time
	let "cnt += 1"
done

# Do scan at the end to avoid channel switch during rest of the script execution
for radio in $radiolist
do
	exec_cmd wl -i $radio escanresults
done
