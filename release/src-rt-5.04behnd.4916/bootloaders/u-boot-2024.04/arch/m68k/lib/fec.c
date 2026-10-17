// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) 2019 Angelo Dureghello <angelo.dureghello@timesys.com>
 */

#include <asm/global_data.h>
#include <linux/errno.h>
#include <linux/libfdt.h>
#include <fdt_support.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_MCFFEC)
static int fec_get_node(int fec_idx)
{
	char fec_alias[5] = {"fec"};
	const char *path;
	int node;

	if (fec_idx > 1) {
		puts("Invalid MII base index");
		return -ENOENT;
	}

	fec_alias[3] = fec_idx + '0';

	path = fdt_get_alias(gd->fdt_blob, fec_alias);
	if (!path) {
		puts("Invalid MII path");
		return -ENOENT;
	}

	node = fdt_path_offset(gd->fdt_blob, path);
	if (node < 0)
		return -ENOENT;

	return node;
}

int fec_get_fdt_prop(int fec_idx, const char *prop, u32 *value)
{
	int node;
	const u32 *val;

	node = fec_get_node(fec_idx);
	if (node < 0)
		return node;

	val = fdt_getprop(gd->fdt_blob, node, prop, NULL);
	if (!val)
		return -ENOENT;

	*value = fdt32_to_cpu(*val);

	return 0;
}

int fec_get_base_addr(int fec_idx, u32 *fec_iobase)
{
	int node;
	fdt_size_t size;
	fdt_addr_t addr;

	node = fec_get_node(fec_idx);
	if (node < 0)
		return node;

	addr = fdtdec_get_addr_size(gd->fdt_blob, node, "reg", &size);

	*fec_iobase = (u32)addr;

	return 0;
}

int fec_get_mii_base(int fec_idx, u32 *mii_base)
{
	return fec_get_fdt_prop(fec_idx, "mii-base", mii_base);
}

#endif //CONFIG_MCFFEC
