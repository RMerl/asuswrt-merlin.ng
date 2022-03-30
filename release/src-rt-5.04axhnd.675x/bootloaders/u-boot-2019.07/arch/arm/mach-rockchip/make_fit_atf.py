#!/usr/bin/env python
"""
# SPDX-License-Identifier: GPL-2.0+
#
# A script to generate FIT image source for rockchip boards
# with ARM Trusted Firmware
# and multiple device trees (given on the command line)
#
# usage: $0 <dt_name> [<dt_name> [<dt_name] ...]
"""

import os
import sys
import getopt
import logging

# pip install pyelftools
from elftools.elf.elffile import ELFFile

ELF_SEG_P_TYPE = 'p_type'
ELF_SEG_P_PADDR = 'p_paddr'
ELF_SEG_P_VADDR = 'p_vaddr'
ELF_SEG_P_OFFSET = 'p_offset'
ELF_SEG_P_FILESZ = 'p_filesz'
ELF_SEG_P_MEMSZ = 'p_memsz'

DT_HEADER = """
/*
 * This is a generated file.
 */
/dts-v1/;

/ {
	description = "FIT image for U-Boot with bl31 (TF-A)";
	#address-cells = <1>;

	images {
"""

DT_UBOOT = """
		uboot {
			description = "U-Boot (64-bit)";
			data = /incbin/("u-boot-nodtb.bin");
			type = "standalone";
			os = "U-Boot";
			arch = "arm64";
			compression = "none";
			load = <0x%08x>;
		};

"""

DT_IMAGES_NODE_END = """	};

"""

DT_END = "};"

def append_bl31_node(file, atf_index, phy_addr, elf_entry):
    # Append BL31 DT node to input FIT dts file.
    data = 'bl31_0x%08x.bin' % phy_addr
    file.write('\t\tatf_%d {\n' % atf_index)
    file.write('\t\t\tdescription = \"ARM Trusted Firmware\";\n')
    file.write('\t\t\tdata = /incbin/("%s");\n' % data)
    file.write('\t\t\ttype = "firmware";\n')
    file.write('\t\t\tarch = "arm64";\n')
    file.write('\t\t\tos = "arm-trusted-firmware";\n')
    file.write('\t\t\tcompression = "none";\n')
    file.write('\t\t\tload = <0x%08x>;\n' % phy_addr)
    if atf_index == 1:
        file.write('\t\t\tentry = <0x%08x>;\n' % elf_entry)
    file.write('\t\t};\n')
    file.write('\n')

def append_fdt_node(file, dtbs):
    # Append FDT nodes.
    cnt = 1
    for dtb in dtbs:
        dtname = os.path.basename(dtb)
        file.write('\t\tfdt_%d {\n' % cnt)
        file.write('\t\t\tdescription = "%s";\n' % dtname)
        file.write('\t\t\tdata = /incbin/("%s");\n' % dtb)
        file.write('\t\t\ttype = "flat_dt";\n')
        file.write('\t\t\tcompression = "none";\n')
        file.write('\t\t};\n')
        file.write('\n')
        cnt = cnt + 1

def append_conf_section(file, cnt, dtname, segments):
    file.write('\t\tconfig_%d {\n' % cnt)
    file.write('\t\t\tdescription = "%s";\n' % dtname)
    file.write('\t\t\tfirmware = "atf_1";\n')
    file.write('\t\t\tloadables = "uboot"')
    if segments != 0:
        file.write(',')
    for i in range(1, segments):
        file.write('"atf_%d"' % (i + 1))
        if i != (segments - 1):
            file.write(',')
        else:
            file.write(';\n')
    if segments == 0:
        file.write(';\n')
    file.write('\t\t\tfdt = "fdt_1";\n')
    file.write('\t\t};\n')
    file.write('\n')

def append_conf_node(file, dtbs, segments):
    # Append configeration nodes.
    cnt = 1
    file.write('\tconfigurations {\n')
    file.write('\t\tdefault = "config_1";\n')
    for dtb in dtbs:
        dtname = os.path.basename(dtb)
        append_conf_section(file, cnt, dtname, segments)
        cnt = cnt + 1
    file.write('\t};\n')
    file.write('\n')

