#!/bin/sh

boot_dev=``
rdev=`which rdev`
if [ ! -z "$rdev" ]; then
	# Try rdev
	boot_dev=`$rdev`
	if [ ! -z "$boot_dev" ]; then
		# Block based rootfs
		boot_dev=${boot_dev%% *}
		boot_dev=${boot_dev##*/}
	else
		# Probably non-block rootfs, check ubifs
		boot_dev=`mount | grep " / "`
		boot_dev=${boot_dev%% *}
		boot_dev=${boot_dev##*/}
		if [ "$boot_dev" != "overlay" ]; then
			echo -n "$boot_dev" 
			exit 0;
		fi
		# boot_dev is overlay. This could be the rootfs of container.
		# Set boot_dev to empty string to get from bootline
		boot_dev=``
	fi
fi

if [ -z "$boot_dev" ]; then 
	# No rdev, rely on bootline - legacy method
	boot_dev=`cat /proc/cmdline`
	boot_dev=${boot_dev##*root=}
	boot_dev=${boot_dev%% *}

	# Check if non-block ubi rootfs
	non_blk_ubi=`expr "$boot_dev" : 'ubi:'`
	if [ $non_blk_ubi -ne 0 ]; then
		echo -n "$boot_dev"
		exit 0;
	fi

	# Block device rootfs
	boot_dev=${boot_dev##*/}
fi

# Block devices - check for dm mapped targets
mapped_dev=`expr "$boot_dev" : 'dm-'`
while [ $mapped_dev -ne 0 ]
do
	boot_dev=`ls /sys/block/${boot_dev}/slaves`
	mapped_dev=`expr "${boot_dev}" : 'dm-'`
done
echo -n "/dev/$boot_dev"
