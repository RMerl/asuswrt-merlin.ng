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
#include <linux/err.h>
#include <linux/printk.h>

struct syscon_reboot_priv {
	struct regmap *regmap;
	unsigned int offset;
	unsigned int mask;
	unsigned int value;
};

static int syscon_reboot_request(struct udevice *dev, enum sysreset_t type)
{
	struct syscon_reboot_priv *priv = dev_get_priv(dev);
	ulong driver_data = dev_get_driver_data(dev);

	if (type != driver_data)
		return -EPROTONOSUPPORT;

	regmap_update_bits(priv->regmap, priv->offset, priv->mask, priv->value);

	return -EINPROGRESS;
}

static struct sysreset_ops syscon_reboot_ops = {
	.request = syscon_reboot_request,
};

static int syscon_reboot_probe(struct udevice *dev)
{
	struct syscon_reboot_priv *priv = dev_get_priv(dev);
	int err;
	int mask_err, value_err;

	priv->regmap = syscon_regmap_lookup_by_phandle(dev, "regmap");
	if (IS_ERR(priv->regmap)) {
		pr_err("unable to find regmap\n");
		return -ENODEV;
	}

	err = dev_read_u32(dev, "offset", &priv->offset);
	if (err) {
		pr_err("unable to find offset\n");
		return -ENOENT;
	}

	mask_err = dev_read_u32(dev, "mask", &priv->mask);
	value_err = dev_read_u32(dev, "value", &priv->value);
	if (mask_err && value_err) {
		pr_err("unable to find mask and value\n");
		return -EINVAL;
	}

	if (value_err) {
		/* support old binding */
		priv->value = priv->mask;
		priv->mask = 0xffffffff;
	} else if (mask_err) {
		/* support value without mask*/
		priv->mask = 0xffffffff;
	}

	return 0;
}

static const struct udevice_id syscon_reboot_ids[] = {
	{ .compatible = "syscon-reboot", .data = SYSRESET_COLD },
	{ .compatible = "syscon-poweroff", .data = SYSRESET_POWER_OFF },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(syscon_reboot) = {
	.name = "syscon_reboot",
	.id = UCLASS_SYSRESET,
	.of_match = syscon_reboot_ids,
	.probe = syscon_reboot_probe,
	.priv_auto	= sizeof(struct syscon_reboot_priv),
	.ops = &syscon_reboot_ops,
};
