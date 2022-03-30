// SPDX-License-Identifier: GPL-2.0+
/*
 * ddrmc DDR3 calibration code for NXP's VF610
 *
 * Copyright (C) 2018 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 *
 */
/* #define DEBUG */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <linux/bitmap.h>

#include "ddrmc-vf610-calibration.h"

/*
 * Documents:
 *
 * [1] "Vybrid: About DDR leveling feature on DDRMC."
 * https://community.nxp.com/thread/395323
 *
 * [2] VFxxx Controller Reference Manual, Rev. 0, 10/2016
 *
 *
 * NOTE
 * ====
 *
 * NXP recommends setting 'fixed' parameters instead of performing the
 * training at each boot.
 *
 * Use those functions to determine those values on new HW, read the
 * calculated value from registers and add them to the board specific
 * struct ddrmc_cr_setting.
 *
 * SW leveling supported operations - CR93[SW_LVL_MODE]:
 *
 * - 0x0 (b'00) - No leveling
 *
 * - 0x1 (b'01) - WRLVL_DL_X - It is not recommended to perform this tuning
 *                             on HW designs utilizing non-flyback topology
 *                             (Single DDR3 with x16).
 *                             Instead the WRLVL_DL_0/1 fields shall be set
 *                             based on trace length differences from their
 *                             layout.
 *                             Mismatches up to 25% or tCK (clock period) are
 *                             allowed, so the value in the filed doesn’t have
 *                             to be very accurate.
 *
 * - 0x2 (b'10) - RDLVL_DL_0/1 - refers to adjusting the DQS strobe in relation
 *                             to the DQ signals so that the strobe edge is
 *                             centered in the window of valid read data.
 *
 * - 0x3 (b'11) - RDLVL_GTDL_0/1 - refers to the delay the PHY uses to un-gate
 *                             the Read DQS strobe pad from the time that the
 *                             PHY enables the pad to input the strobe signal.
 *
 */
static int ddr_cal_get_first_edge_index(unsigned long *bmap, enum edge e,
					int samples, int start, int max)
{
	int i, ret = -1;

	/*
	 * We look only for the first value (and filter out
	 * some wrong data)
	 */
	switch (e) {
	case RISING_EDGE:
		for (i = start; i <= max - samples; i++) {
			if (test_bit(i, bmap)) {
				if (!test_bit(i - 1, bmap) &&
				    test_bit(i + 1, bmap) &&
				    test_bit(i + 2, bmap) &&
				    test_bit(i + 3, bmap)) {
					return i;
				}
			}
		}
		break;
	case FALLING_EDGE:
		for (i = start; i <= max - samples; i++) {
			if (!test_bit(i, bmap)) {
				if (test_bit(i - 1, bmap) &&
				    test_bit(i - 2, bmap) &&
				    test_bit(i - 3, bmap)) {
					return i;
				}
			}
		}
	}

	return ret;
}

static void bitmap_print(unsigned long *bmap, int max)
{
	int i;

	debug("BITMAP [0x%p]:\n", bmap);
	for (i = 0; i <= max; i++) {
		debug("%d ", test_bit(i, bmap) ? 1 : 0);
		if (i && (i % 32) == (32 - 1))
			debug("\n");
	}
	debug("\n");
}

#define sw_leveling_op_done \
	while (!(readl(&ddrmr->cr[94]) & DDRMC_CR94_SWLVL_OP_DONE))

#define sw_leveling_load_value \
	do { clrsetbits_le32(&ddrmr->cr[93], DDRMC_CR93_SWLVL_LOAD, \
			     DDRMC_CR93_SWLVL_LOAD); } while (0)

#define sw_leveling_start \
	do { clrsetbits_le32(&ddrmr->cr[93], DDRMC_CR93_SWLVL_START, \
			     DDRMC_CR93_SWLVL_START); } while (0)

#define sw_leveling_exit \
	do { clrsetbits_le32(&ddrmr->cr[94], DDRMC_CR94_SWLVL_EXIT, \
			     DDRMC_CR94_SWLVL_EXIT); } while (0)

/*
 * RDLVL_DL calibration:
 *
 * NXP is _NOT_ recommending performing the leveling at each
 * boot. Instead - one shall run this procedure on new boards
 * and then use hardcoded values.
 *
 */
