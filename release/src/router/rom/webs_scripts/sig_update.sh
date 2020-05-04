#!/bin/sh

wget_options="-q -t 2 -T 30"

nvram set sig_state_update=0 # INITIALIZING
nvram set sig_state_flag=0   # 0: Don't do upgrade  1: Do upgrade	
nvram set sig_state_error=0
#nvram set sig_state_url=""

IS_SUPPORT_NOTIFICATION_CENTER=`nvram get rc_support|grep -i nt_center`
if [ "$IS_SUPPORT_NOTIFICATION_CENTER" != "" ]; then
. /tmp/nc/event.conf
fi

# current signature information
current_sig_ver=`nvram get bwdpi_sig_ver`
current_sig_ver=`echo $current_sig_ver | sed s/'\.'//g;`

# get signature information
forsq=`nvram get apps_sq`
if [ -z "$forsq" ]; then
	forsq=0
fi

wl0_support=`nvram show | grep rc_support | grep 2.4G`

if [ "$wl0_support" != "" ]; then
	country_code=`nvram get wl0_country_code`
else
	country_code=`nvram get wl1_country_code`
fi

echo "---- sig update start: ----" > /tmp/sig_upgrade.log
sig_ver=""
model=`nvram get productid`
if [ "$forsq" == "1" ]; then
	echo "---- sig update sq normal----" >> /tmp/sig_upgrade.log
	wget $wget_options http://dlcdnet.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/sig_update.zip -O /tmp/sig_update.txt		
else
	echo "---- sig update real normal----" >> /tmp/sig_upgrade.log
	wget $wget_options http://dlcdnet.asus.com/pub/ASUS/LiveUpdate/Release/Wireless/sig_update.zip -O /tmp/sig_update.txt
fi
dlinfo="$?"
echo "---- sig wget exit: $dlinfo ----" >> /tmp/sig_upgrade.log

if [ "$dlinfo" != "0" ]; then
	echo "---- sig wget failure ----" >> /tmp/sig_upgrade.log
	nvram set sig_state_error=1
else
	# TODO get and parse information
	sig_ver=`grep $country_code /tmp/sig_update.txt | sed s/.*#//;`
	if [ "$sig_ver" == "" ]; then
		sig_ver=`grep WW /tmp/sig_update.txt | sed s/.*#//;`
		nvram set SKU="WW";
	else
		nvram set SKU=$country_code;
	fi
	sig_ver=`echo $sig_ver | sed s/'\.'//g;`
	nvram set sig_state_info=${sig_ver}
	#urlpath=`grep $model /tmp/sig_update.txt | sed s/.*#URL//;`
	#urlpath=`echo $urlpath | sed s/#.*//;`
	#echo $urlpath
	#nvram set sig_state_url=${urlpath}
	rm -f /tmp/sig_update.*
fi

update_sig_state_info=`nvram get sig_state_info`
last_sig_state_info=`nvram get sig_last_info`

echo "---- current sig : $current_sig_ver ---" >> /tmp/sig_upgrade.log
echo "---- latest sig : $sig_ver ----" >> /tmp/sig_upgrade.log
if [ "$sig_ver" == "" ]; then
	echo "---- parse no info ---" >> /tmp/sig_upgrade.log
	nvram set sig_state_error=1	# exist no Info
else
	if [ "$current_sig_ver" -lt "$sig_ver" ]; then
		echo "---- < sig_ver, Do upgrade ----" >> /tmp/sig_upgrade.log
		nvram set sig_state_flag=1	# Do upgrade
		if [ "$IS_SUPPORT_NOTIFICATION_CENTER" != "" ]; then
			if [ "$last_sig_state_info" != "$update_sig_state_info" ]; then
				Notify_Event2NC "$SYS_NEW_SIGNATURE_UPDATED_EVENT" "{\"fw_ver\":\"$update_sig_state_info\"}"	#Send Event to Notification Center
				nvram set sig_last_info="$update_webs_state_info"
			fi
		fi
	fi	
fi

echo "---- sig update end ----" >> /tmp/sig_upgrade.log
nvram set sig_state_update=1
