# cat /sbin/bringup.sh
#!/bin/sh
RED='\033[0;31m'
NC='\033[0m'
echo -e "----- ${RED}EasyMesh SCRIPT${NC} -----"
rm -rf /tmp/wapp_ctrl
killall -9 mapd
killall -9 wapp
killall -9 p1905_managerd
killall -15 bs20
echo -e "----- ${RED}killed all apps ${NC} -----"

#GET EasyMesh settings from NVRAM
map_enable=$(nvram_get 2860 MapEnable)
map_dev_role=$(nvram_get 2860 DeviceRole)
map_ap_rssi_th=$(nvram_get 2860 APSteerRssiTh)
map_channel_planning=$(nvram_get 2860 ChPlanningEnable)
map_ChPlanningIdleByteCount=$(nvram_get 2860 ChPlanningIdleByteCount)
map_ChPlanningIdleTime=$(nvram_get 2860 ChPlanningIdleTime)
map_ChPlanningUserPreferredChannel5G=$(nvram_get 2860 ChPlanningUserPreferredChannel5G)
map_ChPlanningUserPreferredChannel5GH=$(nvram_get 2860 ChPlanningUserPreferredChannel5GH)
map_ChPlanningUserPreferredChannel2G=$(nvram_get 2860 ChPlanningUserPreferredChannel2G)
map_bh_priority_2g=$(nvram_get 2860 BhPriority2G)
map_bh_priority_5gl=$(nvram_get 2860 BhPriority5GL)
map_bh_priority_5gh=$(nvram_get 2860 BhPriority5GH)
map_steer_enable=$(nvram_get 2860 SteerEnable)
map_CUOverloadTh_2G=$(nvram_get 2860 CUOverloadTh_2G)
map_CUOverloadTh_5G_L=$(nvram_get 2860 CUOverloadTh_5G_L)
map_CUOverloadTh_5G_H=$(nvram_get 2860 CUOverloadTh_5G_H)
map_bss_config_priority=$(nvram_get 2860 bss_config_priority)
map_turnkey=$(nvram_get 2860 MAP_Turnkey)
bs_enable=$(nvram_get 2860 BSEnable)
rssi_2g_thresh=$(nvram_get 2860 rssi_thresh_2g)
rssi_5g_thresh=$(nvram_get 2860 rssi_thresh_5g)
ra_band=$(nvram_get 2860 radio_band)
ch_pl_init_timeout=$(nvram_get 2860 ChPlanInitTimeout)
mapd_dhcpctl=$(nvram_get 2860 DhcpCtl)
NetworkOptimizationEnabled=$(nvram_get 2860 NetworkOptimizationEnabled)
NtwrkOptBootupWaitTime=$(nvram_get 2860 NtwrkOptBootupWaitTime)
NtwrkOptConnectWaitTime=$(nvram_get 2860 NtwrkOptConnectWaitTime)
NtwrkOptDisconnectWaitTime=$(nvram_get 2860 NtwrkOptDisconnectWaitTime)
NtwrkOptPeriodicity=$(nvram_get 2860 NtwrkOptPeriodicity)
NetworkOptimizationScoreMargin=$(nvram_get 2860 NetworkOptimizationScoreMargin)
AutoBHSwitch=$(nvram_get 2860 AutoBHSwitch)
band_switch_time=$(nvram_get 2860 BandSwitchTime)
Third_Party_Connection=$(nvram_get 2860 ThirdPartyConnection)
Avoid_Scan_During_Cac=$(nvram_get 2860 AvoidScanDuringCac)
ulimit -c unlimited
check_default="$1"
default_map_dev_role=2
default_map_ap_rssi_th=-54
default_map_steer_enable=1
default_map_CUOverloadTh_2G=70
default_map_CUOverloadTh_5G_L=80
default_map_CUOverloadTh_5G_H=80
default_map_bh_priority_2g=3
default_map_bh_priority_5gl=1
default_map_bh_priority_5gh=1
default_map_channel_planning=1
default_map_dhcpctl=0
default_rssi_threshold_2g=-75
default_rssi_threshold_5g=-75
default_ra_band="24G;5G;5G;"
default_ch_pl_init_timeout=120
default_ntwrk_opt_bootup_wait_time=45
default_ntwrk_opt_connect_wait_time=45
default_ntwrk_opt_disconnect_wait_time=45
default_ntwrk_opt_periodicity=3600
default_ntwrk_opt_score_margin=100
default_auto_bh_switch=1
default_ThirdPartyConnection=0
default_AvoidScanDuringCac=0
default_bss_config_priority="ra0;ra1;ra2;ra3;rax0;rax1;rax2;rax3;rai0;rai1;rai2;rai3;apcli0;apclix0;apclii0"
if [ -z "$map_dev_role"  -o  "$check_default" = "default" ];then 
	map_dev_role="$default_map_dev_role"
	nvram_set 2860 DeviceRole "$map_dev_role"
