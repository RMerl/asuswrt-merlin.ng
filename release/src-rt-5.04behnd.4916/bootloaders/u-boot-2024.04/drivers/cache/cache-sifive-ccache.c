// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 SiFive
 */

#include <common.h>
#include <cache.h>
#include <dm.h>
#include <asm/io.h>
#include <dm/device.h>
#include <linux/bitfield.h>

#define SIFIVE_CCACHE_CONFIG		0x000
#define SIFIVE_CCACHE_CONFIG_WAYS	GENMASK(15, 8)

#define SIFIVE_CCACHE_WAY_ENABLE	0x008

struct sifive_ccache {
	void __iomem *base;
};

static int sifive_ccache_enable(struct udevice *dev)
{
	struct sifive_ccache *priv = dev_get_priv(dev);
	u32 config;
	u32 ways;

	/* Enable all ways of composable cache */
	config = readl(priv->base + SIFIVE_CCACHE_CONFIG);
	ways = FIELD_GET(SIFIVE_CCACHE_CONFIG_WAYS, config);

	writel(ways - 1, priv->base + SIFIVE_CCACHE_WAY_ENABLE);

	return 0;
}

static int sifive_ccache_get_info(struct udevice *dev, struct cache_info *info)
{
	struct sifive_ccache *priv = dev_get_priv(dev);

	info->base = (uintptr_t)priv->base;

	return 0;
}

static const struct cache_ops sifive_ccache_ops = {
	.enable = sifive_ccache_enable,
	.get_info = sifive_ccache_get_info,
};

static int sifive_ccache_probe(struct udevice *dev)
{
	struct sifive_ccache *priv = dev_get_priv(dev);

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;

	return 0;
}

static const struct udevice_id sifive_ccache_ids[] = {
	{ .compatible = "sifive,fu540-c000-ccache" },
	{ .compatible = "sifive,fu740-c000-ccache" },
	{ .compatible = "sifive,ccache0" },
	{}
};

U_BOOT_DRIVER(sifive_ccache) = {
	.name = "sifive_ccache",
	.id = UCLASS_CACHE,
	.of_match = sifive_ccache_ids,
	.probe = sifive_ccache_probe,
	.priv_auto = sizeof(struct sifive_ccache),
	.ops = &sifive_ccache_ops,
};
