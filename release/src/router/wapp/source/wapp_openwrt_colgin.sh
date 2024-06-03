#!/bin/sh

########################################
# define parameters
########################################
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
	
	all_inf_list="${all_inf_list}${card_main_ifname};"    #add to all main inf list
	all_conf_list="${all_conf_list}$CONFDIR/wapp_ap_$card_main_ifname.conf;"
	count=0
	while [ $count -lt $card_bssid_num ]
	do
		wapp_intf_name="${wapp_intf_name} -c${card_ext_ifname}${count}"
		count=$( expr $count + 1 )
	done
	echo "band 1 $wapp_intf_name"
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
	
	all_inf_list="${all_inf_list}${card_main_ifname};"    #add to all main inf list
	all_conf_list="${all_conf_list}$CONFDIR/wapp_ap_$card_main_ifname.conf;"
	count=0
	while [ $count -lt $card_bssid_num ]
	do
		wapp_intf_name="${wapp_intf_name} -c${card_ext_ifname}${count}"
		count=$( expr $count + 1 )
	done
	echo "band 0 $wapp_intf_name"
	
	card_ext_ifname=`cat /etc/wireless/l1profile.dat | grep INDEX${card_idx}_ext_ifname |awk -F "[=;]" '{ print $3 }'`
	echo $card_ext_ifname
	card_profile_path=`cat /etc/wireless/l1profile.dat | grep INDEX${card_idx}_profile_path |awk -F "[=;]" '{ print $3 }'`
	echo $card_profile_path
	card_apcli_ifname=`cat /etc/wireless/l1profile.dat | grep INDEX${card_idx}_apcli_ifname |awk -F "[=;]" '{ print $3 }'`
	echo $card_apcli_ifname
	card_main_ifname=`cat /etc/wireless/l1profile.dat | grep INDEX${card_idx}_main_ifname |awk -F "[=;]" '{ print $3 }'`
	card_bssid_num=`cat ${card_profile_path} | grep BssidNum | awk -F "=" '{ print $2 }'`
	echo $card_bssid_num
	
	all_inf_list="${all_inf_list}${card_main_ifname};"    #add to all main inf list
	all_conf_list="${all_conf_list}$CONFDIR/wapp_ap_$card_main_ifname.conf;"
	count=0
	while [ $count -lt $card_bssid_num ]
	do
		wapp_intf_name="${wapp_intf_name} -c${card_ext_ifname}${count}"
		count=$( expr $count + 1 )
	done
	echo "band 1 $wapp_intf_name"
}

prepare()
{
	card_idx=0
	active_cards=0
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

CONFDIR=/etc

########################################
# sanity check
########################################

#$1: 2860/rtdev , $2: ra0/rai0 $3: card_exist

killall wapp

### for certification use
if [ ! -f $CONFDIR/wapp_ap_ra0.conf ]; then
	cp $CONFDIR/wapp_ap_ra0_default.conf $CONFDIR/wapp_ap_ra0.conf
fi
if [ ! -f $CONFDIR/wapp_ap_rai0.conf ]; then
	cp $CONFDIR/wapp_ap_rai0_default.conf $CONFDIR/wapp_ap_rai0.conf
fi
if [ ! -f $CONFDIR/wapp_ap_wlan0.conf ]; then
	cp $CONFDIR/wapp_ap_wlan0_default.conf $CONFDIR/wapp_ap_wlan0.conf
fi
if [ ! -f $CONFDIR/wapp_ap_rax0.conf ]; then
	cp $CONFDIR/wapp_ap_rax0_default.conf $CONFDIR/wapp_ap_rax0.conf
fi

rm -rf /tmp/wapp*

wapp_intf_name=
########################################
# gen card1 config
########################################
all_inf_list="all_main_inf="
all_conf_list="conf_list="

########################################
# prepare all_inf_list+all_conf_list
########################################
prepare
echo "$all_inf_list"
echo "$all_conf_list"
########################################
# write conf_list, inf_list & start wapp daemon
########################################
if [ "$all_conf_list" != "" ]; then
	echo "${all_conf_list}" > $CONFDIR/wapp_ap.conf
fi
if [ "$all_inf_list" != "" ]; then
	echo "${all_inf_list}" > $CONFDIR/wapp_main_inf.conf
	#wapp -d1 -v2
	#Fix 11R Roaming Fail Issue.
	wapp -d1 -v2 $wapp_intf_name
fi