fi
if [ -z "$map_ap_rssi_th"  -o "$check_default" = "default" ];then 
	map_ap_rssi_th="$default_map_ap_rssi_th"
	nvram_set 2860 APSteerRssiTh "$map_ap_rssi_th"
fi
if [ -z "$map_steer_enable"  -o  "$check_default" = "default" ];then 
	map_steer_enable="$default_map_steer_enable"
	nvram_set 2860 SteerEnable "$map_steer_enable"
fi
if [ -z "$map_CUOverloadTh_2G"  -o  "$check_default" = "default" ];then 
	map_CUOverloadTh_2G="$default_map_CUOverloadTh_2G"
	nvram_set 2860 CUOverloadTh_2G "$map_CUOverloadTh_2G"
fi
if [ -z "$map_CUOverloadTh_5G_L"  -o "$check_default" = "default" ];then 
	map_CUOverloadTh_5G_L="$default_map_CUOverloadTh_5G_L"
	nvram_set 2860 CUOverloadTh_5G_L "$map_CUOverloadTh_5G_L"
fi
if [ -z "$map_CUOverloadTh_5G_H"  -o  "$check_default" = "default" ];then 
	map_CUOverloadTh_5G_H="$default_map_CUOverloadTh_5G_H"
	nvram_set 2860 CUOverloadTh_5G_H "$map_CUOverloadTh_5G_H"
fi
if [ -z "$map_bh_priority_2g"  -o "$check_default" = "default" ];then 
	map_bh_priority_2g="$default_map_bh_priority_2g"
	nvram_set 2860 BhPriority2G "$map_bh_priority_2g"
fi
if [ -z "$map_bh_priority_5gl"  -o  "$check_default" = "default" ];then 
	map_bh_priority_5gl="$default_map_bh_priority_5gl"
	nvram_set 2860 BhPriority5GL "$map_bh_priority_5gl"
fi
if [ -z "$map_bh_priority_5gh" -o "$check_default" = "default" ];then 
	map_bh_priority_5gh="$default_map_bh_priority_5gh"
	nvram_set 2860 BhPriority5GH "$map_bh_priority_5gh"
fi
if [ -z "$map_channel_planning"  -o "$check_default" = "default" ];then 
	map_channel_planning="$default_map_channel_planning"
	nvram_set 2860 ChPlanningEnable "$map_channel_planning"
fi
if [ -z "$rssi_2g_thresh"  -o "$check_default" = "default" ];then
        rssi_2g_thresh="$default_rssi_threshold_2g"
		nvram_set 2860 rssi_thresh_2g "$rssi_2g_thresh"
fi
if [ -z "$rssi_5g_thresh"  -o "$check_default" = "default" ];then
        rssi_5g_thresh="$default_rssi_threshold_5g"
		nvram_set 2860 rssi_thresh_5g "$rssi_5g_thresh"
fi
if [ -z "$ra_band"  -o "$check_default" = "default" ];then
        ra_band="$default_ra_band"
		nvram_set 2860 radio_band "$ra_band"
fi
if [ -z "$ch_pl_init_timeout"  -o "$check_default" = "default" ];then
        ch_pl_init_timeout="$default_ch_pl_init_timeout"
		nvram_set 2860 ChPlanInitTimeout "$ch_pl_init_timeout"
fi
if [ -z "$NtwrkOptBootupWaitTime"  -o "$check_default" = "default" ];then
        NtwrkOptBootupWaitTime="$default_ntwrk_opt_bootup_wait_time"
		nvram_set 2860 NtwrkOptBootupWaitTime "$NtwrkOptBootupWaitTime"
fi
if [ -z "$NtwrkOptConnectWaitTime"  -o "$check_default" = "default" ];then
        NtwrkOptConnectWaitTime="$default_ntwrk_opt_connect_wait_time"
		nvram_set 2860 NtwrkOptConnectWaitTime "$NtwrkOptConnectWaitTime"
