#!/bin/sh

IS_BCMHND=`nvram get rc_support|grep -i bcmhnd`

wget_timeout=`nvram get apps_wget_timeout`
#wget_options="-nv -t 2 -T $wget_timeout --dns-timeout=120"
wget_options="-q -t 2 -T $wget_timeout --no-check-certificate"

dl_path_MR="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/MR"
dl_path_SQ="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ"
dl_path_SQ_beta="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/app"
dl_path_file="https://dlcdnets.asus.com/pub/ASUS/wireless/ASUSWRT"

nvram set webs_state_upgrade=0 # INITIALIZING
nvram set auto_upgrade=1

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

# get specific model
fw_check=`nvram get fw_check`
model=`nvram get productid`
if [ "$model" == "RT-AC68U" ] && [ "$fw_check" == "1" ]; then
	fw_check
else

#openssl support rsa check
rsa_enabled=`nvram show | grep rc_support | grep HTTPS`

touch /tmp/update_url
update_url=`cat /tmp/update_url`
#update_url="http://192.168.123.198"

# current firmware information
productid=`nvram get productid`
if [ "$productid" == "BLUECAVE" ]; then
       rc rc_service stop_wrs_force
fi

get_productid=`echo $productid | sed s/+/plus/;`    #replace 'plus' to '+' for one time
odmpid_support=`nvram get webs_state_odm`
if [ "$odmpid_support" == "1" ]; then
	get_productid=`nvram get odmpid`
fi

firmware_file=`echo $get_productid`_`nvram get webs_state_info`_un.zip

if [ "$rsa_enabled" != "" ]; then
	firmware_rsasign=`echo $get_productid`_`nvram get webs_state_info`_rsa.zip
fi

small_fw_update=`nvram show | grep rc_support | grep small_fw`

if [ "$small_fw_update" != "" ]; then
	mkdir /tmp/mytmpfs
	mount -t tmpfs -o size=16M,nr_inodes=10k,mode=700 tmpfs /tmp/mytmpfs
	firmware_path="/tmp/mytmpfs/linux.trx"
	rc rc_service stop_upgrade
else
	firmware_path="/tmp/linux.trx"
fi
rsa_path=/tmp/rsasign.bin

rm -f $firmware_path
wget_result=0

# get firmware zip file
formr=`nvram get MRFLAG`
forsq=`nvram get apps_sq`
urlpath=`nvram get webs_state_url`
if [ -z "$IS_BCMHND" ]; then
	echo 3 > /proc/sys/vm/drop_caches
fi
if [ "$update_url" != "" ]; then
	echo "---- wget fw nvram webs_state_url ----" > /tmp/webs_upgrade.log
	wget -t 2 -T $wget_timeout --no-check-certificate --output-file=/tmp/fwget_log ${update_url}/$firmware_file -O $firmware_path
	wget_result=$?
	if [ "$rsa_enabled" != "" ]; then
		wget $wget_options ${update_url}/$firmware_rsasign -O $rsa_path
	fi
elif [ "$formr" == "1" ]; then
	echo "---- wget fw MR1 ${dl_path_MR}1/$firmware_file ----" >> /tmp/webs_upgrade.log
	wget -t 2 -T $wget_timeout --no-check-certificate --output-file=/tmp/fwget_log ${dl_path_MR}1/$firmware_file -O $firmware_path
	wget_result=$?
	if [ "$rsa_enabled" != "" ]; then
		echo "---- wget fw MR1 ${dl_path_MR}1/$firmware_rsasign ----" >> /tmp/webs_upgrade.log
		wget $wget_options ${dl_path_MR}1/$firmware_rsasign -O $rsa_path
	fi
elif [ "$forsq" -ge 2 ] && [ "$forsq" -le 9 ]; then
	echo "---- wget fw sq beta_user ${dl_path_SQ_beta}${forsq}/$firmware_file ----" >> /tmp/webs_upgrade.log
	wget -t 2 -T $wget_timeout --no-check-certificate --output-file=/tmp/fwget_log ${dl_path_SQ_beta}${forsq}/$firmware_file -O $firmware_path
	wget_result=$?
	if [ "$rsa_enabled" != "" ]; then
		echo "---- wget fw sq beta_user ${dl_path_SQ_beta}${forsq}/$firmware_rsasign ----" >> /tmp/webs_upgrade.log
		wget $wget_options ${dl_path_SQ_beta}${forsq}/$firmware_rsasign -O $rsa_path
	fi
