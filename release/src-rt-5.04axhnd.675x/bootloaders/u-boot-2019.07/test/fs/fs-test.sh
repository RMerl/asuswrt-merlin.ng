#!/bin/bash
# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2014 Suriyan Ramasami

# Invoke this test script from U-Boot base directory as ./test/fs/fs-test.sh
# It currently tests the fs/sb and native commands for ext4 and fat partitions
# Expected results are as follows:
# EXT4 tests:
# fs-test.sb.ext4	Summary: PASS: 24 FAIL: 0
# fs-test.nonfs.ext4	Summary: PASS: 24 FAIL: 0
# fs-test.fs.ext4	Summary: PASS: 24 FAIL: 0
# FAT16 tests:
# fs-test.sb.fat16	Summary: PASS: 24 FAIL: 0
# fs-test.nonfs.fat16	Summary: PASS: 24 FAIL: 0
# fs-test.fs.fat16	Summary: PASS: 24 FAIL: 0
# FAT32 tests:
# fs-test.sb.fat32	Summary: PASS: 24 FAIL: 0
# fs-test.nonfs.fat32	Summary: PASS: 24 FAIL: 0
# fs-test.fs.fat32	Summary: PASS: 24 FAIL: 0
# --------------------------------------------
# Total Summary: TOTAL PASS: 216 TOTAL FAIL: 0
# --------------------------------------------

# pre-requisite binaries list.
PREREQ_BINS="md5sum mkfs mount umount dd fallocate mkdir"

# All generated output files from this test will be in $OUT_DIR
# Hence everything is sandboxed.
OUT_DIR="sandbox/test/fs"

# Location of generated sandbox u-boot
UBOOT="./sandbox/u-boot"

# Our mount directory will be in the sandbox
MOUNT_DIR="${OUT_DIR}/mnt"

# The file system image we create will have the $IMG prefix.
IMG="${OUT_DIR}/3GB"

# $SMALL_FILE is the name of the 1MB file in the file system image
SMALL_FILE="1MB.file"

# $BIG_FILE is the name of the 2.5GB file in the file system image
BIG_FILE="2.5GB.file"

# $MD5_FILE will have the expected md5s when we do the test
# They shall have a suffix which represents their file system (ext4/fat16/...)
MD5_FILE="${OUT_DIR}/md5s.list"

# $OUT shall be the prefix of the test output. Their suffix will be .out
OUT="${OUT_DIR}/fs-test"

# Full Path of the 1 MB file that shall be created in the fs image.
MB1="${MOUNT_DIR}/${SMALL_FILE}"
GB2p5="${MOUNT_DIR}/${BIG_FILE}"

# ************************
# * Functions start here *
# ************************

# Check if the prereq binaries exist, or exit
function check_prereq() {
	for prereq in $PREREQ_BINS; do
		if [ ! -x "`which $prereq`" ]; then
			echo "Missing $prereq binary. Exiting!"
			exit
		fi
	done

	# We use /dev/urandom to create files. Check if it exists.
	if [ ! -c /dev/urandom ]; then
		echo "Missing character special /dev/urandom. Exiting!"
		exit
	fi
}

# If 1st param is "clean", then clean out the generated files and exit
function check_clean() {
	if [ "$1" = "clean" ]; then
		rm -rf "$OUT_DIR"
		echo "Cleaned up generated files. Exiting"
		exit
	fi
}

# Generate sandbox U-Boot - gleaned from /test/dm/test-dm.sh
function compile_sandbox() {
	unset CROSS_COMPILE
	NUM_CPUS=$(cat /proc/cpuinfo |grep -c processor)
	make O=sandbox sandbox_config
	make O=sandbox -s -j${NUM_CPUS}

	# Check if U-Boot exists
	if [ ! -x "$UBOOT" ]; then
		echo "$UBOOT does not exist or is not executable"
		echo "Build error?"
		echo "Please run this script as ./test/fs/`basename $0`"
		exit
	fi
}

