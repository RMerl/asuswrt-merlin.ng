# argument details
# First Argument is SKU second Argument is device role 
# Device Role 1 - MCUT Device Role 2 - MAUT
# 1  --> MT7621 + MT7615D LSDK
# 2  --> MT7621 + MT7615 + MT7615  LSDK
# 3  --> MT7621 + MT7615D OPENWRT
# 4  --> MT7621 + MT7615 + MT7615  OPENWRT dual-band
# 5  --> MT7621 + MT7615D + MT7615  OPENWRT tri-band
# 6  --> MT7622 + MT7615 dual-band
# 7  --> MT7622 + MT7615D tri-band 
# 8  --> red board MT7622 + MT7615 + MT7615 tri-band  + switch RTL8367S
# 9	 --> MT7629 OPENWRT/LEDE 
# 11  --> MT7621 + MT7615D + MT7613  OPENWRT tri-band
# 12  --> green board MT7622 + MT7615 + MT7615 tri-band + switch MT7531
# 13  --> MT7621 + 7603 + MT7613 OPENWRT
# 14  --> MT7621 + MT7603 + MT7615 OPENWRT
# 15  --> MT7621 + MT7615D LEDE
# 16  --> MT7622 + MT7915D + switch MT7531
RED='\033[0;31m'
NC='\033[0m'
echo -e "----- ${RED}EasyMesh CERT SCRIPT dev=$1 Role=$2 ${NC} -----" 
dev="$1"
Role="$2"
MAP_R2="$3"
echo "1  --> MT7621 + MT7615D LSDK"
echo "2  --> MT7621 + MT7615 + MT7615  LSDK"
echo "3  --> MT7621 + MT7615D OPENWRT"
echo "4  --> MT7621 + MT7615 + MT7615  OPENWRT dual-band"
echo "5  --> MT7621 + MT7615 + MT7615  OPENWRT tri-band"
echo "6  --> MT7622 + MT7615 dual-band"
echo "7  --> MT7622 + MT7615D tri-band" 
echo "8  --> MT7622 + MT7615 + MT7615 tri-band with switch RTL8367S"
echo "9  --> MT7629 OPENWRT/LEDE"
echo "12 --> MT7622 + MT7615 + MT7615 tri-band with switch MT7531"
echo "13  --> MT7621 + MT7603 + MT7613 OPENWRT"
echo "14  --> MT7621 + MT7603 + MT7615 OPENWRT"
echo "15  --> MT7621 + MT7915D LEDE"
echo "16  --> MT7622 + MT7915D + switch MT7531"

#1) check SDK Type
echo -e "----- ${RED}EasyMesh CERT SCRIPT dev=$dev ${NC} -----" 
if [ "$dev" = "3" -o "$dev" = "4" -o "$dev" = 5 -o\
 "$dev" = "6" -o "$dev" = "7" -o "$dev" = "8" -o\
 "$dev" = "9" -o "$dev" = "11" -o "$dev" = "12" -o\
 "$dev" = "13" -o "$dev" = "14"  -o "$dev" = "15" -o "$dev" = "16" ];then
SDK_TYPE=OPENWRT
br0_mac=$(cat /sys/class/net/br-lan/address)
br0_mac_last_byte="0x${br0_mac##*:}"
br0_mac_first_five_str=${br0_mac%:*}
al_mac_last_byte=$(printf "%02X" $((br0_mac_last_byte ^ 0xAA)))
al_mac="$br0_mac_first_five_str:$al_mac_last_byte"
uci set network.lan.macaddr="$br0_mac_first_five_str:$al_mac_last_byte"
uci commit 
else
SDK_TYPE=LSDK
al_mac=$(cat /sys/class/net/br0/address)
fi

#2) check Platform
if [ "$dev" = "6" -o "$dev" = "7" -o "$dev" = "8" -o "$dev" = "12" -o "$dev" = "16" ];then
PLATFORM=MT7622
LAN_INTF=eth0
WAN_INTF=eth1
if [ "$dev" = "12" -o "$dev" = "16" ]; then
#forward 1905 multicast to cpu port not flooding
switch reg w 34 8160816
switch reg w 4 60
switch reg w 10 ffffffff

switch vlan clear

