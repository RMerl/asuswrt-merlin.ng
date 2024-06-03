// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <syscon.h>
#include <dm.h>
#include <errno.h>
#include <regmap.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/root.h>
#include <linux/err.h>

/*
 * Caution:
 * This API requires the given device has alerady been bound to syscon driver.
 * For example,
 *    compatible = "syscon", "simple-mfd";
 * works, but
 *    compatible = "simple-mfd", "syscon";
 * does not.  The behavior is different from Linux.
 */
struct regmap *syscon_get_regmap(struct udevice *dev)
{
	struct syscon_uc_info *priv;

	if (device_get_uclass_id(dev) != UCLASS_SYSCON)
		return ERR_PTR(-ENOEXEC);
	priv = dev_get_uclass_priv(dev);
	return priv->regmap;
}

static int syscon_pre_probe(struct udevice *dev)
{
	struct syscon_uc_info *priv = dev_get_uclass_priv(dev);

	/* Special case for PCI devices, which don't have a regmap */
	if (device_get_uclass_id(dev->parent) == UCLASS_PCI)
		return 0;

	/*
	 * With OF_PLATDATA we really have no way of knowing the format of
	 * the device-specific platform data. So we assume that it starts with
	 * a 'reg' member, and this holds a single address and size. Drivers
	 * using OF_PLATDATA will need to ensure that this is true.
	 */
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct syscon_base_platdata *plat = dev_get_platdata(dev);

	return regmap_init_mem_platdata(dev, plat->reg, ARRAY_SIZE(plat->reg),
					&priv->regmap);
#else
	return regmap_init_mem(dev_ofnode(dev), &priv->regmap);
#endif
}

static int syscon_probe_by_ofnode(ofnode node, struct udevice **devp)
{
	struct udevice *dev, *parent;
	int ret;

	/* found node with "syscon" compatible, not bounded to SYSCON UCLASS */
	if (!ofnode_device_is_compatible(node, "syscon")) {
		dev_dbg(dev, "invalid compatible for syscon device\n");
		return -EINVAL;
	}

	/* bound to driver with same ofnode or to root if not found */
	if (device_find_global_by_ofnode(node, &parent))
		parent = dm_root();

	/* force bound to syscon class */
	ret = device_bind_driver_to_node(parent, "syscon",
					 ofnode_get_name(node),
					 node, &dev);
	if (ret) {
		dev_dbg(dev, "unable to bound syscon device\n");
		return ret;
	}
	ret = device_probe(dev);
	if (ret) {
		dev_dbg(dev, "unable to probe syscon device\n");
		return ret;
	}

	*devp = dev;
	return 0;
}

struct regmap *syscon_regmap_lookup_by_phandle(struct udevice *dev,
					       const char *name)
{
	struct udevice *syscon;
	struct regmap *r;
	u32 phandle;
	ofnode node;
	int err;

	err = uclass_get_device_by_phandle(UCLASS_SYSCON, dev,
					   name, &syscon);
	if (err) {
		/* found node with "syscon" compatible, not bounded to SYSCON */
		err = ofnode_read_u32(dev_ofnode(dev), name, &phandle);
		if (err)
			return ERR_PTR(err);

		node = ofnode_get_by_phandle(phandle);
		if (!ofnode_valid(node)) {
			dev_dbg(dev, "unable to find syscon device\n");
			return ERR_PTR(-EINVAL);
		}
		err = syscon_probe_by_ofnode(node, &syscon);
		if (err)
			return ERR_PTR(-ENODEV);
	}

	r = syscon_get_regmap(syscon);
	if (!r) {
		dev_dbg(dev, "unable to find regmap\n");
		return ERR_PTR(-ENODEV);
	}

	return r;
}

int syscon_get_by_driver_data(ulong driver_data, struct udevice **devp)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	*devp = NULL;
	ret = uclass_get(UCLASS_SYSCON, &uc);
	if (ret)
		return ret;
	uclass_foreach_dev(dev, uc) {
		if (dev->driver_data == driver_data) {
			*devp = dev;
			return device_probe(dev);
		}
	}

	return -ENODEV;
}

struct regmap *syscon_get_regmap_by_driver_data(ulong driver_data)
{
	struct syscon_uc_info *priv;
	struct udevice *dev;
	int ret;

	ret = syscon_get_by_driver_data(driver_data, &dev);
	if (ret)
		return ERR_PTR(ret);
	priv = dev_get_uclass_priv(dev);

	return priv->regmap;
}

void *syscon_get_first_range(ulong driver_data)
{
	struct regmap *map;

	map = syscon_get_regmap_by_driver_data(driver_data);
	if (IS_ERR(map))
		return map;
	return regmap_get_range(map, 0);
}

UCLASS_DRIVER(syscon) = {
	.id		= UCLASS_SYSCON,
	.name		= "syscon",
	.per_device_auto_alloc_size = sizeof(struct syscon_uc_info),
	.pre_probe = syscon_pre_probe,
};

static const struct udevice_id generic_syscon_ids[] = {
	{ .compatible = "syscon" },
	{ }
};

U_BOOT_DRIVER(generic_syscon) = {
	.name	= "syscon",
	.id	= UCLASS_SYSCON,
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	.bind           = dm_scan_fdt_dev,
#endif
	.of_match = generic_syscon_ids,
};

/*
 * Linux-compatible syscon-to-regmap
 * The syscon node can be bound to another driver, but still works
 * as a syscon provider.
 */
struct regmap *syscon_node_to_regmap(ofnode node)
{
	struct udevice *dev;
	struct regmap *r;

	if (uclass_get_device_by_ofnode(UCLASS_SYSCON, node, &dev))
		if (syscon_probe_by_ofnode(node, &dev))
			return ERR_PTR(-ENODEV);

	r = syscon_get_regmap(dev);
	if (!r) {
		dev_dbg(dev, "unable to find regmap\n");
		return ERR_PTR(-ENODEV);
	}

	return r;
}
