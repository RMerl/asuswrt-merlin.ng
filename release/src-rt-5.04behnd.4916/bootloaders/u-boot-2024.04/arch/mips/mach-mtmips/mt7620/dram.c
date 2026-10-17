// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 MediaTek Inc. All Rights Reserved.
 *
 * Author:  Weijie Gao <weijie.gao@mediatek.com>
 */

#include <asm/addrspace.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/sizes.h>
#include <linux/io.h>
#include <mach/ddr.h>
#include <mach/mc.h>
#include "mt7620.h"

/* SDR parameters */
#define SDR_CFG0_VAL		0x51B283B3
#define SDR_CFG1_VAL		0xC00003A9

/* DDR2 DQ_DLY */
#define DDR2_DQ_DLY		0x88888888

/* DDR2 DQS_DLY */
#define DDR2_DQS_DLY		0x88888888

static const struct mc_ddr_cfg ddr1_cfgs_200mhz[] = {
	[DRAM_8MB]   = { 0x34A1EB94, 0x20262324, 0x28000033, 0x00000002, 0x00000000 },
	[DRAM_16MB]  = { 0x34A1EB94, 0x202A2324, 0x28000033, 0x00000002, 0x00000000 },
	[DRAM_32MB]  = { 0x34A1E5CA, 0x202E2324, 0x28000033, 0x00000002, 0x00000000 },
	[DRAM_64MB]  = { 0x3421E5CA, 0x20322324, 0x28000033, 0x00000002, 0x00000000 },
	[DRAM_128MB] = { 0x241B05CA, 0x20362334, 0x28000033, 0x00000002, 0x00000000 },
};

static const struct mc_ddr_cfg ddr1_cfgs_160mhz[] = {
	[DRAM_8MB]   = { 0x239964A1, 0x20262323, 0x00000033, 0x00000002, 0x00000000 },
	[DRAM_16MB]  = { 0x239964A1, 0x202A2323, 0x00000033, 0x00000002, 0x00000000 },
	[DRAM_32MB]  = { 0x239964A1, 0x202E2323, 0x00000033, 0x00000002, 0x00000000 },
	[DRAM_64MB]  = { 0x239984A1, 0x20322323, 0x00000033, 0x00000002, 0x00000000 },
	[DRAM_128MB] = { 0x239AB4A1, 0x20362333, 0x00000033, 0x00000002, 0x00000000 },
};

static const struct mc_ddr_cfg ddr2_cfgs_200mhz[] = {
	[DRAM_32MB]  = { 0x2519E2E5, 0x222E2323, 0x68000C43, 0x00000416, 0x0000000A },
	[DRAM_64MB]  = { 0x249AA2E5, 0x22322323, 0x68000C43, 0x00000416, 0x0000000A },
	[DRAM_128MB] = { 0x249B42E5, 0x22362323, 0x68000C43, 0x00000416, 0x0000000A },
	[DRAM_256MB] = { 0x249CE2E5, 0x223A2323, 0x68000C43, 0x00000416, 0x0000000A },
};

static const struct mc_ddr_cfg ddr2_cfgs_160mhz[] = {
	[DRAM_32MB]  = { 0x23918250, 0x222E2322, 0x40000A43, 0x00000416, 0x00000006 },
	[DRAM_64MB]  = { 0x239A2250, 0x22322322, 0x40000A43, 0x00000416, 0x00000008 },
	[DRAM_128MB] = { 0x2392A250, 0x22362322, 0x40000A43, 0x00000416, 0x00000008 },
	[DRAM_256MB] = { 0x24140250, 0x223A2322, 0x40000A43, 0x00000416, 0x00000008 },
};

static void mt7620_memc_reset(int assert)
{
	void __iomem *sysc = ioremap_nocache(SYSCTL_BASE, SYSCTL_SIZE);

	if (assert)
		setbits_32(sysc + SYSCTL_RSTCTL_REG, MC_RST);
	else
		clrbits_32(sysc + SYSCTL_RSTCTL_REG, MC_RST);
}

void mt7620_dram_init(void)
{
	void __iomem *sysc;
	bool lspd = false;
	int ddr_type, aux;
	struct mc_ddr_init_param param;

	sysc = ioremap_nocache(SYSCTL_BASE, SYSCTL_SIZE);
	ddr_type = (readl(sysc + SYSCTL_SYSCFG0_REG) & DRAM_TYPE_M)
		   >> DRAM_TYPE_S;
	aux = readl(sysc + SYSCTL_CPLL_CFG1_REG) &
	      (CPU_CLK_AUX1 | CPU_CLK_AUX0);

	if (aux == CPU_CLK_AUX1 || aux == CPU_CLK_AUX0)
		lspd = true;

	mt7620_memc_reset(1);
	__udelay(200);

	param.memc = ioremap_nocache(MEMCTL_BASE, MEMCTL_SIZE);
	param.dq_dly = DDR2_DQ_DLY;
	param.dqs_dly = DDR2_DQS_DLY;
	param.mc_reset = mt7620_memc_reset;
	param.memsize = 0;
	param.bus_width = 0;

	if (ddr_type == DRAM_DDR1) {
		if (lspd)
			param.cfgs = ddr1_cfgs_160mhz;
		else
			param.cfgs = ddr1_cfgs_200mhz;

		ddr1_init(&param);
	} else if (ddr_type == DRAM_DDR2) {
		if (lspd)
			param.cfgs = ddr2_cfgs_160mhz;
		else
			param.cfgs = ddr2_cfgs_200mhz;

		ddr2_init(&param);
	} else {
		param.sdr_cfg0 = SDR_CFG0_VAL;
		param.sdr_cfg1 = SDR_CFG1_VAL;

		sdr_init(&param);
	}
}
