#!/bin/sh

RED='\033[0;31m'
NC='\033[0m'
echo -e "----- ${RED}EasyMesh SCRIPT${NC} -----"
check_default="$1"

perportpervlan="$1"
mode="$2"
echo "param1 perportpervlan=$perportpervlan"
echo "param1  0--> disable perportpervlan"
echo "param1  1--> enable perportpervlan"
echo "param2 mode=$mode"
echo "param2  0--> router mode on dev"
echo "param2  1--> bridge mode on dev"

ramips_board_name() {
	local name

	[ -f /tmp/sysinfo/board_name ] && name=$(cat /tmp/sysinfo/board_name)
	[ -z "$name" ] && name="unknown"

	echo "${name%-[0-9]*M}"
}

board=$(ramips_board_name)
platform=${board:0:6}
echo "##################################$platform"

clean()
{
	rm -rf /tmp/wapp_ctrl
	killall -15 mapd
	killall -15 wapp
	killall -15 p1905_managerd
	killall -15 bs20
	sleep 5
	rmmod mapfilter
	echo -e "----- ${RED}killed all apps ${NC} -----"
}
#call routine to decide operating mode
get_operating_mode()
{
	card0_profile_path=`cat /etc/wireless/l1profile.dat | grep INDEX0_profile_path |awk -F "[=;]" '{ print $2 }'`
	#MapEnable=`cat ${card0_profile_path} | grep MapEnable | awk -F "=" '{ print $2 }'`
	#MapTurnKey=`cat ${card0_profile_path} | grep MAP_Turnkey | awk -F "=" '{ print $2 }'`
	#BSEnable=`cat ${card0_profile_path} | grep BSEnable | awk -F "=" '{ print $2 }'`
	MapMode=`cat ${card0_profile_path} | grep MapMode | awk -F "=" '{ print $2 }'`
	LastMapMode=`cat /etc/map/mapd_default.cfg | grep LastMapMode | awk -F "=" '{ print $2 }'`
	if [ -z ${LastMapMode} ]
	then
	echo "fist start init last_map_mode ${MapMode}"
	LastMapMode="${MapMode}"
	fi
	echo "update last_map_mode ${MapMode}"
	sed -i "s/LastMapMode=.*/LastMapMode=${MapMode}/g" /etc/map/mapd_default.cfg
}
#Function reads interface name from the l1profile
prepare_ifname_card()
{
	echo "$card_idx"
	card_ext_ifname=`cat /etc/wireless/l1profile.dat | grep INDEX${card_idx}_ext_ifname |awk -F "=" '{ print $2 }'`
	echo $card_ext_ifname
	card_profile_path=`cat /etc/wireless/l1profile.dat | grep INDEX${card_idx}_profile_path |awk -F "=" '{ print $2 }'`
	echo $card_profile_path
	card_apcli_ifname=`cat /etc/wireless/l1profile.dat | grep INDEX${card_idx}_apcli_ifname |awk -F "=" '{ print $2 }'`
	echo $card_apcli_ifname
	card_main_ifname=`cat /etc/wireless/l1profile.dat | grep INDEX${card_idx}_main_ifname |awk -F "=" '{ print $2 }'`
	card_bssid_num=`cat ${card_profile_path} | grep BssidNum | awk -F "=" '{ print $2 }'`
	echo $card_bssid_num
	iwconfig ${card_main_ifname} | grep "Access Point"
	card_exist=$?
	echo $card_exist
	if [ $card_exist == 0 ]
	then
		active_cards=`expr $active_cards + 1`
		bssid_num=0
		while [ $bssid_num -lt $card_bssid_num ]
		do
			if [ -z ${if_list} ]
			then
			if_list="${card_ext_ifname}${bssid_num}"
			else
			if_list="${if_list};${card_ext_ifname}${bssid_num}"
			fi
			bssid_num=`expr $bssid_num + 1`
		done
		if_list="${if_list};${card_apcli_ifname}0"
		echo ${card_main_ifname} >> main_ifname
		echo ${card_apcli_ifname}"0" >> apcli_ifname
	fi
	echo $if_list
}
prepare_ifame_band()
{
	card_ext_ifname=`cat /etc/wireless/l1profile.dat | grep INDEX${card_idx}_ext_ifname |awk -F "[=;]" '{ print $2 }'`
	echo $card_ext_ifname
	card_profile_path=`cat /etc/wireless/l1profile.dat | grep INDEX${card_idx}_profile_path |awk -F "[=;]" '{ print $2 }'`
	echo $card_profile_path
	card_apcli_ifname=`cat /etc/wireless/l1profile.dat | grep INDEX${card_idx}_apcli_ifname |awk -F "[=;]" '{ print $2 }'`
	echo $card_apcli_ifname
	card_main_ifname=`cat /etc/wireless/l1profile.dat | grep INDEX${card_idx}_main_ifname |awk -F "[=;]" '{ print $2 }'`
	card_bssid_num=`cat ${card_profile_path} | grep BssidNum | awk -F "=" '{ print $2 }'`
	echo $card_bssid_num
	iwconfig ${card_main_ifname} | grep "Access Point"
	card_exist=$?
	echo $card_exist
	if [ $card_exist == 0 ]
	then
		active_cards=`expr $active_cards + 1`
		bssid_num=0
		while [ $bssid_num -lt $card_bssid_num ]
		do
			if [ -z ${if_list} ]
			then
			if_list="${card_ext_ifname}${bssid_num}"
			else
			if_list="${if_list};${card_ext_ifname}${bssid_num}"
			fi
			bssid_num=`expr $bssid_num + 1`
		done
		if_list="${if_list};${card_apcli_ifname}0"
		echo ${card_main_ifname} >> main_ifname
		echo ${card_apcli_ifname}"0" >> apcli_ifname
	fi
	echo $if_list
	card_ext_ifname=`cat /etc/wireless/l1profile.dat | grep INDEX${card_idx}_ext_ifname |awk -F "[=;]" '{ print $3 }'`
	echo $card_ext_ifname
	card_profile_path=`cat /etc/wireless/l1profile.dat | grep INDEX${card_idx}_profile_path |awk -F "[=;]" '{ print $3 }'`
	echo $card_profile_path
	card_apcli_ifname=`cat /etc/wireless/l1profile.dat | grep INDEX${card_idx}_apcli_ifname |awk -F "[=;]" '{ print $3 }'`
	echo $card_apcli_ifname
	card_main_ifname=`cat /etc/wireless/l1profile.dat | grep INDEX${card_idx}_main_ifname |awk -F "[=;]" '{ print $3 }'`
	card_bssid_num=`cat ${card_profile_path} | grep BssidNum | awk -F "=" '{ print $2 }'`
	echo $card_bssid_num
	iwconfig ${card_main_ifname} | grep "Access Point"
	card_exist=$?
	echo $card_exist
	if [ $card_exist == 0 ]
	then
		active_cards=`expr $active_cards + 1`
		bssid_num=0
		while [ $bssid_num -lt $card_bssid_num ]
		do
			if [ -z ${if_list} ]
			then
			if_list="${card_ext_ifname}${bssid_num}"
			else
			if_list="${if_list};${card_ext_ifname}${bssid_num}"
			fi
			bssid_num=`expr $bssid_num + 1`
		done
		if_list="${if_list};${card_apcli_ifname}0"
		echo ${card_main_ifname} >> main_ifname
		echo ${card_apcli_ifname}"0" >> apcli_ifname
	fi
	echo $if_list
}
prepare_ifname()
{
	card_idx=0
	active_cards=0
	rm -rf apcli_ifname main_ifname
	while [ $card_idx -le 2 ]
	do
		card=`cat /etc/wireless/l1profile.dat | grep INDEX${card_idx}= | awk -F "=" '{ print $2 }'`
		echo "$card"
		if [ -z "$card" ]
		then
			echo "No card"
		else
			dbdc_detect=`cat /etc/wireless/l1profile.dat | grep INDEX${card_idx}_main_ifname= | awk -F ";" '{ print $2 }'`

			if [ -z "$dbdc_detect" ]
			then
				prepare_ifname_card
			else
				echo "dbdc detected"
				prepare_ifame_band
			fi
		fi
		card_idx=`expr $card_idx + 1`
	done
}

