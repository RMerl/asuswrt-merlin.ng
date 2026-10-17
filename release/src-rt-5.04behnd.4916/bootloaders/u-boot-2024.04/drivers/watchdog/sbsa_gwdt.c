// SPDX-License-Identifier: GPL-2.0+
/*
 * Watchdog driver for SBSA
 *
 * Copyright 2020 NXP
 */

#include <asm/global_data.h>
#include <asm/io.h>
#include <common.h>
#include <dm/device.h>
#include <dm/fdtaddr.h>
#include <dm/read.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <watchdog.h>
#include <wdt.h>

DECLARE_GLOBAL_DATA_PTR;

/* SBSA Generic Watchdog register definitions */
/* refresh frame */
#define SBSA_GWDT_WRR		0x000

/* control frame */
#define SBSA_GWDT_WCS		0x000
#define SBSA_GWDT_WOR		0x008
#define SBSA_GWDT_WCV		0x010

/* refresh/control frame */
#define SBSA_GWDT_W_IIDR	0xfcc
#define SBSA_GWDT_IDR		0xfd0

/* Watchdog Control and Status Register */
#define SBSA_GWDT_WCS_EN	BIT(0)
#define SBSA_GWDT_WCS_WS0	BIT(1)
#define SBSA_GWDT_WCS_WS1	BIT(2)

struct sbsa_gwdt_priv {
	void __iomem *reg_refresh;
	void __iomem *reg_control;
};

static int sbsa_gwdt_reset(struct udevice *dev)
{
	struct sbsa_gwdt_priv *priv = dev_get_priv(dev);

	writel(0, priv->reg_refresh + SBSA_GWDT_WRR);

	return 0;
}

static int sbsa_gwdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	struct sbsa_gwdt_priv *priv = dev_get_priv(dev);
	u32 clk;

	/*
	 * it work in the single stage mode in u-boot,
	 * The first signal (WS0) is ignored,
	 * the timeout is (WOR * 2), so the WOR should be configured
	 * to half value of timeout.
	 */
	clk = get_tbclk();
	writel(clk / (2 * 1000) * timeout,
	       priv->reg_control + SBSA_GWDT_WOR);

	/* writing WCS will cause an explicit watchdog refresh */
	writel(SBSA_GWDT_WCS_EN, priv->reg_control + SBSA_GWDT_WCS);

	return 0;
}

static int sbsa_gwdt_stop(struct udevice *dev)
{
	struct sbsa_gwdt_priv *priv = dev_get_priv(dev);

	writel(0, priv->reg_control + SBSA_GWDT_WCS);

	return 0;
}

static int sbsa_gwdt_expire_now(struct udevice *dev, ulong flags)
{
	sbsa_gwdt_start(dev, 0, flags);

	return 0;
}

static int sbsa_gwdt_probe(struct udevice *dev)
{
	debug("%s: Probing wdt%u (sbsa-gwdt)\n", __func__, dev_seq(dev));

	return 0;
}

static int sbsa_gwdt_of_to_plat(struct udevice *dev)
{
	struct sbsa_gwdt_priv *priv = dev_get_priv(dev);

	priv->reg_control = dev_read_addr_index_ptr(dev, 0);
	if (!priv->reg_control)
		return -EINVAL;

	priv->reg_refresh = dev_read_addr_index_ptr(dev, 1);
	if (!priv->reg_refresh)
		return -EINVAL;

	return 0;
}

static const struct wdt_ops sbsa_gwdt_ops = {
	.start = sbsa_gwdt_start,
	.reset = sbsa_gwdt_reset,
	.stop = sbsa_gwdt_stop,
	.expire_now = sbsa_gwdt_expire_now,
};

static const struct udevice_id sbsa_gwdt_ids[] = {
	{ .compatible = "arm,sbsa-gwdt" },
	{}
};

U_BOOT_DRIVER(sbsa_gwdt) = {
	.name = "sbsa_gwdt",
	.id = UCLASS_WDT,
	.of_match = sbsa_gwdt_ids,
	.probe = sbsa_gwdt_probe,
	.priv_auto	= sizeof(struct sbsa_gwdt_priv),
	.of_to_plat = sbsa_gwdt_of_to_plat,
	.ops = &sbsa_gwdt_ops,
};
