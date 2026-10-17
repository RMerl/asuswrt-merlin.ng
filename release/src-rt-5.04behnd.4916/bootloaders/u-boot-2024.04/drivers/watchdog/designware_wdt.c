// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Altera Corporation <www.altera.com>
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <reset.h>
#include <wdt.h>
#include <asm/io.h>
#include <linux/bitops.h>

#define DW_WDT_CR	0x00
#define DW_WDT_TORR	0x04
#define DW_WDT_CRR	0x0C

#define DW_WDT_CR_EN_OFFSET	0x00
#define DW_WDT_CR_RMOD_OFFSET	0x01
#define DW_WDT_CRR_RESTART_VAL	0x76

struct designware_wdt_priv {
	void __iomem	*base;
	unsigned int	clk_khz;
	struct reset_ctl_bulk resets;
};

/*
 * Set the watchdog time interval.
 * Counter is 32 bit.
 */
static int designware_wdt_settimeout(void __iomem *base, unsigned int clk_khz,
				     unsigned int timeout)
{
	signed int i;

	/* calculate the timeout range value */
	i = fls(timeout * clk_khz - 1) - 16;
	i = clamp(i, 0, 15);

	writel(i | (i << 4), base + DW_WDT_TORR);

	return 0;
}

static void designware_wdt_enable(void __iomem *base)
{
	writel(BIT(DW_WDT_CR_EN_OFFSET), base + DW_WDT_CR);
}

static unsigned int designware_wdt_is_enabled(void __iomem *base)
{
	return readl(base + DW_WDT_CR) & BIT(0);
}

static void designware_wdt_reset_common(void __iomem *base)
{
	if (designware_wdt_is_enabled(base))
		/* restart the watchdog counter */
		writel(DW_WDT_CRR_RESTART_VAL, base + DW_WDT_CRR);
}

static int designware_wdt_reset(struct udevice *dev)
{
	struct designware_wdt_priv *priv = dev_get_priv(dev);

	designware_wdt_reset_common(priv->base);

	return 0;
}

static int designware_wdt_stop(struct udevice *dev)
{
	struct designware_wdt_priv *priv = dev_get_priv(dev);
	__maybe_unused int ret;

	designware_wdt_reset(dev);
	writel(0, priv->base + DW_WDT_CR);

	if (CONFIG_IS_ENABLED(DM_RESET) &&
	    ofnode_read_prop(dev_ofnode(dev), "resets", &ret)) {
		ret = reset_assert_bulk(&priv->resets);
		if (ret)
			return ret;

		ret = reset_deassert_bulk(&priv->resets);
		if (ret)
			return ret;
	}

	return 0;
}

static int designware_wdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	struct designware_wdt_priv *priv = dev_get_priv(dev);

	designware_wdt_stop(dev);

	/* set timer in miliseconds */
	designware_wdt_settimeout(priv->base, priv->clk_khz, timeout);

	designware_wdt_enable(priv->base);

	/* reset the watchdog */
	return designware_wdt_reset(dev);
}

static int designware_wdt_probe(struct udevice *dev)
{
	struct designware_wdt_priv *priv = dev_get_priv(dev);
	__maybe_unused int ret;

	priv->base = dev_remap_addr(dev);
	if (!priv->base)
		return -EINVAL;

#if CONFIG_IS_ENABLED(CLK)
	struct clk clk;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	ret = clk_enable(&clk);
	if (ret)
		return ret;

	priv->clk_khz = clk_get_rate(&clk) / 1000;
	if (!priv->clk_khz)
		return -EINVAL;
#else
	priv->clk_khz = CFG_DW_WDT_CLOCK_KHZ;
#endif

	if (CONFIG_IS_ENABLED(DM_RESET) &&
	    ofnode_read_prop(dev_ofnode(dev), "resets", &ret)) {
		ret = reset_get_bulk(dev, &priv->resets);
		if (ret)
			return ret;

		ret = reset_deassert_bulk(&priv->resets);
		if (ret)
			return ret;
	}

	/* reset to disable the watchdog */
	return designware_wdt_stop(dev);
}

static const struct wdt_ops designware_wdt_ops = {
	.start = designware_wdt_start,
	.reset = designware_wdt_reset,
	.stop = designware_wdt_stop,
};

static const struct udevice_id designware_wdt_ids[] = {
	{ .compatible = "snps,dw-wdt"},
	{}
};

U_BOOT_DRIVER(designware_wdt) = {
	.name = "designware_wdt",
	.id = UCLASS_WDT,
	.of_match = designware_wdt_ids,
	.priv_auto	= sizeof(struct designware_wdt_priv),
	.probe = designware_wdt_probe,
	.ops = &designware_wdt_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