# Clean out all generated files other than the file system images
# We save time by not deleting and recreating the file system images
function prepare_env() {
	rm -f ${MD5_FILE}.* ${OUT}.*
	mkdir -p ${OUT_DIR}
}

# 1st parameter is the name of the image file to be created
# 2nd parameter is the filesystem - fat16 ext4 etc
# -F cant be used with fat as it means something else.
function create_image() {
	# Create image if not already present - saves time, while debugging
	case "$2" in
		fat16)
		MKFS_OPTION="-F 16"
		FS_TYPE="fat"
		;;
		fat32)
		MKFS_OPTION="-F 32"
		FS_TYPE="fat"
		;;
		ext4)
		MKFS_OPTION="-F"
		FS_TYPE="ext4"
		;;
	esac

	if [ ! -f "$1" ]; then
		fallocate -l 3G "$1" &> /dev/null
		if [ $? -ne 0 ]; then
			echo fallocate failed - using dd instead
			dd if=/dev/zero of=$1 bs=1024 count=$((3 * 1024 * 1024))
			if [ $? -ne 0 ]; then
				echo Could not create empty disk image
				exit $?
			fi
		fi
		mkfs -t "$FS_TYPE" $MKFS_OPTION "$1" &> /dev/null
		if [ $? -ne 0 -a "$FS_TYPE" = "fat" ]; then
			# If we fail and we did fat, try vfat.
			mkfs -t vfat $MKFS_OPTION "$1" &> /dev/null
		fi
		if [ $? -ne 0 ]; then
			echo Could not create filesystem
			exit $?
		fi
	fi
}

# 1st parameter is image file
# 2nd parameter is file system type - fat16/ext4/...
# 3rd parameter is name of small file
# 4th parameter is name of big file
# 5th parameter is fs/nonfs/sb - to dictate generic fs commands or
# otherwise or sb hostfs
# 6th parameter is the directory path for the files. Its "" for generic
# fs and ext4/fat and full patch for sb hostfs
# UBOOT is set in env
function test_image() {
	addr="0x01000008"
	length="0x00100000"

	case "$2" in
		fat*)
		FPATH=""
		PREFIX="fat"
		WRITE="write"
		;;

		ext4)
		# ext4 needs absolute path
		FPATH="/"
		PREFIX="ext4"
		WRITE="write"
		;;

		*)
		echo "Unhandled filesystem $2. Exiting!"
		exit
		;;
	esac

	case "$5" in
		fs)
		PREFIX=""
		WRITE="save"
		SUFFIX=" 0:0"
		;;

		nonfs)
		SUFFIX=" 0:0"
		;;

		sb)
		PREFIX="host "
		WRITE="save"
		SUFFIX="fs -"
		;;

		*)
		echo "Unhandled mode $5. Exiting!"
		exit
		;;

	esac

	# sb always uses full path to mointpoint, irrespective of filesystem
	if [ "$5" = "sb" ]; then
		FPATH=${6}/
	fi

	FILE_WRITE=${3}.w
	FILE_SMALL=$3
	FILE_BIG=$4

	# In u-boot commands, <interface> stands for host or hostfs
	# hostfs maps to the host fs.
	# host maps to the "host bind" that we do

	$UBOOT << EOF
sb=$5
setenv bind 'if test "\$sb" != sb; then host bind 0 "$1"; fi'
run bind
# Test Case 1 - ls
${PREFIX}ls host${SUFFIX} $6
# In addition, test with a nonexistent directory to see if we crash.
${PREFIX}ls host${SUFFIX} invalid_d
#
# We want ${PREFIX}size host 0:0 $3 for host commands and
# host size hostfs - $3 for hostfs commands.
# 1MB is 0x0010 0000
# Test Case 2a - size of small file
${PREFIX}size host${SUFFIX} ${FPATH}$FILE_SMALL
printenv filesize
setenv filesize
# Test Case 2b - size of small file via a path using '..'
${PREFIX}size host${SUFFIX} ${FPATH}SUBDIR/../$FILE_SMALL
printenv filesize
setenv filesize

