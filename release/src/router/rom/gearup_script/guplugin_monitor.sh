#!/bin/sh


# Online URL
DOWNLOAD_URL="https://router.booster.gearupportal.com/api/plugin?type="

MONITOR_FILE="guplugin_monitor.sh"
UPDATE_FILE="gu.update"
RUNNING_DIR="/tmp/gu"
PLUGIN_TAR="gu.tar.gz"
PLUGIN_EXE="guplugin"
PLUGIN_CONF="gu.conf"
PID_FILE="/var/run/guplugin.pid"
PLUGIN_TAR_CHECKSUM_FILE="gu.tar.gz.checksum"
PLUGIN_MOUNT_DIR=""
PLUGIN_DIR=""

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

        mount_point=$(dirname "$mount_point")
    done
    echo "$mount_point"
    return 0
}

# Initialization based on the model of this router.
# Return: 0 means success; 1 means failure.
init() {
    insmod nfnetlink
    insmod nf_conntrack_netlink

    # Read model from '/var/model'
    local model=$(cat /var/model | head -n1 | cut -f1 -d,)
    if [ -z "${model}" ];then
        # Use 'odmpid'.
        model=$(nvram get odmpid)
    fi

    if [ -z "${model}" ];then
        # Use 'productid'.
        model=$(nvram get productid)
    fi

    [ -z "${model}" ] &&  exit 1

    # Change to lower case.
    model=$(echo "${model}" | sed 'y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/')
    # Remove all '-'.
    model=$(echo "${model}" | sed 's/-//g')
    model="asuswrt-${model}"

    DOWNLOAD_URL="${DOWNLOAD_URL}""${model}"

    local sn=$(cat /var/label_macaddr | head -n1)
    DOWNLOAD_URL="${DOWNLOAD_URL}""&sn=${sn}"
    return 0
}

check_dir() {
    PLUGIN_DIR=$(cat "/var/gu_plugin_dir")
    [ ! -d "${PLUGIN_DIR}" ] && exit 1

    if [ ! -d "${RUNNING_DIR}" ];then
        mkdir -p "${RUNNING_DIR}"
        [ "$?" != "0" ] && exit 1
    fi
    PLUGIN_MOUNT_DIR=$(get_mount_point "${PLUGIN_DIR}")
    [ ! -d "${PLUGIN_MOUNT_DIR}" ] && exit 1
}

check_plugin_file() {
    # One of ${exefile}, ${runtar}, ${backtar} must exist.
    local exefile="${RUNNING_DIR}/${PLUGIN_EXE}"
    local runtar="${RUNNING_DIR}/${PLUGIN_TAR}"
    local backtar="${PLUGIN_DIR}/${PLUGIN_TAR}"

    if [ ! -e "${exefile}" ] && [ ! -e "${runtar}" ] && [ ! -e "${backtar}" ];then
        download_acc "${runtar}"
    fi
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
    return 1
}

# Return: 0 means update flag is set.
check_update() {
    if [ -f "${RUNNING_DIR}/${UPDATE_FILE}" ];then
        return 0
    else
        return 1
    fi
}

cal_checksum() {
    local file_checksum=$(openssl dgst -sha256 "${1}" | awk '{print $2}')
    if [ "${file_checksum}" != "" ];then
        echo -n "${file_checksum}"
        return
    fi

    file_checksum=$(md5sum "${1}")
    echo -n "${file_checksum}"
    return
}

