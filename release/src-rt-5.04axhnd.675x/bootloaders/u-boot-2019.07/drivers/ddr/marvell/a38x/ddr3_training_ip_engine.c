// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include "ddr3_init.h"
#include "mv_ddr_regs.h"
#include "ddr_training_ip_db.h"

#define PATTERN_1	0x55555555
#define PATTERN_2	0xaaaaaaaa

#define VALIDATE_TRAINING_LIMIT(e1, e2)			\
	((((e2) - (e1) + 1) > 33) && ((e1) < 67))

u32 phy_reg_bk[MAX_INTERFACE_NUM][MAX_BUS_NUM][BUS_WIDTH_IN_BITS];

u32 training_res[MAX_INTERFACE_NUM * MAX_BUS_NUM * BUS_WIDTH_IN_BITS *
		 HWS_SEARCH_DIR_LIMIT];
u8 byte_status[MAX_INTERFACE_NUM][MAX_BUS_NUM];	/* holds the bit status in the byte in wrapper function*/

u16 mask_results_dq_reg_map[] = {
	RESULT_CONTROL_PUP_0_BIT_0_REG, RESULT_CONTROL_PUP_0_BIT_1_REG,
	RESULT_CONTROL_PUP_0_BIT_2_REG, RESULT_CONTROL_PUP_0_BIT_3_REG,
	RESULT_CONTROL_PUP_0_BIT_4_REG, RESULT_CONTROL_PUP_0_BIT_5_REG,
	RESULT_CONTROL_PUP_0_BIT_6_REG, RESULT_CONTROL_PUP_0_BIT_7_REG,
	RESULT_CONTROL_PUP_1_BIT_0_REG, RESULT_CONTROL_PUP_1_BIT_1_REG,
	RESULT_CONTROL_PUP_1_BIT_2_REG, RESULT_CONTROL_PUP_1_BIT_3_REG,
	RESULT_CONTROL_PUP_1_BIT_4_REG, RESULT_CONTROL_PUP_1_BIT_5_REG,
	RESULT_CONTROL_PUP_1_BIT_6_REG, RESULT_CONTROL_PUP_1_BIT_7_REG,
	RESULT_CONTROL_PUP_2_BIT_0_REG, RESULT_CONTROL_PUP_2_BIT_1_REG,
	RESULT_CONTROL_PUP_2_BIT_2_REG, RESULT_CONTROL_PUP_2_BIT_3_REG,
	RESULT_CONTROL_PUP_2_BIT_4_REG, RESULT_CONTROL_PUP_2_BIT_5_REG,
	RESULT_CONTROL_PUP_2_BIT_6_REG, RESULT_CONTROL_PUP_2_BIT_7_REG,
	RESULT_CONTROL_PUP_3_BIT_0_REG, RESULT_CONTROL_PUP_3_BIT_1_REG,
	RESULT_CONTROL_PUP_3_BIT_2_REG, RESULT_CONTROL_PUP_3_BIT_3_REG,
	RESULT_CONTROL_PUP_3_BIT_4_REG, RESULT_CONTROL_PUP_3_BIT_5_REG,
	RESULT_CONTROL_PUP_3_BIT_6_REG, RESULT_CONTROL_PUP_3_BIT_7_REG,
	RESULT_CONTROL_PUP_4_BIT_0_REG, RESULT_CONTROL_PUP_4_BIT_1_REG,
	RESULT_CONTROL_PUP_4_BIT_2_REG, RESULT_CONTROL_PUP_4_BIT_3_REG,
	RESULT_CONTROL_PUP_4_BIT_4_REG, RESULT_CONTROL_PUP_4_BIT_5_REG,
	RESULT_CONTROL_PUP_4_BIT_6_REG, RESULT_CONTROL_PUP_4_BIT_7_REG,
#if MAX_BUS_NUM == 9
	RESULT_CONTROL_PUP_5_BIT_0_REG, RESULT_CONTROL_PUP_5_BIT_1_REG,
	RESULT_CONTROL_PUP_5_BIT_2_REG, RESULT_CONTROL_PUP_5_BIT_3_REG,
	RESULT_CONTROL_PUP_5_BIT_4_REG, RESULT_CONTROL_PUP_5_BIT_5_REG,
	RESULT_CONTROL_PUP_5_BIT_6_REG, RESULT_CONTROL_PUP_5_BIT_7_REG,
	RESULT_CONTROL_PUP_6_BIT_0_REG, RESULT_CONTROL_PUP_6_BIT_1_REG,
	RESULT_CONTROL_PUP_6_BIT_2_REG, RESULT_CONTROL_PUP_6_BIT_3_REG,
	RESULT_CONTROL_PUP_6_BIT_4_REG, RESULT_CONTROL_PUP_6_BIT_5_REG,
	RESULT_CONTROL_PUP_6_BIT_6_REG, RESULT_CONTROL_PUP_6_BIT_7_REG,
	RESULT_CONTROL_PUP_7_BIT_0_REG, RESULT_CONTROL_PUP_7_BIT_1_REG,
	RESULT_CONTROL_PUP_7_BIT_2_REG, RESULT_CONTROL_PUP_7_BIT_3_REG,
	RESULT_CONTROL_PUP_7_BIT_4_REG, RESULT_CONTROL_PUP_7_BIT_5_REG,
	RESULT_CONTROL_PUP_7_BIT_6_REG, RESULT_CONTROL_PUP_7_BIT_7_REG,
	RESULT_CONTROL_PUP_8_BIT_0_REG, RESULT_CONTROL_PUP_8_BIT_1_REG,
	RESULT_CONTROL_PUP_8_BIT_2_REG, RESULT_CONTROL_PUP_8_BIT_3_REG,
	RESULT_CONTROL_PUP_8_BIT_4_REG, RESULT_CONTROL_PUP_8_BIT_5_REG,
	RESULT_CONTROL_PUP_8_BIT_6_REG, RESULT_CONTROL_PUP_8_BIT_7_REG,
#endif
	0xffff
};

u16 mask_results_pup_reg_map[] = {
	RESULT_CONTROL_BYTE_PUP_0_REG, RESULT_CONTROL_BYTE_PUP_1_REG,
	RESULT_CONTROL_BYTE_PUP_2_REG, RESULT_CONTROL_BYTE_PUP_3_REG,
	RESULT_CONTROL_BYTE_PUP_4_REG,
#if MAX_BUS_NUM == 9
	RESULT_CONTROL_BYTE_PUP_5_REG, RESULT_CONTROL_BYTE_PUP_6_REG,
	RESULT_CONTROL_BYTE_PUP_7_REG, RESULT_CONTROL_BYTE_PUP_8_REG,
#endif
	0xffff
};

#if MAX_BUS_NUM == 5
u16 mask_results_dq_reg_map_pup3_ecc[] = {
	RESULT_CONTROL_PUP_0_BIT_0_REG, RESULT_CONTROL_PUP_0_BIT_1_REG,
	RESULT_CONTROL_PUP_0_BIT_2_REG, RESULT_CONTROL_PUP_0_BIT_3_REG,
	RESULT_CONTROL_PUP_0_BIT_4_REG, RESULT_CONTROL_PUP_0_BIT_5_REG,
	RESULT_CONTROL_PUP_0_BIT_6_REG, RESULT_CONTROL_PUP_0_BIT_7_REG,
	RESULT_CONTROL_PUP_1_BIT_0_REG, RESULT_CONTROL_PUP_1_BIT_1_REG,
	RESULT_CONTROL_PUP_1_BIT_2_REG, RESULT_CONTROL_PUP_1_BIT_3_REG,
	RESULT_CONTROL_PUP_1_BIT_4_REG, RESULT_CONTROL_PUP_1_BIT_5_REG,
	RESULT_CONTROL_PUP_1_BIT_6_REG, RESULT_CONTROL_PUP_1_BIT_7_REG,
	RESULT_CONTROL_PUP_2_BIT_0_REG, RESULT_CONTROL_PUP_2_BIT_1_REG,
	RESULT_CONTROL_PUP_2_BIT_2_REG, RESULT_CONTROL_PUP_2_BIT_3_REG,
	RESULT_CONTROL_PUP_2_BIT_4_REG, RESULT_CONTROL_PUP_2_BIT_5_REG,
	RESULT_CONTROL_PUP_2_BIT_6_REG, RESULT_CONTROL_PUP_2_BIT_7_REG,
	RESULT_CONTROL_PUP_4_BIT_0_REG, RESULT_CONTROL_PUP_4_BIT_1_REG,
	RESULT_CONTROL_PUP_4_BIT_2_REG, RESULT_CONTROL_PUP_4_BIT_3_REG,
	RESULT_CONTROL_PUP_4_BIT_4_REG, RESULT_CONTROL_PUP_4_BIT_5_REG,
	RESULT_CONTROL_PUP_4_BIT_6_REG, RESULT_CONTROL_PUP_4_BIT_7_REG,
	RESULT_CONTROL_PUP_3_BIT_0_REG, RESULT_CONTROL_PUP_3_BIT_1_REG,
	RESULT_CONTROL_PUP_3_BIT_2_REG, RESULT_CONTROL_PUP_3_BIT_3_REG,
	RESULT_CONTROL_PUP_3_BIT_4_REG, RESULT_CONTROL_PUP_3_BIT_5_REG,
	RESULT_CONTROL_PUP_3_BIT_6_REG, RESULT_CONTROL_PUP_3_BIT_7_REG
};
#endif

#if MAX_BUS_NUM == 5
u16 mask_results_pup_reg_map_pup3_ecc[] = {
	RESULT_CONTROL_BYTE_PUP_0_REG, RESULT_CONTROL_BYTE_PUP_1_REG,
	RESULT_CONTROL_BYTE_PUP_2_REG, RESULT_CONTROL_BYTE_PUP_4_REG,
	RESULT_CONTROL_BYTE_PUP_4_REG
};
#endif

