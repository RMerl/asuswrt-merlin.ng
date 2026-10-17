// SPDX-License-Identifier: GPL-2.0+
/*
 * TI clock utilities
 *
 * Copyright (C) 2020 Dario Binacchi <dariobin@libero.it>
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <regmap.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include "clk.h"

#define CLK_MAX_MEMMAPS           10

struct clk_iomap {
	struct regmap *regmap;
	ofnode node;
};

static unsigned int clk_memmaps_num;
static struct clk_iomap clk_memmaps[CLK_MAX_MEMMAPS];

static void clk_ti_rmw(u32 val, u32 mask, struct clk_ti_reg *reg)
{
	u32 v;

	v = clk_ti_readl(reg);
	v &= ~mask;
	v |= val;
	clk_ti_writel(v, reg);
}

void clk_ti_latch(struct clk_ti_reg *reg, s8 shift)
{
	u32 latch;

	if (shift < 0)
		return;

	latch = 1 << shift;

	clk_ti_rmw(latch, latch, reg);
	clk_ti_rmw(0, latch, reg);
	clk_ti_readl(reg);		/* OCP barrier */
}

void clk_ti_writel(u32 val, struct clk_ti_reg *reg)
{
	struct clk_iomap *io = &clk_memmaps[reg->index];

	regmap_write(io->regmap, reg->offset, val);
}

u32 clk_ti_readl(struct clk_ti_reg *reg)
{
	struct clk_iomap *io = &clk_memmaps[reg->index];
	u32 val;

	regmap_read(io->regmap, reg->offset, &val);
	return val;
}

static ofnode clk_ti_get_regmap_node(struct udevice *dev)
{
	ofnode node = dev_ofnode(dev), parent;

	if (!ofnode_valid(node))
		return ofnode_null();

	parent = ofnode_get_parent(node);
	if (strcmp(ofnode_get_name(parent), "clocks"))
		return ofnode_null();

	return ofnode_get_parent(parent);
}

int clk_ti_get_reg_addr(struct udevice *dev, int index, struct clk_ti_reg *reg)
{
	ofnode node;
	int i, ret;
	u32 val;

	ret = ofnode_read_u32_index(dev_ofnode(dev), "reg", index, &val);
	if (ret) {
		dev_err(dev, "%s must have reg[%d]\n", ofnode_get_name(node),
			index);
		return ret;
	}

	/* parent = ofnode_get_parent(parent); */
	node = clk_ti_get_regmap_node(dev);
	if (!ofnode_valid(node)) {
		dev_err(dev, "failed to get regmap node\n");
		return -EFAULT;
	}

	for (i = 0; i < clk_memmaps_num; i++) {
		if (ofnode_equal(clk_memmaps[i].node, node))
			break;
	}

	if (i == clk_memmaps_num) {
		if (i == CLK_MAX_MEMMAPS)
			return -ENOMEM;

		ret = regmap_init_mem(node, &clk_memmaps[i].regmap);
		if (ret)
			return ret;

		clk_memmaps[i].node = node;
		clk_memmaps_num++;
	}

	reg->index = i;
	reg->offset = val;
	return 0;
}
