#!/bin/sh
#
# script to generate FIT image source for 64-bit sunxi boards with
# ARM Trusted Firmware and multiple device trees (given on the command line)
#
# usage: $0 <dt_name> [<dt_name> [<dt_name] ...]

[ -z "$BL31" ] && BL31="bl31.bin"

if [ ! -f $BL31 ]; then
	echo "WARNING: BL31 file $BL31 NOT found, resulting binary is non-functional" >&2
	echo "Please read the section on ARM Trusted Firmware (ATF) in board/sunxi/README.sunxi64" >&2
	BL31=/dev/null
fi

if grep -q "^CONFIG_MACH_SUN50I_H6=y" .config; then
	BL31_ADDR=0x104000
else
	BL31_ADDR=0x44000
fi

cat << __HEADER_EOF
/dts-v1/;

/ {
	description = "Configuration to load ATF before U-Boot";
	#address-cells = <1>;

	images {
		uboot {
			description = "U-Boot (64-bit)";
			data = /incbin/("u-boot-nodtb.bin");
			type = "standalone";
			arch = "arm64";
			compression = "none";
			load = <0x4a000000>;
		};
		atf {
			description = "ARM Trusted Firmware";
			data = /incbin/("$BL31");
			type = "firmware";
			arch = "arm64";
			compression = "none";
			load = <$BL31_ADDR>;
			entry = <$BL31_ADDR>;
		};
__HEADER_EOF

cnt=1
for dtname in $*
do
	cat << __FDT_IMAGE_EOF
		fdt_$cnt {
			description = "$(basename $dtname .dtb)";
			data = /incbin/("$dtname");
			type = "flat_dt";
			compression = "none";
		};
__FDT_IMAGE_EOF
	cnt=$((cnt+1))
done

cat << __CONF_HEADER_EOF
	};
	configurations {
		default = "config_1";

__CONF_HEADER_EOF

cnt=1
for dtname in $*
do
	cat << __CONF_SECTION_EOF
		config_$cnt {
			description = "$(basename $dtname .dtb)";
			firmware = "uboot";
			loadables = "atf";
			fdt = "fdt_$cnt";
		};
__CONF_SECTION_EOF
	cnt=$((cnt+1))
done

cat << __ITS_EOF
	};
};
__ITS_EOF