fi
if [ -z "$NtwrkOptDisconnectWaitTime"  -o "$check_default" = "default" ];then
        NtwrkOptDisconnectWaitTime="$default_ntwrk_opt_disconnect_wait_time"
		nvram_set 2860 NtwrkOptDisconnectWaitTime "$NtwrkOptDisconnectWaitTime"
fi
if [ -z "$NtwrkOptPeriodicity"  -o "$check_default" = "default" ];then
        NtwrkOptPeriodicity="$default_ntwrk_opt_periodicity"
		nvram_set 2860 NtwrkOptPeriodicity "$NtwrkOptPeriodicity"
fi
if [ -z "$NetworkOptimizationScoreMargin"  -o "$check_default" = "default" ];then
        NetworkOptimizationScoreMargin="$default_ntwrk_opt_score_margin"
		nvram_set 2860 NetworkOptimizationScoreMargin "$NetworkOptimizationScoreMargin"
fi
if [ -z "$mapd_dhcpctl"  -o "$check_default" = "default" ];then
        mapd_dhcpctl="$default_map_dhcpctl"
		nvram_set 2860 DhcpCtl "$mapd_dhcpctl"
fi
if [ -z "$AutoBHSwitch"  -o "$check_default" = "default" ];then
       AutoBHSwitch="$default_auto_bh_switch"
	   nvram_set 2860 AutoBHSwitch "$AutoBHSwitch"
fi
if [ -z "$Third_Party_Connection"  -o "$check_default" = "default" ];then
       Third_Party_Connection="$default_ThirdPartyConnection"
	   nvram_set 2860 ThirdPartyConnection "$Third_Party_Connection"
fi
if [ -z "$Avoid_Scan_During_Cac"  -o "$check_default" = "default" ];then
       Avoid_Scan_During_Cac="$default_AvoidScanDuringCac"
	   nvram_set 2860 AvoidScanDuringCac "$Avoid_Scan_During_Cac"
fi

if [ -z "$map_bss_config_priority"  -o "$check_default" = "default" ];then
       map_bss_config_priority=$default_bss_config_priority
       nvram_set 2860 bss_config_priority "$map_bss_config_priority"
fi
#Update /etc_ro/map_cfg.txt file with AL-MAC
ctrlr_al_mac=$(cat /sys/class/net/br0/address)
agent_al_mac=$(cat /sys/class/net/br0/address)
#enable QuickChannelChange feature:
nvram_set 2860 MAP_QuickChChange 1
#Enable 1905 packets on VO category by default
iwpriv ra0 set fwlog=1:204
iwpriv rai0 set fwlog=1:204

echo "# Controller's ALID
map_controller_alid=$ctrlr_al_mac

# Has MAP agent on this device
map_agent=1

# This device is a MAP root
map_root=0

# Agent's ALID
map_agent_alid=$agent_al_mac

# Default Back-haul Type
bh_type=wifi

#Config Band setting of each Radio
radio_band=$ra_band

#bridge interface
br_inf=br0

#lan interface
lan=eth2

bss_config_priority=$map_bss_config_priority
" > /etc_ro/map_cfg.txt

#Generate /etc/mapd_cfg.txt file
echo "#lan interface for device
lan_interface=eth2
#wan interface for device
wan_interface=eth3
MapEnable=$map_enable
DeviceRole=$map_dev_role
APSteerRssiTh=$map_ap_rssi_th
ChPlanningEnable=$map_channel_planning
BhPriority2G=$map_bh_priority_2g
BhPriority5GL=$map_bh_priority_5gl
BhPriority5GH=$map_bh_priority_5gh
ChPlanningIdleByteCount=$map_ChPlanningIdleByteCount
ChPlanningIdleTime=$map_ChPlanningIdleTime
SteerEnable=$map_steer_enable
CUOverloadTh_2G=$map_CUOverloadTh_2G
CUOverloadTh_5G_L=$map_CUOverloadTh_5G_L
CUOverloadTh_5G_H=$map_CUOverloadTh_5G_H
ChPlanningUserPreferredChannel5G=$map_ChPlanningUserPreferredChannel5G
ChPlanningUserPreferredChannel5GH=$map_ChPlanningUserPreferredChannel5GH
ChPlanningUserPreferredChannel2G=$map_ChPlanningUserPreferredChannel2G
ScanThreshold2g=$rssi_2g_thresh
ScanThreshold5g=$rssi_5g_thresh
ChPlanningInitTimeout=$ch_pl_init_timeout
NetworkOptimizationEnabled=$NetworkOptimizationEnabled
NtwrkOptBootupWaitTime=$NtwrkOptBootupWaitTime
NtwrkOptConnectWaitTime=$NtwrkOptConnectWaitTime
NtwrkOptDisconnectWaitTime=$NtwrkOptDisconnectWaitTime
NtwrkOptPeriodicity=$NtwrkOptPeriodicity
NetworkOptimizationScoreMargin=$NetworkOptimizationScoreMargin
DhcpCtl=$mapd_dhcpctl
AutoBHSwitching=$AutoBHSwitch
BandSwitchTime=$band_switch_time
ThirdPartyConnection=$Third_Party_Connection
AvoidScanDuringCac=$Avoid_Scan_During_Cac
" > /etc/mapd_cfg.txt