struct pattern_info pattern_table_64[] = {
	/*
	 * num_of_phases_tx, tx_burst_size;
	 * delay_between_bursts, num_of_phases_rx,
	 * start_addr, pattern_len
	 */
	{0x7, 0x7, 2, 0x7, 0x00000, 8},		/* PATTERN_PBS1 */
	{0x7, 0x7, 2, 0x7, 0x00080, 8},		/* PATTERN_PBS2 */
	{0x7, 0x7, 2, 0x7, 0x00100, 8},		/* PATTERN_PBS3 */
	{0x7, 0x7, 2, 0x7, 0x00030, 8},		/* PATTERN_TEST */
	{0x7, 0x7, 2, 0x7, 0x00100, 8},		/* PATTERN_RL */
	{0x7, 0x7, 2, 0x7, 0x00100, 8},		/* PATTERN_RL2 */
	{0x1f, 0xf, 2, 0xf, 0x00680, 32},	/* PATTERN_STATIC_PBS */
	{0x1f, 0xf, 2, 0xf, 0x00a80, 32},	/* PATTERN_KILLER_DQ0 */
	{0x1f, 0xf, 2, 0xf, 0x01280, 32},	/* PATTERN_KILLER_DQ1 */
	{0x1f, 0xf, 2, 0xf, 0x01a80, 32},	/* PATTERN_KILLER_DQ2 */
	{0x1f, 0xf, 2, 0xf, 0x02280, 32},	/* PATTERN_KILLER_DQ3 */
	{0x1f, 0xf, 2, 0xf, 0x02a80, 32},	/* PATTERN_KILLER_DQ4 */
	{0x1f, 0xf, 2, 0xf, 0x03280, 32},	/* PATTERN_KILLER_DQ5 */
	{0x1f, 0xf, 2, 0xf, 0x03a80, 32},	/* PATTERN_KILLER_DQ6 */
	{0x1f, 0xf, 2, 0xf, 0x04280, 32},	/* PATTERN_KILLER_DQ7 */
	{0x1f, 0xf, 2, 0xf, 0x00e80, 32},	/* PATTERN_KILLER_DQ0_64 */
	{0x1f, 0xf, 2, 0xf, 0x01680, 32},	/* PATTERN_KILLER_DQ1_64 */
	{0x1f, 0xf, 2, 0xf, 0x01e80, 32},	/* PATTERN_KILLER_DQ2_64 */
	{0x1f, 0xf, 2, 0xf, 0x02680, 32},	/* PATTERN_KILLER_DQ3_64 */
	{0x1f, 0xf, 2, 0xf, 0x02e80, 32},	/* PATTERN_KILLER_DQ4_64 */
	{0x1f, 0xf, 2, 0xf, 0x03680, 32},	/* PATTERN_KILLER_DQ5_64 */
	{0x1f, 0xf, 2, 0xf, 0x03e80, 32},	/* PATTERN_KILLER_DQ6_64 */
	{0x1f, 0xf, 2, 0xf, 0x04680, 32},	/* PATTERN_KILLER_DQ7_64 */
	{0x1f, 0xf, 2, 0xf, 0x04a80, 32},	/* PATTERN_KILLER_DQ0_INV */
	{0x1f, 0xf, 2, 0xf, 0x05280, 32},	/* PATTERN_KILLER_DQ1_INV */
	{0x1f, 0xf, 2, 0xf, 0x05a80, 32},	/* PATTERN_KILLER_DQ2_INV */
	{0x1f, 0xf, 2, 0xf, 0x06280, 32},	/* PATTERN_KILLER_DQ3_INV */
	{0x1f, 0xf, 2, 0xf, 0x06a80, 32},	/* PATTERN_KILLER_DQ4_INV */
	{0x1f, 0xf, 2, 0xf, 0x07280, 32},	/* PATTERN_KILLER_DQ5_INV */
	{0x1f, 0xf, 2, 0xf, 0x07a80, 32},	/* PATTERN_KILLER_DQ6_INV */
	{0x1f, 0xf, 2, 0xf, 0x08280, 32},	/* PATTERN_KILLER_DQ7_INV */
	{0x1f, 0xf, 2, 0xf, 0x04e80, 32},	/* PATTERN_KILLER_DQ0_INV_64 */
	{0x1f, 0xf, 2, 0xf, 0x05680, 32},	/* PATTERN_KILLER_DQ1_INV_64 */
	{0x1f, 0xf, 2, 0xf, 0x05e80, 32},	/* PATTERN_KILLER_DQ2_INV_64 */
	{0x1f, 0xf, 2, 0xf, 0x06680, 32},	/* PATTERN_KILLER_DQ3_INV_64 */
	{0x1f, 0xf, 2, 0xf, 0x06e80, 32},	/* PATTERN_KILLER_DQ4_INV_64 */
	{0x1f, 0xf, 2, 0xf, 0x07680, 32},	/* PATTERN_KILLER_DQ5_INV_64 */
	{0x1f, 0xf, 2, 0xf, 0x07e80, 32},	/* PATTERN_KILLER_DQ6_INV_64 */
	{0x1f, 0xf, 2, 0xf, 0x08680, 32},	/* PATTERN_KILLER_DQ7_INV_64 */
	{0x1f, 0xf, 2, 0xf, 0x08a80, 32},	/* PATTERN_SSO_FULL_XTALK_DQ0 */
	{0x1f, 0xf, 2, 0xf, 0x09280, 32},	/* PATTERN_SSO_FULL_XTALK_DQ1 */
	{0x1f, 0xf, 2, 0xf, 0x09a80, 32},	/* PATTERN_SSO_FULL_XTALK_DQ2 */
	{0x1f, 0xf, 2, 0xf, 0x0a280, 32},	/* PATTERN_SSO_FULL_XTALK_DQ3 */
	{0x1f, 0xf, 2, 0xf, 0x0aa80, 32},	/* PATTERN_SSO_FULL_XTALK_DQ4 */
	{0x1f, 0xf, 2, 0xf, 0x0b280, 32},	/* PATTERN_SSO_FULL_XTALK_DQ5 */
	{0x1f, 0xf, 2, 0xf, 0x0ba80, 32},	/* PATTERN_SSO_FULL_XTALK_DQ6 */
	{0x1f, 0xf, 2, 0xf, 0x0c280, 32},	/* PATTERN_SSO_FULL_XTALK_DQ7 */
	{0x1f, 0xf, 2, 0xf, 0x08e80, 32},	/* PATTERN_SSO_FULL_XTALK_DQ0_64 */
	{0x1f, 0xf, 2, 0xf, 0x09680, 32},	/* PATTERN_SSO_FULL_XTALK_DQ1_64 */
	{0x1f, 0xf, 2, 0xf, 0x09e80, 32},	/* PATTERN_SSO_FULL_XTALK_DQ2_64 */
	{0x1f, 0xf, 2, 0xf, 0x0a680, 32},	/* PATTERN_SSO_FULL_XTALK_DQ3_64 */
	{0x1f, 0xf, 2, 0xf, 0x0ae80, 32},	/* PATTERN_SSO_FULL_XTALK_DQ4_64 */
	{0x1f, 0xf, 2, 0xf, 0x0b680, 32},	/* PATTERN_SSO_FULL_XTALK_DQ5_64 */
	{0x1f, 0xf, 2, 0xf, 0x0be80, 32},	/* PATTERN_SSO_FULL_XTALK_DQ6_64 */
	{0x1f, 0xf, 2, 0xf, 0x0c680, 32},	/* PATTERN_SSO_FULL_XTALK_DQ7_64 */
	{0x1f, 0xf, 2, 0xf, 0x0ca80, 32},	/* PATTERN_SSO_XTALK_FREE_DQ0 */
	{0x1f, 0xf, 2, 0xf, 0x0d280, 32},	/* PATTERN_SSO_XTALK_FREE_DQ1 */
	{0x1f, 0xf, 2, 0xf, 0x0da80, 32},	/* PATTERN_SSO_XTALK_FREE_DQ2 */
	{0x1f, 0xf, 2, 0xf, 0x0e280, 32},	/* PATTERN_SSO_XTALK_FREE_DQ3 */
	{0x1f, 0xf, 2, 0xf, 0x0ea80, 32},	/* PATTERN_SSO_XTALK_FREE_DQ4 */
	{0x1f, 0xf, 2, 0xf, 0x0f280, 32},	/* PATTERN_SSO_XTALK_FREE_DQ5 */
	{0x1f, 0xf, 2, 0xf, 0x0fa80, 32},	/* PATTERN_SSO_XTALK_FREE_DQ6 */
	{0x1f, 0xf, 2, 0xf, 0x10280, 32},	/* PATTERN_SSO_XTALK_FREE_DQ7 */
	{0x1f, 0xf, 2, 0xf, 0x0ce80, 32},	/* PATTERN_SSO_XTALK_FREE_DQ0_64 */
	{0x1f, 0xf, 2, 0xf, 0x0d680, 32},	/* PATTERN_SSO_XTALK_FREE_DQ1_64 */
	{0x1f, 0xf, 2, 0xf, 0x0de80, 32},	/* PATTERN_SSO_XTALK_FREE_DQ2_64 */
	{0x1f, 0xf, 2, 0xf, 0x0e680, 32},	/* PATTERN_SSO_XTALK_FREE_DQ3_64 */
	{0x1f, 0xf, 2, 0xf, 0x0ee80, 32},	/* PATTERN_SSO_XTALK_FREE_DQ4_64 */
	{0x1f, 0xf, 2, 0xf, 0x0f680, 32},	/* PATTERN_SSO_XTALK_FREE_DQ5_64 */
	{0x1f, 0xf, 2, 0xf, 0x0fe80, 32},	/* PATTERN_SSO_XTALK_FREE_DQ6_64 */
	{0x1f, 0xf, 2, 0xf, 0x10680, 32},	/* PATTERN_SSO_XTALK_FREE_DQ7_64 */
	{0x1f, 0xf, 2, 0xf, 0x10a80, 32},	/* PATTERN_ISI_XTALK_FREE */
	{0x1f, 0xf, 2, 0xf, 0x10e80, 32},	/* PATTERN_ISI_XTALK_FREE_64 */
	{0x1f, 0xf, 2, 0xf, 0x11280, 32},	/* PATTERN_VREF */
	{0x1f, 0xf, 2, 0xf, 0x11680, 32},	/* PATTERN_VREF_64 */
	{0x1f, 0xf, 2, 0xf, 0x11a80, 32},	/* PATTERN_VREF_INV */
	{0x1f, 0xf, 2, 0xf, 0x11e80, 32},	/* PATTERN_FULL_SSO_0T */
	{0x1f, 0xf, 2, 0xf, 0x12280, 32},	/* PATTERN_FULL_SSO_1T */
	{0x1f, 0xf, 2, 0xf, 0x12680, 32},	/* PATTERN_FULL_SSO_2T */
	{0x1f, 0xf, 2, 0xf, 0x12a80, 32},	/* PATTERN_FULL_SSO_3T */
	{0x1f, 0xf, 2, 0xf, 0x12e80, 32},	/* PATTERN_RESONANCE_1T */
	{0x1f, 0xf, 2, 0xf, 0x13280, 32},	/* PATTERN_RESONANCE_2T */
	{0x1f, 0xf, 2, 0xf, 0x13680, 32},	/* PATTERN_RESONANCE_3T */
	{0x1f, 0xf, 2, 0xf, 0x13a80, 32},	/* PATTERN_RESONANCE_4T */
	{0x1f, 0xf, 2, 0xf, 0x13e80, 32},	/* PATTERN_RESONANCE_5T */
	{0x1f, 0xf, 2, 0xf, 0x14280, 32},	/* PATTERN_RESONANCE_6T */
	{0x1f, 0xf, 2, 0xf, 0x14680, 32},	/* PATTERN_RESONANCE_7T */
	{0x1f, 0xf, 2, 0xf, 0x14a80, 32},	/* PATTERN_RESONANCE_8T */
	{0x1f, 0xf, 2, 0xf, 0x14e80, 32},	/* PATTERN_RESONANCE_9T */
	{0x1f, 0xf, 2, 0xf, 0x15280, 32},	/* PATTERN_ZERO */
	{0x1f, 0xf, 2, 0xf, 0x15680, 32}	/* PATTERN_ONE */
	/* Note: actual start_address is "<< 3" of defined address */
};

