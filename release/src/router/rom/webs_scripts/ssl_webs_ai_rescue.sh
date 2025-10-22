#!/bin/sh

# First, check original files and rsa
# If failed or missed files, then download with envram version.
# If download failed or rsa check failed then flush all files and download all of files with FRS rescue version

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
    echo "[AI_RESCUE][$(date '+%F %T')][INFO] $1" >> /ai/log/ai_rescue.log
}

error() {
    echo "[AI_RESCUE][$(date '+%F %T')][ERROR] $1" >> /ai/log/ai_rescue.log
}

echo "[AI_RESCUE][$(date '+%F %T')][INFO] ---- Start RESET TO DEFAULT/RESCUE AISOM Process ----" > /ai/log/ai_rescue.log
logger -t AI_RESCUE "Start RESET TO DEFAULT/RESCUE AISOM Process"

# Initialize
nvram set ai_fw_path="firmware/aisom_reset.swu"
update_ai_prog "START" 0 "" 0 0 0 0 0 "aisom_reset.swu"
ai_reset_beta_support=`nvram get rc_support | grep -i ai_reset_beta`

dl_path_SQ="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ"
dl_path_SQ_beta="https://dlcdnets.asus.com/pub/ASUS/LiveUpdate/Release/Wireless_SQ/app"
dl_path_file="https://dlcdnets.asus.com/pub/ASUS/wireless/ASUSWRT"

# Procedure is same as fw upgrade
# Thus use the same nvram to record
nvram set webs_state_ai_upgrade=0
nvram set webs_state_ai_error=0

productid=`nvram get productid`
urlpath=`nvram get webs_state_url`
update_url=`cat /tmp/update_url`
ai_productid=`nvram get ai_productid`

fw_filepath="/ai/firmware"
apps_filepath="/ai/docker_images"

fw_path="/ai/firmware/aisom_reset.swu"
fw_rsa_path="/ai/firmware/aisom_reset_rsasign$(nvram get live_update_rsa_ver).bin"

# Frigate NVR is a common app
frigate_path="/ai/docker_images/frigate.tar"
frigate_rsa_path="/ai/docker_images/frigate_rsasign$(nvram get live_update_rsa_ver).bin"

# Currently, SLM only belongs to GT-BE19000AI
slm_path="/ai/docker_images/slm.tar"
slm_rsa_path="/ai/docker_images/slm_rsasign$(nvram get live_update_rsa_ver).bin"

# Currently, Portainer only belongs to GT-BE96AI
portainer_path="/ai/docker_images/portainer.tar"
portainer_rsa_path="/ai/docker_images/portainer_rsasign$(nvram get live_update_rsa_ver).bin"
ha_path="/ai/docker_images/homeassistant.tar"
ha_rsa_path="/ai/docker_images/ha_rsasign$(nvram get live_update_rsa_ver).bin"

# for beta path
forbeta=0
if [ "$ai_reset_beta_support" != "" ]; then
	forbeta=`nvram get webs_update_beta`
fi

# for sq
forsq=`nvram get apps_sq`
if [ -z "$forsq" ]; then
	forsq=0
fi

urlpath=`nvram get webs_state_url`

if [ "$update_url" != "" ]; then
	dl_url=${update_url}
elif [ "$ai_reset_beta_support" != "" ] && [ "$forbeta" == "1" ]; then
	dl_url=${dl_path_SQ}
elif [ "$forsq" -ge 2 ] && [ "$forsq" -le 9 ]; then
	dl_url=${dl_path_SQ_beta}${forsq}
elif [ "$forsq" == "1" ]; then
	dl_url=${dl_path_SQ}
elif [ "$urlpath" == "" ]; then
	dl_url=${dl_path_file}
else
	dl_url=${urlpath}
fi

# This part is default
default_rescue_ver=$(envram get aiboard_rescue_ver | tr '.' '_')
def_fw_file=${ai_productid}_${default_rescue_ver}_un_rescue.zip
def_fw_rsa_file=${ai_productid}_${default_rescue_ver}_rsa`nvram get live_update_rsa_ver`_rescue.zip
def_frigate_file=${ai_productid}_frigate_${default_rescue_ver}_un_rescue.zip
def_frigate_rsa_file=${ai_productid}_frigate_${default_rescue_ver}_rsa`nvram get live_update_rsa_ver`_rescue.zip
if [ "$productid" == "GT-BE96_AI" ]; then
	def_slm_file=${ai_productid}_slm_cn_${default_rescue_ver}_un_rescue.zip
	def_slm_rsa_file=${ai_productid}_slm_cn_${default_rescue_ver}_rsa`nvram get live_update_rsa_ver`_rescue.zip
