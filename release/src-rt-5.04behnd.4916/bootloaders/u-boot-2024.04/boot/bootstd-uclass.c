// SPDX-License-Identifier: GPL-2.0+
/*
 * Uclass implementation for standard boot
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <bootflow.h>
#include <bootstd.h>
#include <dm.h>
#include <env.h>
#include <log.h>
#include <malloc.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/read.h>
#include <dm/uclass-internal.h>

DECLARE_GLOBAL_DATA_PTR;

/* These are used if filename-prefixes is not present */
const char *const default_prefixes[] = {"/", "/boot/", NULL};

static int bootstd_of_to_plat(struct udevice *dev)
{
	struct bootstd_priv *priv = dev_get_priv(dev);
	int ret;

	if (IS_ENABLED(CONFIG_BOOTSTD_FULL)) {
		/* Don't check errors since livetree and flattree are different */
		ret = dev_read_string_list(dev, "filename-prefixes",
					   &priv->prefixes);
		dev_read_string_list(dev, "bootdev-order",
				     &priv->bootdev_order);

		priv->theme = ofnode_find_subnode(dev_ofnode(dev), "theme");
	}

	return 0;
}

static void bootstd_clear_glob_(struct bootstd_priv *priv)
{
	while (!list_empty(&priv->glob_head)) {
		struct bootflow *bflow;

		bflow = list_first_entry(&priv->glob_head, struct bootflow,
					 glob_node);
		bootflow_remove(bflow);
	}
}

void bootstd_clear_glob(void)
{
	struct bootstd_priv *std;

	if (bootstd_get_priv(&std))
		return;

	bootstd_clear_glob_(std);
}

static int bootstd_remove(struct udevice *dev)
{
	struct bootstd_priv *priv = dev_get_priv(dev);

	free(priv->prefixes);
	free(priv->bootdev_order);
	bootstd_clear_glob_(priv);

	return 0;
}

const char *const *const bootstd_get_bootdev_order(struct udevice *dev,
						   bool *okp)
{
	struct bootstd_priv *std = dev_get_priv(dev);
	const char *targets = env_get("boot_targets");

	*okp = true;
	log_debug("- targets %s %p\n", targets, std->bootdev_order);
	if (targets && *targets) {
		str_free_list(std->env_order);
		std->env_order = str_to_list(targets);
		if (!std->env_order) {
			*okp = false;
			return NULL;
		}
		return std->env_order;
	}

	return std->bootdev_order;
}

const char *const *const bootstd_get_prefixes(struct udevice *dev)
{
	struct bootstd_priv *std = dev_get_priv(dev);

	return std->prefixes ? std->prefixes : default_prefixes;
}

int bootstd_get_priv(struct bootstd_priv **stdp)
{
	struct udevice *dev;
	int ret;

	ret = uclass_first_device_err(UCLASS_BOOTSTD, &dev);
	if (ret)
		return ret;
	*stdp = dev_get_priv(dev);

	return 0;
}

static int bootstd_probe(struct udevice *dev)
{
	struct bootstd_priv *std = dev_get_priv(dev);

	INIT_LIST_HEAD(&std->glob_head);

	return 0;
}

/* For now, bind the boormethod device if none are found in the devicetree */
int dm_scan_other(bool pre_reloc_only)
{
	struct driver *drv = ll_entry_start(struct driver, driver);
	const int n_ents = ll_entry_count(struct driver, driver);
	struct udevice *dev, *bootstd;
	int i, ret;

	/* These are not needed before relocation */
	if (!(gd->flags & GD_FLG_RELOC))
		return 0;

	/* Create a bootstd device if needed */
	uclass_find_first_device(UCLASS_BOOTSTD, &bootstd);
	if (!bootstd) {
		ret = device_bind_driver(gd->dm_root, "bootstd_drv", "bootstd",
					 &bootstd);
		if (ret)
			return log_msg_ret("bootstd", ret);
	}

	/* If there are no bootmeth devices, create them */
	uclass_find_first_device(UCLASS_BOOTMETH, &dev);
	if (dev)
		return 0;

	for (i = 0; i < n_ents; i++, drv++) {
		if (drv->id == UCLASS_BOOTMETH) {
			const char *name = drv->name;

			if (!strncmp("bootmeth_", name, 9))
				name += 9;
			ret = device_bind(bootstd, drv, name, 0, ofnode_null(),
					  &dev);
			if (ret)
				return log_msg_ret("meth", ret);
		}
	}

	return 0;
}

static const struct udevice_id bootstd_ids[] = {
	{ .compatible = "u-boot,boot-std" },
	{ }
};

U_BOOT_DRIVER(bootstd_drv) = {
	.id		= UCLASS_BOOTSTD,
	.name		= "bootstd_drv",
	.of_to_plat	= bootstd_of_to_plat,
	.probe		= bootstd_probe,
	.remove		= bootstd_remove,
	.of_match	= bootstd_ids,
	.priv_auto	= sizeof(struct bootstd_priv),
};

UCLASS_DRIVER(bootstd) = {
	.id		= UCLASS_BOOTSTD,
	.name		= "bootstd",
#if CONFIG_IS_ENABLED(OF_REAL)
	.post_bind	= dm_scan_fdt_dev,
#endif
};
