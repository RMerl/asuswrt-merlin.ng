#!/bin/sh

export KSROOT=/koolshare
source $KSROOT/scripts/base.sh

ACTION=$1

if [ $# -lt 1 ]; then
    printf "Usage: $0 {start|stop|restart|reconfigure|check|kill}\n" >&2
    exit 1
fi

[ $ACTION = stop -o $ACTION = restart -o $ACTION = kill ] && ORDER="-r"

for i in $(find /koolshare/init.d/ -name 'V*' | sort $ORDER ) ;
do
    case "$i" in
        V* | *.sh )
            # Source shell script for speed.
            trap "" INT QUIT TSTP EXIT
            #set $1
            logger "plugin_service_log_1 $i"
            if [ -r "$i" ]; then
            $i $ACTION
            fi
            ;;
        *)
            # No sh extension, so fork subprocess.
            logger "plugin_service_log_2 $i"
            . $i $ACTION
            ;;
    esac
done
