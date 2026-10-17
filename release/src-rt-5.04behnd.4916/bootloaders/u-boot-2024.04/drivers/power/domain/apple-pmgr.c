// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Mark Kettenis <kettenis@openbsd.org>
 */

#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <linux/err.h>
#include <linux/bitfield.h>
#include <power-domain-uclass.h>
#include <reset-uclass.h>
#include <regmap.h>
#include <syscon.h>

#define APPLE_PMGR_RESET	BIT(31)
#define APPLE_PMGR_DEV_DISABLE	BIT(10)
#define APPLE_PMGR_WAS_CLKGATED	BIT(9)
#define APPLE_PMGR_WAS_PWRGATED BIT(8)
#define APPLE_PMGR_PS_ACTUAL	GENMASK(7, 4)
#define APPLE_PMGR_PS_TARGET	GENMASK(3, 0)

#define APPLE_PMGR_FLAGS	(APPLE_PMGR_WAS_CLKGATED | APPLE_PMGR_WAS_PWRGATED)

#define APPLE_PMGR_PS_ACTIVE	0xf
#define APPLE_PMGR_PS_PWRGATE	0x0

#define APPLE_PMGR_PS_SET_TIMEOUT_US	100

struct apple_pmgr_priv {
	struct regmap *regmap;
	u32 offset;		/* offset within regmap for this domain */
};

static int apple_reset_of_xlate(struct reset_ctl *reset_ctl,
				struct ofnode_phandle_args *args)
{
	if (args->args_count != 0)
		return -EINVAL;

	return 0;
}

static int apple_reset_assert(struct reset_ctl *reset_ctl)
{
	struct apple_pmgr_priv *priv = dev_get_priv(reset_ctl->dev->parent);

	regmap_update_bits(priv->regmap, priv->offset,
			   APPLE_PMGR_FLAGS | APPLE_PMGR_DEV_DISABLE,
			   APPLE_PMGR_DEV_DISABLE);
	regmap_update_bits(priv->regmap, priv->offset,
			   APPLE_PMGR_FLAGS | APPLE_PMGR_RESET,
			   APPLE_PMGR_RESET);

	return 0;
}

static int apple_reset_deassert(struct reset_ctl *reset_ctl)
{
	struct apple_pmgr_priv *priv = dev_get_priv(reset_ctl->dev->parent);

	regmap_update_bits(priv->regmap, priv->offset,
			   APPLE_PMGR_FLAGS | APPLE_PMGR_RESET, 0);
	regmap_update_bits(priv->regmap, priv->offset,
			   APPLE_PMGR_FLAGS | APPLE_PMGR_DEV_DISABLE, 0);

	return 0;
}

struct reset_ops apple_reset_ops = {
	.of_xlate = apple_reset_of_xlate,
	.rst_assert = apple_reset_assert,
	.rst_deassert = apple_reset_deassert,
};

static struct driver apple_reset_driver = {
	.name = "apple_reset",
	.id = UCLASS_RESET,
	.ops = &apple_reset_ops,
};

static int apple_pmgr_ps_set(struct power_domain *power_domain, u32 pstate)
{
	struct apple_pmgr_priv *priv = dev_get_priv(power_domain->dev);
	uint reg;

	regmap_update_bits(priv->regmap, priv->offset, APPLE_PMGR_PS_TARGET,
			   FIELD_PREP(APPLE_PMGR_PS_TARGET, pstate));

	return regmap_read_poll_timeout(
		priv->regmap, priv->offset, reg,
		(FIELD_GET(APPLE_PMGR_PS_ACTUAL, reg) == pstate), 1,
		APPLE_PMGR_PS_SET_TIMEOUT_US);
}

static int apple_pmgr_on(struct power_domain *power_domain)
{
	return apple_pmgr_ps_set(power_domain, APPLE_PMGR_PS_ACTIVE);
}

static int apple_pmgr_of_xlate(struct power_domain *power_domain,
			       struct ofnode_phandle_args *args)
{
	if (args->args_count != 0) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	return 0;
}

static const struct udevice_id apple_pmgr_ids[] = {
	{ .compatible = "apple,pmgr-pwrstate" },
	{ /* sentinel */ }
};

static int apple_pmgr_probe(struct udevice *dev)
{
	struct apple_pmgr_priv *priv = dev_get_priv(dev);
	struct udevice *child;
	int ret;

	ret = dev_power_domain_on(dev);
	if (ret)
		return ret;

	priv->regmap = syscon_get_regmap(dev->parent);
	if (IS_ERR(priv->regmap))
		return PTR_ERR(priv->regmap);

	ret = dev_read_u32(dev, "reg", &priv->offset);
	if (ret < 0)
		return ret;

	device_bind(dev, &apple_reset_driver, "apple_reset", NULL,
		    dev_ofnode(dev), &child);

	return 0;
}

struct power_domain_ops apple_pmgr_ops = {
	.on = apple_pmgr_on,
	.of_xlate = apple_pmgr_of_xlate,
};

U_BOOT_DRIVER(apple_pmgr) = {
	.name = "apple_pmgr",
	.id = UCLASS_POWER_DOMAIN,
	.of_match = apple_pmgr_ids,
	.ops = &apple_pmgr_ops,
	.probe = apple_pmgr_probe,
	.priv_auto = sizeof(struct apple_pmgr_priv),
};
