#!/bin/sh

#
# helper function to convert from DOS to Unix, if necessary, and handle
# lines ending in '\'.
#
fix_newlines_in_macros() {
	sed -n ':next;s/\r$//;/[^\\]\\$/ {N;s/\\\n//;b next};p' $1
}

#
# Process iocsr_config_*.[ch]
# $1:	SoC type
# $2:	Input handoff directory
# $3:	Input BSP Generated directory
# $4:	Output directory
#
process_iocsr_config() {
	soc="$1"
	in_qts_dir="$2"
	in_bsp_dir="$3"
	out_dir="$4"

	(
	cat << EOF
/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Altera SoCFPGA IOCSR configuration
 */

#ifndef __SOCFPGA_IOCSR_CONFIG_H__
#define __SOCFPGA_IOCSR_CONFIG_H__

EOF

	# Retrieve the scan chain lengths
	fix_newlines_in_macros \
		${in_bsp_dir}/generated/iocsr_config_${soc}.h |
	grep 'CONFIG_HPS_IOCSR_SCANCHAIN[0-9]\+_LENGTH'	| tr -d "()"

	echo ""

	# Retrieve the scan chain config and zap the ad-hoc length encoding
	fix_newlines_in_macros \
		${in_bsp_dir}/generated/iocsr_config_${soc}.c |
	sed -n '/^const/ !b; :next {/^const/ s/(.*)//;p;n;b next}'

	cat << EOF

#endif /* __SOCFPGA_IOCSR_CONFIG_H__ */
EOF
	) > "${out_dir}/iocsr_config.h"
}

#
# Process pinmux_config_*.c (and ignore pinmux_config.h)
# $1:	SoC type
# $2:	Input directory
# $3:	Output directory
#
process_pinmux_config() {
	soc="$1"
	in_qts_dir="$2"
	in_bsp_dir="$3"
	out_dir="$4"

	(
	cat << EOF
/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Altera SoCFPGA PinMux configuration
 */

#ifndef __SOCFPGA_PINMUX_CONFIG_H__
#define __SOCFPGA_PINMUX_CONFIG_H__

EOF

	# Retrieve the pinmux config and zap the ad-hoc length encoding
	fix_newlines_in_macros \
		${in_bsp_dir}/generated/pinmux_config_${soc}.c |
	sed -n '/^unsigned/ !b; :next {/^unsigned/ {s/\[.*\]/[]/;s/unsigned long/const u8/};p;n;b next}'

	cat << EOF

#endif /* __SOCFPGA_PINMUX_CONFIG_H__ */
EOF
	) > "${out_dir}/pinmux_config.h"
}

#
# Process pll_config.h
# $1:	SoC type (not used)
# $2:	Input directory
# $3:	Output directory
#
process_pll_config() {
	soc="$1"
	in_qts_dir="$2"
	in_bsp_dir="$3"
	out_dir="$4"

	(
	cat << EOF
/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Altera SoCFPGA Clock and PLL configuration
 */

#ifndef __SOCFPGA_PLL_CONFIG_H__
#define __SOCFPGA_PLL_CONFIG_H__

EOF

	# Retrieve the pll config and zap parenthesis
	fix_newlines_in_macros \
		${in_bsp_dir}/generated/pll_config.h |
	sed -n '/CONFIG_HPS/ !b; :next {/CONFIG_HPS/ s/[()]//g;/endif/ b;p;n;b next}'

	cat << EOF

#endif /* __SOCFPGA_PLL_CONFIG_H__ */
EOF
	) > "${out_dir}/pll_config.h"
}