struct pattern_info pattern_table_16[] = {
	/*
	 * num tx phases, tx burst, delay between, rx pattern,
	 * start_address, pattern_len
	 */
	{1, 1, 2, 1, 0x0080, 2},	/* PATTERN_PBS1 */
	{1, 1, 2, 1, 0x00c0, 2},	/* PATTERN_PBS2 */
	{1, 1, 2, 1, 0x0380, 2},	/* PATTERN_PBS3 */
	{1, 1, 2, 1, 0x0040, 2},	/* PATTERN_TEST */
	{1, 1, 2, 1, 0x0100, 2},	/* PATTERN_RL */
	{1, 1, 2, 1, 0x0000, 2},	/* PATTERN_RL2 */
	{0xf, 0x7, 2, 0x7, 0x0140, 16},	/* PATTERN_STATIC_PBS */
	{0xf, 0x7, 2, 0x7, 0x0190, 16},	/* PATTERN_KILLER_DQ0 */
	{0xf, 0x7, 2, 0x7, 0x01d0, 16},	/* PATTERN_KILLER_DQ1 */
	{0xf, 0x7, 2, 0x7, 0x0210, 16},	/* PATTERN_KILLER_DQ2 */
	{0xf, 0x7, 2, 0x7, 0x0250, 16},	/* PATTERN_KILLER_DQ3 */
	{0xf, 0x7, 2, 0x7, 0x0290, 16},	/* PATTERN_KILLER_DQ4 */
	{0xf, 0x7, 2, 0x7, 0x02d0, 16},	/* PATTERN_KILLER_DQ5 */
	{0xf, 0x7, 2, 0x7, 0x0310, 16},	/* PATTERN_KILLER_DQ6 */
	{0xf, 0x7, 2, 0x7, 0x0350, 16},	/* PATTERN_KILLER_DQ7 */
	{0xf, 0x7, 2, 0x7, 0x04c0, 16},	/* PATTERN_VREF */
	{0xf, 0x7, 2, 0x7, 0x03c0, 16},	/* PATTERN_FULL_SSO_1T */
	{0xf, 0x7, 2, 0x7, 0x0400, 16},	/* PATTERN_FULL_SSO_2T */
	{0xf, 0x7, 2, 0x7, 0x0440, 16},	/* PATTERN_FULL_SSO_3T */
	{0xf, 0x7, 2, 0x7, 0x0480, 16},	/* PATTERN_FULL_SSO_4T */
	{0xf, 7, 2, 7, 0x6280, 16},	/* PATTERN_SSO_FULL_XTALK_DQ1 */
	{0xf, 7, 2, 7, 0x6680, 16},	/* PATTERN_SSO_FULL_XTALK_DQ1 */
	{0xf, 7, 2, 7, 0x6A80, 16},	/* PATTERN_SSO_FULL_XTALK_DQ2 */
	{0xf, 7, 2, 7, 0x6E80, 16},	/* PATTERN_SSO_FULL_XTALK_DQ3 */
	{0xf, 7, 2, 7, 0x7280, 16},	/* PATTERN_SSO_FULL_XTALK_DQ4 */
	{0xf, 7, 2, 7, 0x7680, 16},	/* PATTERN_SSO_FULL_XTALK_DQ5 */
	{0xf, 7, 2, 7, 0x7A80, 16},	/* PATTERN_SSO_FULL_XTALK_DQ6 */
	{0xf, 7, 2, 7, 0x7E80, 16},	/* PATTERN_SSO_FULL_XTALK_DQ7 */
	{0xf, 7, 2, 7, 0x8280, 16},	/* PATTERN_SSO_XTALK_FREE_DQ0 */
	{0xf, 7, 2, 7, 0x8680, 16},	/* PATTERN_SSO_XTALK_FREE_DQ1 */
	{0xf, 7, 2, 7, 0x8A80, 16},	/* PATTERN_SSO_XTALK_FREE_DQ2 */
	{0xf, 7, 2, 7, 0x8E80, 16},	/* PATTERN_SSO_XTALK_FREE_DQ3 */
	{0xf, 7, 2, 7, 0x9280, 16},	/* PATTERN_SSO_XTALK_FREE_DQ4 */
	{0xf, 7, 2, 7, 0x9680, 16},	/* PATTERN_SSO_XTALK_FREE_DQ5 */
	{0xf, 7, 2, 7, 0x9A80, 16},	/* PATTERN_SSO_XTALK_FREE_DQ6 */
	{0xf, 7, 2, 7, 0x9E80, 16},	/* PATTERN_SSO_XTALK_FREE_DQ7 */
	{0xf, 7, 2, 7, 0xA280, 16}	/* PATTERN_ISI_XTALK_FREE */
	/* Note: actual start_address is "<< 3" of defined address */
};

struct pattern_info pattern_table_32[] = {
	/*
	 * num tx phases, tx burst, delay between, rx pattern,
	 * start_address, pattern_len
	 */
	{3, 3, 2, 3, 0x0080, 4},	/* PATTERN_PBS1 */
	{3, 3, 2, 3, 0x00c0, 4},	/* PATTERN_PBS2 */
	{3, 3, 2, 3, 0x0380, 4},	/* PATTERN_PBS3 */
	{3, 3, 2, 3, 0x0040, 4},	/* PATTERN_TEST */
	{3, 3, 2, 3, 0x0100, 4},	/* PATTERN_RL */
	{3, 3, 2, 3, 0x0000, 4},	/* PATTERN_RL2 */
	{0x1f, 0xf, 2, 0xf, 0x0140, 32},	/* PATTERN_STATIC_PBS */
	{0x1f, 0xf, 2, 0xf, 0x0190, 32},	/* PATTERN_KILLER_DQ0 */
	{0x1f, 0xf, 2, 0xf, 0x01d0, 32},	/* PATTERN_KILLER_DQ1 */
	{0x1f, 0xf, 2, 0xf, 0x0210, 32},	/* PATTERN_KILLER_DQ2 */
	{0x1f, 0xf, 2, 0xf, 0x0250, 32},	/* PATTERN_KILLER_DQ3 */
	{0x1f, 0xf, 2, 0xf, 0x0290, 32},	/* PATTERN_KILLER_DQ4 */
	{0x1f, 0xf, 2, 0xf, 0x02d0, 32},	/* PATTERN_KILLER_DQ5 */
	{0x1f, 0xf, 2, 0xf, 0x0310, 32},	/* PATTERN_KILLER_DQ6 */
	{0x1f, 0xf, 2, 0xf, 0x0350, 32},	/* PATTERN_KILLER_DQ7 */
	{0x1f, 0xf, 2, 0xf, 0x04c0, 32},	/* PATTERN_VREF */
	{0x1f, 0xf, 2, 0xf, 0x03c0, 32},	/* PATTERN_FULL_SSO_1T */
	{0x1f, 0xf, 2, 0xf, 0x0400, 32},	/* PATTERN_FULL_SSO_2T */
	{0x1f, 0xf, 2, 0xf, 0x0440, 32},	/* PATTERN_FULL_SSO_3T */
	{0x1f, 0xf, 2, 0xf, 0x0480, 32},	/* PATTERN_FULL_SSO_4T */
	{0x1f, 0xF, 2, 0xf, 0x6280, 32},	/* PATTERN_SSO_FULL_XTALK_DQ0 */
	{0x1f, 0xF, 2, 0xf, 0x6680, 32},	/* PATTERN_SSO_FULL_XTALK_DQ1 */
	{0x1f, 0xF, 2, 0xf, 0x6A80, 32},	/* PATTERN_SSO_FULL_XTALK_DQ2 */
	{0x1f, 0xF, 2, 0xf, 0x6E80, 32},	/* PATTERN_SSO_FULL_XTALK_DQ3 */
	{0x1f, 0xF, 2, 0xf, 0x7280, 32},	/* PATTERN_SSO_FULL_XTALK_DQ4 */
	{0x1f, 0xF, 2, 0xf, 0x7680, 32},	/* PATTERN_SSO_FULL_XTALK_DQ5 */
	{0x1f, 0xF, 2, 0xf, 0x7A80, 32},	/* PATTERN_SSO_FULL_XTALK_DQ6 */
	{0x1f, 0xF, 2, 0xf, 0x7E80, 32},	/* PATTERN_SSO_FULL_XTALK_DQ7 */
	{0x1f, 0xF, 2, 0xf, 0x8280, 32},	/* PATTERN_SSO_XTALK_FREE_DQ0 */
	{0x1f, 0xF, 2, 0xf, 0x8680, 32},	/* PATTERN_SSO_XTALK_FREE_DQ1 */
	{0x1f, 0xF, 2, 0xf, 0x8A80, 32},	/* PATTERN_SSO_XTALK_FREE_DQ2 */
	{0x1f, 0xF, 2, 0xf, 0x8E80, 32},	/* PATTERN_SSO_XTALK_FREE_DQ3 */
	{0x1f, 0xF, 2, 0xf, 0x9280, 32},	/* PATTERN_SSO_XTALK_FREE_DQ4 */
	{0x1f, 0xF, 2, 0xf, 0x9680, 32},	/* PATTERN_SSO_XTALK_FREE_DQ5 */
	{0x1f, 0xF, 2, 0xf, 0x9A80, 32},	/* PATTERN_SSO_XTALK_FREE_DQ6 */
	{0x1f, 0xF, 2, 0xf, 0x9E80, 32},	/* PATTERN_SSO_XTALK_FREE_DQ7 */
	{0x1f, 0xF, 2, 0xf, 0xA280, 32}		/* PATTERN_ISI_XTALK_FREE */
	/* Note: actual start_address is "<< 3" of defined address */
};

u32 train_dev_num;
enum hws_ddr_cs traintrain_cs_type;
u32 train_pup_num;
enum hws_training_result train_result_type;
enum hws_control_element train_control_element;
enum hws_search_dir traine_search_dir;
enum hws_dir train_direction;
u32 train_if_select;
u32 train_init_value;
u32 train_number_iterations;
enum hws_pattern train_pattern;
enum hws_edge_compare train_edge_compare;
u32 train_cs_num;
u32 train_if_acess, train_if_id, train_pup_access;
u32 max_polling_for_done = 1000000;

u32 *ddr3_tip_get_buf_ptr(u32 dev_num, enum hws_search_dir search,
			  enum hws_training_result result_type,
			  u32 interface_num)
{
	u32 *buf_ptr = NULL;

	buf_ptr = &training_res
		[MAX_INTERFACE_NUM * MAX_BUS_NUM * BUS_WIDTH_IN_BITS * search +
		 interface_num * MAX_BUS_NUM * BUS_WIDTH_IN_BITS];

	return buf_ptr;
}

enum {
	PASS,
	FAIL
};
/*
 * IP Training search
 * Note: for one edge search only from fail to pass, else jitter can
 * be be entered into solution.
 */
