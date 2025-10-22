#!/bin/sh

FILENAME="/ai/$(nvram get ai_fw_path)"
AI_TX_TIMEOUT=90
AI_UPG_TIMEOUT=300

info() {
    echo "[AI_UPGRADE][$(date '+%F %T')][INFO] $1" >> /ai/log/ai_upgrade.log
}

error() {
    echo "[AI_UPGRADE][$(date '+%F %T')][ERROR] $1" >> /ai/log/ai_upgrade.log
}

echo "[AI_UPGRADE][$(date '+%F %T')][INFO] Start upgrade ai board" > /ai/log/ai_upgrade.log
nvram set webs_state_ai_upgrade=0
nvram set webs_state_ai_error=0

if [ ! -f "$FILENAME" ]; then
    error "AI FW isn't exist [$FILENAME]"
    nvram set ai_prog_status="failed"
    nvram set ai_prog_error_code="400"
    nvram set ai_prog_reason="AI FW isn't exist"
    exit 0
else
    nvram set ai_fw_md5=$(md5sum $FILENAME | cut -d ' ' -f 1)
fi

ERROR_STATE=$(nvram get ai_prog_status)
if [ "$ERROR_STATE" != "failed" ]; then
    info "Init AI FW upgrade"
    UPGRADE_STAGE=$(nvram get webs_state_ai_upgrade)

    rc rc_service "start_ai_request update"
    if [ $? -ne 0 ]; then
    	nvram set ai_prog_status="failed"
    	nvram set ai_prog_error_code="400"
    	nvram set ai_prog_reason="Failed to send update request"
        error "Failed to send ai upgrade request"
        exit 0
    fi

    nvram set webs_state_ai_upgrade=1
    UPGRADE_STAGE=$(nvram get webs_state_ai_upgrade)
    # Wait for stage 1 → 2
    START_TIME=$(date +%s)
    while [ "$UPGRADE_STAGE" == "1" ]; do
        CURRENT_TIME=$(date +%s)
        if [ $((CURRENT_TIME - START_TIME)) -ge $AI_TX_TIMEOUT ]; then
    	    nvram set ai_prog_status="failed"
    	    nvram set ai_prog_error_code="400"
    	    nvram set ai_prog_reason="FW transmit was failed"
            break
        fi
        sleep 1
        UPGRADE_STAGE=$(nvram get webs_state_ai_upgrade)
    done

    ERROR_STATE=$(nvram get ai_prog_status)
    if [ "$ERROR_STATE" == "failed" ]; then
        error "Transmit FW timeout [$ERROR_STATE] [$FILENAME]"
        exit 0
    fi

    # Wait for stage 2 → 3
    START_TIME=$(date +%s)
    while [ "$UPGRADE_STAGE" == "2" ]; do
        CURRENT_TIME=$(date +%s)
        if [ $((CURRENT_TIME - START_TIME)) -ge $AI_UPG_TIMEOUT ]; then
    	    nvram set ai_prog_status="failed"
    	    nvram set ai_prog_error_code="400"
    	    nvram set ai_prog_reason="FW upgrade was failed"
            break
        fi
        sleep 1
        UPGRADE_STAGE=$(nvram get webs_state_ai_upgrade)
    done

    ERROR_STATE=$(nvram get ai_prog_status)
    if [ "$ERROR_STATE" == "failed" ]; then
        error "Upgrade FW timeout [$ERROR_STATE] [$FILENAME]"
        exit 0
    fi
    if [ "$ERROR_STATE" != "failed" ] && [ "$UPGRADE_STAGE" == "3" ]; then
        info "Succeed in Upgrading FW [$FILENAME]"
    fi
fi

# Final cleanup
if [ -f "$FILENAME" ]; then
    info "Remove [$FILENAME]"
    rm -f $FILENAME
fi
info "AI FW Upgrade END"
nvram set webs_state_ai_upgrade=0
nvram set webs_state_ai_error=0
exit 0