#lan port 0-3 and cpu port, vlan id 1
switch vlan set 0 1 11110010
#wan port 4-5, vlan id 2
switch vlan set 1 2 00001100

#lan port and cpu port set pvid to 1
switch vlan pvid 0 1
switch vlan pvid 1 1
switch vlan pvid 2 1
switch vlan pvid 3 1
switch vlan pvid 6 1

#wan port set pvid to 2
switch vlan pvid 4 2
switch vlan pvid 5 2

#add vlan 10,20,30 to lan port and cpu port
# P0/P1/P2/P3/P6(242 bitmap) belog to group of vid=10,P6(64 index) in VLAN 10  egress is tagged.
switch vlan vid 0 1 10 242 64 1 0 0
# P0/P1/P2/P3/P6(242 bitmap) belog to group of vid=20,P6(64 index) in VLAN 20  egress is tagged.
switch vlan vid 0 1 20 242 64 1 0 0
# P0/P1/P2/P3/P6(242 bitmap) belog to group of vid=30,P6(64 index) in VLAN 30  egress is tagged.
switch vlan vid 0 1 30 242 64 1 0 0


# reg: 2n04, n = port num
#set all port to check vlan after receiving pkt(security mode=3)
switch vlan port-mode 0 3
switch vlan port-mode 1 3
switch vlan port-mode 2 3
switch vlan port-mode 3 3
switch vlan port-mode 4 3
switch vlan port-mode 5 3
switch vlan port-mode 6 3

switch clear
fi
elif [ "$dev" = "1" -o "$dev" = "2" -o "$dev" = "3" -o "$dev" = "4" -o "$dev" = "5" -o "$dev" = "11" ]; then
PLATFORM=MT7621
LAN_INTF=eth0
WAN_INTF=eth1
elif [ "$dev" = "13" -o "$dev" = "14" ]; then
#master branch autobuild sqc image
#config for map1.0 certification
#interfaces need config to eth0/1
uci set network.lan.ifname=eth0
uci set network.wan.ifname=eth1
uci set network.wan6.ifname=eth1
uci commit
PLATFORM=MT7621
LAN_INTF=eth0
WAN_INTF=eth1
elif [ "$dev" = "9" ]; then
PLATFORM=MT7629
LAN_INTF=eth0
WAN_INTF=eth1
elif [ "$dev" = "15" ]; then
PLATFORM=MT7915
LAN_INTF=eth0
WAN_INTF=eth1
fi

#3) Check IF DBDC Mode
if [ "$dev" = "1" -o "$dev" = "3" -o "$dev" = "7" -o "$dev" = "9" -o "$dev" = "11" -o "$dev" = "15" ];then
DBDC_MODE=1
fi
if [ "$SDK_TYPE" = "OPENWRT" ];then
echo "CONFIGURING PLATFORM $PLATFORM FOR OPENWRT SDK"
else
echo "CONFIGURING PLATFORM $PLATFORM FOR LSDK"
fi


echo -e "----- ${RED}EasyMesh CERT SCRIPT SDK=$SDK_TYPE  Genric Config${NC} -----" 

