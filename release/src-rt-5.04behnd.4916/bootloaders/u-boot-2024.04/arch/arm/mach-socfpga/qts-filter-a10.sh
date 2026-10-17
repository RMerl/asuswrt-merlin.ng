#!/bin/bash

#
# helper function to convert from DOS to Unix, if necessary, and handle
# lines ending in '\'.
#
fix_newlines_in_macros() {
	sed -n ':next;s/\r$//;/[^\\]\\$/ {N;s/\\\n//;b next};p' $1
}

#filter out only what we need from a10 hps.xml
grep_a10_hps_config() {
	grep -E "clk_hz|i_clk_mgr|i_io48_pin_mux|AXI_SLAVE|AXI_MASTER"
}

#
# Process hps.xml
# $1:	hps.xml
# $2:	Output File
#
process_a10_hps_config() {
	hps_xml="$1"
	outfile="$2"

	(cat << EOF
// SPDX-License-Identifier: BSD-3-Clause
/*
 * Intel Arria 10 SoCFPGA configuration
 */

#ifndef __SOCFPGA_ARRIA10_CONFIG_H__
#define __SOCFPGA_ARRIA10_CONFIG_H__

EOF

	echo "/* Clocks */"
	fix_newlines_in_macros \
		${hps_xml} | grep "clk_hz" |
			awk -F"'" '{ gsub("\\.","_",$2) ; \
				print "#define" " " toupper($2) " " $4}' |
			sed 's/\.[0-9]//' |
			sed 's/I_CLK_MGR_//' |
			sort
	fix_newlines_in_macros \
		${hps_xml} | grep "i_clk_mgr_mainpll" |
			awk -F"'" '{ gsub("\\.","_",$2) ; \
				print "#define" " " toupper($2) " " $4}' |
			sed 's/\.[0-9]//' |
			sed 's/I_CLK_MGR_//' |
			sort
	fix_newlines_in_macros \
		${hps_xml} | grep "i_clk_mgr_perpll" |
			awk -F"'" '{ gsub("\\.","_",$2) ; \
				print "#define" " " toupper($2) " " $4}' |
			sed 's/\.[0-9]//' |
			sed 's/I_CLK_MGR_//' |
			sort
	fix_newlines_in_macros \
		${hps_xml} | grep "i_clk_mgr_clkmgr" |
			awk -F"'" '{ gsub("\\.","_",$2) ; \
				print "#define" " " toupper($2) " " $4}' |
			sed 's/\.[0-9]//' |
			sed 's/I_CLK_MGR_//' |
			sort
	fix_newlines_in_macros \
		${hps_xml} | grep "i_clk_mgr_alteragrp" |
			awk -F"'" '{ gsub("\\.","_",$2) ; \
				print "#define" " " toupper($2) " " $4}' |
			sed 's/\.[0-9]//' |
			sed 's/I_CLK_MGR_//' |
			sort
	echo "#define ALTERAGRP_MPUCLK ((ALTERAGRP_MPUCLK_PERICNT << 16) | \\"
	echo "	(ALTERAGRP_MPUCLK_MAINCNT))"
	echo "#define ALTERAGRP_NOCCLK ((ALTERAGRP_NOCCLK_PERICNT << 16) | \\"
	echo "	(ALTERAGRP_NOCCLK_MAINCNT))"

	echo
	echo "/* Pin Mux Configuration */"
	fix_newlines_in_macros \
		${hps_xml} | grep "i_io48_pin_mux" |
			awk -F"'" '{ gsub("\\.","_",$2) ; \
				print "#define" " " toupper($2) " " $4}' |
			sed 's/I_IO48_PIN_MUX_//' |
			sed 's/SHARED_3V_IO_GRP_//' |
			sed 's/FPGA_INTERFACE_GRP_//' |
			sed 's/DEDICATED_IO_GRP_//' |
			sed 's/CONFIGURATION_DEDICATED/CONFIG/' |
			sort

	echo
	echo "/* Bridge Configuration */"
	fix_newlines_in_macros \
		${hps_xml} | grep -E "AXI_SLAVE|AXI_MASTER" |
			awk -F"'" '{ gsub("\\.","_",$2) ; \
				print "#define" " " toupper($2) " " $4}' |
			sed 's/true/1/' |
			sed 's/false/0/' |
			sort

	echo
	echo "/* Voltage Select for Config IO */"
	echo "#define CONFIG_IO_BANK_VSEL \\"
	echo "	(((CONFIG_IO_BANK_VOLTAGE_SEL_CLKRST_IO & 0x3) << 8) | \\"
	echo "	(CONFIG_IO_BANK_VOLTAGE_SEL_PERI_IO & 0x3))"

	echo
	echo "/* Macro for Config IO bit mapping */"
	echo -n "#define CONFIG_IO_MACRO(NAME) "
	echo "(((NAME ## _RTRIM & 0xff) << 19) | \\"
	echo "	((NAME ## _INPUT_BUF_EN & 0x3) << 17) | \\"
	echo "	((NAME ## _WK_PU_EN & 0x1) << 16) | \\"
	echo "	((NAME ## _PU_SLW_RT & 0x1) << 13) | \\"
	echo "	((NAME ## _PU_DRV_STRG & 0xf) << 8) | \\"
	echo "	((NAME ## _PD_SLW_RT & 0x1) << 5) | \\"
	echo "	(NAME ## _PD_DRV_STRG & 0x1f))"

	cat << EOF

#endif /* __SOCFPGA_ARRIA10_CONFIG_H__ */
EOF
	) > "${outfile}"
}

usage() {
	echo "$0 [hps_xml] [output_file]"
	echo "Process QTS-generated hps.xml into devicetree header."
	echo ""
	echo "  hps_xml      - hps.xml file from hps_isw_handoff"
	echo "  output_file  - Output header file for dtsi include"
	echo ""
}

hps_xml="$1"
outfile="$2"

if [ "$#" -ne 2 ] ; then
	usage
	exit 1
fi

process_a10_hps_config "${hps_xml}" "${outfile}"