# 2.5GB (1024*1024*2500) is 0x9C40 0000
# Test Case 3 - size of big file
${PREFIX}size host${SUFFIX} ${FPATH}$FILE_BIG
printenv filesize
setenv filesize

# Notes about load operation
# If I use 0x01000000 I get DMA misaligned error message
# Last two parameters are size and offset.

# Test Case 4a - Read full 1MB of small file
${PREFIX}load host${SUFFIX} $addr ${FPATH}$FILE_SMALL
printenv filesize
# Test Case 4b - Read full 1MB of small file
md5sum $addr \$filesize
setenv filesize

# Test Case 5a - First 1MB of big file
${PREFIX}load host${SUFFIX} $addr ${FPATH}$FILE_BIG $length 0x0
printenv filesize
# Test Case 5b - First 1MB of big file
md5sum $addr \$filesize
setenv filesize

# fails for ext as no offset support
# Test Case 6a - Last 1MB of big file
${PREFIX}load host${SUFFIX} $addr ${FPATH}$FILE_BIG $length 0x9C300000
printenv filesize
# Test Case 6b - Last 1MB of big file
md5sum $addr \$filesize
setenv filesize

# fails for ext as no offset support
# Test Case 7a - One from the last 1MB chunk of 2GB
${PREFIX}load host${SUFFIX} $addr ${FPATH}$FILE_BIG $length 0x7FF00000
printenv filesize
# Test Case 7b - One from the last 1MB chunk of 2GB
md5sum $addr \$filesize
setenv filesize

# fails for ext as no offset support
# Test Case 8a - One from the start 1MB chunk from 2GB
${PREFIX}load host${SUFFIX} $addr ${FPATH}$FILE_BIG $length 0x80000000
printenv filesize
# Test Case 8b - One from the start 1MB chunk from 2GB
md5sum $addr \$filesize
setenv filesize

# fails for ext as no offset support
# Test Case 9a - One 1MB chunk crossing the 2GB boundary
${PREFIX}load host${SUFFIX} $addr ${FPATH}$FILE_BIG $length 0x7FF80000
printenv filesize
# Test Case 9b - One 1MB chunk crossing the 2GB boundary
md5sum $addr \$filesize
setenv filesize

# Generic failure case
# Test Case 10 - 2MB chunk from the last 1MB of big file
${PREFIX}load host${SUFFIX} $addr ${FPATH}$FILE_BIG 0x00200000 0x9C300000
printenv filesize
#

# Read 1MB from small file
${PREFIX}load host${SUFFIX} $addr ${FPATH}$FILE_SMALL
# Write it back to test the writes
# Test Case 11a - Check that the write succeeded
${PREFIX}${WRITE} host${SUFFIX} $addr ${FPATH}$FILE_WRITE \$filesize
mw.b $addr 00 100
${PREFIX}load host${SUFFIX} $addr ${FPATH}$FILE_WRITE
# Test Case 11b - Check md5 of written to is same as the one read from
md5sum $addr \$filesize
setenv filesize
#

# Next test case checks writing a file whose dirent
# is the first in the block, which is always true for "."
# The write should fail, but the lookup should work
# Test Case 12 - Check directory traversal
${PREFIX}${WRITE} host${SUFFIX} $addr ${FPATH}. 0x10

# Read 1MB from small file
${PREFIX}load host${SUFFIX} $addr ${FPATH}$FILE_SMALL
# Write it via "same directory", i.e. "." dirent
# Test Case 13a - Check directory traversal
${PREFIX}${WRITE} host${SUFFIX} $addr ${FPATH}./${FILE_WRITE}2 \$filesize
mw.b $addr 00 100
${PREFIX}load host${SUFFIX} $addr ${FPATH}./${FILE_WRITE}2
# Test Case 13b - Check md5 of written to is same as the one read from
md5sum $addr \$filesize
setenv filesize
mw.b $addr 00 100
${PREFIX}load host${SUFFIX} $addr ${FPATH}${FILE_WRITE}2
# Test Case 13c - Check md5 of written to is same as the one read from
md5sum $addr \$filesize
setenv filesize
#
reset

