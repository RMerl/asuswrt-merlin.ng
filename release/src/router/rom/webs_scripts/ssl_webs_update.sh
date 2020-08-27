#!/bin/sh

wget_options="-q -t 2 -T 30 --no-check-certificate"

dl_path_SQ="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ"
dl_path_SQ_beta="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/app"
dl_path_info="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless"
dl_path_file="https://dlcdnets.asus.com/pub/ASUS/wireless/ASUSWRT"

nvram set webs_state_update=0 # INITIALIZING
nvram set webs_state_flag=0   # 0: Do not do upgrade  1: New firmeware available  2: Do Force Upgrade
nvram set webs_state_error=0
nvram set webs_state_odm=0
nvram set webs_state_url=""
nvram set webs_state_info=""
nvram set webs_state_REQinfo=""

#nt_center
IS_SUPPORT_NOTIFICATION_CENTER=`nvram get rc_support|grep -i nt_center`
if [ "$IS_SUPPORT_NOTIFICATION_CENTER" != "" ]; then
. /tmp/nc/event.conf
fi

#support FORCE_AUTO_UPGRADE
IS_FUPGRADE=`nvram get rc_support|grep -i fupgrade`
# current firmware information
current_firm=`nvram get firmver`
current_firm=`echo $current_firm | sed s/'\.'//g;`
current_firm_1st_bit=${current_firm:0:1}	#To see v7 fw as general v3 fw, we replace 7.x.x.x with 3.x.x.x
if [ "$current_firm_1st_bit" == "7" ]; then
	current_firm=`echo $current_firm |sed s/7/3/1;`
fi
current_buildno=`nvram get buildno`
current_extendno=`nvram get extendno`
current_extendno=`echo $current_extendno | sed s/-g.*//;`

# get firmware information
forsq=`nvram get apps_sq`
if [ -z "$forsq" ]; then
	forsq=0
fi

model=`nvram get productid`
model="$model#"
odmpid=`nvram get odmpid`
odmpid="$odmpid#"

# change info path
model_31="0"
model_30="0"
if [ "$model" == "RT-N18U#" ]; then
	model_31="1"
elif [ "$model" == "RT-AX92U#" ] || [ "$model" == "RT-N11P_B1#" ]; then
	model_30="1"	#Use another info after middle firmware
fi

echo "---- To download control file, Start ----" > /tmp/webs_upgrade.log

dlinfo=0
if [ "$forsq" -ge 2 ] && [ "$forsq" -le 9 ]; then
	echo "---- update SQ beta_user ${dl_path_SQ_beta}${forsq}/wlan_update_beta${forsq}.zip ----" >> /tmp/webs_upgrade.log
	wget $wget_options ${dl_path_SQ_beta}${forsq}/wlan_update_beta${forsq}.zip -O /tmp/wlan_update.txt
elif [ "$forsq" == "1" ]; then
	if [ "$model_31" == "1" ]; then
		echo "---- update SQ for model_31 ${dl_path_SQ}/wlan_update_31.zip ----" >> /tmp/webs_upgrade.log
		wget $wget_options ${dl_path_SQ}/wlan_update_31.zip -O /tmp/wlan_update.txt
	elif [ "$model_30" == "1" ]; then
		echo "---- update SQ for model_30 ${dl_path_SQ}/wlan_update_30.zip ----" >> /tmp/webs_upgrade.log
		wget $wget_options ${dl_path_SQ}/wlan_update_30.zip -O /tmp/wlan_update.txt
	else
		echo "---- update SQ for general ${dl_path_SQ}/wlan_update_v2.zip ----" >> /tmp/webs_upgrade.log
		wget $wget_options ${dl_path_SQ}/wlan_update_v2.zip -O /tmp/wlan_update.txt
	fi
