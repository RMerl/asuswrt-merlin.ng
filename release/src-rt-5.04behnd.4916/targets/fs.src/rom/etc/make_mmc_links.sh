#!/bin/sh

# Check for volname entry for 4.1 Kernel implementation
volname_path="/sys/class/"$2"/"$1"/volname"

if [ -e $volname_path ]
then
    link_path="/dev/"$(cat $volname_path)
else
    # No volname entry, try 4.19 kernel implementation
    mmc_vol_name=`grep PARTNAME /sys/class/$2/$1/uevent`
    mmc_vol_name=${mmc_vol_name#PARTNAME=}
    if [ -z $mmc_vol_name ]
    then
        exit 1
    else
        link_path="/dev/"$mmc_vol_name
        link_path2="/dev/disk/by-label/"$mmc_vol_name
    fi
fi
    
dev_path="/dev/"$1
ln -s $dev_path $link_path

if [ ! -z $link_path2 ]
then
   mkdir -p /dev/disk/by-label
   ln -s $dev_path $link_path2
fi
