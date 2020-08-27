#!/bin/sh

IS_BCMHND=`nvram get rc_support|grep -i bcmhnd`

wget_options="-q -t 2 -T 30 --no-check-certificate"

dl_path_SQ="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ"
dl_path_SQ_beta="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/app"
dl_path_file="https://dlcdnets.asus.com/pub/ASUS/wireless/ASUSWRT"

echo "---- To download fw/rsa, Start ----" > /tmp/webs_upgrade.log
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


# get specific model
fw_check=`nvram get fw_check`
model=`nvram get productid`
if [ "$model" == "RT-AC68U" ] && [ "$fw_check" == "1" ]; then
	echo "---- TM model restore ----" >> /tmp/webs_upgrade.log
	fw_check
else

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
firmware_rsasign=`echo $get_productid`_`nvram get webs_state_info`_rsa`nvram get live_update_rsa_ver`.zip

#for small size fw to increase free
small_fw_update=`nvram show | grep rc_support | grep small_fw`

if [ "$small_fw_update" != "" ]; then
	echo "---- small_fw_update path ----" >> /tmp/webs_upgrade.log
	mkdir /tmp/mytmpfs
	mount -t tmpfs -o size=16M,nr_inodes=10k,mode=700 tmpfs /tmp/mytmpfs
	firmware_path="/tmp/mytmpfs/linux.trx"
	rc rc_service stop_upgrade
else
	firmware_path="/tmp/linux.trx"
fi
rsa_path=/tmp/rsasign.bin

#kill old files
rm -f $firmware_path
rm -f $rsa_path

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

wget_result=0
wget_result2=0
if [ "$update_url" != "" ]; then
	echo "---- wget fw nvram webs_state_url ${update_url}/$firmware_file ----" >> /tmp/webs_upgrade.log
	wget -t 2 -T 30 --no-check-certificate --output-file=/tmp/fwget_log ${update_url}/$firmware_file -O $firmware_path
	wget_result=$?
	echo "---- wget fw nvram webs_state_url, exit code: ${wget_result} ----" >> /tmp/webs_upgrade.log

	echo "---- wget rsa nvram webs_state_url ${update_url}/$firmware_rsasign ----" >> /tmp/webs_upgrade.log
	wget $wget_options ${update_url}/$firmware_rsasign -O $rsa_path
	wget_result2=$?
	echo "---- wget rsa nvram webs_state_url, exit code: ${wget_result2} ----" >> /tmp/webs_upgrade.log

elif [ "$forsq" -ge 2 ] && [ "$forsq" -le 9 ]; then
	echo "---- wget fw sq beta_user ${dl_path_SQ_beta}${forsq}/$firmware_file ----" >> /tmp/webs_upgrade.log
	wget -t 2 -T 30 --no-check-certificate --output-file=/tmp/fwget_log ${dl_path_SQ_beta}${forsq}/$firmware_file -O $firmware_path
	wget_result=$?
	echo "---- [LiveUpdate] wget fw, exit code: ${wget_result} ----" >> /tmp/webs_upgrade.log

	echo "---- wget fw sq beta_user ${dl_path_SQ_beta}${forsq}/$firmware_rsasign ----" >> /tmp/webs_upgrade.log
	wget $wget_options ${dl_path_SQ_beta}${forsq}/$firmware_rsasign -O $rsa_path
	wget_result2=$?
	echo "---- [LiveUpdate] wget rsa, exit code: ${wget_result2} ----" >> /tmp/webs_upgrade.log

elif [ "$forsq" == "1" ]; then
	echo "---- wget fw sq ${dl_path_SQ}/$firmware_file ----" >> /tmp/webs_upgrade.log
	wget -t 2 -T 30 --no-check-certificate --output-file=/tmp/fwget_log ${dl_path_SQ}/$firmware_file -O $firmware_path
	wget_result=$?
	echo "---- [LiveUpdate] wget fw, exit code: ${wget_result} ----" >> /tmp/webs_upgrade.log

	echo "---- wget fw sq ${dl_path_SQ}/$firmware_rsasign ----" >> /tmp/webs_upgrade.log
	wget $wget_options ${dl_path_SQ}/$firmware_rsasign -O $rsa_path
	wget_result2=$?
	echo "---- [LiveUpdate] wget rsa, exit code: ${wget_result2} ----" >> /tmp/webs_upgrade.log