EOF
}

# 1st argument is the name of the image file.
# 2nd argument is the file where we generate the md5s of the files
# generated with the appropriate start and length that we use to test.
# It creates the necessary files in the image to test.
# $GB2p5 is the path of the big file (2.5 GB)
# $MB1 is the path of the small file (1 MB)
# $MOUNT_DIR is the path we can use to mount the image file.
function create_files() {
	# Mount the image so we can populate it.
	mkdir -p "$MOUNT_DIR"
	sudo mount -o loop,rw "$1" "$MOUNT_DIR"

	# Create a subdirectory.
	sudo mkdir -p "$MOUNT_DIR/SUBDIR"

	# Create big file in this image.
	# Note that we work only on the start 1MB, couple MBs in the 2GB range
	# and the last 1 MB of the huge 2.5GB file.
	# So, just put random values only in those areas.
	if [ ! -f "${GB2p5}" ]; then
		sudo dd if=/dev/urandom of="${GB2p5}" bs=1M count=1 \
			&> /dev/null
		sudo dd if=/dev/urandom of="${GB2p5}" bs=1M count=2 seek=2047 \
			&> /dev/null
		sudo dd if=/dev/urandom of="${GB2p5}" bs=1M count=1 seek=2499 \
			&> /dev/null
	fi

	# Create a small file in this image.
	if [ ! -f "${MB1}" ]; then
		sudo dd if=/dev/urandom of="${MB1}" bs=1M count=1 \
			&> /dev/null
	fi

	# Delete the small file copies which possibly are written as part of a
	# previous test.
	sudo rm -f "${MB1}.w"
	sudo rm -f "${MB1}.w2"

	# Generate the md5sums of reads that we will test against small file
	dd if="${MB1}" bs=1M skip=0 count=1 2> /dev/null | md5sum > "$2"

	# Generate the md5sums of reads that we will test against big file
	# One from beginning of file.
	dd if="${GB2p5}" bs=1M skip=0 count=1 \
		2> /dev/null | md5sum >> "$2"

	# One from end of file.
	dd if="${GB2p5}" bs=1M skip=2499 count=1 \
		2> /dev/null | md5sum >> "$2"

	# One from the last 1MB chunk of 2GB
	dd if="${GB2p5}" bs=1M skip=2047 count=1 \
		2> /dev/null | md5sum >> "$2"

	# One from the start 1MB chunk from 2GB
	dd if="${GB2p5}" bs=1M skip=2048 count=1 \
		2> /dev/null | md5sum >> "$2"

	# One 1MB chunk crossing the 2GB boundary
	dd if="${GB2p5}" bs=512K skip=4095 count=2 \
		2> /dev/null | md5sum >> "$2"

	sync
	sudo umount "$MOUNT_DIR"
	rmdir "$MOUNT_DIR"
}

# 1st parameter is the text to print
# if $? is 0 its a pass, else a fail
# As a side effect it shall update env variable PASS and FAIL
function pass_fail() {
	if [ $? -eq 0 ]; then
		echo pass - "$1"
		PASS=$((PASS + 1))
	else
		echo FAIL - "$1"
		FAIL=$((FAIL + 1))
	fi
}

# 1st parameter is the string which leads to an md5 generation
# 2nd parameter is the file we grep, for that string
# 3rd parameter is the name of the file which has md5s in it
# 4th parameter is the line # in the md5 file that we match it against
# This function checks if the md5 of the file in the sandbox matches
# that calculated while generating the file
# 5th parameter is the string to print with the result
check_md5() {
	# md5sum in u-boot has output of form:
	# md5 for 01000008 ... 01100007 ==> <md5>
	# the 7th field is the actual md5
	md5_src=`grep -A2 "$1" "$2" | grep "md5 for" | tr -d '\r'`
	md5_src=($md5_src)
	md5_src=${md5_src[6]}

	# The md5 list, each line is of the form:
	# - <md5>
	# the 2nd field is the actual md5
	md5_dst=`sed -n $4p $3`
	md5_dst=($md5_dst)
	md5_dst=${md5_dst[0]}

	# For a pass they should match.
	[ "$md5_src" = "$md5_dst" ]
	pass_fail "$5"
}