if [ "$MAP_R2" = "1" ];then
nvram_set 2860 BssidNum 4
nvram_set wifi3 BssidNum 4
nvram_set rtdev BssidNum 4
nvram_set 2860 MboSupport "1;1;1;1"
nvram_set rtdev MboSupport "1;1;1;1"
nvram_set wifi3 MboSupport "1;1;1;1"
nvram_set 2860 PMFMFPC "1;1;1;1"
nvram_set rtdev PMFMFPC "1;1;1;1"
nvram_set wifi3 PMFMFPC "1;1;1;1"
nvram_set 2860 MapR2Enable 1
nvram_set rtdev MapR2Enable 1
nvram_set wifi3 MapR2Enable 1
nvram_set 2860 HT_AMSDU 0
nvram_set rtdev HT_AMSDU 0
nvram_set wifi3 HT_AMSDU 0
nvram_set 2860 HT_AutoBA 0
nvram_set rtdev HT_AutoBA 0
nvram_set wifi3 HT_AutoBA 0
nvram_set 2860 HT_BADecline 1
nvram_set rtdev HT_BADecline 1
nvram_set wifi3 HT_BADecline 1
nvram_set 2860 G_BAND_256QAM 0
nvram_set rtdev G_BAND_256QAM 0
nvram_set wifi3 G_BAND_256QAM 0
else
nvram_set 2860 BssidNum 2
nvram_set wifi3 BssidNum 2
nvram_set rtdev BssidNum 2
fi
nvram_set 2860 MAP_Ext "32;32;32;32;32;32;32;32"
nvram_set rtdev MAP_Ext "32;32;32;32;32;32;32;32"
nvram_set wifi3 MAP_Ext "32;32;32;32;32;32;32;32"
nvram_set 2860 map_controller 1
nvram_set 2860 map_agent 1
nvram_set 2860 map_root 0
nvram_set 2860 bh_type wifi
nvram_set 2860 HT_BW 0
nvram_set rtdev HT_BW 0
nvram_set wifi3 HT_BW 0
if [ "$MAP_R2" = "1" ];then
nvram_set 2860 RRMEnable "1;1;1;1"
nvram_set rtdev RRMEnable "1;1;1;1"
nvram_set wifi3 RRMEnable "1;1;1;1"
else
nvram_set 2860 RRMEnable "1;1"
nvram_set rtdev RRMEnable "1;1"
nvram_set wifi3 RRMEnable "1;1"
fi
nvram_set 2860 VHT_BW 0
nvram_set rtdev VHT_BW 0
nvram_set wifi3 VHT_BW 0
nvram_set 2860 AutoChannelSelect 0
nvram_set wifi3 AutoChannelSelect 0
nvram_set rtdev AutoChannelSelect 0
nvram_set 2860 SteerEnable 1
nvram_set 2860 MapMode 4 
nvram_set rtdev MapMode 4 
nvram_set wifi3 MapMode 4

#4) Configure Per SKU Params
#Default channel for 24G is 6 / 5GL is 36 /5GH is 149
#Radio band in case of 2 band 2.4G;5G;5G & in case of tri-band 2.4G;5Gh;5GL
# BSS priority should be configured in the order of interfaces and bss registered
echo -e "----- ${RED}EasyMesh CERT SCRIPT SKU=$dev Per SKU Config${NC} -----" 
if [ "$dev" = "1" ];then
	bss_config_priority="ra0;ra1;rax0;rax1;apcli0;apclix0"
	nvram_set 2860 radio_band "24G;5G;5G;"
	nvram_set 2860 Channel 6
	nvram_set wifi3 Channel 36
elif [ "$dev" = "2" ];then
	bss_config_priority="ra0;ra1;rai0;rai1;apcli0;apclii0"
	nvram_set 2860 Channel 6
	nvram_set rtdev Channel 36
	nvram_set 2860 radio_band "24G;5G;5G;"
elif [ "$dev" = "3" ];then
	bss_config_priority="ra0;ra1;rax0;rax1;apcli0;apclix0"
	nvram_set 2860 radio_band "24G;5G;5G;"
	nvram_set 2860 Channel 36
	nvram_set rtdev Channel 6
elif [ "$dev" = "4" ];then
	bss_config_priority="ra0;ra1;rai0;rai1;apcli0;apclii0"
	nvram_set 2860 Channel 6
	nvram_set rtdev Channel 36
	nvram_set 2860 radio_band "24G;5G;5G;"
elif [ "$dev" = "5" ];then
	bss_config_priority="ra0;ra1;rai0;rai1;rax0;rax1;apcli0;apclii0;apclix0"
	nvram_set 2860 Channel 6
	nvram_set wifi3 Channel 36
	nvram_set rtdev Channel 149
	nvram_set 2860 radio_band "24G;5GL;5GH;"