else
	if [ "$model_31" == "1" ]; then
		echo "---- update dl_path_info for model_31 ${dl_path_info}/wlan_update_31.zip ----" >> /tmp/webs_upgrade.log
		wget $wget_options ${dl_path_info}/wlan_update_31.zip -O /tmp/wlan_update.txt
	elif [ "$model_30" == "1" ]; then
		echo "---- update dl_path_info for model_30  ${dl_path_info}/wlan_update_30.zip ----" >> /tmp/webs_upgrade.log
		wget $wget_options ${dl_path_info}/wlan_update_30.zip -O /tmp/wlan_update.txt
	else
		echo "---- update dl_path_info for general ${dl_path_info}/wlan_update_v2.zip ----" >> /tmp/webs_upgrade.log
		wget $wget_options ${dl_path_info}/wlan_update_v2.zip -O /tmp/wlan_update.txt
	fi
fi	
dlinfo=$?
echo "---- [LiveUpdate] wget ctrl file, exit code: ${dlinfo} ----" >> /tmp/webs_upgrade.log

if [ "$dlinfo" != "0" ]; then
	echo "---- download ctrl failure ----" >> /tmp/webs_upgrade.log
	nvram set webs_state_error=1
else
	# parse latest information
	firmver=`grep $model /tmp/wlan_update.txt | sed s/.*#FW//;`
	firmver=`echo $firmver | sed s/#.*//;`
	buildno=`echo $firmver | sed s/....//;`
	firmver=`echo $firmver | sed s/$buildno$//;`
	extendno=`grep $model /tmp/wlan_update.txt | sed s/.*#EXT//;`
	extendno=`echo $extendno | sed s/#.*//;`
	lextendno=`echo $extendno | sed s/-g.*//;`
	# parse Requested information
	REQfirmver=`grep $model /tmp/wlan_update.txt | sed s/.*#REQFW//;`
	REQfirmver=`echo $REQfirmver | sed s/#.*//;`
	REQbuildno=`echo $REQfirmver | sed s/....//;`
	REQfirmver=`echo $REQfirmver | sed s/$REQbuildno$//;`
	REQextendno=`grep $model /tmp/wlan_update.txt | sed s/.*#REQEXT//;`
	REQextendno=`echo $REQextendno | sed s/#.*//;`
	REQlextendno=`echo $REQextendno | sed s/-g.*//;`

	# webs_state_info for odmpid
	if [ "$odmpid" != "#" ]; then
		# parse latest information
		firmver_odmpid=`grep $odmpid /tmp/wlan_update.txt | sed s/.*#FW//;`
		firmver_odmpid=`echo $firmver_odmpid | sed s/#.*//;`
		buildno_odmpid=`echo $firmver_odmpid | sed s/....//;`
		firmver_odmpid=`echo $firmver_odmpid | sed s/$buildno_odmpid$//;`
		extendno_odmpid=`grep $odmpid /tmp/wlan_update.txt | sed s/.*#EXT//;`
		extendno_odmpid=`echo $extendno_odmpid | sed s/#.*//;`
		lextendno_odmpid=`echo $extendno_odmpid | sed s/-g.*//;`
		# parse Requested information
		REQfirmver_odmpid=`grep $odmpid /tmp/wlan_update.txt | sed s/.*#REQFW//;`
		REQfirmver_odmpid=`echo $REQfirmver_odmpid | sed s/#.*//;`
		REQbuildno_odmpid=`echo $REQfirmver_odmpid | sed s/....//;`
		REQfirmver_odmpid=`echo $REQfirmver_odmpid | sed s/$REQbuildno_odmpid$//;`
		REQextendno_odmpid=`grep $odmpid /tmp/wlan_update.txt | sed s/.*#REQEXT//;`
		REQextendno_odmpid=`echo $REQextendno_odmpid | sed s/#.*//;`
		REQlextendno_odmpid=`echo $REQextendno_odmpid | sed s/-g.*//;`
	else
		firmver_odmpid=""
		buildno_odmpid=""
		lextendno_odmpid=""
		REQfirmver_odmpid=""
		REQbuildno_odmpid=""
		REQlextendno_odmpid=""
	fi
	

	if [ "$firmver_odmpid" == "" ] || [ "$buildno_odmpid" == "" ] || [ "$lextendno_odmpid" == "" ]; then
		nvram set webs_state_info=${firmver}_${buildno}_${extendno}
		nvram set webs_state_REQinfo=${REQfirmver}_${REQbuildno}_${REQextendno}
	else
		nvram set webs_state_info=${firmver_odmpid}_${buildno_odmpid}_${extendno_odmpid}
		nvram set webs_state_REQinfo=${REQfirmver_odmpid}_${REQbuildno_odmpid}_${REQextendno_odmpid}
		firmver="$firmver_odmpid"
		buildno="$buildno_odmpid"
		lextendno="$lextendno_odmpid"
		REQfirmver="$REQfirmver_odmpid"
		REQbuildno="$REQbuildno_odmpid"
		REQlextendno="$REQlextendno_odmpid"
		nvram set webs_state_odm=1		# with Live Update odmpid sku
		echo "---- webs_state_odm=${webs_state_odm} ----" >> /tmp/webs_upgrade.log
	fi

	urlpath=`grep $model /tmp/wlan_update.txt | sed s/.*#URL//;`
	urlpath=`echo $urlpath | sed s/#.*//;`
	urlpath_odmpid=`grep $odmpid /tmp/wlan_update.txt | sed s/.*#URL//;`
	urlpath_odmpid=`echo $urlpath_odmpid | sed s/#.*//;`
	if [ "$urlpath_odmpid" == "" ]; then
		nvram set webs_state_url=${urlpath}
	else
		nvram set webs_state_url=${urlpath_odmpid}
		urlpath="$urlpath_odmpid"
	fi

	
	odmpid_support=`nvram get webs_state_odm`
	rm -f /tmp/wlan_update.*
