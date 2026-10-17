// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 * Copyright (C) 2020-2022 Intel Corporation <www.intel.com>
 */

#include <common.h>
#include <asm/arch/clock_manager.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include "clk-mem-n5x.h"
#include <clk-uclass.h>
#include <dm.h>
#include <dm/lists.h>
#include <dm/util.h>
#include <dt-bindings/clock/n5x-clock.h>

DECLARE_GLOBAL_DATA_PTR;

struct socfpga_mem_clk_plat {
	void __iomem *regs;
};

void clk_mem_wait_for_lock(struct socfpga_mem_clk_plat *plat, u32 mask)
{
	u32 inter_val;
	u32 retry = 0;

	do {
		inter_val = CM_REG_READL(plat, MEMCLKMGR_STAT) & mask;

		/* Wait for stable lock */
		if (inter_val == mask)
			retry++;
		else
			retry = 0;

		if (retry >= 10)
			return;
	} while (1);
}

/*
 * function to write the bypass register which requires a poll of the
 * busy bit
 */
void clk_mem_write_bypass_mempll(struct socfpga_mem_clk_plat *plat, u32 val)
{
	CM_REG_WRITEL(plat, val, MEMCLKMGR_MEMPLL_BYPASS);
}

/*
 * Setup clocks while making no assumptions about previous state of the clocks.
 */
static void clk_mem_basic_init(struct udevice *dev,
			       const struct cm_config * const cfg)
{
	struct socfpga_mem_clk_plat *plat = dev_get_plat(dev);

	if (!cfg)
		return;

	/* Put PLLs in bypass */
	clk_mem_write_bypass_mempll(plat, MEMCLKMGR_BYPASS_MEMPLL_ALL);

	/* Put PLLs in Reset */
	CM_REG_SETBITS(plat, MEMCLKMGR_MEMPLL_PLLCTRL,
		       MEMCLKMGR_PLLCTRL_BYPASS_MASK);

	/* setup mem PLL */
	CM_REG_WRITEL(plat, cfg->mem_memdiv, MEMCLKMGR_MEMPLL_MEMDIV);
	CM_REG_WRITEL(plat, cfg->mem_pllglob, MEMCLKMGR_MEMPLL_PLLGLOB);
	CM_REG_WRITEL(plat, cfg->mem_plldiv, MEMCLKMGR_MEMPLL_PLLDIV);
	CM_REG_WRITEL(plat, cfg->mem_plloutdiv, MEMCLKMGR_MEMPLL_PLLOUTDIV);

	/* Take PLL out of reset and power up */
	CM_REG_CLRBITS(plat, MEMCLKMGR_MEMPLL_PLLCTRL,
		       MEMCLKMGR_PLLCTRL_BYPASS_MASK);
}

static int socfpga_mem_clk_enable(struct clk *clk)
{
	const struct cm_config *cm_default_cfg = cm_get_default_config();
	struct socfpga_mem_clk_plat *plat = dev_get_plat(clk->dev);

	clk_mem_basic_init(clk->dev, cm_default_cfg);

	clk_mem_wait_for_lock(plat, MEMCLKMGR_STAT_ALLPLL_LOCKED_MASK);

	CM_REG_WRITEL(plat, CM_REG_READL(plat, MEMCLKMGR_MEMPLL_PLLGLOB) |
		      MEMCLKMGR_PLLGLOB_CLR_LOSTLOCK_BYPASS_MASK,
		      MEMCLKMGR_MEMPLL_PLLGLOB);

	/* Take all PLLs out of bypass */
	clk_mem_write_bypass_mempll(plat, 0);

	/* Clear the loss of lock bits (write 1 to clear) */
	CM_REG_CLRBITS(plat, MEMCLKMGR_INTRCLR,
		       MEMCLKMGR_INTER_MEMPLLLOST_MASK);

	/* Take all ping pong counters out of reset */
	CM_REG_CLRBITS(plat, MEMCLKMGR_MEMPLL_EXTCNTRST,
		       MEMCLKMGR_EXTCNTRST_ALLCNTRST);

	return 0;
}

static int socfpga_mem_clk_of_to_plat(struct udevice *dev)
{
	struct socfpga_mem_clk_plat *plat = dev_get_plat(dev);
	fdt_addr_t addr;

	addr = devfdt_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;
	plat->regs = (void __iomem *)addr;

	return 0;
}

static struct clk_ops socfpga_mem_clk_ops = {
	.enable		= socfpga_mem_clk_enable
};

static const struct udevice_id socfpga_mem_clk_match[] = {
	{ .compatible = "intel,n5x-mem-clkmgr" },
	{}
};

U_BOOT_DRIVER(socfpga_n5x_mem_clk) = {
	.name		= "mem-clk-n5x",
	.id		= UCLASS_CLK,
	.of_match	= socfpga_mem_clk_match,
	.ops		= &socfpga_mem_clk_ops,
	.of_to_plat     = socfpga_mem_clk_of_to_plat,
	.plat_auto	= sizeof(struct socfpga_mem_clk_plat),
};
