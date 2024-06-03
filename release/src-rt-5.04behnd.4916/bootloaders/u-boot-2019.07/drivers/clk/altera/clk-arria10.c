// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <asm/io.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dm/lists.h>
#include <dm/util.h>

#include <asm/arch/clock_manager.h>

enum socfpga_a10_clk_type {
	SOCFPGA_A10_CLK_MAIN_PLL,
	SOCFPGA_A10_CLK_PER_PLL,
	SOCFPGA_A10_CLK_PERIP_CLK,
	SOCFPGA_A10_CLK_GATE_CLK,
	SOCFPGA_A10_CLK_UNKNOWN_CLK,
};

struct socfpga_a10_clk_platdata {
	enum socfpga_a10_clk_type type;
	struct clk_bulk	clks;
	u32		regs;
	/* Fixed divider */
	u16		fix_div;
	/* Control register */
	u16		ctl_reg;
	/* Divider register */
	u16		div_reg;
	u8		div_len;
	u8		div_off;
	/* Clock gating register */
	u16		gate_reg;
	u8		gate_bit;
};

static int socfpga_a10_clk_get_upstream(struct clk *clk, struct clk **upclk)
{
	struct socfpga_a10_clk_platdata *plat = dev_get_platdata(clk->dev);
	u32 reg, maxval;

	if (plat->clks.count == 0)
		return 0;

	if (plat->clks.count == 1) {
		*upclk = &plat->clks.clks[0];
		return 0;
	}

	if (!plat->ctl_reg) {
		dev_err(clk->dev, "Invalid control register\n");
		return -EINVAL;
	}

	reg = readl(plat->regs + plat->ctl_reg);

	/* Assume PLLs are ON for now */
	if (plat->type == SOCFPGA_A10_CLK_MAIN_PLL) {
		reg = (reg >> 8) & 0x3;
		maxval = 2;
	} else if (plat->type == SOCFPGA_A10_CLK_PER_PLL) {
		reg = (reg >> 8) & 0x3;
		maxval = 3;
	} else {
		reg = (reg >> 16) & 0x7;
		maxval = 4;
	}

	if (reg > maxval) {
		dev_err(clk->dev, "Invalid clock source\n");
		return -EINVAL;
	}

	*upclk = &plat->clks.clks[reg];
	return 0;
}

static int socfpga_a10_clk_endisable(struct clk *clk, bool enable)
{
	struct socfpga_a10_clk_platdata *plat = dev_get_platdata(clk->dev);
	struct clk *upclk = NULL;
	int ret;

	if (!enable && plat->gate_reg)
		clrbits_le32(plat->regs + plat->gate_reg, BIT(plat->gate_bit));

	ret = socfpga_a10_clk_get_upstream(clk, &upclk);
	if (ret)
		return ret;

	if (upclk) {
		if (enable)
			clk_enable(upclk);
		else
			clk_disable(upclk);
	}

	if (enable && plat->gate_reg)
		setbits_le32(plat->regs + plat->gate_reg, BIT(plat->gate_bit));

	return 0;
}

static int socfpga_a10_clk_enable(struct clk *clk)
{
	return socfpga_a10_clk_endisable(clk, true);
}

static int socfpga_a10_clk_disable(struct clk *clk)
{
	return socfpga_a10_clk_endisable(clk, false);
}

