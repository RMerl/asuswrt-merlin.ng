#!/bin/sh
# This script runs before kernel load modules, first wlcsm modules is required to load as
# first one to serve nvram for lkm and user spaces applications  LNXVERSION will be replaced
# by LINUX_VER_STR

vol_nm="mfg_data"  			#nvram volume and dir name
wl_srom_nm=".wlsromcustomerfile.nvm"	#wl calibration file name
mfg_base="mnt"
to_mount=$(cat /proc/nvram/wl_nand_manufacturer)
is_nand=$(($to_mount & 1))
is_manufacturer=$(($to_mount & 2))
nand_hassize=$(($to_mount & 4))
original_kernel_nvram_file="/rom/etc/wlan/KERNEL_NVRAM_FILE_NAME"
kernel_nvram_file="/data/.KERNEL_NVRAM_FILE_NAME"

case "$1" in
	start)
		# first to check if it is NAND, then check calibration data
		# if calibration data is not there, try to restore from hidden
		# partition, OR if the mfg_data partition's size is not set,
		# there is no action since to_mount will be 0
		echo "is_nand:$is_nand  is_manufacturer:$is_manufacturer"
		if [ $is_nand -eq 1 ] && [ $nand_hassize -gt 0 ]; then
			mtd_name="/dev/$(cat /proc/nvram/wl_nand_mtdname)"
			if [ $is_manufacturer -eq 2 ] || [ ! -f /data/$wl_srom_nm ]; then
				#attach the mtd device,then we will find ubi and check
				ubinum=$(ubiattach -p $mtd_name 2>&1 |
					{ read a
				          b=${a##UBI device number }
					  c=`echo $b | { read d e
					  echo ${d%%,}
					}`
					echo $c 
				        })
				ubi_name="ubi$ubinum"
				#simple way to see if ubinum is a number as simple shell
				#on gateway does not support regex
				ubinum=$(($ubinum + 1))
				if [ $? -eq 0 ]; then
				has_volume=$(cat /sys/class/ubi/$ubi_name/volumes_count)
				if [ $has_volume -eq 0 ]
				then
					echo "To create volume on $ubi_name"
					ubidetach -p $mtd_name>/dev/null 2>&1
					ubiformat -y $mtd_name>/dev/null 2>&1
					ubiattach -p $mtd_name>/dev/null 2>&1
					dev_num=$(cat /sys/class/ubi/$ubi_name/dev)
					idx=`expr index $dev_num ":"`
					if [ $idx -gt 0 ]
					then
						idx=$(($idx-1))
						cdev=${dev_num:0:$idx}
						mknod /tmp/$ubi_name c $cdev 0
						ubimkvol /tmp/$ubi_name -N $vol_nm -m
					fi
				fi

				mkdir -p /$mfg_base/$vol_nm
				mount -t ubifs $ubi_name:$vol_nm /$mfg_base/$vol_nm

				if [ ! -f /data/$wl_srom_nm ]; then
					cp /$mfg_base/$vol_nm/$wl_srom_nm /data/$wl_srom_nm>/dev/null  2>&1
				fi

				if [ $is_manufacturer -ne 2 ]; then
					umount /$mfg_base/$vol_nm>/dev/null 2>&1
					ubidetach -p  $mtd_name>/dev/null 2>&1
					rm -rf /$mfg_base/$vol_nm>/dev/null 2>&1
				fi
				fi
			fi
		fi

		exit 0
		;;
	stop)
		sync
		if [ $is_nand -eq 1 ] && [ $nand_hassize -gt 0 ]; then
	         mtd_name="/dev/$(cat /proc/nvram/wl_nand_mtdname)"
		 if [ $is_manufacturer -gt 0 ]; then
			umount /$mfg_base/$vol_nm>/dev/null 2>&1
			ubidetach -p  $mtd_name>/dev/null 2>&1
			rm -rf /$mfg_base/$vol_nm>/dev/null 2>&1
		 fi
		fi
		exit 0
		;;

	*)
		echo "$0: unrecognized option $1"
		;;

esac
