/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <dm.h>
#include <timer.h>
#include <clk.h>
#include <asm/io.h>
#include <linux/ioport.h>
#include <linux/io.h>

// periph timer registers
#define TIMER_CTL 0
#define TIMER_CNT 0x10

#define BCM6XXX_TIMER_ENABLE		BIT(31)
#define BCM6XXX_TIMER_CNT_MASK		0x3fffffff
#define BCM6XXX_TIMER_CNT_HIGH_SHIFT	30

struct bcm6xxx_timer_priv {
	void __iomem *reg_base;
  	u64 cnt_high;
  	u32 cnt_low;
};

static int bcm6xxx_timer_get_count(struct udevice *dev, u64 *count)
{
	struct bcm6xxx_timer_priv *priv = dev_get_priv(dev);
	u32 val = readl(priv->reg_base + TIMER_CNT) & BCM6XXX_TIMER_CNT_MASK;

	/* hardware counter overflow */
	if (val < priv->cnt_low)
		priv->cnt_high++;
	priv->cnt_low = val;
	
	*count = priv->cnt_high<<BCM6XXX_TIMER_CNT_HIGH_SHIFT | 
			priv->cnt_low;

	return 0;
}

static void bcm6xxx_timer_start(struct udevice *dev)
{
	struct bcm6xxx_timer_priv *priv = dev_get_priv(dev);

	writel(BCM6XXX_TIMER_ENABLE | BCM6XXX_TIMER_CNT_MASK, priv->reg_base + TIMER_CTL);

	return;
}

static int bcm6xxx_timer_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct bcm6xxx_timer_priv *priv = dev_get_priv(dev);
	struct clk clk;
	struct resource res;
	int ret;

	priv->reg_base = dev_remap_addr(dev);
	printf("timer reg_base = %p\n", priv->reg_base);
	if (priv->reg_base == NULL)
		return -ENOENT;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	uc_priv->clock_rate = clk_get_rate(&clk);
	if (!uc_priv->clock_rate)
		return -EINVAL;

	priv->cnt_high = 0;
	priv->cnt_low = 0;
	bcm6xxx_timer_start(dev);

	return 0;
}

static const struct timer_ops bcm6xxx_timer_ops = {
	.get_count = bcm6xxx_timer_get_count,
};

static const struct udevice_id bcm6xxx_timer_ids[] = {
	{ .compatible = "brcm,bcm6xxx-timer" },
	{ }
};

U_BOOT_DRIVER(bcm6xxx_timer) = {
	.name = "bcm6xxx_timer",
	.id = UCLASS_TIMER,
	.of_match = bcm6xxx_timer_ids,
	.priv_auto_alloc_size = sizeof(struct bcm6xxx_timer_priv),
	.probe = bcm6xxx_timer_probe,
	.ops = &bcm6xxx_timer_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