#Generate /etc/mapd_strng.conf file
rssi=$((map_ap_rssi_th + 94))
echo "LowRSSIAPSteerEdge_RE=$rssi
CUOverloadTh_2G=$map_CUOverloadTh_2G
CUOverloadTh_5G_L=$map_CUOverloadTh_5G_L
CUOverloadTh_5G_H=$map_CUOverloadTh_5G_H
idle_count_th=10
" > /etc/mapd_strng.conf

# To make wts_bss_info_config config parameters persistent after reboot
get_WTS_OK=$(nvram_get 2860 WTS_CONFIG_OK)
if [  ! -z "$get_WTS_OK" ];then
     get_WTS=$(nvram_get cert WTS_BSS_INFO_CONFIG)
	if [  ! -z "$get_WTS" ];then
	echo "easymesh load nvram cert wts"
	echo "$get_WTS" > /etc/wts_bss_info_config
	fi   
else
	cp /etc_ro/wts_bss_info_config /etc/
	RenewProfile=$(cat /etc_ro/wts_bss_info_config)
	nvram_set cert WTS_BSS_INFO_CONFIG "$RenewProfile"
	nvram_set 2860 WTS_CONFIG_OK 1
    echo "easymesh load default wts file"
fi



cp /etc_ro/map_cfg.txt /etc/
cp /etc_ro/secondary_config.txt /etc/
echo -e "----- ${RED}copied txt files${NC} -----"

ModifySwitchReg()
{
	# To make switch related MC register setting for eth on platform mt7621
	# support lsdk
	if [ -f /sbin/config.sh ]; then     
	  echo "easymesh in lsdk ver"
	  . /sbin/config.sh
	fi
	
	# support openwrt 
	if [ -f /etc/kernel.config ]; then                         
	  echo "easymesh in openwrt ver"
	  . /etc/kernel.config
	fi
	
	if [ "$CONFIG_RALINK_MT7621" = "y" -o "$CONFIG_MT7621_ASIC" = "y" ]; then
	  echo "easymesh board name is 7621"
	  switch reg w 10 ffffffe0
	  switch reg w 34 8160816
	fi
}

ModifySwitchReg
chmod 777 libmapd_interface_client.so
chmod 777 wapp mapd_cli
cp libmapd_interface_client.so /lib/
cp mapd_cli  /sbin/
cp wapp /sbin/