fi

echo "---- current version : $current_firm $current_buildno $current_extendno----" >> /tmp/webs_upgrade.log
echo "---- odmpid : $firmver_odmpid $buildno_odmpid $lextendno_odmpid ----" >> /tmp/webs_upgrade.log
echo "---- productid : $firmver $buildno $lextendno ----" >> /tmp/webs_upgrade.log
echo "---- REQodmpid : $REQfirmver_odmpid $REQbuildno_odmpid $REQlextendno_odmpid ----" >> /tmp/webs_upgrade.log
echo "---- REQproductid : $REQfirmver $REQbuildno $REQlextendno ----" >> /tmp/webs_upgrade.log

update_webs_state_info=`nvram get webs_state_info`
last_webs_state_info=`nvram get webs_last_info` 
if [ "$firmver" == "" ] || [ "$buildno" == "" ] || [ "$lextendno" == "" ]; then
	if [ "$forsq" == "8" ]; then
		echo `date "+%Y-%m-%d %H:%M:%S"`"---- parsing ctrl info failure ----" >> /jffs/KEEP_UPGRADE_DEBUG.log
	fi
	nvram set webs_state_error=1	# exist no Info
else
	if [ -n "$IS_FUPGRADE" ]; then
		if [ -n "$REQbuildno" ] && [ -n "$REQfirmver" ] && [ -n "$REQlextendno" ]; then
			if [ "$current_buildno" -lt "$REQbuildno" ]; then
	    	   	nvram set webs_state_flag=2 # Do force Upgrade
				echo "---- < REQbuildno ----" >> /tmp/webs_upgrade.log
				if [ "$IS_SUPPORT_NOTIFICATION_CENTER" != "" ]; then
					if [ "$last_webs_state_info" != "$update_webs_state_info" ]; then
	        	   	    Notify_Event2NC "$SYS_FW_NWE_VERSION_AVAILABLE_EVENT" "{\"fw_ver\":\"$update_webs_state_info\"}"    #Send Event to Notification Center
						nvram set webs_last_info="$update_webs_state_info"
		   	        fi
				fi
			elif [ "$current_buildno" -eq "$REQbuildno" ]; then
				if [ "$current_firm" -lt "$REQfirmver" ]; then
					nvram set webs_state_flag=2 # Do Force Upgrade
					echo "---- < REQfirmver ----" >> /tmp/webs_upgrade.log
					if [ "$IS_SUPPORT_NOTIFICATION_CENTER" != "" ]; then
						if [ "$last_webs_state_info" != "$update_webs_state_info" ]; then
							Notify_Event2NC "$SYS_FW_NWE_VERSION_AVAILABLE_EVENT" "{\"fw_ver\":\"$update_webs_state_info\"}"    #Send Event to Notification Center
							nvram set webs_last_info="$update_webs_state_info"
						fi
					fi
				elif [ "$current_firm" -eq "$REQfirmver" ]; then
					if [ "$current_extendno" -lt "$REQlextendno" ]; then
						nvram set webs_state_flag=2 # Do Force Upgrade
						echo "---- < REQlextendno ----" >> /tmp/webs_upgrade.log
						if [ "$IS_SUPPORT_NOTIFICATION_CENTER" != "" ]; then
							if [ "$last_webs_state_info" != "$update_webs_state_info" ]; then
								Notify_Event2NC "$SYS_FW_NWE_VERSION_AVAILABLE_EVENT" "{\"fw_ver\":\"$update_webs_state_info\"}"    #Send Event to Notification Center
								nvram set webs_last_info="$update_webs_state_info"
							fi
						fi
					fi
				fi
			fi
		else
			echo "---- Can't compare REQFW ----" >> /tmp/webs_upgrade.log
		fi
	fi	

	tmp_webs_state_flag=`nvram get webs_state_flag`
	if [ "$tmp_webs_state_flag" == "0" ]; then	##compare to webs_state_info, because DUT fwv > webs_state_REQinfo
		if [ -n "$buildno" ] && [ -n "$firmver" ] && [ -n "$lextendno" ]; then
			if [ "$current_buildno" -lt "$buildno" ]; then
				nvram set webs_state_flag=1 # Do upgrade
				echo "---- < buildno ----" >> /tmp/webs_upgrade.log
				if [ "$IS_SUPPORT_NOTIFICATION_CENTER" != "" ]; then
					if [ "$last_webs_state_info" != "$update_webs_state_info" ]; then
						Notify_Event2NC "$SYS_FW_NWE_VERSION_AVAILABLE_EVENT" "{\"fw_ver\":\"$update_webs_state_info\"}"    #Send Event to Notification Center
						nvram set webs_last_info="$update_webs_state_info"
					fi
				fi
			elif [ "$current_buildno" -eq "$buildno" ]; then
				if [ "$current_firm" -lt "$firmver" ]; then
					nvram set webs_state_flag=1 # Do upgrade
					echo "---- < firmver ----" >> /tmp/webs_upgrade.log
					if [ "$IS_SUPPORT_NOTIFICATION_CENTER" != "" ]; then
						if [ "$last_webs_state_info" != "$update_webs_state_info" ]; then
							Notify_Event2NC "$SYS_FW_NWE_VERSION_AVAILABLE_EVENT" "{\"fw_ver\":\"$update_webs_state_info\"}"    #Send Event to Notification Center
							nvram set webs_last_info="$update_webs_state_info"
						fi
					fi
				elif [ "$current_firm" -eq "$firmver" ]; then
					if [ "$current_extendno" -lt "$lextendno" ]; then
						nvram set webs_state_flag=1 # Do upgrade
						echo "---- < lextendno ----" >> /tmp/webs_upgrade.log
						if [ "$IS_SUPPORT_NOTIFICATION_CENTER" != "" ]; then
							if [ "$last_webs_state_info" != "$update_webs_state_info" ]; then
								Notify_Event2NC "$SYS_FW_NWE_VERSION_AVAILABLE_EVENT" "{\"fw_ver\":\"$update_webs_state_info\"}"    #Send Event to Notification Center
								nvram set webs_last_info="$update_webs_state_info"
							fi
						fi
					fi
				fi
			fi
		else
			echo "---- Can't compare FW ----" >> /tmp/webs_upgrade.log
		fi
	fi

