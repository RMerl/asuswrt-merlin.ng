#!/bin/sh

update_ai_prog(){
	nvram set ai_prog_status="$1"
	nvram set ai_prog_error_code="$2"
	nvram set ai_prog_reason="$3"
	nvram set ai_prog_dwl_percent="$4"
	nvram set ai_prog_dwl_bytes="$5"
	nvram set ai_prog_total_steps="$6"
	nvram set ai_prog_current_step="$7"
	nvram set ai_prog_current_percent="$8"
	nvram set ai_prog_current_image="$9"
}

info() {
    echo "[AI_UPGRADE][$(date '+%F %T')][INFO] $1" >> /ai/log/webs_ai_upgrade.log
}

error() {
    echo "[AI_UPGRADE][$(date '+%F %T')][ERROR] $1" >> /ai/log/webs_ai_upgrade.log
}

# ===== ai board upgrade =====

betaupg_support=`nvram get rc_support|grep -i betaupg`

wget_options="--ciphers='DEFAULT:@SECLEVEL=1:!CAMELLIA:!3DES' -q -t 2 -T 30 --no-check-certificate"

dl_path_SQ="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ"
dl_path_SQ_beta="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/app"
dl_path_file="https://dlcdnets.asus.com/pub/ASUS/wireless/ASUSWRT"

touch /tmp/update_url
update_url=`cat /tmp/update_url`

# for beta path
forbeta=0
if [ "$betaupg_support" != "" ]; then
	forbeta=`nvram get webs_update_beta`
fi

# for sq
forsq=`nvram get apps_sq`
if [ -z "$forsq" ]; then
	forsq=0
fi

urlpath=`nvram get webs_state_url`

ai_support=`nvram get rc_support | grep -i ai_support`

