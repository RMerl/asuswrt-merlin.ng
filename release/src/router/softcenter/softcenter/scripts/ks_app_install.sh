#!/bin/sh

export KSROOT=/koolshare
source $KSROOT/scripts/base.sh


#From dbus to local variable
eval $(dbus export softcenter_installing_)
source /koolshare/scripts/base.sh
export PERP_BASE=/koolshare/perp

#softcenter_installing_module 	#正在安装的模块
#softcenter_installing_todo 	#希望安装的模块
#softcenter_installing_tick 	#上次安装开始的时间
#softcenter_installing_version 	#正在安装的版本
#softcenter_installing_md5 	#正在安装的版本的md5值
#softcenter_installing_tar_url 	#模块对应的下载地址

#softcenter_installing_status=		#尚未安装
#softcenter_installing_status=0		#尚未安装
#softcenter_installing_status=1		#已安装
#softcenter_installing_status=2		#将被安装到jffs分区...
#softcenter_installing_status=3		#正在下载中...请耐心等待...
#softcenter_installing_status=4		#正在安装中...
#softcenter_installing_status=5		#安装成功！请5秒后刷新本页面！...
#softcenter_installing_status=6		#卸载中......
#softcenter_installing_status=7		#卸载成功！
#softcenter_installing_status=8		#没有检测到在线版本号！
#softcenter_installing_status=9		#正在下载更新......
#softcenter_installing_status=10	#正在安装更新...
#softcenter_installing_status=11	#安装更新成功，5秒后刷新本页！
#softcenter_installing_status=12	#下载文件校验不一致！
#softcenter_installing_status=13	#然而并没有更新！
#softcenter_installing_status=14	#正在检查是否有更新~
#softcenter_installing_status=15	#检测更新错误！
#softcenter_installing_status=16	#下载错误，代码：！
#softcenter_installing_status=17	#卸载失败！请关闭插件后重试！

softcenter_home_url=$(dbus get softcenter_home_url)
CURR_TICK=$(date +%s)
BIN_NAME=$(basename "$0")
BIN_NAME="${BIN_NAME%.*}"
if [ "$ACTION" != "" ]; then
	BIN_NAME=$ACTION
fi

if [ "$(nvram get model)" == "GT-AC5300" ] || [ "$(nvram get model)" == "GT-AX11000" ] || [ -n "$(nvram get extendno | grep koolshare)" -a "$(nvram get productid)" == "RT-AC86U" ];then
	ROG=1
fi

LOGGER() {
#	echo $1
	logger $1
}

