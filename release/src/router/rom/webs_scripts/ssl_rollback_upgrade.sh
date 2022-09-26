#!/bin/sh

IS_BCMHND=`nvram get rc_support|grep -i bcmhnd`

wget_options="-q -t 2 -T 30 --no-check-certificate"

dl_path_SQ="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ"
dl_path_file="https://dlcdnets.asus.com/pub/ASUS/wireless/ASUSWRT"

echo "`date -R` [ROLLBACK_FW] To download fw/rsa, Start ----" > /tmp/webs_upgrade.log
logger -t ROLLBACK_FW "To download fw/rsa, Start"
nvram set webs_state_upgrade=0 # INITIALIZING
nvram set webs_state_error=0

#get rbk fw info
rbk_model=`echo $1`
rbk_firm=`echo $2`

if [ "rbk_firm" == "" ]; then
	rbk_firm=0
fi
# fw/rsa file
if [ "$rbk_firm" == "0" ]; then
	nvram set webs_state_upgrade=1
	echo "`date -R` [ROLLBACK_FW] rollback fw not existed ----" >> /tmp/webs_upgrade.log
	logger -t ROLLBACK_FW "rollback fw not existed"
	exit 1	#exit scripts
else
	firmware_file=`echo $rbk_model`_`echo $rbk_firm`_un.zip
	firmware_rsasign=`echo $rbk_model`_`echo $rbk_firm`_rsa`nvram get live_update_rsa_ver`.zip
	echo "`date -R` rollback fw : $firmware_file ----" >> /tmp/webs_upgrade.log
	logger -t ROLLBACK_FW "$firmware_file"
	echo "`date -R` rollback rsa : $firmware_rsasign ----" >> /tmp/webs_upgrade.log
	logger -t ROLLBACK_FW "$firmware_rsasign"
fi

#for small size fw to increase free
small_fw_update=`nvram show | grep rc_support | grep small_fw`

if [ "$small_fw_update" != "" ]; then
	echo "`date -R` small_fw_update path ----" >> /tmp/webs_upgrade.log
	logger -t ROLLBACK_FW "small_fw_update path"
	mkdir /tmp/mytmpfs
	mount -t tmpfs -o size=16M,nr_inodes=10k,mode=700 tmpfs /tmp/mytmpfs
	firmware_path="/tmp/mytmpfs/linux.trx"
else
	firmware_path="/tmp/linux.trx"
fi
rsa_path=/tmp/rsasign.bin

#kill old files
rm -f $firmware_path
rm -f $rsa_path


if [ -z "$IS_BCMHND" ]; then
	echo "`date -R` ! IS_BCMHND ----" >> /tmp/webs_upgrade.log
	logger -t ROLLBACK_FW "! IS_BCMHND"
	echo 3 > /proc/sys/vm/drop_caches
fi

wget_result=0
wget_result2=0

echo "`date -R` wget fw Real ${dl_path_file}/$firmware_file ----" >> /tmp/webs_upgrade.log
logger -t ROLLBACK_FW "wget fw Real $firmware_file"
wget -t 2 -T 30 --no-check-certificate --output-file=/tmp/fwget_log ${dl_path_file}/$firmware_file -O $firmware_path
wget_result=$?
echo "`date -R` [ROLLBACK_FW] wget fw, exit code: ${wget_result} ----" >> /tmp/webs_upgrade.log
logger -t ROLLBACK_FW "exit code: ${wget_result}"

echo "`date -R` wget fw Real ${dl_path_file}/$firmware_rsasign ----" >> /tmp/webs_upgrade.log
logger -t ROLLBACK_FW "wget fw Real $firmware_file"
wget $wget_options ${dl_path_file}/$firmware_rsasign -O $rsa_path
wget_result2=$?
echo "`date -R` [ROLLBACK_FW] wget rsa, exit code: ${wget_result2} ----" >> /tmp/webs_upgrade.log
logger -t ROLLBACK_FW "exit code: ${wget_result2}"

if [ "$wget_result" != "0" ]; then
	echo "`date -R` [ROLLBACK_FW] download fw failure ----" >> /tmp/webs_upgrade.log
	logger -t ROLLBACK_FW "download fw failure"
	rm -f $firmware_path
	nvram set webs_state_error=1	# fail to download the firmware
	reboot
elif [ "$wget_result2" != "0" ]; then
	echo "`date -R` [ROLLBACK_FW] download rsa failure ----" >> /tmp/webs_upgrade.log
	logger -t ROLLBACK_FW "download rsa failure"
	rm -f $firmware_path
	nvram set webs_state_error=1	# fail to download the rsa
	reboot
else
	nvram set webs_state_upgrade=2
	echo "`date -R` [ROLLBACK_FW] mv trx OK ----" >> /tmp/webs_upgrade.log
	logger -t ROLLBACK_FW "mv trx OK"
	nvram set firmware_check=0
	firmware_check $firmware_path
	sleep 1

	nvram set rsasign_check=0
	rsasign_check $firmware_path
	sleep 1

	firmware_check_ret=`nvram get firmware_check`
	rsasign_check_ret=`nvram get rsasign_check`

	if [ "$firmware_check_ret" == "1" ] && [ "$rsasign_check_ret" == "1" ]; then
		echo "`date -R` [ROLLBACK_FW] fw check OK ----" >> /tmp/webs_upgrade.log
		logger -t ROLLBACK_FW "fw check OK"
		/sbin/ejusb -1 0
		nvram commit
		rc rc_service restart_upgrade
	else
		echo "`date -R` [ROLLBACK_FW] fw check error, CRC: ${firmware_check_ret}  rsa: ${rsasign_check_ret} ----" >> /tmp/webs_upgrade.log
		logger -t ROLLBACK_FW "fw check error, CRC: ${firmware_check_ret}  rsa: ${rsasign_check_ret}"
		rm -f $firmware_path
		nvram set webs_state_error=1	# wrong fw
		reboot
	fi
fi

echo "`date -R` [ROLLBACK_FW] To download fw/rsa, End ----" >> /tmp/webs_upgrade.log
logger -t ROLLBACK_FW "To download fw/rsa, End"
nvram set webs_state_upgrade=1
nvram commit