def generate_atf_fit_dts_uboot(fit_file, uboot_file_name):
    num_load_seg = 0
    p_paddr = 0xFFFFFFFF
    with open(uboot_file_name, 'rb') as uboot_file:
        uboot = ELFFile(uboot_file)
        for i in range(uboot.num_segments()):
            seg = uboot.get_segment(i)
            if seg.__getitem__(ELF_SEG_P_TYPE) == 'PT_LOAD':
                p_paddr = seg.__getitem__(ELF_SEG_P_PADDR)
                num_load_seg = num_load_seg + 1

    assert (p_paddr != 0xFFFFFFFF and num_load_seg == 1)

    fit_file.write(DT_UBOOT % p_paddr)

def generate_atf_fit_dts_bl31(fit_file, bl31_file_name, dtbs_file_name):
    with open(bl31_file_name, 'rb') as bl31_file:
        bl31 = ELFFile(bl31_file)
        elf_entry = bl31.header['e_entry']
        segments = bl31.num_segments()
        for i in range(segments):
            seg = bl31.get_segment(i)
            if seg.__getitem__(ELF_SEG_P_TYPE) == 'PT_LOAD':
                paddr = seg.__getitem__(ELF_SEG_P_PADDR)
                append_bl31_node(fit_file, i + 1, paddr, elf_entry)
    append_fdt_node(fit_file, dtbs_file_name)
    fit_file.write(DT_IMAGES_NODE_END)
    append_conf_node(fit_file, dtbs_file_name, segments)

def generate_atf_fit_dts(fit_file_name, bl31_file_name, uboot_file_name, dtbs_file_name):
    # Generate FIT script for ATF image.
    if fit_file_name != sys.stdout:
        fit_file = open(fit_file_name, "wb")
    else:
        fit_file = sys.stdout

    fit_file.write(DT_HEADER)
    generate_atf_fit_dts_uboot(fit_file, uboot_file_name)
    generate_atf_fit_dts_bl31(fit_file, bl31_file_name, dtbs_file_name)
    fit_file.write(DT_END)

    if fit_file_name != sys.stdout:
        fit_file.close()

def generate_atf_binary(bl31_file_name):
    with open(bl31_file_name, 'rb') as bl31_file:
        bl31 = ELFFile(bl31_file)

        num = bl31.num_segments()
        for i in range(num):
            seg = bl31.get_segment(i)
            if seg.__getitem__(ELF_SEG_P_TYPE) == 'PT_LOAD':
                paddr = seg.__getitem__(ELF_SEG_P_PADDR)
                file_name = 'bl31_0x%08x.bin' % paddr
                with open(file_name, "wb") as atf:
                    atf.write(seg.data())

def main():
    uboot_elf = "./u-boot"
    fit_its = sys.stdout
    if "BL31" in os.environ:
        bl31_elf=os.getenv("BL31");
    elif os.path.isfile("./bl31.elf"):
        bl31_elf = "./bl31.elf"
    else:
        os.system("echo 'int main(){}' > bl31.c")
        os.system("${CROSS_COMPILE}gcc -c bl31.c -o bl31.elf")
        bl31_elf = "./bl31.elf"
        logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.DEBUG)
        logging.warning(' BL31 file bl31.elf NOT found, resulting binary is non-functional')
        logging.warning(' Please read Building section in doc/README.rockchip')

    opts, args = getopt.getopt(sys.argv[1:], "o:u:b:h")
    for opt, val in opts:
        if opt == "-o":
            fit_its = val
        elif opt == "-u":
            uboot_elf = val
        elif opt == "-b":
            bl31_elf = val
        elif opt == "-h":
            print(__doc__)
            sys.exit(2)

    dtbs = args

    generate_atf_fit_dts(fit_its, bl31_elf, uboot_elf, dtbs)
    generate_atf_binary(bl31_elf)

if __name__ == "__main__":
    main()
