// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 *
 * Derived from linux/drivers/power/reset/syscon-reboot.c:
 *	Copyright (C) 2013, Applied Micro Circuits Corporation
 *	Author: Feng Kan <fkan@apm.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <regmap.h>
#include <sysreset.h>
#include <syscon.h>

struct syscon_reboot_priv {
	struct regmap *regmap;
	unsigned int offset;
	unsigned int mask;
};

static int syscon_reboot_request(struct udevice *dev, enum sysreset_t type)
{
	struct syscon_reboot_priv *priv = dev_get_priv(dev);

	if (type == SYSRESET_POWER)
		return -EPROTONOSUPPORT;

	regmap_write(priv->regmap, priv->offset, priv->mask);

	return -EINPROGRESS;
}

static struct sysreset_ops syscon_reboot_ops = {
	.request = syscon_reboot_request,
};

int syscon_reboot_probe(struct udevice *dev)
{
	struct syscon_reboot_priv *priv = dev_get_priv(dev);

	priv->regmap = syscon_regmap_lookup_by_phandle(dev, "regmap");
	if (IS_ERR(priv->regmap)) {
		pr_err("unable to find regmap\n");
		return -ENODEV;
	}

	priv->offset = dev_read_u32_default(dev, "offset", 0);
	priv->mask = dev_read_u32_default(dev, "mask", 0);

	return 0;
}

static const struct udevice_id syscon_reboot_ids[] = {
	{ .compatible = "syscon-reboot" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(syscon_reboot) = {
	.name = "syscon_reboot",
	.id = UCLASS_SYSRESET,
	.of_match = syscon_reboot_ids,
	.probe = syscon_reboot_probe,
	.priv_auto_alloc_size = sizeof(struct syscon_reboot_priv),
	.ops = &syscon_reboot_ops,
};
