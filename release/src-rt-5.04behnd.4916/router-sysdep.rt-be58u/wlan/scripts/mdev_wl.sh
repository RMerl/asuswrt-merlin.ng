#!/bin/sh
IS_RDK_BUILD=RDK_BUILD_HOLDER
IS_WRT_BUILD=WRT_BUILD_HOLDER
source /etc/profile

dev_name=$1
br_name="br0"
# echo "dev_name = $dev_name action= $ACTION">> /tmp/hotplug.log

echo $LINENO "mdev wifi" >>/tmp/hotplug.log

if [[ ! -z $IS_WRT_BUILD ]]; then
	br_name="br-lan"
fi

if [[ ! -z $IS_RDK_BUILD ]]; then
    ACTION=$ACTION INTERFACE=$INTERFACE /etc/init.d/bcm_dwds_hot_plug.sh net
else

    if [ ! -x "/bin/send_cms_msg" ]; then exit 1; fi

    pidof "wlssk" > /dev/null 2>&1
    if [[ "$?" != "0" ]]; then
        # echo "The wlssk is not running." >> /tmp/hotplug.log
        exit 1
    fi

    # Below definitions should be synced with
    # userspace/public/libs/cms_util/cms_eid.h and userspace/public/libs/cms_msg/cms_msg.h
    EID_WLSSK=89
    CMS_MSG_WLAN_CHANGED=0x10000300  #/**< Tell wlssk to restart the wlan daemons */


    pidof "wifi_md" > /dev/null 2>&1
    if [[ "$?" == "0" ]]; then
        send_msg_cmd="send_cms_msg -e -c wifi $CMS_MSG_WLAN_CHANGED $EID_WLSSK"
        # echo "This is BDK" >> /tmp/hotplug.log
    else
        send_msg_cmd="send_cms_msg -e $CMS_MSG_WLAN_CHANGED $EID_WLSSK"
        # echo "This is CMS" >> /tmp/hotplug.log
    fi

    case "$ACTION" in
        "add" | "register" | "pwrup")
            $send_msg_cmd -D "Hotplug $dev_name $ACTION"
            echo "$send_msg_cmd -D Hotplug $dev_name $ACTION"  >> /tmp/hotplug.log
            if [ -z "${dev_name##"wds"*}" ]; then
                ifconfig $dev_name up
                if [ -e "/sys/devices/virtual/net/$br_name" ]; then
                    echo "dev_name = $dev_name - $br_name STP on" >> /tmp/hotplug.log
                    brctl stp $br_name on
                    bcm_sendarp -s $br_name -d $br_name
                fi
            fi
            ;;
        "remove" | "pwrdn")
            $send_msg_cmd -D "Hotplug $dev_name $ACTION"
            echo "$send_msg_cmd -D Hotplug $dev_name $ACTION"  >> /tmp/hotplug.log
            ;;
    esac
fi
exit 0
