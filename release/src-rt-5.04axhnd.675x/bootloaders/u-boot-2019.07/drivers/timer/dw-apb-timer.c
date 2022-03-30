// SPDX-License-Identifier: GPL-2.0+
/*
 * Designware APB Timer driver
 *
 * Copyright (C) 2018 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <clk.h>
#include <timer.h>

#include <asm/io.h>
#include <asm/arch/timer.h>

#define DW_APB_LOAD_VAL		0x0
#define DW_APB_CURR_VAL		0x4
#define DW_APB_CTRL		0x8

struct dw_apb_timer_priv {
	fdt_addr_t	regs;
};

static int dw_apb_timer_get_count(struct udevice *dev, u64 *count)
{
	struct dw_apb_timer_priv *priv = dev_get_priv(dev);

	/*
	 * The DW APB counter counts down, but this function
	 * requires the count to be incrementing. Invert the
	 * result.
	 */
	*count = timer_conv_64(~readl(priv->regs + DW_APB_CURR_VAL));

	return 0;
}

static int dw_apb_timer_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct dw_apb_timer_priv *priv = dev_get_priv(dev);
	struct clk clk;
	int ret;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	uc_priv->clock_rate = clk_get_rate(&clk);

	clk_free(&clk);

	/* init timer */
	writel(0xffffffff, priv->regs + DW_APB_LOAD_VAL);
	writel(0xffffffff, priv->regs + DW_APB_CURR_VAL);
	setbits_le32(priv->regs + DW_APB_CTRL, 0x3);

	return 0;
}

static int dw_apb_timer_ofdata_to_platdata(struct udevice *dev)
{
	struct dw_apb_timer_priv *priv = dev_get_priv(dev);

	priv->regs = dev_read_addr(dev);

	return 0;
}

static const struct timer_ops dw_apb_timer_ops = {
	.get_count	= dw_apb_timer_get_count,
};

static const struct udevice_id dw_apb_timer_ids[] = {
	{ .compatible = "snps,dw-apb-timer" },
	{}
};

U_BOOT_DRIVER(dw_apb_timer) = {
	.name		= "dw_apb_timer",
	.id		= UCLASS_TIMER,
	.ops		= &dw_apb_timer_ops,
	.probe		= dw_apb_timer_probe,
	.of_match	= dw_apb_timer_ids,
	.ofdata_to_platdata = dw_apb_timer_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct dw_apb_timer_priv),
};
