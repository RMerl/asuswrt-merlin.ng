// SPDX-License-Identifier:	GPL-2.0+
/*
 * D1/R528/T113 DRAM controller register and constant defines
 *
 * (C) Copyright 2022 Arm Ltd.
 * Based on H6 and H616 header, which are:
 * (C) Copyright 2017  Icenowy Zheng <icenowy@aosc.io>
 * (C) Copyright 2020  Jernej Skrabec <jernej.skrabec@siol.net>
 *
 */

#ifndef _SUNXI_DRAM_SUN20I_D1_H
#define _SUNXI_DRAM_SUN20I_D1_H

enum sunxi_dram_type {
	SUNXI_DRAM_TYPE_DDR2 = 2,
	SUNXI_DRAM_TYPE_DDR3 = 3,
	SUNXI_DRAM_TYPE_LPDDR2 = 6,
	SUNXI_DRAM_TYPE_LPDDR3 = 7,
};

/*
 * This structure contains a mixture of fixed configuration settings,
 * variables that are used at runtime to communicate settings between
 * different stages and functions, and unused values.
 * This is copied from Allwinner's boot0 data structure, which can be
 * found at offset 0x38 in any boot0 binary. To allow matching up some
 * board specific settings, this struct is kept compatible, even though
 * we don't need all members in our code.
 */
typedef struct dram_para {
	/* normal configuration */
	const u32	dram_clk;
	const u32	dram_type;
	const u32	dram_zq;
	const u32	dram_odt_en;

	/* timing configuration */
	const u32	dram_mr0;
	const u32	dram_mr1;
	const u32	dram_mr2;
	const u32	dram_mr3;
	const u32	dram_tpr0;	//DRAMTMG0
	const u32	dram_tpr1;	//DRAMTMG1
	const u32	dram_tpr2;	//DRAMTMG2
	const u32	dram_tpr3;	//DRAMTMG3
	const u32	dram_tpr4;	//DRAMTMG4
	const u32	dram_tpr5;	//DRAMTMG5
	const u32	dram_tpr6;	//DRAMTMG8
	const u32	dram_tpr7;
	const u32	dram_tpr8;
	const u32	dram_tpr9;
	const u32	dram_tpr10;
	const u32	dram_tpr11;
	const u32	dram_tpr12;
} dram_para_t;

typedef struct dram_config {
	/* control configuration */
	u32	dram_para1;
	u32	dram_para2;
	/* contains a bitfield of DRAM setup settings */
	u32	dram_tpr13;
} dram_config_t;

static inline int ns_to_t(int nanoseconds)
{
	const unsigned int ctrl_freq = CONFIG_DRAM_CLK / 2;

	return DIV_ROUND_UP(ctrl_freq * nanoseconds, 1000);
}

#endif /* _SUNXI_DRAM_SUN20I_D1_H */
