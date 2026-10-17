#!/bin/sh
# SPDX-License-Identifier: GPL-2.0+
#
# script to generate FIT image source for Xilinx ZynqMP boards with
# ARM Trusted Firmware and multiple device trees (given on the command line)
#
# usage: $0 <dt_name> [<dt_name> [<dt_name] ...]

BL33="u-boot-nodtb.bin"
[ -z "$BL31" ] && BL31="bl31.bin"
BL31_ELF="${BL31%.*}.elf"
[ -f ${BL31_ELF} ] && ATF_LOAD_ADDR=`${CROSS_COMPILE}readelf -l "${BL31_ELF}" | \
awk '/Entry point/ { print $3 }'`

[ -z "$ATF_LOAD_ADDR" ] && ATF_LOAD_ADDR="0xfffea000"
ATF_LOAD_ADDR_LOW=`printf 0x%x $((ATF_LOAD_ADDR & 0xffffffff))`
ATF_LOAD_ADDR_HIGH=`printf 0x%x $((ATF_LOAD_ADDR >> 32))`

[ -z "$BL32" ] && BL32="tee.bin"
BL32_ELF="${BL32%.*}.elf"
[ -f ${BL32_ELF} ] && TEE_LOAD_ADDR=`${CROSS_COMPILE}readelf -l "${BL32_ELF}" | \
awk '/Entry point/ { print $3 }'`

[ -z "$TEE_LOAD_ADDR" ] && TEE_LOAD_ADDR="0x60000000"
TEE_LOAD_ADDR_LOW=`printf 0x%x $((TEE_LOAD_ADDR & 0xffffffff))`
TEE_LOAD_ADDR_HIGH=`printf 0x%x $((TEE_LOAD_ADDR >> 32))`

if [ -z "$BL33_LOAD_ADDR" ];then
	BL33_LOAD_ADDR=`awk '/CONFIG_TEXT_BASE/ { print $3 }' include/generated/autoconf.h`
fi
BL33_LOAD_ADDR_LOW=`printf 0x%x $((BL33_LOAD_ADDR & 0xffffffff))`
BL33_LOAD_ADDR_HIGH=`printf 0x%x $((BL33_LOAD_ADDR >> 32))`

DTB_LOAD_ADDR=`awk '/CONFIG_XILINX_OF_BOARD_DTB_ADDR/ { print $3 }' include/generated/autoconf.h`
if [ ! -z "$DTB_LOAD_ADDR" ]; then
	DTB_LOAD_ADDR_LOW=`printf 0x%x $((DTB_LOAD_ADDR & 0xffffffff))`
	DTB_LOAD_ADDR_HIGH=`printf 0x%x $((DTB_LOAD_ADDR >> 32))`
	DTB_LOAD="load = <$DTB_LOAD_ADDR_HIGH $DTB_LOAD_ADDR_LOW>;"
else
	DTB_LOAD=""
fi

if [ -z "$*" ]; then
	DT=arch/arm/dts/${DEVICE_TREE}.dtb
else
	DT=$*
fi

if [ ! -f $BL31 ]; then
	echo "WARNING: BL31 file $BL31 NOT found, U-Boot will run in EL3" >&2
	BL31=/dev/null
fi

cat << __HEADER_EOF
// SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause)

/dts-v1/;