#call routine for SDK specfic config preparation(ra_band/ bh_priority etc)
prepare_platform_variables()
{
	#prepare_ifname
	if [ $active_cards == "2" ]
	then
		radio_band="24G;5G;5G"
	elif [ $active_cards == "3" ]
	then
		radio_band="24G;5GH;5GL"
	fi
	lan_iface=`uci get network.lan.ifname | awk -F " " '{ print $1 }'`
	wan_iface=`uci get network.wan.ifname`
#derive almac from br mac
	br0_mac=$(cat /sys/class/net/br-lan/address)
	ctrlr_al_mac=$br0_mac
	agent_al_mac=$br0_mac
}

R2_config_check() {
#MAP version check
		MapVer=`cat /etc/map/1905d.cfg | grep map_ver| awk -F "=" '{ print $2 }'`
		if [ "$MapVer" = "R1" ]; then
			echo "MAP version 1.0"
			loop_count=1
			while [ $loop_count -le $active_cards ]
			do
				if_name=`sed -n "${loop_count}"p ./main_ifname`
				iwpriv ${if_name} set mapR2Enable=0
				echo "iwpriv $if_name set mapR2Enable=0"
				iwpriv ${if_name} set mapTSEnable=0
				echo "iwpriv $if_name set mapTSEnable=0"
				loop_count=`expr $loop_count + 1`
			done
		else
			echo "MAP version 2.0"
			loop_count=1
			while [ $loop_count -le $active_cards ]
			do
				if_name=`sed -n "${loop_count}"p ./main_ifname`
				iwpriv ${if_name} set mapR2Enable=1
				echo "iwpriv $if_name set mapR2Enable=1"
				iwpriv ${if_name} set mapTSEnable=1
				echo "iwpriv $if_name set mapTSEnable=1"
				loop_count=`expr $loop_count + 1`
			done
		fi
}

