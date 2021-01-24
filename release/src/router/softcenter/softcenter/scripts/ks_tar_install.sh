#!/bin/sh

# for arm384 platform

export KSROOT=/koolshare
source $KSROOT/scripts/base.sh
alias echo_date='echo 【$(date +%Y年%m月%d日\ %X)】:'
LOG_FILE=/tmp/upload/soft_log.txt
LOG_FILE_BACKUP=/tmp/upload/soft_install_log_backup.txt
eval $(dbus export soft)
TARGET_DIR=/tmp/upload
MODEL=$(nvram get productid)

jffs_space(){
	local JFFS_AVAL=$(df | grep -w "/jffs" | awk '{print $4}')
	echo ${JFFS_AVAL}
}

clean(){
	[ -n "$name" ] && rm -rf /tmp/$name >/dev/null 2>&1
	[ -n "$MODULE_NAME" ] && rm -rf /tmp/$MODULE_NAME >/dev/null 2>&1
	[ -n "$soft_name" ] && rm -rf /tmp/$soft_name >/dev/null 2>&1
	rm -rf /tmp/*.tar.gz >/dev/null 2>&1
	dbus remove soft_install_version
	dbus remove soft_name
}

detect_package(){
	local TEST_WORD="$1"
	local ILLEGAL_KEYWORDS="ss|ssr|shadowsocks|shadowsocksr|v2ray|trojan|clash|wireguard|koolss|brook|fuck"
	local KEY_MATCH=$(echo "${TEST_WORD}" | grep -Eo "$ILLEGAL_KEYWORDS")

	if [ -n "${KEY_MATCH}" ]; then
		echo_date =======================================================
		echo_date "检测到离线安装包：${soft_name} 含非法关键词！！！"
		echo_date "根据法律规定，koolshare软件中心将不会安装此插件！！！"
		echo_date "删除相关文件并退出..."
		echo_date =======================================================
		clean
		exit 1
	fi
}

install_tar(){
	detect_package "$soft_name"
	name=$(echo "$soft_name"|sed 's/.tar.gz//g'|awk -F "_" '{print $1}'|awk -F "-" '{print $1}')
	INSTALL_SUFFIX=_install
	VER_SUFFIX=_version
	NAME_SUFFIX=_name
	cd /tmp
	echo_date ====================== step 1 ===========================
	echo_date 开启软件离线安装！
	if [ -f $TARGET_DIR/$soft_name ];then
		local _SIZE=$(ls -lh $TARGET_DIR/$soft_name|awk '{print $5}')
		echo_date $TARGET_DIR目录下检测到上传的离线安装包$soft_name，大小：$_SIZE
		mv /tmp/upload/$soft_name /tmp
		echo_date 尝试解压离线安装包离线安装包
		tar -zxvf $soft_name >/dev/null 2>&1
		if [ "$?" == "0" ];then
			echo_date 解压完成！
			cd /tmp
		else
			echo_date 解压错误，错误代码："$?"！
			echo_date 估计是错误或者不完整的的离线安装包！
			echo_date 删除相关文件并退出...
			clean
			echo_date ======================== end ============================
			echo XU6J03M6
			exit
		fi

		if [ -f /tmp/$name/install.sh ];then
			INSTALL_SCRIPT=/tmp/$name/install.sh
		else
			INSTALL_SCRIPT_NU=$(find /tmp -name "install.sh"|wc -l) 2>/dev/null
			[ "$INSTALL_SCRIPT_NU" == "1" ] && INSTALL_SCRIPT=$(find /tmp -name "install.sh") || INSTALL_SCRIPT=""
		fi

		if [ -n "$INSTALL_SCRIPT" -a -f "$INSTALL_SCRIPT" ];then
			SCRIPT_AB_DIR=$(dirname $INSTALL_SCRIPT)
			MODULE_NAME=${SCRIPT_AB_DIR##*/}

			detect_package "${MODULE_NAME}"

			# 检查jffs空间，不可描述不做检测，交给插件自己处理
			local JFFS_AVAL=$(jffs_space)
			if [ "${MODULE_NAME}" != "shadowsocks" ];then
				echo_date "检测jffs分区剩余空间..."
				local JFFS_NEED=$(du -s /tmp/${MODULE_NAME} | awk '{print $1}')
				local TAR_SIZE=$(du -s /tmp/${soft_name} | awk '{print $1}')
				### echo_date "JFFS剩余空间：${JFFS_AVAL}KB"
				### echo_date "插件文件夹大小：${JFFS_NEED}KB"
				### echo_date "插件压缩包大小：${TAR_SIZE}KB"
				if [ "${JFFS_AVAL}" -lt "${JFFS_NEED}" ];then
					echo_date "-------------------------------------------------------------------"
					echo_date "软件中心：当前jffs分区剩余${JFFS_AVAL}KB, 插件安装大致需要${JFFS_NEED}KB，空间不足！"
					echo_date "请清理jffs分区内不要的文件，或者使用USB2JFFS插件对jffs分区进行扩容后再试！！"
					echo_date "-------------------------------------------------------------------"
					echo_date "本次插件安装失败！退出！"
					clean
					echo_date ======================== end ============================
					echo XU6J03M6
					exit
				fi
				echo_date "软件中心：当前jffs分区剩余：${JFFS_AVAL}KB, 空间满足，继续安装！"
			fi

			# 检查下安装包是否是hnd的
			if [ -f "${SCRIPT_AB_DIR}/.valid" -a -n "$(grep arm384 ${SCRIPT_AB_DIR}/.valid)" ];then
				continue
			elif [ "${MODULE_NAME}" == "shadowsocks" ];then
				# hnd的不可描述包没有校验字符串，避免安装失败
				continue
			else
				echo_date 你上传的离线安装包不是arm384平台的离线包！！！
				echo_date 请上传正确的离线安装包！！！
				echo_date 删除相关文件并退出...
				clean
				dbus remove "softcenter_module_${MODULE_NAME}${INSTALL_SUFFIX}"
				echo_date ======================== end ============================
				echo XU6J03M6
				exit
			fi

			echo_date 准备安装${MODULE_NAME}插件！
			echo_date 找到安装脚本！
			chmod +x $INSTALL_SCRIPT >/dev/null 2>&1
			sed -i "s/ 384/ 386/g" $INSTALL_SCRIPT >/dev/null 2>&1
			echo_date 运行安装脚本...
			echo_date ====================== step 2 ===========================

			start-stop-daemon -S -q -x $INSTALL_SCRIPT 2>&1
			if [ "$?" != "0" ];then
				echo_date 因为${MODULE_NAME}插件安装失败！退出离线安装！
				clean
				dbus remove "softcenter_module_${MODULE_NAME}${INSTALL_SUFFIX}"
				echo_date ======================== end ============================
				echo XU6J03M6
				exit
			fi

			sed -i '/rogcss/d' /koolshare/webs/Module_${MODULE_NAME}.asp >/dev/null 2>&1

			echo_date ====================== step 3 ===========================
			dbus set "softcenter_module_${MODULE_NAME}${NAME_SUFFIX}=${MODULE_NAME}"
			dbus set "softcenter_module_${MODULE_NAME}${INSTALL_SUFFIX}=1"
			if [ -n "$soft_install_version" ];then
				dbus set "softcenter_module_${MODULE_NAME}${VER_SUFFIX}=$soft_install_version"
				echo_date "从插件文件名中获取到了版本号：$soft_install_version"
			else
				if [ -z "$(dbus get softcenter_module_${MODULE_NAME}${VER_SUFFIX})" ];then
					dbus set "softcenter_module_${MODULE_NAME}${VER_SUFFIX}=0.1"
					echo_date "插件安装脚本里没有找到版本号，设置默认版本号为0.1"
				else
					echo_date "插件安装脚本已经设置了插件版本号为：$(dbus get softcenter_module_${MODULE_NAME}${VER_SUFFIX})"
				fi
			fi
			install_pid=$(ps | grep -w install.sh | grep -v grep | awk '{print $1}')
			i=120
			until [ -z "$install_pid" ]
			do
				install_pid=$(ps | grep -w install.sh | grep -v grep | awk '{print $1}')
				i=$(($i-1))
				if [ "$i" -lt 1 ];then
					echo_date "Could not load nat rules!"
					echo_date 安装似乎出了点问题，请手动重启路由器后重新尝试...
					echo_date 删除相关文件并退出...
					sleep 1
					clean
					dbus remove "softcenter_module_${MODULE_NAME}${INSTALL_SUFFIX}"
					echo_date ======================== end ============================
					echo XU6J03M6
					exit
				fi
				sleep 1
			done
			echo_date 离线包安装完成！
			echo_date 一点点清理工作...

			# 安装完毕，打印剩余空间
			local JFFS_AVAL_2=$(jffs_space)
			local JFFS_USED=$(($JFFS_AVAL - $JFFS_AVAL_2))
			if [ "${JFFS_USED}" -ge "0" ];then
				echo_date "软件中心：本次安装占用了${JFFS_USED}KB空间，目前jffs分区剩余容量：${JFFS_AVAL_2}KB"
			elif [ "${JFFS_USED}" -lt "0" ];then
				local JFFS_RELEASED=${JFFS_USED#-}
				echo_date "软件中心：本次安装释放了${JFFS_RELEASED}KB空间，目前jffs分区剩余容量：${JFFS_AVAL_2}KB"
			fi
			if [ "${JFFS_AVAL_2}" -lt "2000" ];then
				echo_date "软件中心：注意！目前jffs分区剩余容量只剩下：${JFFS_AVAL_2}KB，已不足2MB！"
			fi

			clean
			echo_date 完成！离线安装插件成功，现在你可以退出本页面~
		else
			echo_date 没有找到安装脚本！
			echo_date 删除相关文件并退出...
			clean
		fi
	else
		echo_date 没有找到离线安装包！
		echo_date 删除相关文件并退出...
		clean
	fi
	clean
	echo_date ======================== end ============================
	echo XU6J03M6
}

clean_backup_log() {
	local LOG_MAX=1000
	[ $(wc -l "$LOG_FILE_BACKUP" | awk '{print $1}') -le "$LOG_MAX" ] && return
	local logdata=$(tail -n 500 "$LOG_FILE_BACKUP")
	echo "$logdata" > $LOG_FILE_BACKUP 2> /dev/null
	unset logdata
}

backup_log_file(){
	sleep 3
	clean_backup_log
	echo XU6J03M6 | tee -a $LOG_FILE
	cat $LOG_FILE >> $LOG_FILE_BACKUP
}

true > $LOG_FILE
http_response "$1"
install_tar | tee -a $LOG_FILE
backup_log_file

