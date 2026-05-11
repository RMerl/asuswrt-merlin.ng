#!/bin/sh

IS_BCMHND=`nvram get rc_support|grep -i bcmhnd`
betaupg_support=`nvram get rc_support|grep -i betaupg`

#wget_options="-q -t 2 -T 30 --no-check-certificate"

dl_path_SQ="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ"
dl_path_SQ_beta="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/app"
dl_path_file="https://dlcdnets.asus.com/pub/ASUS/wireless/ASUSWRT"
#wget -q -y -O- ${path}/${file} | hnd-write -

echo "---- [Revert FW] URLFW To download fw/rsa, Start ----" > /tmp/webs_upgrade.log
logger -t REVERT_FW "URLFW To download fw/rsa, Start"
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
firmver_org_1st_bit=${firmver_org:0:1}
echo "---- firmver_org_1st_bit : $firmver_org_1st_bit ----" >> /tmp/webs_upgrade.log
logger -t REVERT_FW "firmver_org_1st_bit : $firmver_org_1st_bit"

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
	exit 1	#exit scripts
else
	firmware_file=`echo $get_productid`_`echo $revert_version`_un.zip
	#firmware_rsasign=`echo $get_productid`_`echo $revert_version`_rsa`nvram get live_update_rsa_ver`.zip
	echo "---- revert_file : $firmware_file ----" >> /tmp/webs_upgrade.log
	logger -t REVERT_FW "$firmware_file"
fi

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
if [ "$betaupg_support" != "" ] && [ "$forbeta" == "1" ]; then
	echo "---- wget fw beta ${dl_path_SQ}/$firmware_file ----" >> /tmp/webs_upgrade.log
	logger -t REVERT_FW "wget fw beta $firmware_file"
	hnd-write ${dl_path_SQ}/$firmware_file
else
	echo "---- wget fw Real ${dl_path_file}/$firmware_file ----" >> /tmp/webs_upgrade.log
	logger -t REVERT_FW "wget fw Real $firmware_file"
	hnd-write ${dl_path_file}/$firmware_file
fi
wget_result=$?
echo "---- [LiveUpdate] hnd-write fw, exit code: ${wget_result} ----" >> /tmp/webs_upgrade.log
logger -t REVERT_FW "exit code: ${wget_result}"


hndwr_status=`nvram get hndwr`
if [ "$hndwr_status" != "99" ] && [ "$hndwr_status" != "-100" ]; then
	echo "---- download fw failure, End ----" >> /tmp/webs_upgrade.log
	logger -t REVERT_FW "download fw failure, End"
	rc rc_service stop_logger
	rc rc_service "stop_jffs2 0"
	sleep 1
	nvram set webs_state_error=1 # fail to download the firmware
	reboot
else
	nvram set webs_state_upgrade=2	
	echo "---- mv trx OK, End ----" >> /tmp/webs_upgrade.log
	logger -t REVERT_FW "mv trx OK, End"
	rc rc_service stop_logger
	rc rc_service "stop_jffs2 0"
	sleep 1
	/sbin/ejusb -1 0
	nvram commit
	reboot
fi

echo "---- [Revert FW] To download fw/rsa, End ----" >> /tmp/webs_upgrade.log
logger -t REVERT_FW "URLFW To download fw/rsa, End"
nvram set webs_state_upgrade=1
nvram commit
