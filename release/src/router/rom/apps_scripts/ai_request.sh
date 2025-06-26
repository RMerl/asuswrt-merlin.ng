#!/bin/sh

log() {
    echo "[AI_REQUEST][$(date '+%F %T')] $1" >> /ai/log/ai_service.log
}

if [ ! -z "$(nvram get ai_request)" ]; then
	log "Currently is running AI REQ $(nvram get ai_request)"
	exit 0
fi

nvram set ai_request="$@"
ai_command=$1
response=0
retry=3

rc rc_service "start_ai_request_action $(nvram get ai_request)"

while [ "$retry" -gt 0 ]; do
    start_time=$(date +%s)
    timeout=10
    success=0

    while true; do
        current_time=$(date +%s)
        elapsed=$((current_time - start_time))
	case "$ai_command" in
		update|feedback)
			# update and feedback don't need to retry
			response=1
			break
			;;
		query)
			if [ "$(nvram get ai_sys_status)" == "success" ]; then
				response=1
				break
			fi
			if [ "$(nvram get ai_sys_status)" == "failure" ]; then
				break
			fi
			;;
		reset)
			if [ "$(nvram get ai_def_status)" == "success" ]; then
				response=1
				break
			fi
			if [ "$(nvram get ai_def_status)" == "failure" ]; then
				break
			fi
			;;
		apply)
			if [ "$(nvram get ai_sav_res_ssh_enable_status)" == "failure" ] || [ "$(nvram get ai_sav_res_lan_setting_status)" == "failure" ];then
				break
			fi
			if [ "$(nvram get ai_sav_res_ssh_enable_status)" == "success" ] && [ "$(nvram get ai_sav_res_lan_setting_status)" == "success" ];then
				if [ -z "$(nvram get ai_ssh_pass)" ]; then
					response=1
					break
				else
					if [ "$(nvram get ai_sav_res_ssh_account_password_status)" == "success" ]; then
						response=1
						break
					fi
				fi
			fi
			;;
		*)
			log "Unknown command $ai_command"
			;;
	esac
	if [ ! -z "$(nvram get ai_comm_code)" ]; then
		break
	fi
        if [ "$elapsed" -ge "$timeout" ]; then
            break
        fi

        sleep 1
    done
    if [ "$response" == "1" ]; then
	    break
    fi
    if [ ! -z "$(nvram get ai_comm_code)" ]; then
	    log "communication error | error_code: $(nvram get ai_comm_code)"
	    nvram set ai_comm_code=""
    fi

    retry=$((retry - 1))
    log "Retry attempt: $((4 - retry)) | AI REQ $ai_command"
    rc rc_service "start_ai_request_action $(nvram get ai_request)"
done

if [ "$success" -eq 1 ]; then
    log "AI REQ $(nvram get ai_request) Success"
else
    log "AI REQ $(nvram get ai_request) Fail"
fi

# clean up 
nvram set ai_ssh_acc=""
nvram set ai_ssh_pass=""
nvram set ai_sys_status=""
nvram set ai_def_status=""
nvram set ai_sav_res_ssh_enable_status=""
nvram set ai_sav_res_ssh_account_password_status=""
nvram set ai_sav_res_lan_setting_status=""
nvram set ai_fw_path=""
nvram set ai_sav_status=""
nvram set ai_comm_code=""
nvram set ai_request=""
