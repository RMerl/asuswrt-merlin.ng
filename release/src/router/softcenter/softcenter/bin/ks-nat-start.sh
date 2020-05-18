#!/bin/sh

ACTION=$1
#ACTION="start_nat"

#echo start `date` > /tmp/ks_nat_log.txt

ks_nat=`nvram get ks_nat`
[ "$ks_nat" == "1" ] && echo exit `date` >> /tmp/ks_nat_log.txt && exit

for i in $(find /koolshare/init.d/ -name 'N*' | sort) ;
do
    case "$i" in
        *.sh )
            # Source shell script for speed.
            trap "" INT QUIT TSTP EXIT
            #set $1
            logger "plugin_nat_log_1 $i"
            if [ -r "$i" ]; then
            $i $ACTION
            fi
            ;;
        *)
            # No sh extension, so fork subprocess.
            logger "plugin_nat_log_2 $i"
            . $i $ACTION
            ;;
    esac
done

#echo finish `date` >> /tmp/ks_nat_log.txt