elif [ "$dev" = "6" ];then
	bss_config_priority="rai0;rai1;ra0;ra1;apclii0;apcli0"
	nvram_set 2860 radio_band "24G;5G;5G;"
	nvram_set 2860 Channel 6
	nvram_set rtdev Channel 36
	nvram_set 2860 WirelessMode 9
	nvram_set rtdev WirelessMode 14
	nvram_set 2860 HT_AMSDU 0
	nvram_set rtdev HT_AMSDU 0
	nvram_set wifi3 HT_AMSDU 0
	nvram_set 2860 HT_AutoBA 0
	nvram_set rtdev HT_AutoBA 0
	nvram_set wifi3 HT_AutoBA 0
	nvram_set 2860 HT_BADecline 1
	nvram_set rtdev HT_BADecline 1
	nvram_set wifi3 HT_BADecline 1
	nvram_set 2860 G_BAND_256QAM 0
	nvram_set rtdev G_BAND_256QAM 0
	nvram_set wifi3 G_BAND_256QAM 0
	nvram_set 2860 HT_TxStream 1
        nvram_set rtdev HT_TxStream 1
        nvram_set wifi3 HT_TxStream 1
        nvram_set 2860 HT_RxStream 1
        nvram_set rtdev HT_RxStream 1
        nvram_set wifi3 HT_RxStream 1
elif [ "$dev" = "7" ];then
	bss_config_priority="ra0;ra1;rai0;rai1;wlan0;wlan1;apcli0;apclii0;wlan-apcli0"
	nvram_set 2860 Channel 6
	nvram_set wifi3 Channel 36
	nvram_set rtdev Channel 149
	nvram_set 2860 radio_band "24G;5GL;5GH;"
elif [ "$dev" = "8" -o "$dev" = "12" ];then
	bss_config_priority="ra0;ra1;rai0;rai1;wlan0;wlan1;apcli0;apclii0;wlan-apcli0"
	nvram_set 2860 Channel 6
	nvram_set rtdev Channel 36
	nvram_set wifi3 Channel 149
	nvram_set rtdev ChannelGrp "1:1:0:0"
	nvram_set wifi3 ChannelGrp "0:0:1:1"
	nvram_set 2860 radio_band "24G;5GL;5GH;"
	if [ "$dev" = "12" ];then
		nvram_set 2860 HT_TxStream 1
		nvram_set rtdev HT_TxStream 1
		nvram_set wifi3 HT_TxStream 1
		nvram_set 2860 HT_RxStream 1
		nvram_set rtdev HT_RxStream 1
		nvram_set wifi3 HT_RxStream 1
		nvram_set rtdev Channel 149
		nvram_set wifi3 Channel 36
		nvram_set 2860 Channel 6
		nvram_set wifi3 ChannelGrp "1:1:0:0"
		nvram_set rtdev ChannelGrp "0:0:1:1"
	fi
elif [ "$dev" = "9" ];then
	bss_config_priority="ra0;ra1;rax0;rax1;apcli0;apclix0"
	nvram_set 2860 radio_band "24G;5G;5G;"
	nvram_set 2860 Channel 6
	nvram_set wifi3 Channel 36
elif [ "$dev" = "11" ];then
	bss_config_priority="rax0;rax1;rax2;rax3;rai0;rai1;rai2;rai3;ra0;ra1;ra2;ra3;apclix0;apclii0;apcli0"
	nvram_set 2860 radio_band "24G;5GL;5GH;"
	nvram_set 2860 Channel 149
	nvram_set wifi3 Channel 36
	nvram_set rtdev Channel 6
elif [ "$dev" = "13" -o "$dev" = "14" ];then
	bss_config_priority="ra0;ra1;rai0;rai1;apcli0;apclii0"
	nvram_set 2860 radio_band "24G;5G;5G;"
	nvram_set 2860 Channel 6
	nvram_set rtdev Channel 36
elif [ "$dev" = "15" ];then
        bss_config_priority="ra0;ra1;rax0;rax1;apcli0;apclix0"
	nvram_set 2860 radio_band "24G;5G;5G;"
	nvram_set 2860 Channel 6
	nvram_set rtdev Channel 36
	nvram_set 2860 WirelessMode 9
	nvram_set rtdev WirelessMode 14
	nvram_set 2860 HT_AMSDU 0
	nvram_set rtdev HT_AMSDU 0
	nvram_set wifi3 HT_AMSDU 0
	nvram_set 2860 HT_AutoBA 0
	nvram_set rtdev HT_AutoBA 0
	nvram_set wifi3 HT_AutoBA 0
	nvram_set 2860 HT_BADecline 1
	nvram_set rtdev HT_BADecline 1
	nvram_set wifi3 HT_BADecline 1
	nvram_set 2860 G_BAND_256QAM 0
	nvram_set rtdev G_BAND_256QAM 0
	nvram_set wifi3 G_BAND_256QAM 0
	nvram_set 2860 HT_TxStream 1
        nvram_set rtdev HT_TxStream 1
        nvram_set wifi3 HT_TxStream 1
        nvram_set 2860 HT_RxStream 1
        nvram_set rtdev HT_RxStream 1
        nvram_set wifi3 HT_RxStream 1
