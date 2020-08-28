#!/bin/sh

wget_options="-q -t 2 -T 30"

fwsite="https://fwupdate.asuswrt-merlin.net"

nvram set webs_state_update=0 # INITIALIZING
nvram set webs_state_flag=0   # 0: Don't do upgrade  1: Do upgrade
nvram set webs_state_error=0
nvram set webs_state_url=""

#openssl support rsa check
IS_SUPPORT_NOTIFICATION_CENTER=$(nvram get rc_support|grep -i nt_center)
if [ "$IS_SUPPORT_NOTIFICATION_CENTER" != "" ]; then
. /tmp/nc/event.conf
fi
# current firmware information
current_firm=$(nvram get buildno | cut -d. -f1)
current_buildno=$(nvram get buildno | cut -d. -f2)
current_extendno=$(nvram get extendno)

# Overload extendno: alpha is 11-19, beta is 51-59, release is 100-109.
current_extendno=$(echo $current_extendno | sed s/-g.*//;)
current_extendno=$(echo $current_extendno | sed "s/^[0-9]$/10&/;")
current_extendno=$(echo $current_extendno | sed s/^alpha/1/;)
current_extendno=$(echo $current_extendno | sed s/^beta/5/;)

# get firmware information
forsq=$(nvram get apps_sq)
model=$(nvram get productid)
model="$model#"

if [ "$forsq" == "1" ]; then
	echo "---- update sq normal----" > /tmp/webs_upgrade.log
	/usr/sbin/wget $wget_options $fwsite/test/manifest.txt -O /tmp/wlan_update.txt
else
	echo "---- update real normal----" > /tmp/webs_upgrade.log
	/usr/sbin/wget $wget_options $fwsite/manifest.txt -O /tmp/wlan_update.txt
fi

if [ "$?" != "0" ]; then
	nvram set webs_state_error=1
else

	fullver=$(grep $model /tmp/wlan_update.txt | sed s/.*#FW//;)
	fullver=$(echo $fullver | sed s/#.*//;)
	firmver=$(echo $fullver | cut -d. -f1)
	buildno=$(echo $fullver | cut -d. -f2)
	extendno=$(grep $model /tmp/wlan_update.txt | sed s/.*#EXT//;)
	extendno=$(echo $extendno | sed s/#.*//;)
	lextendno=$(echo $extendno | sed s/-g.*//;)
 	lextendno=$(echo $lextendno | sed "s/^[0-9]$/10&/;")
	nvram set webs_state_info=3004_${firmver}_${buildno}_${extendno}
	nvram set webs_state_info_am=${firmver}_${buildno}_${extendno}

	rm -f /tmp/wlan_update.*
fi

echo "---- Have ${current_firm}.${current_buildno}_${current_extendno}----" >> /tmp/webs_upgrade.log
echo "---- Stable available ${firmver}.${buildno}_${extendno}----" >> /tmp/webs_upgrade.log

update_webs_state_info=$(nvram get webs_state_info)
last_webs_state_info=$(nvram get webs_last_info)
if [ "$firmver" == "" ] || [ "$buildno" == "" ] || [ "$lextendno" == "" ]; then
	nvram set webs_state_error=1	# exist no Info
else
	if [ "$current_firm" -lt "$firmver" ]; then
		echo "---- firmver: $firmver ----" >> /tmp/webs_upgrade.log
		nvram set webs_state_flag=1	# Do upgrade
		if [ "$IS_SUPPORT_NOTIFICATION_CENTER" != "" ]; then
			if [ "$last_webs_state_info" != "$update_webs_state_info" ]; then
				Notify_Event2NC "$SYS_FW_NWE_VERSION_AVAILABLE_EVENT" "{\"fw_ver\":\"$update_webs_state_info\"}"
				nvram set webs_last_info="$update_webs_state_info"
			fi
		fi
	elif [ "$current_firm" -eq "$firmver" ]; then
		if [ "$current_buildno" -lt "$buildno" ]; then
				echo "---- buildno: $buildno ----" >> /tmp/webs_upgrade.log
				nvram set webs_state_flag=1	# Do upgrade
				if [ "$IS_SUPPORT_NOTIFICATION_CENTER" != "" ]; then
					if [ "$last_webs_state_info" != "$update_webs_state_info" ]; then
						Notify_Event2NC "$SYS_FW_NWE_VERSION_AVAILABLE_EVENT" "{\"fw_ver\":\"$update_webs_state_info\"}"
						nvram set webs_last_info="$update_webs_state_info"
					fi
				fi
		elif [ "$current_buildno" -eq "$buildno" ]; then
			if [ "$current_extendno" -lt "$lextendno" ]; then
				echo "---- lextendno: $lextendno ----" >> /tmp/webs_upgrade.log
				nvram set webs_state_flag=1	# Do upgrade
				if [ "$IS_SUPPORT_NOTIFICATION_CENTER" != "" ]; then
					if [ "$last_webs_state_info" != "$update_webs_state_info" ]; then
						Notify_Event2NC "$SYS_FW_NWE_VERSION_AVAILABLE_EVENT" "{\"fw_ver\":\"$update_webs_state_info\"}"
						nvram set webs_last_info="$update_webs_state_info"
					fi
				fi
			fi
		fi
	fi
fi

# download releasee note
webs_state_flag=$(nvram get webs_state_flag)
get_productid=$(nvram get productid)
get_productid=$(echo $get_productid | sed s/+/plus/;)	#replace 'plus' to '+' for one time
get_preferred_lang=$(nvram get preferred_lang)

if [ "$webs_state_flag" -eq "1" ]; then
	releasenote_file0_US=$(nvram get webs_state_info_am)_note.txt
	releasenote_path0="/tmp/release_note0.txt"
	if [ "$forsq" == "1" ]; then
		echo "---- download SQ release note $fwsite/test/$releasenote_file0_US ----" >> /tmp/webs_upgrade.log
		/usr/sbin/wget $wget_options $fwsite/test/$releasenote_file0_US -O $releasenote_path0
		echo "---- $fwsite/test/$releasenote_file0 ----" >> /tmp/webs_upgrade.log
	else
		echo "---- download real release note ----" >> /tmp/webs_upgrade.log
		/usr/sbin/wget $wget_options $fwsite/$releasenote_file0_US -O $releasenote_path0
		echo "---- $fwsite/$releasenote_file0 ----" >> /tmp/webs_upgrade.log
	fi

	if [ "$?" != "0" ]; then
		echo "---- download SQ release note failed ----" >> /tmp/webs_upgrade.log
		nvram set webs_state_error=1
	fi
fi

nvram set webs_state_update=1