disable_R2_config() {
	echo "disable_R2_config"
	loop_count=1
	while [ $loop_count -le $active_cards ]
	do
		if_name=`sed -n "${loop_count}"p ./main_ifname`
		iwpriv ${if_name} set mapR2Enable=0
		echo "iwpriv $if_name set mapR2Enable=0"
		iwpriv ${if_name} set mapTSEnable=0
		echo "iwpriv $if_name set mapTSEnable=0"
		loop_count=`expr $loop_count + 1`
	done
}

#call routine to prepare wapp configuration files
#call routine to prepare 1905 config file
prepare_1905_config()
{
		DeviceRole=`cat /etc/map/mapd_cfg | grep DeviceRole | awk -F "=" '{ print $2 }'`
		lan_iface=`uci get network.lan.ifname | awk -F " " '{ print $1 }'`
		wan_iface=`uci get network.wan.ifname`
		bridge_name=`cat br_name | awk '{ print $1}'`
		brctl addif $bridge_name $lan_iface
		#if [ `grep -c "lan=$lan_iface" /etc/map/1905d.cfg` -ne '0' ]; then
		#elif [ `grep -c "lan=" /etc/map/1905d.cfg` -ne '0' ]; then
		sed -i "s/radio_band=.*/radio_band=${radio_band}/g" /etc/map/1905d.cfg
		sed -i "s/map_controller_alid=.*/map_controller_alid=${ctrlr_al_mac}/g" /etc/map/1905d.cfg
		sed -i "s/map_agent_alid=.*/map_agent_alid=${agent_al_mac}/g" /etc/map/1905d.cfg
		sed -i "s/bss_config_priority=.*/bss_config_priority=${if_list}/g" /etc/map/1905d.cfg
		if [ $DeviceRole = "1" ]
		then
			sed -i "s/map_agent=.*/map_agent=0/g" /etc/map/1905d.cfg
			sed -i "s/map_root=.*/map_root=1/g" /etc/map/1905d.cfg
		else
			sed -i "s/map_agent=.*/map_agent=1/g" /etc/map/1905d.cfg
			sed -i "s/map_root=.*/map_root=0/g" /etc/map/1905d.cfg
		fi
		
		####delete all lan= wan=
		sed -i '/lan=/d' /etc/map/1905d.cfg
		sed -i '/wan=/d' /etc/map/1905d.cfg
		lastlinestr='$a '
		lanstr='lan='
		wanstr='wan='
		#######vlan config setting
		if [ "${perportpervlan}" = "1" ]; then
			echo "prepare_1905_config perportpervlan enabled"
			lan_iface1=$lan_iface.1
			lan_iface2=$lan_iface.2
			lan_iface3=$lan_iface.3
			lan_iface4=$lan_iface.4
			lan1_insert_str=$lastlinestr$lanstr$lan_iface1
			lan2_insert_str=$lastlinestr$lanstr$lan_iface2
			lan3_insert_str=$lastlinestr$lanstr$lan_iface3
			lan4_insert_str=$lastlinestr$lanstr$lan_iface4
			echo "!!!!!!!!!!!!!!!!!!!!!$lan1_insert_str"
			`sed -i "$lan1_insert_str" /etc/map/1905d.cfg`
			`sed -i "$lan2_insert_str" /etc/map/1905d.cfg`
			`sed -i "$lan3_insert_str" /etc/map/1905d.cfg`
			`sed -i "$lan4_insert_str" /etc/map/1905d.cfg`
			sed -i "s/lan_vid=.*/lan_vid=1;2;3;4;/g" /etc/ethernet_cfg.txt
			sed -i "s/wan_vid=.*/wan_vid=5;/g" /etc/ethernet_cfg.txt
		else
			echo "prepare_1905_config perportpervlan disabled"
			lan_insert_str=$lastlinestr$lanstr$lan_iface
			`sed -i "$lan_insert_str" /etc/map/1905d.cfg`
			sed -i "s/lan_vid=.*/lan_vid=1;/g" /etc/ethernet_cfg.txt
			sed -i "s/wan_vid=.*/wan_vid=2;/g" /etc/ethernet_cfg.txt
		fi
		#######mode
		if [ $mode = "1" ]; then
			echo "prepare_1905_config bridge mode"
			wan_insert_str=$lastlinestr$wanstr$wan_iface
			`sed -i "$wan_insert_str" /etc/map/1905d.cfg`
			brctl addif $bridge_name $wan_iface
		else
			brctl delif $bridge_name $wan_iface
		fi
}
prepare_logging_config()
{
echo "
/tmp/log/log.mapd {
	size 64K
	copytruncate
	rotate 2
}	
" > /etc/logrotate.d/mapd.log.conf
}

