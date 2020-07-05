#!/bin/sh

usb_type="ext3|ext4|thfsplus"
usb_path=$(mount | grep "/tmp/mnt/" | grep -E ${usb_type} | grep "/dev/s" | head -1 | awk '{print $3}')
usb_folder="${usb_path}/.usb_jffs"

stop_software_center(){
	killall skipd >/dev/null 2>&1
	/koolshare/perp/perp.sh stop >/dev/null 2>&1
}

start_software_center(){
	killall skipd >/dev/null 2>&1
	/koolshare/perp/perp.sh stop >/dev/null 2>&1
	service start_skipd >/dev/null 2>&1
	/koolshare/perp/perp.sh start >/dev/null 2>&1
}

mount_jffs(){
	if [ -z "$usb_path" ]; then
		echo "没有找到符合要求的USB存储器！"
		return
	fi
	sync
	if [ ! -d "$usb_folder" ]; then
		echo "没有找到${usb_folder}！"
		echo "复制原始分区/jffs内容到${usb_folder}！"
		mkdir -p "$usb_folder"
		cp -af /jffs/. "$usb_folder"
	else
		echo "找到${usb_folder}！"
		echo "以前挂载过USB型JFFS分区，将为你重新挂载！"
	fi
	echo "停止软件中心相关进程！"
	stop_software_center
	echo "卸载JFFS分区..."
	umount -l /jffs
	echo "将${usb_folder}挂载到JFFS分区..."
	mount -o rbind "$usb_folder" /jffs
	if [ "$?" == "0" ]; then
		echo "USB型JFFS分区挂载成功！"
		echo "将ubi0_0挂载在/cifs2"
		mount -t ubifs -o rw,noatime ubi0_0 /cifs2
		if [ "$?" == "0" ]; then
			echo "/cifs2挂载成功！"
		else
			echo "/cifs2挂载失败，将无法使用同步功能！！"
		fi
	else
		echo "USB型JFFS挂载失败！！"
		echo "恢复原始挂载分区！"
		mount -t ubifs -o rw,noatime ubi0_0 /jffs
		if [ "$?" == "0" ]; then
			echo "已经恢复原始挂载分区！"
		else
			echo "恢复原始挂载分区失败，请重启路由器后重试！"
		fi
	fi
	echo "重启软件中心相关进程..."
	start_software_center
	echo "完成！"
}

unmount_jffs(){
	usb2jffs_rsync=`nvram get usb2jffs_rsync`
	if [ "$usb2jffs_rsync" -eq "1" ]; then
		echo "检测到卸载时同步已开启，开始同步！"
		sync_usb_mtd
	fi
	echo "开始卸载！"
	sync
	echo "停止软件中心相关进程！"
	stop_software_center
	echo "卸载/cifs2..."
	umount -l /cifs2
	if [ "$?" == "0" ]; then
		echo "/cifs2卸载成功..."
	else
		echo "/cifs2卸载失败！"
	fi
	echo "卸载USB型JFFS..."
	umount -l /jffs
	if [ "$?" == "0" ]; then
		echo "USB型JFFS卸载成功..."
		echo "将文件系统ubi0_0挂载到jffs分区..."
		mount -t ubifs -o rw,noatime ubi0_0 /jffs
		if [ "$?" == "0" ]; then
			echo "原始分区挂载成功！"
			echo "重启软件中心相关进程..."
			start_software_center
		else
			echo "原始分区分区挂载失败！请重启后再试！"
		fi
	else
		echo "卸载USB型JFFS分区挂载失败！请重启后再试！"
	fi
}

remove_jffs(){
	unmount_jffs
	echo "删除${usb_folder}！"
	rm -rf "$usb_folder"
}

