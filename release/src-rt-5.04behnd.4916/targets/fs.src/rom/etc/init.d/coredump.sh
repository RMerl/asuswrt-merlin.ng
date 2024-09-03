#!/bin/sh

# Application coredump collector, used to set /proc/sys/kernel/core_pattern by system-config.sh.
# Functions:
# - crash log disk space limit
# - compress
#
# Parameters:
# $1 - new  save locations
# $2 - last save locations
# $3 - MINI:minicoredumper, or DEFAULT:Linux coredump
# $4 - %E{%e} (app pathname{threadname}. e.g. !bin!bas_tr143{BAS_IPC})
# $5 - %P (Process ID)
# $6 - %u (User ID)
# $7 - %g (Group ID)
# $8 - %s (Signal)
# $9 - %t (Timestamp)
# ${10} - %h (Host name)

CORE_DIR_NEW=$1
CORE_DIR_LAST=$2
# Obtain app and thread name
APP_NAME=`echo $4 | awk -F '!' '{print $NF}'`
# If change prefix, please ensure backward compatibility
CORE_NAME_PREFIX=core_
CORE_NAME=${CORE_NAME_PREFIX}${APP_NAME}.gz

# Max total size(KB) limit. 4096KB = 4MB
MAX_TOTAL_SIZE=4096
# Max individual core size(KB) after compressed. 3072KB = 3MB
MAX_CORE_SIZE=3072

if [ $3 == "MINI" ]; then
    CORE_BASE_DIR=/var/crash/minicoredumper
    CORE_FILE_TMP_DIR=$CORE_BASE_DIR/${APP_NAME}.$(date -d @$9 +"%Y%m%d.%H%M%S%z").$5
    CORE_FILE_TMP=$CORE_FILE_TMP_DIR/core.gz

    mkdir -p $CORE_BASE_DIR

    # para order: |/bin/minicoredumper %P %u %g %s %t %h %e
    cat |/bin/minicoredumper $5 $6 $7 $8 $9 ${10} ${APP_NAME}
else
    CORE_FILE_TMP=/tmp/$CORE_NAME.$9
    /bin/gzip -f - | dd bs=1k count=$MAX_CORE_SIZE of=$CORE_FILE_TMP
fi

# Delete existing coredump file for the same app+thread
rm -f $CORE_DIR_NEW/$CORE_NAME

size_used_new=$(du -d 0 -k $CORE_DIR_NEW | awk -F " " '{print $1}')
size_used_last=$(du -d 0 -k $CORE_DIR_LAST | awk -F " " '{print $1}')
size_write=$(du -d 0 -k $CORE_FILE_TMP | awk -F " " '{print $1}')
size_total=$(($size_used_new + $size_used_last + $size_write))

# If no enough disk space, delete some coredumps until free space is enough.
# Delete policy:
# 1. delete all coredumps under last
# 2. delete the oldest coredumps under new

# Delete#1
if [ $size_total -gt $MAX_TOTAL_SIZE ]; then
    rm -f $CORE_DIR_LAST/${CORE_NAME_PREFIX}*
fi
size_used_last=$(du -d 0 -k $CORE_DIR_LAST | awk -F " " '{print $1}')

# Delete#2
for file in $(ls $CORE_DIR_NEW/${CORE_NAME_PREFIX}* -rt); do
    size_used_new=$(du -d 0 -k $CORE_DIR_NEW | awk -F " " '{print $1}')
    size_total=$(($size_used_new + $size_used_last + $size_write))
    if [ $size_total -lt $MAX_TOTAL_SIZE ]; then
        break
    fi
    rm -f $file    
done
size_used_new=$(du -d 0 -k $CORE_DIR_NEW | awk -F " " '{print $1}')

size_total=$(($size_used_new + $size_used_last + $size_write))
if [ $size_total -lt $MAX_TOTAL_SIZE ]; then
    mv $CORE_FILE_TMP $CORE_DIR_NEW/$CORE_NAME
fi

rm -f $CORE_FILE_TMP
rm -rf $CORE_FILE_TMP_DIR