#preapre config file for map steering parameters
prepare_mapd_strng_configs()
{
	if [ ! -f "/etc/mapd_strng.conf" -o "$check_default" = "default" ];then
		echo "Make mapd_strng.conf"
echo "LowRSSIAPSteerEdge_RE=40
CUOverloadTh_2G=70
CUOverloadTh_5G_L=80
CUOverloadTh_5G_H=80
MetricPolicyChUtilThres_24G=70
MetricPolicyChUtilThres_5GL=80
MetricPolicyChUtilThres_5GH=80
ChPlanningChUtilThresh_24G=70
ChPlanningChUtilThresh_5GL=80
ChPlanningEDCCAThresh_24G=200
ChPlanningEDCCAThresh_5GL=200
ChPlanningOBSSThresh_24G=200
ChPlanningOBSSThresh_5GL=200
ChPlanningR2MonitorTimeoutSecs=300
ChPlanningR2MonitorProhibitSecs=900
ChPlanningR2MetricReportingInterval=10
ChPlanningR2MinScoreMargin=10
MetricRepIntv=10
MetricPolicyRcpi_24G=100
MetricPolicyRcpi_5GL=100
MetricPolicyRcpi_5GH=100
" > /etc/mapd_strng.conf
	fi
}
#call routines for mapd_cfg.txt preparation
prepare_mapd_config()
{
	need_loop=1
	line_count=1
	cp /etc/map/mapd_default.cfg /etc/map/mapd_cfg
	if [ "$check_default" = "default" ];then
		need_loop=0
		cat "###!UserConfigs!!!" > /etc/map/mapd_user.cfg
	fi
	sed -i "s/lan_interface=.*/lan_interface=${lan_iface}/g" /etc/map/mapd_cfg
	sed -i "s/wan_interface=.*/wan_interface=${wan_iface}/g" /etc/map/mapd_cfg
	while [ $need_loop == "1" ]
	do
		line=`sed -n "$line_count"p /etc/map/mapd_user.cfg`
		echo $line
		if [ -z $line ]
		then
			need_loop=0
		else
			key=`echo ${line} | awk -F "=" '{ print $1 }'`
			value=`echo ${line} | awk -F "=" '{ print $2 }'`
			echo "Key = ${key}, value = ${value}"
			sed -i "s/${key}=.*/${key}=${value}/g" /etc/map/mapd_cfg
		fi 	
		line_count=`expr $line_count + 1`
	done
	enhanced_logging=`cat /etc/map/mapd_cfg | grep "EnhancedLogging" | awk -F "=" '{ print $2 }'`
	if [ $enhanced_logging = "1" ]
	then
		prepare_logging_config
	fi
	sed -i "s/bss_config_priority=.*/bss_config_priority=${if_list}/g" /etc/map/mapd_cfg
	prepare_mapd_strng_configs
}