# 1st parameter is the name of the output file to check
# 2nd parameter is the name of the file containing the md5 expected
# 3rd parameter is the name of the small file
# 4th parameter is the name of the big file
# 5th paramter is the name of the written file
# This function checks the output file for correct results.
function check_results() {
	echo "** Start $1"

	PASS=0
	FAIL=0

	# Check if the ls is showing correct results for 2.5 gb file
	grep -A7 "Test Case 1 " "$1" | egrep -iq "2621440000 *$4"
	pass_fail "TC1: ls of $4"

	# Check if the ls is showing correct results for 1 mb file
	grep -A7 "Test Case 1 " "$1" | egrep -iq "1048576 *$3"
	pass_fail "TC1: ls of $3"

	# Check size command on 1MB.file
	egrep -A3 "Test Case 2a " "$1" | grep -q "filesize=100000"
	pass_fail "TC2: size of $3"
	# Check size command on 1MB.file via a path using '..'
	egrep -A3 "Test Case 2b " "$1" | grep -q "filesize=100000"
	pass_fail "TC2: size of $3 via a path using '..'"

	# Check size command on 2.5GB.file
	egrep -A3 "Test Case 3 " "$1" | grep -q "filesize=9c400000"
	pass_fail "TC3: size of $4"

	# Check read full mb of 1MB.file
	grep -A4 "Test Case 4a " "$1" | grep -q "filesize=100000"
	pass_fail "TC4: load of $3 size"
	check_md5 "Test Case 4b " "$1" "$2" 1 "TC4: load from $3"

	# Check first mb of 2.5GB.file
	grep -A4 "Test Case 5a " "$1" | grep -q "filesize=100000"
	pass_fail "TC5: load of 1st MB from $4 size"
	check_md5 "Test Case 5b " "$1" "$2" 2 "TC5: load of 1st MB from $4"

	# Check last mb of 2.5GB.file
	grep -A4 "Test Case 6a " "$1" | grep -q "filesize=100000"
	pass_fail "TC6: load of last MB from $4 size"
	check_md5 "Test Case 6b " "$1" "$2" 3 "TC6: load of last MB from $4"

	# Check last 1mb chunk of 2gb from 2.5GB file
	grep -A4 "Test Case 7a " "$1" | grep -q "filesize=100000"
	pass_fail "TC7: load of last 1mb chunk of 2GB from $4 size"
	check_md5 "Test Case 7b " "$1" "$2" 4 \
		"TC7: load of last 1mb chunk of 2GB from $4"

	# Check first 1mb chunk after 2gb from 2.5GB file
	grep -A4 "Test Case 8a " "$1" | grep -q "filesize=100000"
	pass_fail "TC8: load 1st MB chunk after 2GB from $4 size"
	check_md5 "Test Case 8b " "$1" "$2" 5 \
		"TC8: load 1st MB chunk after 2GB from $4"

	# Check 1mb chunk crossing the 2gb boundary from 2.5GB file
	grep -A4 "Test Case 9a " "$1" | grep -q "filesize=100000"
	pass_fail "TC9: load 1MB chunk crossing 2GB boundary from $4 size"
	check_md5 "Test Case 9b " "$1" "$2" 6 \
		"TC9: load 1MB chunk crossing 2GB boundary from $4"

	# Check 2mb chunk from the last 1MB of 2.5GB file loads 1MB
	grep -A5 "Test Case 10 " "$1" | grep -q "filesize=100000"
	pass_fail "TC10: load 2MB from the last 1MB of $4 loads 1MB"

	# Check 1mb chunk write
	grep -A2 "Test Case 11a " "$1" | grep -q '1048576 bytes written'
	pass_fail "TC11: 1MB write to $3.w - write succeeded"
	check_md5 "Test Case 11b " "$1" "$2" 1 \
		"TC11: 1MB write to $3.w - content verified"

	# Check lookup of 'dot' directory
	grep -A4 "Test Case 12 " "$1" | grep -q 'Unable to write'
	pass_fail "TC12: 1MB write to . - write denied"

	# Check directory traversal
	grep -A2 "Test Case 13a " "$1" | grep -q '1048576 bytes written'
	pass_fail "TC13: 1MB write to ./$3.w2 - write succeeded"
	check_md5 "Test Case 13b " "$1" "$2" 1 \
		"TC13: 1MB read from ./$3.w2 - content verified"
	check_md5 "Test Case 13c " "$1" "$2" 1 \
		"TC13: 1MB read from $3.w2 - content verified"

	echo "** End $1"
}