int ddr3_tip_ip_training(u32 dev_num, enum hws_access_type access_type,
			 u32 interface_num,
			 enum hws_access_type pup_access_type,
			 u32 pup_num, enum hws_training_result result_type,
			 enum hws_control_element control_element,
			 enum hws_search_dir search_dir, enum hws_dir direction,
			 u32 interface_mask, u32 init_value, u32 num_iter,
			 enum hws_pattern pattern,
			 enum hws_edge_compare edge_comp,
			 enum hws_ddr_cs cs_type, u32 cs_num,
			 enum hws_training_ip_stat *train_status)
{
	u32 mask_dq_num_of_regs, mask_pup_num_of_regs, index_cnt,
		reg_data, pup_id;
	u32 tx_burst_size;
	u32 delay_between_burst;
	u32 rd_mode;
	u32 data;
	struct pattern_info *pattern_table = ddr3_tip_get_pattern_table();
	u16 *mask_results_pup_reg_map = ddr3_tip_get_mask_results_pup_reg_map();
	u16 *mask_results_dq_reg_map = ddr3_tip_get_mask_results_dq_reg();
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	if (pup_num >= octets_per_if_num) {
		DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_ERROR,
					 ("pup_num %d not valid\n", pup_num));
	}
	if (interface_num >= MAX_INTERFACE_NUM) {
		DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_ERROR,
					 ("if_id %d not valid\n",
					  interface_num));
	}
	if (train_status == NULL) {
		DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_ERROR,
					 ("error param 4\n"));
		return MV_BAD_PARAM;
	}

	/* load pattern */
	if (cs_type == CS_SINGLE) {
		/* All CSs to CS0     */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, interface_num,
			      DUAL_DUNIT_CFG_REG, 1 << 3, 1 << 3));
		/* All CSs to CS0     */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, interface_num,
			      ODPG_DATA_CTRL_REG,
			      (0x3 | (effective_cs << 26)), 0xc000003));
	} else {
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, interface_num,
			      DUAL_DUNIT_CFG_REG, 0, 1 << 3));
		/*  CS select */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, interface_num,
			      ODPG_DATA_CTRL_REG, 0x3 | cs_num << 26,
			      0x3 | 3 << 26));
	}

	/* load pattern to ODPG */
	ddr3_tip_load_pattern_to_odpg(dev_num, access_type, interface_num,
				      pattern,
				      pattern_table[pattern].start_addr);
	tx_burst_size =	(direction == OPER_WRITE) ?
		pattern_table[pattern].tx_burst_size : 0;
	delay_between_burst = (direction == OPER_WRITE) ? 2 : 0;
	rd_mode = (direction == OPER_WRITE) ? 1 : 0;
	CHECK_STATUS(ddr3_tip_configure_odpg
		     (dev_num, access_type, interface_num, direction,
		      pattern_table[pattern].num_of_phases_tx, tx_burst_size,
		      pattern_table[pattern].num_of_phases_rx,
		      delay_between_burst, rd_mode, effective_cs, STRESS_NONE,
		      DURATION_SINGLE));
	reg_data = (direction == OPER_READ) ? 0 : (0x3 << 30);
	reg_data |= (direction == OPER_READ) ? 0x60 : 0xfa;
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, access_type, interface_num,
		      ODPG_WR_RD_MODE_ENA_REG, reg_data,
		      MASK_ALL_BITS));
	reg_data = (edge_comp == EDGE_PF || edge_comp == EDGE_FP) ? 0 : 1 << 6;
	reg_data |= (edge_comp == EDGE_PF || edge_comp == EDGE_PFP) ?
		(1 << 7) : 0;

	/* change from Pass to Fail will lock the result */
	if (pup_access_type == ACCESS_TYPE_MULTICAST)
		reg_data |= 0xe << 14;
	else
		reg_data |= pup_num << 14;

	if (edge_comp == EDGE_FP) {
		/* don't search for readl edge change, only the state */
		reg_data |= (0 << 20);
	} else if (edge_comp == EDGE_FPF) {
		reg_data |= (0 << 20);
	} else {
		reg_data |= (3 << 20);
	}

	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, access_type, interface_num,
		      GENERAL_TRAINING_OPCODE_REG,
		      reg_data | (0x7 << 8) | (0x7 << 11),
		      (0x3 | (0x3 << 2) | (0x3 << 6) | (1 << 5) | (0x7 << 8) |
		       (0x7 << 11) | (0xf << 14) | (0x3 << 18) | (3 << 20))));
	reg_data = (search_dir == HWS_LOW2HIGH) ? 0 : (1 << 8);
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, access_type, interface_num, OPCODE_REG0_REG(1),
		      1 | reg_data | init_value << 9 | (1 << 25) | (1 << 26),
		      0xff | (1 << 8) | (0xffff << 9) | (1 << 25) | (1 << 26)));

	/*
	 * Write2_dunit(0x10b4, Number_iteration , [15:0])
	 * Max number of iterations
	 */
	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, interface_num,
				       OPCODE_REG1_REG(1), num_iter,
				       0xffff));
	if (control_element == HWS_CONTROL_ELEMENT_DQ_SKEW &&
	    direction == OPER_READ) {
		/*
		 * Write2_dunit(0x10c0, 0x5f , [7:0])
		 * MC PBS Reg Address at DDR PHY
		 */
		reg_data = PBS_RX_BCAST_PHY_REG(effective_cs);
	} else if (control_element == HWS_CONTROL_ELEMENT_DQ_SKEW &&
		   direction == OPER_WRITE) {
		reg_data = PBS_TX_BCAST_PHY_REG(effective_cs);
	} else if (control_element == HWS_CONTROL_ELEMENT_ADLL &&
		   direction == OPER_WRITE) {
		/*
		 * LOOP         0x00000001 + 4*n:
		 * where n (0-3) represents M_CS number
		 */
		/*
		 * Write2_dunit(0x10c0, 0x1 , [7:0])
		 * ADLL WR Reg Address at DDR PHY
		 */
		reg_data = CTX_PHY_REG(effective_cs);
	} else if (control_element == HWS_CONTROL_ELEMENT_ADLL &&
		   direction == OPER_READ) {
		/* ADLL RD Reg Address at DDR PHY */
		reg_data = CRX_PHY_REG(effective_cs);
	} else if (control_element == HWS_CONTROL_ELEMENT_DQS_SKEW &&
		   direction == OPER_WRITE) {
		/* TBD not defined in 0.5.0 requirement  */
	} else if (control_element == HWS_CONTROL_ELEMENT_DQS_SKEW &&
		   direction == OPER_READ) {
		/* TBD not defined in 0.5.0 requirement */
	}

	reg_data |= (0x6 << 28);
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, access_type, interface_num, CAL_PHY_REG(1),
		      reg_data | (init_value << 8),
		      0xff | (0xffff << 8) | (0xf << 24) | (u32) (0xf << 28)));

	mask_dq_num_of_regs = octets_per_if_num * BUS_WIDTH_IN_BITS;
	mask_pup_num_of_regs = octets_per_if_num;

	if (result_type == RESULT_PER_BIT) {
		for (index_cnt = 0; index_cnt < mask_dq_num_of_regs;
		     index_cnt++) {
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, interface_num,
				      mask_results_dq_reg_map[index_cnt], 0,
				      1 << 24));
		}

		/* Mask disabled buses */
		for (pup_id = 0; pup_id < octets_per_if_num;
		     pup_id++) {
			if (IS_BUS_ACTIVE(tm->bus_act_mask, pup_id) == 1)
				continue;

			for (index_cnt = (pup_id * 8); index_cnt < (pup_id + 1) * 8; index_cnt++) {
				CHECK_STATUS(ddr3_tip_if_write
					     (dev_num, access_type,
					      interface_num,
					      mask_results_dq_reg_map
					      [index_cnt], (1 << 24), 1 << 24));
			}
		}

		for (index_cnt = 0; index_cnt < mask_pup_num_of_regs;
		     index_cnt++) {
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, interface_num,
				      mask_results_pup_reg_map[index_cnt],
				      (1 << 24), 1 << 24));
		}
	} else if (result_type == RESULT_PER_BYTE) {
		/* write to adll */
		for (index_cnt = 0; index_cnt < mask_pup_num_of_regs;
		     index_cnt++) {
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, interface_num,
				      mask_results_pup_reg_map[index_cnt], 0,
				      1 << 24));
		}
		for (index_cnt = 0; index_cnt < mask_dq_num_of_regs;
		     index_cnt++) {
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, interface_num,
				      mask_results_dq_reg_map[index_cnt],
				      (1 << 24), (1 << 24)));
		}
	}

	/* trigger training */
	mv_ddr_training_enable();

	/* wa for 16-bit mode: wait for all rfu tests to finish or timeout */
	mdelay(1);

	/* check for training done */
	if (mv_ddr_is_training_done(MAX_POLLING_ITERATIONS, &data) != MV_OK) {
		train_status[0] = HWS_TRAINING_IP_STATUS_TIMEOUT;
	} else { /* training done; check for pass */
		if (data == PASS)
			train_status[0] = HWS_TRAINING_IP_STATUS_SUCCESS;
		else
			train_status[0] = HWS_TRAINING_IP_STATUS_FAIL;
	}

	ddr3_tip_if_write(0, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			  ODPG_DATA_CTRL_REG, 0, MASK_ALL_BITS);

	return MV_OK;
}

/*
 * Load expected Pattern to ODPG
 */
int ddr3_tip_load_pattern_to_odpg(u32 dev_num, enum hws_access_type access_type,
				  u32 if_id, enum hws_pattern pattern,
				  u32 load_addr)
{
	u32 pattern_length_cnt = 0;
	struct pattern_info *pattern_table = ddr3_tip_get_pattern_table();
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	for (pattern_length_cnt = 0;
	     pattern_length_cnt < pattern_table[pattern].pattern_len;
	     pattern_length_cnt++) {	/* FIXME: the ecc patch below is only for a7040 A0 */
		if (MV_DDR_IS_64BIT_DRAM_MODE(tm->bus_act_mask)/* || tm->bus_act_mask == MV_DDR_32BIT_ECC_PUP8_BUS_MASK*/) {
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      ODPG_DATA_WR_DATA_LOW_REG,
				      pattern_table_get_word(dev_num, pattern,
							     (u8) (pattern_length_cnt)),
				      MASK_ALL_BITS));
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      ODPG_DATA_WR_DATA_HIGH_REG,
				      pattern_table_get_word(dev_num, pattern,
							     (u8) (pattern_length_cnt)),
				      MASK_ALL_BITS));
		} else {
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
					      ODPG_DATA_WR_DATA_LOW_REG,
				      pattern_table_get_word(dev_num, pattern,
							     (u8) (pattern_length_cnt * 2)),
				      MASK_ALL_BITS));
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      ODPG_DATA_WR_DATA_HIGH_REG,
				      pattern_table_get_word(dev_num, pattern,
							     (u8) (pattern_length_cnt * 2 + 1)),
				      MASK_ALL_BITS));
		}
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      ODPG_DATA_WR_ADDR_REG, pattern_length_cnt,
			      MASK_ALL_BITS));
	}

	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, access_type, if_id,
		      ODPG_DATA_BUFFER_OFFS_REG, load_addr, MASK_ALL_BITS));

	return MV_OK;
}