Cert_switch_config()
{
	# GMAC1 need To config swith vlan
	# platform mt7621(only for certification mode use)
	# support openwrt
	if [ -f /etc/kernel.config ]; then
	  echo "easymesh in openwrt version"
	  . /etc/kernel.config
	  if [ "$CONFIG_RALINK_MT7621" = "y" -o\
	  "$CONFIG_MT7621_ASIC" = "y" ]; then
		echo "easymesh board name is 7621"
		if [ "${MapMode}" = "4" ]; then
		echo "mapmode is certification mode, config switch_setup "
		  . /lib/network/switch.sh
		  setup_switch
		fi
	  fi
	fi

}

cert_switch_changes()
{
	card_gsw_setting=`cat /proc/device-tree/gsw@0/compatible`
	echo $card_gsw_setting
	if [ "$card_gsw_setting" = "mediatek,mt753x" ] ;then
		echo "found green card with MT7631 switch"
		switch reg w 34 8160816
		switch reg w 4 60
		switch reg w 10 ffffffff
	fi
	#card_gsw_setting=`cat /proc/device-tree/rtkgswsys@1b100000/compatible`
	#echo $card_gsw_setting
	#if [ "$card_gsw_setting" = "mediatek,rtk-gsw" ] ;then
	#	echo "found red card with switch RTL8367S"
	#fi
}

disconnect_all_sta()
{
	need_loop=1
	loop_count=1
	while [ $loop_count -le $active_cards ]
	do
		if_name=`sed -n "${loop_count}"p ./main_ifname`
		`iwpriv ${if_name} set DisConnectAllSta=`
		loop_count=`expr $loop_count + 1`
	done
}

ApCliIntfUp()
{
	need_loop=1
	loop_count=1
	while [ $loop_count -le $active_cards ]
	do
		if_name=`sed -n "${loop_count}"p ./apcli_ifname`
		`ifconfig | grep "br" > br_name`
		bridge_name=`cat br_name | awk '{ print $1}'`
		`ifconfig ${if_name} up`
		`brctl addif ${bridge_name} ${if_name}`
		`iwpriv ${if_name} set ApCliEnable=0`
		loop_count=`expr $loop_count + 1`
	done
	sleep 2
}

ApCliIntfDown()
{
	need_loop=1
	loop_count=1
	while [ $loop_count -le $active_cards ]
	do
		if_name=`sed -n "${loop_count}"p ./apcli_ifname`
		`ifconfig | grep "br" > br_name`
		bridge_name=`cat br_name | awk '{ print $1}'`
		`ifconfig ${if_name} down`
		`brctl addif ${bridge_name} ${if_name}`
		`iwpriv ${if_name} set ApCliEnable=0`
		loop_count=`expr $loop_count + 1`
	done
	sleep 2
}