/ {
	description = "Configuration for Xilinx ZynqMP SoC";

	images {
		uboot {
			description = "U-Boot (64-bit)";
			data = /incbin/("$BL33");
			type = "firmware";
			os = "u-boot";
			arch = "arm64";
			compression = "none";
			load = <$BL33_LOAD_ADDR_HIGH $BL33_LOAD_ADDR_LOW>;
			entry = <$BL33_LOAD_ADDR_HIGH $BL33_LOAD_ADDR_LOW>;
			hash {
				algo = "md5";
			};
		};
__HEADER_EOF

if [ -f $BL31 ]; then
cat << __ATF
		atf {
			description = "Trusted Firmware-A";
			data = /incbin/("$BL31");
			type = "firmware";
			os = "arm-trusted-firmware";
			arch = "arm64";
			compression = "none";
			load = <$ATF_LOAD_ADDR_HIGH $ATF_LOAD_ADDR_LOW>;
			entry = <$ATF_LOAD_ADDR_HIGH $ATF_LOAD_ADDR_LOW>;
			hash {
				algo = "md5";
			};
		};
__ATF
fi

if [ -f $BL32 ]; then
cat << __TEE
		tee {
			description = "TEE firmware";
			data = /incbin/("$BL32");
			type = "firmware";
			os = "tee";
			arch = "arm64";
			compression = "none";
			load = <$TEE_LOAD_ADDR_HIGH $TEE_LOAD_ADDR_LOW>;
			entry = <$TEE_LOAD_ADDR_HIGH $TEE_LOAD_ADDR_LOW>;
			hash {
				algo = "md5";
			};
		};
__TEE
fi

MULTI_DTB=`awk '/CONFIG_MULTI_DTB_FIT / { print $3 }' include/generated/autoconf.h`

if [ 1"$MULTI_DTB" -eq 11 ]; then
	cat << __FDT_IMAGE_EOF
		fdt_1 {
			description = "Multi DTB fit image";
			data = /incbin/("fit-dtb.blob");
			type = "flat_dt";
			arch = "arm64";
			compression = "none";
			$DTB_LOAD
			hash {
				algo = "md5";
			};
		};
	};
	configurations {
		default = "config_1";
__FDT_IMAGE_EOF

if [ ! -f $BL31 ]; then
cat << __CONF_SECTION1_EOF
		config_1 {
			description = "Multi DTB without TF-A";
			firmware = "uboot";
			loadables = "fdt_1";
		};
__CONF_SECTION1_EOF
else
if [ -f $BL32 ]; then
cat << __CONF_SECTION1_EOF
		config_1 {
			description = "Multi DTB with TF-A and TEE";
			firmware = "atf";
			loadables = "uboot", "tee", "fdt_1";
		};
__CONF_SECTION1_EOF
else
cat << __CONF_SECTION1_EOF
		config_1 {
			description = "Multi DTB with TF-A";
			firmware = "atf";
			loadables = "uboot", "fdt_1";
		};
__CONF_SECTION1_EOF
fi
fi

cat << __ITS_EOF
	};
};
__ITS_EOF

else

DEFAULT=1
cnt=1
for dtname in $DT
do
	cat << __FDT_IMAGE_EOF
		fdt_$cnt {
			description = "$(basename $dtname .dtb)";
			data = /incbin/("$dtname");
			type = "flat_dt";
			arch = "arm64";
			compression = "none";
			$DTB_LOAD
			hash {
				algo = "md5";
			};
		};
__FDT_IMAGE_EOF

[ "x$(basename $dtname .dtb)" = "x${DEVICE_TREE}" ] && DEFAULT=$cnt

cnt=$((cnt+1))
done

cat << __CONF_HEADER_EOF
	};
	configurations {
		default = "config_$DEFAULT";

__CONF_HEADER_EOF

cnt=1
for dtname in $DT
do
if [ ! -f $BL31 ]; then
cat << __CONF_SECTION1_EOF
		config_$cnt {
			description = "$(basename $dtname .dtb)";
			firmware = "uboot";
			fdt = "fdt_$cnt";
		};
__CONF_SECTION1_EOF
else
if [ -f $BL32 ]; then
cat << __CONF_SECTION1_EOF
		config_$cnt {
			description = "$(basename $dtname .dtb)";
			firmware = "atf";
			loadables = "uboot", "tee";
			fdt = "fdt_$cnt";
		};
__CONF_SECTION1_EOF
else
cat << __CONF_SECTION1_EOF
		config_$cnt {
			description = "$(basename $dtname .dtb)";
			firmware = "atf";
			loadables = "uboot";
			fdt = "fdt_$cnt";
		};
__CONF_SECTION1_EOF
fi
fi

cnt=$((cnt+1))
done

cat << __ITS_EOF
	};
};
__ITS_EOF

fi
