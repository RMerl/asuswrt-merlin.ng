#!/bin/sh

FILENAME="/ai/slmqa/slm-data.tar"
AI_TX_TIMEOUT=90
AI_UPG_TIMEOUT=120

slmqa_data_path="/tmp/slmqa_data.tar"
slmqa_rsa_path="/tmp/slmqa_data_rsa.zip"

AIBOARD_SLMQA_VER_FILE="/ai/slmqa/qa_version.json"

update_ai_prog(){
	nvram set ai_prog_status="$1"
}

info() {
    echo "[SLMQA_UPGRADE][$(date '+%F %T')][INFO] $1" >> /ai/log/slmqa_upgrade.log
}

error() {
    echo "[SLMQA_UPGRADE][$(date '+%F %T')][ERROR] $1" >> /ai/log/slmqa_upgrade.log
}

to_int() {
    ver="$1"
    major=${ver%%.*}      # part before first dot
    minor=${ver#*.}       # part after first dot
    printf "%d" $(( major * 100 + minor ))
}

frs_slmqa_ver_is_newer() {
    frs_sku=`nvram get slmqa_data_sku`
    frs_ver=`nvram get slmqa_data_ver`
    aiboard_sku=`nvram get slmqa_data_sku_aiboard`
    aiboard_ver=`nvram get slmqa_data_ver_aiboard`
    if [ "$frs_sku" == "$aiboard_sku" ] && [ "$frs_ver" != "$aiboard_ver" ]; then
        int_ver1=$(to_int $frs_ver)
        int_ver2=$(to_int $aiboard_ver)
        if [ $int_ver1 -gt $int_ver2 ]; then
            echo 1
            return
        fi
    fi

    echo 0
}

router_slmqa_ver_is_newer() {
    aiboard_sku=`nvram get slmqa_data_sku_aiboard`
    aiboard_ver=`nvram get slmqa_data_ver_aiboard`
    router_sku=`nvram get slmqa_data_sku_airouter`
    router_ver=`nvram get slmqa_data_ver_airouter`
    if [ "$router_sku" == "$aiboard_sku" ] && [ "$router_ver" != "$aiboard_ver" ]; then
        int_ver1=$(to_int $router_ver)
        int_ver2=$(to_int $aiboard_ver)
        if [ $int_ver1 -gt $int_ver2 ]; then
            echo 1
            return
        fi
    fi
 
    echo 0
}


echo "[SLMQA_UPGRADE][$(date '+%F %T')][INFO] Start upgrade slmqa data" > /ai/log/slmqa_upgrade.log

nvram set webs_state_slmqa_error=0

# Fetch frs slmqa-${sku}-${ver}.tar
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
slmqa_data_file=slmqa-`nvram get productid`-`nvram get slmqa_data_sku`-`nvram get slmqa_data_ver`.tar
slmqa_data_rsasign=slmqa-`nvram get productid`-`nvram get slmqa_data_sku`-`nvram get slmqa_data_ver`_rsa`nvram get live_update_rsa_ver`.zip


slmqa_wget_result=0
slmqa_wget_result2=0
	
if [ "$update_url" != "" ]; then
    slmqa_dl_url=${update_url}
elif [ "$betaupg_support" != "" ] && [ "$forbeta" == "1" ]; then
    slmqa_dl_url=${dl_path_SQ}
elif [ "$forsq" -ge 2 ] && [ "$forsq" -le 9 ]; then
    slmqa_dl_url=${dl_path_SQ_beta}${forsq}
elif [ "$forsq" == "1" ]; then
    slmqa_dl_url=${dl_path_SQ}
elif [ "$urlpath" == "" ]; then
    slmqa_dl_url=${dl_path_file}
else
    slmqa_dl_url=${urlpath}
fi

rm $FILENAME
info "---- remove old  $FILENAME----"


if [ "$(frs_slmqa_ver_is_newer)" -eq 1 ]; then
    # download slmqa data file
    info "---- wget slmqa data url ${slmqa_dl_url}/$slmqa_data_file ----"
	logger -t SMLQA_AUTO_UPGRADE "wget smlqa data $slmqa_data_file"
	
	wget --ciphers='DEFAULT:@SECLEVEL=1:!CAMELLIA:!3DES' -t 2 -T 30 --no-check-certificate --output-file=/tmp/fwget_log ${slmqa_dl_url}/$slmqa_data_file -O $slmqa_data_path
	slmqa_wget_result=$?

	info "---- wget slmqa data url, exit code: ${slmqa_wget_result} ----"
	logger -t SMLQA_AUTO_UPGRADE "exit code: ${slmqa_wget_result}"

	info "---- wget rsa slmqa data ${slmqa_dl_url}/$slmqa_rsa_path ----"
	logger -t SMLQA_AUTO_UPGRADE "wget rsa slmqa data $slmqa_rsa_path"
	
	wget --ciphers='DEFAULT:@SECLEVEL=1:!CAMELLIA:!3DES' -t 2 -T 30 --no-check-certificate ${slmqa_dl_url}/$slmqa_data_rsasign -O $slmqa_rsa_path
	slmqa_wget_result2=$?
	info "---- wget rsa slmqa data, exit code: ${slmqa_wget_result2} ----"
	logger -t SMLQA_AUTO_UPGRADE "exit code: ${slmqa_wget_result2}"

    # priority frs -> router -> aiboard
    if [ "$slmqa_wget_result" != "0" ] || [ "$slmqa_wget_result2" != "0" ]; then
        error "---- download slmqa data failure, End ----"
		logger -t SMLQA_AUTO_UPGRADE "download slmqa data failure, End"
    else
        info "---- verify slmqa data signature ----"
		logger -t SMLQA_AUTO_UPGRADE "verify slmqa data signature"
        nvram set ai_rsasign_check=0
		rsasign_check $slmqa_data_path $slmqa_rsa_path
		sleep 1
        
        slmqa_rsasign_check_ret=`nvram get ai_rsasign_check`
	
        if [ "$slmqa_rsasign_check_ret" == "1" ]; then
            info "---- verify slmqa data signature success ----"
		    logger -t SMLQA_AUTO_UPGRADE "verify slmqa data signature success"
            nvram set ai_rsasign_check=0
            cp $slmqa_data_path $FILENAME
            info "---- update $FILENAME from $slmqa_data_path ----"
		    logger -t SMLQA_AUTO_UPGRADE "update $FILENAME from $slmqa_data_path"
           
        fi
    fi

fi

# TODO: check local /ai/slm-data.tar is newer than download one and builtin one
if [ ! -f "$FILENAME" ]; then
    error "SLMQA data isn't exist [$FILENAME]"
    if [ "$(router_slmqa_ver_is_newer)" -eq 1 ]; then
        cp /usr/slmqa/slm-data.archive $FILENAME
        info "---- fallthrough to use router slmqa data ----" 
    fi
    if [  ! -f "$FILENAME" ]; then
        nvram set ai_prog_status="failed"
        nvram set ai_prog_error_code="400"
        nvram set ai_prog_reason="SLMQA data isn't exist"
        exit 0
    fi
fi

do_upgrade=$(nvram get webs_state_slmqa_upgrade)
if [ "$do_upgrade" == "2"  ] || [ "$do_upgrade" = "3" ]; then
    info "another upgrade process is running [$do_upgrade], exit"
    exit 0
fi

update_ai_prog ""

ERROR_STATE=$(nvram get ai_prog_status)
if [ "$ERROR_STATE" != "failed" ]; then
    info "Init SLMQA upgrade"
    UPGRADE_STAGE=$(nvram get webs_state_slmqa_upgrade)
    nvram set ai_app_name="update_slmqa"
    rc rc_service "start_ai_request app"
    if [ $? -ne 0 ]; then
        nvram set ai_prog_status="failed"
        nvram set ai_prog_error_code="400"
        nvram set ai_prog_reason="Failed to send app request"
        error "Failed to send slmqa upgrade request"
        nvram set ai_app_name=""
        exit 0
    fi

    # current we bypass download from server part temporarily
    nvram set webs_state_slmqa_upgrade=2
    UPGRADE_STAGE=$(nvram get webs_state_slmqa_upgrade)
    # Wait for stage 2 → 3
    START_TIME=$(date +%s)
    while [ "$UPGRADE_STAGE" == "2" ]; do
        CURRENT_TIME=$(date +%s)
        if [ $((CURRENT_TIME - START_TIME)) -ge $AI_UPG_TIMEOUT ]; then
            nvram set ai_prog_status="failed"
            nvram set ai_prog_error_code="400"
            nvram set ai_prog_reason="SLMQA data upgrade was failed"
            nvram set ai_app_name=""
            break
        fi
        sleep 1
        UPGRADE_STAGE=$(nvram get webs_state_slmqa_upgrade)
    done

    ERROR_STATE=$(nvram get ai_prog_status)
    if [ "$ERROR_STATE" == "failed" ]; then
        error "Upgrade SLMQA timeout [$ERROR_STATE] [$FILENAME]"
        nvram set ai_app_name=""
        nvram set webs_state_slmqa_upgrade=0
        exit 0
    fi
    if [ "$ERROR_STATE" != "failed" ] && [ "$UPGRADE_STAGE" == "3" ]; then
        info "Succeed in Upgrading SLMQA [$FILENAME]"
        nvram set webs_state_slmqa_upgrade=0
        nvram set webs_state_slmqa_update=0
        nvram set webs_state_slmqa_error=0
        nvram set webs_state_slmqa_flag=0
        nvram set ai_app_name=""
        nvram set nvram set ai_app_prog_current_image=""
    fi
fi

# final update ai board version
if [ -f "$AIBOARD_SLMQA_VER_FILE" ]; then
    AIBOARD_SLMQA_VER=$(jq -r '.ver' $AIBOARD_SLMQA_VER_FILE)
    AIBOARD_SLMQA_SKU=$(jq -r '.sku' $AIBOARD_SLMQA_VER_FILE)
    nvram set slmqa_data_ver_aiboard="$AIBOARD_SLMQA_VER"
    nvram set slmqa_data_sku_aiboard="$AIBOARD_SLMQA_SKU"
fi


info "SLMQA data Upgrade END"

exit 0
