// SPDX-License-Identifier: GPL-2.0+
/*
 * Designware APB Timer driver
 *
 * Copyright (C) 2018 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <clk.h>
#include <dt-structs.h>
#include <malloc.h>
#include <reset.h>
#include <timer.h>
#include <dm/device_compat.h>

#include <asm/io.h>
#include <asm/arch/timer.h>

#define DW_APB_LOAD_VAL		0x0
#define DW_APB_CURR_VAL		0x4
#define DW_APB_CTRL		0x8

struct dw_apb_timer_priv {
	uintptr_t regs;
	struct reset_ctl_bulk resets;
};

struct dw_apb_timer_plat {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_snps_dw_apb_timer dtplat;
#endif
};

static u64 dw_apb_timer_get_count(struct udevice *dev)
{
	struct dw_apb_timer_priv *priv = dev_get_priv(dev);

	/*
	 * The DW APB counter counts down, but this function
	 * requires the count to be incrementing. Invert the
	 * result.
	 */
	return timer_conv_64(~readl(priv->regs + DW_APB_CURR_VAL));
}

static int dw_apb_timer_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct dw_apb_timer_priv *priv = dev_get_priv(dev);
	struct clk clk;
	int ret;
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dw_apb_timer_plat *plat = dev_get_plat(dev);
	struct dtd_snps_dw_apb_timer *dtplat = &plat->dtplat;

	priv->regs = dtplat->reg[0];

	ret = clk_get_by_phandle(dev, &dtplat->clocks[0], &clk);
	if (ret < 0)
		return ret;

	uc_priv->clock_rate = dtplat->clock_frequency;
#endif
	if (CONFIG_IS_ENABLED(OF_REAL)) {
		ret = reset_get_bulk(dev, &priv->resets);
		if (ret)
			dev_warn(dev, "Can't get reset: %d\n", ret);
		else
			reset_deassert_bulk(&priv->resets);

		ret = clk_get_by_index(dev, 0, &clk);
		if (ret)
			return ret;

		uc_priv->clock_rate = clk_get_rate(&clk);
	}

	/* init timer */
	writel(0xffffffff, priv->regs + DW_APB_LOAD_VAL);
	writel(0xffffffff, priv->regs + DW_APB_CURR_VAL);
	setbits_le32(priv->regs + DW_APB_CTRL, 0x3);

	return 0;
}

static int dw_apb_timer_of_to_plat(struct udevice *dev)
{
	if (CONFIG_IS_ENABLED(OF_REAL)) {
		struct dw_apb_timer_priv *priv = dev_get_priv(dev);

		priv->regs = dev_read_addr(dev);
	}

	return 0;
}

static int dw_apb_timer_remove(struct udevice *dev)
{
	struct dw_apb_timer_priv *priv = dev_get_priv(dev);

	return reset_release_bulk(&priv->resets);
}

static const struct timer_ops dw_apb_timer_ops = {
	.get_count	= dw_apb_timer_get_count,
};

static const struct udevice_id dw_apb_timer_ids[] = {
	{ .compatible = "snps,dw-apb-timer" },
	{}
};

U_BOOT_DRIVER(snps_dw_apb_timer) = {
	.name		= "snps_dw_apb_timer",
	.id		= UCLASS_TIMER,
	.ops		= &dw_apb_timer_ops,
	.probe		= dw_apb_timer_probe,
	.of_match	= dw_apb_timer_ids,
	.of_to_plat	= dw_apb_timer_of_to_plat,
	.remove		= dw_apb_timer_remove,
	.priv_auto	= sizeof(struct dw_apb_timer_priv),
	.plat_auto	= sizeof(struct dw_apb_timer_plat),
};