/*
 * Configure ODPG
 */
int ddr3_tip_configure_odpg(u32 dev_num, enum hws_access_type access_type,
			    u32 if_id, enum hws_dir direction, u32 tx_phases,
			    u32 tx_burst_size, u32 rx_phases,
			    u32 delay_between_burst, u32 rd_mode, u32 cs_num,
			    u32 addr_stress_jump, u32 single_pattern)
{
	u32 data_value = 0;
	int ret;

	data_value = ((single_pattern << 2) | (tx_phases << 5) |
		      (tx_burst_size << 11) | (delay_between_burst << 15) |
		      (rx_phases << 21) | (rd_mode << 25) | (cs_num << 26) |
		      (addr_stress_jump << 29));
	ret = ddr3_tip_if_write(dev_num, access_type, if_id,
				ODPG_DATA_CTRL_REG, data_value, 0xaffffffc);
	if (ret != MV_OK)
		return ret;

	return MV_OK;
}

int ddr3_tip_process_result(u32 *ar_result, enum hws_edge e_edge,
			    enum hws_edge_search e_edge_search,
			    u32 *edge_result)
{
	u32 i, res;
	int tap_val, max_val = -10000, min_val = 10000;
	int lock_success = 1;

	for (i = 0; i < BUS_WIDTH_IN_BITS; i++) {
		res = GET_LOCK_RESULT(ar_result[i]);
		if (res == 0) {
			lock_success = 0;
			break;
		}
		DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_ERROR,
					 ("lock failed for bit %d\n", i));
	}

	if (lock_success == 1) {
		for (i = 0; i < BUS_WIDTH_IN_BITS; i++) {
			tap_val = GET_TAP_RESULT(ar_result[i], e_edge);
			if (tap_val > max_val)
				max_val = tap_val;
			if (tap_val < min_val)
				min_val = tap_val;
			if (e_edge_search == TRAINING_EDGE_MAX)
				*edge_result = (u32) max_val;
			else
				*edge_result = (u32) min_val;

			DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_ERROR,
						 ("i %d ar_result[i] 0x%x tap_val %d max_val %d min_val %d Edge_result %d\n",
						  i, ar_result[i], tap_val,
						  max_val, min_val,
						  *edge_result));
		}
	} else {
		return MV_FAIL;
	}

	return MV_OK;
}

/*
 * Read training search result
 */
int ddr3_tip_read_training_result(u32 dev_num, u32 if_id,
				  enum hws_access_type pup_access_type,
				  u32 pup_num, u32 bit_num,
				  enum hws_search_dir search,
				  enum hws_dir direction,
				  enum hws_training_result result_type,
				  enum hws_training_load_op operation,
				  u32 cs_num_type, u32 **load_res,
				  int is_read_from_db, u8 cons_tap,
				  int is_check_result_validity)
{
	u32 reg_offset, pup_cnt, start_pup, end_pup, start_reg, end_reg;
	u32 *interface_train_res = NULL;
	u16 *reg_addr = NULL;
	u32 read_data[MAX_INTERFACE_NUM];
	u16 *mask_results_pup_reg_map = ddr3_tip_get_mask_results_pup_reg_map();
	u16 *mask_results_dq_reg_map = ddr3_tip_get_mask_results_dq_reg();
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	/*
	 * Agreed assumption: all CS mask contain same number of bits,
	 * i.e. in multi CS, the number of CS per memory is the same for
	 * all pups
	 */
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_UNICAST, if_id, DUAL_DUNIT_CFG_REG,
		      (cs_num_type == 0) ? 1 << 3 : 0, (1 << 3)));
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_UNICAST, if_id,
		      ODPG_DATA_CTRL_REG, (cs_num_type << 26), (3 << 26)));
	DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_TRACE,
				 ("Read_from_d_b %d cs_type %d oper %d result_type %d direction %d search %d pup_num %d if_id %d pup_access_type %d\n",
				  is_read_from_db, cs_num_type, operation,
				  result_type, direction, search, pup_num,
				  if_id, pup_access_type));

	if ((load_res == NULL) && (is_read_from_db == 1)) {
		DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_ERROR,
					 ("ddr3_tip_read_training_result load_res = NULL"));
		return MV_FAIL;
	}
	if (pup_num >= octets_per_if_num) {
		DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_ERROR,
					 ("pup_num %d not valid\n", pup_num));
	}
	if (if_id >= MAX_INTERFACE_NUM) {
		DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_ERROR,
					 ("if_id %d not valid\n", if_id));
	}
	if (result_type == RESULT_PER_BIT)
		reg_addr = mask_results_dq_reg_map;
	else
		reg_addr = mask_results_pup_reg_map;
	if (pup_access_type == ACCESS_TYPE_UNICAST) {
		start_pup = pup_num;
		end_pup = pup_num;
	} else {		/*pup_access_type == ACCESS_TYPE_MULTICAST) */

		start_pup = 0;
		end_pup = octets_per_if_num - 1;
	}

	for (pup_cnt = start_pup; pup_cnt <= end_pup; pup_cnt++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, pup_cnt);
		DEBUG_TRAINING_IP_ENGINE(
			DEBUG_LEVEL_TRACE,
			("if_id %d start_pup %d end_pup %d pup_cnt %d\n",
			 if_id, start_pup, end_pup, pup_cnt));
		if (result_type == RESULT_PER_BIT) {
			if (bit_num == ALL_BITS_PER_PUP) {
				start_reg = pup_cnt * BUS_WIDTH_IN_BITS;
				end_reg = (pup_cnt + 1) * BUS_WIDTH_IN_BITS - 1;
			} else {
				start_reg =
					pup_cnt * BUS_WIDTH_IN_BITS + bit_num;
				end_reg = pup_cnt * BUS_WIDTH_IN_BITS + bit_num;
			}
		} else {
			start_reg = pup_cnt;
			end_reg = pup_cnt;
		}

		interface_train_res =
			ddr3_tip_get_buf_ptr(dev_num, search, result_type,
					     if_id);
		DEBUG_TRAINING_IP_ENGINE(
			DEBUG_LEVEL_TRACE,
			("start_reg %d end_reg %d interface %p\n",
			 start_reg, end_reg, interface_train_res));
		if (interface_train_res == NULL) {
			DEBUG_TRAINING_IP_ENGINE(
				DEBUG_LEVEL_ERROR,
				("interface_train_res is NULL\n"));
			return MV_FAIL;
		}

		for (reg_offset = start_reg; reg_offset <= end_reg;
		     reg_offset++) {
			if (operation == TRAINING_LOAD_OPERATION_UNLOAD) {
				if (is_read_from_db == 0) {
					CHECK_STATUS(ddr3_tip_if_read
						     (dev_num,
						      ACCESS_TYPE_UNICAST,
						      if_id,
						      reg_addr[reg_offset],
						      read_data,
						      MASK_ALL_BITS));
					if (is_check_result_validity == 1) {
						if ((read_data[if_id] &
						     TIP_ENG_LOCK) == 0) {
							interface_train_res
								[reg_offset] =
								TIP_ENG_LOCK +
								TIP_TX_DLL_RANGE_MAX;
						} else {
							interface_train_res
								[reg_offset] =
								read_data
								[if_id] +
								cons_tap;
						}
					} else {
						interface_train_res[reg_offset]
							= read_data[if_id] +
							cons_tap;
					}
					DEBUG_TRAINING_IP_ENGINE
						(DEBUG_LEVEL_TRACE,
						 ("reg_offset %d value 0x%x addr %p\n",
						  reg_offset,
						  interface_train_res
						  [reg_offset],
						  &interface_train_res
						  [reg_offset]));
				} else {
					*load_res =
						&interface_train_res[start_reg];
					DEBUG_TRAINING_IP_ENGINE
						(DEBUG_LEVEL_TRACE,
						 ("*load_res %p\n", *load_res));
				}
			} else {
				DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_TRACE,
							 ("not supported\n"));
			}
		}
	}

	return MV_OK;
}

/*
 * Load all pattern to memory using ODPG
 */
int ddr3_tip_load_all_pattern_to_mem(u32 dev_num)
{
	u32 pattern = 0, if_id;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		training_result[training_stage][if_id] = TEST_SUCCESS;
	}

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		/* enable single cs */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      DUAL_DUNIT_CFG_REG, (1 << 3), (1 << 3)));
	}

	for (pattern = 0; pattern < PATTERN_LAST; pattern++)
		ddr3_tip_load_pattern_to_mem(dev_num, pattern);

	return MV_OK;
}

/*
 * Load specific pattern to memory using ODPG
 */
int ddr3_tip_load_pattern_to_mem(u32 dev_num, enum hws_pattern pattern)
{
	u32 reg_data, if_id;
	struct pattern_info *pattern_table = ddr3_tip_get_pattern_table();
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	/* load pattern to memory */
	/*
	 * Write Tx mode, CS0, phases, Tx burst size, delay between burst,
	 * rx pattern phases
	 */
	reg_data =
		0x1 | (pattern_table[pattern].num_of_phases_tx << 5) |
		(pattern_table[pattern].tx_burst_size << 11) |
		(pattern_table[pattern].delay_between_bursts << 15) |
		(pattern_table[pattern].num_of_phases_rx << 21) | (0x1 << 25) |
		(effective_cs << 26);
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ODPG_DATA_CTRL_REG, reg_data, MASK_ALL_BITS));
	/* ODPG Write enable from BIST */
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ODPG_DATA_CTRL_REG, (0x1 | (effective_cs << 26)),
		      0xc000003));
	/* disable error injection */
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ODPG_DATA_WR_DATA_ERR_REG, 0, 0x1));
	/* load pattern to ODPG */
	ddr3_tip_load_pattern_to_odpg(dev_num, ACCESS_TYPE_MULTICAST,
				      PARAM_NOT_CARE, pattern,
				      pattern_table[pattern].start_addr);

	if (ddr3_tip_dev_attr_get(dev_num, MV_ATTR_TIP_REV) >= MV_TIP_REV_3) {
		for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
			VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);

			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      SDRAM_ODT_CTRL_HIGH_REG,
				      0x3, 0xf));
		}

		mv_ddr_odpg_enable();
	} else {
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      ODPG_DATA_CTRL_REG, (u32)(0x1 << 31),
			      (u32)(0x1 << 31)));
	}
	mdelay(1);

	if (mv_ddr_is_odpg_done(MAX_POLLING_ITERATIONS) != MV_OK)
		return MV_FAIL;

	/* Disable ODPG and stop write to memory */
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ODPG_DATA_CTRL_REG, (0x1 << 30), (u32) (0x3 << 30)));

	/* return to default */
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ODPG_DATA_CTRL_REG, 0, MASK_ALL_BITS));

	if (ddr3_tip_dev_attr_get(dev_num, MV_ATTR_TIP_REV) >= MV_TIP_REV_3) {
		/* Disable odt0 for CS0 training - need to adjust for multy CS */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      SDRAM_ODT_CTRL_HIGH_REG, 0x0, 0xf));
	}
	/* temporary added */
	mdelay(1);

	return MV_OK;
}

