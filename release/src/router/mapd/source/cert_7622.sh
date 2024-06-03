dev="$1"
uci set dhcp.lan.ignore='1'
uci set firewall.@zone[1].input='ACCEPT'
uci set firewall.@zone[1].forward='ACCEPT'
uci set network.wan.proto='static'
uci set network.lan.proto='static'
uci set network.lan.ipaddr='192.165.100.150'
uci set network.lan.netmask='255.255.255.0'
uci set network.wan.ipaddr='192.168.250.150'
uci set network.wan.netmask='255.255.255.0'
uci commit
nvram_set 2860 DBDC_MODE 0
nvram_set rtdev DBDC_MODE 0
nvram_set wifi3 DBDC_MODE 0
nvram_set 2860 BssidNum 2
nvram_set wifi3 BssidNum 2
nvram_set rtdev BssidNum 2
if [ "$dev" = "C" ];then
nvram_set 2860 DeviceRole 1
wificonf -f /etc/map/mapd_cfg set DeviceRole 1
elif [ "$dev" = "A" ];then
nvram_set 2860 DeviceRole 2
wificonf -f /etc/map/mapd_cfg set DeviceRole 2
else
nvram_set 2860 DeviceRole 2
wificonf -f /etc/map/mapd_cfg set DeviceRole 2
fi
al_mac=$(cat /sys/class/net/br-lan/address)
nvram_set 2860 MapEnable 1
nvram_set 2860 MAP_Turnkey 0
nvram_set 2860 MAP_Ext "32;32;32;32;32;32;32;32"
nvram_set rtdev MapEnable 1
nvram_set rtdev MAP_Turnkey 0
nvram_set rtdev MAP_Ext "32;32;32;32;32;32;32;32"
nvram_set wifi3 MapEnable 1
nvram_set wifi3 MAP_Turnkey 0
nvram_set wifi3 MAP_Ext "32;32;32;32;32;32;32;32"
nvram_set 2860 map_controller 1
nvram_set 2860 map_agent 1
nvram_set 2860 map_root 0
nvram_set 2860 bh_type wifi
nvram_set 2860 radio_band "24G;5G;5G;"
nvram_set lan_inf_name eth1
nvram_set br_inf_name br-lan
nvram_set map_agent_alid $al_mac
nvram_set map_controller_alid $al_mac
nvram_set 2860 RRMEnable "1;1"
nvram_set rtdev RRMEnable "1;1"
nvram_set wifi3 RRMEnable "1;1"
nvram_set 2860 HT_BW 0
nvram_set rtdev HT_BW 0
nvram_set wifi3 HT_BW 0
nvram_set 2860 Channel 6
nvram_set rtdev Channel 36
nvram_set 2860 E2pAccessMode 1
nvram_set 2860 CalCacheApply 1
nvram_set rtdev E2pAccessMode 1
nvram_set rtdev CalCacheApply 1
nvram_set wifi3 E2pAccessMode 1
nvram_set wifi3 CalCacheApply 1
nvram_set 2860 AutoChannelSelect 0
nvram_set wifi3 AutoChannelSelect 0
nvram_set rtdev AutoChannelSelect 0

nvram_set 2860 SteerEnable 1
nvram_set 2860 bss_config_priority "ra0;ra1;rai0;rai1;apcli0;apclii0"
#sed -i '/MapEnable=0/c\MapEnable=1' /etc/map/mapd_cfg
#sed -i '/lan_interface=eth0/c\lan_interface=eth1' /etc/map/mapd_cfg
#sed -i '/wan_interface=eth3/c\wan_interface=eth0' /etc/map/mapd_cfg
#sed -i '/lan=eth0/c\lan=eth1' /etc/map_cfg.txt
#sed -i '/lan=eth0/c\lan=eth1' /etc/map_cfg.txt
wificonf -f /etc/map/mapd_cfg set MapEnable 1
wificonf -f /etc/map/mapd_cfg set lan_interface "eth0"
wificonf -f /usr/bin/EasyMesh_7622.sh set lan_interface "eth0"
wificonf -f /usr/bin/EasyMesh_7622.sh set wan_interface "eth1"
wificonf -f /etc/map/mapd_cfg set wan_interface "eth1"
wificonf -f /etc/map_cfg.txt set lan "eth0"
wificonf -f /usr/bin/EasyMesh_7622.sh set lan "eth0"
wificonf -f /etc/map/mapd_cfg set BandSwitchTime 0
wificonf -f /etc/map/mapd_cfg set AutoBHSwitching 0
wificonf -f /etc/map/mapd_cfg set ChPlanningEnable 0
wificonf -f /etc/map_cfg.txt set bss_config_priority "ra0;ra1;rai0;rai1;apcli0;apclii0"
#cat /usr/bin/switch_7622.sh > /etc/rc1
#cat /etc/rc.local >> /etc/rc1
#cat /etc/rc1 > /etc/rc.local
reboot
