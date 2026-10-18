#!/bin/sh
PROC_QOS_CTRL=/proc/ark/qos_ctrl

code=$1
shift

get_wan_carrier() {
    local dev=$1

    [ -z ${dev} ] && return 1

    dev=$(cat /sys/class/net/${dev}/carrier)
    [ -f ${PROC_QOS_CTRL} ] && echo "b:wan_carrier:${dev}" > ${PROC_QOS_CTRL}
    return 0
}


case ${code} in
    get-wan-carrier)
        get_wan_carrier $@
        ;;
esac