/*
 * Training search routine
 */
int ddr3_tip_ip_training_wrapper_int(u32 dev_num,
				     enum hws_access_type access_type,
				     u32 if_id,
				     enum hws_access_type pup_access_type,
				     u32 pup_num, u32 bit_num,
				     enum hws_training_result result_type,
				     enum hws_control_element control_element,
				     enum hws_search_dir search_dir,
				     enum hws_dir direction,
				     u32 interface_mask, u32 init_value_l2h,
				     u32 init_value_h2l, u32 num_iter,
				     enum hws_pattern pattern,
				     enum hws_edge_compare edge_comp,
				     enum hws_ddr_cs train_cs_type, u32 cs_num,
				     enum hws_training_ip_stat *train_status)
{
	u32 interface_num = 0, start_if, end_if, init_value_used;
	enum hws_search_dir search_dir_id, start_search, end_search;
	enum hws_edge_compare edge_comp_used;
	u8 cons_tap = 0;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	if (train_status == NULL) {
		DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_ERROR,
					 ("train_status is NULL\n"));
		return MV_FAIL;
	}

	if ((train_cs_type > CS_NON_SINGLE) ||
	    (edge_comp >= EDGE_PFP) ||
	    (pattern >= PATTERN_LAST) ||
	    (direction > OPER_WRITE_AND_READ) ||
	    (search_dir > HWS_HIGH2LOW) ||
	    (control_element > HWS_CONTROL_ELEMENT_DQS_SKEW) ||
	    (result_type > RESULT_PER_BYTE) ||
	    (pup_num >= octets_per_if_num) ||
	    (pup_access_type > ACCESS_TYPE_MULTICAST) ||
	    (if_id > 11) || (access_type > ACCESS_TYPE_MULTICAST)) {
		DEBUG_TRAINING_IP_ENGINE(
			DEBUG_LEVEL_ERROR,
			("wrong parameter train_cs_type %d edge_comp %d pattern %d direction %d search_dir %d control_element %d result_type %d pup_num %d pup_access_type %d if_id %d access_type %d\n",
			 train_cs_type, edge_comp, pattern, direction,
			 search_dir, control_element, result_type, pup_num,
			 pup_access_type, if_id, access_type));
		return MV_FAIL;
	}

	if (edge_comp == EDGE_FPF) {
		start_search = HWS_LOW2HIGH;
		end_search = HWS_HIGH2LOW;
		edge_comp_used = EDGE_FP;
	} else {
		start_search = search_dir;
		end_search = search_dir;
		edge_comp_used = edge_comp;
	}

	for (search_dir_id = start_search; search_dir_id <= end_search;
	     search_dir_id++) {
		init_value_used = (search_dir_id == HWS_LOW2HIGH) ?
			init_value_l2h : init_value_h2l;
		DEBUG_TRAINING_IP_ENGINE(
			DEBUG_LEVEL_TRACE,
			("dev_num %d, access_type %d, if_id %d, pup_access_type %d,pup_num %d, result_type %d, control_element %d search_dir_id %d, direction %d, interface_mask %d,init_value_used %d, num_iter %d, pattern %d, edge_comp_used %d, train_cs_type %d, cs_num %d\n",
			 dev_num, access_type, if_id, pup_access_type, pup_num,
			 result_type, control_element, search_dir_id,
			 direction, interface_mask, init_value_used, num_iter,
			 pattern, edge_comp_used, train_cs_type, cs_num));

		ddr3_tip_ip_training(dev_num, access_type, if_id,
				     pup_access_type, pup_num, result_type,
				     control_element, search_dir_id, direction,
				     interface_mask, init_value_used, num_iter,
				     pattern, edge_comp_used, train_cs_type,
				     cs_num, train_status);
		if (access_type == ACCESS_TYPE_MULTICAST) {
			start_if = 0;
			end_if = MAX_INTERFACE_NUM - 1;
		} else {
			start_if = if_id;
			end_if = if_id;
		}

		for (interface_num = start_if; interface_num <= end_if;
		     interface_num++) {
			VALIDATE_IF_ACTIVE(tm->if_act_mask, interface_num);
			cs_num = 0;
			CHECK_STATUS(ddr3_tip_read_training_result
				     (dev_num, interface_num, pup_access_type,
				      pup_num, bit_num, search_dir_id,
				      direction, result_type,
				      TRAINING_LOAD_OPERATION_UNLOAD,
				      train_cs_type, NULL, 0, cons_tap,
				      0));
		}
	}

	return MV_OK;
}
/*
 * Training search & read result routine
 * This function implements the search algorithm
 * first it calls the function ddr3_tip_ip_training_wrapper_int which triggers the search from l2h and h2l
 * this function handles rx and tx search cases
 * in case of rx it only triggers the search (l2h and h2l)
 * in case of tx there are 3 optional algorithm phases:
 * phase 1:
 * it first triggers the search and handles the results as following (phase 1):
 * each bit, which defined by the search two edges (e1 or VW_L and e2 or VW_H), match on of cases:
 *  1.	BIT_LOW_UI	0 =< VW =< 31 in case of jitter use: VW_L <= 31, VW_H <= 31
 *  2.	BIT_HIGH_UI	32 =< VW =< 63 in case of jitter use: VW_L >= 32, VW_H >= 32
 *  3.	BIT_SPLIT_IN	VW_L <= 31 & VW_H >= 32
 *  4.	BIT_SPLIT_OUT*	VW_H < 32 &  VW_L > 32
 * note: the VW units is adll taps
 * phase 2:
 * only bit case BIT_SPLIT_OUT requires another search (phase 2) from the middle range in two directions h2l and l2h
 * because only this case is not locked by the search engine in the first search trigger (phase 1).
 * phase 3:
 * each subphy is categorized according to its bits definition.
 * the sub-phy cases are as follows:
 *  1.BYTE_NOT_DEFINED			the byte has not yet been categorized
 *  2.BYTE_HOMOGENEOUS_LOW		0 =< VW =< 31
 *  3.BYTE_HOMOGENEOUS_HIGH		32 =< VW =< 63
 *  4.BYTE_HOMOGENEOUS_SPLIT_IN		VW_L <= 31 & VW_H >= 32
 *					or the center of all bits in the byte  =< 31
 *  5.BYTE_HOMOGENEOUS_SPLIT_OUT	VW_H < 32 &  VW_L > 32
 *  6.BYTE_SPLIT_OUT_MIX		at least one bits is in split out state and one bit is in other
 *					or the center of all bits in the byte => 32
 * after the two phases above a center valid window for each subphy is calculated accordingly:
 * center valid window = maximum center of all bits in the subphy - minimum center of all bits in the subphy.
 * now decisions are made in each subphy as following:
 * all subphys which are homogeneous remains as is
 * all subphys which are homogeneous low | homogeneous high and the subphy center valid window is less than 32
 *	mark this subphy as homogeneous split in.
 * now the bits in the bytes which are BYTE_SPLIT_OUT_MIX needed to be reorganized and handles as following
 * all bits which are BIT_LOW_UI will be added with 64 adll,
 * this will hopefully ensures that all the bits in the sub phy can be sampled by the dqs
 */
int ddr3_tip_ip_training_wrapper(u32 dev_num, enum hws_access_type access_type,
	u32 if_id,
	enum hws_access_type pup_access_type,
	u32 pup_num,
	enum hws_training_result result_type,
	enum hws_control_element control_element,
	enum hws_search_dir search_dir,
	enum hws_dir direction, u32 interface_mask,
	u32 init_value_l2h, u32 init_value_h2l,
	u32 num_iter, enum hws_pattern pattern,
	enum hws_edge_compare edge_comp,
	enum hws_ddr_cs train_cs_type, u32 cs_num,
	enum hws_training_ip_stat *train_status)
{
	u8 e1, e2;
	u32 bit_id, start_if, end_if, bit_end = 0;
	u32 *result[HWS_SEARCH_DIR_LIMIT] = { 0 };
	u8 cons_tap = (direction == OPER_WRITE) ? (64) : (0);
	u8 bit_bit_mask[MAX_BUS_NUM] = { 0 }, bit_bit_mask_active = 0;
	u8 bit_state[MAX_BUS_NUM * BUS_WIDTH_IN_BITS] = {0};
	u8 h2l_adll_value[MAX_BUS_NUM][BUS_WIDTH_IN_BITS];
	u8 l2h_adll_value[MAX_BUS_NUM][BUS_WIDTH_IN_BITS];
	u8 center_subphy_adll_window[MAX_BUS_NUM];
	u8 min_center_subphy_adll[MAX_BUS_NUM];
	u8 max_center_subphy_adll[MAX_BUS_NUM];
	u32 *l2h_if_train_res = NULL;
	u32 *h2l_if_train_res = NULL;
	enum hws_search_dir search_dir_id;
	int status;
	u32 bit_lock_result;

