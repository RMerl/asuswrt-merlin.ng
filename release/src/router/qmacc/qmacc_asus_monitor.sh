#!/bin/sh

exec 2>/tmp/qmacc_monitor.log || true
set -x

# Online URL
URL_PREFIX="https://"
DOWNLOAD_URL="n.guanjia.qq.com/cgi-xnews/router/text?type="
UNINSTALL_DOWNLOAD_URL="n.guanjia.qq.com/cgi-xnews/router/uninstall/text?type="

ROUTER=""
MODEL=""

ASUSWRT_MERLIN="asuswrt-merlin"
XIAOMI="xiaomi"
HIWIFI="hiwifi"

BASEDIR="$(cd "$(dirname "$0")"; pwd -P)"
MONITOR_CONFIG="${BASEDIR}/qmacc_monitor.config"

MONITOR_FILE="qmplugin_monitor.sh"
RUNNING_DIR="/tmp/qm"
PLUGIN_DIR=""
PID_FILE="/var/run/qmacc.pid"
TEST_FILE="/tmp/qmplugin/test"
PLUGIN_EXE="qmacc-asus"
PLUGIN_TAR="qm.tar.gz"
UPDATE_FILE="qmplugin.update"
UNINSTALL_FILE="qm.uninstall"
PLUGIN_TAR_MD5_FILE="qm.tar.gz.md5"
PLUGIN_CFG="qmplugin_cfg.conf"
QMACC_EXCEPT="/tmp/qm/qmacc.except"

trap ignore_sighup SIGHUP
ignore_sighup() { 
    true;
}

