#!/bin/sh

IS_BCMHND=`nvram get rc_support|grep -i bcmhnd`

wget_options="--ciphers=DEFAULT:@SECLEVEL=1:!CAMELLIA:!3DES -q -t 2 -T 30 --no-check-certificate"

dl_path_SQ="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ"
dl_path_file="https://dlcdnets.asus.com/pub/ASUS/wireless/ASUSWRT"

nvram set sig_state_upgrade=0 # INITIALIZING
nvram set sig_state_error=0
nvram set sig_state_update=0

touch /tmp/update_url
update_url=`cat /tmp/update_url`
#update_url="http://192.168.123.198"

sig_type=`nvram get sig_type`
sig_file=`nvram get SKU`_`nvram get sig_state_info`_un.zip
sig_rsasign=`nvram get SKU`_`nvram get sig_state_info`_rsa`nvram get live_update_rsa_ver`.zip

mv_sig_file="/jffs/signature/rule.trf"
if [ "$sig_type" == "HNS" ]; then
	mv_sig_file="/jffs/signature/rule.zip"
fi

echo "---- sig upgrade start: ----" > /tmp/sig_upgrade.log
echo "$sig_file" >> /tmp/sig_upgrade.log
echo "$sig_rsasign" >> /tmp/sig_upgrade.log

# get signature zip file
forsq=`nvram get apps_sq`
if [ -z "$forsq" ]; then
	forsq=0
fi
dl_path=${dl_path_file}
dl_path_log="Official"
if [ "$forsq" == "1" ]; then
	dl_path=${dl_path_SQ}
	dl_path_log="SQ"
fi

#urlpath=`nvram get sig_state_url`
if [ -z "$IS_BCMHND" ]; then
	echo 3 > /proc/sys/vm/drop_caches
fi

wget_result=0
echo "---- wget trf ${sig_type} : ${dl_path_log} ----" >> /tmp/sig_upgrade.log
logger -t SIG_UPGRADE "wget trf ${sig_type} : ${dl_path_log}"
wget ${wget_options} ${dl_path}/${sig_file} -O /tmp/rule.trf
wget_result=$?
echo "---- wget sig, exit code: ${wget_result} ----" >> /tmp/sig_upgrade.log
logger -t SIG_UPGRADE "wget sig, exit code: ${wget_result}"

wget_result2=0
wget ${wget_options} ${dl_path}/${sig_rsasign} -O /tmp/rsasign.bin
wget_result2=$?
echo "---- wget rsa, exit code: ${wget_result2} ----" >> /tmp/sig_upgrade.log
logger -t SIG_UPGRADE "wget rsa, exit code: ${wget_result2}"

model=`nvram get productid`
if [ "$wget_result" != "0" ]; then	#download failure
	echo "---- Download trf file Failure ----" >> /tmp/sig_upgrade.log
	logger -t SIG_UPGRADE "Download trf file Failure"
	nvram set sig_state_error=1
elif [ "$wget_result2" != "0" ]; then	#download failure
	echo "---- Download rsa file Failure ----" >> /tmp/sig_upgrade.log
	logger -t SIG_UPGRADE "Download rsa file Failure"
	nvram set sig_state_error=1
else
	nvram set sig_state_upgrade=2
	echo "---- Download trf/rsa OK ----" >> /tmp/sig_upgrade.log
	logger -t SIG_UPGRADE "Download trf/rsa OK"
	
	nvram set bwdpi_rsa_check=0
	rsasign_sig_check /tmp/rule.trf
	sleep 1

	rsasign_check_ret=`nvram get bwdpi_rsa_check`

	if [ "$rsasign_check_ret" == "1" ]; then
		echo "---- sig check OK ----" >> /tmp/sig_upgrade.log
		logger -t SIG_UPGRADE "sig check OK"
		nvram set sig_update_t=`date +%s`   #set timestamp for download signature and restart_wrs
		if [ -f ${mv_sig_file} ]; then
			echo "---- rm original ${mv_sig_file} ----" >> /tmp/sig_upgrade.log
			rm ${mv_sig_file}
		else
			echo "---- mkdir /jffs/signature ----" >> /tmp/sig_upgrade.log
			mkdir /jffs/signature
		fi
		echo "---- mv ${mv_sig_file} to jffs ----" >> /tmp/sig_upgrade.log
		logger -t SIG_UPGRADE "mv ${mv_sig_file} to jffs"
		mv /tmp/rule.trf ${mv_sig_file}

		if [ "$1" == "" ];then
			# special case for bluecave
			if [ "$model" == "BLUECAVE" ]; then
				echo "stop_wrs_force and free memory" >> /tmp/sig_upgrade.log
				rc rc_service stop_wrs_force
				if [ -z "$IS_BCMHND" ]; then
					echo 1 > /proc/sys/vm/drop_caches
				fi
			fi
			echo "Do restart_wrs" >> /tmp/sig_upgrade.log
			logger -t SIG_UPGRADE "Do restart_wrs"
			rc rc_service restart_wrs
		else
			echo "do nothing..." >> /tmp/sig_upgrade.log
			logger -t SIG_UPGRADE "do nothing..."
		fi
	else
		echo "---- sig rsa check error ----" >> /tmp/sig_upgrade.log
		logger -t SIG_UPGRADE "sig rsa check error"
		nvram set sig_state_error=3	# wrong sig trf
	fi
fi

rm -f /tmp/rsasign.bin

echo "---- sig upgrade end ----" >> /tmp/sig_upgrade.log
nvram set sig_state_upgrade=1
