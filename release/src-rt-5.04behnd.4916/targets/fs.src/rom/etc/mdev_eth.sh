#!/bin/sh
source /etc/profile
# This is mainly for the usb NICs

br_names="br0 br-lan"
for br_name in $br_names; do
if [ -x "/sys/devices/virtual/net/$br_name" ]; then 
if [ ! -x "/sys/devices/virtual/net/$1" ]; then
    case "$ACTION" in
        "add")
            ifconfig $1 up 
            brctl addif $br_name $1
            ;;
        "remove")
            brctl delif $br_name $1
            ifconfig $1 down
            ;;
    esac
fi
break
fi
done
