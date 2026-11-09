#!/bin/sh

wget_options="--ciphers=DEFAULT:@SECLEVEL=1:!CAMELLIA:!3DES -q -t 2 -T 30"

dl_path_SQ="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ"
dl_path_file="https://dlcdnets.asus.com/pub/ASUS/wireless/ASUSWRT"

nvram set ark_state_upgrade=0 # INITIALIZING
nvram set ark_sig_state=3     # UPGRADING
nvram set sig_state_error=0
nvram set ark_state_update=0

#get L10 version4 only:
sig_type="$(nvram get Ark_SKU)"  # WW_4L_10M
sig_engine="${sig_type:3:1}"
sig_attr="${sig_type:4:5}"
sig_state_info="$(nvram get sigArk_state_info)"
live_update_rsa_ver="$(nvram get live_update_rsa_ver)"
sig_file="${sig_type}_${sig_state_info}_un.zip"
sig_rsasign="${sig_type}_${sig_state_info}_rsa${live_update_rsa_ver}.zip"

mv_sig_file="/jffs/signature/ark_rule.zip"

echo "---- sigArk upgrade start: ----" >> /tmp/sigArk_upgrade.log
echo "$sig_file" >> /tmp/sigArk_upgrade.log
echo "$sig_rsasign" >> /tmp/sigArk_upgrade.log

# get signature zip file
forsq="$(nvram get apps_sq)"
if [ -z "$forsq" ]; then
	forsq=0
fi
dl_path=${dl_path_file}
dl_path_log="Official"
if [ "$forsq" == "1" ]; then
	dl_path=${dl_path_SQ}
	dl_path_log="SQ"
fi

wget_result=0
echo "---- wget ark signature ${sig_type} : ${dl_path_log} ----" >> /tmp/sigArk_upgrade.log
logger -t SIG_ARK_UPGRADE "wget ark signature ${sig_type} : ${dl_path_log}"
wget ${wget_options} "${dl_path}/${sig_file}" -O /tmp/rule_ark.trf
wget_result=$?
echo "---- wget ark_zip, exit code: ${wget_result} ----" >> /tmp/sigArk_upgrade.log
logger -t SIG_ARK_UPGRADE "wget ark_zip, exit code: ${wget_result}"

wget_result2=0
wget ${wget_options} "${dl_path}/${sig_rsasign}" -O /tmp/rsasign_ark.bin
wget_result2=$?
echo "---- wget ark_rsa, exit code: ${wget_result2} ----" >> /tmp/sigArk_upgrade.log
logger -t SIG_ARK_UPGRADE "wget ark_rsa, exit code: ${wget_result2}"

model="$(nvram get productid)"
if [ "$wget_result" != "0" ]; then	#download failure
	echo "---- Download ark_zip file Failure ----" >> /tmp/sigArk_upgrade.log
	logger -t SIG_ARK_UPGRADE "Download ark_zip file Failure"
	nvram set sig_state_error=4
	nvram set ark_sig_unzip=1
	nvram set ark_sig_state=6    # FAILED
elif [ "$wget_result2" != "0" ]; then	#download failure
	echo "---- Download ark_rsa Failure ----" >> /tmp/sigArk_upgrade.log
	logger -t SIG_ARK_UPGRADE "Download ark_rsa file Failure"
	nvram set sig_state_error=4
	nvram set ark_sig_unzip=1
	nvram set ark_sig_state=6    # FAILED
else
	nvram set ark_state_upgrade=2
	echo "---- Download ark_zip/ark_rsa OK ----" >> /tmp/sigArk_upgrade.log
	logger -t SIG_ARK_UPGRADE "Download ark_zip/ark_rsa OK"

	rsasign_sigArk_check /tmp/rule_ark.trf
	rsasign_ret=$?

	if [ "$rsasign_ret" = "0" ]; then
		echo "---- sigArk check OK ----" >> /tmp/sigArk_upgrade.log
		logger -t SIG_ARK_UPGRADE "sigArk check OK"
		nvram set ark_sig_state=4    # VERIFIED
		nvram set sig_update_t="$(date +%s)"   #set timestamp for download signature and stop/start_ark_engine
		if [ -f "${mv_sig_file}" ]; then
			echo "---- rm original ${mv_sig_file} ----" >> /tmp/sigArk_upgrade.log
			rm -f "${mv_sig_file}"
		else
			echo "---- mkdir /jffs/signature ----" >> /tmp/sigArk_upgrade.log
			mkdir -p /jffs/signature
		fi
		echo "---- mv ${mv_sig_file} to jffs ----" >> /tmp/sigArk_upgrade.log
		logger -t SIG_ARK_UPGRADE "mv ${mv_sig_file} to jffs"
		mv /tmp/rule_ark.trf "${mv_sig_file}"
		nvram set ark_sig_ver="GS_${sig_engine}${sig_state_info}${sig_attr}"	#Update ark_sig_ver after parsing newer info correctly
		echo "---- set nvram: GS_${sig_engine}${sig_state_info}${sig_attr} ----" >> /tmp/sigArk_upgrade.log
		logger -t SIG_ARK_UPGRADE "set nvram: GS_${sig_engine}${sig_state_info}${sig_attr}"
		nvram set ark_sig_state=5    # APPLIED
		#//WW_4L_10M_2512150_un.zip //GS_42512010L_10M
	else
		echo "---- sigArk rsa check failed: ${rsasign_ret} ----" >> /tmp/sigArk_upgrade.log
		logger -t SIG_ARK_UPGRADE "sigArk rsa check failed: ${rsasign_ret}"
		nvram set sig_state_error=5	 #wrong sigArk
		nvram set ark_sig_unzip=1
		nvram set ark_sig_state=6    # FAILED
	fi
fi

rm -f /tmp/rsasign_ark.bin

echo "---- sigArk upgrade end ----" >> /tmp/sigArk_upgrade.log
nvram set ark_state_upgrade=1
