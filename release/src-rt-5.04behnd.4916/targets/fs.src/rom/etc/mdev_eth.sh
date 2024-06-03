#!/bin/sh
source /etc/profile

if [ ! -x "/sys/devices/virtual/net/$1" ] && [ -x "/sys/devices/virtual/net/br0" ]; then 

    case "$ACTION" in
        "add")
            ifconfig $1 up 
            brctl addif br0 $1
            ;;
        "remove")
            brctl delif br0 $1
            ifconfig $1 down
            ;;
    esac

fi
