#!/bin/sh

IS_BCMHND=`nvram get rc_support|grep -i bcmhnd`

wget_timeout=`nvram get apps_wget_timeout`
#wget_options="-nv -t 2 -T $wget_timeout --dns-timeout=120"
wget_options="-q -t 2 -T $wget_timeout"

dl_path_MR="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/MR"
dl_path_SQ="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ"
dl_path_SQ_beta="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/app"
dl_path_info="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless"
dl_path_file="https://dlcdnets.asus.com/pub/ASUS/wireless/ASUSWRT"

nvram set webs_state_upgrade=0 # INITIALIZING
nvram set webs_state_error=0

#openssl support rsa check
rsa_enabled=`nvram show | grep rc_support | grep HTTPS`

touch /tmp/update_url
update_url=`cat /tmp/update_url`
#update_url="http://192.168.123.198"

# current firmware information
firmware_path=`nvram get firmware_path`
get_productid=`nvram get productid`
get_odmpid=`nvram get odmpid`
odmpid_support=`nvram get webs_state_odm`
if [ "$odmpid_support" == "1" ] || [ "$odmpid_support" == "$get_odmpid" ]; then
	get_productid=`nvram get odmpid`
fi
get_productid=`echo $get_productid | sed s/+/plus/;`	#replace 'plus' to '+' for one time

if [ "$firmware_path" = "1" ]; then
	firmware_file=`echo $get_productid`_`nvram get webs_state_info_beta`_un.zip
else
	firmware_file=`echo $get_productid`_`nvram get webs_state_info`_un.zip
fi

if [ "$rsa_enabled" != "" ]; then
	if [ "$firmware_path" = "1" ]; then
		firmware_rsasign=`echo $get_productid`_`nvram get webs_state_info_beta`_rsa.zip
	else
		firmware_rsasign=`echo $get_productid`_`nvram get webs_state_info`_rsa.zip
	fi	
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

# get firmware zip file
formr=`nvram get MRFLAG`
forsq=`nvram get apps_sq`
urlpath=`nvram get webs_state_url`
wget_result=""
wget_rsa=""
if [ -z "$IS_BCMHND" ]; then
	echo 3 > /proc/sys/vm/drop_caches
fi
nvram set cfg_fwstatus=6
if [ "$update_url" != "" ]; then
	echo "---- wget fw/rsa nvram webs_state_url ----" >> /tmp/webs_upgrade.log
	wget $wget_options ${update_url}/$firmware_file -O $firmware_path
	wget_result=$?
	if [ "$rsa_enabled" != "" ]; then
		wget $wget_options ${update_url}/$firmware_rsasign -O /tmp/rsasign.bin
		wget_rsa=$?
	fi
elif [ "$formr" == "1" ]; then
	echo "---- wget fw MR1 ${dl_path_MR}1/$firmware_file ----" >> /tmp/webs_upgrade.log
	wget $wget_options ${dl_path_MR}1/$firmware_file -O $firmware_path
	wget_result=$?
	if [ "$rsa_enabled" != "" ]; then
		echo "---- wget rsa MR1 ${dl_path_MR}1/$firmware_rsasign ----" >> /tmp/webs_upgrade.log
		wget $wget_options ${dl_path_MR}1/$firmware_rsasign -O /tmp/rsasign.bin
		wget_rsa=$?
	fi
elif [ "$forsq" -ge 2 ] && [ "$forsq" -le 9 ]; then
	echo "---- wget fw sq beta_user ${dl_path_SQ_beta}${forsq}/$firmware_file ----" >> /tmp/webs_upgrade.log
	touch /jffs/KEEP_UPGRADE_DEBUG.log
	wget $wget_options ${dl_path_SQ_beta}${forsq}/$firmware_file -O $firmware_path
	wget_result=$?
	if [ "$rsa_enabled" != "" ]; then
		echo "---- wget rsa sq beta_user ${dl_path_SQ_beta}${forsq}/$firmware_rsasign ----" >> /tmp/webs_upgrade.log
		wget $wget_options ${dl_path_SQ_beta}${forsq}/$firmware_rsasign -O /tmp/rsasign.bin
		wget_rsa=$?
	fi
elif [ "$forsq" == "1" ]; then
	echo "---- wget fw sq ${dl_path_SQ}/$firmware_file ----" >> /tmp/webs_upgrade.log
	wget $wget_options ${dl_path_SQ}/$firmware_file -O $firmware_path
	wget_result=$?
	if [ "$rsa_enabled" != "" ]; then
		echo "---- wget rsa sq ${dl_path_SQ}/$firmware_rsasign ----" >> /tmp/webs_upgrade.log
		wget $wget_options https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/$firmware_rsasign -O /tmp/rsasign.bin
		wget_rsa=$?
	fi
elif [ "$urlpath" == "" ]; then
	echo "---- wget fw Real ${dl_path_file}/$firmware_file ----" >> /tmp/webs_upgrade.log
	wget $wget_options ${dl_path_file}/$firmware_file -O $firmware_path
	wget_result=$?
	if [ "$rsa_enabled" != "" ]; then
		echo "---- wget rsa Real ${dl_path_file}/$firmware_rsasign ----" >> /tmp/webs_upgrade.log
		wget $wget_options https://dlcdnets.asus.com/pub/ASUS/wireless/ASUSWRT/$firmware_rsasign -O /tmp/rsasign.bin
		wget_rsa=$?
	fi
else
	echo "---- wget fw/rsa URL ----" >> /tmp/webs_upgrade.log
	wget $wget_options $urlpath/$firmware_file -O $firmware_path
	wget_result=$?
	if [ "$rsa_enabled" != "" ]; then
		wget $wget_options $urlpath/$firmware_rsasign -O /tmp/rsasign.bin
		wget_rsa=$?
	fi
fi	

if [ "$wget_result" != "0" ] || [ "$wget_rsa" != "0" ]; then	#download failure
	echo "---- wget fw/rsa failure:  ${wget_result} ${wget_rsa} ----" >> /tmp/webs_upgrade.log
	nvram set webs_state_error=1
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
		nvram set fwpath=2
		nvram commit
#		/sbin/ejusb -1 0
#		rc rc_service restart_upgrade
	else
		echo "---- fw check error ----" >> /tmp/webs_upgrade.log
		nvram set webs_state_error=3	# wrong fw
	fi
fi

rm -f /tmp/rsasign.bin

nvram set webs_state_upgrade=1