elif [ "$forsq" == "1" ]; then
	echo "---- wget fw sq ${dl_path_SQ}/$firmware_file ----" >> /tmp/webs_upgrade.log
	wget -t 2 -T $wget_timeout --no-check-certificate --output-file=/tmp/fwget_log ${dl_path_SQ}/$firmware_file -O $firmware_path
	wget_result=$?
	if [ "$rsa_enabled" != "" ]; then
		echo "---- wget fw sq ${dl_path_SQ}/$firmware_rsasign ----" >> /tmp/webs_upgrade.log
		wget $wget_options ${dl_path_SQ}/$firmware_rsasign -O $rsa_path
	fi
elif [ "$urlpath" == "" ]; then
	echo "---- wget fw Real ${dl_path_file}/$firmware_file ----" >> /tmp/webs_upgrade.log
	wget -t 2 -T $wget_timeout --no-check-certificate --output-file=/tmp/fwget_log ${dl_path_file}/$firmware_file -O $firmware_path
	wget_result=$?
	if [ "$rsa_enabled" != "" ]; then
		echo "---- wget fw Real ${dl_path_file}/$firmware_rsasign ----" >> /tmp/webs_upgrade.log
		wget $wget_options ${dl_path_file}/$firmware_rsasign -O $rsa_path
	fi
else
	echo "---- wget fw URL ----" >> /tmp/webs_upgrade.log
	wget -t 2 -T $wget_timeout --no-check-certificate --output-file=/tmp/fwget_log $urlpath/$firmware_file -O $firmware_path
	wget_result=$?
	if [ "$rsa_enabled" != "" ]; then
		wget $wget_options $urlpath/$firmware_rsasign -O $rsa_path
	fi
fi	

if [ "$wget_result" != "0" ]; then
	echo "---- download fw failure: $wget_result ----" >> /tmp/webs_upgrade.log
	rm -f $firmware_path
	nvram set $record=1	# fail to download the firmware
	if [ "$force_upgrade" == "1" ]; then
		webs_state_dl_error_count=$((webs_state_dl_error_count+1))
		nvram set webs_state_dl_error_count=$webs_state_dl_error_count
		nvram set webs_state_dl_error_day=$error_day
	fi
elif [ "$rsa_enabled" != "" ] && [ "$?" != "0" ]; then
	echo "---- download rsa failure ----" >> /tmp/webs_upgrade.log
	rm -f $firmware_path
	nvram set $record=2	# fail to download the rsa
	if [ "$force_upgrade" == "1" ]; then
		webs_state_dl_error_count=$((webs_state_dl_error_count+1))
		nvram set webs_state_dl_error_count=$webs_state_dl_error_count
		nvram set webs_state_dl_error_day=$error_day
	fi
else
	nvram set webs_state_upgrade=2	
	echo "---- mv trx OK ----" >> /tmp/webs_upgrade.log
	nvram set firmware_check=0
	firmware_check $firmware_path
	sleep 1

	if [ "$rsa_enabled" != "" ]; then
		nvram set rsasign_check=0
		rsasign_check $firmware_path
		sleep 1
	fi

	firmware_check_ret=`nvram get firmware_check`
	if [ "$rsa_enabled" != "" ]; then
		rsasign_check_ret=`nvram get rsasign_check`
	else
		rsasign_check_ret="1"
	fi


	if [ "$firmware_check_ret" == "1" ] && [ "$rsasign_check_ret" == "1" ]; then
		echo "---- fw check OK ----" >> /tmp/webs_upgrade.log
		/sbin/ejusb -1 0
		nvram set fwpath=2
		nvram set auto_upgrade=0
		nvram set webs_state_dl=0
		nvram commit
		rc rc_service restart_upgrade
	else
		echo "---- fw check error ----" >> /tmp/webs_upgrade.log
		rm -f $firmware_path
		nvram set $record=3	# wrong fw
		if [ "$force_upgrade" == "1" ]; then
			webs_state_dl_error_count=$((webs_state_dl_error_count+1))
			nvram set webs_state_dl_error_count=$webs_state_dl_error_count
			nvram set webs_state_dl_error_day=$error_day
		fi
		if [ "$productid" == "BLUECAVE" ]; then
			rc rc_service start_wrs
		fi
	fi
fi

rm -f $rsa_path

fi

nvram set webs_state_upgrade=1
nvram commit