SwitchSetting()
{
	if [ "$platform" == "mt7621" -o "$platform" == "mt7622" ]; then
	  echo "easymesh board name is 7621"
	#Rule 1:  P5 1905 multicast forwarding to WAN port(P4)
	  #step 1: enable port 5 ACL function
	   switch  reg w 2504 ff0403

	  #step 2:ACL pattern
	  # pattern 16bit
	  switch reg w 94 ffff0180
	  #check MAC header OFST_TP=000   offset=0  check p5 frame
	  switch reg w 98 82000
	  #function:0101 ACL rule  rule0
	  switch reg w 90 80005004

	  switch reg w 94 ffffc200
	  #check MAC header OFST_TP=000   offset=2  check p5 frame
	  switch reg w 98 82002
	  #function:0101 ACL rule1
	  switch reg w 90 80005005

	  switch reg w 94 ffff0013
	  #check MAC header offset=4  check p5 frame 
	  switch reg w 98 82004
	  #function:0101 ACL rule2
	  switch reg w 90 80005006

	  switch reg w 94 ffff893A
	  switch reg w 98 8200c
	  switch reg w 90 80005007

	  #step3: ACL mask entry
	   switch reg w 94 0xf0
	  switch reg w 98 0
	  #FUNC= 0x0101
	  switch reg w 90 80009001
	 
	  #step4: ACL rule control :force forward to P4 or P1
	  #PORT_EN = 1 forward to P4; if forward to P1, 18000284
	  switch reg w 94 18001084
	  switch reg w 98 0
	  #func=1011   ACL rule control
	  switch reg w 90 8000B001
	  
	#Rule 2:  P0/P4 1905 multicast forwarding to P5 port
	  #step 1:  enable port 4 function, if you want to enable port 0. Switch reg w 2004 ff403
	  switch  reg w 2404 ff0403  

	  #step 2:ACL pattern
	  #pattern 16bit
	  switch reg w 94 ffff0180
	  #check MAC header OFST_TP=000   offset=0  check p4 frame, if P0, 80100
	  switch reg w 98 81000
	  #function:0101 ACL rule  rule0
	  switch reg w 90 80005000

	  switch reg w 94 ffffc200
	  #check MAC header OFST_TP=000   offset=2  check p4 frame, if P0, 80102
	  switch reg w 98 81002
	  #function:0101 ACL rule1
	  switch reg w 90 80005001

	  switch reg w 94 ffff0013
	  #check MAC header offset=4  check p4 frame, if P0, 80104 
	  switch reg w 98 81004
	  #function:0101 ACL rule2
	  switch reg w 90 80005002

	  #step 3: ACL mask entry
	  switch reg w 94 7
	  switch reg w 98 0
	  #FUNC= 0x0101 
	  switch reg w 90 80009000
	 
	  #step 4: ACL rule control :force forward to P5
	  #PORT_EN = 1 dp=5
	  switch reg w 94 18002084
	  switch reg w 98 0
	  #func=1011   ACL rule control
	  switch reg w 90 8000B000
	fi
}

perportpervlan_test()
{
	#per port per vlan setting
	br0_mac=$(cat /sys/class/net/br-lan/address)
	lan_iface=`uci get network.lan.ifname | awk -F " " '{ print $1 }'`
	bridge_name=`cat br_name | awk '{ print $1}'`
	lan_iface1=$lan_iface.1
	lan_iface2=$lan_iface.2
	lan_iface3=$lan_iface.3
	lan_iface4=$lan_iface.4
	br0_mac=$(cat /sys/class/net/br-lan/address)
	br0_mac_first_five_str=${br0_mac%:*}
	lan_addr2=$br0_mac_first_five_str:22
	lan_addr3=$br0_mac_first_five_str:33
	lan_addr4=$br0_mac_first_five_str:44
	echo "#####################perportpervlan_test"
	echo $lan_iface1
	echo $lan_iface2
	echo $lan_iface3
	echo $lan_iface4
	echo $lan_addr2
	echo $lan_addr3
	echo $lan_addr4
	
	brctl delif $bridge_name $lan_iface
	vconfig add $lan_iface 1
	vconfig add $lan_iface 2
	vconfig add $lan_iface 3
	vconfig add $lan_iface 4

	ifconfig $lan_iface1 hw ether $br0_mac
	ifconfig $lan_iface1 up
	ifconfig $lan_iface2 hw ether $lan_addr2
	ifconfig $lan_iface2 up
	ifconfig $lan_iface3 hw ether $lan_addr3
	ifconfig $lan_iface3 up
	ifconfig $lan_iface4 hw ether $lan_addr4
	ifconfig $lan_iface4 up

	switch vlan set 0 1 10000011
	switch vlan set 1 2 01000011
	switch vlan set 2 3 00100011
	switch vlan set 3 4 00010011
	switch vlan set 5 5 00001101
	switch vlan pvid 0 1
	switch vlan pvid 1 2
	switch vlan pvid 2 3
	switch vlan pvid 3 4
	switch vlan pvid 4 5
	switch vlan pvid 5 5

	switch reg w 2610 81000000
	#tag mode
	switch reg w 2604 20ff0003 

	brctl addif $bridge_name $lan_iface1
	brctl addif $bridge_name $lan_iface2
	brctl addif $bridge_name $lan_iface3
	brctl addif $bridge_name $lan_iface4
}


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
	
	if [ "$platform" == "mt7621" -o "$platform" == "mt7622" ]; then
	  echo "ModifySwitchReg easymesh board name is 7621"
	  switch reg w 10 ffffffe0
	  switch reg w 34 8160816
	fi
}