elif [ "$dev" = "16" ];then
	bss_config_priority="ra0;ra1;rai0;rai1;apcli0;apclii0"
	nvram_set 2860 Channel 6
	nvram_set rtdev Channel 36
	nvram_set wifi3 Channel 149
	nvram_set 2860 radio_band "24G;5G;5G;"
	nvram_set 2860 HT_TxStream 1
	nvram_set rtdev HT_TxStream 1
	nvram_set wifi3 HT_TxStream 1
	nvram_set 2860 HT_RxStream 1
	nvram_set rtdev HT_RxStream 1
	nvram_set wifi3 HT_RxStream 1
	nvram_set 2860 WirelessMode 9
	nvram_set rtdev WirelessMode 14
	nvram_set 2860 HT_AMSDU 0
	nvram_set rtdev HT_AMSDU 0
	nvram_set wifi3 HT_AMSDU 0
	nvram_set 2860 HT_AutoBA 0
	nvram_set rtdev HT_AutoBA 0
	nvram_set wifi3 HT_AutoBA 0
	nvram_set 2860 HT_BADecline 1
	nvram_set rtdev HT_BADecline 1
	nvram_set wifi3 HT_BADecline 1
	nvram_set 2860 G_BAND_256QAM 0
	nvram_set rtdev G_BAND_256QAM 0
	nvram_set wifi3 G_BAND_256QAM 0
fi

echo -e "----- ${RED}EasyMesh CERT SCRIPT BSS Config=$bss_config_priority ${NC} -----" 
echo -e "----- ${RED}EasyMesh CERT SCRIPT DBDC=$DBDC_MODE ${NC} -----" 
#Enable DBDC For SKU's
if [ "$DBDC_MODE" = "1" ];then
	nvram_set 2860 DBDC_MODE 1
	nvram_set rtdev DBDC_MODE 1
	nvram_set wifi3 DBDC_MODE 1
else
	nvram_set 2860 DBDC_MODE 0
	nvram_set rtdev DBDC_MODE 0
	nvram_set wifi3 DBDC_MODE 0
fi
if [ "$Role" = "1" ];then
	if [ "$SDK_TYPE" = "OPENWRT" ];then
		wificonf -f /etc/map/mapd_user.cfg set DeviceRole 1
	else
		nvram_set 2860 DeviceRole 1
	fi
else
	if [ "$SDK_TYPE" = "OPENWRT" ];then
		wificonf -f /etc/map/mapd_user.cfg set DeviceRole 2
	else
		nvram_set 2860 DeviceRole 2
	fi
fi

#5) Configure the Lan and WAN Interfaces Per  Platform
echo -e "----- ${RED}EasyMesh CERT SCRIPT set LAN WAN Interface ${NC} -----" 
if [ "$PLATFORM" = MT7622 ];then
	nvram_set lan_inf_name $LAN_INTF
	nvram_set br_inf_name br-lan
	wificonf -f /etc/map/mapd_user.cfg set lan_interface $LAN_INTF
	wificonf -f /etc/map/mapd_user.cfg set wan_interface $WAN_INTF
	wificonf -f /etc/map/1905d.cfg set lan $LAN_INTF

	elif [ "$PLATFORM" = MT7629 ];then
	nvram_set lan_inf_name $LAN_INTF
	nvram_set br_inf_name br-lan
	wificonf -f /etc/map/mapd_user.cfg set lan_interface $LAN_INTF
	wificonf -f /etc/map/mapd_user.cfg set wan_interface $WAN_INTF
	wificonf -f /etc/map/1905d.cfg set lan $LAN_INTF

elif [ "$PLATFORM" = MT7915 ];then
	nvram_set lan_inf_name $LAN_INTF
	nvram_set br_inf_name br-lan
	wificonf -f /etc/map/mapd_user.cfg set lan_interface $LAN_INTF
	wificonf -f /etc/map/mapd_user.cfg set wan_interface $WAN_INTF
	wificonf -f /etc/map/1905d.cfg set lan $LAN_INTF