fi

if [ "$forsq" == "8" ]; then
	echo `date "+%Y-%m-%d %H:%M:%S"`"---- webs_state_flag=`nvram get webs_state_flag` ----" >> /jffs/KEEP_UPGRADE_DEBUG.log
fi

# download releasee note
get_productid=`nvram get productid`
if [ "$odmpid_support" == "1" ]; then
	get_productid=`nvram get odmpid`
fi
get_productid=`echo $get_productid | sed s/+/plus/;`	#replace 'plus' to '+' for one time
get_preferred_lang=`nvram get preferred_lang`
LANG="$get_preferred_lang"

releasenote_file0=`echo $get_productid`_`nvram get webs_state_info`_"$LANG"_note.zip
releasenote_file0_US=`echo $get_productid`_`nvram get webs_state_info`_US_note.zip
releasenote_path0="/tmp/release_note0.txt"

dlinfo2=""
dlinfo3=""
if [ "$forsq" -ge 2 ] && [ "$forsq" -le 9 ]; then
	echo "---- download SQ beta_user release note ${dl_path_SQ_beta}${forsq}/$releasenote_file0 ----" >> /tmp/webs_upgrade.log
	wget $wget_options ${dl_path_SQ_beta}${forsq}/$releasenote_file0 -O $releasenote_path0
	dlinfo2=$?
	echo "---- [LiveUpdate] wget pLang release note, exit code: ${dlinfo2} ----" >> /tmp/webs_upgrade.log
	if [ "$dlinfo2" != "0" ]; then
		echo "---- download SQ beta_user release note ${dl_path_SQ_beta}${forsq}/$releasenote_file0_US ----" >> /tmp/webs_upgrade.log
		wget $wget_options ${dl_path_SQ_beta}${forsq}/$releasenote_file0_US -O $releasenote_path0
		dlinfo3=$?
		echo "---- [LiveUpdate] wget US release note, exit code: ${dlinfo3} ----" >> /tmp/webs_upgrade.log
	fi