sync_usb_mtd(){
	# 检测是否有USB磁盘挂载在/jffs上
	local mounted_nu=$(mount | grep "/jffs" | grep -c "/dev/s")
	if [ "$mounted_nu" -ge "1" ]; then
		echo "检测到USB磁盘的${usb_folder}挂载在/jffs上"
	else
		echo "没有检测到任何挂载在/jffs上的USB磁盘设备，退出同步！"
		return
	fi

	# 检查cifs2是否挂载
	local cifs2_device=$(df -h | grep cifs2 | awk '{print $1}' 2>&1)
	if [ -z "${cifs2_device}" -o "${cifs2_device}" != "ubi0_0" ]; then
		echo "/cifs2未挂载！文件同步无法进行，请重启路由器后重试！"
		return
	fi

	# 检测jffs分区剩余空间...
	local TOTAL_MTD_SIZE=$(df | grep cifs2 | awk '{print $2}')
	local SPACE_USB_JFFS=$(du -s ${usb_folder} | awk '{print $1}')
	if [ "${TOTAL_MTD_SIZE}" -gt "${SPACE_USB_JFFS}" ];then
		# echo "空间满足，继续下一步！"
		# 文件夹同步
		# ls 找出所有的目录，写入文件
		ls -aelR /jffs/ | grep -i "^/" | sed 's/:$//g' | sed 's/^\/jffs\///g' | sed '/^$/d' > /tmp/upload/jffs_folders.txt
		ls -aelR /cifs2/ | grep -i "^/" | sed 's/:$//g' | sed 's/^\/cifs2\///g' | sed '/^$/d' > /tmp/upload/cifs2_folders.txt
		# 文件同步
		# find 找出/jffs下所有的文件+目录，ls列出来后，只取文件的名字，写入文件（因为固件不支持-type f，只能这么搞了）
		find /jffs -exec ls -ael {} \; | sed '/^d/d' | awk '{print $11}' | sed '/^\//!d' | sed 's/^\/jffs\///g' | sed '/^$/d' | sort > /tmp/upload/jffs_files.txt
		find /cifs2 -exec ls -ael {} \; | sed '/^d/d' | awk '{print $11}' | sed '/^\//!d' | sed 's/^\/cifs2\///g' | sed '/^$/d' | sort > /tmp/upload/cifs2_files.txt
	else
		echo "ubi0_0总容量为：${TOTAL_MTD_SIZE}KB, USB磁盘中.usb_jffs文件夹大小：${SPACE_USB_JFFS}KB"
		echo "ubi0_0空间容量不够，只进行系统相关文件的同步，软件中心和插件文件不进行同步！"
		# 文件夹同步
		# ls 找出所有的目录，写入文件
		ls -aelR /jffs/ | grep -i "^/" | sed 's/:$//g' | sed 's/^\/jffs\///g' | sed '/^$/d' | sed '/koolshare/d' | sed '/dnsmasq.d/d' | sort > /tmp/upload/jffs_folders.txt
		ls -aelR /cifs2/ | grep -i "^/" | sed 's/:$//g' | sed 's/^\/cifs2\///g' | sed '/^$/d' | sed '/koolshare/d' | sed '/dnsmasq.d/d' | sort > /tmp/upload/cifs2_folders.txt
		# 文件同步
		# find 找出/jffs下所有的文件+目录，ls列出来后，只取文件的名字，写入文件（因为固件不支持-type f，只能这么搞了）
		find /jffs -exec ls -ael {} \; | sed '/^d/d' | awk '{print $11}' | sed '/^\//!d' | sed 's/^\/jffs\///g' | sed '/^$/d' | sed '/koolshare/d' | sed '/dnsmasq.d/d' | sort > /tmp/upload/jffs_files.txt
		find /cifs2 -exec ls -ael {} \; | sed '/^d/d' | awk '{print $11}' | sed '/^\//!d' | sed 's/^\/cifs2\///g' | sed '/^$/d' | sed '/koolshare/d' | sed '/dnsmasq.d/d' | sort > /tmp/upload/cifs2_files.txt
	fi
	# 开始同步
	echo "文件同步：USB_JFFS → MTD_JFFS！（/jffs → /cifs2）"
	if [ -s /tmp/upload/cifs2_folders.txt -a -s /tmp/upload/jffs_folders.txt ]; then
		# /cifs2和/jffs下都有目录的情况下
		# MTD_JFFS目录下独有的目录
		local mtd_uniq_folders=$(awk 'NR==FNR{a[$1]=$1} NR>FNR{if(a[$1] == ""){print $1}}' /tmp/upload/jffs_folders.txt /tmp/upload/cifs2_folders.txt)
		for mtd_uniq_folder in ${mtd_uniq_folders}; do
			echo "删除文件夹：${mtd_uniq_folder}"
			rm -rf /cifs2/${mtd_uniq_folder}
		done

		# USB_JFFS目录下独有的目录
		local usb_uniq_folders=$(awk 'NR==FNR{a[$1]=$1} NR>FNR{if(a[$1] == ""){print $1}}' /tmp/upload/cifs2_folders.txt /tmp/upload/jffs_folders.txt)
		for usb_uniq_folder in ${usb_uniq_folders}; do
			echo "添加文件夹：${usb_uniq_folder}"
			mkdir -p /cifs2/${usb_uniq_folder}
		done
	elif [ ! -s /tmp/upload/cifs2_folders.txt -a -s /tmp/upload/jffs_folders.txt ]; then
		# CIFS2下没有任何目录，USB_JFFS所有目录都是独有的
		local usb_uniq_folders=$(cat /tmp/upload/jffs_folders.txt)
		for usb_uniq_folder in ${usb_uniq_folders}; do
			echo "添加文件夹：${usb_uniq_folder}"
			mkdir -p /cifs2/${usb_uniq_folder}
		done
	else
		echo "文件同步：未知错误，请重启或者格式化USB磁盘后重试！"
		return
	fi

	# MTD_JFFS目录下独有的文件
	local mtd_uniq_files=$(awk 'NR==FNR{a[$1]=$1} NR>FNR{if(a[$1] == ""){print $1}}' /tmp/upload/jffs_files.txt /tmp/upload/cifs2_files.txt)
	for mtd_uniq_file in ${mtd_uniq_files}; do
		echo "删除文件：${mtd_uniq_file}"
		rm -rf /cifs2/${mtd_uniq_file}
	done

	# USB_JFFS目录下独有的文件
	local usb_uniq_files=$(awk 'NR==FNR{a[$1]=$1} NR>FNR{if(a[$1] == ""){print $1}}' /tmp/upload/cifs2_files.txt /tmp/upload/jffs_files.txt)
	for usb_uniq_file in ${usb_uniq_files}; do
		echo "添加文件：${usb_uniq_file}"
		cp -af /jffs/${usb_uniq_file} /cifs2/${usb_uniq_file}
	done

	# 更新变化的文件
	# 1. × 使用时间戳，在生成文件列表的时候附带生成时间戳，用以判断文件是否变化，但是实际测试但是很不准，但是可以告知用户哪些文件变化了
	# 2. √ 使用md5sum，每个文件都要比较，非常消耗！！但是这个至少比较准确，可以告知用户那些文件变化了
	local FILES=$(cat /tmp/upload/jffs_files.txt)
	for FILE in ${FILES}
	do
		USB_MD5=$(md5sum /jffs/$FILE | awk '{print $1}' 2>/dev/null)
		MTD_MD5=$(md5sum /cifs2/$FILE | awk '{print $1}' 2>/dev/null)
		if [ "${USB_MD5}" != "${MTD_MD5}" ];then
			echo "更新文件：${FILE}"
			cp -af /jffs/${FILE} /cifs2/${FILE}
		fi
	done

	rm -rf /tmp/upload/jffs_folders.txt
	rm -rf /tmp/upload/cifs2_folders.txt
	rm -rf /tmp/upload/jffs_files.txt
	rm -rf /tmp/upload/cifs2_files.txt
	sync
	echo "同步完成！"
}

