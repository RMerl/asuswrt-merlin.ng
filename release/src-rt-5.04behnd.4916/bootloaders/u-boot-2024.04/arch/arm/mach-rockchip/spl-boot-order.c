// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017 Theobroma Systems Design und Consulting GmbH
 */

#include <common.h>
#include <dm.h>
#include <fdt_support.h>
#include <log.h>
#include <mmc.h>
#include <spl.h>
#include <asm/global_data.h>
#include <dm/uclass-internal.h>

#if CONFIG_IS_ENABLED(OF_LIBFDT)
/**
 * spl_node_to_boot_device() - maps from a DT-node to a SPL boot device
 * @node:	of_offset of the node
 *
 * The SPL framework uses BOOT_DEVICE_... constants to identify its boot
 * sources.  These may take on a device-specific meaning, depending on
 * what nodes are enabled in a DTS (e.g. BOOT_DEVICE_MMC1 may refer to
 * different controllers/block-devices, depending on which SD/MMC controllers
 * are enabled in any given DTS).  This function maps from a DT-node back
 * onto a BOOT_DEVICE_... constant, considering the currently active devices.
 *
 * Returns
 *   -ENOENT, if no device matching the node could be found
 *   -ENOSYS, if the device matching the node can not be mapped onto a
 *            SPL boot device (e.g. the third MMC device)
 *   -1, for unspecified failures
 *   a positive integer (from the BOOT_DEVICE_... family) on succes.
 */

static int spl_node_to_boot_device(int node)
{
	struct udevice *parent;

	/*
	 * This should eventually move into the SPL code, once SPL becomes
	 * aware of the block-device layer.  Until then (and to avoid unneeded
	 * delays in getting this feature out), it lives at the board-level.
	 */
	if (!uclass_get_device_by_of_offset(UCLASS_MMC, node, &parent)) {
		struct udevice *dev;
		struct blk_desc *desc = NULL;

		for (device_find_first_child(parent, &dev);
		     dev;
		     device_find_next_child(&dev)) {
			if (device_get_uclass_id(dev) == UCLASS_BLK) {
				desc = dev_get_uclass_plat(dev);
				break;
			}
		}

		if (!desc)
			return -ENOENT;

		switch (desc->devnum) {
		case 0:
			return BOOT_DEVICE_MMC1;
		case 1:
			return BOOT_DEVICE_MMC2;
		default:
			return -ENOSYS;
		}
	} else if (!uclass_get_device_by_of_offset(UCLASS_SPI_FLASH, node,
		&parent)) {
		return BOOT_DEVICE_SPI;
	}

	/*
	 * SPL doesn't differentiate SPI flashes, so we keep the detection
	 * brief and inaccurate... hopefully, the common SPL layer can be
	 * extended with awareness of the BLK layer (and matching OF_CONTROL)
	 * soon.
	 */
	if (!uclass_get_device_by_of_offset(UCLASS_SPI_FLASH, node, &parent))
		return BOOT_DEVICE_SPI;

	return -1;
}

/**
 * board_spl_was_booted_from() - retrieves the of-path the SPL was loaded from
 *
 * To support a 'same-as-spl' specification in the search-order for the next
 * stage, we need a SoC- or board-specific way to handshake with what 'came
 * before us' (either a BROM or TPL stage) and map the info retrieved onto
 * a OF path.
 *
 * Returns
 *   NULL, on failure or if the device could not be identified
 *   a of_path (a string), on success
 */
__weak const char *board_spl_was_booted_from(void)
{
	debug("%s: no support for 'same-as-spl' for this board\n", __func__);
	return NULL;
}