elif [ "$forsq" == "1" ]; then
	echo "---- download SQ release note ${dl_path_SQ}/$releasenote_file0 ----" >> /tmp/webs_upgrade.log
	wget $wget_options ${dl_path_SQ}/$releasenote_file0 -O $releasenote_path0
	dlinfo2=$?
	echo "---- [LiveUpdate] wget pLang release note, exit code: ${dlinfo2} ----" >> /tmp/webs_upgrade.log
	if [ "$dlinfo2" != "0" ]; then
		echo "---- download SQ release note ${dl_path_SQ}/$releasenote_file0_US ----" >> /tmp/webs_upgrade.log
		wget $wget_options ${dl_path_SQ}/$releasenote_file0_US -O $releasenote_path0
		dlinfo3=$?
		echo "---- [LiveUpdate] wget US release note, exit code: ${dlinfo3} ----" >> /tmp/webs_upgrade.log
	fi
else
	echo "---- download real release note ${dl_path_file}/$releasenote_file0 ----" >> /tmp/webs_upgrade.log
	wget $wget_options ${dl_path_file}/$releasenote_file0 -O $releasenote_path0
	dlinfo2=$?
	echo "---- [LiveUpdate] wget pLang release note, exit code: ${dlinfo2} ----" >> /tmp/webs_upgrade.log
	if [ "$dlinfo2" != "0" ]; then
		echo "---- download real release note ${dl_path_file}/$releasenote_file0_US ----" >> /tmp/webs_upgrade.log
		wget $wget_options ${dl_path_file}/$releasenote_file0_US -O $releasenote_path0
		dlinfo3=$?
		echo "---- [LiveUpdate] wget US release note, exit code: ${dlinfo3} ----" >> /tmp/webs_upgrade.log
	fi
fi

echo "---- To download control file, End ----" >> /tmp/webs_upgrade.log
nvram set webs_state_update=1
