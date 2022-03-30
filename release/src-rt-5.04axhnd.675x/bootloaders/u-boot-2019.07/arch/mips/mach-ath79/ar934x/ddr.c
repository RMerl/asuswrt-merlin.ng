// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Marek Vasut <marex@denx.de>
 *
 * Based on RAM init sequence by Piotr Dymacz <pepe2k@gmail.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/addrspace.h>
#include <asm/types.h>
#include <mach/ar71xx_regs.h>
#include <mach/ath79.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	AR934X_SDRAM = 0,
	AR934X_DDR1,
	AR934X_DDR2,
};

struct ar934x_mem_config {
	u32	config1;
	u32	config2;
	u32	mode;
	u32	extmode;
	u32	tap;
};

static const struct ar934x_mem_config ar934x_mem_config[] = {
	[AR934X_SDRAM] = { 0x7fbe8cd0, 0x959f66a8, 0x33, 0, 0x1f1f },
	[AR934X_DDR1]  = { 0x7fd48cd0, 0x99d0e6a8, 0x33, 0, 0x14 },
	[AR934X_DDR2]  = { 0xc7d48cd0, 0x9dd0e6a8, 0x33, 0, 0x10012 },
};

void ar934x_ddr_init(const u16 cpu_mhz, const u16 ddr_mhz, const u16 ahb_mhz)
{
	void __iomem *ddr_regs;
	const struct ar934x_mem_config *memcfg;
	int memtype;
	u32 reg, cycle, ctl;

	ddr_regs = map_physmem(AR71XX_DDR_CTRL_BASE, AR71XX_DDR_CTRL_SIZE,
			       MAP_NOCACHE);

	reg = ath79_get_bootstrap();
	if (reg & AR934X_BOOTSTRAP_SDRAM_DISABLED) {	/* DDR */
		if (reg & AR934X_BOOTSTRAP_DDR1) {	/* DDR 1 */
			memtype = AR934X_DDR1;
			cycle = 0xffff;
		} else {				/* DDR 2 */
			memtype = AR934X_DDR2;
			if (gd->arch.rev) {
				ctl = BIT(6);	/* Undocumented bit :-( */
				if (reg & BIT(3))
					cycle = 0xff;
				else
					cycle = 0xffff;
			} else {
				/* Force DDR2/x16 configuratio on old chips. */
				ctl = 0;
				cycle = 0xffff;		/* DDR2 16bit */
			}

			writel(0xe59, ddr_regs + AR934X_DDR_REG_DDR2_CONFIG);
			udelay(100);

			writel(0x10, ddr_regs + AR71XX_DDR_REG_CONTROL);
			udelay(10);

			writel(0x20, ddr_regs + AR71XX_DDR_REG_CONTROL);
			udelay(10);

			writel(ctl, ddr_regs + AR934X_DDR_REG_CTL_CONF);
			udelay(10);
		}
	} else {					/* SDRAM */
		memtype = AR934X_SDRAM;
		cycle = 0xffffffff;

		writel(0x13b, ddr_regs + AR934X_DDR_REG_CTL_CONF);
		udelay(100);

		/* Undocumented register */
		writel(0x13b, ddr_regs + 0x118);
		udelay(100);
	}

	memcfg = &ar934x_mem_config[memtype];

	writel(memcfg->config1, ddr_regs + AR71XX_DDR_REG_CONFIG);
	udelay(100);

	writel(memcfg->config2, ddr_regs + AR71XX_DDR_REG_CONFIG2);
	udelay(100);

	writel(0x8, ddr_regs + AR71XX_DDR_REG_CONTROL);
	udelay(10);

	writel(memcfg->mode | 0x100, ddr_regs + AR71XX_DDR_REG_MODE);
	mdelay(1);

	writel(0x1, ddr_regs + AR71XX_DDR_REG_CONTROL);
	udelay(10);

	if (memtype == AR934X_DDR2) {
		writel(memcfg->mode | 0x100, ddr_regs + AR71XX_DDR_REG_EMR);
		udelay(100);

		writel(0x2, ddr_regs + AR71XX_DDR_REG_CONTROL);
		udelay(10);
	}

	if (memtype != AR934X_SDRAM)
		writel(0x402, ddr_regs + AR71XX_DDR_REG_EMR);

	udelay(100);

	writel(0x2, ddr_regs + AR71XX_DDR_REG_CONTROL);
	udelay(10);

	writel(0x8, ddr_regs + AR71XX_DDR_REG_CONTROL);
	udelay(10);

	writel(memcfg->mode, ddr_regs + AR71XX_DDR_REG_MODE);
	udelay(100);

	writel(0x1, ddr_regs + AR71XX_DDR_REG_CONTROL);
	udelay(10);

	writel(0x412c /* FIXME */, ddr_regs + AR71XX_DDR_REG_REFRESH);
	udelay(100);

	writel(memcfg->tap, ddr_regs + AR71XX_DDR_REG_TAP_CTRL0);
	writel(memcfg->tap, ddr_regs + AR71XX_DDR_REG_TAP_CTRL1);

	if (memtype != AR934X_SDRAM) {
		if ((gd->arch.rev && (reg & BIT(3))) || !gd->arch.rev) {
			writel(memcfg->tap,
			       ddr_regs + AR934X_DDR_REG_TAP_CTRL2);
			writel(memcfg->tap,
			       ddr_regs + AR934X_DDR_REG_TAP_CTRL3);
		}
	}

	writel(cycle, ddr_regs + AR71XX_DDR_REG_RD_CYCLE);
	udelay(100);

	writel(0x74444444, ddr_regs + AR934X_DDR_REG_BURST);
	udelay(100);

	writel(0x222, ddr_regs + AR934X_DDR_REG_BURST2);
	udelay(100);

	writel(0xfffff, ddr_regs + AR934X_DDR_REG_TIMEOUT_MAX);
	udelay(100);
}

void ddr_tap_tuning(void)
{
}
