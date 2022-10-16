#!/bin/sh

IS_BCMHND=`nvram get rc_support|grep -i bcmhnd`
betaupg_support=`nvram get rc_support|grep -i betaupg`

wget_options="-q -t 2 -T 30 --no-check-certificate"

dl_path_SQ="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ"
dl_path_file="https://dlcdnets.asus.com/pub/ASUS/wireless/ASUSWRT"

echo "---- [Revert FW] To download fw/rsa, Start ----" > /tmp/webs_upgrade.log
logger -t REVERT_FW "To download fw/rsa, Start"
nvram set webs_state_upgrade=0 # INITIALIZING
nvram set webs_state_error=0

# org fw information
firmver_org=`nvram get firmver_org`
firmver_org=`echo $firmver_org | sed s/'\.'//g;`
buildno_org=`nvram get buildno_org`
extendno_org=`nvram get extendno_org`
if [ "$firmver_org" == "" ] || [ "$buildno_org" == "" ] || [ "$extendno_org" == "" ]; then
	revert_version=0
else
	revert_version=`echo $firmver_org`_`echo $buildno_org`_`echo $extendno_org`
fi

if [ "$betaupg_support" != "" ]; then
	logger -t REVERT_FW "betaupg_support"
	firmver_org_1st_bit=${firmver_org:0:1}
	echo "---- firmver_org_1st_bit : $firmver_org_1st_bit ----" >> /tmp/webs_upgrade.log
	logger -t REVERT_FW "firmver_org_1st_bit : $firmver_org_1st_bit"
fi
# current firmware information
productid=`nvram get productid`
get_productid=`echo $productid | sed s/+/plus/;`    #replace 'plus' to '+' for one time
odmpid=`nvram get odmpid`
get_odmpid=`echo $odmpid | sed s/+/plus/;`    #replace 'plus' to '+' for one time
odmpid_support=`nvram get webs_state_odm`
odmpid_support=`echo $odmpid_support | sed s/+/plus/;`    #replace 'plus' to '+' for one time
if [ "$odmpid_support" == "1" ] || [ "$odmpid_support" == "$get_odmpid" ]; then	# MIS || FRS
	if [ "$get_odmpid" != "" ]; then
		get_productid=$get_odmpid
	fi
fi

# fw/rsa file
if [ "$revert_version" == "0" ]; then
	nvram set webs_state_upgrade=1
	echo "---- revert fw not existed ----" >> /tmp/webs_upgrade.log
	logger -t REVERT_FW "revert fw not existed"
	exit 1	#exit scripts
else
	firmware_file=`echo $get_productid`_`echo $revert_version`_un.zip
	firmware_rsasign=`echo $get_productid`_`echo $revert_version`_rsa`nvram get live_update_rsa_ver`.zip
	echo "---- revert_file : $firmware_file ----" >> /tmp/webs_upgrade.log
	logger -t REVERT_FW "$firmware_file"
	echo "---- revert_rsasign : $firmware_rsasign ----" >> /tmp/webs_upgrade.log
	logger -t REVERT_FW "$firmware_rsasign"
fi

#for small size fw to increase free
small_fw_update=`nvram show | grep rc_support | grep small_fw`

if [ "$small_fw_update" != "" ]; then
	echo "---- small_fw_update path ----" >> /tmp/webs_upgrade.log
	logger -t REVERT_FW "small_fw_update path"
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


# for beta path
forbeta=0
if [ "$betaupg_support" != "" ]; then
	if [ "$firmver_org_1st_bit" == "9" ]; then
        	forbeta=1
	fi
	echo "---- forbeta : $forbeta ----" >> /tmp/webs_upgrade.log
	logger -t REVERT_FW "forbeta : $forbeta"
fi

if [ -z "$IS_BCMHND" ]; then
	echo "---- ! IS_BCMHND ----" >> /tmp/webs_upgrade.log
	logger -t REVERT_FW "! IS_BCMHND"
	echo 3 > /proc/sys/vm/drop_caches
fi

