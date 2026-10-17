// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 MediaTek Inc.
 *
 * Author:  Weijie Gao <weijie.gao@mediatek.com>
 */

#include <asm/addrspace.h>
#include <asm/global_data.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/sizes.h>
#include <linux/io.h>
#include <mach/ddr.h>
#include <mach/mc.h>
#include "mt7628.h"

DECLARE_GLOBAL_DATA_PTR;

/* DDR2 DQ_DLY */
#define DDR2_DQ_DLY \
				((0x8 << DQ1_DELAY_COARSE_TUNING_S) | \
				(0x2 << DQ1_DELAY_FINE_TUNING_S) | \
				(0x8 << DQ0_DELAY_COARSE_TUNING_S) | \
				(0x2 << DQ0_DELAY_FINE_TUNING_S))

/* DDR2 DQS_DLY */
#define DDR2_DQS_DLY \
				((0x8 << DQS1_DELAY_COARSE_TUNING_S) | \
				(0x3 << DQS1_DELAY_FINE_TUNING_S) | \
				(0x8 << DQS0_DELAY_COARSE_TUNING_S) | \
				(0x3 << DQS0_DELAY_FINE_TUNING_S))

const struct mc_ddr_cfg ddr1_cfgs_200mhz[] = {
	[DRAM_8MB]   = { 0x34A1EB94, 0x20262324, 0x28000033, 0x00000002, 0x00000000 },
	[DRAM_16MB]  = { 0x34A1EB94, 0x202A2324, 0x28000033, 0x00000002, 0x00000000 },
	[DRAM_32MB]  = { 0x34A1E5CA, 0x202E2324, 0x28000033, 0x00000002, 0x00000000 },
	[DRAM_64MB]  = { 0x3421E5CA, 0x20322324, 0x28000033, 0x00000002, 0x00000000 },
	[DRAM_128MB] = { 0x241B05CA, 0x20362334, 0x28000033, 0x00000002, 0x00000000 },
};

const struct mc_ddr_cfg ddr1_cfgs_160mhz[] = {
	[DRAM_8MB]   = { 0x239964A1, 0x20262323, 0x00000033, 0x00000002, 0x00000000 },
	[DRAM_16MB]  = { 0x239964A1, 0x202A2323, 0x00000033, 0x00000002, 0x00000000 },
	[DRAM_32MB]  = { 0x239964A1, 0x202E2323, 0x00000033, 0x00000002, 0x00000000 },
	[DRAM_64MB]  = { 0x239984A1, 0x20322323, 0x00000033, 0x00000002, 0x00000000 },
	[DRAM_128MB] = { 0x239AB4A1, 0x20362333, 0x00000033, 0x00000002, 0x00000000 },
};

const struct mc_ddr_cfg ddr2_cfgs_200mhz[] = {
	[DRAM_32MB]  = { 0x2519E2E5, 0x222E2323, 0x68000C43, 0x00000452, 0x0000000A },
	[DRAM_64MB]  = { 0x249AA2E5, 0x22322323, 0x68000C43, 0x00000452, 0x0000000A },
	[DRAM_128MB] = { 0x249B42E5, 0x22362323, 0x68000C43, 0x00000452, 0x0000000A },
	[DRAM_256MB] = { 0x249CE2E5, 0x223A2323, 0x68000C43, 0x00000452, 0x0000000A },
};

const struct mc_ddr_cfg ddr2_cfgs_160mhz[] = {
	[DRAM_32MB]  = { 0x23918250, 0x222E2322, 0x40000A43, 0x00000452, 0x00000006 },
	[DRAM_64MB]  = { 0x239A2250, 0x22322322, 0x40000A43, 0x00000452, 0x00000008 },
	[DRAM_128MB] = { 0x2392A250, 0x22362322, 0x40000A43, 0x00000452, 0x00000008 },
	[DRAM_256MB] = { 0x24140250, 0x223A2322, 0x40000A43, 0x00000452, 0x00000008 },
};

static void mt7628_memc_reset(int assert)
{
	void __iomem *sysc = ioremap_nocache(SYSCTL_BASE, SYSCTL_SIZE);

	if (assert)
		setbits_32(sysc + SYSCTL_RSTCTL_REG, MC_RST);
	else
		clrbits_32(sysc + SYSCTL_RSTCTL_REG, MC_RST);
}