elif [ "$PLAFORM" = MT7621 ];then
	if ["$SDK_TYPE" = LSDK ]; then
		nvram_set lan_inf_name $LAN_INTF
		nvram_set br_inf_name br0
	else
		nvram_set lan_inf_name $LAN_INTF
		nvram_set br_inf_name br-lan
	fi
fi

#6) Configure IP Address
echo -e "----- ${RED}EasyMesh CERT SCRIPT Configure IP Address  ${NC} -----" 
if [ "$SDK_TYPE" = "OPENWRT" ];then
uci set dhcp.lan.ignore='1'
uci set firewall.@zone[1].input='ACCEPT'
uci set firewall.@zone[1].forward='ACCEPT'
uci set network.wan6.proto='static'
uci set network.wan.proto='static'
uci set network.lan.proto='static'
uci set network.lan.ipaddr='192.165.100.200'
uci set network.lan.netmask='255.255.255.0'
uci set network.wan.ipaddr='192.168.250.200'
uci set network.wan.netmask='255.255.255.0'
uci commit
fi

if [ "$SDK_TYPE" = "LSDK" ];then
	nvram_set map_agent_alid $al_mac
	nvram_set map_controller_alid $al_mac
	nvram_set wan_ipaddr 192.168.250.200
	nvram_set lan_ipaddr 192.165.100.200
	nvram_set 2860 BSS_CONFIG_PRIORITY $bss_config_priority
fi
#7) Configuration for map_cfg.txt and mapd_cfg.txt
echo -e "----- ${RED}EasyMesh CERT SCRIPT Configure map_cfg.txt and mapd_cfg.txt  ${NC} -----" 
if [ "$SDK_TYPE" = "OPENWRT" ];then
wificonf -f /etc/map/1905d.cfg set lan $LAN_INTF
wificonf -f /etc/map/1905d.cfg set bss_config_priority $bss_config_priority
wificonf -f /etc/map/mapd_default.cfg set bss_config_priority $bss_config_priority
wificonf -f /etc/map/mapd_default.cfg set MapEnable 1
wificonf -f /etc/map/mapd_default.cfg set lan_interface $LAN_INTF
wificonf -f /etc/map/mapd_default.cfg set wan_interface $WAN_INTF
wificonf -f /etc/map/mapd_default.cfg set BandSwitchTime 0
wificonf -f /etc/map/mapd_default.cfg set AutoBHSwitching 0
wificonf -f /etc/map/mapd_default.cfg set ChPlanningEnable 0
wificonf -f /etc/map/mapd_default.cfg set ChPlanningEnableR2 0
wificonf -f /etc/map/mapd_default.cfg set NetworkOptimizationEnabled 0
wificonf -f /etc/map/mapd_default.cfg set DhcpCtl 0
wificonf -f /etc/map/1905d.cfg set config_agent_port 9008
wificonf -f /etc/wapp_ap_wlan0.conf set gas_cb_delay 0
wificonf -f /etc/wapp_ap_ra0.conf set gas_cb_delay 0
wificonf -f /etc/wapp_ap_rai0.conf set gas_cb_delay 0
wificonf -f /etc/wapp_ap_rax0.conf set gas_cb_delay 0
wificonf -f /etc/wapp_ap_wlan0_default.conf set gas_cb_delay 0
wificonf -f /etc/wapp_ap_ra0_default.conf set gas_cb_delay 0
wificonf -f /etc/wapp_ap_rai0_default.conf set gas_cb_delay 0
wificonf -f /etc/wapp_ap_rax0_default.conf set gas_cb_delay 0
wificonf -f /etc/map/mapd_default.cfg set CentralizedSteering 0
fi

if [ "$MAP_R2" = "1" ]; then
wificonf -f /etc/map/1905d.cfg set map_ver R2
else
wificonf -f /etc/map/1905d.cfg set map_ver R1
fi

if [ "$SDK_TYPE" = "LSDK" ];then
nvram_set 2860 MapEnable 1
nvram_set 2860 AutoBHSwitch 0
nvram_set 2860 BandSwitchTime 0
nvram_set 2860 ChPlanningEnable 0
fi
reboot

