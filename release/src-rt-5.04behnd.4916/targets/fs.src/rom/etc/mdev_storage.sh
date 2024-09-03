#!/bin/sh
source /etc/profile


HINT_OF_CMS_STORAGE_INIT_DONE="/var/initStorageServiceDone"
dev_name=$1
dev_subname="a b c d e f g h i j k"
set -- $dev_subname
idx=0;
while [ -n "$1" ]; do
    idx=$( expr $idx + 1)
    if [ ${dev_name:2:1} = $1 ]; then disk=$idx; break; fi
    shift
done
if [ -z $disk ]; then exit 1;fi

partition=${dev_name:3:1}
mnt="/mnt/disk$disk"_"$partition"
# echo "$ACTION dev_name = $dev_name, mnt disk:partition = $mnt" >> /tmp/hotplug.log

# Below definitions should be synced with
# userspace/public/libs/cms_util/cms_eid.h and userspace/public/libs/cms_msg/cms_msg.h
EID_SMD=20
CMS_MSG_STORAGE_ADD_PHYSICAL_MEDIUM=0x10000310     # /**< A physical storage medium has been added */
CMS_MSG_STORAGE_REMOVE_PHYSICAL_MEDIUM=0x10000311 #/**< A physical storage medium has been removed */
CMS_MSG_STORAGE_ADD_LOGICAL_VOLUME=0x10000312     #/**< A logical storage medium has been added */
CMS_MSG_STORAGE_REMOVE_LOGICAL_VOLUME=0x10000313  #/**< A logical storage medium has been removed */

is_cms_storage_ready()
{
    maxRetries=5
    while [ ! -e $HINT_OF_CMS_STORAGE_INIT_DONE ];
    do
        if [ $maxRetries -le 0 ]; then
            echo "0"
            return;
        else
            sleep 5
        fi
        maxRetries=$((maxRetries-1))
    done

    echo "1"
}

send_cms_message()
{
    # This is for non-cms platforms, just return
    if [ ! -x "/bin/send_cms_msg" ]; then return; fi

    # local_msg_id is got from the parameter
    local_msg_id=$1
    # local_partition_id is 0 when partition is not set, $partition otherwise
    if [ -z $partition ]; then
        local_partition_id=0
    else
        local_partition_id=$partition
    fi

    # send the message to cms ssk
    if [ $(is_cms_storage_ready) -eq 1 ]; then
        send_cms_msg -e $local_msg_id $EID_SMD -D "$disk $local_partition_id $dev_name"
        # echo "send_cms_msg message $local_msg_id eid $EID_SMD disk $disk partition $local_partition_id dev_name $dev_name" >> /tmp/hotplug.log
    fi
}

case "$ACTION" in
    "add")
        if [ -z $partition ]; then
            echo 1024 > "/sys/block/$dev_name/queue/read_ahead_kb"
            send_cms_message $CMS_MSG_STORAGE_ADD_PHYSICAL_MEDIUM

            if [ -e "$dev_name"?"" ]; then
                # echo "partition exist. Do mount in next run(ex, sda1)." >> /tmp/hotplug.log
                exit 0
            else
                # echo "partition not exist. Try mount disk on physical device(ex, sda)" >> /tmp/hotplug.log;
                mnt="/mnt/disk$disk"_0""
                partition=0
            fi
        fi
        mkdir -p $mnt
        rc=$?; if [ ${rc} -ne 0 ]; then exit 1;fi
        mount -t auto /dev/$dev_name $mnt
        rc=$?;
        if [ ${rc} -ne 0 ]; then

          if [ -x "/bin/ntfs-3g" ]; then
            # echo " - mount fail. try ntfs-3g" >> /tmp/hotplug.log;
            ntfs-3g /dev/$dev_name $mnt -o use_ino,direct_io,big_writes
            rc=$?;
            if [ ${rc} -eq 0 ]; then
               send_cms_message $CMS_MSG_STORAGE_ADD_LOGICAL_VOLUME
               # echo " - add $dev_name done ntfs-3g" >> /tmp/hotplug.log
               exit 0
            fi
          fi
          umount -l $mnt
          rm -r $mnt
          exit 1
        fi
        send_cms_message $CMS_MSG_STORAGE_ADD_LOGICAL_VOLUME
        # echo " - add $dev_name done" >> /tmp/hotplug.log

        ;;
    "remove")
        if [ -z $partition ]; then
            send_cms_message $CMS_MSG_STORAGE_REMOVE_PHYSICAL_MEDIUM
            if [ -e "$dev_name"?"" ]; then
                # echo "partition exist." >> /tmp/hotplug.log
                exit 0;
            else
                # echo "partition not exist. Try umount disk" >> /tmp/hotplug.log;
                mnt="/mnt/disk$disk"_0""
                partition=0
            fi
        fi
        send_cms_message $CMS_MSG_STORAGE_REMOVE_LOGICAL_VOLUME
        umount -l $mnt
        rc=$?; if [ ${rc} -ne 0 ]; then exit 1;fi
        rm -r $mnt
        rc=$?; if [ ${rc} -ne 0 ]; then exit 1;fi
        # echo " - remove $dev_name done" >> /tmp/hotplug.log

        ;;
esac
exit 0