#
# Filter out only the macros which are actually used by the code
#
grep_sdram_config() {
	egrep "#define (CONFIG_HPS_SDR_CTRLCFG_CTRLCFG_MEMTYPE|CONFIG_HPS_SDR_CTRLCFG_CTRLCFG_MEMBL|CONFIG_HPS_SDR_CTRLCFG_CTRLCFG_ADDRORDER|CONFIG_HPS_SDR_CTRLCFG_CTRLCFG_ECCEN|CONFIG_HPS_SDR_CTRLCFG_CTRLCFG_ECCCORREN|CONFIG_HPS_SDR_CTRLCFG_CTRLCFG_REORDEREN|CONFIG_HPS_SDR_CTRLCFG_CTRLCFG_STARVELIMIT|CONFIG_HPS_SDR_CTRLCFG_CTRLCFG_DQSTRKEN|CONFIG_HPS_SDR_CTRLCFG_CTRLCFG_NODMPINS|CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING1_TCWL|CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING1_AL|CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING1_TCL|CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING1_TRRD|CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING1_TFAW|CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING1_TRFC|CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING2_IF_TREFI|CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING2_IF_TRCD|CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING2_IF_TRP|CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING2_IF_TWR|CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING2_IF_TWTR|CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING3_TRTP|CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING3_TRAS|CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING3_TRC|CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING3_TMRD|CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING3_TCCD|CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING4_SELFRFSHEXIT|CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING4_PWRDOWNEXIT|CONFIG_HPS_SDR_CTRLCFG_LOWPWRTIMING_AUTOPDCYCLES|CONFIG_HPS_SDR_CTRLCFG_LOWPWRTIMING_CLKDISABLECYCLES|CONFIG_HPS_SDR_CTRLCFG_DRAMODT_READ|CONFIG_HPS_SDR_CTRLCFG_DRAMODT_WRITE|CONFIG_HPS_SDR_CTRLCFG_DRAMADDRW_COLBITS|CONFIG_HPS_SDR_CTRLCFG_DRAMADDRW_ROWBITS|CONFIG_HPS_SDR_CTRLCFG_DRAMADDRW_BANKBITS|CONFIG_HPS_SDR_CTRLCFG_DRAMADDRW_CSBITS|CONFIG_HPS_SDR_CTRLCFG_DRAMIFWIDTH_IFWIDTH|CONFIG_HPS_SDR_CTRLCFG_DRAMDEVWIDTH_DEVWIDTH|CONFIG_HPS_SDR_CTRLCFG_DRAMINTR_INTREN|CONFIG_HPS_SDR_CTRLCFG_LOWPWREQ_SELFRFSHMASK|CONFIG_HPS_SDR_CTRLCFG_STATICCFG_MEMBL|CONFIG_HPS_SDR_CTRLCFG_STATICCFG_USEECCASDATA|CONFIG_HPS_SDR_CTRLCFG_CTRLWIDTH_CTRLWIDTH|CONFIG_HPS_SDR_CTRLCFG_CPORTWIDTH_CPORTWIDTH|CONFIG_HPS_SDR_CTRLCFG_CPORTWMAP_CPORTWMAP|CONFIG_HPS_SDR_CTRLCFG_CPORTRMAP_CPORTRMAP|CONFIG_HPS_SDR_CTRLCFG_RFIFOCMAP_RFIFOCMAP|CONFIG_HPS_SDR_CTRLCFG_WFIFOCMAP_WFIFOCMAP|CONFIG_HPS_SDR_CTRLCFG_CPORTRDWR_CPORTRDWR|CONFIG_HPS_SDR_CTRLCFG_PORTCFG_AUTOPCHEN|CONFIG_HPS_SDR_CTRLCFG_FPGAPORTRST|CONFIG_HPS_SDR_CTRLCFG_FIFOCFG_SYNCMODE|CONFIG_HPS_SDR_CTRLCFG_FIFOCFG_INCSYNC|CONFIG_HPS_SDR_CTRLCFG_MPPRIORITY_USERPRIORITY|CONFIG_HPS_SDR_CTRLCFG_MPWIEIGHT_0_STATICWEIGHT_31_0|CONFIG_HPS_SDR_CTRLCFG_MPWIEIGHT_1_STATICWEIGHT_49_32|CONFIG_HPS_SDR_CTRLCFG_MPWIEIGHT_1_SUMOFWEIGHT_13_0|CONFIG_HPS_SDR_CTRLCFG_MPWIEIGHT_2_SUMOFWEIGHT_45_14|CONFIG_HPS_SDR_CTRLCFG_MPWIEIGHT_3_SUMOFWEIGHT_63_46|CONFIG_HPS_SDR_CTRLCFG_PHYCTRL_PHYCTRL_0|CONFIG_HPS_SDR_CTRLCFG_MPPACING_0_THRESHOLD1_31_0|CONFIG_HPS_SDR_CTRLCFG_MPPACING_1_THRESHOLD1_59_32|CONFIG_HPS_SDR_CTRLCFG_MPPACING_1_THRESHOLD2_3_0|CONFIG_HPS_SDR_CTRLCFG_MPPACING_2_THRESHOLD2_35_4|CONFIG_HPS_SDR_CTRLCFG_MPPACING_3_THRESHOLD2_59_36|CONFIG_HPS_SDR_CTRLCFG_MPTHRESHOLDRST_0_THRESHOLDRSTCYCLES_31_0|CONFIG_HPS_SDR_CTRLCFG_MPTHRESHOLDRST_1_THRESHOLDRSTCYCLES_63_32|CONFIG_HPS_SDR_CTRLCFG_MPTHRESHOLDRST_2_THRESHOLDRSTCYCLES_79_64|RW_MGR_ACTIVATE_0_AND_1|RW_MGR_ACTIVATE_0_AND_1_WAIT1|RW_MGR_ACTIVATE_0_AND_1_WAIT2|RW_MGR_ACTIVATE_1|RW_MGR_CLEAR_DQS_ENABLE|RW_MGR_GUARANTEED_READ|RW_MGR_GUARANTEED_READ_CONT|RW_MGR_GUARANTEED_WRITE|RW_MGR_GUARANTEED_WRITE_WAIT0|RW_MGR_GUARANTEED_WRITE_WAIT1|RW_MGR_GUARANTEED_WRITE_WAIT2|RW_MGR_GUARANTEED_WRITE_WAIT3|RW_MGR_IDLE|RW_MGR_IDLE_LOOP1|RW_MGR_IDLE_LOOP2|RW_MGR_INIT_RESET_0_CKE_0|RW_MGR_INIT_RESET_1_CKE_0|RW_MGR_LFSR_WR_RD_BANK_0|RW_MGR_LFSR_WR_RD_BANK_0_DATA|RW_MGR_LFSR_WR_RD_BANK_0_DQS|RW_MGR_LFSR_WR_RD_BANK_0_NOP|RW_MGR_LFSR_WR_RD_BANK_0_WAIT|RW_MGR_LFSR_WR_RD_BANK_0_WL_1|RW_MGR_LFSR_WR_RD_DM_BANK_0|RW_MGR_LFSR_WR_RD_DM_BANK_0_DATA|RW_MGR_LFSR_WR_RD_DM_BANK_0_DQS|RW_MGR_LFSR_WR_RD_DM_BANK_0_NOP|RW_MGR_LFSR_WR_RD_DM_BANK_0_WAIT|RW_MGR_LFSR_WR_RD_DM_BANK_0_WL_1|RW_MGR_MRS0_DLL_RESET|RW_MGR_MRS0_DLL_RESET_MIRR|RW_MGR_MRS0_USER|RW_MGR_MRS0_USER_MIRR|RW_MGR_MRS1|RW_MGR_MRS1_MIRR|RW_MGR_MRS2|RW_MGR_MRS2_MIRR|RW_MGR_MRS3|RW_MGR_MRS3_MIRR|RW_MGR_PRECHARGE_ALL|RW_MGR_READ_B2B|RW_MGR_READ_B2B_WAIT1|RW_MGR_READ_B2B_WAIT2|RW_MGR_REFRESH_ALL|RW_MGR_RETURN|RW_MGR_SGLE_READ|RW_MGR_ZQCL|RW_MGR_TRUE_MEM_DATA_MASK_WIDTH|RW_MGR_MEM_ADDRESS_MIRRORING|RW_MGR_MEM_DATA_MASK_WIDTH|RW_MGR_MEM_DATA_WIDTH|RW_MGR_MEM_DQ_PER_READ_DQS|RW_MGR_MEM_DQ_PER_WRITE_DQS|RW_MGR_MEM_IF_READ_DQS_WIDTH|RW_MGR_MEM_IF_WRITE_DQS_WIDTH|RW_MGR_MEM_NUMBER_OF_CS_PER_DIMM|RW_MGR_MEM_NUMBER_OF_RANKS|RW_MGR_MEM_VIRTUAL_GROUPS_PER_READ_DQS|RW_MGR_MEM_VIRTUAL_GROUPS_PER_WRITE_DQS|IO_DELAY_PER_DCHAIN_TAP|IO_DELAY_PER_DQS_EN_DCHAIN_TAP|IO_DELAY_PER_OPA_TAP|IO_DLL_CHAIN_LENGTH|IO_DQDQS_OUT_PHASE_MAX|IO_DQS_EN_DELAY_MAX|IO_DQS_EN_DELAY_OFFSET|IO_DQS_EN_PHASE_MAX|IO_DQS_IN_DELAY_MAX|IO_DQS_IN_RESERVE|IO_DQS_OUT_RESERVE|IO_IO_IN_DELAY_MAX|IO_IO_OUT1_DELAY_MAX|IO_IO_OUT2_DELAY_MAX|IO_SHIFT_DQS_EN_WHEN_SHIFT_DQS|AFI_RATE_RATIO|CALIB_LFIFO_OFFSET|CALIB_VFIFO_OFFSET|ENABLE_SUPER_QUICK_CALIBRATION|MAX_LATENCY_COUNT_WIDTH|READ_VALID_FIFO_SIZE|REG_FILE_INIT_SEQ_SIGNATURE|TINIT_CNTR0_VAL|TINIT_CNTR1_VAL|TINIT_CNTR2_VAL|TRESET_CNTR0_VAL|TRESET_CNTR1_VAL|TRESET_CNTR2_VAL|CONFIG_HPS_SDR_CTRLCFG_EXTRATIME1_CFG_EXTRA_CTL_CLK_RD_TO_WR|CONFIG_HPS_SDR_CTRLCFG_EXTRATIME1_CFG_EXTRA_CTL_CLK_RD_TO_WR_BC|CONFIG_HPS_SDR_CTRLCFG_EXTRATIME1_CFG_EXTRA_CTL_CLK_RD_TO_WR_DIFF_CHIP)[[:space:]]"
}

