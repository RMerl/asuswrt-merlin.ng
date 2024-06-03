#!/bin/sh
#
# SPDX-License-Identifier:      GPL-2.0+
#
# Copyright (C) 2019 Jagan Teki <jagan@amarulasolutions.com>
#
# Based on the board/sunxi/mksunxi_fit_atf.sh
#
# Script to generate FIT image source for 64-bit puma boards with
# U-Boot proper, ATF, PMU firmware and devicetree.
#
# usage: $0 <dt_name> [<dt_name> [<dt_name] ...]

[ -z "$BL31" ] && BL31="bl31.bin"

if [ ! -f $BL31 ]; then
	echo "WARNING: BL31 file $BL31 NOT found, resulting binary is non-functional" >&2
	echo "Please read Building section in doc/README.rockchip" >&2
	BL31=/dev/null
fi

[ -z "$PMUM0" ] && PMUM0="rk3399m0.bin"

if [ ! -f $PMUM0 ]; then
	echo "WARNING: PMUM0 file $PMUM0 NOT found, resulting binary is non-functional" >&2
	echo "Please read Building section in doc/README.rockchip" >&2
	PMUM0=/dev/null
fi

cat << __HEADER_EOF
/* SPDX-License-Identifier: GPL-2.0+ OR X11 */
/*
 * Copyright (C) 2017 Theobroma Systems Design und Consulting GmbH
 *
 * Minimal dts for a SPL FIT image payload.
 */

/dts-v1/;

/ {
	description = "FIT image with U-Boot proper, ATF bl31, M0 Firmware, DTB";
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
			os = "arm-trusted-firmware";
			compression = "none";
			load = <0x1000>;
			entry = <0x1000>;
		};
		pmu {
		        description = "Cortex-M0 firmware";
			data = /incbin/("$PMUM0");
			type = "pmu-firmware";
			compression = "none";
			load = <0x180000>;
                };
		fdt {
			description = "RK3399-Q7 (Puma) flat device-tree";
			data = /incbin/("u-boot.dtb");
			type = "flat_dt";
			compression = "none";
		};
__HEADER_EOF

cat << __CONF_HEADER_EOF
	};

	configurations {
		default = "conf";
		conf {
			description = "Theobroma Systems RK3399-Q7 (Puma) SoM";
			firmware = "atf";
			loadables = "uboot", "pmu";
			fdt = "fdt";
		};
__CONF_HEADER_EOF

cat << __ITS_EOF
	};
};
__ITS_EOF
