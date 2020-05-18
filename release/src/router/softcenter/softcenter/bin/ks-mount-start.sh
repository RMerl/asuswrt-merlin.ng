#!/bin/sh

export KSROOT=/koolshare
source $KSROOT/scripts/base.sh

ACTION=$1
KSPATH=$2

for i in $(find /koolshare/init.d/ -name 'M*' | sort -n) ;
do
    case "$i" in
        M* | *.sh )
            # fork subprocess.
            trap "" INT QUIT TSTP EXIT
            logger "plugin_mount_log_1 $i"
            if [ -r "$i" ]; then
            	$i $ACTION $KSPATH
            fi
            ;;
        *)
            # No sh extension, Source shell script for speed.
            logger "plugin_mount_log_2 $i"
            . $i $ACTION $KSPATH
            ;;
    esac
done
