// SPDX-License-Identifier: GPL-2.0
/*
 * Verified Boot for Embedded (VBE) access functions
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <bootmeth.h>
#include <bootstd.h>
#include <dm.h>
#include <image.h>
#include <vbe.h>
#include <dm/uclass-internal.h>

/**
 * is_vbe() - Check if a device is a VBE method
 *
 * @dev: Device to check
 * @return true if this is a VBE bootmth device, else false
 */
static bool is_vbe(struct udevice *dev)
{
	return !strncmp("vbe", dev->driver->name, 3);
}

int vbe_find_next_device(struct udevice **devp)
{
	for (uclass_find_next_device(devp);
	     *devp;
	     uclass_find_next_device(devp)) {
		if (is_vbe(*devp))
			return 0;
	}

	return 0;
}

int vbe_find_first_device(struct udevice **devp)
{
	uclass_find_first_device(UCLASS_BOOTMETH, devp);
	if (!*devp || is_vbe(*devp))
		return 0;

	return vbe_find_next_device(devp);
}

int vbe_list(void)
{
	struct bootstd_priv *std;
	struct udevice *dev;
	int ret;

	ret = bootstd_get_priv(&std);
	if (ret)
		return ret;

	printf("%3s  %-3s  %-15s  %-15s %s\n", "#", "Sel", "Device", "Driver",
	       "Description");
	printf("%3s  %-3s  %-15s  %-15s %s\n", "---", "---", "--------------",
	       "--------------", "-----------");
	for (ret = vbe_find_first_device(&dev); dev;
	     ret = vbe_find_next_device(&dev)) {
		const struct bootmeth_uc_plat *plat = dev_get_uclass_plat(dev);

		printf("%3d  %-3s  %-15s  %-15s %s\n", dev_seq(dev),
		       std->vbe_bootmeth == dev ? "*" : "", dev->name,
		       dev->driver->name, plat->desc);
	}
	printf("%3s  %-3s  %-15s  %-15s %s\n", "---", "---", "--------------",
	       "--------------", "-----------");

	return 0;
}

int vbe_select(struct udevice *dev)
{
	struct bootstd_priv *std;
	int ret;

	ret = bootstd_get_priv(&std);
	if (ret)
		return ret;
	std->vbe_bootmeth = dev;

	return 0;
}

int vbe_find_by_any(const char *name, struct udevice **devp)
{
	struct udevice *dev;
	int ret, seq;
	char *endp;

	seq = simple_strtol(name, &endp, 16);

	/* Select by name */
	if (*endp) {
		ret = uclass_get_device_by_name(UCLASS_BOOTMETH, name, &dev);
		if (ret) {
			printf("Cannot probe VBE bootmeth '%s' (err=%d)\n", name,
			       ret);
			return ret;
		}

	/* select by number */
	} else {
		ret = uclass_get_device_by_seq(UCLASS_BOOTMETH, seq, &dev);
		if (ret) {
			printf("Cannot find '%s' (err=%d)\n", name, ret);
			return ret;
		}
	}

	*devp = dev;

	return 0;
}
