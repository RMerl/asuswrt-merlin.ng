// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Cortina-Access
 *
 */

#include <common.h>
#include <dm.h>
#include <hang.h>
#include <asm/io.h>
#include <wdt.h>
#include <linux/bitops.h>

#define CA_WDT_CTRL		0x00
#define CA_WDT_PS		0x04
#define CA_WDT_DIV		0x08
#define CA_WDT_LD		0x0C
#define CA_WDT_LOADE		0x10
#define CA_WDT_CNT		0x14
#define CA_WDT_IE		0x18
#define CA_WDT_INT		0x1C
#define CA_WDT_STAT		0x20

/* CA_WDT_CTRL */
#define CTL_WDT_EN		BIT(0)
#define CTL_WDT_RSTEN		BIT(1)
#define CTL_WDT_CLK_SEL		BIT(2)
/* CA_WDT_LOADE */
#define WDT_UPD			BIT(0)
#define WDT_UPD_PS		BIT(1)

/* Global config */
#define WDT_RESET_SUB		BIT(4)
#define WDT_RESET_ALL_BLOCK	BIT(6)
#define WDT_RESET_REMAP		BIT(7)
#define WDT_EXT_RESET		BIT(8)
#define WDT_RESET_DEFAULT	(WDT_EXT_RESET | WDT_RESET_REMAP | \
				 WDT_RESET_ALL_BLOCK | WDT_RESET_SUB)

struct ca_wdt_priv {
	void __iomem *base;
	void __iomem *global_config;
};

static void cortina_wdt_set_timeout(struct udevice *dev, u64 timeout_ms)
{
	struct ca_wdt_priv *priv = dev_get_priv(dev);

	/* Prescale using millisecond unit */
	writel(CORTINA_PER_IO_FREQ / 1000, priv->base + CA_WDT_PS);

	/* Millisecond */
	writel(1, priv->base + CA_WDT_DIV);

	writel(timeout_ms, priv->base + CA_WDT_LD);
	writel(WDT_UPD | WDT_UPD_PS, priv->base + CA_WDT_LOADE);
}

static int cortina_wdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	struct ca_wdt_priv *priv = dev_get_priv(dev);

	cortina_wdt_set_timeout(dev, timeout);

	/* WDT Reset option */
	setbits_32(priv->global_config, WDT_RESET_DEFAULT);

	/* Enable WDT */
	setbits_32(priv->base, CTL_WDT_EN | CTL_WDT_RSTEN | CTL_WDT_CLK_SEL);

	return 0;
}

static int cortina_wdt_stop(struct udevice *dev)
{
	struct ca_wdt_priv *priv = dev_get_priv(dev);

	/* Disable WDT */
	writel(0, priv->base);

	return 0;
}

static int cortina_wdt_reset(struct udevice *dev)
{
	struct ca_wdt_priv *priv = dev_get_priv(dev);

	/* Reload WDT counter */
	writel(WDT_UPD, priv->base + CA_WDT_LOADE);

	return 0;
}

static int cortina_wdt_expire_now(struct udevice *dev, ulong flags)
{
	/* Set 1ms timeout to reset system */
	cortina_wdt_set_timeout(dev, 1);
	hang();

	return 0;
}

static int cortina_wdt_probe(struct udevice *dev)
{
	struct ca_wdt_priv *priv = dev_get_priv(dev);

	priv->base = dev_remap_addr_index(dev, 0);
	if (!priv->base)
		return -ENOENT;

	priv->global_config = dev_remap_addr_index(dev, 1);
	if (!priv->global_config)
		return -ENOENT;

	/* Stop WDT */
	cortina_wdt_stop(dev);

	return 0;
}

static const struct wdt_ops cortina_wdt_ops = {
	.start = cortina_wdt_start,
	.reset = cortina_wdt_reset,
	.stop = cortina_wdt_stop,
	.expire_now = cortina_wdt_expire_now,
};

static const struct udevice_id cortina_wdt_ids[] = {
	{.compatible = "cortina,ca-wdt"},
	{}
};

U_BOOT_DRIVER(cortina_wdt) = {
	.name = "cortina_wdt",
	.id = UCLASS_WDT,
	.probe = cortina_wdt_probe,
	.of_match = cortina_wdt_ids,
	.ops = &cortina_wdt_ops,
};