#
# Process sdram_config.h, sequencer_auto*h and sequencer_defines.h
# $1:	SoC type (not used)
# $2:	Input directory
# $3:	Output directory
#
process_sdram_config() {
	soc="$1"
	in_qts_dir="$2"
	in_bsp_dir="$3"
	out_dir="$4"

	(
	cat << EOF
/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Altera SoCFPGA SDRAM configuration
 *
 */

#ifndef __SOCFPGA_SDRAM_CONFIG_H__
#define __SOCFPGA_SDRAM_CONFIG_H__

EOF

	echo "/* SDRAM configuration */"
	# Retrieve the sdram config, zap broken lines and zap parenthesis
	fix_newlines_in_macros \
		${in_bsp_dir}/generated/sdram/sdram_config.h |
	sed -n "/\\\\$/ {N;s/ \\\\\n/\t/};p" |
	sed -n '/CONFIG_HPS/ !b; :next {/CONFIG_HPS/ s/[()]//g;/endif/ b;p;n;b next}' |
		sort -u | grep_sdram_config

	echo ""
	echo "/* Sequencer auto configuration */"
	fix_newlines_in_macros \
		${in_qts_dir}/hps_isw_handoff/*/sequencer_auto.h |
	sed -n "/__RW_MGR/ {s/__//;s/ \+\([^ ]\+\)$/\t\1/p}" |
		sort -u | grep_sdram_config

	echo ""
	echo "/* Sequencer defines configuration */"
	fix_newlines_in_macros \
		${in_qts_dir}/hps_isw_handoff/*/sequencer_defines.h |
	sed -n "/^#define [^_]/ {s/__//;s/ \+\([^ ]\+\)$/\t\1/p}" |
		sort -u | grep_sdram_config

	echo ""
	echo "/* Sequencer ac_rom_init configuration */"
	fix_newlines_in_macros \
		${in_qts_dir}/hps_isw_handoff/*/sequencer_auto_ac_init.c |
	sed -n '/^const.*\[/ !b; :next {/^const.*\[/ {N;s/\n//;s/alt_u32/u32/;s/\[.*\]/[]/};/endif/ b;p;n;b next}'

	echo ""
	echo "/* Sequencer inst_rom_init configuration */"
	fix_newlines_in_macros \
		${in_qts_dir}/hps_isw_handoff/*/sequencer_auto_inst_init.c |
	sed -n '/^const.*\[/ !b; :next {/^const.*\[/ {N;s/\n//;s/alt_u32/u32/;s/\[.*\]/[]/};/endif/ b;p;n;b next}'

	cat << EOF

#endif /* __SOCFPGA_SDRAM_CONFIG_H__ */
EOF
	) > "${out_dir}/sdram_config.h"
}

