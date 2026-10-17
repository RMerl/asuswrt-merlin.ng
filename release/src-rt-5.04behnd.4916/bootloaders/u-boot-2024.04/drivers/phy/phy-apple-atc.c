// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Mark Kettenis <kettenis@openbsd.org>
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <generic-phy.h>
#include <reset-uclass.h>

static const struct phy_ops apple_atcphy_ops = {
};

static struct driver apple_atcphy_driver = {
	.name = "apple-atcphy",
	.id = UCLASS_PHY,
	.ops = &apple_atcphy_ops,
};

static int apple_atcphy_reset_of_xlate(struct reset_ctl *reset_ctl,
				       struct ofnode_phandle_args *args)
{
	if (args->args_count != 0)
		return -EINVAL;

	return 0;
}

static const struct reset_ops apple_atcphy_reset_ops = {
	.of_xlate = apple_atcphy_reset_of_xlate,
};

static int apple_atcphy_reset_probe(struct udevice *dev)
{
	struct udevice *child;

	device_bind(dev, &apple_atcphy_driver, "apple-atcphy", NULL,
		    dev_ofnode(dev), &child);

	return 0;
}

static const struct udevice_id apple_atcphy_ids[] = {
	{ .compatible = "apple,t6000-atcphy" },
	{ .compatible = "apple,t8103-atcphy" },
	{ }
};

U_BOOT_DRIVER(apple_atcphy_reset) = {
	.name = "apple-atcphy-reset",
	.id = UCLASS_RESET,
	.of_match = apple_atcphy_ids,
	.ops = &apple_atcphy_reset_ops,
	.probe = apple_atcphy_reset_probe,
};
