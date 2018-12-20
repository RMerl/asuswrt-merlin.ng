#!/bin/sh

wget_timeout=`nvram get apps_wget_timeout`
wget_options="-q -t 2 -T $wget_timeout --no-check-certificate"

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
echo "$current_sig_ver"
echo "$current_sig_ver" > /tmp/sig_upgrade.log

#get FULL / PARTIAL / Lite signature: FULL/PART/WRS @update_sig_type() iqos.c
sig_type=`nvram get sig_type`
if [ "$sig_type" == "" ]; then
	sig_type="FULL";
fi

#for SQ test
forsq=`nvram get apps_sq`

# get signature information
tcode=`nvram get territory_code`
if [ "$tcode" == "" ]; then
	territory_type="WW"_"$sig_type"
else
	territory_type="$tcode"_"$sig_type"
fi

if [ "$forsq" == "1" ]; then
	echo "---- sig update sq normal----" >> /tmp/sig_upgrade.log
	wget $wget_options https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/sig2nd_update.zip -O /tmp/sig_update.txt		
else
	echo "---- sig update real normal----" >> /tmp/sig_upgrade.log
	wget $wget_options https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless/sig2nd_update.zip -O /tmp/sig_update.txt
fi
r=$?

if [ "$r" != "0" ]; then
	echo "Download sig info Failure, return_code : $r"
	echo "Download sig info Failure, return_code : $r" >> /tmp/sig_upgrade.log
	nvram set sig_state_error=1
else
	echo "Download sig info OK"
	echo "Download sig info OK" >> /tmp/sig_upgrade.log
	# TODO get and parse information
	sig_ver=`grep $territory_type /tmp/sig_update.txt | sed s/.*#//;`
	if [ "$sig_ver" == "" ]; then
		WW_type="WW"_$sig_type
		sig_ver=`grep $WW_type /tmp/sig_update.txt | sed s/.*#//;`
		nvram set SKU="WW"_"$sig_type"
	else
		territory_type=`echo $territory_type | sed s/'\/'/'\_'/g;`
		nvram set SKU="$territory_type"
	fi
	echo `nvram get SKU`
	echo `nvram get SKU` >> /tmp/sig_upgrade.log
	echo $sig_ver
	echo $sig_ver >> /tmp/sig_upgrade.log
	
	sig_ver=`echo $sig_ver | sed s/'\.'//g;`
	nvram set sig_state_info=${sig_ver}
	rm -f /tmp/sig_update.*
fi

update_sig_state_info=`nvram get sig_state_info`
last_sig_state_info=`nvram get sig_last_info`

if [ "$sig_ver" == "" ]; then
	nvram set sig_state_error=1	# exist no Info
else
	if [ "$current_sig_ver" -lt "$sig_ver" ]; then
		echo "---- sig_ver: $sig_ver ----"
		echo "---- sig_ver: $sig_ver ----" >> /tmp/sig_upgrade.log
		nvram set sig_state_flag=1	# Do upgrade
		if [ "$IS_SUPPORT_NOTIFICATION_CENTER" != "" ]; then
			if [ "$last_sig_state_info" != "$update_sig_state_info" ]; then
				echo "---- The 1st time to detect new sig ver available ----"
				echo "---- The 1st time to detect new sig ver available ----" >> /tmp/sig_upgrade.log
				Notify_Event2NC "$SYS_NEW_SIGNATURE_UPDATED_EVENT" "{\"fw_ver\":\"$update_sig_state_info\"}"	#Send Event to Notification Center
				nvram set sig_last_info="$update_sig_state_info"
			fi
		fi
	fi	
fi

nvram set sig_state_update=1
