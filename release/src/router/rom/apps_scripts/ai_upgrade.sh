#!/bin/sh

FILENAME="/ai/$(nvram get ai_fw_path)"
AI_TX_TIMEOUT=90
AI_UPG_TIMEOUT=300

log() {
    echo "[AI_MANUAL_UPGRADE][$(date '+%F %T')] $1" >> /ai/log/ai_manual_upgrade.log
}

echo "[AI_MANUAL_UPGRADE][$(date '+%F %T')] Start upgrade ai board" > /ai/log/ai_manual_upgrade.log
nvram set webs_state_ai_upgrade=0
nvram set webs_state_ai_error=0

if [ ! -f "$FILENAME" ]; then
    log "AI FW isn't exist [$FILENAME]"
    exit 1
fi

ERROR_STATE=$(nvram get webs_state_ai_error)
if [ "$ERROR_STATE" == "0" ]; then
    log "Init AI FW upgrade"
    UPGRADE_STAGE=$(nvram get webs_state_ai_upgrade)

    rc rc_service "start_ai_request update"
    if [ $? -ne 0 ]; then
        nvram set webs_state_ai_error=8
        log "Failed to send ai upgrade request"
        exit 1
    fi

    nvram set webs_state_ai_upgrade=1
    UPGRADE_STAGE=$(nvram get webs_state_ai_upgrade)
    # Wait for stage 1 → 2
    START_TIME=$(date +%s)
    while [ "$UPGRADE_STAGE" == "1" ]; do
        CURRENT_TIME=$(date +%s)
        if [ $((CURRENT_TIME - START_TIME)) -ge $AI_TX_TIMEOUT ]; then
            nvram set webs_state_ai_error=9
            break
        fi
        sleep 1
        UPGRADE_STAGE=$(nvram get webs_state_ai_upgrade)
    done

    ERROR_STATE=$(nvram get webs_state_ai_error)
    if [ "$ERROR_STATE" != "0" ]; then
        log "Transmit FW timeout [$ERROR_STATE]"
        exit 1
    fi

    # Wait for stage 2 → 3
    START_TIME=$(date +%s)
    while [ "$UPGRADE_STAGE" == "2" ]; do
        CURRENT_TIME=$(date +%s)
        if [ $((CURRENT_TIME - START_TIME)) -ge $AI_UPG_TIMEOUT ]; then
            nvram set webs_state_ai_error=10
            break
        fi
        sleep 1
        UPGRADE_STAGE=$(nvram get webs_state_ai_upgrade)
    done

    ERROR_STATE=$(nvram get webs_state_ai_error)
    if [ "$ERROR_STATE" != "0" ]; then
        log "Upgrade FW timeout [$ERROR_STATE] [$FILENAME]"
        exit 1
    fi
    if [ "$ERROR_STATE" == "0" ] && [ "$UPGRADE_STAGE" == "3" ]; then
        log "Succeed in Upgrading FW manually [$FILENAME]"
    fi
fi

# Final cleanup
if [ -f "$FILENAME" ]; then
    log "Remove [$FILENAME]"
    rm $FILENAME
fi
log "AI FW Upgrade END"
nvram set webs_state_ai_upgrade=0
nvram set webs_state_ai_error=0
exit 0
