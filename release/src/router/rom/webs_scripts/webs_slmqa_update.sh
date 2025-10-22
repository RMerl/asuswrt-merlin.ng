#!/bin/sh

ROUTER_SLMQA_FILE="/usr/slmqa/slm-data.archive"
ROUTER_SLMQA_VER_FILE="/tmp/slm_version.json"
AIBOARD_SLMQA_VER_FILE="/ai/slmqa/qa_version.json"

AI_TX_TIMEOUT=90
AI_UPG_TIMEOUT=60

update_ai_prog(){
	nvram set ai_prog_status="$1"
}

info() {
    echo "[SLMQA_UPGRADE][$(date '+%F %T')][INFO] $1" >> /ai/log/slmqa_update.log
}

error() {
    echo "[SLMQA_UPGRADE][$(date '+%F %T')][ERROR] $1" >> /ai/log/slmqa_update.log

}

to_int() {
    ver="$1"
    major=${ver%%.*}      # part before first dot
    minor=${ver#*.}       # part after first dot
    printf "%d" $(( major * 100 + minor ))
}

tag_finish() {
    nvram set webs_state_slmqa_update_task="$1"
}
fetch_aiboard_slmqa_version() {
    ERROR_STATE=$(nvram get ai_prog_status)
    if [ "$ERROR_STATE" != "failed" ]; then
        info "Init SLMQA update"
        UPGRADE_STAGE=$(nvram get webs_state_slmqa_update)
        nvram set ai_app_name="get_slmqaver"
        rc rc_service "start_ai_request app"
        if [ $? -ne 0 ]; then
            nvram set ai_prog_status="failed"
            nvram set ai_prog_error_code="400"
            nvram set ai_prog_reason="Failed to send app request"
            error "Failed to send slmqa ver fetch request"
            nvram set ai_app_name=""
            tag_finish "1"
            nvram set webs_state_slmqa_update=0
            exit 0
        fi

        # current we bypass download from server part temporarily
        nvram set webs_state_slmqa_update=2
        UPGRADE_STAGE=$(nvram get webs_state_slmqa_update)
        # Wait for stage 2 → 3
        START_TIME=$(date +%s)
        while [ "$UPGRADE_STAGE" == "2" ]; do
            CURRENT_TIME=$(date +%s)
            if [ $((CURRENT_TIME - START_TIME)) -ge $AI_UPG_TIMEOUT ]; then
                nvram set ai_prog_status="failed"
                nvram set ai_prog_error_code="400"
                nvram set ai_prog_reason="SLMQA data ver fetch  failed"
                nvram set ai_app_name=""
                break
            fi
            sleep 1
            UPGRADE_STAGE=$(nvram get webs_state_slmqa_update)
        done

        ERROR_STATE=$(nvram get ai_prog_status)
        if [ "$ERROR_STATE" == "failed" ]; then
            error "fetch SLMQA ver file timeout [$ERROR_STATE] [$AIBOARD_SLMQA_VER_FILE]"
            nvram set ai_app_name=""
            tag_finish "1"
            nvram set webs_state_slmqa_update=0
            exit 0
        fi
        if [ "$ERROR_STATE" != "failed" ] && [ "$UPGRADE_STAGE" == "3" ]; then
            info "Succeed fetch SLMQA [$AIBOARD_SLMQA_VER_FILE]"
            nvram set ai_app_name=""
            nvram set nvram set ai_app_prog_current_image=""
        fi
    fi
}

echo "[SLMQA_UPDATE][$(date '+%F %T')][INFO] Start fetch slmqa ver file" > /ai/log/slmqa_update.log

# TODO: avoid potential re-entry (watchdog might call webs_update.sh (frs_services)) twice in short time
check_update_running=$(nvram get webs_state_slmqa_update)
if [ "$check_update_running" == '1' ] || [ "$check_update_running" == '2' ]; then
    info "another webs_slmaqa_update.sh is running. abort"
    tag_finish "1"
    exit 0
fi

nvram set webs_state_slmqa_update=1
nvram set webs_state_slmqa_error=0

# always fetch new qa_version.json to avoid potential issue of time sync
now=`date +%s`                # current epoch seconds
info "At $now and fetch new qa_version.json"
update_ai_prog ""
fetch_aiboard_slmqa_version
AIBOARD_SLMQA_VER=$(jq -r '.ver' $AIBOARD_SLMQA_VER_FILE)
AIBOARD_SLMQA_SKU=$(jq -r '.sku' $AIBOARD_SLMQA_VER_FILE)
AIBOARD_SLMQA_DATE=$(jq -r '.date' $AIBOARD_SLMQA_VER_FILE)
AIBOARD_SLMQA_LAST_UPDATE=$(jq -r '.last_update' $AIBOARD_SLMQA_VER_FILE)

info "get version aiboard slmqa version [$AIBOARD_SLMQA_VER] sku [$AIBOARD_SLMQA_SKU] date [$AIBOARD_SLMQA_DATE] last_update [$AIBOARD_SLMQA_LAST_UPDATE]"

info "extract version.json from router slmqa file"
tar -xvf $ROUTER_SLMQA_FILE ./version.json -C /tmp/
cp /tmp/version.json $ROUTER_SLMQA_VER_FILE

ROUTER_SLMQA_VER=$(jq -r '.ver' $ROUTER_SLMQA_VER_FILE)
ROUTER_SLMQA_SKU=$(jq -r '.sku' $ROUTER_SLMQA_VER_FILE)
ROUTER_SLMQA_DATE=$(jq -r '.date' $ROUTER_SLMQA_VER_FILE)

info "get version builtin slmqa version [$ROUTER_SLMQA_VER] sku [$ROUTER_SLMQA_SKU] date [$ROUTER_SLMQA_DATE]"

router_ver=$(to_int $ROUTER_SLMQA_VER)
aiboard_ver=$(to_int $AIBOARD_SLMQA_VER)

info "router_ver: $router_ver, aiboard_ver: $aiboard_ver"
if [ "$AIBOARD_SLMQA_SKU" == "$ROUTER_SLMQA_SKU" ] && [ $aiboard_ver -lt $router_ver ]; then
    info "Need update slmqa data, aiboard ver [$AIBOARD_SLMQA_VER] router ver [$ROUTER_SLMQA_VER]"
    nvram set webs_state_slmqa_update=3
    nvram set webs_state_slmqa_error=0
    nvram set webs_state_slmqa_flag=2
else
    info "No need update slmqa data, aiboard ver [$AIBOARD_SLMQA_VER] router ver [$ROUTER_SLMQA_VER]"
    nvram set webs_state_slmqa_update=0
fi

# update slmqa related nvram
nvram set slmqa_data_ver_aiboard="$AIBOARD_SLMQA_VER"
nvram set slmqa_data_sku_aiboard="$AIBOARD_SLMQA_SKU"
nvram set slmqa_data_ver_airouter="$ROUTER_SLMQA_VER"
nvram set slmqa_data_sku_airouter="$ROUTER_SLMQA_SKU"


tag_finish "1"
info "SLMQA fetch version END"
exit 0
