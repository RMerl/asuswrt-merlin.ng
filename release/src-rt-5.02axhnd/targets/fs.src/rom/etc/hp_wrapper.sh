#!/bin/sh

case "$ACTION" in
    "add")
        /sbin/mdev $1
        if [ -x "/sbin/hotplug" ]; then 
            /sbin/hotplug $1 
        fi 
        ;;
    "remove")
        if [ -x "/sbin/hotplug" ]; then 
            /sbin/hotplug $1 
        fi 
        /sbin/mdev $1
        ;;
esac