elif [ "$productid" == "GT-BE19000AI" ]; then
	def_slm_file=${ai_productid}_slm_${default_rescue_ver}_un_rescue.zip
	def_slm_rsa_file=${ai_productid}_slm_${default_rescue_ver}_rsa`nvram get live_update_rsa_ver`_rescue.zip
fi
def_portainer_file=${ai_productid}_portainer_${default_rescue_ver}_un_rescue.zip
def_portainer_rsa_file=${ai_productid}_portainer_${default_rescue_ver}_rsa`nvram get live_update_rsa_ver`_rescue.zip
def_ha_file=${ai_productid}_ha_${default_rescue_ver}_un_rescue.zip
def_ha_rsa_file=${ai_productid}_ha_${default_rescue_ver}_rsa`nvram get live_update_rsa_ver`_rescue.zip

# This part is from server
live_update_rescue_ver=$(nvram get aiboard_rescue_ver | tr '.' '_')
fw_file=${ai_productid}_${live_update_rescue_ver}_un_rescue.zip
fw_rsa_file=${ai_productid}_${live_update_rescue_ver}_rsa`nvram get live_update_rsa_ver`_rescue.zip
frigate_file=${ai_productid}_frigate_${live_update_rescue_ver}_un_rescue.zip
frigate_rsa_file=${ai_productid}_frigate_${live_update_rescue_ver}_rsa`nvram get live_update_rsa_ver`_rescue.zip
if [ "$productid" == "GT-BE96_AI" ]; then
	slm_file=${ai_productid}_slm_cn_${live_update_rescue_ver}_un_rescue.zip
	slm_rsa_file=${ai_productid}_slm_cn_${live_update_rescue_ver}_rsa`nvram get live_update_rsa_ver`_rescue.zip
elif [ "$productid" == "GT-BE19000AI" ]; then
	slm_file=${ai_productid}_slm_${live_update_rescue_ver}_un_rescue.zip
	slm_rsa_file=${ai_productid}_slm_${live_update_rescue_ver}_rsa`nvram get live_update_rsa_ver`_rescue.zip
fi
portainer_file=${ai_productid}_portainer_${live_update_rescue_ver}_un_rescue.zip
portainer_rsa_file=${ai_productid}_portainer_${live_update_rescue_ver}_rsa`nvram get live_update_rsa_ver`_rescue.zip
ha_file=${ai_productid}_ha_${live_update_rescue_ver}_un_rescue.zip
ha_rsa_file=${ai_productid}_ha_${live_update_rescue_ver}_rsa`nvram get live_update_rsa_ver`_rescue.zip

AI_APPS_ENABLE=$(nvram get ai_default_docker_apps_enable)

check_signature() {
	local file_path="$1"
	local rsa_path="$2"
	nvram set ai_rsasign_check=0
	
	rsasign_check "$file_path" "$rsa_path"
	sleep 2

	info "---- rsa check: $rsa_path $(nvram get ai_rsasign_check) ----"
	logger -t AI_RESCUE "rsa check: $rsa_path $(nvram get ai_rsasign_check)"

	if [ $(nvram get ai_rsasign_check) -ne 1 ]; then
		error "---- rsa check error: $rsa_path ----"
		logger -t AI_RESCUE "rsa check error: $rsa_path"
	    	return 1
	fi

    	return 0
}

download_file() {
    local download_url="$1"
    local download_file="$2"
    local local_filepath="$3"
    local download_result=0

    info "---- wget aisom reset file ${download_url}/${download_file} ----"
    logger -t AI_RESCUE "wget aisom reset file $download_file"

    wget --ciphers='DEFAULT:@SECLEVEL=1:!CAMELLIA:!3DES' \
         -t 2 \
         -T 30 \
         --no-check-certificate \
         --output-file=/tmp/ai_rst_fw_get_log \
         "${download_url}/${download_file}" \
         -O "$local_filepath"

    download_result=$?

    info "---- download aisom reset file, exit code: ${download_result} ----"
    logger -t AI_RESCUE "download aisom reset file exit code: ${download_result}"

    if [ "$download_result" -ne 0 ]; then
        error "Download failed: $download_file"
        logger -t AI_RESCUE "Download failed: $download_file"
        return 1
    fi

    return 0
}