set_sync_job(){
	usb2jffs_sync_time=`nvram get usb2jffs_sync_time`
	if [ "$usb2jffs_sync_time" -eq "0" ]; then
		echo "删除插件定时同步任务..."
		sed -i '/usb2jffs_sync/d' /var/spool/cron/crontabs/* >/dev/null 2>&1
	elif [ "$usb2jffs_sync_time" -gt "0" ]; then
		echo "设置每隔${usb2jffs_sync_time}分钟同步文件..."
		cru a usb2jffs_sync "*/${usb2jffs_sync_time} * * * * sh /usr/sbin/usb2jffs.sh sync"
	fi
}

case $1 in
mount)
	echo "========================= USB2JFFS - 手动挂载 ========================"
	mount_jffs
	echo "========================= USB2JFFS - 运行完成 ========================"
	;;
unmount)
	echo "========================= USB2JFFS - 手动卸载 ========================"
	unmount_jffs
	echo "========================= USB2JFFS - 运行完成 ========================"
	;;
remove)
	echo "========================= USB2JFFS - 手动删除 ========================"
	remove_jffs
	echo "========================= USB2JFFS - 运行完成 ========================"
	;;
sync)
	echo "========================= USB2JFFS - 手动同步 ========================"
	sync_usb_mtd
	echo "========================= USB2JFFS - 运行完成 ========================"
	;;
setjob)
	echo "========================= USB2JFFS - 定时同步 ========================"
	set_sync_job
	echo "========================= USB2JFFS - 运行完成 ========================"
	;;
*)
	echo "用法:usb2jffs.sh [mount] [unmount] [remove] [sync]"
	;;
esac