wget_result=0
wget_result2=0
if [ "$betaupg_support" != "" ] && [ "$forbeta" == "1" ]; then
	echo "---- wget fw beta ${dl_path_SQ}/$firmware_file ----" >> /tmp/webs_upgrade.log
	logger -t REVERT_FW "wget fw beta $firmware_file"
	wget -t 2 -T 30 --no-check-certificate --output-file=/tmp/fwget_log ${dl_path_SQ}/$firmware_file -O $firmware_path
	wget_result=$?
	echo "---- [LiveUpdate] wget fw, exit code: ${wget_result} ----" >> /tmp/webs_upgrade.log
	logger -t REVERT_FW "exit code: ${wget_result}"

	echo "---- wget fw beta ${dl_path_SQ}/$firmware_rsasign ----" >> /tmp/webs_upgrade.log
	logger -t REVERT_FW "wget fw beta $firmware_rsasign"
	wget $wget_options ${dl_path_SQ}/$firmware_rsasign -O $rsa_path
	wget_result2=$?
	echo "---- [LiveUpdate] wget rsa, exit code: ${wget_result2} ----" >> /tmp/webs_upgrade.log
	logger -t REVERT_FW "exit code: ${wget_result2}"

else
	echo "---- wget fw Real ${dl_path_file}/$firmware_file ----" >> /tmp/webs_upgrade.log
	logger -t REVERT_FW "wget fw Real $firmware_file"
	wget -t 2 -T 30 --no-check-certificate --output-file=/tmp/fwget_log ${dl_path_file}/$firmware_file -O $firmware_path
	wget_result=$?
	echo "---- [LiveUpdate] wget fw, exit code: ${wget_result} ----" >> /tmp/webs_upgrade.log
	logger -t REVERT_FW "exit code: ${wget_result}"

	echo "---- wget fw Real ${dl_path_file}/$firmware_rsasign ----" >> /tmp/webs_upgrade.log
	logger -t REVERT_FW "wget fw Real $firmware_file"
	wget $wget_options ${dl_path_file}/$firmware_rsasign -O $rsa_path
	wget_result2=$?
	echo "---- [LiveUpdate] wget rsa, exit code: ${wget_result2} ----" >> /tmp/webs_upgrade.log
	logger -t REVERT_FW "exit code: ${wget_result2}"
fi	

if [ "$wget_result" != "0" ]; then
	echo "---- download fw failure, End ----" >> /tmp/webs_upgrade.log
	logger -t REVERT_FW "download fw failure, End"
	rc rc_service stop_logger
	rc rc_service "stop_jffs2 0"
	sleep 1
	rm -f $firmware_path
	nvram set webs_state_error=1	# fail to download the firmware
	reboot
elif [ "$wget_result2" != "0" ]; then
	echo "---- download rsa failure, End ----" >> /tmp/webs_upgrade.log
	logger -t REVERT_FW "download rsa failure, End"
	rc rc_service stop_logger
	rc rc_service "stop_jffs2 0"
	sleep 1
	rm -f $firmware_path
	nvram set webs_state_error=1	# fail to download the rsa
	reboot
else
	nvram set webs_state_upgrade=2
	echo "---- mv trx OK ----" >> /tmp/webs_upgrade.log
	logger -t REVERT_FW "mv trx OK"
	nvram set firmware_check=0
	firmware_check $firmware_path
	sleep 1

	nvram set rsasign_check=0
	rsasign_check $firmware_path
	sleep 1

	firmware_check_ret=`nvram get firmware_check`
	rsasign_check_ret=`nvram get rsasign_check`

	if [ "$firmware_check_ret" == "1" ] && [ "$rsasign_check_ret" == "1" ]; then
		echo "---- fw check OK, End ----" >> /tmp/webs_upgrade.log
		logger -t REVERT_FW "fw check OK, End"
		rc rc_service stop_logger
                rc rc_service "stop_jffs2 0"
                sleep 1
		/sbin/ejusb -1 0
		nvram commit
		rc rc_service restart_upgrade
	else
		echo "---- fw check error, CRC: ${firmware_check_ret}  rsa: ${rsasign_check_ret}, End ----" >> /tmp/webs_upgrade.log
		logger -t REVERT_FW "fw check error, CRC: ${firmware_check_ret}  rsa: ${rsasign_check_ret}, End"
		rc rc_service stop_logger
                rc rc_service "stop_jffs2 0"
                sleep 1
		rm -f $firmware_path
		nvram set webs_state_error=1	# wrong fw
		reboot
	fi
fi

echo "---- [Revert FW] To download fw/rsa, End ----" >> /tmp/webs_upgrade.log
logger -t REVERT_FW "To download fw/rsa, End"
nvram set webs_state_upgrade=1
nvram commit
