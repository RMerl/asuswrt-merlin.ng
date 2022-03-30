#!/bin/bash
# SPDX-License-Identifier: GPL-2.0
#
######################################################
# Copyright (C) 2016 Marvell International Ltd.
#
# https://spdx.org/licenses
#
# Author: Konstantin Porotchkin kostap@marvell.com
#
# Version 0.3
#
# UART recovery downloader for Armada SoCs
#
######################################################

port=$1
file=$2
speed=$3

pattern_repeat=1500
default_baudrate=115200
tmpfile=/tmp/xmodem.pattern
tools=( dd stty sx minicom )

case "$3" in
    2)
        fast_baudrate=230400
        prefix="\xF2"
        ;;
    4)
        fast_baudrate=460800
        prefix="\xF4"
        ;;
    8)
    	fast_baudrate=921600
        prefix="\xF8"
        ;;
    *)
    	fast_baudrate=$default_baudrate
        prefix="\xBB"
esac

if [[ -z "$port" || -z "$file" ]]
then
    echo -e "\nMarvell recovery image downloader for Armada SoC family."
    echo -e "Command syntax:"
    echo -e "\t$(basename $0) <port> <file> [2|4|8]"
    echo -e "\tport  - serial port the target board is connected to"
    echo -e "\tfile  - recovery boot image for target download"
    echo -e "\t2|4|8 - times to increase the default serial port speed by"
    echo -e "For example - load the image over ttyUSB0 @ 460800 baud:"
    echo -e "$(basename $0) /dev/ttyUSB0 /tmp/flash-image.bin 4\n"
    echo -e "=====WARNING====="
    echo -e "- The speed-up option is not available in SoC families prior to A8K+"
    echo -e "- This utility is not compatible with Armada 37xx SoC family\n"
fi

# Sanity checks
if [ -c "$port" ]
then
   echo -e "Using device connected on serial port \"$port\""
else
   echo "Wrong serial port name!"
   exit 1
fi

if [ -f "$file" ]
then
   echo -e "Loading flash image file \"$file\""
else
   echo "File $file does not exist!"
   exit 1
fi

# Verify required tools installation
for tool in ${tools[@]}
do
    toolname=`which $tool`
    if [ -z "$toolname" ]
    then
        echo -e "Missing installation of \"$tool\" --> Exiting"
        exit 1
    fi
done


echo -e "Recovery will run at $fast_baudrate baud"
echo -e "========================================"

if [ -f "$tmpfile" ]
then
    rm -f $tmpfile
fi

# Send the escape sequence to target board using default debug port speed
stty -F $port raw ignbrk time 5 $default_baudrate
counter=0
while [ $counter -lt $pattern_repeat ]; do
    echo -n -e "$prefix\x11\x22\x33\x44\x55\x66\x77" >> $tmpfile
    let counter=counter+1
done

echo -en "Press the \"Reset\" button on the target board and "
echo -en "the \"Enter\" key on the host keyboard simultaneously"
read
dd if=$tmpfile of=$port &>/dev/null

# Speed up the binary image transfer
stty -F $port raw ignbrk time 5 $fast_baudrate
sx -vv $file > $port < $port
#sx-at91 $port $file

# Return the port to the default speed
stty -F $port raw ignbrk time 5 $default_baudrate

# Optional - fire up Minicom
minicom -D $port -b $default_baudrate

