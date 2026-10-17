// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Microchip Technology Inc.
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

#define MPFS_MSSPLL_CLOCK "mpfs_msspll_clock"

/* address offset of control registers */
#define REG_MSSPLL_REF_CR	0x08u
#define REG_MSSPLL_POSTDIV_CR	0x10u
#define REG_MSSPLL_SSCG_2_CR	0x2Cu

#define MSSPLL_FBDIV_SHIFT	0x00u
#define MSSPLL_FBDIV_WIDTH	0x0Cu
#define MSSPLL_REFDIV_SHIFT	0x08u
#define MSSPLL_REFDIV_WIDTH	0x06u
#define MSSPLL_POSTDIV_SHIFT	0x08u
#define MSSPLL_POSTDIV_WIDTH	0x07u
#define MSSPLL_FIXED_DIV	4u

/**
 * struct mpfs_msspll_hw_clock
 * @id: index of the msspll clock
 * @name: the msspll clocks name
 * @reg_offset: offset to the core complex's output of the msspll
 * @shift: shift to the divider bit field of a msspll clock output
 * @width: width of the divider bit field of the msspll clock output
 * @flags: common clock framework flags
 * @prate: the reference clock rate
 * @hw: clock instance
 */
struct mpfs_msspll_hw_clock {
	void __iomem *base;
	unsigned int id;
	const char *name;
	u32 reg_offset;
	u32 shift;
	u32 width;
	u32 flags;
	u32 prate;
	struct clk hw;
};

#define to_mpfs_msspll_clk(_hw) container_of(_hw, struct mpfs_msspll_hw_clock, hw)

static unsigned long mpfs_clk_msspll_recalc_rate(struct clk *hw)
{
	struct mpfs_msspll_hw_clock *msspll_hw = to_mpfs_msspll_clk(hw);
	void __iomem *mult_addr = msspll_hw->base + msspll_hw->reg_offset;
	void __iomem *ref_div_addr = msspll_hw->base + REG_MSSPLL_REF_CR;
	void __iomem *postdiv_addr = msspll_hw->base + REG_MSSPLL_POSTDIV_CR;
	u32 mult, ref_div, postdiv;
	unsigned long temp;

	mult = readl(mult_addr) >> MSSPLL_FBDIV_SHIFT;
	mult &= clk_div_mask(MSSPLL_FBDIV_WIDTH);
	ref_div = readl(ref_div_addr) >> MSSPLL_REFDIV_SHIFT;
	ref_div &= clk_div_mask(MSSPLL_REFDIV_WIDTH);
	postdiv = readl(postdiv_addr) >> MSSPLL_POSTDIV_SHIFT;
	postdiv &= clk_div_mask(MSSPLL_POSTDIV_WIDTH);

	temp = msspll_hw->prate / (ref_div * MSSPLL_FIXED_DIV * postdiv);
	return temp * mult;
}

#define CLK_PLL(_id, _name, _shift, _width, _reg_offset, _flags) {	\
	.id = _id,							\
	.name = _name,							\
	.shift = _shift,						\
	.width = _width,						\
	.reg_offset = _reg_offset,					\
	.flags = _flags,						\
}

static struct mpfs_msspll_hw_clock mpfs_msspll_clks[] = {
	CLK_PLL(CLK_MSSPLL, "clk_msspll", MSSPLL_FBDIV_SHIFT,
		MSSPLL_FBDIV_WIDTH, REG_MSSPLL_SSCG_2_CR, 0),
};

int mpfs_clk_register_msspll(void __iomem *base, struct clk *parent)
{
	int id, ret;
	const char *name;
	struct clk *hw;

	hw = &mpfs_msspll_clks[0].hw;
	mpfs_msspll_clks[0].base = base;
	mpfs_msspll_clks[0].prate = clk_get_rate(parent);
	name = mpfs_msspll_clks[0].name;
	ret = clk_register(hw, MPFS_MSSPLL_CLOCK, name, parent->dev->name);
	if (ret)
		ERR_PTR(ret);
	id = mpfs_msspll_clks[0].id;
	clk_dm(id, hw);

	return 0;
}

const struct clk_ops mpfs_msspll_clk_ops = {
	.get_rate = mpfs_clk_msspll_recalc_rate,
};

U_BOOT_DRIVER(mpfs_msspll_clock) = {
	.name	= MPFS_MSSPLL_CLOCK,
	.id	= UCLASS_CLK,
	.ops	= &mpfs_msspll_clk_ops,
};

