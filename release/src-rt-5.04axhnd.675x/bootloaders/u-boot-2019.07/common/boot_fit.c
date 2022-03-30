// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017
 * Texas Instruments, <www.ti.com>
 *
 * Franklin S Cooper Jr. <fcooper@ti.com>
 */

#include <boot_fit.h>
#include <common.h>
#include <errno.h>
#include <image.h>
#include <linux/libfdt.h>

static int fdt_offset(const void *fit)
{
	int images, node, fdt_len, fdt_node, fdt_offset;
	const char *fdt_name;

	node = fit_find_config_node(fit);
	if (node < 0)
		return node;

	images = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__, images);
		return -EINVAL;
	}

	fdt_name = fdt_getprop(fit, node, FIT_FDT_PROP, &fdt_len);
	if (!fdt_name) {
		debug("%s: Cannot find fdt name property: %d\n",
		      __func__, fdt_len);
		return -EINVAL;
	}

	fdt_node = fdt_subnode_offset(fit, images, fdt_name);
	if (fdt_node < 0) {
		debug("%s: Cannot find fdt node '%s': %d\n",
		      __func__, fdt_name, fdt_node);
		return -EINVAL;
	}

	fdt_offset = fdt_getprop_u32(fit, fdt_node, "data-offset");

	if (fdt_offset == FDT_ERROR)
		return -ENOENT;

	fdt_len = fdt_getprop_u32(fit, fdt_node, "data-size");

	if (fdt_len < 0)
		return fdt_len;

	return fdt_offset;
}

void *locate_dtb_in_fit(const void *fit)
{
	struct image_header *header;
	int size;
	int ret;

	size = fdt_totalsize(fit);
	size = (size + 3) & ~3;

	header = (struct image_header *)fit;

	if (image_get_magic(header) != FDT_MAGIC) {
		debug("No FIT image appended to U-boot\n");
		return NULL;
	}

	ret = fdt_offset(fit);

	if (ret < 0)
		return NULL;
	else
		return (void *)fit+size+ret;
}