static ulong socfpga_a10_clk_get_rate(struct clk *clk)
{
	struct socfpga_a10_clk_platdata *plat = dev_get_platdata(clk->dev);
	struct clk *upclk = NULL;
	ulong rate = 0, reg, numer, denom;
	int ret;

	ret = socfpga_a10_clk_get_upstream(clk, &upclk);
	if (ret || !upclk)
		return 0;

	rate = clk_get_rate(upclk);

	if (plat->type == SOCFPGA_A10_CLK_MAIN_PLL) {
		reg = readl(plat->regs + plat->ctl_reg + 4);	/* VCO1 */
		numer = reg & CLKMGR_MAINPLL_VCO1_NUMER_MSK;
		denom = (reg >> CLKMGR_MAINPLL_VCO1_DENOM_LSB) &
			CLKMGR_MAINPLL_VCO1_DENOM_MSK;

		rate /= denom + 1;
		rate *= numer + 1;
	} else if (plat->type == SOCFPGA_A10_CLK_PER_PLL) {
		reg = readl(plat->regs + plat->ctl_reg + 4);	/* VCO1 */
		numer = reg & CLKMGR_PERPLL_VCO1_NUMER_MSK;
		denom = (reg >> CLKMGR_PERPLL_VCO1_DENOM_LSB) &
			CLKMGR_PERPLL_VCO1_DENOM_MSK;

		rate /= denom + 1;
		rate *= numer + 1;
	} else {
		rate /= plat->fix_div;

		if (plat->fix_div == 1 && plat->ctl_reg) {
			reg = readl(plat->regs + plat->ctl_reg);
			reg &= 0x7ff;
			rate /= reg + 1;
		}

		if (plat->div_reg) {
			reg = readl(plat->regs + plat->div_reg);
			reg >>= plat->div_off;
			reg &= (1 << plat->div_len) - 1;
			if (plat->type == SOCFPGA_A10_CLK_PERIP_CLK)
				rate /= reg + 1;
			if (plat->type == SOCFPGA_A10_CLK_GATE_CLK)
				rate /= 1 << reg;
		}
	}

	return rate;
}

static struct clk_ops socfpga_a10_clk_ops = {
	.enable		= socfpga_a10_clk_enable,
	.disable	= socfpga_a10_clk_disable,
	.get_rate	= socfpga_a10_clk_get_rate,
};

/*
 * This workaround tries to fix the massively broken generated "handoff" DT,
 * which contains duplicate clock nodes without any connection to the clock
 * manager DT node. Yet, those "handoff" DT nodes contain configuration of
 * the fixed input clock of the Arria10 which are missing from the base DT
 * for Arria10.
 *
 * This workaround sets up upstream clock for the fixed input clocks of the
 * A10 described in the base DT such that they map to the fixed clock from
 * the "handoff" DT. This does not fully match how the clock look on the
 * A10, but it is the least intrusive way to fix this mess.
 */
static void socfpga_a10_handoff_workaround(struct udevice *dev)
{
	struct socfpga_a10_clk_platdata *plat = dev_get_platdata(dev);
	const void *fdt = gd->fdt_blob;
	struct clk_bulk	*bulk = &plat->clks;
	int i, ret, offset = dev_of_offset(dev);
	static const char * const socfpga_a10_fixedclk_map[] = {
		"osc1", "altera_arria10_hps_eosc1",
		"cb_intosc_ls_clk", "altera_arria10_hps_cb_intosc_ls",
		"f2s_free_clk", "altera_arria10_hps_f2h_free",
	};

	if (fdt_node_check_compatible(fdt, offset, "fixed-clock"))
		return;

	for (i = 0; i < ARRAY_SIZE(socfpga_a10_fixedclk_map); i += 2)
		if (!strcmp(dev->name, socfpga_a10_fixedclk_map[i]))
			break;

	if (i == ARRAY_SIZE(socfpga_a10_fixedclk_map))
		return;

	ret = uclass_get_device_by_name(UCLASS_CLK,
					socfpga_a10_fixedclk_map[i + 1], &dev);
	if (ret)
		return;

	bulk->count = 1;
	bulk->clks = devm_kcalloc(dev, bulk->count,
				  sizeof(struct clk), GFP_KERNEL);
	if (!bulk->clks)
		return;

	ret = clk_request(dev, &bulk->clks[0]);
	if (ret)
		free(bulk->clks);
}

static int socfpga_a10_clk_bind(struct udevice *dev)
{
	const void *fdt = gd->fdt_blob;
	int offset = dev_of_offset(dev);
	bool pre_reloc_only = !(gd->flags & GD_FLG_RELOC);
	const char *name;
	int ret;

	for (offset = fdt_first_subnode(fdt, offset);
	     offset > 0;
	     offset = fdt_next_subnode(fdt, offset)) {
		name = fdt_get_name(fdt, offset, NULL);
		if (!name)
			return -EINVAL;

		if (!strcmp(name, "clocks")) {
			offset = fdt_first_subnode(fdt, offset);
			name = fdt_get_name(fdt, offset, NULL);
			if (!name)
				return -EINVAL;
		}

		/* Filter out supported sub-clock */
		if (fdt_node_check_compatible(fdt, offset,
					      "altr,socfpga-a10-pll-clock") &&
		    fdt_node_check_compatible(fdt, offset,
					      "altr,socfpga-a10-perip-clk") &&
		    fdt_node_check_compatible(fdt, offset,
					      "altr,socfpga-a10-gate-clk") &&
		    fdt_node_check_compatible(fdt, offset, "fixed-clock"))
			continue;

		if (pre_reloc_only &&
		    !dm_ofnode_pre_reloc(offset_to_ofnode(offset)))
			continue;

		ret = device_bind_driver_to_node(dev, "clk-a10", name,
						 offset_to_ofnode(offset),
						 NULL);
		if (ret)
			return ret;
	}

	return 0;
}

