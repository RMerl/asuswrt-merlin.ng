/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 NXP
 */

#ifndef __ASM_ARCH_IMX8ULP_DDR_H
#define __ASM_ARCH_IMX8ULP_DDR_H

#include <asm/io.h>
#include <asm/types.h>

struct dram_cfg_param {
	unsigned int reg;
	unsigned int val;
};

struct dram_timing_info2 {
	/* ddr controller config */
	struct dram_cfg_param *ctl_cfg;
	unsigned int ctl_cfg_num;
	/* pi config */
	struct dram_cfg_param *pi_cfg;
	unsigned int pi_cfg_num;
	/* phy freq1 config */
	struct dram_cfg_param *phy_f1_cfg;
	unsigned int phy_f1_cfg_num;
	/* phy freq2 config */
	struct dram_cfg_param *phy_f2_cfg;
	unsigned int phy_f2_cfg_num;
	/* initialized drate table */
	unsigned int fsp_table[3];
};

extern struct dram_timing_info2 dram_timing;

int ddr_init(struct dram_timing_info2 *dram_timing);

#endif