verify_signature() {
    # $1: file
    # $2: signature
    local FILE="${1}"
    local SIGNATURE="${2}"
    local SIGNATURE_SHA256_BASE64="${RUNNING_DIR}/gu_asuswrt_signature.base64"
    local SIGNATURE_SHA256_BIN="${RUNNING_DIR}/gu_asuswrt_signature.bin"
    local GU_ASUSWRT_PUBLIC_KEY_FILE="${RUNNING_DIR}/gu_asuswrt_public_key.pem"
    {
        echo '-----BEGIN PUBLIC KEY-----'
        echo 'MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA0sCfL+IN8XRNzJMEoB/L'
        echo 'C+1uu1S3qDy0GZO2eArV1pl16b2TD/PN8z9Q7QwOOt9hISJAnZqQHRcxv/BvuJJU'
        echo 'Ku4FNn8xuYx7obLzQzW7KZz5SCduI3pIm0ZnkBj5kkvek0hzQDrUo294LTg3PJqv'
        echo 'Eszi3jKPDglizC7Q0y/xfu2kDm0ARe/md4crUApO314+xgOtnaFB/D2N/9yM21FG'
        echo '3Oycc26sz3vJUNzKiM+miQgIWmwFqCmX/eA4AJ9LMp5Qh9oLDB6IIuGXmxNeo5UO'
        echo 'J1Dvv9ccvXDiPa5fcFI6ISgEwBf9R6rWHW7DzAc9xSVEYq+ZXZOGiyKDQM3Z0Ha8'
        echo 'BwIDAQAB'
        echo '-----END PUBLIC KEY-----'
    } > ${GU_ASUSWRT_PUBLIC_KEY_FILE}

    touch "${SIGNATURE_SHA256_BASE64}"
    > "${SIGNATURE_SHA256_BASE64}"
    touch "${SIGNATURE_SHA256_BIN}"
    > "${SIGNATURE_SHA256_BIN}"

    echo -n "${SIGNATURE}" > "${SIGNATURE_SHA256_BASE64}"
    openssl enc -d -A -base64 -in "${SIGNATURE_SHA256_BASE64}" \
        -out "${SIGNATURE_SHA256_BIN}"
    openssl dgst -sha256 -verify "${GU_ASUSWRT_PUBLIC_KEY_FILE}" \
        -signature "${SIGNATURE_SHA256_BIN}" "${FILE}"
    return $?
}

# $1: FileName to where content to be saved. 
# Return: 0 means success.
download_acc() {
    local plugin_info=$(curl -s -H "Accept:text/plain" "$DOWNLOAD_URL" || \
        wget --header=Accept:text/plain -q -O - "${DOWNLOAD_URL}"
    )

    [ "$?" != "0" ] && return 1
    [ -z "$plugin_info" ] && return 1

    local plugin_url=$(echo "$plugin_info" | cut  -d ',' -f 1)
    local plugin_checksum=$(echo "$plugin_info" | cut  -d ',' -f 2)
    local plugin_signature=$(echo "${plugin_checksum}" | cut -d '.' -f 2)
    plugin_checksum=$(echo "${plugin_checksum}" | cut -d '.' -f 1)

    if [ -z "${plugin_url}" ];then
        echo "plugin_url is empty."
        return 1
    fi

    if [ -z "${plugin_checksum}" ];then
        echo "plugin_checksum is empty."
        return 1
    fi

    curl -s "$plugin_url" -o "${1}" >/dev/null 2>&1 || \
        wget -q "$plugin_url" -O "${1}" >/dev/null 2>&1

    if [ "$?" != "0" ];then
        echo "Failed: curl (-L) -s -k ${plugin_url} -o ${1} ||
            wget -q --no-check-certificate $plugin_url -O ${1}"
        # Clean up
        [ -f "$1" ] && rm "${1}"
        return 1
    fi

    if [ -f "$1" ];then
        local download_checksum=$(cal_checksum "$1")
        download_checksum=$(echo "${download_checksum}" | sed 's/[ ][ ]*/ /g' | cut -d' ' -f1)
        if [ "$download_checksum" != "${plugin_checksum}" ];then
            echo "${1} downloaded has incorrect checksum."
            rm "$1"
            return 1
        fi
        verify_signature "${1}" "${plugin_signature}"
        if [ "$?" != "0" ];then
            echo "${1} donwloaded has incorrect signature"
            rm "${1}"
            return 1
        fi

        echo "${1} has been successfully downloaded."
        return 0
    else
        echo "Fail to download ${1}."
        return 1
    fi
}

