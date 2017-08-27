#!/bin/sh
# Enable Swap script
# $1: usb path  $2: Swap enable/disable

SWAP_THRESHOLD=`nvram get apps_swap_threshold`
SWAP_FILE=`nvram get apps_swap_file`
SWAP_SIZE=`nvram get apps_swap_size`
USB_NAME=`nvram get usb_path1_act`
USB_PATH=/tmp/mnt/$USB_NAME

if [ -z "$1" ]; then
	USB_PATH=/tmp/mnt/$USB_NAME
else
	USB_PATH=$1	
fi

if [ -z "$2" ]; then
	SWAP_ENABLE=`nvram get apps_swap_enable`
else
	SWAP_ENABLE=$2	
fi

echo "USB_PATH :" $USB_PATH
echo "SWAP_ENABLE :" $SWAP_ENABLE
echo "SWAP_THRESHOLD :" $SWAP_THRESHOLD
echo "SWAP_SIZE :"  $SWAP_SIZE

if [ "$SWAP_ENABLE" != "1" ]; then
	echo "Disable to swap!"	
	if [ -e "$USB_PATH/$SWAP_FILE" ]; then
		swapoff $USB_PATH/$SWAP_FILE
		rm -rf $USB_PATH/$SWAP_FILE
		echo 1 > /proc/sys/vm/drop_caches
	fi	
else
	mem_size=`free |sed '1,3d' |awk '{print $4}'`
	if [ "$SWAP_THRESHOLD" == "" ] || [ $mem_size -lt $SWAP_THRESHOLD ]; then
		pool_size=`df /dev/$APPS_DEV |sed '1d' |awk '{print $4}'`
		if [ $pool_size -gt $SWAP_SIZE ]; then
			if [ -e "$USB_PATH/$SWAP_FILE" ]; then
				swapoff $USB_PATH/$SWAP_FILE
				rm -rf $USB_PATH/$SWAP_FILE
			fi

			swap_count=`expr $SWAP_SIZE / 1000 - 1`
			echo "dd if=/dev/zero of=$USB_PATH/$SWAP_FILE bs=1M count=$swap_count"
			dd if=/dev/zero of=$USB_PATH/$SWAP_FILE bs=1M count=$swap_count
			echo "mkswap $USB_PATH/$SWAP_FILE"
			mkswap $USB_PATH/$SWAP_FILE
			echo "swapon $USB_PATH/$SWAP_FILE"
			swapon $USB_PATH/$SWAP_FILE
		else
			SWAP_SIZE=$(($pool_size -$pool_size % 10000))
			echo "pool_size is " $pool_size
			echo "No enough partition size! Change SWAP_SIZE to" $SWAP_SIZE 
				if [ $pool_size -gt $SWAP_SIZE ]; then
					if [ -e "$USB_PATH/$SWAP_FILE" ]; then
						swapoff $USB_PATH/$SWAP_FILE
						rm -rf $USB_PATH/$SWAP_FILE
					fi

					swap_count=`expr $SWAP_SIZE / 1000 - 1`
					echo "dd if=/dev/zero of=$USB_PATH/$SWAP_FILE bs=1M count=$swap_count"
					dd if=/dev/zero of=$USB_PATH/$SWAP_FILE bs=1M count=$swap_count
					echo "mkswap $USB_PATH/$SWAP_FILE"
					mkswap $USB_PATH/$SWAP_FILE
					echo "swapon $USB_PATH/$SWAP_FILE"
					swapon $USB_PATH/$SWAP_FILE
				else
					echo "No enough partition size! Can't crate swap!"
					exit 1
				fi
		fi
		echo 1 > /proc/sys/vm/drop_caches
	fi
fi