	u8 sybphy_id;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	if (pup_num >= octets_per_if_num) {
		DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_ERROR,
			("pup_num %d not valid\n", pup_num));
	}

	if (if_id >= MAX_INTERFACE_NUM) {
		DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_ERROR,
			("if_id %d not valid\n", if_id));
	}

	status = ddr3_tip_ip_training_wrapper_int
		(dev_num, access_type, if_id, pup_access_type, pup_num,
		ALL_BITS_PER_PUP, result_type, control_element,
		search_dir, direction, interface_mask, init_value_l2h,
		init_value_h2l, num_iter, pattern, edge_comp,
		train_cs_type, cs_num, train_status);

	if (MV_OK != status)
		return status;

	if (access_type == ACCESS_TYPE_MULTICAST) {
		start_if = 0;
		end_if = MAX_INTERFACE_NUM - 1;
	} else {
		start_if = if_id;
		end_if = if_id;
	}

	for (if_id = start_if; if_id <= end_if; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		/* zero the database */
		bit_bit_mask_active = 0;	/* clean the flag for level2 search */
		memset(bit_state, 0, sizeof(bit_state));
		/* phase 1 */
		for (sybphy_id = 0; sybphy_id < octets_per_if_num; sybphy_id++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, sybphy_id);
			if (result_type == RESULT_PER_BIT)
				bit_end = BUS_WIDTH_IN_BITS;
			else
				bit_end = 0;

			/* zero the data base */
			bit_bit_mask[sybphy_id] = 0;
			byte_status[if_id][sybphy_id] = BYTE_NOT_DEFINED;
			for (bit_id = 0; bit_id < bit_end; bit_id++) {
				h2l_adll_value[sybphy_id][bit_id] = 64;
				l2h_adll_value[sybphy_id][bit_id] = 0;
				for (search_dir_id = HWS_LOW2HIGH; search_dir_id <= HWS_HIGH2LOW;
					search_dir_id++) {
					status = ddr3_tip_read_training_result
						(dev_num, if_id,
							ACCESS_TYPE_UNICAST, sybphy_id, bit_id,
							search_dir_id, direction, result_type,
							TRAINING_LOAD_OPERATION_UNLOAD, CS_SINGLE,
							&result[search_dir_id], 1, 0, 0);

					if (MV_OK != status)
						return status;
				}

				e1 = GET_TAP_RESULT(result[HWS_LOW2HIGH][0], EDGE_1);
				e2 = GET_TAP_RESULT(result[HWS_HIGH2LOW][0], EDGE_1);
				DEBUG_TRAINING_IP_ENGINE
					(DEBUG_LEVEL_INFO,
					 ("if_id %d sybphy_id %d bit %d l2h 0x%x (e1 0x%x) h2l 0x%x (e2 0x%x)\n",
					 if_id, sybphy_id, bit_id, result[HWS_LOW2HIGH][0], e1,
					 result[HWS_HIGH2LOW][0], e2));
				bit_lock_result =
					(GET_LOCK_RESULT(result[HWS_LOW2HIGH][0]) &&
						GET_LOCK_RESULT(result[HWS_HIGH2LOW][0]));

				if (bit_lock_result) {
					/* in case of read operation set the byte status as homogeneous low */
					if (direction == OPER_READ) {
						byte_status[if_id][sybphy_id] |= BYTE_HOMOGENEOUS_LOW;
					} else if ((e2 - e1) > 32) { /* oper_write */
						/* split out */
						bit_state[sybphy_id * BUS_WIDTH_IN_BITS + bit_id] =
							BIT_SPLIT_OUT;
						byte_status[if_id][sybphy_id] |= BYTE_HOMOGENEOUS_SPLIT_OUT;
						/* mark problem bits */
						bit_bit_mask[sybphy_id] |= (1 << bit_id);
						bit_bit_mask_active = 1;
						DEBUG_TRAINING_IP_ENGINE
							(DEBUG_LEVEL_TRACE,
							 ("if_id %d sybphy_id %d bit %d BIT_SPLIT_OUT\n",
							 if_id, sybphy_id, bit_id));
					} else {
						/* low ui */
						if (e1 <= 31 && e2 <= 31) {
							bit_state[sybphy_id * BUS_WIDTH_IN_BITS + bit_id] =
								BIT_LOW_UI;
							byte_status[if_id][sybphy_id] |= BYTE_HOMOGENEOUS_LOW;
							l2h_adll_value[sybphy_id][bit_id] = e1;
							h2l_adll_value[sybphy_id][bit_id] = e2;
							DEBUG_TRAINING_IP_ENGINE
								(DEBUG_LEVEL_TRACE,
								 ("if_id %d sybphy_id %d bit %d BIT_LOW_UI\n",
								 if_id, sybphy_id, bit_id));
						}
							/* high ui */
						if (e1 >= 32 && e2 >= 32) {
							bit_state[sybphy_id * BUS_WIDTH_IN_BITS + bit_id] =
								BIT_HIGH_UI;
							byte_status[if_id][sybphy_id] |= BYTE_HOMOGENEOUS_HIGH;
							l2h_adll_value[sybphy_id][bit_id] = e1;
							h2l_adll_value[sybphy_id][bit_id] = e2;
							DEBUG_TRAINING_IP_ENGINE
								(DEBUG_LEVEL_TRACE,
								 ("if_id %d sybphy_id %d bit %d BIT_HIGH_UI\n",
								 if_id, sybphy_id, bit_id));
						}
						/* split in */
						if (e1 <= 31 && e2 >= 32) {
							bit_state[sybphy_id * BUS_WIDTH_IN_BITS + bit_id] =
								BIT_SPLIT_IN;
							byte_status[if_id][sybphy_id] |=
								BYTE_HOMOGENEOUS_SPLIT_IN;
							l2h_adll_value[sybphy_id][bit_id] = e1;
							h2l_adll_value[sybphy_id][bit_id] = e2;
							DEBUG_TRAINING_IP_ENGINE
								(DEBUG_LEVEL_TRACE,
								 ("if_id %d sybphy_id %d bit %d BIT_SPLIT_IN\n",
								 if_id, sybphy_id, bit_id));
						}
					}
				} else {
					DEBUG_TRAINING_IP_ENGINE
						(DEBUG_LEVEL_INFO,
						 ("if_id %d sybphy_id %d bit %d l2h 0x%x (e1 0x%x)"
						 "h2l 0x%x (e2 0x%x): bit cannot be categorized\n",
						 if_id, sybphy_id, bit_id, result[HWS_LOW2HIGH][0], e1,
						 result[HWS_HIGH2LOW][0], e2));
					/* mark the byte as not defined */
					byte_status[if_id][sybphy_id] = BYTE_NOT_DEFINED;
					break; /* continue to next pup - no reason to analyze this byte */
				}
			} /* for all bits */
		} /* for all PUPs */

		/* phase 2 will occur only in write operation */
		if (bit_bit_mask_active != 0) {
			l2h_if_train_res = ddr3_tip_get_buf_ptr(dev_num, HWS_LOW2HIGH, result_type, if_id);
			h2l_if_train_res = ddr3_tip_get_buf_ptr(dev_num, HWS_HIGH2LOW, result_type, if_id);
			/* search from middle to end */
			ddr3_tip_ip_training
				(dev_num, ACCESS_TYPE_UNICAST,
				 if_id, ACCESS_TYPE_MULTICAST,
				 PARAM_NOT_CARE, result_type,
				 control_element, HWS_LOW2HIGH,
				 direction, interface_mask,
				 num_iter / 2, num_iter / 2,
				 pattern, EDGE_FP, train_cs_type,
				 cs_num, train_status);

			for (sybphy_id = 0; sybphy_id < octets_per_if_num; sybphy_id++) {
				VALIDATE_BUS_ACTIVE(tm->bus_act_mask, sybphy_id);
				if (byte_status[if_id][sybphy_id] != BYTE_NOT_DEFINED) {
					if (bit_bit_mask[sybphy_id] == 0)
						continue; /* this byte bits have no split out state */

					for (bit_id = 0; bit_id < bit_end; bit_id++) {
						if ((bit_bit_mask[sybphy_id] & (1 << bit_id)) == 0)
							continue; /* this bit is non split goto next bit */

						/* enter the result to the data base */
						status = ddr3_tip_read_training_result
							(dev_num, if_id, ACCESS_TYPE_UNICAST, sybphy_id,
							 bit_id, HWS_LOW2HIGH, direction, result_type,
							 TRAINING_LOAD_OPERATION_UNLOAD, CS_SINGLE,
							 &l2h_if_train_res, 0, 0, 1);

						if (MV_OK != status)
							return status;

						l2h_adll_value[sybphy_id][bit_id] =
							l2h_if_train_res[sybphy_id *
							BUS_WIDTH_IN_BITS + bit_id] & PUP_RESULT_EDGE_1_MASK;
					}
				}
			}
			/* Search from middle to start */
			ddr3_tip_ip_training
				(dev_num, ACCESS_TYPE_UNICAST,
				 if_id, ACCESS_TYPE_MULTICAST,
				 PARAM_NOT_CARE, result_type,
				 control_element, HWS_HIGH2LOW,
				 direction, interface_mask,
				 num_iter / 2, num_iter / 2,
				 pattern, EDGE_FP, train_cs_type,
				 cs_num, train_status);

			for (sybphy_id = 0; sybphy_id < octets_per_if_num; sybphy_id++) {
				VALIDATE_BUS_ACTIVE(tm->bus_act_mask, sybphy_id);
				if (byte_status[if_id][sybphy_id] != BYTE_NOT_DEFINED) {
					if (bit_bit_mask[sybphy_id] == 0)
						continue;

					for (bit_id = 0; bit_id < bit_end; bit_id++) {
						if ((bit_bit_mask[sybphy_id] & (1 << bit_id)) == 0)
							continue;

						status = ddr3_tip_read_training_result
							(dev_num, if_id, ACCESS_TYPE_UNICAST, sybphy_id,
							 bit_id, HWS_HIGH2LOW, direction, result_type,
							 TRAINING_LOAD_OPERATION_UNLOAD, CS_SINGLE,
							 &h2l_if_train_res, 0, cons_tap, 1);

						if (MV_OK != status)
							return status;

						h2l_adll_value[sybphy_id][bit_id] =
							h2l_if_train_res[sybphy_id *
							BUS_WIDTH_IN_BITS + bit_id] & PUP_RESULT_EDGE_1_MASK;
					}
				}
			}
		} /* end if bit_bit_mask_active */
		/*
			* phase 3 will occur only in write operation
			* find the maximum and the minimum center of each subphy
			*/
		for (sybphy_id = 0; sybphy_id < octets_per_if_num; sybphy_id++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, sybphy_id);

			if ((byte_status[if_id][sybphy_id] != BYTE_NOT_DEFINED) && (direction == OPER_WRITE)) {
				/* clear the arrays and parameters */
				center_subphy_adll_window[sybphy_id] = 0;
				max_center_subphy_adll[sybphy_id] = 0;
				min_center_subphy_adll[sybphy_id] = 64;
				/* find the max and min center adll value in the current subphy */
				for (bit_id = 0; bit_id < bit_end; bit_id++) {
					/* debug print all the bit edges after alignment */
					DEBUG_TRAINING_IP_ENGINE
						(DEBUG_LEVEL_TRACE,
						 ("if_id %d sybphy_id %d bit %d l2h %d h2l %d\n",
						 if_id, sybphy_id, bit_id, l2h_adll_value[sybphy_id][bit_id],
						 h2l_adll_value[sybphy_id][bit_id]));

					if (((l2h_adll_value[sybphy_id][bit_id] +
					      h2l_adll_value[sybphy_id][bit_id]) / 2) >
					      max_center_subphy_adll[sybphy_id])
						max_center_subphy_adll[sybphy_id] =
						(l2h_adll_value[sybphy_id][bit_id] +
						 h2l_adll_value[sybphy_id][bit_id]) / 2;
					if (((l2h_adll_value[sybphy_id][bit_id] +
					      h2l_adll_value[sybphy_id][bit_id]) / 2) <
					      min_center_subphy_adll[sybphy_id])
						min_center_subphy_adll[sybphy_id] =
						(l2h_adll_value[sybphy_id][bit_id] +
						 h2l_adll_value[sybphy_id][bit_id]) / 2;
				}

				/* calculate the center of the current subphy */
				center_subphy_adll_window[sybphy_id] =
					max_center_subphy_adll[sybphy_id] -
					min_center_subphy_adll[sybphy_id];
				DEBUG_TRAINING_IP_ENGINE
					(DEBUG_LEVEL_TRACE,
					 ("if_id %d sybphy_id %d min center %d max center %d center %d\n",
					 if_id, sybphy_id, min_center_subphy_adll[sybphy_id],
					 max_center_subphy_adll[sybphy_id],
					 center_subphy_adll_window[sybphy_id]));
			}
		}
		/*
			* check byte state and fix bits state if needed
			* in case the level 1 and 2 above subphy results are
			* homogeneous continue to the next subphy
			*/
		for (sybphy_id = 0; sybphy_id < octets_per_if_num; sybphy_id++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, sybphy_id);
			if ((byte_status[if_id][sybphy_id] == BYTE_HOMOGENEOUS_LOW) ||
			    (byte_status[if_id][sybphy_id] == BYTE_HOMOGENEOUS_HIGH) ||
			    (byte_status[if_id][sybphy_id] == BYTE_HOMOGENEOUS_SPLIT_IN) ||
			    (byte_status[if_id][sybphy_id] == BYTE_HOMOGENEOUS_SPLIT_OUT) ||
			    (byte_status[if_id][sybphy_id] == BYTE_NOT_DEFINED))
			continue;

			/*
			 * in case all of the bits in the current subphy are
			 * less than 32 which will find alignment in the subphy bits
			 * mark this subphy as homogeneous split in
			*/
			if (center_subphy_adll_window[sybphy_id] <= 31)
				byte_status[if_id][sybphy_id] = BYTE_HOMOGENEOUS_SPLIT_IN;

			/*
				* in case the current byte is split_out and the center is bigger than 31
				* the byte can be aligned. in this case add 64 to the the low ui bits aligning it
				* to the other ui bits
				*/
			if (center_subphy_adll_window[sybphy_id] >= 32) {
				byte_status[if_id][sybphy_id] = BYTE_SPLIT_OUT_MIX;

				DEBUG_TRAINING_IP_ENGINE
					(DEBUG_LEVEL_TRACE,
					 ("if_id %d sybphy_id %d byte state 0x%x\n",
					 if_id, sybphy_id, byte_status[if_id][sybphy_id]));
				for (bit_id = 0; bit_id < bit_end; bit_id++) {
					if (bit_state[sybphy_id * BUS_WIDTH_IN_BITS + bit_id] == BIT_LOW_UI) {
						l2h_if_train_res[sybphy_id * BUS_WIDTH_IN_BITS + bit_id] += 64;
						h2l_if_train_res[sybphy_id * BUS_WIDTH_IN_BITS + bit_id] += 64;
					}
					DEBUG_TRAINING_IP_ENGINE
						(DEBUG_LEVEL_TRACE,
						 ("if_id %d sybphy_id %d bit_id %d added 64 adlls\n",
						 if_id, sybphy_id, bit_id));
				}
			}
		}
	} /* for all interfaces */

	return MV_OK;
}

