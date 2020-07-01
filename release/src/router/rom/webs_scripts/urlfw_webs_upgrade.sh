#!/bin/sh

IS_BCMHND=`nvram get rc_support|grep -i bcmhnd`

#wget_options="-q -t 2 -T 30 --no-check-certificate"

dl_path_SQ="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ"
dl_path_SQ_beta="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/app"
dl_path_file="https://dlcdnets.asus.com/pub/ASUS/wireless/ASUSWRT"
#wget -q -y -O- ${path}/${file} | hnd-write -

echo "---- URLFW To download fw/rsa, Start ----" > /tmp/webs_upgrade.log
nvram set webs_state_upgrade=0 # INITIALIZING
nvram set auto_upgrade=1

cfg_trigger=`echo $1`	# cfg_mnt skip
echo "---- cfg_trigger=${cfg_trigger} ----" >> /tmp/webs_upgrade.log

if [ "$cfg_trigger" != "1" ]; then # cfg_mnt skip these

	force_upgrade=`nvram get webs_state_dl`
	if [ "$force_upgrade" == "1" ]; then
		record="webs_state_dl_error"
		nvram set webs_state_dl_error=0
		nvram set webs_state_dl_error_day=
	else
		record="webs_state_error"
		nvram set webs_state_error=0
	fi

	webs_state_dl_error_count=`nvram get webs_state_dl_error_count`
	if [ -z "$webs_state_dl_error_count" ]; then
		webs_state_dl_error_count=0
	fi
	error_day=`date |awk '{print $1}'`

fi #cfg_trigger!=1

touch /tmp/update_url
update_url=`cat /tmp/update_url`
#update_url="http://192.168.123.198"

# current firmware information
productid=`nvram get productid`

get_productid=`echo $productid | sed s/+/plus/;`    #replace 'plus' to '+' for one time
odmpid=`nvram get odmpid`
get_odmpid=`echo $odmpid | sed s/+/plus/;`    #replace 'plus' to '+' for one time
odmpid_support=`nvram get webs_state_odm`
if [ "$odmpid_support" == "1" ] || [ "$odmpid_support" == "$get_odmpid" ]; then	# MIS || FRS
	get_productid=$get_odmpid
fi

# fw/rsa file
firmware_file=`echo $get_productid`_`nvram get webs_state_info`_un.zip
#firmware_rsasign=`echo $get_productid`_`nvram get webs_state_info`_rsa.zip

# for sq
forsq=`nvram get apps_sq`
if [ -z "$forsq" ]; then
	forsq=0
fi

urlpath=`nvram get webs_state_url`
if [ -z "$IS_BCMHND" ]; then
	echo "---- ! IS_BCMHND ----" >> /tmp/webs_upgrade.log
	echo 3 > /proc/sys/vm/drop_caches
fi

if [ "$cfg_trigger" == "1" ]; then	# cfg_mnt need it
	nvram set cfg_fwstatus=6
fi #cfg_trigger==1

#killall watchdog check_watchdog cfg_server

wget_result=0
if [ "$update_url" != "" ]; then
	echo "---- wget fw nvram webs_state_url ${update_url}/$firmware_file ----" >> /tmp/webs_upgrade.log
	hnd-write ${update_url}/$firmware_file
elif [ "$forsq" -ge 2 ] && [ "$forsq" -le 9 ]; then
	echo "---- wget fw sq beta_user ${dl_path_SQ_beta}${forsq}/$firmware_file ----" >> /tmp/webs_upgrade.log
	hnd-write ${dl_path_SQ_beta}${forsq}/$firmware_file
elif [ "$forsq" == "1" ]; then
	echo "---- wget fw sq ${dl_path_SQ}/$firmware_file ----" >> /tmp/webs_upgrade.log
	hnd-write ${dl_path_SQ}/$firmware_file
elif [ "$urlpath" == "" ]; then
	echo "---- wget fw Real ${dl_path_file}/$firmware_file ----" >> /tmp/webs_upgrade.log
	hnd-write ${dl_path_file}/$firmware_file
else
	echo "---- wget fw URL ----" >> /tmp/webs_upgrade.log
	hnd-write ${urlpath}/$firmware_file
fi
wget_result=$?
echo "---- [LiveUpdate] hnd-write fw, exit code: ${wget_result} ----" >> /tmp/webs_upgrade.log


hndwr_status=`nvram get hndwr`
if [ "$hndwr_status" != "99" ] && [ "$hndwr_status" != "-100" ]; then
	echo "---- download fw failure ----" >> /tmp/webs_upgrade.log
	
	nvram set $record=1	# fail to download the firmware
	if [ "$cfg_trigger" != "1" ]; then	# cfg_mnt skip
		if [ "$force_upgrade" == "1" ]; then
			webs_state_dl_error_count=$((webs_state_dl_error_count+1))
			nvram set webs_state_dl_error_count=$webs_state_dl_error_count
			nvram set webs_state_dl_error_day=$error_day
		fi
		reboot
	fi # cfg_trigger!=1
else
	nvram set webs_state_upgrade=2	
	echo "---- mv trx OK ----" >> /tmp/webs_upgrade.log

	/sbin/ejusb -1 0
	nvram set fwpath=2
	if [ "$cfg_trigger" != "1" ]; then	# cfg_mnt skip
		nvram set auto_upgrade=0
		nvram set webs_state_dl=0
		nvram commit
		reboot
		#rc rc_service restart_upgrade
	fi # cfg_trigger!=1
fi

echo "---- To download fw/rsa, End ----" >> /tmp/webs_upgrade.log
nvram set webs_state_upgrade=1
nvram commit
