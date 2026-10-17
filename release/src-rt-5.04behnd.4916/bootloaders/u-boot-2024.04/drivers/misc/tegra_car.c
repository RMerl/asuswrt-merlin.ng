// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <dm/lists.h>
#include <dm/root.h>

/**
 * The CAR exposes multiple different services. We create a sub-device for
 * each separate type of service, since each device must be of the appropriate
 * UCLASS.
 */
static int tegra_car_bpmp_bind(struct udevice *dev)
{
	int ret;
	struct udevice *child;

	debug("%s(dev=%p)\n", __func__, dev);

	ret = device_bind_driver_to_node(dev, "tegra_car_clk", "tegra_car_clk",
					 dev_ofnode(dev), &child);
	if (ret)
		return ret;

	ret = device_bind_driver_to_node(dev, "tegra_car_reset",
					 "tegra_car_reset", dev_ofnode(dev),
					 &child);
	if (ret)
		return ret;

	return 0;
}

static int tegra_car_bpmp_probe(struct udevice *dev)
{
	debug("%s(dev=%p)\n", __func__, dev);

	return 0;
}

static int tegra_car_bpmp_remove(struct udevice *dev)
{
	debug("%s(dev=%p)\n", __func__, dev);

	return 0;
}

static const struct udevice_id tegra_car_bpmp_ids[] = {
	{ .compatible = "nvidia,tegra20-car" },
	{ .compatible = "nvidia,tegra30-car" },
	{ .compatible = "nvidia,tegra114-car" },
	{ .compatible = "nvidia,tegra124-car" },
	{ .compatible = "nvidia,tegra210-car" },
	{ }
};

U_BOOT_DRIVER(tegra_car_bpmp) = {
	.name		= "tegra_car",
	.id		= UCLASS_MISC,
	.of_match	= tegra_car_bpmp_ids,
	.bind		= tegra_car_bpmp_bind,
	.probe		= tegra_car_bpmp_probe,
	.remove		= tegra_car_bpmp_remove,
};