usage() {
	echo "$0 [soc_type] [input_qts_dir] [input_bsp_dir] [output_dir]"
	echo "Process QTS-generated headers into U-Boot compatible ones."
	echo ""
	echo "  soc_type      - Type of SoC, either 'cyclone5' or 'arria5'."
	echo "  input_qts_dir - Directory with compiled Quartus project"
	echo "                  and containing the Quartus project file (QPF)."
	echo "  input_bsp_dir - Directory with generated bsp containing"
	echo "                  the settings.bsp file."
	echo "  output_dir    - Directory to store the U-Boot compatible"
	echo "                  headers."
	echo ""
}

soc="$1"
in_qts_dir="$2"
in_bsp_dir="$3"
out_dir="$4"

if [ "$#" -ne 4 ] ; then
	usage
	exit 1
fi

if [ ! -d "${in_qts_dir}" -o ! -d "${in_bsp_dir}" -o \
	! -d "${out_dir}" -o -z "${soc}" ] ; then
	usage
	exit 3
fi

process_iocsr_config  "${soc}" "${in_qts_dir}" "${in_bsp_dir}" "${out_dir}"
process_pinmux_config "${soc}" "${in_qts_dir}" "${in_bsp_dir}" "${out_dir}"
process_pll_config    "${soc}" "${in_qts_dir}" "${in_bsp_dir}" "${out_dir}"
process_sdram_config  "${soc}" "${in_qts_dir}" "${in_bsp_dir}" "${out_dir}"
