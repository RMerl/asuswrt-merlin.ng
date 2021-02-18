#!/bin/sh -euf

#
# This script inserts NAND simulator module to emulate NAND flash of specified
# size.
#
# Author: Artem Bityutskiy
#

fatal()
{
        echo "Error: $1" 1>&2
        exit 1
}

usage()
{
	cat 1>&2 <<EOF
Load NAND simulator to simulate flash of a specified size.

Usage: ${0##*/} <size in MiB> <eraseblock size in KiB> \\
       <page size (512 or 2048)>

Only the first parameter is mandatory. Default eraseblock size
is 16KiB, default NAND page size is 512 bytes.

Only the following combinations are supported:
--------------------------------------------------
| size (MiB) | EB size (KiB) | Page size (bytes) |
--------------------------------------------------
| 16         | 16            | 512               |
| 32         | 16            | 512               |
| 64         | 16            | 512               |
| 128        | 16            | 512               |
| 256        | 16            | 512               |
| 64         | 64            | 2048              |
| 64         | 128           | 2048              |
| 64         | 256           | 2048              |
| 64         | 512           | 2048              |
| 128        | 64            | 2048              |
| 128        | 128           | 2048              |
| 128        | 256           | 2048              |
| 128        | 512           | 2048              |
| 256        | 64            | 2048              |
| 256        | 128           | 2048              |
| 256        | 256           | 2048              |
| 256        | 512           | 2048              |
| 512        | 64            | 2048              |
| 512        | 128           | 2048              |
| 512        | 256           | 2048              |
| 512        | 512           | 2048              |
| 1024       | 64            | 2048              |
| 1024       | 128           | 2048              |
| 1024       | 256           | 2048              |
| 1024       | 512           | 2048              |
--------------------------------------------------
EOF
}

if grep -q "NAND simulator" /proc/mtd; then
	fatal "nandsim is already loaded"
fi

if [ "$#" -lt "1" ]; then
	usage
	exit 1
fi

size="$1"
eb_size="$2"
page_size="$3"
if [ "$#" = "1" ]; then
	eb_size="16"
	page_size="512"
elif [ "$#" = "2" ]; then
	page_size="512"
fi

if [ "$page_size" -eq 512 ] && [ "$eb_size" -ne "16" ]; then
	fatal "only 16KiB eraseblocks are possible in case of 512 bytes page"
fi

first=
second=
third=
fourth=

if [ "$page_size" -eq "512" ]; then
	first="0x20"
	case "$size" in
	16)  second=0x33 ;;
	32)  second=0x35 ;;
	64)  second=0x36 ;;
	128) second=0x78 ;;
	256) second=0x71 ;;
	*) fatal "flash size ${size}MiB is not supported, try 16, 32, 64 or 256"
	esac
elif [ "$page_size" -eq "2048" ]; then
	case "$eb_size" in
	64)  fourth="0x05" ;;
	128) fourth="0x15" ;;
	256) fourth="0x25" ;;
	512) fourth="0x35" ;;
	*)   fatal "eraseblock ${eb_size}KiB is not supported"
	esac


	case "$size" in
	64)   first="0x20"; second="0xa2"; third="0x00 ";;
	128)  first="0xec"; second="0xa1"; third="0x00 ";;
	256)  first="0x20"; second="0xaa"; third="0x00 ";;
	512)  first="0x20"; second="0xac"; third="0x00 ";;
	1024) first="0xec"; second="0xd3"; third="0x51 ";;
	*) fatal "unable to emulate ${size}MiB flash with ${eb_size}KiB eraseblock"
	esac
else
	fatal "bad NAND page size ${page_size}KiB, it has to be either 512 or 2048"
fi

first="first_id_byte=$first"
second="second_id_byte=$second"
[ -z "$third" ]  || third="third_id_byte=$third"
[ -z "$fourth" ] || fourth="fourth_id_byte=$fourth"

modprobe nandsim "$first" "$second" $third $fourth

echo "Loaded NAND simulator (${size}MiB, ${eb_size}KiB eraseblock, $page_size bytes NAND page)"
