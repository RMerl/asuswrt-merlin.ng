# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2018 Synopsys, Inc. All rights reserved.
# Author: Eugeniy Paltsev <Eugeniy.Paltsev@synopsys.com>

import os, getopt, sys, zlib
from elftools.elf.elffile import ELFFile


def usage(exit_code):
    print("usage:")
    print(sys.argv[0] + " --arc-id 0x52 --image u-boot.bin --elf u-boot")
    sys.exit(exit_code)


def elf_get_entry(filename):
    with open(filename, 'rb') as f:
        elffile = ELFFile(f)
        return elffile.header['e_entry']


def calc_check_sum(filename):
    # u-boot.head check_sum for preloader - it is sum of all u-boot binary bytes
    with open(filename, "rb") as file:
        ba = bytearray(file.read())
    return sum(ba) & 0xFF


def arg_verify(uboot_bin_filename, uboot_elf_filename, arc_id):
    if arc_id not in [0x52, 0x53]:
        print("unknown ARC ID: " + hex(arc_id))
        sys.exit(2)

    if not os.path.isfile(uboot_bin_filename):
        print("uboot bin file not exists: " + uboot_bin_filename)
        sys.exit(2)

    if not os.path.isfile(uboot_elf_filename):
        print("uboot elf file not exists: " + uboot_elf_filename)
        sys.exit(2)


def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:],
            "ha:i:l:e:", ["help", "arc-id=", "image=", "elf="])
    except getopt.GetoptError as err:
        print(err)
        usage(2)

    # default filenames
    uboot_elf_filename  = "u-boot"
    uboot_bin_filename  = "u-boot.bin"
    headerised_filename = "u-boot.head"
    uboot_scrypt_file   = "u-boot-update.txt"

    # initial header values: place where preloader will store u-boot binary,
    # should be equal to CONFIG_SYS_TEXT_BASE
    image_copy_adr  = 0x81000000

    # initial constant header values, do not change these values
    arc_id          = 0x52         # 0x52 for 1st HSDK release (hardcoded in RTL)
    magic1          = 0xdeadbeafaf # big endian byte order
    flash_address   = 0x0
    flash_type      = 0x0          # 0 - SPI flash, 1 - NOR flash
    magic2          = [            # big endian byte order
    0x20202a2020202020202020202a20202020207c5c2e20202020202e2f7c20202020207c2d,
    0x2e5c2020202f2e2d7c20202020205c2020602d2d2d6020202f20202020202f205f202020,
    0x205f20205c20202020207c205f60712070205f207c2020202020272e5f3d2f205c3d5f2e,
    0x272020202020202020605c202f60202020202020202020202020206f2020202020202020]

    for opt, arg in opts:
        if opt in ('-h', "--help"):   usage(0)
        if opt in ('-a', "--arc-id"): arc_id             = int(arg, 16)
        if opt in ('-i', "--image"):  uboot_bin_filename = arg
        if opt in ('-e', "--elf"):    uboot_elf_filename = arg

    arg_verify(uboot_bin_filename, uboot_elf_filename, arc_id)

    uboot_img_size = os.path.getsize(uboot_bin_filename)
    jump_address = elf_get_entry(uboot_elf_filename)
    check_sum = calc_check_sum(uboot_bin_filename)

    # write header to file
    with open(headerised_filename, "wb") as file:
        file.write(arc_id.to_bytes(2, byteorder='little'))
        file.write(uboot_img_size.to_bytes(4, byteorder='little'))
        file.write(check_sum.to_bytes(1, byteorder='little'))
        file.write(image_copy_adr.to_bytes(4, byteorder='little'))
        file.write(magic1.to_bytes(5, byteorder='big'))
        file.write(jump_address.to_bytes(4, byteorder='little'))
        for i in range(12): file.write(0xFF.to_bytes(1, byteorder='little'))
        for byte in magic2: file.write(byte.to_bytes(36, byteorder='big'))
        for i in range(208 - len(magic2) * 36):
            file.write(0xFF.to_bytes(1, byteorder='little'))
        file.write(flash_address.to_bytes(4, byteorder='little'))
        for i in range(11): file.write(0xFF.to_bytes(1, byteorder='little'))
        file.write(flash_type.to_bytes(1, byteorder='little'))

    # append u-boot image to header
    with open(headerised_filename, "ab") as fo:
        with open(uboot_bin_filename,'rb') as fi:
            fo.write(fi.read())

    # calc u-boot headerized image CRC32 (will be used by uboot update
    # command for check)
    headerised_image_crc = ""
    with open(headerised_filename, "rb") as fi:
        headerised_image_crc = hex(zlib.crc32(fi.read()) & 0xffffffff)

    load_addr = 0x81000000
    crc_store_adr = load_addr - 0x8
    crc_calc_adr = crc_store_adr - 0x4
    load_size = os.path.getsize(headerised_filename)
    crc_calc_cmd = \
        "crc32 " + hex(load_addr) + " " + hex(load_size) + " " + hex(crc_calc_adr)
    crc_check_cmd = \
        "mw.l " + hex(crc_store_adr) + " " + headerised_image_crc + " && " + \
        crc_calc_cmd + " && " + \
        "cmp.l " + hex(crc_store_adr) + " " + hex(crc_calc_adr) + " 1"

    # make errase size to be allighned by 64K
    if load_size & 0xFFFF == 0:
        errase_size = load_size
    else:
        errase_size = load_size - (load_size & 0xFFFF) + 0x10000

    # u-bood CMD to load u-bood with header to SPI flash
    sf_load_image_cmd = \
        "fatload mmc 0:1 " + hex(load_addr) + " " + headerised_filename + " && " + \
        "sf probe 0:0 && " + \
        crc_check_cmd + " && " + \
        "sf protect unlock 0x0 " + hex(errase_size) + " && " + \
        "sf erase 0x0 " + hex(errase_size) + " && " + \
        "sf write " + hex(load_addr) + " 0x0 " + hex(load_size) + " && " + \
        "sf protect lock 0x0 " + hex(errase_size)

    update_uboot_cmd = sf_load_image_cmd + " && echo \"u-boot update: OK\""

    with open(uboot_scrypt_file, "wb") as fo:
        fo.write(update_uboot_cmd.encode('ascii'))


if __name__ == "__main__":
    try:
        main()
    except Exception as err:
        print(err)
        sys.exit(2)
