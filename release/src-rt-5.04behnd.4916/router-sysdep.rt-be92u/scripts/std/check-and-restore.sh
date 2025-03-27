#!/bin/sh


check_and_restore() {
#
#   $1 - directory to where files will be restored
#   $2 - files to restore
#    
    defaults_part_name="/mnt/defaults"

    if [[ "$2" ]]; then
        for fname in $2
        do
            echo "check-and-restore:  ${fname} "
            tgt_fname_with_path=$1/$fname
            src_fname_with_path=${defaults_part_name}/$fname
                if [[  -f ${src_fname_with_path} ]]; then
                    if [[  -f ${tgt_fname_with_path} ]]; then
                        echo "check_and_restore: file ${tgt_fname_with_path} exists"
                    else 
                        echo "check_and_restore: copying ${src_fname_with_path} to ${tgt_fname_with_path}" 
                        cp ${src_fname_with_path} ${tgt_fname_with_path}
                    fi
                fi        
        done
    fi

}

case "$1" in
    start)
        boardid=`cat /proc/nvram/boardid`   # for future extensions
        check_and_restore "/data" __DEFAULT_FILES_LIST__
        ;;
    stop)
        ;;
esac