if [ "$ai_support" != "" ]; then
	update_ai_prog "START" 0 "" 0 0 0 0 0 "aisom.swu"
	
	echo "[AI_UPGRADE][$(date '+%F %T')][INFO]---- Start to download AI fw/rsa ----" > /ai/log/webs_ai_upgrade.log
	logger -t AI_AUTO_UPGRADE "Start to download AI fw/rsa"
	nvram set webs_state_ai_upgrade=0
	nvram set webs_state_ai_error=0
	
	ai_productid=`nvram get ai_productid`

	ai_firmware_path="/ai/firmware/aisom.swu"
	ai_rsa_path="/tmp/ai_rsasign.bin"
	
	#kill old files
	rm -f $ai_firmware_path
	rm -f $ai_rsa_path
	
	ai_wget_result=0
	ai_wget_result2=0
	
	ai_firmware_file=`echo $ai_productid`_`nvram get webs_state_ai_info`_un.zip
	ai_firmware_rsasign=`echo $ai_productid`_`nvram get webs_state_ai_info`_rsa`nvram get live_update_rsa_ver`.zip
	#ai_firmware_file=SL1680_3_0_0_2_102_80-g588fa7e_19-gec2d4cb_un.zip
	#ai_firmware_rsasign=SL1680_3_0_0_2_102_80-g588fa7e_19-gec2d4cb_rsa.zip
	
	if [ "$update_url" != "" ]; then
		ai_dl_url=${update_url}
	elif [ "$betaupg_support" != "" ] && [ "$forbeta" == "1" ]; then
		ai_dl_url=${dl_path_SQ}
	elif [ "$forsq" -ge 2 ] && [ "$forsq" -le 9 ]; then
		ai_dl_url=${dl_path_SQ_beta}${forsq}
	elif [ "$forsq" == "1" ]; then
		ai_dl_url=${dl_path_SQ}
	elif [ "$urlpath" == "" ]; then
		ai_dl_url=${dl_path_file}
	else
		ai_dl_url=${urlpath}
	fi
	
	info "---- wget fw nvram ai_webs_state_url ${ai_dl_url}/$ai_firmware_file ----"
	logger -t AI_AUTO_UPGRADE "wget fw nvram ai_webs_state_url $ai_firmware_file"
	update_ai_prog "DOWNLOAD_FW" 0 "" 0 0 0 0 0 "aisom.swu"
	
	wget --ciphers='DEFAULT:@SECLEVEL=1:!CAMELLIA:!3DES' -t 2 -T 30 --no-check-certificate --output-file=/tmp/fwget_log ${ai_dl_url}/$ai_firmware_file -O $ai_firmware_path
	ai_wget_result=$?
	info "---- wget fw nvram ai_webs_state_url, exit code: ${ai_wget_result} ----"
	logger -t AI_AUTO_UPGRADE "exit code: ${ai_wget_result}"
	
	info "---- wget rsa nvram ai_webs_state_url ${ai_dl_url}/$ai_firmware_rsasign ----"
	logger -t AI_AUTO_UPGRADE "wget rsa nvram ai_webs_state_url $ai_firmware_rsasign"
	update_ai_prog "DOWNLOAD_FW_RSA" 0 "" 0 0 0 0 0 "aisom_rsasign.bin"
	
	wget --ciphers='DEFAULT:@SECLEVEL=1:!CAMELLIA:!3DES' -t 2 -T 30 --no-check-certificate ${ai_dl_url}/$ai_firmware_rsasign -O $ai_rsa_path
	ai_wget_result2=$?
	info "---- wget rsa nvram ai_webs_state_url, exit code: ${ai_wget_result2} ----"
	logger -t AI_AUTO_UPGRADE "exit code: ${ai_wget_result2}"
	
	if [ "$ai_wget_result" != "0" ]; then
		error "---- download fw failure, End ----"
		logger -t AI_AUTO_UPGRADE "download fw failure, End"
		rm -f $ai_firmware_path
		nvram set webs_state_ai_error=2
		update_ai_prog "failed" 500 "FW download was failed" 0 0 0 0 0 "aisom.swu"
		sleep 1
		exit 0
	elif [ "$ai_wget_result2" != "0" ]; then
		error "---- download rsa failure, End ----"
		logger -t AI_AUTO_UPGRADE "download rsa failure, End"
		rm -f $ai_firmware_path
		nvram set webs_state_ai_error=3
		update_ai_prog "failed" 500 "FW verification file download was failed" 0 0 0 0 0 "aisom_rsasign.bin"
		sleep 1
		exit 0
	else
		nvram set firmware_check=0
		nvram set rsasign_check=0
		info "---- Download AI FW OK ----"
		logger -t AI_AUTO_UPGRADE "Download AI FW OK"
		update_ai_prog "VERIFY_FW" 0 "" 0 0 0 0 0 "aisom.swu"
	
		nvram set ai_rsasign_check=0
		rsasign_check $ai_firmware_path $ai_rsa_path
		sleep 1
	
		ai_rsasign_check_ret=`nvram get ai_rsasign_check`
	
		if [ "$ai_rsasign_check_ret" == "1" ]; then
			info "---- AI fw check OK ----"
			logger -t AI_AUTO_UPGRADE "AI fw check OK"
			nvram set webs_state_ai_upgrade=1
			info "---- Download AI fw/rsa, End ----"
			logger -t AI_AUTO_UPGRADE "Downoad AI fw/rsa, End"
			update_ai_prog "VERIFY_FW" 200 "FW verify success" 0 0 0 0 0 "aisom.swu"
			sleep 1 
			nvram set ai_fw_md5=$(md5sum $ai_firmware_path | cut -d ' ' -f 1)
			sleep 1
			# ===== Call for upgrade =====
			rc rc_service "start_ai_upgrade aisom.swu"
			# ============================
		else
			error "---- fw check error, CRC: ${ai_firmware_check_ret}  rsa: ${ai_rsasign_check_ret} ----"
			logger -t AI_AUTO_UPGRADE "fw check error, CRC: ${ai_firmware_check_ret}  rsa: ${ai_rsasign_check_ret}"
			update_ai_prog "VERIFY_FW" 406 "FW verify fail" 0 0 0 0 0 "aisom.swu"
			rm -f $ai_firmware_path
			nvram set webs_state_ai_error=4
			error "---- Download AI fw/rsa, End ----"
			logger -t AI_AUTO_UPGRADE "Download AI fw/rsa, End"
			sleep 1
			exit 0
		fi
	fi
fi
# ===== End of download ai board firmware from server to router =====
