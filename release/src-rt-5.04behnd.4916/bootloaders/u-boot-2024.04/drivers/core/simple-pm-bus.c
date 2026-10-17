// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Sean Anderson <seanga2@gmail.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>

/*
 * Power domains are taken care of by driver_probe, so we just have to enable
 * clocks
 */
static int simple_pm_bus_probe(struct udevice *dev)
{
	int ret;
	struct clk_bulk *bulk = dev_get_priv(dev);

	ret = clk_get_bulk(dev, bulk);
	if (ret)
		return ret;

	ret = clk_enable_bulk(bulk);
	if (ret && ret != -ENOSYS) {
		clk_release_bulk(bulk);
		return ret;
	}
	return 0;
}

static int simple_pm_bus_remove(struct udevice *dev)
{
	int ret;
	struct clk_bulk *bulk = dev_get_priv(dev);

	ret = clk_release_bulk(bulk);
	if (ret && ret != -ENOSYS)
		return ret;
	else
		return 0;
}

static const struct udevice_id simple_pm_bus_ids[] = {
	{ .compatible = "simple-pm-bus" },
	{ }
};

U_BOOT_DRIVER(simple_pm_bus_drv) = {
	.name	= "simple_pm_bus",
	.id	= UCLASS_SIMPLE_BUS,
	.of_match = simple_pm_bus_ids,
	.probe = simple_pm_bus_probe,
	.remove = simple_pm_bus_remove,
	.priv_auto	= sizeof(struct clk_bulk),
	.flags	= DM_FLAG_PRE_RELOC,
};
