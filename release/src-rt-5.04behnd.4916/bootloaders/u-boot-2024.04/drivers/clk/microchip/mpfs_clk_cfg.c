// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Microchip Technology Inc.
 * Padmarao Begari <padmarao.begari@microchip.com>
 */
#include <common.h>
#include <clk.h>
#include <clk-uclass.h>
#include <asm/io.h>
#include <dm/device.h>
#include <dm/devres.h>
#include <dm/uclass.h>
#include <dt-bindings/clock/microchip-mpfs-clock.h>
#include <linux/err.h>

#include "mpfs_clk.h"

#define MPFS_CFG_CLOCK "mpfs_cfg_clock"

#define REG_CLOCK_CONFIG_CR 0x08

/* CPU and AXI clock divisors */
static const struct clk_div_table mpfs_div_cpu_axi_table[] = {
	{ 0, 1 }, { 1, 2 }, { 2, 4 }, { 3, 8 },
	{ 0, 0 }
};

/* AHB clock divisors */
static const struct clk_div_table mpfs_div_ahb_table[] = {
	{ 1, 2 }, { 2, 4}, { 3, 8 },
	{ 0, 0 }
};

/**
 * struct mpfs_cfg_clock - per instance of configuration clock
 * @id: index of a configuration clock
 * @name: name of a configuration clock
 * @shift: shift to the divider bit field of a configuration clock
 * @width: width of the divider bit field of a configation clock
 * @table: clock divider table instance
 * @flags: common clock framework flags
 */
struct mpfs_cfg_clock {
	unsigned int id;
	const char *name;
	u8 shift;
	u8 width;
	const struct clk_div_table *table;
	unsigned long flags;
};

/**
 * struct mpfs_cfg_hw_clock - hardware configuration clock (cpu, axi, ahb)
 * @cfg: configuration clock instance
 * @sys_base: base address of the mpfs system register
 * @prate: the pll clock rate
 * @hw: clock instance
 */
struct mpfs_cfg_hw_clock {
	struct mpfs_cfg_clock cfg;
	void __iomem *sys_base;
	u32 prate;
	struct clk hw;
};

#define to_mpfs_cfg_clk(_hw) container_of(_hw, struct mpfs_cfg_hw_clock, hw)

static ulong mpfs_cfg_clk_recalc_rate(struct clk *hw)
{
	struct mpfs_cfg_hw_clock *cfg_hw = to_mpfs_cfg_clk(hw);
	struct mpfs_cfg_clock *cfg = &cfg_hw->cfg;
	void __iomem *base_addr = cfg_hw->sys_base;
	unsigned long rate;
	u32 val;

	val = readl(base_addr + REG_CLOCK_CONFIG_CR) >> cfg->shift;
	val &= clk_div_mask(cfg->width);
	rate = cfg_hw->prate / (1u << val);
	hw->rate = rate;

	return rate;
}

static ulong mpfs_cfg_clk_set_rate(struct clk *hw, ulong rate)
{
	struct mpfs_cfg_hw_clock *cfg_hw = to_mpfs_cfg_clk(hw);
	struct mpfs_cfg_clock *cfg = &cfg_hw->cfg;
	void __iomem *base_addr = cfg_hw->sys_base;
	u32  val;
	int divider_setting;

	divider_setting = divider_get_val(rate, cfg_hw->prate, cfg->table, cfg->width, cfg->flags);

	if (divider_setting < 0)
		return divider_setting;

	val = readl(base_addr + REG_CLOCK_CONFIG_CR);
	val &= ~(clk_div_mask(cfg->width) << cfg_hw->cfg.shift);
	val |= divider_setting << cfg->shift;
	writel(val, base_addr + REG_CLOCK_CONFIG_CR);

	return clk_get_rate(hw);
}

#define CLK_CFG(_id, _name, _shift, _width, _table, _flags) {	\
		.cfg.id = _id,					\
		.cfg.name = _name,				\
		.cfg.shift = _shift,				\
		.cfg.width = _width,				\
		.cfg.table = _table,				\
		.cfg.flags = _flags,				\
	}

static struct mpfs_cfg_hw_clock mpfs_cfg_clks[] = {
	CLK_CFG(CLK_CPU, "clk_cpu", 0, 2, mpfs_div_cpu_axi_table, 0),
	CLK_CFG(CLK_AXI, "clk_axi", 2, 2, mpfs_div_cpu_axi_table, 0),
	CLK_CFG(CLK_AHB, "clk_ahb", 4, 2, mpfs_div_ahb_table, 0),
};

int mpfs_clk_register_cfgs(void __iomem *base, struct clk *parent)
{
	int ret;
	int i, id, num_clks;
	const char *name;
	struct clk *hw;

	num_clks = ARRAY_SIZE(mpfs_cfg_clks);
	for (i = 0; i < num_clks; i++) {
		hw = &mpfs_cfg_clks[i].hw;
		mpfs_cfg_clks[i].sys_base = base;
		mpfs_cfg_clks[i].prate = clk_get_rate(parent);
		name = mpfs_cfg_clks[i].cfg.name;
		ret = clk_register(hw, MPFS_CFG_CLOCK, name, parent->dev->name);
		if (ret)
			ERR_PTR(ret);
		id = mpfs_cfg_clks[i].cfg.id;
		clk_dm(id, hw);
	}
	return 0;
}

const struct clk_ops mpfs_cfg_clk_ops = {
	.set_rate = mpfs_cfg_clk_set_rate,
	.get_rate = mpfs_cfg_clk_recalc_rate,
};

U_BOOT_DRIVER(mpfs_cfg_clock) = {
	.name	= MPFS_CFG_CLOCK,
	.id	= UCLASS_CLK,
	.ops	= &mpfs_cfg_clk_ops,
};
