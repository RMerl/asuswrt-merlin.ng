#!/bin/sh
source /etc/profile

EID_SSK=21
CMS_MSG_ID=0x10000316  # tell ssk usb eth device attach or detach

# This is for USB Ethernet devices
if [ ! -x "/sys/devices/virtual/net/$1" ]; then
    case "$ACTION" in
        "add")
            echo "mdev_eth: detected PLUG of $1"
            if [ -x /bin/sysmgmt_md ]; then
                # BDK actions (other platforms can look for their unique exe and add custom logic for them)
                # suppress error msgs and run in background so we don't block mdev during retry
                (send_cms_msg -r -t 2000 -y 5 $CMS_MSG_ID $EID_SSK -D "PLUG $1" > /dev/null 2>&1)&
            else
                ifconfig $1 up 
                if [ -x "/sys/devices/virtual/net/br0" ]; then
                    brctl addif br0 $1
                elif [ -x "/sys/devices/virtual/net/br-lan" ]; then
                    brctl addif br-lan $1
                fi
            fi
            ;;
        "remove")
            echo "mdev_eth: detected UNPLUG of $1"
            if [ -x /bin/sysmgmt_md ]; then
                (send_cms_msg -r -t 2000 -y 5 $CMS_MSG_ID $EID_SSK -D "UNPLUG $1" > /dev/null 2>&1)&
            else
                if [ -x "/sys/devices/virtual/net/br0" ]; then
                    brctl delif br0 $1
                elif [ -x "/sys/devices/virtual/net/br-lan" ]; then
                    brctl delif br-lan $1
                fi
                ifconfig $1 down
            fi
            ;;
    esac
fi