install_module() {
	if [ "${softcenter_home_url}" = "" -o "${softcenter_installing_md5}" = "" -o "${softcenter_installing_version}" = "" ]; then
		LOGGER "【软件中心】input error, something not found"
		exit 1
	fi

	if [ "${softcenter_installing_tick}" = "" ]; then
		export softcenter_installing_tick=0
	fi
	LAST_TICK=$(expr ${softcenter_installing_tick} + 20)
	if [ "${LAST_TICK}" -ge "${CURR_TICK}" -a "${softcenter_installing_module}" != "" ]; then
		LOGGER "【软件中心】module ${softcenter_installing_module} is installing"
		exit 2
	fi

	if [ "${softcenter_installing_todo}" = "" ]; then
		#curr module name not found
		LOGGER "【软件中心】module name not found"
		exit 3
	fi

	# Just ignore the old installing_module
	export softcenter_installing_module=${softcenter_installing_todo}
	export softcenter_installing_tick=$(date +%s)
	dbus set softcenter_installing_status="2"
	sleep 1
	dbus save softcenter_installing_

	URL_SPLIT="/"
	#OLD_MD5=$(dbus get softcenter_module_${softcenter_installing_module_md5})
	OLD_VERSION=$(dbus get softcenter_module_${softcenter_installing_module}_version)
	HOME_URL=$(dbus get softcenter_home_url)
	TAR_URL=${HOME_URL}${URL_SPLIT}${softcenter_installing_tar_url}
	FNAME=$(basename ${softcenter_installing_tar_url})

	if [ "$OLD_VERSION" = "" ]; then
		OLD_VERSION=0
	fi

	CMP=$(versioncmp ${softcenter_installing_version} ${OLD_VERSION})
	if [ -f "/koolshare/webs/Module_${softcenter_installing_module}.sh" -o "${softcenter_installing_todo}" = "softcenter" ]; then
		CMP="-1"
	fi
	if [ "$CMP" = "-1" ]; then

	cd /tmp
	rm -f ${FNAME}
	rm -rf "/tmp/$softcenter_installing_module"
	dbus set softcenter_installing_status="3"
	sleep 1
	wget -t 2 -T 20 --dns-timeout=15 --no-check-certificate -q ${TAR_URL}
	RETURN_CODE=$?

	if [ "$RETURN_CODE" != "0" ]; then
		dbus set softcenter_installing_status="16"
		sleep 3
		dbus set softcenter_installing_status="0"
		dbus set softcenter_installing_module=""
		dbus set softcenter_installing_todo=""
		LOGGER "【软件中心】wget ${TAR_URL} error, ${RETURN_CODE}"
		exit ${RETURN_CODE}
	fi

	md5sum_gz=$(md5sum /tmp/${FNAME} | sed 's/ /\n/g'| sed -n 1p)
	if [ "$md5sum_gz"x != "$softcenter_installing_md5"x ]; then
		LOGGER "【软件中心】md5 not equal $md5sum_gz"
		dbus set softcenter_installing_status="12"
		rm -f ${FNAME}
		sleep 3

		dbus set softcenter_installing_status="0"
		dbus set softcenter_installing_module=""
		dbus set softcenter_installing_todo=""

		rm -f ${FNAME}
		rm -rf "/tmp/$softcenter_installing_module"
		exit
	else
		tar -zxf ${FNAME}
		dbus set softcenter_installing_status="4"

		if [ ! -f /tmp/${softcenter_installing_module}/install.sh ]; then
			dbus set softcenter_installing_status="0"
			dbus set softcenter_installing_module=""
			dbus set softcenter_installing_todo=""

			#rm -f ${FNAME}
			#rm -rf "/tmp/${softcenter_installing_module}"

			LOGGER "【软件中心】package hasn't install.sh"
			exit 5
		fi

		if [ -f /tmp/${softcenter_installing_module}/uninstall.sh ]; then
			chmod 755 /tmp/${softcenter_installing_module}/uninstall.sh
			mv /tmp/${softcenter_installing_module}/uninstall.sh /koolshare/scripts/uninstall_${softcenter_installing_todo}.sh
		fi

		if [ -d /tmp/${softcenter_installing_module}/GT-AC5300 -a "$ROG" == "1" ]; then
			cp -rf /tmp/${softcenter_installing_module}/GT-AC5300/* /tmp/${softcenter_installing_module}/
		fi

		if [ -d /tmp/${softcenter_installing_module}/ROG -a "$ROG" == "1" ]; then
			cp -rf /tmp/${softcenter_installing_module}/ROG/* /tmp/${softcenter_installing_module}/
		fi

		chmod a+x /tmp/${softcenter_installing_module}/install.sh
		sh /tmp/${softcenter_installing_module}/install.sh
		sleep 3

		rm -f ${FNAME}
		rm -rf "/tmp/$softcenter_installing_module"

		if [ "$softcenter_installing_module" != "softcenter" ]; then
			dbus set softcenter_module_${softcenter_installing_module}_md5=${softcenter_installing_md5}
			dbus set softcenter_module_${softcenter_installing_module}_version=${softcenter_installing_version}
			dbus set softcenter_module_${softcenter_installing_module}_install=1
			dbus set ${softcenter_installing_module}_version=${softcenter_installing_version}
		else
			dbus set softcenter_version=${softcenter_installing_version};
			dbus set softcenter_md5=${softcenter_installing_md5}
		fi
		dbus set softcenter_installing_module=""
		dbus set softcenter_installing_todo=""
		dbus set softcenter_installing_status="1"
	fi

	else
		LOGGER "【软件中心】current version is newest version"
		dbus set softcenter_installing_status="13"
		sleep 3

		dbus set softcenter_installing_status="0"
		dbus set softcenter_installing_module=""
		dbus set softcenter_installing_todo=""
	fi
}

uninstall_module() {
	if [ "${softcenter_installing_tick}" = "" ]; then
		export softcenter_installing_tick=0
	fi
	LAST_TICK=$(expr ${softcenter_installing_tick} + 20)
	if [ "${LAST_TICK}" -ge "${CURR_TICK}" -a "${softcenter_installing_module}" != "" ]; then
		LOGGER "【软件中心】module ${softcenter_installing_module} is installing"
		exit 2
	fi

	if [ "${softcenter_installing_todo}" = "" -o "${softcenter_installing_todo}" = "softcenter" ]; then
		#curr module name not found
		LOGGER "【软件中心】module name not found"
		exit 3
	fi

	local ENABLED=$(dbus get ${softcenter_installing_todo}_enable)
	if [ "${ENABLED}" == "1" ]; then
		LOGGER "【软件中心】please disable ${softcenter_installing_module} then try again"
		dbus set softcenter_installing_status="17"
		sleep 3
		dbus set softcenter_installing_status="0"
		exit 4
	fi

	# Just ignore the old installing_module
	export softcenter_installing_module=${softcenter_installing_todo}
	export softcenter_installing_tick=$(date +%s)
	export softcenter_installing_status="6"
	dbus save softcenter_installing_

	dbus remove softcenter_module_${softcenter_installing_module}_md5
	dbus remove softcenter_module_${softcenter_installing_module}_version
	dbus remove softcenter_module_${softcenter_installing_module}_install
	dbus remove softcenter_module_${softcenter_installing_module}_description
	dbus remove softcenter_module_${softcenter_installing_module}_name
	dbus remove softcenter_module_${softcenter_installing_module}_title

	txt=$(dbus list ${softcenter_installing_todo})
	printf "%s\n" "$txt" |
	while IFS= read -r line; do
		line2="${line%=*}"
		if [ "${line2}" != "" ]; then
			dbus remove ${line2}
		fi
	done

	sleep 3
	dbus set softcenter_installing_module=""
	dbus set softcenter_installing_status="7"
	dbus set softcenter_installing_todo=""

	#try to call uninstall script
	if [ -f /koolshare/scripts/${softcenter_installing_todo}_uninstall.sh]; then
 		sh /koolshare/scripts/${softcenter_installing_todo}_uninstall.sh
	elif [ -f "/koolshare/scripts/uninstall_${softcenter_installing_todo}.sh" ]; then
		sh /koolshare/scripts/uninstall_${softcenter_installing_todo}.sh
	else
		if [ -n "${softcenter_installing_todo}" ]; then
			rm -rf /koolshare/${softcenter_installing_todo}
			rm -rf /koolshare/bin/${softcenter_installing_todo}
			rm -rf /koolshare/init.d/*${softcenter_installing_todo}*
			rm -rf /koolshare/scripts/${softcenter_installing_todo}*.sh
			rm -rf /koolshare/res/icon-${softcenter_installing_todo}.png
			rm -rf /koolshare/webs/Module_${softcenter_installing_todo}.asp
		fi
	fi
}

#LOGGER $BIN_NAME
case $2 in
update)
	http_response $1
	install_module
	;;
install)
	http_response $1
	install_module
	;;
ks_app_install)
	http_response $1
	install_module
	;;
ks_app_remove)
	http_response $1
	uninstall_module
	;;
*)
	http_response $1
	install_module
	;;
esac