# $1: FileName of which checksum will be saved.
# $2: FileName where checksum will be saved.
save_checksum() {
    [ ! -f "${1}" ] && return

    local file_checksum=$(cal_checksum "$1")
    file_checksum=$(echo "${file_checksum}" | sed 's/[ ][ ]*/ /g' | cut -d' ' -f1)
    echo "File checksum: ${file_checksum}"
    echo "${file_checksum}" > "${2}"
}

check_backtar_file() {
    local runtar="${RUNNING_DIR}/${PLUGIN_TAR}"
    local backtar="${PLUGIN_DIR}/${PLUGIN_TAR}"
    local file_checksum="${PLUGIN_DIR}/${PLUGIN_TAR_CHECKSUM_FILE}"

    if [ ! -e "${backtar}" ];then
        download_acc "${runtar}"
        if [ "$?" != "0" ]; then
            [ -f "${runtar}" ] && rm "${runtar}"
            return
        fi

        local exefile="${RUNNING_DIR}/${PLUGIN_EXE}"
        tar zxvf "${runtar}" -C "${RUNNING_DIR}"
        [ "$?" != "0" ] && return
        chmod u+x "${exefile}"
        ${exefile} -v >/dev/null 2>&1
        [ "$?" != "0" ] && return

        check_space
        if [ "$?" = "0" ];then
            cp "${runtar}" "${backtar}"
            if [ "$?" != "0" ]; then
                [ -f "${backtar}" ] && rm "${backtar}"
                [ -f "${runtar}" ] && rm "${runtar}"
                return
            fi

            save_checksum "${runtar}" "${file_checksum}"
        else
            echo "No enough space is available."
        fi

        [ -f "${runtar}" ] && rm "${runtar}"
        return
    fi

    [ -f "${file_checksum}" ] && return
    save_checksum "${backtar}" "${file_checksum}"
}

# $1: FileName of which checksum to be checked.
# $2: FileName that contains checksum.
# Return: 0 means success.
check_checksum() {
    [ ! -f "$1" -o ! -f "$2" ] && return 1

    local plugin_checksum=$(cal_checksum "$1")
    [ "$?" != "0" ] && return 1

    local right_checksum=$(cat "${2}")
    [ "$?" != "0" ] && return 1

    plugin_checksum=$(echo "$plugin_checksum" | sed 's/[ ][ ]*/ /g' | cut -d' ' -f1)
    if [ "$right_checksum" != "$plugin_checksum" ];then
        echo "Error: checksum does not match."
        return 1
    fi
    return 0
}

# Check if enough space is available in flash.
# Return: 0 means enough space is available; other mean errors.
check_space() {
    local df_res=$(df -k | grep "${PLUGIN_MOUNT_DIR}" | grep -v "grep")
    [ -z "${df_res}" ] && return 0

    df_res=$(echo "${df_res}" | sed 's/[ ][ ]*/#/g')
    local available=$(echo "${df_res}" | cut -d'#' -f4)
    echo "Available space is ${available}"

    [ "${available}" -ge 500 ] && return 0
    return 1
}

# Check to see if upgrade is possible.
# The old copy will be deleted if upgrade is required.
# Return: no return value, continue on any failure.
check_plugin_upgrade() {
    # get latest plugin release info
    local latest_plugin_info=$(curl -s -H "Accept:text/plain" "${DOWNLOAD_URL}" || \
        wget --header=Accept:text/plain -q -O - "${DOWNLOAD_URL}"
    )

    [ "$?" != "0" ] && return
    [ -z "$latest_plugin_info" ] && return

    local latest_plugin_checksum=$(echo "$latest_plugin_info" | cut  -d ',' -f 2)
    latest_plugin_checksum=$(echo "${latest_plugin_checksum}" | cut -d '.' -f 1)
    [ -z "${latest_plugin_checksum}" ] && return

    # get local checksum
    local local_plugin_checksum=$(cat ${PLUGIN_DIR}/${PLUGIN_TAR_CHECKSUM_FILE} 2>/dev/null)
    [ "$?" != "0" ] && return

    #compare checksum
    if [ "$local_plugin_checksum" != "$latest_plugin_checksum" ]; then
        # delete local plugin copy and force to upgrade to latest
        rm -f "${PLUGIN_DIR}/${PLUGIN_TAR}" "${PLUGIN_DIR}/${PLUGIN_TAR_CHECKSUM_FILE}"
    fi
}

