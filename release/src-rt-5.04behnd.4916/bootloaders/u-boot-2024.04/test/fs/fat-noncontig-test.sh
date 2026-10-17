#!/bin/bash
# SPDX-License-Identifier: GPL-2.0+

# (C) Copyright 2015 Stephen Warren

# This script tests U-Boot's FAT filesystem code's ability to read non-
# contiguous files.

# When porting the ff.c FAT parsing code into U-Boot, it was found that ff.c
# always reads files cluster-by-cluster, which results in poor performance.
# This was solved by adding a patch to ff.c to coalesce reads of adjacent
# clusters. Since this patch needed to correctly handle non-contiguous files,
# this test was written to validate that.
#
# To execute the test, simply run it from the U-Boot source root directory:
#
#    cd u-boot
#    ./test/fs/fat-noncontig-test.sh
#
# The test will create a FAT filesystem image, record the CRC of a randomly
# generated file in the image, build U-Boot sandbox, invoke U-Boot sandbox to
# read the file and validate that the CRCs match. Expected output is shown
# below. The important part of the log is the penultimate line that contains
# either "PASS" or "FAILURE".
#
#    mkfs.fat 3.0.26 (2014-03-07)
#
#
#    U-Boot 2015.10-rc4-00018-g4b22a3e5513f (Oct 03 2015 - 13:49:23 -0600)
#
#    DRAM:  128 MiB
#    Using default environment
#
#    In:    serial
#    Out:   lcd
#    Err:   lcd
#    Net:   No ethernet found.
#    => host bind 0 sandbox/fat-noncontig.img
#    => load host 0:0 1000 noncontig.img
#    33584964 bytes read in 18 ms (1.7 GiB/s)
#    => crc32 1000 $filesize 0
#    crc32 for 00001000 ... 02008743 ==> 6a080523
#    => if itest.l *0 != 2305086a; then echo FAILURE; else echo PASS; fi
#    PASS
#    => reset
#
# All temporary files used by this script are created in ./sandbox to avoid
# polluting the source tree. test/fs/fs-test.sh also uses this directory for
# the same purpose.
#
# TODO: Integrate this (and many other corner-cases e.g. different types of
# FAT) with fs-test.sh so that a single script tests everything filesystem-
# related.

odir=sandbox
img=${odir}/fat-noncontig.img
mnt=${odir}/mnt
fill=/dev/urandom
testfn=noncontig.img
mnttestfn=${mnt}/${testfn}
crcaddr=0
loadaddr=1000

for prereq in fallocate mkfs.fat dd crc32; do
    if [ ! -x "`which $prereq`" ]; then
        echo "Missing $prereq binary. Exiting!"
        exit 1
    fi
done

make O=${odir} -s sandbox_defconfig && make O=${odir} -s -j8

mkdir -p ${mnt}
if [ ! -f ${img} ]; then
    fallocate -l 40M ${img}
    if [ $? -ne 0 ]; then
        echo fallocate failed - using dd instead
        dd if=/dev/zero of=${img} bs=1024 count=$((40 * 1024))
        if [ $? -ne 0 ]; then
            echo Could not create empty disk image
            exit $?
        fi
    fi
    mkfs.fat ${img}
    if [ $? -ne 0 ]; then
        echo Could not create FAT filesystem
        exit $?
    fi

    sudo mount -o loop,uid=$(id -u) ${img} ${mnt}
    if [ $? -ne 0 ]; then
        echo Could not mount test filesystem
        exit $?
    fi

    for ((sects=8; sects < 512; sects += 8)); do
        fn=${mnt}/keep-${sects}.img
        dd if=${fill} of=${fn} bs=512 count=${sects} >/dev/null 2>&1
        fn=${mnt}/remove-${sects}.img
        dd if=${fill} of=${fn} bs=512 count=${sects} >/dev/null 2>&1
    done

    rm -f ${mnt}/remove-*.img

    # 511 deliberately to trigger a file size that's not a multiple of the
    # sector size (ignoring sizes that are multiples of both).
    dd if=${fill} of=${mnttestfn} bs=511 >/dev/null 2>&1

    sudo umount ${mnt}
    if [ $? -ne 0 ]; then
        echo Could not unmount test filesystem
        exit $?
    fi
fi

sudo mount -o ro,loop,uid=$(id -u) ${img} ${mnt}
if [ $? -ne 0 ]; then
    echo Could not mount test filesystem
    exit $?
fi
crc=0x`crc32 ${mnttestfn}`
sudo umount ${mnt}
if [ $? -ne 0 ]; then
    echo Could not unmount test filesystem
    exit $?
fi

crc=`printf %02x%02x%02x%02x \
    $((${crc} & 0xff)) \
    $(((${crc} >> 8) & 0xff)) \
    $(((${crc} >> 16) & 0xff)) \
    $((${crc} >> 24))`

./sandbox/u-boot << EOF
host bind 0 ${img}
load host 0:0 ${loadaddr} ${testfn}
crc32 ${loadaddr} \$filesize ${crcaddr}
if itest.l *${crcaddr} != ${crc}; then echo FAILURE; else echo PASS; fi
reset
EOF
if [ $? -ne 0 ]; then
    echo U-Boot exit status indicates an error
    exit $?
fi