verify_file() {
    local download_url="$1"
    local download_file="$2"
    local local_filepath="$3"
    local download_rsa_file="$4"
    local local_rsa_filepath="$5"
    
    local download_result=0
    local download_rsa_result=0
    local rsa_check_result=0

    if [ ! -f "$local_filepath" ]; then
    	download_file "$download_url" "$download_file" "$local_filepath"
	download_result=$?
    	download_file "$download_url" "$download_rsa_file" "$local_rsa_filepath"
	download_rsa_result=$?
    else
    	info "---- $local_filepath is already exist ----"
    	logger -t AI_RESCUE " $local_filepath is already exist"
    fi

    if [ ! -f "$local_rsa_filepath" ]; then
    	download_file "$download_url" "$download_rsa_file" "$local_rsa_filepath"
	download_rsa_result=$?
    else
    	info "---- $local_rsa_filepath rsa is already exist ----"
    	logger -t AI_RESCUE " $local_rsa_filepath rsa is already exist"
    fi
    
    nvram set ai_rsasign_check=0
    check_signature "$local_filepath" "$local_rsa_filepath"
    if [ $(nvram get ai_rsasign_check) -eq 1 ]; then
    	rsa_check_result=1
    	info "---- Verify $local_filepath $local_rsa_filepath success ----"
    	logger -t AI_RESCUE " Verify $local_filepath $local_rsa_filepath success"
	return 0
    else
    	rm -f $local_filepath
    	rm -f $local_rsa_filepath
    	error "---- Can't verify $local_filepath $local_rsa_filepath image ----"
    	logger -t AI_RESCUE " Can't verify $local_filepath $local_rsa_filepath image "
	return 1
    fi
}

fw_res=0
frigate_res=0
slm_res=0
portainer_res=0
ha_res=0
REFRESH_FLAG=0

verify_file "$dl_url" "$def_fw_file" "$fw_path" "$def_fw_rsa_file" "$fw_rsa_path"
fw_res="$?"

if [ "$AI_APPS_ENABLE" -eq 1 ]; then
	verify_file "$dl_url" "$def_frigate_file" "$frigate_path" "$def_frigate_rsa_file" "$frigate_rsa_path"
	frigate_res="$?"
	verify_file "$dl_url" "$def_slm_file" "$slm_path" "$def_slm_rsa_file" "$slm_rsa_path"
	slm_res="$?"
	if [ "$productid" == "GT-BE96_AI" ]; then
		verify_file "$dl_url" "$def_portainer_file" "$portainer_path" "$def_portainer_rsa_file" "$portainer_rsa_path"
		portainer_res="$?"
		verify_file "$dl_url" "$def_ha_file" "$ha_path" "$def_ha_rsa_file" "$ha_rsa_path"
		ha_res="$?"
	fi
fi

if [ "$fw_res" -eq 1 ]; then
	rm -f $fw_path
	rm -f $fw_rsa_path
	rm -f $frigate_path
	rm -f $frigate_rsa_path
	rm -f $slm_path
	rm -f $slm_rsa_path
	if [ "$productid" == "GT-BE96_AI" ]; then
		rm -f $portainer_path
		rm -f $portainer_rsa_path
		rm -f $ha_path
		rm -f $ha_rsa_path
	fi
	sleep 1

	REFRESH_FLAG=1
else
	if [ "$AI_APPS_ENABLE" -eq 1 ]; then
		if [ "$frigate_res" -eq 1 ] || [ "$slm_res" -eq 1 ] || [ "$portainer_res" -eq 1 ] || [ "$ha_res" -eq 1 ]; then
			rm -f $fw_path
			rm -f $fw_rsa_path
			rm -f $frigate_path
			rm -f $frigate_rsa_path
			rm -f $slm_path
			rm -f $slm_rsa_path
			if [ "$productid" == "GT-BE96_AI" ]; then
				rm -f $portainer_path
				rm -f $portainer_rsa_path
				rm -f $ha_path
				rm -f $ha_rsa_path
			fi
			sleep 1

			REFRESH_FLAG=1
		fi
	fi
fi

# If default file or default version file verification has failed, download and verify the file with FRS version
if [ "$REFRESH_FLAG" -eq 1 ]; then
	rm -f $fw_path
	rm -f $fw_rsa_path
	sleep 1

	verify_file "$dl_url" "$fw_file" "$fw_path" "$fw_rsa_file" "$fw_rsa_path"
	fw_res="$?"

	if [ "$AI_APPS_ENABLE" -eq 1 ]; then
		rm -f $frigate_path
		rm -f $frigate_rsa_path
		rm -f $slm_path
		rm -f $slm_rsa_path
		if [ "$productid" == "GT-BE96_AI" ]; then
			rm -f $portainer_path
			rm -f $portainer_rsa_path
			rm -f $ha_path
			rm -f $ha_rsa_path
		fi
		sleep 1

		verify_file "$dl_url" "$frigate_file" "$frigate_path" "$frigate_rsa_file" "$frigate_rsa_path"
		frigate_res="$?"
		verify_file "$dl_url" "$slm_file" "$slm_path" "$slm_rsa_file" "$slm_rsa_path"
		slm_res="$?"
		if [ "$productid" == "GT-BE96_AI" ]; then
			verify_file "$dl_url" "$portainer_file" "$portainer_path" "$portainer_rsa_file" "$portainer_rsa_path"
			portainer_res="$?"
			verify_file "$dl_url" "$ha_file" "$ha_path" "$ha_rsa_file" "$ha_rsa_path"
			ha_res="$?"
		fi
	fi