start_acc() {
    # Start order:
    # 1. ${exefile}
    # 2. ${runtar}
    # 3. ${backtar}

    local exefile="${RUNNING_DIR}/${PLUGIN_EXE}"
    local runtar="${RUNNING_DIR}/${PLUGIN_TAR}"
    local backtar="${PLUGIN_DIR}/${PLUGIN_TAR}"
    local confile="${RUNNING_DIR}/${PLUGIN_CONF}"

    if [ -e "${exefile}" ];then
        chmod u+x "${exefile}"
        ${exefile} "${confile}" >/dev/null 2>&1 &
        return
    fi

    if [ -f "${runtar}" ];then
        tar zxvf "${runtar}" -C "${RUNNING_DIR}" 
        if [ "$?" = "0" ];then
            chmod u+x "${exefile}"
            ${exefile} "${confile}" >/dev/null 2>&1 &

            # ${runtar} is not needed any more.
            rm "${runtar}"
            return
        fi
    fi

    if [ -f "${backtar}" ];then
        check_checksum "${backtar}" "${PLUGIN_DIR}/${PLUGIN_TAR_CHECKSUM_FILE}"
        if [ "$?" != "0" ];then
            # Download a new one. Next time ${runtar} will be used.
            download_acc "${runtar}"
            return
        fi

        tar zxvf "${backtar}" -C "${RUNNING_DIR}"
        if [ "$?" != "0" ];then
            return
        fi

        chmod u+x "${exefile}"
        ${exefile} "${confile}" >/dev/null 2>&1 &
        return
    else
        # Download a new one. Next time ${runtar} will be used.
        download_acc "${runtar}"
        return
    fi
}

check_acc() {
    cd $RUNNING_DIR

    check_running
    [ "$?" = "0" ] && return

    check_update
    if [ "$?" != "0" ];then
        # "guplugin" is not running, and no need to be updated.
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
    tar zxvf "${runtar}" -C "${RUNNING_DIR}"
    if [ "$?" != "0" ];then
        # Clean up.
        rm $(ls | grep -v "$MONITOR_FILE")
        start_acc
        return
    fi

    chmod u+x "${exefile}"
    ${exefile} "${RUNNING_DIR}/$PLUGIN_CONF" >/dev/null 2>&1 &

    # If guplugin fails to start in 5 seconds, something wrong must happened.
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
        rm $(ls | grep -v "$MONITOR_FILE")
        start_acc
        return
    fi

    # Check if flash space is enough
    rm ${backtar}
    check_space
    if [ "$?" = "0" ];then
        local backtar_file_checksum="${PLUGIN_DIR}/${PLUGIN_TAR_CHECKSUM_FILE}"
        cp ${runtar} ${backtar}
        if [ "$?" != "0" ];then
            [ -f "${backtar}" ] && rm "${backtar}"
            [ -f "${backtar_file_checksum}" ] && rm "${backtar_file_checksum}"
            rm ${runtar}
            echo "Update operation failed."
            return
        fi

        save_checksum "${runtar}" "${backtar_file_checksum}"

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

init
check_dir
check_plugin_upgrade

while :
do
    check_backtar_file
    check_plugin_file
    check_acc
    sleep 1
    check_running
    if [ "$?" = "0" ];then
        # Plugin is running, so we will check again in 60 seconds.
        sleep 60
    else
        # Plugin is not running now, so check it more frequently.
        sleep 5
    fi
done
