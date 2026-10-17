// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <errno.h>
#include <image.h>
#include <log.h>
#include <linux/libfdt.h>

ulong fdt_getprop_u32(const void *fdt, int node, const char *prop)
{
	const u32 *cell;
	int len;

	cell = fdt_getprop(fdt, node, prop, &len);
	if (!cell || len != sizeof(*cell))
		return FDT_ERROR;

	return fdt32_to_cpu(*cell);
}

__weak int board_fit_config_name_match(const char *name)
{
	return -EINVAL;
}

/*
 * Iterate over all /configurations subnodes and call a platform specific
 * function to find the matching configuration.
 * Returns the node offset or a negative error number.
 */
int fit_find_config_node(const void *fdt)
{
	const char *name;
	int conf, node, len;
	const char *dflt_conf_name;
	const char *dflt_conf_desc = NULL;
	int dflt_conf_node = -ENOENT;

	conf = fdt_path_offset(fdt, FIT_CONFS_PATH);
	if (conf < 0) {
		debug("%s: Cannot find /configurations node: %d\n", __func__,
		      conf);
		return -EINVAL;
	}

	dflt_conf_name = fdt_getprop(fdt, conf, "default", &len);

	for (node = fdt_first_subnode(fdt, conf);
	     node >= 0;
	     node = fdt_next_subnode(fdt, node)) {
		name = fdt_getprop(fdt, node, "description", &len);
		if (!name) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
			printf("%s: Missing FDT description in DTB\n",
			       __func__);
#endif
			return -EINVAL;
		}

		if (dflt_conf_name) {
			const char *node_name = fdt_get_name(fdt, node, NULL);
			if (strcmp(dflt_conf_name, node_name) == 0) {
				dflt_conf_node = node;
				dflt_conf_desc = name;
			}
		}

		if (board_fit_config_name_match(name))
			continue;

		debug("Selecting config '%s'\n", name);

		return node;
	}

	if (dflt_conf_node != -ENOENT) {
		debug("Selecting default config '%s'\n", dflt_conf_desc);
		return dflt_conf_node;
	}

	return -ENOENT;
}
