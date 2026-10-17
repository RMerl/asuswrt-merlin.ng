#!/bin/sh


HINT_OF_PRPL_STORAGE_INIT_DONE="/var/initPrplStorageServiceDone"
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


is_prpl_storage_ready()
{
    maxRetries=5
    while [ ! -e $HINT_OF_PRPL_STORAGE_INIT_DONE ];
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

case "$ACTION" in
    "add")
        if [ -z $partition ]; then
            echo 1024 > "/sys/block/$dev_name/queue/read_ahead_kb"
            if [ $(is_prpl_storage_ready) -eq 1 ]; then
                phyName="disk$disk"
                # echo "ubus call Storage.StorageService.1.PhysicalMedium _add '{"name":"'"${phyName}"'","parameters":{"Name":"'"${phyName}"'"}}'" >> /tmp/hotplug.log
                ubus call Storage.StorageService.1.PhysicalMedium _add '{"name":"'"${phyName}"'","parameters":{"Name":"'"${phyName}"'"}}'
            fi
            if [ -e "$dev_name"?"" ]; then
                # echo "partition exist. Do mount in next run(ex, sda1)." >> /tmp/hotplug.log
                exit 0
            else
                # echo "partition not exist. Try mount disk on physical device(ex, sda)" >> /tmp/hotplug.log
                mnt="/mnt/disk$disk"_0""
                partition=0
            fi
        fi
        mkdir -p $mnt
        rc=$?; if [ ${rc} -ne 0 ]; then exit 1;fi
        name="disk$disk"_"$partition"
        # echo "$name" >> /tmp/hotplug.log
        busybox mount -t auto /dev/$dev_name $mnt
        rc=$?;
        if [ ${rc} -ne 0 ]; then
          busybox umount -l $mnt
          rm -r $mnt
          exit 1
        fi
        if [ $(is_prpl_storage_ready) -eq 1 ]; then
             # echo "ubus call Storage.StorageService.1.LogicalVolume _add" >> /tmp/hotplug.log
             ubus call Storage.StorageService.1.LogicalVolume _add '{"name":"'"${name}"'","parameters":{"Name":"'"${name}"'"}}'
        fi

        # echo " - add $dev_name done" >> /tmp/hotplug.log
        ;;
    "remove")
        # echo "remove" >> /tmp/hotplug.log
        if [ -z $partition ]; then
            if [ $(is_prpl_storage_ready) -eq 1 ]; then
                phyName="disk$disk"
                # echo "ubus call Storage.StorageService.1.PhysicalMedium _del" >> /tmp/hotplug.log
                ubus call Storage.StorageService.1.PhysicalMedium _del '{"name":"'"${phyName}"'"}'
            fi

            if [ -e "$dev_name"?"" ]; then
                # echo "partition exist." >> /tmp/hotplug.log
                exit 0;
            else
                # echo "partition not exist. Try umount disk" >> /tmp/hotplug.log;
                mnt="/mnt/disk$disk"_0""
                partition=0
            fi
        fi
        if [ $(is_prpl_storage_ready) -eq 1 ]; then
             name="disk$disk"_"$partition"
             # echo "ubus call Storage.StorageService.1.LogicalVolume _del '{"name":"$name"}'" >> /tmp/hotplug.log
             ubus call Storage.StorageService.1.LogicalVolume _del '{"name":"'"${name}"'"}'
        fi
        busybox umount -l $mnt
        rc=$?; if [ ${rc} -ne 0 ]; then exit 1;fi
        rm -r $mnt
        rc=$?; if [ ${rc} -ne 0 ]; then exit 1;fi
        # echo " - remove $dev_name done" >> /tmp/hotplug.log

        ;;
esac
exit 0