DHCP_INIT()
{
	DeviceRole=`cat /etc/map/mapd_cfg|\
		grep DeviceRole|awk -F "=" '{print $2}'`
	echo "DeviceRole: $DeviceRole (0: Auto 1:Controller 2:Agent)"
	DHCP_CTRL=`cat /etc/map/mapd_cfg|\
		grep DhcpCtl| awk -F "=" '{print $2}'`
	ThrdPrtCon=`cat /etc/map/mapd_cfg|grep ThirdPartyConnection\
		|awk -F "=" '{print $2}'`
	echo "DHCP Server setting: $DeviceRole $DHCP_CTRL $ThrdPrtCon"
	if [ $DHCP_CTRL = "1" ];then
		if [ $ThrdPrtCon == "1" -a "${MapMode}" = "1" ];then
		echo "Role($DeviceRole ) ThrdPrtCon($ThrdPrtCon):\
		Disable DHCP Server!"
			uci set dhcp.lan.ignore=1
			uci commit
			/etc/init.d/dnsmasq reload
		else
		echo "Role($DeviceRole ) ThrdPrtCon($ThrdPrtCon):\
		Enable DHCP Server!"
			if [ -z "$(uci -q get network.lan.ipaddr)" ];then
				echo "no IP, reload network ip to 192.168.1.1"
				uci -q set network.lan.ipaddr=192.168.1.1;
				ifconfig ${bridge_name} 192.168.1.1 up
				uci set dhcp.lan.ignore=\"\"
				uci commit
				/etc/init.d/dnsmasq reload
				/etc/init.d/network reload
			 else
				br_ip=`uci -q get network.lan.ipaddr`
				echo "br_ip: $br_ip"
				ifconfig ${bridge_name} $br_ip up
				uci set dhcp.lan.ignore=\"\"
				uci commit
				/etc/init.d/dnsmasq reload
			fi
		fi
	fi
	sleep 1
}

Traffic_Separation_Init()
{
	for index in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
	do
		iwpriv ra$index set VLANTag=1
		iwpriv rax$index set VLANTag=1
		iwpriv rai$index set VLANTag=1
		iwpriv ra$index set VLANPolicy=0:4
		iwpriv rax$index set VLANPolicy=0:4
		iwpriv rai$index set VLANPolicy=0:4
		iwpriv ra$index set VLANPolicy=1:2
		iwpriv rai$index set VLANPolicy=1:2
		iwpriv rai$index set VLANPolicy=1:2
	done
	iwpriv apcli0 set VLANTag=1
	iwpriv apcli0 set VLANPolicy=0:4
	iwpriv apcli0 set VLANPolicy=1:2
	iwpriv apclix0 set VLANTag=1
	iwpriv apclix0 set VLANPolicy=0:4
	iwpriv apclix0 set VLANPolicy=1:2
	iwpriv apclii0 set VLANTag=1
	iwpriv apclii0 set VLANPolicy=0:4
	iwpriv apclii0 set VLANPolicy=1:2
}

Traffic_Separation_Init_Cert()
{
	iwpriv ra0 set VLANEn=0
	iwpriv rax0 set VLANEn=0
	iwpriv rai0 set VLANEn=0
}

StartStandAloneBS()
{
	sleep 3
	echo "WAPP starting..."
	wapp_openwrt.sh > /dev/null
	sleep 3
	echo "BS2.0 Daemon starting..."
	bs20 &
	sleep 3
	disconnect_all_sta
	echo "Stand Alone BS2.0 is ready"
}