u8 mv_ddr_tip_sub_phy_byte_status_get(u32 if_id, u32 subphy_id)
{
	return byte_status[if_id][subphy_id];
}

void mv_ddr_tip_sub_phy_byte_status_set(u32 if_id, u32 subphy_id, u8 byte_status_data)
{
	byte_status[if_id][subphy_id] = byte_status_data;
}

/*
 * Load phy values
 */
int ddr3_tip_load_phy_values(int b_load)
{
	u32 bus_cnt = 0, if_id, dev_num = 0;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
		for (bus_cnt = 0; bus_cnt < octets_per_if_num; bus_cnt++) {
			VALIDATE_BUS_ACTIVE(tm->bus_act_mask, bus_cnt);
			if (b_load == 1) {
				CHECK_STATUS(ddr3_tip_bus_read
					     (dev_num, if_id,
					      ACCESS_TYPE_UNICAST, bus_cnt,
					      DDR_PHY_DATA,
					      CTX_PHY_REG(effective_cs),
					      &phy_reg_bk[if_id][bus_cnt]
					      [0]));
				CHECK_STATUS(ddr3_tip_bus_read
					     (dev_num, if_id,
					      ACCESS_TYPE_UNICAST, bus_cnt,
					      DDR_PHY_DATA,
					      RL_PHY_REG(effective_cs),
					      &phy_reg_bk[if_id][bus_cnt]
					      [1]));
				CHECK_STATUS(ddr3_tip_bus_read
					     (dev_num, if_id,
					      ACCESS_TYPE_UNICAST, bus_cnt,
					      DDR_PHY_DATA,
					      CRX_PHY_REG(effective_cs),
					      &phy_reg_bk[if_id][bus_cnt]
					      [2]));
			} else {
				CHECK_STATUS(ddr3_tip_bus_write
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id, ACCESS_TYPE_UNICAST,
					      bus_cnt, DDR_PHY_DATA,
					      CTX_PHY_REG(effective_cs),
					      phy_reg_bk[if_id][bus_cnt]
					      [0]));
				CHECK_STATUS(ddr3_tip_bus_write
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id, ACCESS_TYPE_UNICAST,
					      bus_cnt, DDR_PHY_DATA,
					      RL_PHY_REG(effective_cs),
					      phy_reg_bk[if_id][bus_cnt]
					      [1]));
				CHECK_STATUS(ddr3_tip_bus_write
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id, ACCESS_TYPE_UNICAST,
					      bus_cnt, DDR_PHY_DATA,
					      CRX_PHY_REG(effective_cs),
					      phy_reg_bk[if_id][bus_cnt]
					      [2]));
			}
		}
	}

	return MV_OK;
}

int ddr3_tip_training_ip_test(u32 dev_num, enum hws_training_result result_type,
			      enum hws_search_dir search_dir,
			      enum hws_dir direction,
			      enum hws_edge_compare edge,
			      u32 init_val1, u32 init_val2,
			      u32 num_of_iterations,
			      u32 start_pattern, u32 end_pattern)
{
	u32 pattern, if_id, pup_id;
	enum hws_training_ip_stat train_status[MAX_INTERFACE_NUM];
	u32 *res = NULL;
	u32 search_state = 0;
	u32 octets_per_if_num = ddr3_tip_dev_attr_get(dev_num, MV_ATTR_OCTET_PER_INTERFACE);
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	ddr3_tip_load_phy_values(1);

	for (pattern = start_pattern; pattern <= end_pattern; pattern++) {
		for (search_state = 0; search_state < HWS_SEARCH_DIR_LIMIT;
		     search_state++) {
			ddr3_tip_ip_training_wrapper(dev_num,
						     ACCESS_TYPE_MULTICAST, 0,
						     ACCESS_TYPE_MULTICAST, 0,
						     result_type,
						     HWS_CONTROL_ELEMENT_ADLL,
						     search_dir, direction,
						     0xfff, init_val1,
						     init_val2,
						     num_of_iterations, pattern,
						     edge, CS_SINGLE,
						     PARAM_NOT_CARE,
						     train_status);

			for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1;
			     if_id++) {
				VALIDATE_IF_ACTIVE(tm->if_act_mask, if_id);
				for (pup_id = 0; pup_id <
					     octets_per_if_num;
				     pup_id++) {
					VALIDATE_BUS_ACTIVE(tm->bus_act_mask,
							pup_id);
					CHECK_STATUS
						(ddr3_tip_read_training_result
						 (dev_num, if_id,
						  ACCESS_TYPE_UNICAST, pup_id,
						  ALL_BITS_PER_PUP,
						  search_state,
						  direction, result_type,
						  TRAINING_LOAD_OPERATION_UNLOAD,
						  CS_SINGLE, &res, 1, 0,
						  0));
					if (result_type == RESULT_PER_BYTE) {
						DEBUG_TRAINING_IP_ENGINE
							(DEBUG_LEVEL_INFO,
							 ("search_state %d if_id %d pup_id %d 0x%x\n",
							  search_state, if_id,
							  pup_id, res[0]));
					} else {
						DEBUG_TRAINING_IP_ENGINE
							(DEBUG_LEVEL_INFO,
							 ("search_state %d if_id %d pup_id %d 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
							  search_state, if_id,
							  pup_id, res[0],
							  res[1], res[2],
							  res[3], res[4],
							  res[5], res[6],
							  res[7]));
					}
				}
			}	/* interface */
		}		/* search */
	}			/* pattern */

	ddr3_tip_load_phy_values(0);

	return MV_OK;
}

int mv_ddr_pattern_start_addr_set(struct pattern_info *pattern_tbl, enum hws_pattern pattern, u32 addr)
{
	pattern_tbl[pattern].start_addr = addr;

	return 0;
}

struct pattern_info *ddr3_tip_get_pattern_table()
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	if (MV_DDR_IS_64BIT_DRAM_MODE(tm->bus_act_mask))
		return pattern_table_64;
	else if (DDR3_IS_16BIT_DRAM_MODE(tm->bus_act_mask) == 0)
		return pattern_table_32;
	else
		return pattern_table_16;
}

u16 *ddr3_tip_get_mask_results_dq_reg()
{
#if MAX_BUS_NUM == 5
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	if (DDR3_IS_ECC_PUP3_MODE(tm->bus_act_mask))
		return mask_results_dq_reg_map_pup3_ecc;
	else
#endif
		return mask_results_dq_reg_map;
}

u16 *ddr3_tip_get_mask_results_pup_reg_map()
{
#if MAX_BUS_NUM == 5
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	if (DDR3_IS_ECC_PUP3_MODE(tm->bus_act_mask))
		return mask_results_pup_reg_map_pup3_ecc;
	else
#endif
		return mask_results_pup_reg_map;
}

/* load expected dm pattern to odpg */
#define LOW_NIBBLE_BYTE_MASK	0xf
#define HIGH_NIBBLE_BYTE_MASK	0xf0
int mv_ddr_load_dm_pattern_to_odpg(enum hws_access_type access_type, enum hws_pattern pattern,
				   enum dm_direction dm_dir)
{
	struct pattern_info *pattern_table = ddr3_tip_get_pattern_table();
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	u32 pattern_len = 0;
	u32 data_low, data_high;
	u8 dm_data;

	for (pattern_len = 0;
	     pattern_len < pattern_table[pattern].pattern_len;
	     pattern_len++) {
		if (MV_DDR_IS_64BIT_DRAM_MODE(tm->bus_act_mask)) {
			data_low = pattern_table_get_word(0, pattern, (u8)pattern_len);
			data_high = data_low;
		} else {
			data_low = pattern_table_get_word(0, pattern, (u8)(pattern_len * 2));
			data_high = pattern_table_get_word(0, pattern, (u8)(pattern_len * 2 + 1));
		}

		/* odpg mbus dm definition is opposite to ddr4 protocol */
		if (dm_dir == DM_DIR_INVERSE)
			dm_data = ~((data_low & LOW_NIBBLE_BYTE_MASK) | (data_high & HIGH_NIBBLE_BYTE_MASK));
		else
			dm_data = (data_low & LOW_NIBBLE_BYTE_MASK) | (data_high & HIGH_NIBBLE_BYTE_MASK);

		ddr3_tip_if_write(0, access_type, 0, ODPG_DATA_WR_DATA_LOW_REG, data_low, MASK_ALL_BITS);
		ddr3_tip_if_write(0, access_type, 0, ODPG_DATA_WR_DATA_HIGH_REG, data_high, MASK_ALL_BITS);
		ddr3_tip_if_write(0, access_type, 0, ODPG_DATA_WR_ADDR_REG,
				  pattern_len | ((dm_data & ODPG_DATA_WR_DATA_MASK) << ODPG_DATA_WR_DATA_OFFS),
				  MASK_ALL_BITS);
	}

	return MV_OK;
}