elif [ "$urlpath" == "" ]; then
	echo "---- wget fw Real ${dl_path_file}/$firmware_file ----" >> /tmp/webs_upgrade.log
	wget -t 2 -T 30 --no-check-certificate --output-file=/tmp/fwget_log ${dl_path_file}/$firmware_file -O $firmware_path
	wget_result=$?
	echo "---- [LiveUpdate] wget fw, exit code: ${wget_result} ----" >> /tmp/webs_upgrade.log

	echo "---- wget fw Real ${dl_path_file}/$firmware_rsasign ----" >> /tmp/webs_upgrade.log
	wget $wget_options ${dl_path_file}/$firmware_rsasign -O $rsa_path
	wget_result2=$?
	echo "---- [LiveUpdate] wget rsa, exit code: ${wget_result2} ----" >> /tmp/webs_upgrade.log

else
	echo "---- wget fw URL ----" >> /tmp/webs_upgrade.log
	wget -t 2 -T 30 --no-check-certificate --output-file=/tmp/fwget_log $urlpath/$firmware_file -O $firmware_path
	wget_result=$?
	echo "---- [LiveUpdate] wget fw, exit code: ${wget_result} ----" >> /tmp/webs_upgrade.log

	echo "---- [LiveUpdate] wget rsa URL ----" >> /tmp/webs_upgrade.log
	wget $wget_options $urlpath/$firmware_rsasign -O $rsa_path
	wget_result2=$?
	echo "---- [LiveUpdate] wget rsa, exit code: ${wget_result2} ----" >> /tmp/webs_upgrade.log

fi	

if [ "$wget_result" != "0" ]; then
	echo "---- download fw failure ----" >> /tmp/webs_upgrade.log
	rm -f $firmware_path
	nvram set $record=1	# fail to download the firmware
	if [ "$cfg_trigger" != "1" ]; then	# cfg_mnt skip
		if [ "$force_upgrade" == "1" ]; then
			webs_state_dl_error_count=$((webs_state_dl_error_count+1))
			nvram set webs_state_dl_error_count=$webs_state_dl_error_count
			nvram set webs_state_dl_error_day=$error_day
		fi
		reboot
	fi # cfg_trigger!=1
elif [ "$wget_result2" != "0" ]; then
	echo "---- download rsa failure ----" >> /tmp/webs_upgrade.log
	rm -f $firmware_path
	nvram set $record=2	# fail to download the rsa
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
	nvram set firmware_check=0
	firmware_check $firmware_path
	sleep 1

	nvram set rsasign_check=0
	rsasign_check $firmware_path
	sleep 1

	firmware_check_ret=`nvram get firmware_check`
	rsasign_check_ret=`nvram get rsasign_check`

	if [ "$firmware_check_ret" == "1" ] && [ "$rsasign_check_ret" == "1" ]; then
		echo "---- fw check OK ----" >> /tmp/webs_upgrade.log
		/sbin/ejusb -1 0
		nvram set fwpath=2
		if [ "$cfg_trigger" != "1" ]; then	# cfg_mnt skip
			nvram set auto_upgrade=0
			nvram set webs_state_dl=0
			nvram commit
			rc rc_service restart_upgrade
		fi # cfg_trigger!=1
	else
		echo "---- fw check error, CRC: ${firmware_check_ret}  rsa: ${rsasign_check_ret} ----" >> /tmp/webs_upgrade.log
		rm -f $firmware_path
		nvram set $record=3	# wrong fw
		if [ "$cfg_trigger" != "1" ]; then	# cfg_mnt skip
			if [ "$force_upgrade" == "1" ]; then
				webs_state_dl_error_count=$((webs_state_dl_error_count+1))
				nvram set webs_state_dl_error_count=$webs_state_dl_error_count
				nvram set webs_state_dl_error_day=$error_day
			fi
			reboot
		fi # cfg_trigger!=1
	fi
fi


fi # RT-AC68U fw_check

echo "---- To download fw/rsa, End ----" >> /tmp/webs_upgrade.log
nvram set webs_state_upgrade=1
nvram commit