static void mt7628_ddr_pad_ldo_config(int ddr_type, int pkg_type)
{
	void __iomem *rgc = ioremap_nocache(RGCTL_BASE, RGCTL_SIZE);
	u32 ck_pad1, cmd_pad1, dq_pad0, dq_pad1, dqs_pad0, dqs_pad1;

	setbits_32(rgc + RGCTL_PMU_G0_REG, PMU_CFG_EN);

	if (ddr_type == DRAM_DDR1)
		setbits_32(rgc + RGCTL_PMU_G3_REG, RG_DDRLDO_VOSEL);
	else
		clrbits_32(rgc + RGCTL_PMU_G3_REG, RG_DDRLDO_VOSEL);

	setbits_32(rgc + RGCTL_PMU_G3_REG, NI_DDRLDO_EN);

	__udelay(250 * 50);

	setbits_32(rgc + RGCTL_PMU_G3_REG, NI_DDRLDO_STB);
	setbits_32(rgc + RGCTL_PMU_G1_REG, RG_BUCK_FPWM);

	ck_pad1 = readl(rgc + RGCTL_DDR_PAD_CK_G1_REG);
	cmd_pad1 = readl(rgc + RGCTL_DDR_PAD_CMD_G1_REG);
	dq_pad0 = readl(rgc + RGCTL_DDR_PAD_DQ_G0_REG);
	dq_pad1 = readl(rgc + RGCTL_DDR_PAD_DQ_G1_REG);
	dqs_pad0 = readl(rgc + RGCTL_DDR_PAD_DQS_G0_REG);
	dqs_pad1 = readl(rgc + RGCTL_DDR_PAD_DQS_G1_REG);

	ck_pad1 &= ~(DRVP_M | DRVN_M);
	cmd_pad1 &= ~(DRVP_M | DRVN_M);
	dq_pad0 &= ~RTT_M;
	dq_pad1 &= ~(DRVP_M | DRVN_M);
	dqs_pad0 &= ~RTT_M;
	dqs_pad1 &= ~(DRVP_M | DRVN_M);

	if (pkg_type == PKG_ID_KN) {
		ck_pad1 |= (3 << DRVP_S) | (3 << DRVN_S);
		cmd_pad1 |= (3 << DRVP_S) | (3 << DRVN_S);
		dq_pad1 |= (3 << DRVP_S) | (3 << DRVN_S);
		dqs_pad1 |= (3 << DRVP_S) | (3 << DRVN_S);
	} else {
		ck_pad1 |= (12 << DRVP_S) | (12 << DRVN_S);
		cmd_pad1 |= (2 << DRVP_S) | (2 << DRVN_S);
		dqs_pad1 |= (12 << DRVP_S) | (12 << DRVN_S);
		if (ddr_type == DRAM_DDR1)
			dq_pad1 |= (7 << DRVP_S) | (7 << DRVN_S);
		else
			dq_pad1 |= (4 << DRVP_S) | (4 << DRVN_S);
	}

	writel(ck_pad1, rgc + RGCTL_DDR_PAD_CK_G1_REG);
	writel(cmd_pad1, rgc + RGCTL_DDR_PAD_CMD_G1_REG);
	writel(dq_pad0, rgc + RGCTL_DDR_PAD_DQ_G0_REG);
	writel(dq_pad1, rgc + RGCTL_DDR_PAD_DQ_G1_REG);
	writel(dqs_pad0, rgc + RGCTL_DDR_PAD_DQS_G0_REG);
	writel(dqs_pad1, rgc + RGCTL_DDR_PAD_DQS_G1_REG);
}

void mt7628_ddr_init(void)
{
	void __iomem *sysc;
	int ddr_type, pkg_type, lspd;
	struct mc_ddr_init_param param;

	sysc = ioremap_nocache(SYSCTL_BASE, SYSCTL_SIZE);
	ddr_type = readl(sysc + SYSCTL_SYSCFG0_REG) & DRAM_TYPE;
	pkg_type = !!(readl(sysc + SYSCTL_CHIP_REV_ID_REG) & PKG_ID);
	lspd = readl(sysc + SYSCTL_CLKCFG0_REG) &
	       (CPU_PLL_FROM_BBP | CPU_PLL_FROM_XTAL);

	if (pkg_type == PKG_ID_KN)
		ddr_type = DRAM_DDR1;

	mt7628_memc_reset(1);
	__udelay(200);

	mt7628_ddr_pad_ldo_config(ddr_type, pkg_type);

	param.memc = ioremap_nocache(MEMCTL_BASE, MEMCTL_SIZE);
	param.dq_dly = DDR2_DQ_DLY;
	param.dqs_dly = DDR2_DQS_DLY;
	param.mc_reset = mt7628_memc_reset;
	param.memsize = 0;
	param.bus_width = 0;

	if (ddr_type == DRAM_DDR1) {
		if (lspd)
			param.cfgs = ddr1_cfgs_160mhz;
		else
			param.cfgs = ddr1_cfgs_200mhz;
		ddr1_init(&param);
	} else {
		if (lspd)
			param.cfgs = ddr2_cfgs_160mhz;
		else
			param.cfgs = ddr2_cfgs_200mhz;
		ddr2_init(&param);
	}

	ddr_calibrate(param.memc, param.memsize, param.bus_width);

	gd->ram_size = param.memsize;
}