# Takes in one parameter which is "fs" or "nonfs", which then dictates
# if a fs test (size/load/save) or a nonfs test (fatread/extread) needs to
# be performed.
function test_fs_nonfs() {
	echo "Creating files in $fs image if not already present."
	create_files $IMAGE $MD5_FILE_FS

	OUT_FILE="${OUT}.$1.${fs}.out"
	test_image $IMAGE $fs $SMALL_FILE $BIG_FILE $1 "" \
		> ${OUT_FILE} 2>&1
	# strip out noise from fs code
	grep -v -e "File System is consistent\|update journal finished" \
		-e "reading .*\.file\|writing .*\.file.w" \
		< ${OUT_FILE} > ${OUT_FILE}_clean
	check_results ${OUT_FILE}_clean $MD5_FILE_FS $SMALL_FILE \
		$BIG_FILE
	TOTAL_FAIL=$((TOTAL_FAIL + FAIL))
	TOTAL_PASS=$((TOTAL_PASS + PASS))
	echo "Summary: PASS: $PASS FAIL: $FAIL"
	echo "--------------------------------------------"
}

# ********************
# * End of functions *
# ********************

check_clean "$1"
check_prereq
compile_sandbox
prepare_env

# Track TOTAL_FAIL and TOTAL_PASS
TOTAL_FAIL=0
TOTAL_PASS=0

# In each loop, for a given file system image, we test both the
# fs command, like load/size/write, the file system specific command
# like: ext4load/ext4size/ext4write and the host load/ls/save commands.
for fs in ext4 fat16 fat32; do

	echo "Creating $fs image if not already present."
	IMAGE=${IMG}.${fs}.img
	MD5_FILE_FS="${MD5_FILE}.${fs}"
	create_image $IMAGE $fs

	# host commands test
	echo "Creating files in $fs image if not already present."
	create_files $IMAGE $MD5_FILE_FS

	# Lets mount the image and test host hostfs commands
	mkdir -p "$MOUNT_DIR"
	case "$fs" in
		fat*)
		uid="uid=`id -u`"
		;;
		*)
		uid=""
		;;
	esac
	sudo mount -o loop,rw,$uid "$IMAGE" "$MOUNT_DIR"
	sudo chmod 777 "$MOUNT_DIR"

	OUT_FILE="${OUT}.sb.${fs}.out"
	test_image $IMAGE $fs $SMALL_FILE $BIG_FILE sb `pwd`/$MOUNT_DIR \
		> ${OUT_FILE} 2>&1
	sudo umount "$MOUNT_DIR"
	rmdir "$MOUNT_DIR"

	check_results $OUT_FILE $MD5_FILE_FS $SMALL_FILE $BIG_FILE
	TOTAL_FAIL=$((TOTAL_FAIL + FAIL))
	TOTAL_PASS=$((TOTAL_PASS + PASS))
	echo "Summary: PASS: $PASS FAIL: $FAIL"
	echo "--------------------------------------------"

	test_fs_nonfs nonfs
	test_fs_nonfs fs
done

echo "Total Summary: TOTAL PASS: $TOTAL_PASS TOTAL FAIL: $TOTAL_FAIL"
echo "--------------------------------------------"
if [ $TOTAL_FAIL -eq 0 ]; then
	echo "PASSED"
	exit 0
else
	echo "FAILED"
	exit 1
fi
