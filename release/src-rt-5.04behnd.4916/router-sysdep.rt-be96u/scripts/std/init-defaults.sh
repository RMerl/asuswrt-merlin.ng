#!/bin/sh

init_defaults() {
#
#   $1 - directory to find initial default files
#   $2 - files to restore
#    
    defaults_part_name="/mnt/defaults"

    if [[ "$2" ]]; then
        for fname in $2
        do
            src_fname_with_path=$1/$fname
            tgt_fname_with_path=${defaults_part_name}/$fname
                if [[  -f ${src_fname_with_path} ]]; then
                    if [[  -f ${tgt_fname_with_path} ]]; then
                        :
                    else 
                        echo "init_defaults: copying ${src_fname_with_path} to ${tgt_fname_with_path}" 
                        mount -t ubifs -o remount,rw ubi:defaults /mnt/defaults
                        sleep 1
                        cp ${src_fname_with_path} ${tgt_fname_with_path}
                        sync
                        mount -t ubifs -o remount,ro ubi:defaults /mnt/defaults
                    fi
                fi        
        done
    fi

}

case "$1" in
    start)
        boardid=`cat /proc/nvram/boardid`   # for future extensions
        init_defaults "/data" __INIT_DEFAULT_FILES_LIST__
        ;;
    stop)
        ;;
esac