StartMapTurnkey()
{
	SwitchSetting
	echo $lan_iface > /sys/kernel/debug/hnat/hnat_ppd_if
	if [ "${perportpervlan}" = "1" ]; then
		echo "StartMapTurnkey perportpervlan enabled"
		echo $lan_iface.1 > /sys/kernel/debug/hnat/hnat_ppd_if
		perportpervlan_test
	fi
	R2_config_check
	ulimit -c unlimited
	#Controller
    DeviceRole=`cat /etc/map/mapd_cfg|\
	grep DeviceRole|awk -F "=" '{ print $2 }'`
   	echo $DeviceRole
	echo "dhcp starting..."
	DHCP_INIT
	Traffic_Separation_Init
	echo "WAPP starting..."
	wapp_openwrt.sh > /dev/null
	echo "1905 starting..."
	if [ $DeviceRole = "1" ]
	then
		p1905_managerd -r0 -f "/etc/map/1905d.cfg" -F "/etc/map/wts_bss_info_config" > /dev/console&
	else
		p1905_managerd -r1 -f "/etc//map/1905d.cfg" -F "/etc/map/wts_bss_info_config" > /dev/console&
	fi
	echo "MAP Daemon starting..."
	if [ $enhanced_logging = "1" ]
	then
		mapd -I "/etc/map/mapd_cfg" -O "/etc/mapd_strng.conf" > /tmp/log/log.mapd&
	else
		mapd -I "/etc/map/mapd_cfg" -O "/etc/mapd_strng.conf" > /dev/console&
	fi
	ModifySwitchReg
	sleep 3
	echo -e "----- ${RED}MAP DEVICE STARTED${NC} -----"
}

clean
prepare_ifname
get_operating_mode
prepare_platform_variables
prepare_1905_config
prepare_mapd_config
disable_R2_config

echo "kill fwdd daemon and remove mtfwd module"
#Enable QuickChChange feature.

ulimit -c unlimited

if [ "${MapMode}" = "0" ];
	then
	echo "dhcp starting..."
	if [ "${LastMapMode}" = "1" ]
	then
	echo "default dhcp server enable..."
	DHCP_INIT
	echo "defautl ApCliIntfDown..."
	ApCliIntfDown
	echo "defautl dat apcli disable..."
	nvram_set 2860 ApCliEnable 0
	nvram_set rtdev ApCliEnable 0
	nvram_set wifi3 ApCliEnable 0
	fi
	echo "Non MAP mode"
	need_loop=1
	loop_count=1
	while [ $loop_count -le $active_cards ]
	do
		if_name=`sed -n "${loop_count}"p ./main_ifname`
		iwpriv ${if_name} set mapEnable=0
		loop_count=`expr $loop_count + 1`
	done
	wapp_openwrt.sh
	modprobe mtfwd
	sleep 1
	fwdd -p ra0 apcli0 -p rai0 apclii0 -p rax0 apclix0 -p wlan0 wlan-apcli0 -e eth0 5G&
elif [ ${MapMode} = "2" ];
	then
	rmmod mapfilter
	echo "BS2.0 mode"
	while [ $loop_count -le $active_cards ]
	do
		if_name=`sed -n "${loop_count}"p ./main_ifname`
		iwpriv ${if_name} set mapEnable=2
		loop_count=`expr $loop_count + 1`
	done
	sleep 1
	StartStandAloneBS
	modprobe mtfwd
	sleep 1
	fwdd -p ra0 apcli0 -p rai0 apclii0 -p rax0 apclix0 -p wlan0 wlan-apcli0 -e eth0 5G&
elif [ ${MapMode} = "4" ];
	then
	killall -15 fwdd
	sleep 1
	rmmod mtfwd
	echo "Certification"
	Cert_switch_config
	cert_switch_changes
	mkdir /libmapd
	cp /usr/lib/libmapd_interface_client.so /libmapd/
	modprobe mapfilter
	ApCliIntfUp
	map_config_agent.lua start
	echo 458752 > /proc/sys/net/core/rmem_max
	Traffic_Separation_Init_Cert
	R2_config_check
else
	if [ "${MapMode}" = "1" ];
	then
		echo "TurnKeyMode"
		killall -15 fwdd
		sleep 1
		rmmod mtfwd
		mkdir /libmapd
		cp /usr/lib/libmapd_interface_client.so /libmapd/
		modprobe mapfilter
		ApCliIntfUp
		need_loop=1
		loop_count=1
		while [ $loop_count -le $active_cards ]
		do
			if_name=`sed -n "${loop_count}"p ./main_ifname`
			iwpriv ${if_name} set mapEnable=1
			loop_count=`expr $loop_count + 1`
		done
		sleep 1
		StartMapTurnkey
	fi
fi





