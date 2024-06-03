// SPDX-License-Identifier: GPL-2.0
/*
 *  Copyright (C) 2016-2017 Intel Corporation <www.intel.com>
 */

#include <asm/arch/pinmux.h>
#include <asm/io.h>
#include <common.h>
#include <fdtdec.h>

static int do_pinctr_pin(const void *blob, int child, const char *node_name)
{
	int len;
	fdt_addr_t base_addr;
	fdt_size_t size;
	const u32 *cell;
	u32 offset, value;

	base_addr = fdtdec_get_addr_size(blob, child, "reg", &size);
	if (base_addr != FDT_ADDR_T_NONE) {
		cell = fdt_getprop(blob, child, "pinctrl-single,pins", &len);
		if (!cell || len <= 0)
			return -EFAULT;

		debug("%p %d\n", cell, len);
		for (; len > 0; len -= (2 * sizeof(u32))) {
			offset = fdt32_to_cpu(*cell++);
			value = fdt32_to_cpu(*cell++);
			debug("<0x%x 0x%x>\n", offset, value);
			writel(value, base_addr + offset);
		}
		return 0;
	}
	return -EFAULT;
}

static int do_pinctrl_pins(const void *blob, int node, const char *child_name)
{
	int child, len;
	const char *node_name;

	child = fdt_first_subnode(blob, node);

	if (child < 0)
		return -EINVAL;

	node_name = fdt_get_name(blob, child, &len);

	while (node_name) {
		if (!strcmp(child_name, node_name))
			return do_pinctr_pin(blob, child, node_name);

		child = fdt_next_subnode(blob, child);

		if (child < 0)
			break;

		node_name = fdt_get_name(blob, child, &len);
	}

	return -EFAULT;
}

int config_dedicated_pins(const void *blob)
{
	int node;

	node = fdtdec_next_compatible(blob, 0,
			COMPAT_ALTERA_SOCFPGA_PINCTRL_SINGLE);
	if (node < 0)
		return -EINVAL;

	if (do_pinctrl_pins(blob, node, "dedicated_cfg"))
		return -EFAULT;

	if (do_pinctrl_pins(blob, node, "dedicated"))
		return -EFAULT;

	return 0;
}

int config_pins(const void *blob, const char *pin_grp)
{
	int node;

	node = fdtdec_next_compatible(blob, 0,
			COMPAT_ALTERA_SOCFPGA_PINCTRL_SINGLE);
	if (node < 0)
		return -EINVAL;

	if (do_pinctrl_pins(blob, node, pin_grp))
		return -EFAULT;

	return 0;
}