get_mount_point() {
    [ $# -ne 1 ] && return 1

    local mount_point=$1
    while true
    do
        if [ "$mount_point" = "/" ]
        then
            break
        fi

        if [ ! -z "$(df -k | grep -m 1 -E "[ \t]+$mount_point[ \t]*$" | grep -v 'grep')" ]
        then
            break
        fi

        mount_point=$(dirname $mount_point)
    done
    echo $mount_point
    return 0
}

init_param() {
    PLUGIN_DIR="/jffs/qm"

    PLUGIN_MOUNT_DIR=$(get_mount_point "${PLUGIN_DIR}") || return 1
    [ ! -d "${PLUGIN_MOUNT_DIR}" ] && return 1
    return 0
}

# $1: FileName of which md5sum to be checked.
# $2: FileName that contains md5.
# Return: 0 means success.
check_md5sum() {
    [ ! -f "$1" -o ! -f "$2" ] && return 1

    local plugin_md5=$(md5sum "$1")
    [ "$?" != "0" ] && return 1

    local right_md5=$(cat "${2}")
    [ "$?" != "0" ] && return 1

    plugin_md5=$(echo "$plugin_md5" | sed 's/[ ][ ]*/ /g' | cut -d' ' -f1)
	# TO LOWER
    right_md5=$(echo "${right_md5}" | sed 'y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/')
    plugin_md5=$(echo "${plugin_md5}" | sed 'y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/')
    if [ "$right_md5" != "$plugin_md5" ];then
        echo "Error: md5 does not match."
        return 1
    fi
    return 0
}

asuswrt_download_url_init() {
    DOWNLOAD_URL="${URL_PREFIX}${DOWNLOAD_URL}asus-tob"
    UNINSTALL_DOWNLOAD_URL="${URL_PREFIX}${UNINSTALL_DOWNLOAD_URL}asuswrt-merlin"

    if [ -f "$TEST_FILE" ];then
        DOWNLOAD_URL="${DOWNLOAD_URL}&test=1"
        UNINSTALL_DOWNLOAD_URL="${UNINSTALL_DOWNLOAD_URL}&test=1"
    fi

    echo ${DOWNLOAD_URL}

    return 0
}

system_init() {
    ulimit -HS -s 8192
}

check_dir() {
    if [ ! -d "${RUNNING_DIR}" ];then
        mkdir -p "${RUNNING_DIR}" >/dev/null 2>&1
        [ "$?" != "0" ] && return 1
    fi

    if [ ! -d "${PLUGIN_DIR}" ];then
        mkdir -p "${PLUGIN_DIR}" >/dev/null 2>&1
        [ "$?" != "0" ] && return 1
    fi
    return 0
}

# Return: 0 means running.
check_running() {
    if [ -f "$PID_FILE" ];then
        pid=$(cat $PID_FILE)
        running_pid=$(ps | sed 's/^[ \t ]*//g;s/[ \t ]*$//g' | \
            sed 's/[ ][ ]*/#/g' | grep "${PLUGIN_EXE}" | \
            grep -v "grep" | cut -d'#' -f1 | grep -e "^${pid}$")
        if [ "$pid" = "${running_pid}" ];then
            return 0
        fi
    fi
	echo "1" > "${QMACC_EXCEPT}"
    return 1
}

# $1: FileName of which md5sum will be saved.
# $2: FileName where md5sum will be saved.
save_md5sum() {
    [ ! -f "${1}" ] && return
    touch "${2}"

    local filemd5sum=$(md5sum "$1")
    filemd5sum=$(echo "${filemd5sum}" | sed 's/[ ][ ]*/ /g' | cut -d' ' -f1)
    echo "File md5sum: ${filemd5sum}"
    echo "${filemd5sum}" > "${2}"
}

# Return: 0 means success.
download() {
    local url="$1"
    local file="$2"
    local plugin_info=$(curl -s -k -H "Accept:text/plain" "${url}" || \
        wget --header=Accept:text/plain -q --no-check-certificate -O - "${url}"
    )

    [ "$?" != "0" ] && return 1
    [ -z "$plugin_info" ] && return 1

    local plugin_url=$(echo "$plugin_info" | cut  -d ',' -f 1)
    local plugin_md5=$(echo "$plugin_info" | cut  -d ',' -f 2)

    [ -z "${plugin_url}" ] && return 1
    [ -z "${plugin_md5}" ] && return 1

    wget -q "$plugin_url" -O "${file}" >/dev/null 2>&1 || \
        curl -k "$plugin_url" -o "${file}" >/dev/null 2>&1

    if [ "$?" != "0" ];then
        echo "Failed: curl -s -k ${plugin_url} -o ${file} ||
            wget -q --no-check-certificate $plugin_url -O ${file}"
        # Clean up
        [ -f "${file}" ] && rm "${file}"
        return 1
    fi

    if [ ! -e "${file}" ];then
        curl -k "$plugin_url" -o "${file}" >/dev/null 2>&1 || \
            wget -q "$plugin_url" -O "${file}" >/dev/null 2>&1
    fi

    if [ -f "${file}" ];then
        local download_md5=$(md5sum "${file}")
        local download_md5=$(echo "$download_md5" | sed 's/[ ][ ]*/ /g' | cut -d' ' -f1)
        # TO LOWER
        download_md5=$(echo "${download_md5}" | sed 'y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/')
        plugin_md5=$(echo "${plugin_md5}" | sed 'y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/')
        if [ "$download_md5" != "$plugin_md5" ];then
            rm "${file}"
            return 1
        fi
        return 0
    else
        return 1
    fi
}

# $1: FileName to where content to be saved.
# Return: 0 means success.
download_acc() {
    download "${DOWNLOAD_URL}" "$1"
    return $?
}

start_acc() {
    # Start order:
    # 1. ${exefile}
    # 2. ${runtar}
    # 3. ${backtar}

    local exefile="${RUNNING_DIR}/${PLUGIN_EXE}"
    local runtar="${RUNNING_DIR}/${PLUGIN_TAR}"
    local backtar="${PLUGIN_DIR}/${PLUGIN_TAR}"

    if [ -e "${exefile}" ];then
        chmod u+x "${exefile}"
        ${exefile} >/dev/null 2>&1 &
        return
    fi

    if [ -f "${runtar}" ];then
        tar zxvf "${runtar}" -C "${RUNNING_DIR}" 
        if [ "$?" = "0" ];then
            chmod u+x "${exefile}"
            ${exefile} >/dev/null 2>&1 &

            # ${runtar} is not needed any more.
            rm "${runtar}"
            return
        fi
    fi

    if [ -f "${backtar}" ];then
        check_md5sum "${backtar}" "${PLUGIN_DIR}/${PLUGIN_TAR_MD5_FILE}"
        if [ "$?" != "0" ];then
            # Download a new one. Next time ${runtar} will be used.
            download_acc "${runtar}"
			rm "${RUNNING_DIR}/${UPDATE_FILE}"
            return
        fi

        tar zxvf "${backtar}" -C "${RUNNING_DIR}"
        if [ "$?" != "0" ];then
            return
        fi

        chmod u+x "${exefile}"
        ${exefile} >/dev/null 2>&1 &
        return
    else
        # Download a new one. Next time ${runtar} will be used.
        download_acc "${runtar}"
		rm "${RUNNING_DIR}/${UPDATE_FILE}"
        return
    fi
}

# Return: 0 means update flag is set.
check_update() {
	# get latest plugin release info
    local latest_plugin_info=$(curl -s -k -H "Accept:text/plain" "${DOWNLOAD_URL}" || \
        wget -q --no-check-certificate -O - "${DOWNLOAD_URL}"
    )

    [ "$?" != "0" ] && return
    [ -z "$latest_plugin_info" ] && return 1

    local latest_plugin_md5=$(echo "$latest_plugin_info" | cut  -d ',' -f 2)
    [ -z "${latest_plugin_md5}" ] && return 1

    # get local md5
    local local_plugin_md5=$(cat ${PLUGIN_DIR}/${PLUGIN_TAR_MD5_FILE} 2>/dev/null)
    [ "$?" != "0" ] && return 1
	
	#get latest plugin version
	local latest_plugin_version=$(echo "$latest_plugin_info" | cut  -d ',' -f 3)
	[ -z "${latest_plugin_version}" ] && return 1
	
	#get local plugin version
	local local_plugin_version=$(cat ${RUNNING_DIR}/${PLUGIN_CFG} | grep "version" | cut -d'=' -f2 | head -n1)
	
	local latest_version_num=$(echo "$latest_plugin_version" | cut  -d '.' -f 4)
	local local_version_num=$(echo "$local_plugin_version" | cut  -d '.' -f 4)
	if [ $local_version_num -gt $latest_version_num ]; then
		return 1
	fi
	
	local latest_update_force=$(echo "$latest_plugin_info" | cut  -d ',' -f 4)

    #compare md5
	local_plugin_md5=$(echo "${local_plugin_md5}" | sed 'y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/')
    latest_plugin_md5=$(echo "${latest_plugin_md5}" | sed 'y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/')

    if [ "$local_plugin_md5" != "$latest_plugin_md5" ]; then
        # create update file
        touch "${RUNNING_DIR}/${UPDATE_FILE}"
		if [ $latest_update_force -eq 1 ];then
			echo 1 > "${RUNNING_DIR}/${UPDATE_FILE}"
		else
			echo 0 > "${RUNNING_DIR}/${UPDATE_FILE}"
		fi
    fi

    if [ -f "${RUNNING_DIR}/${UPDATE_FILE}" ];then
		rm "${QMACC_EXCEPT}"
        return 0
    else
        return 1
    fi
}

# Return: 0 means uninstall flag is set.
check_uninstall() {
    if [ -f "${RUNNING_DIR}/${UNINSTALL_FILE}" ];then
        return 0
    else
        return 1
    fi
}

check_acc() {
    cd $RUNNING_DIR

    check_running
    [ "$?" = "0" ] && return
	
	check_uninstall
    if [ "$?" = "0" ];then
        # Download uninstall script to uninstall ourself.
        local uninstall_script="${RUNNING_DIR}/uninstall.sh"
        local uninstall_flag="${RUNNING_DIR}/${UNINSTALL_FILE}"

        download "${UNINSTALL_DOWNLOAD_URL}" "${uninstall_script}"
        if [ "$?" != "0" ];then
            [ -f "${uninstall_flag}" ] && rm ${uninstall_flag}
            [ -f "${uninstall_script}" ] && rm ${uninstall_script}
            start_acc
            return
        else
            chmod u+x ${uninstall_script}
            /bin/sh ${uninstall_script} "${ROUTER}" "${MODEL}" >/dev/null 2>&1 &
            # Waiting to be uninstalled.
            sleep 20
            return
        fi
    fi
	
	check_update
    if [ "$?" != "0" ];then
        # "qmplugin" is not running, and no need to be updated.
        # Just try to start again.
        start_acc
        return
    fi

    # Try to update.
    local exefile="${RUNNING_DIR}/${PLUGIN_EXE}"
    local runtar="${RUNNING_DIR}/${PLUGIN_TAR}"
    local backtar="${PLUGIN_DIR}/${PLUGIN_TAR}"

    download_acc "${runtar}"
    if [ "$?" != "0" ];then
        # Download failed; Just try to start again.
        [ -f "${runtar}" ] && rm "${runtar}"
        start_acc
        return
    fi

    rm $(ls | grep -v "$MONITOR_FILE" | grep -v "$PLUGIN_TAR")
    # 刚下载的插件，不需要重新检查md5
    tar zxvf "${runtar}" -C "${RUNNING_DIR}"
    if [ "$?" != "0" ];then
        # Clean up.
        rm $(ls | grep -v "$MONITOR_FILE")
        start_acc
        return
    fi
	
	rm "${RUNNING_DIR}/${UPDATE_FILE}"

    chmod u+x "${exefile}"
    ${exefile} >/dev/null 2>&1 &

    # If qmplugin fails to start in 5 seconds, something wrong must happened.
    # Then give up what we have done.
    local TIMES="1 2 3 4 5"
    local not_running=1
    for i in ${TIMES};do
        check_running
        if [ "$?" = "0" ];then
            not_running=0
            break
        else
            sleep 1
        fi
    done

    if [ "${not_running}" = "1" ];then
        rm ${RUNNING_DIR}/*
        start_acc
        return
    fi

    # Check if flash space is enough
    rm ${backtar}
    check_space
    if [ "$?" = "0" ];then
        local backtar_md5file="${PLUGIN_DIR}/${PLUGIN_TAR_MD5_FILE}"
        cp ${runtar} ${backtar}
        if [ "$?" != "0" ];then
            [ -f "${backtar}" ] && rm "${backtar}"
            [ -f "${backtar_md5file}" ] && rm "${backtar_md5file}"
            rm ${runtar}
            echo "Update operation failed."
            return
        fi

        save_md5sum "${runtar}" "${backtar_md5file}"

        echo "Update operation succeeded."
        rm ${runtar}
        return
    else
        echo "Warning: no enough space is available."
        echo "Update operation failed."
        rm ${runtar}
        return
    fi
}

# Check if need to upgrade the jffs one plugin copy.
# The old copy will be deleted if require upgrade
# Return: no return value, continue on any failure
check_plugin_upgrade() {
	if [-f "${PLUGIN_DIR}/${PLUGIN_TAR_MD5_FILE}" ];then
		# get latest plugin release info
		local latest_plugin_info=$(curl -s -k -H "Accept:text/plain" "${DOWNLOAD_URL}" || \
			wget --header=Accept:text/plain -q --no-check-certificate -O - "${DOWNLOAD_URL}"
		)

		[ "$?" != "0" ] && return
		[ -z "$latest_plugin_info" ] && return

		local latest_plugin_md5=$(echo "$latest_plugin_info" | cut  -d ',' -f 2)
		[ -z "${latest_plugin_md5}" ] && return

	
		# get local md5
		local local_plugin_md5=$(cat ${PLUGIN_DIR}/${PLUGIN_TAR_MD5_FILE} 2>/dev/null)
		[ "$?" != "0" ] && return

		#compare md5
		if [ "$local_plugin_md5" != "$latest_plugin_md5" ]; then
			# delete local plugin copy and force to upgrade to latest
			rm -f "${PLUGIN_DIR}/${PLUGIN_TAR}" "${PLUGIN_DIR}/${PLUGIN_TAR_MD5_FILE}"
		fi
	
	fi
}

# Check if enough space is available in flash.
# Return: 0 means enough space is available; other mean errors.
check_space() {
    local df_res=$(df -k | grep -m 1 -E "[ \t]+${PLUGIN_MOUNT_DIR}[ \t]*" | grep -v "grep")
    [ -z "${df_res}" ] && return 0

    df_res=$(echo "${df_res}" | sed 's/[ ][ ]*/#/g')
    local available=$(echo "${df_res}" | cut -d'#' -f4)
    echo "Available space is ${available}"

    [ "${available}" -ge 500 ] && return 0
    return 1
}

check_backtar_file() {
    echo 'check_backtar_file in'
    local exefile="${RUNNING_DIR}/${PLUGIN_EXE}"   	#/tmp/qm/qmplugin
    local runtar="${RUNNING_DIR}/${PLUGIN_TAR}"
    local backtar="${PLUGIN_DIR}/${PLUGIN_TAR}"
    local md5file="${PLUGIN_DIR}/${PLUGIN_TAR_MD5_FILE}"

    if [ ! -e "${backtar}" ];then
        check_space
        if [ "$?" != "0" ];then
            [ -e "${exefile}" ] && return
        fi

        download_acc "${runtar}"
        if [ "$?" != "0" ]; then
            [ -f "${runtar}" ] && rm "${runtar}"
            return
        fi
 
        local exefile="${RUNNING_DIR}/${PLUGIN_EXE}"
        tar zxvf "${runtar}" -C "${RUNNING_DIR}"
        [ "$?" != "0" ] && return
        chmod u+x "${exefile}"
        ${exefile} >/dev/null 2>&1 &
		[ "$?" != "0" ] && return

		rm "${RUNNING_DIR}/${UPDATE_FILE}"
		
        check_space
        if [ "$?" = "0" ];then
            cp "${runtar}" "${backtar}" 
            if [ "$?" != "0" ]; then
                [ -f "${backtar}" ] && rm "${backtar}"
                [ -f "${runtar}" ] && rm "${runtar}"
                return
            fi

            save_md5sum "${runtar}" "${md5file}"
        else
            echo "No enough space is available." 
        fi

        save_md5sum "${runtar}" "${md5file}"
        [ -f "${runtar}" ] && rm "${runtar}"
        return
    fi

    [ -f "${md5file}" ] && return
    save_md5sum "${backtar}" "${md5file}"
}

check_plugin_file() {
    # One of ${exefile}, ${runtar}, ${backtar} must exist.
    local exefile="${RUNNING_DIR}/${PLUGIN_EXE}"   	#/tmp/qm/qmplugin
    local runtar="${RUNNING_DIR}/${PLUGIN_TAR}"		#/tmp/qm/qm.tar.gz
    local backtar="${PLUGIN_DIR}/${PLUGIN_TAR}"		#/jffs/qm/qm.tar.gz

    if [ ! -e "${exefile}" ] && [ ! -e "${runtar}" ] && [ ! -e "${backtar}" ];then
        download_acc "${runtar}"
		rm "${RUNNING_DIR}/${UPDATE_FILE}"
    fi
}



init_param
[ "$?" != "0" ] && exit 1

asuswrt_download_url_init
[ "$?" != "0" ] && exit 1

system_init
check_dir
[ "$?" != "0" ] && exit 1

check_plugin_upgrade

while :
do
    check_backtar_file
    check_plugin_file
    check_update
    check_acc
    sleep 1
    check_running
    set +x
    exec 2>/dev/null || true
    if [ "$?" = "0" ];then
        rm -f /tmp/qmacc_monitor.log
        # Plugin is running, so we will check again in 60 seconds.
        sleep 60
    else
        # Plugin is not running now, so check it more frequently.
        sleep 5
    fi
done

