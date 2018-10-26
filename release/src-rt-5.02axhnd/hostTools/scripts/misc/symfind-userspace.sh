#!/bin/sh

#
# Usage symfind_userspace.sh console_log_file
#
# Be sure to run dos2unix on the console_log_file first, otherwise, the
# extra carriage return in windows prevents this script from finding the symbols.
#

# point to location in your tree where lib is not stripped, in the userspace dir
VIEW_PATH="/home/miwang/miwang_cc/404-dev/CommEngine/userspace"
TOOLCHAIN_PATH="/opt/toolchains/uclibc-crosstools-gcc-4.2.3-3/lib"

# For desktop linux, NM is just nm
#NM=nm
NM="/opt/toolchains/uclibc-crosstools-gcc-4.2.3-3/usr/mips-linux-uclibc/bin/nm"

SYMBOL=""

find_symbol()
{
    echo "Looking for symbol $2 in file $1"

	# Look for the binary file with symbols
	FILE=`find $VIEW_PATH -name $1 -type f -print0 -quit`
	if [ -z $FILE ]; then
		FILE=`find $TOOLCHAIN_PATH -name $1 -type f -print0 -quit`
		if [ -z $FILE ]; then
                        echo "Sorry, could not find $1 in $VIEW_PATH"
			return 1
		fi
	fi

	if file $FILE|grep "shared object.*, stripped" >/dev/null 2>/dev/null
	then
		SYMBOL=`$NM -Dnr $FILE|awk -v var=$2 '{if ($1 <= var) {print $3; exit}}' 2>/dev/null`
	else
		SYMBOL=`$NM -nr $FILE|awk -v var=$2 '{if ($1 <= var) {print $3; exit}}' 2>/dev/null`
	fi
	if [ -n $SYMBOL ]
	then
		return 0
	else
		return 1
	fi
}

while read LINE
do
	if echo $LINE|grep '^#[0-9]\ \[[0-9a-f]\{8\}\] in .*' >/dev/null 2>/dev/null
	then
		ADDR=`echo $LINE | awk '{print substr($2, 2, 8)}'`
		FULL_PATH=`echo $LINE | awk '{print $4}'`
		FILE=$(echo $FULL_PATH | awk -F"/" '{print $NF}')
		if find_symbol $FILE $ADDR
		then
			echo $LINE|sed "s/\[.*\]/ $SYMBOL()/g"
		else
			echo $LINE
		fi
	else
		echo $LINE
	fi
done < $1