StartStandAloneBS()
{
	chmod 777 bs20
	cp bs20 /sbin/
	sleep 1

	echo "WAPP starting..."
	wapp > /dev/null
	sleep 3
	echo "BS2.0 Daemon starting..."
	bs20 &
	sleep 3
	echo "Stand Alone BS2.0 is ready"
}
ApCliIntfUp()
{
	ifconfig apcli0 up;
	ifconfig apclix0 up;
	ifconfig apclii0 up;
	ifconfig apcli1 up;
	ifconfig wlan-apcli0 up;
	brctl addif br0 apcli0;
	brctl addif br0 apclix0;
	brctl addif br0 apclii0;
	brctl addif br0 apcli1;
	brctl addif br0 wlan-apcli0;
	iwpriv apcli0 set ApCliEnable=0;
	iwpriv apclix0 set ApCliEnable=0;
	iwpriv apclii0 set ApCliEnable=0;
	iwpriv apcli1 set ApCliEnable=0;
	iwpriv wlan-apcli0 set ApCliEnable=0;
	sleep 1
	if [ "$check_default" = "default" ];then
		echo "old profile set invalid"
		nvram_set 2860 BhProfile0Valid 0
	fi
}
StartEasyMesh()
{
	chmod 777 mapd p1905_managerd
	cp mapd /sbin/
	cp p1905_managerd /sbin/

	if [ "$map_turnkey" = "0" ];then
		ifconfig apcli0 up;
		sleep 1
		ifconfig apclix0 up;
		sleep 1
		ifconfig apclii0 up;
		sleep 1
		ifconfig apcli1 up;
		sleep 1
		brctl addif br0 apcli0;
		sleep 1
		brctl addif br0 apclix0;
		sleep 1
		brctl addif br0 apclii0;
		sleep 1
		brctl addif br0 apcli1;
		sleep 1
		iwpriv apcli0 set ApCliEnable=0;
		sleep 1
		iwpriv apclix0 set ApCliEnable=0;
		sleep 1
		iwpriv apclii0 set ApCliEnable=0;
		sleep 1
		iwpriv apcli1 set ApCliEnable=0;
		sleep 1
	fi
	#insmod /lib/modules/3.10.108\+/extra/mapfilter.ko
	modprobe mapfilter
	sleep 1

	if [ "$map_dev_role" = "1" ];then
		#Controller
		ApCliIntfUp
		echo "WAPP starting..."
		wapp > /dev/null
		sleep 3
		echo "1905 starting..."
		p1905_managerd -r0 -f /etc/map_cfg.txt -F "/etc/wts_bss_info_config" > /dev/console &
		sleep 3
		echo "MAP Daemon starting..."
		mapd -G "/etc/wts_bss_info_config" -I "/etc/mapd_cfg.txt" -O "/etc/mapd_strng.conf" > /dev/console &
		ModifySwitchReg
		sleep 3
		iwpriv ra0 set DisConnectAllSta=
		iwpriv ra1 set DisConnectAllSta=
		iwpriv ra2 set DisConnectAllSta=
		iwpriv ra3 set DisConnectAllSta=
		iwpriv rax0 set DisConnectAllSta=
		iwpriv rax1 set DisConnectAllSta=
		iwpriv rax2 set DisConnectAllSta=
		iwpriv rax3 set DisConnectAllSta=
		iwpriv rai0 set DisConnectAllSta=
		iwpriv rai1 set DisConnectAllSta=
		iwpriv rai2 set DisConnectAllSta=
		iwpriv rai3 set DisConnectAllSta=
		if [ "$mapd_dhcpctl" = "1" ];then
			echo "Controller Enable DHCP Server!"
			if [ -z "$(nvram_get 2860 lan_ipaddr)" ];then
				echo "no IP present , reload network ip to 10.10.10.254"
				nvram_set 2860 lan_ipaddr 10.10.10.254; nvram_set 2860 dhcpStart 10.10.10.100; nvram_set 2860 dhcpEnd 10.10.10.200; nvram_set 2860 dhcpGateway 10.10.10.254; ifconfig br0 10.10.10.254 up; udhcpd /etc/udhcpd.conf &
				wifi reload
			else
				ifconfig br0 `nvram_get 2860 lan_ipaddr` up; udhcpd /etc/udhcpd.conf &
			fi
		fi
		echo -e "----- ${RED}MAP DEVICE STARTED${NC} -----"
		sleep 1
	elif [ "$map_dev_role" = "2" -o "$map_dev_role" = "0" ];then
		#Agent or Auto
		ApCliIntfUp
		echo "WAPP starting..."
		wapp > /dev/null
		sleep 3
		echo "1905 starting..."
		p1905_managerd -r1 -f "/etc/map_cfg.txt" -F "/etc/wts_bss_info_config" > /dev/console &
		sleep 3
		echo "MAP_Daemon starting..."
		mapd -G "/etc/wts_bss_info_config" -I "/etc/mapd_cfg.txt" -O "/etc/mapd_strng.conf" > /dev/console &
		ModifySwitchReg
		sleep 3
		echo -e "----- ${RED}MAP DEVICE STARTED${NC} -----"
	else
		echo "Unknown EasyMesh Device Role!"
	fi
}

if [ "$map_enable" = "1" ];then
	echo "EasyMesh is enabled!"
	StartEasyMesh
else
	echo "EasyMesh is disabled!"
	if [ "$bs_enable" = "1" -a "$map_turnkey" = "0" ];then
		echo "Stand Alone BS is enabled!"
		StartStandAloneBS
	else
		exit 1
	fi
fi