static int socfpga_a10_clk_probe(struct udevice *dev)
{
	struct socfpga_a10_clk_platdata *plat = dev_get_platdata(dev);
	const void *fdt = gd->fdt_blob;
	int offset = dev_of_offset(dev);

	clk_get_bulk(dev, &plat->clks);

	socfpga_a10_handoff_workaround(dev);

	if (!fdt_node_check_compatible(fdt, offset,
				       "altr,socfpga-a10-pll-clock")) {
		/* Main PLL has 3 upstream clock */
		if (plat->clks.count == 3)
			plat->type = SOCFPGA_A10_CLK_MAIN_PLL;
		else
			plat->type = SOCFPGA_A10_CLK_PER_PLL;
	} else if (!fdt_node_check_compatible(fdt, offset,
					      "altr,socfpga-a10-perip-clk")) {
		plat->type = SOCFPGA_A10_CLK_PERIP_CLK;
	} else if (!fdt_node_check_compatible(fdt, offset,
					      "altr,socfpga-a10-gate-clk")) {
		plat->type = SOCFPGA_A10_CLK_GATE_CLK;
	} else {
		plat->type = SOCFPGA_A10_CLK_UNKNOWN_CLK;
	}

	return 0;
}

static int socfpga_a10_ofdata_to_platdata(struct udevice *dev)
{
	struct socfpga_a10_clk_platdata *plat = dev_get_platdata(dev);
	struct socfpga_a10_clk_platdata *pplat;
	struct udevice *pdev;
	const void *fdt = gd->fdt_blob;
	unsigned int divreg[3], gatereg[2];
	int ret, offset = dev_of_offset(dev);
	u32 regs;

	regs = dev_read_u32_default(dev, "reg", 0x0);

	if (!fdt_node_check_compatible(fdt, offset, "altr,clk-mgr")) {
		plat->regs = devfdt_get_addr(dev);
	} else {
		pdev = dev_get_parent(dev);
		if (!pdev)
			return -ENODEV;

		pplat = dev_get_platdata(pdev);
		if (!pplat)
			return -EINVAL;

		plat->ctl_reg = regs;
		plat->regs = pplat->regs;
	}

	plat->type = SOCFPGA_A10_CLK_UNKNOWN_CLK;

	plat->fix_div = dev_read_u32_default(dev, "fixed-divider", 1);

	ret = dev_read_u32_array(dev, "div-reg", divreg, ARRAY_SIZE(divreg));
	if (!ret) {
		plat->div_reg = divreg[0];
		plat->div_len = divreg[2];
		plat->div_off = divreg[1];
	}

	ret = dev_read_u32_array(dev, "clk-gate", gatereg, ARRAY_SIZE(gatereg));
	if (!ret) {
		plat->gate_reg = gatereg[0];
		plat->gate_bit = gatereg[1];
	}

	return 0;
}

static const struct udevice_id socfpga_a10_clk_match[] = {
	{ .compatible = "altr,clk-mgr" },
	{}
};

U_BOOT_DRIVER(socfpga_a10_clk) = {
	.name		= "clk-a10",
	.id		= UCLASS_CLK,
	.of_match	= socfpga_a10_clk_match,
	.ops		= &socfpga_a10_clk_ops,
	.bind		= socfpga_a10_clk_bind,
	.probe		= socfpga_a10_clk_probe,
	.ofdata_to_platdata = socfpga_a10_ofdata_to_platdata,

	.platdata_auto_alloc_size = sizeof(struct socfpga_a10_clk_platdata),
};
