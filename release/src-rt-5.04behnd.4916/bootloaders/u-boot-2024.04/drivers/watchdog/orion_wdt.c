/*
 * drivers/watchdog/orion_wdt.c
 *
 * Watchdog driver for Orion/Kirkwood processors
 *
 * Authors:	Tomas Hlavacek <tmshlvck@gmail.com>
 *		Sylver Bruneau <sylver.bruneau@googlemail.com>
 *		Marek Behún <kabel@kernel.org>
 *
 * This file is licensed under  the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <common.h>
#include <dm.h>
#include <clk.h>
#include <log.h>
#include <wdt.h>
#include <asm/global_data.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

DECLARE_GLOBAL_DATA_PTR;

struct orion_wdt_priv {
	void __iomem *reg;
	int wdt_counter_offset;
	void __iomem *rstout;
	void __iomem *rstout_mask;
	u32 timeout;
	unsigned long clk_rate;
	struct clk clk;
};

#define RSTOUT_ENABLE_BIT		BIT(8)
#define RSTOUT_MASK_BIT			BIT(10)
#define WDT_ENABLE_BIT			BIT(8)

#define TIMER_CTRL			0x0000
#define TIMER_A370_STATUS		0x04

#define WDT_AXP_FIXED_ENABLE_BIT	BIT(10)
#define WDT_A370_EXPIRED		BIT(31)

static int orion_wdt_reset(struct udevice *dev)
{
	struct orion_wdt_priv *priv = dev_get_priv(dev);

	/* Reload watchdog duration */
	writel(priv->clk_rate * priv->timeout,
	       priv->reg + priv->wdt_counter_offset);

	return 0;
}

static int orion_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	struct orion_wdt_priv *priv = dev_get_priv(dev);
	u32 reg;

	priv->timeout = DIV_ROUND_UP(timeout_ms, 1000);

	/* Enable the fixed watchdog clock input */
	reg = readl(priv->reg + TIMER_CTRL);
	reg |= WDT_AXP_FIXED_ENABLE_BIT;
	writel(reg, priv->reg + TIMER_CTRL);

	/* Set watchdog duration */
	writel(priv->clk_rate * priv->timeout,
	       priv->reg + priv->wdt_counter_offset);

	/* Clear the watchdog expiration bit */
	reg = readl(priv->reg + TIMER_A370_STATUS);
	reg &= ~WDT_A370_EXPIRED;
	writel(reg, priv->reg + TIMER_A370_STATUS);

	/* Enable watchdog timer */
	reg = readl(priv->reg + TIMER_CTRL);
	reg |= WDT_ENABLE_BIT;
	writel(reg, priv->reg + TIMER_CTRL);

	/* Enable reset on watchdog */
	reg = readl(priv->rstout);
	reg |= RSTOUT_ENABLE_BIT;
	writel(reg, priv->rstout);

	reg = readl(priv->rstout_mask);
	reg &= ~RSTOUT_MASK_BIT;
	writel(reg, priv->rstout_mask);

	return 0;
}

static int orion_wdt_stop(struct udevice *dev)
{
	struct orion_wdt_priv *priv = dev_get_priv(dev);
	u32 reg;

	/* Disable reset on watchdog */
	reg = readl(priv->rstout_mask);
	reg |= RSTOUT_MASK_BIT;
	writel(reg, priv->rstout_mask);

	reg = readl(priv->rstout);
	reg &= ~RSTOUT_ENABLE_BIT;
	writel(reg, priv->rstout);

	/* Disable watchdog timer */
	reg = readl(priv->reg + TIMER_CTRL);
	reg &= ~WDT_ENABLE_BIT;
	writel(reg, priv->reg + TIMER_CTRL);

	return 0;
}

static inline bool save_reg_from_ofdata(struct udevice *dev, int index,
					void __iomem **reg, int *offset)
{
	fdt_addr_t addr;
	fdt_size_t off;

	addr = devfdt_get_addr_size_index(dev, index, &off);
	if (addr == FDT_ADDR_T_NONE)
		return false;

	*reg = (void __iomem *) addr;
	if (offset)
		*offset = off;

	return true;
}

static int orion_wdt_of_to_plat(struct udevice *dev)
{
	struct orion_wdt_priv *priv = dev_get_priv(dev);

	if (!save_reg_from_ofdata(dev, 0, &priv->reg,
				  &priv->wdt_counter_offset))
		goto err;

	if (!save_reg_from_ofdata(dev, 1, &priv->rstout, NULL))
		goto err;

	if (!save_reg_from_ofdata(dev, 2, &priv->rstout_mask, NULL))
		goto err;

	return 0;
err:
	debug("%s: Could not determine Orion wdt IO addresses\n", __func__);
	return -ENXIO;
}

static int orion_wdt_probe(struct udevice *dev)
{
	struct orion_wdt_priv *priv = dev_get_priv(dev);
	int ret;

	debug("%s: Probing wdt%u\n", __func__, dev_seq(dev));
	orion_wdt_stop(dev);

	ret = clk_get_by_name(dev, "fixed", &priv->clk);
	if (!ret)
		priv->clk_rate = clk_get_rate(&priv->clk);
	else
		priv->clk_rate = 25000000;

	return 0;
}

static const struct wdt_ops orion_wdt_ops = {
	.start = orion_wdt_start,
	.reset = orion_wdt_reset,
	.stop = orion_wdt_stop,
};

static const struct udevice_id orion_wdt_ids[] = {
	{ .compatible = "marvell,armada-380-wdt" },
	{}
};

U_BOOT_DRIVER(orion_wdt) = {
	.name = "orion_wdt",
	.id = UCLASS_WDT,
	.of_match = orion_wdt_ids,
	.probe = orion_wdt_probe,
	.priv_auto	= sizeof(struct orion_wdt_priv),
	.of_to_plat = orion_wdt_of_to_plat,
	.ops = &orion_wdt_ops,
};
