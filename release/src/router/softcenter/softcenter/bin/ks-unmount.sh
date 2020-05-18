#!/bin/sh

export KSROOT=/koolshare
source $KSROOT/scripts/base.sh

KSPATH=$1

for i in $(find /koolshare/init.d/ -name 'U*' | sort -n) ;
do
    case "$i" in
        U* | *.sh )
            # fork subprocess.
            trap "" INT QUIT TSTP EXIT
            logger "plugin_mount_log_1 $i"
            if [ -r "$i" ]; then
            	$i $KSPATH
            fi
            ;;
        *)
            # No sh extension, Source shell script for speed.
            logger "plugin_mount_log_2 $i"
            . $i $KSPATH
            ;;
    esac
done