static int ddrmc_cal_dqs_to_dq(struct ddrmr_regs *ddrmr)
{
	DECLARE_BITMAP(rdlvl_rsp, DDRMC_DQS_DQ_MAX_DELAY + 1);
	int rdlvl_dl_0_min = -1, rdlvl_dl_0_max = -1;
	int rdlvl_dl_1_min = -1, rdlvl_dl_1_max = -1;
	int rdlvl_dl_0, rdlvl_dl_1;
	u8 swlvl_rsp;
	u32 tmp;
	int i;

	/* Read defaults */
	u16 rdlvl_dl_0_def =
		(readl(&ddrmr->cr[105]) >> DDRMC_CR105_RDLVL_DL_0_OFF) & 0xFFFF;
	u16 rdlvl_dl_1_def = readl(&ddrmr->cr[110]) & 0xFFFF;

	debug("\nRDLVL: ======================\n");
	debug("RDLVL: DQS to DQ (RDLVL)\n");

	debug("RDLVL: RDLVL_DL_0_DFL:\t 0x%x\n", rdlvl_dl_0_def);
	debug("RDLVL: RDLVL_DL_1_DFL:\t 0x%x\n", rdlvl_dl_1_def);

	/*
	 * Set/Read setup for calibration
	 *
	 * Values necessary for leveling from Vybrid RM [2] - page 1600
	 */
	writel(0x40703030, &ddrmr->cr[144]);
	writel(0x40, &ddrmr->cr[145]);
	writel(0x40, &ddrmr->cr[146]);

	tmp = readl(&ddrmr->cr[144]);
	debug("RDLVL: PHY_RDLVL_RES:\t 0x%x\n", (tmp >> 24) & 0xFF);// set 0x40
	debug("RDLVL: PHY_RDLV_LOAD:\t 0x%x\n", (tmp >> 16) & 0xFF);// set 0x70
	debug("RDLVL: PHY_RDLV_DLL:\t 0x%x\n", (tmp >> 8) & 0xFF); // set 0x30
	debug("RDLVL: PHY_RDLV_EN:\t 0x%x\n", tmp & 0xFF); //set 0x30

	tmp = readl(&ddrmr->cr[145]);
	debug("RDLVL: PHY_RDLV_RR:\t 0x%x\n", tmp & 0x3FF); //set 0x40

	tmp = readl(&ddrmr->cr[146]);
	debug("RDLVL: PHY_RDLV_RESP:\t 0x%x\n", tmp); //set 0x40

	/*
	 * Program/read the leveling edge RDLVL_EDGE = 0
	 *
	 * 0x00 is the correct output on SWLVL_RSP_X
	 * If by any chance 1s are visible -> wrong number read
	 */
	clrbits_le32(&ddrmr->cr[101], DDRMC_CR101_PHY_RDLVL_EDGE);

	tmp = readl(&ddrmr->cr[101]);
	debug("RDLVL: PHY_RDLVL_EDGE:\t 0x%x\n",
	      (tmp >> DDRMC_CR101_PHY_RDLVL_EDGE_OFF) & 0x1); //set 0

	/* Program Leveling mode - CR93[SW_LVL_MODE] to ’b10 */
	clrsetbits_le32(&ddrmr->cr[93], DDRMC_CR93_SW_LVL_MODE(0x3),
			DDRMC_CR93_SW_LVL_MODE(0x2));
	tmp = readl(&ddrmr->cr[93]);
	debug("RDLVL: SW_LVL_MODE:\t 0x%x\n",
	      (tmp >> DDRMC_CR93_SW_LVL_MODE_OFF) & 0x3);

	/* Start procedure - CR93[SWLVL_START] to ’b1 */
	sw_leveling_start;

	/* Poll CR94[SWLVL_OP_DONE] */
	sw_leveling_op_done;

	/*
	 * Program delays for RDLVL_DL_0
	 *
	 * The procedure is to increase the delay values from 0 to 0xFF
	 * and read the response from the DDRMC
	 */
	debug("\nRDLVL: ---> RDLVL_DL_0\n");
	bitmap_zero(rdlvl_rsp, DDRMC_DQS_DQ_MAX_DELAY + 1);

	for (i = 0; i <= DDRMC_DQS_DQ_MAX_DELAY; i++) {
		clrsetbits_le32(&ddrmr->cr[105],
				0xFFFF << DDRMC_CR105_RDLVL_DL_0_OFF,
				i << DDRMC_CR105_RDLVL_DL_0_OFF);

		/* Load values CR93[SWLVL_LOAD] to ’b1 */
		sw_leveling_load_value;

		/* Poll CR94[SWLVL_OP_DONE] */
		sw_leveling_op_done;

		/*
		 * Read Responses - SWLVL_RESP_0
		 *
		 * The 0x00 (correct response when PHY_RDLVL_EDGE = 0)
		 * -> 1 in the bit vector
		 */
		swlvl_rsp = (readl(&ddrmr->cr[94]) >>
			     DDRMC_CR94_SWLVL_RESP_0_OFF) & 0xF;
		if (swlvl_rsp == 0)
			generic_set_bit(i, rdlvl_rsp);
	}

	bitmap_print(rdlvl_rsp, DDRMC_DQS_DQ_MAX_DELAY);

	/*
	 * First test for rising edge 0x0 -> 0x1 in bitmap
	 */
	rdlvl_dl_0_min = ddr_cal_get_first_edge_index(rdlvl_rsp, RISING_EDGE,
						      N_SAMPLES, N_SAMPLES,
						      DDRMC_DQS_DQ_MAX_DELAY);

	/*
	 * Secondly test for falling edge 0x1 -> 0x0 in bitmap
	 */
	rdlvl_dl_0_max = ddr_cal_get_first_edge_index(rdlvl_rsp, FALLING_EDGE,
						      N_SAMPLES, rdlvl_dl_0_min,
						      DDRMC_DQS_DQ_MAX_DELAY);

	debug("RDLVL: DL_0 min: %d [0x%x] DL_0 max: %d [0x%x]\n",
	      rdlvl_dl_0_min, rdlvl_dl_0_min, rdlvl_dl_0_max, rdlvl_dl_0_max);
	rdlvl_dl_0 = (rdlvl_dl_0_max - rdlvl_dl_0_min) / 2;

	if (rdlvl_dl_0_max == -1 || rdlvl_dl_0_min == -1 || rdlvl_dl_0 <= 0) {
		debug("RDLVL: The DQS to DQ delay cannot be found!\n");
		debug("RDLVL: Using default - slice 0: %d!\n", rdlvl_dl_0_def);
		rdlvl_dl_0 = rdlvl_dl_0_def;
	}

	debug("\nRDLVL: ---> RDLVL_DL_1\n");
	bitmap_zero(rdlvl_rsp, DDRMC_DQS_DQ_MAX_DELAY + 1);

	for (i = 0; i <= DDRMC_DQS_DQ_MAX_DELAY; i++) {
		clrsetbits_le32(&ddrmr->cr[110],
				0xFFFF << DDRMC_CR110_RDLVL_DL_1_OFF,
				i << DDRMC_CR110_RDLVL_DL_1_OFF);

		/* Load values CR93[SWLVL_LOAD] to ’b1 */
		sw_leveling_load_value;

		/* Poll CR94[SWLVL_OP_DONE] */
		sw_leveling_op_done;

		/*
		 * Read Responses - SWLVL_RESP_1
		 *
		 * The 0x00 (correct response when PHY_RDLVL_EDGE = 0)
		 * -> 1 in the bit vector
		 */
		swlvl_rsp = (readl(&ddrmr->cr[95]) >>
			     DDRMC_CR95_SWLVL_RESP_1_OFF) & 0xF;
		if (swlvl_rsp == 0)
			generic_set_bit(i, rdlvl_rsp);
	}

	bitmap_print(rdlvl_rsp, DDRMC_DQS_DQ_MAX_DELAY);

	/*
	 * First test for rising edge 0x0 -> 0x1 in bitmap
	 */
	rdlvl_dl_1_min = ddr_cal_get_first_edge_index(rdlvl_rsp, RISING_EDGE,
						      N_SAMPLES, N_SAMPLES,
						      DDRMC_DQS_DQ_MAX_DELAY);

	/*
	 * Secondly test for falling edge 0x1 -> 0x0 in bitmap
	 */
	rdlvl_dl_1_max = ddr_cal_get_first_edge_index(rdlvl_rsp, FALLING_EDGE,
						      N_SAMPLES, rdlvl_dl_1_min,
						      DDRMC_DQS_DQ_MAX_DELAY);

	debug("RDLVL: DL_1 min: %d [0x%x] DL_1 max: %d [0x%x]\n",
	      rdlvl_dl_1_min, rdlvl_dl_1_min, rdlvl_dl_1_max, rdlvl_dl_1_max);
	rdlvl_dl_1 = (rdlvl_dl_1_max - rdlvl_dl_1_min) / 2;

	if (rdlvl_dl_1_max == -1 || rdlvl_dl_1_min == -1 || rdlvl_dl_1 <= 0) {
		debug("RDLVL: The DQS to DQ delay cannot be found!\n");
		debug("RDLVL: Using default - slice 1: %d!\n", rdlvl_dl_1_def);
		rdlvl_dl_1 = rdlvl_dl_1_def;
	}

	debug("RDLVL: CALIBRATED: rdlvl_dl_0: 0x%x\t rdlvl_dl_1: 0x%x\n",
	      rdlvl_dl_0, rdlvl_dl_1);

	/* Write new delay values */
	writel(DDRMC_CR105_RDLVL_DL_0(rdlvl_dl_0), &ddrmr->cr[105]);
	writel(DDRMC_CR110_RDLVL_DL_1(rdlvl_dl_1), &ddrmr->cr[110]);

	sw_leveling_load_value;
	sw_leveling_op_done;

	/* Exit procedure - CR94[SWLVL_EXIT] to ’b1 */
	sw_leveling_exit;

	/* Poll CR94[SWLVL_OP_DONE] */
	sw_leveling_op_done;

	return 0;
}

/*
 * WRLVL_DL calibration:
 *
 * For non-flyback memory architecture - where one have a single DDR3 x16
 * memory - it is NOT necessary to perform "Write Leveling"
 * [3] 'Vybrid DDR3 write leveling' https://community.nxp.com/thread/429362
 *
 */

int ddrmc_calibration(struct ddrmr_regs *ddrmr)
{
	ddrmc_cal_dqs_to_dq(ddrmr);

	return 0;
}