void board_boot_order(u32 *spl_boot_list)
{
	/* In case of no fdt (or only plat), use spl_boot_device() */
	if (!CONFIG_IS_ENABLED(OF_CONTROL) || CONFIG_IS_ENABLED(OF_PLATDATA)) {
		spl_boot_list[0] = spl_boot_device();
		return;
	}

	const void *blob = gd->fdt_blob;
	int chosen_node = fdt_path_offset(blob, "/chosen");
	int idx = 0;
	int elem;
	int boot_device;
	int node;
	const char *conf;

	if (chosen_node < 0) {
		debug("%s: /chosen not found, using spl_boot_device()\n",
		      __func__);
		spl_boot_list[0] = spl_boot_device();
		return;
	}

	for (elem = 0;
	     (conf = fdt_stringlist_get(blob, chosen_node,
					"u-boot,spl-boot-order", elem, NULL));
	     elem++) {
		const char *alias;

		/* Handle the case of 'same device the SPL was loaded from' */
		if (strncmp(conf, "same-as-spl", 11) == 0) {
			conf = board_spl_was_booted_from();
			if (!conf)
				continue;
		}

		/* First check if the list element is an alias */
		alias = fdt_get_alias(blob, conf);
		if (alias)
			conf = alias;

		/* Try to resolve the config item (or alias) as a path */
		node = fdt_path_offset(blob, conf);
		if (node < 0) {
			debug("%s: could not find %s in FDT\n", __func__, conf);
			continue;
		}

		/* Try to map this back onto SPL boot devices */
		boot_device = spl_node_to_boot_device(node);
		if (boot_device < 0) {
			debug("%s: could not map node @%x to a boot-device\n",
			      __func__, node);
			continue;
		}

		spl_boot_list[idx++] = boot_device;
	}

	/* If we had no matches, fall back to spl_boot_device */
	if (idx == 0)
		spl_boot_list[0] = spl_boot_device();
}

int spl_decode_boot_device(u32 boot_device, char *buf, size_t buflen)
{
	struct udevice *dev;
#if CONFIG_IS_ENABLED(BLK)
	int dev_num;
#endif
	int ret;

	if (boot_device == BOOT_DEVICE_SPI) {
		/* Revert spl_node_to_boot_device() logic to find appropriate SPI flash device */

		/*
		 * Devices with multiple SPI flash devices will take the first SPI flash found in
		 * /chosen/u-boot,spl-boot-order.
		 */
		const void *blob = gd->fdt_blob;
		int chosen_node = fdt_path_offset(blob, "/chosen");
		int elem;
		int node;
		const char *conf;

		if (chosen_node < 0) {
			debug("%s: /chosen not found\n", __func__);
			return -ENODEV;
		}

		for (elem = 0;
		     (conf = fdt_stringlist_get(blob, chosen_node,
						"u-boot,spl-boot-order", elem, NULL));
		     elem++) {
			const char *alias;

			/* Handle the case of 'same device the SPL was loaded from' */
			if (strncmp(conf, "same-as-spl", 11) == 0) {
				conf = board_spl_was_booted_from();
				if (!conf)
					continue;
			}

			/* First check if the list element is an alias */
			alias = fdt_get_alias(blob, conf);
			if (alias)
				conf = alias;

			/* Try to resolve the config item (or alias) as a path */
			node = fdt_path_offset(blob, conf);
			if (node < 0) {
				debug("%s: could not find %s in FDT\n", __func__, conf);
				continue;
			}

			ret = uclass_find_device_by_of_offset(UCLASS_SPI_FLASH, node, &dev);
			if (ret) {
				debug("%s: could not find udevice for %s\n", __func__, conf);
				continue;
			}

			return ofnode_get_path(dev_ofnode(dev), buf, buflen);
		}

		return -ENODEV;
	}

#if CONFIG_IS_ENABLED(BLK)
	dev_num = (boot_device == BOOT_DEVICE_MMC1) ? 0 : 1;

	ret = blk_find_device(UCLASS_MMC, dev_num, &dev);
	if (ret) {
		debug("%s: could not find blk device for MMC device %d: %d\n",
		      __func__, dev_num, ret);
		return ret;
	}

	dev = dev_get_parent(dev);
	return ofnode_get_path(dev_ofnode(dev), buf, buflen);
#else
	return -ENODEV;
#endif
}

void spl_perform_fixups(struct spl_image_info *spl_image)
{
	void *blob = spl_image_fdt_addr(spl_image);
	char boot_ofpath[512];
	int chosen, ret;

	/*
	 * Inject the ofpath of the device the full U-Boot (or Linux in
	 * Falcon-mode) was booted from into the FDT, if a FDT has been
	 * loaded at the same time.
	 */
	if (!blob)
		return;

	ret = spl_decode_boot_device(spl_image->boot_device, boot_ofpath, sizeof(boot_ofpath));
	if (ret) {
		pr_err("%s: could not map boot_device to ofpath: %d\n", __func__, ret);
		return;
	}

	chosen = fdt_find_or_add_subnode(blob, 0, "chosen");
	if (chosen < 0) {
		pr_err("%s: could not find/create '/chosen'\n", __func__);
		return;
	}
	fdt_setprop_string(blob, chosen,
			   "u-boot,spl-boot-device", boot_ofpath);
}
#endif
