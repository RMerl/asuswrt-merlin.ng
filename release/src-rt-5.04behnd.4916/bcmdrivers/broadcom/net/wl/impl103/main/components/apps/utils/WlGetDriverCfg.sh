#!/bin/sh
#
# WlGetDriverCfg.sh <WiFi interface name> <Band: 2|5|6> <Driver mode: nic|dhd>
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
IFNAME="-e"
PRIM_IFNAME="-p"
MODE="dhd"
BAND="5"

show_help () {
	echo "Two input formats are supported by this script."
	echo "First one is for backward compatibility."
	echo "Second one to make it more user friendly, and developer friendly for future expansion."
	echo -e "\nFirst input format:"
	echo "Syntax: $0 <WiFi interface name> <Band: 2|5|6> <Driver mode: auto|nic|dhd>"
	echo "Example 1: $0 wl1 2 auto"
	echo "Example 2: $0 wl1 2 nic"
	echo "Example 3: $0 wl0 5 dhd"
	echo "Try \`$0 --help' for more information."

	echo -e "\nSecond input format:"
	echo "Syntax: $0 [-i ifname] [-b band] [-m mode] "
	echo "Options:"
	echo -e "\t-i ifname: Name of the WLAN driver interface. Default is wl0."
	echo -e "\t-m mode: Valid driver modes: auto|nic|dhd. Default is auto."
	echo -e "\t-b band: 2|5|6"
	exit
}