fi

if [ "$fw_res" -eq 1 ]; then
	rm -f $fw_path
	rm -f $fw_rsa_path
	rm -f $frigate_path
	rm -f $frigate_rsa_path
	rm -f $slm_path
	rm -f $slm_rsa_path
	if [ "$productid" == "GT-BE96_AI" ]; then
		rm -f $portainer_path
		rm -f $portainer_rsa_path
		rm -f $ha_path
		rm -f $ha_rsa_path
	fi
	update_ai_prog "failed" 400 "FW verification was failed" 0 0 0 0 0 "aisom_reset.swu"
	sleep 1

	exit 1
else
	if [ "$AI_APPS_ENABLE" -eq 1 ]; then
		if [ "$frigate_res" -eq 1 ] || [ "$slm_res" -eq 1 ] || [ "$portainer_res" -eq 1 ] || [ "$ha_res" -eq 1 ]; then
			rm -f $fw_path
			rm -f $fw_rsa_path
			rm -f $frigate_path
			rm -f $frigate_rsa_path
			rm -f $slm_path
			rm -f $slm_rsa_path
			if [ "$productid" == "GT-BE96_AI" ]; then
				rm -f $portainer_path
				rm -f $portainer_rsa_path
				rm -f $ha_path
				rm -f $ha_rsa_path
			fi
			
			if [ "$frigate_res" -eq 1 ]; then
				update_ai_prog "failed" 400 "Frigate verification was failed" 0 0 0 0 0 "frigate.tar"
			elif [ "$slm_res" -eq 1 ]; then
				update_ai_prog "failed" 400 "SLM verification was failed" 0 0 0 0 0 "slm.tar"
			elif [ "$portainer_res" -eq 1 ]; then
				update_ai_prog "failed" 400 "Portainer verification was failed" 0 0 0 0 0 "portainer.tar"
			elif [ "$ha_res" -eq 1 ]; then
				update_ai_prog "failed" 400 "HA verification was failed" 0 0 0 0 0 "homeassistant.tar"
			else
				update_ai_prog "failed" 400 "unknown verification was failed" 0 0 0 0 0 "unknown"
			fi
			
			sleep 1

			exit 1
		fi
	fi
fi

if [ "$AI_APPS_ENABLE" -eq 1 ]; then
	nvram set frigate_md5=$(md5sum $frigate_path | cut -d ' ' -f 1)
	nvram set slm_md5=$(md5sum $slm_path | cut -d ' ' -f 1)
	if [ "$productid" == "GT-BE96_AI" ]; then
		nvram set portainer_md5=$(md5sum $portainer_path | cut -d ' ' -f 1)
		nvram set ha_md5=$(md5sum $ha_path | cut -d ' ' -f 1)
	fi
fi

info "---- RSA verification end ----"
logger -t AI_RESCUE " RSA verification end "

##### Step 3 calculate md5 #####
nvram set ai_fw_md5=$(md5sum $fw_path | cut -d ' ' -f 1)

# set ai_fw_path for request
nvram set ai_fw_path="firmware/aisom_reset.swu"
nvram set webs_state_ai_upgrade=1

rc rc_service "start_ai_request update rescue"
sleep 3

info "---- Start to get into rescue mode ----"
logger -t AI_RESCUE "Start to get into rescue mode"

# pull GPIO 16 -> low -> high to reboot
echo 255 > /sys/class/leds/led_gpio_16/brightness
echo 0 > /sys/class/leds/led_gpio_16/brightness
# pull GPIO 21 -> low -> high to get into rescue mode (tiny linux) when boot
echo 255 > /sys/class/leds/led_gpio_21/brightness
sleep 5
echo 0 > /sys/class/leds/led_gpio_21/brightness

info "---- End of AI RESCUE Process. Please wait for 20 to 30 mins ----"
logger -t AI_RESCUE " End Of AI RESCUE Process. Please wait for 20 to 30 mins"
