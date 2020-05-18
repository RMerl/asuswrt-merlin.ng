#!/bin/sh

source /koolshare/scripts/base.sh

detect_skipd(){
	i=120
	skipd=$(pidof skipd)
	until [ -n "$skipd" ]
	do
		i=$(($i-1))
		    skipd=$(pidof skipd)
		if [ "$i" -lt 1 ];then
		    logger "[软件中心]services-start报告: 错误，skipd进程未能成功启动！"
		    exit
		fi
		sleep 1
	done
	logger "[软件中心]services-start报告: skipd进程准备就绪！"
	dbus set softcenter_installing_status="1"
}

start(){
	detect_skipd
	sleep 1
	if [ -f "/koolshare/.soft_ver" ];then
		dbus set softcenter_version=$(cat /koolshare/.soft_ver)
	fi
}

start