# $1 Heading
# $2 cmd to execute
display_cmd_op(){
	cmd="$2"
	echo "---------------------------------------------"
	echo -e "$1"
	echo "---------------------------------------------"
	$cmd |
	while IFS= read -r line
	do
		echo -e "\t""$line"
	done
	echo ""
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

# Help option
if [[ $# -eq 1 ]] && [[ $1 == "--help" ]]; then
	show_help
	exit 0
fi

# Invalid # of parameters
if [[ $# -ne 3 ]] && [[ $# -ne 4 ]]; then
echo "Invalid number of parameters!"
show_help
exit 0
fi

# Argument 4 is skip dump nvram
skip_nvram=$4
if [[ $skip_nvram == "1" ]]; then
	skip_nvram="1"
else
	skip_nvram="0"
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
		-b|--band)
		BAND="$2"
		shift # past argument
		shift # past value
		;;
		*)    # unknown option
		echo "Unknown Option $1"
		show_help
		exit 0;
	esac
	done

else
	if [ $# -eq 3 ]; then
		# Argument 1 is WiFi interface
		IFNAME="-i $1"
		PRIM_IFNAME=$IFNAME

		# Argument 2 is band
		BAND=$2
		if [[ $BAND != "2" ]] && [[ $BAND != "5" ]] && [[ $BAND != "6" ]] ; then
		echo "Invalid band!"
		show_help
		exit 0
		fi

		# Argument 3 is driver mode
		MODE=$3
		if [ $MODE = "auto" ]; then
			fwid=$($WLCMD $IFNAME ver | grep -c FWID)
			if [ $fwid = "1" ]; then
				MODE="dhd"
			else
				MODE="nic"
			fi
		fi

		if [[ $MODE != "nic" ]] && [[ $MODE != "dhd" ]]; then
		echo "Invalid driver mode!"
		show_help
		exit 0
		fi

		ifconfig $1 1>/dev/null 2>&1
		if [ $? != 0 ]; then
		echo "Invalid interface!"
		show_help
		exit 0
		fi
	fi
fi # end of input argument parsing

echo ""
echo ""
driver_info

# Overwrite msglevel
if [[ $IFNAME == $PRIM_IFNAME ]]; then
	WLMSGLVL=`$WLCMD $IFNAME msglevel | cut -d ' ' -f1`
	$WLCMD $IFNAME msglevel 0
	if [[ $MODE == "dhd" ]]; then
		DHDMSGLVL=`$DHDCMD $IFNAME msglevel | cut -d ' ' -f1`
		$DHDCMD $IFNAME msglevel 0
	fi
fi

echo ""
echo ""
echo "============================="
echo "$IFNAME WL config parameters"
echo "============================="
items="allmulti rtsthresh clmver rtsthresh ack_ratio ack_ratio_depth
	ampdu ampdu_rts amsdu ap_isolate apsta auth ampdu_ba_wsize bcn_rotate bss bss_maxassocbtc_mode btc_mode
	chanim_mode chanim_sample_period chanspec closednet country dwds eap_restrict event_msgs event_msgs_ext
	fragthresh he hw_rxchain hw_txchain interference interference_override leddc lrl srl maxscb maxassoc mbss
	mbss_ign_mac_valid mcast_list min_txpower mode_reqd mpc msglevel vhtmode vht_features mu_features mu_policy
	muinfo wet_enab nar nar_transit_limit nmode mac macmode nmode_protection_override noise_metric obss_coex
	obss_dyn_bw dyn_bwsw_params osen chan_info acksupr_mac_filter authresp_mac_filter probresp_mac_filter probresp_sw
	pspretend_retry_limit radar radio_pwrsave_enable radio_pwrsave_level radio_pwrsave_pps radio_pwrsave_quiet_time
	radio_pwrsave_stas_assoc_check regulatory rifs rifs_advert rtsthresh rxchain roam_trigger nrate rate rxchain_pwrsave_enable
	rxchain_pwrsave_pps rxchain_pwrsave_quiet_time rxchain_pwrsave_stas_assoc_check sar_enable spect split_assoc_req
	stbc_rx stbc_tx ampdu_density ampdu_mpdu chanim_stats rx_amsdu_in_ampdu frameburst frameburst_override obss_coex
	sgi_rx sgi_tx txbf txbf_bfe_cap txbf_bfr_cap txbf_imp txchain txchain_pwr_offset vlan_mode vndr_ie wdstimeout
	wme wme_ac_ap wme_ac_sta wme_apsd wme_bss_disable wme_noack wmf_bss_enable wnm_url wpa_auth wpa_cap wsec wsec_restrict
	PM promisc cwmin cwmax infra bssid bssmax channel chanspecs txpwr txpwr1 eap cur_etheraddr perm_etheraddr rateset
	txbf_rateset txbf_rateset roam_delta prb_resp_timeout shortslot protection_control legacy_erp txpwr_percent arpoe wet bi
	list_ie pmkid_info intfer_params mempool pm2_sleep_ret_ext monitor_promisc_level desired_bssid ht_features atim phy_afeoverride
	phy_rxiqest phy_lesi smth_enable povars radarargs pkt_filter_list txcore txcore_override mimo_ss_stf spatial_policy tpc_mode ap
	scb_timeout beacon_info probe_resp_info cur_mcsset mimo_ps wepstatus primary_key passive scan_channel_time scan_unassoc_time
	scan_home_time scan_passive_time scan_nprobes scansuppress obss_scan_params mrate curppr suprates rrm msched umsched
	dfs_postism dfs_preism ldpc_cap ldpc_tx mfp pkteng_cmd twt_prestop twt_prestrt txbf_mutimer"

for x in $items; do
  display_cmd_op "$WLCMD $PRIM_IFNAME $x" "$WLCMD $PRIM_IFNAME $x"
done

items="ssid"
for x in $items; do
  display_cmd_op "$WLCMD $IFNAME $x" "$WLCMD $IFNAME $x"
done

echo "============================="
echo "$IFNAME MLO config parameters"
echo "============================="
items="mld_unit mld_nlinks mlo_enable mlo_unit"

for x in $items; do
  display_cmd_op "$WLCMD $IFNAME $x" "$WLCMD $IFNAME $x"
done

items="info emlresp"

for x in $items; do
  display_cmd_op "$WLCMD $IFNAME mlo $x" "$WLCMD $IFNAME mlo $x"
done

echo "============================="
echo "$IFNAME EHT config parameters"
echo "============================="
items="enab features bssehtmode dissubchan"

for x in $items; do
  display_cmd_op "$WLCMD $PRIM_IFNAME eht $x" "$WLCMD $PRIM_IFNAME eht $x"
done

echo "============================="
echo "$IFNAME HE config parameters"
echo "============================="
items="cap enab features bsscolor dynfrag muedca peduration ppet range_ext rtsdurthresh"

for x in $items; do
  display_cmd_op "$WLCMD $PRIM_IFNAME he $x" "$WLCMD $PRIM_IFNAME he $x"
done

echo "============================="
echo "$IFNAME TAF config parameters"
echo "============================="
items="enable"

for x in $items; do
  display_cmd_op "$WLCMD $PRIM_IFNAME taf $x" "$WLCMD $PRIM_IFNAME taf $x"
done

echo "============================="
echo "$IFNAME LIFETIME config parameters"
echo "============================="
items="vi vo be bk"

for x in $items; do
  display_cmd_op "$WLCMD $PRIM_IFNAME lifetime $x" "$WLCMD $PRIM_IFNAME lifetime $x"
done

echo "============================="
echo "$IFNAME PKT FILTER config parameters"
echo "============================="
items="0 1"

for x in $items; do
  display_cmd_op "$WLCMD $PRIM_IFNAME pkt_filter_list $x" "$WLCMD $PRIM_IFNAME pkt_filter_list $x"
done

echo "============================="
echo "$IFNAME MSCHED config parameters"
echo "============================="
items="rucfg maxn"

for x in $items; do
  display_cmd_op "$WLCMD $PRIM_IFNAME msched $x" "$WLCMD $PRIM_IFNAME msched $x"
done

#
# This is not needed for 11ax
#

if [[ $BAND == "6" ]]; then
	items="6g_mrate 6g_rate"

	for x in $items; do
		display_cmd_op "$WLCMD $IFNAME $x" "$WLCMD $IFNAME $x"
	done

fi

if [[ $BAND == "5" ]]; then
	items="5g_mrate 5g_rate a_rate"

	for x in $items; do
		display_cmd_op "$WLCMD $IFNAME $x" "$WLCMD $IFNAME $x"
	done

fi

if [[ $BAND == "2" ]]; then
	items="2g_mrate 2g_rate bg_rate bg_mrate gmode gmode_protection gmode_protection_control gmode_protection_override"

	for x in $items; do
		display_cmd_op "$WLCMD $IFNAME $x" "$WLCMD $IFNAME $x"
	done
fi

echo ""
echo ""
echo "---------------------"
echo "$IFNAME WL nvram_dump"
echo "---------------------"
wl $IFNAME nvram_dump

if [[ $MODE == "dhd" ]]; then

	echo ""
	echo ""
	echo "============================="
	echo "$IFNAME DHD config parameters"
	echo "============================="
	items="alignctl ap_isolate aspm assert_type bcmerrorstr block_ping cons db1_for_mb dconpoll devcap devreset devsleep dhcp_unicast
		dma_ring_indices dngl_isolation flow_prio_map forceeven fw_hang_report grat_arp host_reorder_flows idleclock idletime ioctl_timeout
		kso lmtest logdump ltrsleep_on_unload mcast_regen_bss_enable msi_sim oob_bt_reg_on pmodule_ignore pollrate proptx proptx_opt
		proxy_arp ptxmode ptxstatus_ignore ramsize ramstart readahead rxbound rxpkt_chk sleep sleep_allowed srdump tcpack_suppress tuning_mode
		txbound txglommode txglomsize txinrx_thres txminmax txp_thresh wdtick wmf_bss_enable wmf_mcast_data_sendup
		wmf_psta_disable wmf_ucast_igmp wmf_ucast_upnp wowl_wakeind op_mode "

	for x in $items; do
		display_cmd_op "$DHDCMD $PRIM_IFNAME $x" "$DHDCMD $PRIM_IFNAME $x"
	done

	echo "============================="
	echo "$IFNAME DHD pciecfgreg parameters"
	echo "============================="
	items="0xb4"

	for x in $items; do
		display_cmd_op "$DHDCMD $PRIM_IFNAME pciecfgreg $x" "$DHDCMD $PRIM_IFNAME pciecfgreg $x"
	done

fi

if [[ $skip_nvram != "1" ]]; then
echo ""
echo ""
echo "-----------------------"
echo "NVRAM config parameters"
echo "-----------------------"
nvram show
fi

# restore msglevel
if [[ $IFNAME == $PRIM_IFNAME ]]; then
	$WLCMD $IFNAME msglevel $WLMSGLVL
	if [[ $MODE == "dhd" ]]; then
		$DHDCMD $IFNAME msglevel $DHDMSGLVL
	fi
fi
# Done
