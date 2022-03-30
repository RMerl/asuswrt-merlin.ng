// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 *
 * Derived from linux/arch/mips/bcm63xx/clk.c:
 *	Copyright (C) 2008 Maxime Bizon <mbizon@freebox.fr>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>

#define MAX_CLKS	32

struct bcm6345_clk_priv {
	void __iomem *regs;
};

static int bcm6345_clk_enable(struct clk *clk)
{
	struct bcm6345_clk_priv *priv = dev_get_priv(clk->dev);

	if (clk->id >= MAX_CLKS)
		return -EINVAL;

	setbits_be32(priv->regs, BIT(clk->id));

	return 0;
}

static int bcm6345_clk_disable(struct clk *clk)
{
	struct bcm6345_clk_priv *priv = dev_get_priv(clk->dev);

	if (clk->id >= MAX_CLKS)
		return -EINVAL;

	clrbits_be32(priv->regs, BIT(clk->id));

	return 0;
}

static struct clk_ops bcm6345_clk_ops = {
	.disable = bcm6345_clk_disable,
	.enable = bcm6345_clk_enable,
};

static const struct udevice_id bcm6345_clk_ids[] = {
	{ .compatible = "brcm,bcm6345-clk" },
	{ /* sentinel */ }
};

static int bcm63xx_clk_probe(struct udevice *dev)
{
	struct bcm6345_clk_priv *priv = dev_get_priv(dev);

	priv->regs = dev_remap_addr(dev);
	if (!priv->regs)
		return -EINVAL;

	return 0;
}

U_BOOT_DRIVER(clk_bcm6345) = {
	.name = "clk_bcm6345",
	.id = UCLASS_CLK,
	.of_match = bcm6345_clk_ids,
	.ops = &bcm6345_clk_ops,
	.probe = bcm63xx_clk_probe,
	.priv_auto_alloc_size = sizeof(struct bcm6345_clk_priv),
};
