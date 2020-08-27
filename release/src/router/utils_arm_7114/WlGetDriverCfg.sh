#!/bin/sh
#
# WlGetDriverCfg.sh <WiFi interface name> <Band: 2|5> <Driver mode: nic|dhd>
#
# Copyright (C) 2020, Broadcom. All Rights Reserved.
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
# $Id: WlGetDriverCfg.sh 665127 2016-10-15 01:43:58Z $
#
# <<Broadcom-WL-IPTag/Proprietary,Open:.*>>
#
IFS=
show_help () {
	echo "Syntax: $0 <WiFi interface name> <Band: 2|5> <Driver mode: auto|nic|dhd>"
	echo "Example 1: $0 wl1  2 auto"
	echo "Example 2: $0 eth5 2 nic"
	echo "Example 3: $0 eth5 5 dhd"
	#echo "Try \`$0 --help' for more information."
	exit
}

# Help option
if [[ $# -eq 1 ]] && [[ $1 == "--help" ]]; then
	show_help
	exit 0
fi

# Invalid # of parameters
if [[ $# -ne 3 ]]; then
echo "Invalid number of parameters!"
show_help
exit 0
fi

# Argument 1 is WiFi interface
ifname=$1

# Argument 2 is band
band=$2
if [[ $band != "2" ]] && [[ $band != "5" ]]; then
echo "Invalid band!"
show_help
exit 0
fi

# Argument 3 is driver mode
mode=$3
if [ $mode = "auto" ]; then
	fwid=$(wl -i $ifname ver | grep -c FWID)
	if [ $fwid = "1" ]; then
		mode="dhd"
	else
		mode="nic"
	fi
fi

if [[ $mode != "nic" ]] && [[ $mode != "dhd" ]]; then
echo "Invalid driver mode!"
show_help
exit 0
fi

ifconfig $ifname 1>/dev/null 2>&1
if [ $? != 0 ]; then
  echo "Invalid interface!"
  show_help
  exit 0
fi

echo ""
echo ""
echo -n $ifname WL driver version = ; echo $(wl -i $ifname ver)
echo -n $ifname WL revinfo = ; echo $(wl -i $ifname revinfo)

# Overwrite msglevel
WLMSGLVL=`wl -i $ifname msglevel | cut -d ' ' -f1`
wl -i $ifname msglevel 0
if [[ $mode == "dhd" ]]; then
  echo -n $ifname DHD version = ; echo $(dhd -i $ifname ver)
  DHDMSGLVL=`dhd -i $ifname msglevel | cut -d ' ' -f1`
  dhd -i $ifname msglevel 0
fi

echo ""
echo ""
echo "----------------------------"
echo "$ifname WL config parameters"
echo "----------------------------"
echo -n allmulti=; echo $(wl -i $ifname allmulti)
echo -n clmver=; echo $(wl -i $ifname clmver)
echo -n bw_cap $band"g"=; echo $(wl -i $ifname bw_cap $band"g")
echo -n rtsthresh=; echo $(wl -i $ifname rtsthresh)
echo -n ack_ratio=; echo $(wl -i $ifname ack_ratio)
echo -n ack_ratio_depth=; echo $(wl -i $ifname ack_ratio_depth)
echo -n ampdu=; echo $(wl -i $ifname ampdu)
echo -n ampdu_rts=; echo $(wl -i $ifname ampdu_rts)
echo -n amsdu=; echo $(wl -i $ifname amsdu)
echo -n ap_isolate=; echo $(wl -i $ifname ap_isolate)
echo -n apsta=; echo $(wl -i $ifname apsta)
echo -n aspm=; echo $(wl -i $ifname aspm)
echo -n atf =; echo $(wl -i $ifname atf)
echo -n ampdu_atf_us =; echo $(wl -i $ifname ampdu_atf_us)
echo -n auth=; echo $(wl -i $ifname auth)
echo -n bcm_dcs=; echo $(wl -i $ifname bcm_dcs)
echo -n ampdu_ba_wsize=; echo $(wl -i $ifname ampdu_ba_wsize)
echo -n bcn_rotate=; echo $(wl -i $ifname bcn_rotate)
echo -n bss=; echo $(wl -i $ifname bss)
echo -n bss_maxassoc=; echo $(wl -i $ifname bss_maxassoc)
echo -n btc_mode=; echo $(wl -i $ifname btc_mode)
echo -n cap=; echo $(wl -i $ifname cap)
echo -n chanim_mode=; echo $(wl -i $ifname chanim_mode)
echo -n chanim_sample_period=; echo $(wl -i $ifname chanim_sample_period)
echo -n chanspec=; echo $(wl -i $ifname chanspec)
echo -n closednet=; echo $(wl -i $ifname closednet)
echo -n country=; echo $(wl -i $ifname country)
echo -n dwds=; echo $(wl -i $ifname dwds)
echo -n eap_restrict=; echo $(wl -i $ifname eap_restrict)
echo -n event_msgs=; echo $(wl -i $ifname event_msgs)
echo -n event_msgs_ext=; echo $(wl -i $ifname event_msgs_ext)
echo -n fragthresh=; echo $(wl -i $ifname fragthresh)
echo -n he=; echo $(wl -i $ifname he)
echo -n he_cap=; echo $(wl -i $ifname he cap)
echo -n he_enab=; echo $(wl -i $ifname he enab)
echo -n he_features=; echo $(wl -i $ifname he features)
echo -n he_bsscolor=; echo $(wl -i $ifname he bsscolor)
echo -n he_partialbsscolor=; echo $(wl -i $ifname he partialbsscolor)
echo -n he_dynfrag=; echo $(wl -i $ifname he dynfrag)
echo -n he_htc=; echo $(wl -i $ifname he htc)
echo -n he_muedca=; echo $(wl -i $ifname he muedca)
echo -n he_peduration=; echo $(wl -i $ifname he peduration)
echo -n he_ppet=; echo $(wl -i $ifname he ppet)
echo -n he_range_ext=; echo $(wl -i $ifname he range_ext)
echo -n he_rtsdurthresh=; echo $(wl -i $ifname he rtsdurthresh)
echo -n hw_rxchain=; echo $(wl -i $ifname hw_rxchain)
echo -n hw_txchain=; echo $(wl -i $ifname hw_txchain)
echo -n interference=; echo $(wl -i $ifname interference)
echo -n interference_override=; echo $(wl -i $ifname interference_override)
echo -n leddc=; echo $(wl -i $ifname leddc)
echo -n lrl=; echo $(wl -i $ifname lrl)
echo -n maxassoc=; echo $(wl -i $ifname maxassoc)
echo -n mbss=; echo $(wl -i $ifname mbss)
echo -n mcast_list=; echo $(wl -i $ifname mcast_list)
echo -n min_txpower=; echo $(wl -i $ifname min_txpower)
echo -n mode_reqd=; echo $(wl -i $ifname mode_reqd)
echo -n mpc=; echo $(wl -i $ifname mpc)
echo -n msglevel=; echo $(wl -i $ifname msglevel)
echo -n vhtmode=; echo $(wl -i $ifname vhtmode)
echo -n vht_features=; echo $(wl -i $ifname vht_features)
echo -n mu_features=; echo $(wl -i $ifname mu_features)
echo -n mu_policy=; echo $(wl -i $ifname mu_policy)
echo -n muinfo=; echo $(wl -i $ifname muinfo)
echo -n wet_enab=; echo $(wl -i $ifname wet_enab)
echo -n nar=; echo $(wl -i $ifname nar)
echo -n nar_handle_ampdu=; echo $(wl -i $ifname nar_handle_ampdu)
echo -n nar_transit_limit=; echo $(wl -i $ifname nar_transit_limit)
echo -n nmode=; echo $(wl -i $ifname nmode)
echo -n mac=; echo $(wl -i $ifname mac)
echo -n macmode=; echo $(wl -i $ifname macmode)
echo -n nmode_protection_override=; echo $(wl -i $ifname nmode_protection_override)
echo -n noise_metric=; echo $(wl -i $ifname noise_metric)
echo -n obss_coex=; echo $(wl -i $ifname obss_coex)
echo -n obss_dyn_bw=; echo $(wl -i $ifname obss_dyn_bw)
echo -n dyn160=; echo $(wl -i $ifname dyn160)
echo -n dyn_bwsw_params=; echo $(wl -i $ifname dyn_bwsw_params)
echo -n phy_dyn_switch=; echo $(wl -i $ifname phy_dyn_switch)
echo -n phy_dyn_switch_th=; echo $(wl -i $ifname phy_dyn_switch_th)
echo -n osen=; echo $(wl -i $ifname osen)
echo -n per_chan_info=; echo $(wl -i $ifname per_chan_info)
echo -n pref_chanspec=; echo $(wl -i $ifname pref_chanspec)
echo -n acksupr_mac_filter=; echo $(wl -i $ifname acksupr_mac_filter)
echo -n authresp_mac_filter=; echo $(wl -i $ifname authresp_mac_filter)
echo -n probresp_mac_filter=; echo $(wl -i $ifname probresp_mac_filter)
echo -n probresp_sw=; echo $(wl -i $ifname probresp_sw)
echo -n pspretend_retry_limit=; echo $(wl -i $ifname pspretend_retry_limit)
echo -n psta=; echo $(wl -i $ifname psta)
echo -n psta_if=; echo $(wl -i $ifname psta_if)
echo -n psta_inact=; echo $(wl -i $ifname psta_inact)
echo -n radar=; echo $(wl -i $ifname radar)
echo -n radio_pwrsave_enable=; echo $(wl -i $ifname radio_pwrsave_enable)
echo -n radio_pwrsave_level=; echo $(wl -i $ifname radio_pwrsave_level)
echo -n radio_pwrsave_pps=; echo $(wl -i $ifname radio_pwrsave_pps)
echo -n radio_pwrsave_quiet_time=; echo $(wl -i $ifname radio_pwrsave_quiet_time)
echo -n radio_pwrsave_stas_assoc_check=; echo $(wl -i $ifname radio_pwrsave_stas_assoc_check)
echo -n regulatory=; echo $(wl -i $ifname regulatory)
echo -n rifs=; echo $(wl -i $ifname  rifs)
echo -n rifs_advert=; echo $(wl -i $ifname rifs_advert)
echo -n rtsthresh=; echo $(wl -i $ifname rtsthresh)
echo -n rxchain=; echo $(wl -i $ifname rxchain)
echo -n roam_trigger=; echo $(wl -i $ifname roam_trigger)
echo -n nrate=; echo $(wl -i $ifname nrate)
echo -n rate=; echo $(wl -i $ifname rate)
echo -n rxchain_pwrsave_enable=; echo $(wl -i $ifname rxchain_pwrsave_enable)
echo -n rxchain_pwrsave_pps=; echo $(wl -i $ifname rxchain_pwrsave_pps)
echo -n rxchain_pwrsave_quiet_time=; echo $(wl -i $ifname rxchain_pwrsave_quiet_time)
echo -n rxchain_pwrsave_stas_assoc_check=; echo $(wl -i $ifname rxchain_pwrsave_stas_assoc_check)
echo -n sar_enable=; echo $(wl -i $ifname sar_enable)
echo -n spect=; echo $(wl -i $ifname spect)
echo -n split_assoc_req=; echo $(wl -i $ifname split_assoc_req)
echo -n srl=; echo $(wl -i $ifname srl)
echo -n ssid=; echo $(wl -i $ifname ssid)
echo -n stbc_rx=; echo $(wl -i $ifname stbc_rx)
echo -n stbc_tx=; echo $(wl -i $ifname stbc_tx)
echo -n ampdu_density=; echo $(wl -i $ifname ampdu_density)
echo -n ampdu_mpdu=; echo $(wl -i $ifname ampdu_mpdu)
echo -n rx_amsdu_in_ampdu=; echo $(wl -i $ifname rx_amsdu_in_ampdu)
echo -n chanim_stats=; echo $(wl -i $ifname chanim_stats)
echo -n frameburst=; echo $(wl -i $ifname frameburst)
echo -n frameburst_override=; echo $(wl -i $ifname frameburst_override)
echo -n obss_coex=; echo $(wl -i $ifname obss_coex)
echo -n sgi_rx=; echo $(wl -i $ifname  sgi_rx)
echo -n sgi_tx=; echo $(wl -i $ifname  sgi_tx)
echo -n taf enable=; echo $(wl -i $ifname taf enable)
echo -n txbf=; echo $(wl -i $ifname txbf)
echo -n txbf_bfe_cap=; echo $(wl -i $ifname txbf_bfe_cap)
echo -n txbf_bfr_cap=; echo $(wl -i $ifname txbf_bfr_cap)
echo -n txbf_imp=; echo $(wl -i $ifname txbf_imp)
echo -n txchain=; echo $(wl -i $ifname txchain)
echo -n txchain_pwr_offset=; echo $(wl -i $ifname txchain_pwr_offset)
echo -n vlan_mode=; echo $(wl -i $ifname vlan_mode)
echo -n vndr_ie=; echo $(wl -i $ifname vndr_ie)
echo -n wdstimeout=; echo $(wl -i $ifname wdstimeout)
echo -n wet_tunnel=; echo $(wl -i $ifname wet_tunnel)
echo -n wme=; echo $(wl -i $ifname wme)
echo -n wme_ac_ap=; echo "$(wl -i $ifname wme_ac_ap)"
echo -n wme_ac ap=; echo $(wl -i $ifname wme_ac ap)
echo -n wme_ac_sta=; echo $(wl -i $ifname wme_ac_sta)
echo -n wme_ac sta=; echo "$(wl -i $ifname wme_ac sta)"
echo -n wme_apsd=; echo $(wl -i $ifname wme_apsd)
echo -n wme_bss_disable=; echo $(wl -i $ifname wme_bss_disable)
echo -n wme_dp=; echo $(wl -i $ifname wme_dp)
echo -n wme_noack=; echo $(wl -i $ifname wme_noack)
echo -n wme_tx_params=; echo $(wl -i $ifname wme_tx_params)

if [[ $mode == "nic" ]]; then
echo -n wmf_bss_enable=; echo $(wl -i $ifname wmf_bss_enable)
echo -n wmf_psta_disable=; echo $(wl -i $ifname wmf_psta_disable)
fi

echo -n wnm_url=; echo $(wl -i $ifname wnm_url)
echo -n wpa_auth=; echo $(wl -i $ifname wpa_auth)
echo -n wpa_cap=; echo $(wl -i $ifname wpa_cap)
echo -n wsec=; echo $(wl -i $ifname wsec)
echo -n wsec_restrict=; echo $(wl -i $ifname wsec_restrict)

if [[ $band == "5" ]]; then
echo -n 5g_mrate=; echo $(wl -i $ifname 5g_mrate)
echo -n 5g_rate=; echo $(wl -i $ifname 5g_rate)
echo -n a_rate=; echo $(wl -i $ifname a_rate)
fi

if [[ $band == "2" ]]; then
echo -n 2g_mrate=; echo $(wl -i $ifname 2g_mrate)
echo -n 2g_rate=; echo $(wl -i $ifname 2g_rate)
echo -n bg_rate=; echo $(wl -i $ifname bg_rate)
echo -n bg_mrate=; echo $(wl -i $ifname bg_mrate)
echo -n gmode=; echo $(wl -i $ifname gmode)
echo -n gmode_protection=; echo $(wl -i $ifname gmode_protection)
echo -n gmode_protection_control=; echo $(wl -i $ifname gmode_protection_control)
echo -n gmode_protection_override=; echo $(wl -i $ifname gmode_protection_override)
fi

echo -n PM=; echo $(wl -i $ifname PM)
echo -n promisc=; echo $(wl -i $ifname promisc)
echo -n cwmin=; echo $(wl -i $ifname cwmin)
echo -n cwmax=; echo $(wl -i $ifname cwmax)
echo -n m_rate=; echo $(wl -i $ifname m_rate)
echo -n infra=; echo $(wl -i $ifname infra)
echo -n bssid=; echo $(wl -i $ifname bssid)
echo -n bssmax=; echo $(wl -i $ifname bssmax)
echo -n channel=; echo $(wl -i $ifname channel)
echo -n chanspecs=; echo $(wl -i $ifname chanspecs)
echo -n txpwr=; echo $(wl -i $ifname txpwr)
echo -n txpwr1=; echo $(wl -i $ifname txpwr1)
echo -n eap=; echo $(wl -i $ifname eap)
echo -n cur_etheraddr=; echo $(wl -i $ifname cur_etheraddr)
echo -n perm_etheraddr=; echo $(wl -i $ifname perm_etheraddr)
echo -n rateset=; echo $(wl -i $ifname rateset)
echo -n txbf_rateset=; echo $(wl -i $ifname txbf_rateset)
echo -n roam_delta=; echo $(wl -i $ifname roam_delta)
echo -n prb_resp_timeout=; echo $(wl -i $ifname prb_resp_timeout)
echo -n shortslot=; echo $(wl -i $ifname shortslot)
echo -n protection_control=; echo $(wl -i $ifname protection_control)
echo -n legacy_erp=; echo $(wl -i $ifname legacy_erp)
echo -n pwr_percent=; echo $(wl -i $ifname pwr_percent)
echo -n arpoe=; echo $(wl -i $ifname arpoe)
echo -n wet=; echo $(wl -i $ifname wet)
echo -n bi=; echo $(wl -i $ifname bi)
echo -n lifetime vi=; echo $(wl -i $ifname lifetime vi)
echo -n lifetime vo=; echo $(wl -i $ifname lifetime vo)
echo -n lifetime be=; echo $(wl -i $ifname lifetime be)
echo -n lifetime bk=; echo $(wl -i $ifname lifetime bk)
echo -n list_ie=; echo $(wl -i $ifname list_ie)
echo -n pmkid_info=; echo $(wl -i $ifname pmkid_info)
echo -n phy_antsel=; echo $(wl -i $ifname phy_antsel)
echo -n intfer_params=; echo $(wl -i $ifname intfer_params)
echo -n mempool=; echo $(wl -i $ifname mempool)
echo -n pm2_sleep_ret_ext=; echo $(wl -i $ifname pm2_sleep_ret_ext)
echo -n monitor_promisc_level=; echo $(wl -i $ifname monitor_promisc_level)
echo -n desired_bssid=; echo $(wl -i $ifname desired_bssid)
echo -n ht_features=; echo $(wl -i $ifname ht_features)
echo -n atim=; echo $(wl -i $ifname atim)
echo -n phy_afeoverride=; echo $(wl -i $ifname phy_afeoverride)
echo -n phy_rxiqest=; echo $(wl -i $ifname phy_rxiqest)
echo -n phy_lesi=; echo $(wl -i $ifname phy_lesi)
echo -n smth_enable=; echo $(wl -i $ifname smth_enable)
echo -n povars=; echo $(wl -i $ifname povars)
echo -n radarargs=; echo $(wl -i $ifname radarargs)
echo -n pkt_filter_list 0=; echo $(wl -i $ifname pkt_filter_list 0)
echo -n pkt_filter_list 1=; echo $(wl -i $ifname pkt_filter_list 1)
echo -n pkt_filter_list=; echo $(wl -i $ifname pkt_filter_list)
echo -n txcore=; echo $(wl -i $ifname txcore)
echo -n txcore_override=; echo $(wl -i $ifname txcore_override)
echo -n mimo_ss_stf=; echo $(wl -i $ifname mimo_ss_stf)
echo -n spatial_policy=; echo $(wl -i $ifname spatial_policy)
echo -n tpc_mode=; echo $(wl -i $ifname tpc_mode)
echo -n ap=; echo $(wl -i $ifname ap)
echo -n scb_timeout=; echo $(wl -i $ifname scb_timeout)
echo -n beacon_info=; echo $(wl -i $ifname beacon_info)
echo -n probe_resp_info=; echo $(wl -i $ifname probe_resp_info)
echo -n cur_mcsset=; echo $(wl -i $ifname cur_mcsset)
echo -n mimo_ps=; echo $(wl -i $ifname mimo_ps)
echo -n wepstatus=; echo $(wl -i $ifname wepstatus)
echo -n primary_key=; echo $(wl -i $ifname primary_key)
echo -n passive=; echo $(wl -i $ifname passive)
echo -n scan_channel_time=; echo $(wl -i $ifname scan_channel_time)
echo -n scan_unassoc_time=; echo $(wl -i $ifname scan_unassoc_time)
echo -n scan_home_time=; echo $(wl -i $ifname scan_home_time)
echo -n scan_passive_time=; echo $(wl -i $ifname scan_passive_time)
echo -n scan_nprobes=; echo $(wl -i $ifname scan_nprobes)
echo -n scansuppress=; echo $(wl -i $ifname scansuppress)
echo -n obss_scan_params=; echo $(wl -i $ifname obss_scan_params)
echo -n scb_alloc=; echo $(wl -i $ifname scb_alloc)
echo -n scb_alloc_class=; echo $(wl -i $ifname scb_alloc_class)
echo -n scb_alloc_max_dscb=; echo $(wl -i $ifname scb_alloc_max_dscb)
echo -n mrate=; echo $(wl -i $ifname mrate)
echo -n curppr=; echo $(wl -i $ifname curppr)
echo -n suprates=; echo $(wl -i $ifname suprates)
echo -n rrm=; echo $(wl -i $ifname rrm)
echo -n msched=; echo $(wl -i $ifname msched)
echo -n msched rucfg=; echo $(wl -i $ifname msched rucfg)
echo -n msched maxn=; echo $(wl -i $ifname msched maxn)
echo -n umsched=; echo $(wl -i $ifname umsched)
echo -n dfs_postism=; echo $(wl -i $ifname dfs_postism)
echo -n dfs_pretism=; echo $(wl -i $ifname dfs_preism)
echo -n ldpc_cap=; echo $(wl -i $ifname ldpc_cap)
echo -n ldpc_tx=; echo $(wl -i $ifname ldpc_tx)
echo -n mfp=; echo $(wl -i $ifname mfp)
echo -n pkteng_cmd=; echo $(wl -i $ifname pkteng_cmd)
echo -n twt_prestop=; echo $(wl -i $ifname twt_prestop)
echo -n twt_prestrt=; echo $(wl -i $ifname twt_prestrt)
echo -n txbf_mutimer=; echo $(wl -i $ifname txbf_mutimer)

echo ""
echo ""
echo "---------------------"
echo "$ifname WL nvram_dump"
echo "---------------------"
wl -i $ifname nvram_dump

if [[ $mode == "dhd" ]]; then
echo ""
echo ""
echo "-----------------------------"
echo "$ifname DHD config parameters"
echo "-----------------------------"
echo -n alignctl=; echo $(dhd -i $ifname alignctl)
echo -n ap_isolate=; echo $(dhd -i $ifname ap_isolate)
echo -n aspm=; echo $(dhd -i $ifname aspm)
echo -n assert_type=; echo $(dhd -i $ifname assert_type)
echo -n bcmerrorstr=; echo $(dhd -i $ifname bcmerrorstr)
echo -n block_ping=; echo $(dhd -i $ifname block_ping)
echo -n cons=; echo $(dhd -i $ifname cons)
echo -n db1_for_mb=; echo $(dhd -i $ifname db1_for_mb)
echo -n dconpoll=; echo $(dhd -i $ifname dconpoll)
# Commented out dev_def
#echo dev_def=$(dhd -i $ifname dev_def)
echo -n devcap=; echo $(dhd -i $ifname devcap)
echo -n devreset=; echo $(dhd -i $ifname devreset)
echo -n devsleep=; echo $(dhd -i $ifname devsleep)
echo -n dhcp_unicast=; echo $(dhd -i $ifname dhcp_unicast)
# Commented out dldn, command downloads file to specified dongle ram address 0
#echo dldn=$(dhd -i $ifname dldn)
echo -n dma_ring_indices=; echo $(dhd -i $ifname dma_ring_indices)
echo -n dngl_isolation=; echo $(dhd -i $ifname dngl_isolation)
# Commented out download, command downloads file to specified dongle ram address and start CPU
#echo download=$(dhd -i $ifname download)
# Commented out dump
#echo dump=$(dhd -i $ifname dump)
# Commented out, dump_mac shows messages, "External imprecise Data abort at addr=<>"
#echo dump_mac=$(dhd -i $ifname dump_mac)
# Commented out, outputs too many lines
#echo dump_ringupdblk=$(dhd -i $ifname dump_ringupdblk)
echo -n flow_prio_map=; echo $(dhd -i $ifname flow_prio_map)
echo -n forceeven=; echo $(dhd -i $ifname forceeven)
echo -n fw_hang_report=; echo $(dhd -i $ifname fw_hang_report)
echo -n grat_arp=; echo $(dhd -i $ifname grat_arp)
echo -n host_reorder_flows=; echo $(dhd -i $ifname host_reorder_flows)
echo -n idleclock=; echo $(dhd -i $ifname idleclock)
echo -n idletime=; echo $(dhd -i $ifname idletime)
# Commented out intr, command uses interrupts on the bus
#echo intr=$(dhd -i $ifname intr)
echo -n ioctl_timeout=; echo $(dhd -i $ifname ioctl_timeout)
echo -n kso=; echo $(dhd -i $ifname kso)
echo -n lmtest=; echo $(dhd -i $ifname lmtest)
# Commented out logcal, logcal <n>  -- log around an osl_delay of <n> usecs
#echo logcal=$(dhd -i $ifname logcal)
echo -n logdump=; echo $(dhd -i $ifname logdump)
# Commented out logstamp, logstamp [<n1>] [<n2>] adds message to the log
#echo logstamp=$(dhd -i $ifname logstamp)
echo -n ltrsleep_on_unload=; echo $(dhd -i $ifname ltrsleep_on_unload)
echo -n mcast_regen_bss_enable=; echo $(dhd -i $ifname mcast_regen_bss_enable)
echo -n msi_sim=; echo $(dhd -i $ifname msi_sim)
echo -n oob_bt_reg_on=; echo $(dhd -i $ifname oob_bt_reg_on)
# Commented out pmac, command gets mac obj values such as of SHM and IHR
#echo pmac=$(dhd -i $ifname pmac)
echo -n pmodule_ignore=; echo $(dhd -i $ifname pmodule_ignore)
echo -n pollrate=; echo $(dhd -i $ifname pollrate)
echo -n proptx=; echo $(dhd -i $ifname proptx)
echo -n proptx_opt=; echo $(dhd -i $ifname proptx_opt)
echo -n proxy_arp=; echo $(dhd -i $ifname proxy_arp)
echo -n ptxmode=; echo $(dhd -i $ifname ptxmode)
echo -n ptxstatus_ignore=; echo $(dhd -i $ifname ptxstatus_ignore)
echo -n ramsize=; echo $(dhd -i $ifname ramsize)
echo -n ramstart=; echo $(dhd -i $ifname ramstart)
echo -n readahead=; echo $(dhd -i $ifname readahead)
echo -n rxbound=; echo $(dhd -i $ifname rxbound)
echo -n rxpkt_chk=; echo $(dhd -i $ifname rxpkt_chk)

echo -n sleep=; echo $(dhd -i $ifname sleep)
echo -n sleep_allowed=; echo $(dhd -i $ifname sleep_allowed)
echo -n srdump=; echo $(dhd -i $ifname srdump)
echo -n tcpack_suppress=; echo $(dhd -i $ifname tcpack_suppress)
echo -n tuning_mode=; echo $(dhd -i $ifname tuning_mode)
echo -n txbound=; echo $(dhd -i $ifname txbound)
echo -n txglommode=; echo $(dhd -i $ifname txglommode)
echo -n txglomsize=; echo $(dhd -i $ifname txglomsize)
echo -n txinrx_thres=; echo $(dhd -i $ifname txinrx_thres)
echo -n txminmax=; echo $(dhd -i $ifname txminmax)
echo -n txp_thresh=; echo $(dhd -i $ifname txp_thresh)
# Commented out vars, command overrides SPROM vars with <file> (before download)
#echo vars=$(dhd -i $ifname vars)
echo -n version=; echo $(dhd -i $ifname version)
echo -n wdtick=; echo $(dhd -i $ifname wdtick)
echo -n wmf_bss_enable=; echo $(dhd -i $ifname wmf_bss_enable)
echo -n wmf_mcast_data_sendup=; echo $(dhd -i $ifname wmf_mcast_data_sendup)
echo -n wmf_psta_disable=; echo $(dhd -i $ifname wmf_psta_disable)
echo -n wmf_ucast_igmp=; echo $(dhd -i $ifname wmf_ucast_igmp)
echo -n wmf_ucast_upnp=; echo $(dhd -i $ifname wmf_ucast_upnp)
echo -n wowl_wakeind=; echo $(dhd -i $ifname wowl_wakeind)
echo -n op_mode=; echo $(dhd -i $ifname op_mode)
echo -n pciecfgreg offset 0xb4=; echo $(dhd -i $ifname pciecfgreg 0xb4)
fi

echo ""
echo ""
echo "-----------------------"
echo "NVRAM config parameters"
echo "-----------------------"
nvram show
# restore msglevel
wl -i $ifname msglevel $WLMSGLVL
if [[ $mode == "dhd" ]]; then
  dhd -i $ifname msglevel $DHDMSGLVL
fi

# Done
