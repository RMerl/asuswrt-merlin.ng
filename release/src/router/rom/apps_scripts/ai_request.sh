#!/bin/sh

info() {
    echo "[AI_REQUEST][$(date '+%F %T')][INFO] $1" >> /ai/log/ai_service.log
}

error() {
    echo "[AI_REQUEST][$(date '+%F %T')][ERROR] $1" >> /ai/log/ai_service.log
}

# (initialize) clean up 
nvram set ai_sys_status=""
nvram set ai_def_status=""
nvram set ai_sav_status=""
nvram set ai_comm_code=""

VALID_COMMANDS="query update apply reset feedback rescue docker app"
CUR_REQUEST="$(nvram get ai_request)"

# check if input arg(request cmd) has weird value/cmd
for arg in "$@"; do
	echo "$VALID_COMMANDS" | grep -qw "$arg"
	if [ $? -ne 0 ]; then
		error "Invalid command: $arg"
        	exit 0
    	fi
done

# check if current ai_request has weird value/cmd
CLEANUP=0
if [ ! -z "$CUR_REQUEST" ]; then
	for cmd in $CUR_REQUEST; do
		echo "$VALID_COMMANDS" | grep -qw "$cmd"
		if [ $? -ne 0 ]; then
	        	CLEANUP=1
	        	break
	    	fi
	done
fi

if [ $CLEANUP -eq 1 ]; then
    info "Invalid command [$CUR_REQUEST] detected, cleaning ai_request"
    nvram set ai_request=""
fi

# Wait for other request to finish
MAX_WAIT=60  # seconds
WAIT_INTERVAL=5
WAITED=0

while [ ! -z "$(nvram get ai_request)" ]; do
    info "Waiting for previous AI request to finish: $(nvram get ai_request)"
    sleep $WAIT_INTERVAL
    WAITED=$((WAITED + WAIT_INTERVAL))
    if [ "$WAITED" -ge "$MAX_WAIT" ]; then
	error "Timeout waiting for previous AI request $(nvram get ai_request). Replace AI request to $*"
        break
    fi
done

nvram set ai_request="$*"
ai_command=$1
response=0
retry=3

FRIGATE_PATH="/ai/docker_images/frigate.tar"
SLM_PATH="/ai/docker_images/slm.tar"
PORTAINER_PATH="/ai/docker_images/portainer.tar"

if [ "$ai_command" == "docker" ]; then
	nvram set frigate_md5=$(md5sum $FRIGATE_PATH | cut -d ' ' -f 1)
	nvram set slm_md5=$(md5sum $SLM_PATH | cut -d ' ' -f 1)
	nvram set portainer_md5=$(md5sum $PORTAINER_PATH | cut -d ' ' -f 1)	
fi

rc rc_service "start_ai_request_action $(nvram get ai_request)"

while [ "$retry" -gt 0 ]; do
    start_time=$(date +%s)
    timeout=10
    response=0

    while true; do
        current_time=$(date +%s)
        elapsed=$((current_time - start_time))
	case "$ai_command" in
		update|feedback|docker|app)
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
			if [ "$(nvram get ai_sav_status)" == "failure" ]; then
				break
			fi
			if [ "$(nvram get ai_sav_status)" == "success" ]; then
				response=1
				break
			fi
			;;
		*)
			error "Unknown command $ai_command"
			break
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
	    error "communication error | error_code: $(nvram get ai_comm_code)"
	    nvram set ai_comm_code=""
    fi

    retry=$((retry - 1))
    info "Retry attempt: $((3 - retry)) | AI REQ $ai_command"
    rc rc_service "start_ai_request_action $(nvram get ai_request)"
done

if [ "$response" -eq 1 ]; then
    info "AI REQ $(nvram get ai_request) Success"
else
    error "AI REQ $(nvram get ai_request) Fail"
fi

nvram set ai_request=""
