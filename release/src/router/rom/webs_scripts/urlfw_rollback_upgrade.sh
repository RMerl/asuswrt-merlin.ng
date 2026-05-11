#!/bin/sh

IS_BCMHND=`nvram get rc_support|grep -i bcmhnd`

#wget_options="-q -t 2 -T 30 --no-check-certificate"

dl_path_SQ="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ"
dl_path_SQ_beta="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/app"
dl_path_file="https://dlcdnets.asus.com/pub/ASUS/wireless/ASUSWRT"
#wget -q -y -O- ${path}/${file} | hnd-write -

echo "---- [ROLLBACK_FW] URLFW To download fw/rsa, Start ----" > /tmp/webs_upgrade.log
logger -t ROLLBACK_FW  "URLFW To download fw/rsa, Start"
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
	echo "`date -R` rollback fw : $firmware_file ----" >> /tmp/webs_upgrade.log
	logger -t ROLLBACK_FW "$firmware_file"
fi

if [ -z "$IS_BCMHND" ]; then
	echo "---- ! IS_BCMHND ----" >> /tmp/webs_upgrade.log
	logger -t ROLLBACK_FW  "! IS_BCMHND"
	echo 3 > /proc/sys/vm/drop_caches
fi

wget_result=0
echo "`date -R` [ROLLBACK_FW] wget fw Real ${dl_path_file}/$firmware_file ----" >> /tmp/webs_upgrade.log
logger -t ROLLBACK_FW  "wget fw Real $firmware_file"
hnd-write ${dl_path_file}/$firmware_file

wget_result=$?
echo "`date -R` [ROLLBACK_FW] hnd-write fw, exit code: ${wget_result} ----" >> /tmp/webs_upgrade.log
logger -t ROLLBACK_FW  "exit code: ${wget_result}"


hndwr_status=`nvram get hndwr`
if [ "$hndwr_status" != "99" ] && [ "$hndwr_status" != "-100" ]; then
	echo "`date -R` [ROLLBACK_FW]  download fw failure ----" >> /tmp/webs_upgrade.log
	logger -t ROLLBACK_FW  "download fw failure"
	nvram set webs_state_error=1 # fail to download the firmware
	reboot
else
	nvram set webs_state_upgrade=2	
	echo "`date -R` [ROLLBACK_FW]  mv trx OK ----" >> /tmp/webs_upgrade.log
	logger -t ROLLBACK_FW  "mv trx OK"
	/sbin/ejusb -1 0
	nvram commit
	reboot
fi

echo "`date -R` [ROLLBACK_FW] To download fw/rsa, End ----" >> /tmp/webs_upgrade.log
logger -t ROLLBACK_FW  "URLFW To download fw/rsa, End"
nvram set webs_state_upgrade=1
nvram commit
