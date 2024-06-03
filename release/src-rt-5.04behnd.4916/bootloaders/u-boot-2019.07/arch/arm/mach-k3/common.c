// SPDX-License-Identifier: GPL-2.0+
/*
 * K3: Common Architecture initialization
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <common.h>
#include <spl.h>
#include "common.h"
#include <dm.h>
#include <remoteproc.h>
#include <linux/soc/ti/ti_sci_protocol.h>
#include <fdt_support.h>

struct ti_sci_handle *get_ti_sci_handle(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_name(UCLASS_FIRMWARE, "dmsc", &dev);
	if (ret)
		panic("Failed to get SYSFW (%d)\n", ret);

	return (struct ti_sci_handle *)ti_sci_get_handle_from_sysfw(dev);
}

#ifdef CONFIG_SYS_K3_SPL_ATF
void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	int ret;

	/*
	 * It is assumed that remoteproc device 1 is the corresponding
	 * Cortex-A core which runs ATF. Make sure DT reflects the same.
	 */
	ret = rproc_dev_init(1);
	if (ret)
		panic("%s: ATF failed to initialize on rproc (%d)\n", __func__,
		      ret);

	ret = rproc_load(1, spl_image->entry_point, 0x200);
	if (ret)
		panic("%s: ATF failed to load on rproc (%d)\n", __func__, ret);

	/* Add an extra newline to differentiate the ATF logs from SPL */
	printf("Starting ATF on ARM64 core...\n\n");

	ret = rproc_start(1);
	if (ret)
		panic("%s: ATF failed to start on rproc (%d)\n", __func__, ret);

	debug("ATF started. Waiting indefinitely...\n");
	while (1)
		asm volatile("wfe");
}
#endif

#if defined(CONFIG_OF_LIBFDT)
int fdt_fixup_msmc_ram(void *blob, char *parent_path, char *node_name)
{
	u64 msmc_start = 0, msmc_end = 0, msmc_size, reg[2];
	struct ti_sci_handle *ti_sci = get_ti_sci_handle();
	int ret, node, subnode, len, prev_node;
	u32 range[4], addr, size;
	const fdt32_t *sub_reg;

	ti_sci->ops.core_ops.query_msmc(ti_sci, &msmc_start, &msmc_end);
	msmc_size = msmc_end - msmc_start + 1;
	debug("%s: msmc_start = 0x%llx, msmc_size = 0x%llx\n", __func__,
	      msmc_start, msmc_size);

	/* find or create "msmc_sram node */
	ret = fdt_path_offset(blob, parent_path);
	if (ret < 0)
		return ret;

	node = fdt_find_or_add_subnode(blob, ret, node_name);
	if (node < 0)
		return node;

	ret = fdt_setprop_string(blob, node, "compatible", "mmio-sram");
	if (ret < 0)
		return ret;

	reg[0] = cpu_to_fdt64(msmc_start);
	reg[1] = cpu_to_fdt64(msmc_size);
	ret = fdt_setprop(blob, node, "reg", reg, sizeof(reg));
	if (ret < 0)
		return ret;

	fdt_setprop_cell(blob, node, "#address-cells", 1);
	fdt_setprop_cell(blob, node, "#size-cells", 1);

	range[0] = 0;
	range[1] = cpu_to_fdt32(msmc_start >> 32);
	range[2] = cpu_to_fdt32(msmc_start & 0xffffffff);
	range[3] = cpu_to_fdt32(msmc_size);
	ret = fdt_setprop(blob, node, "ranges", range, sizeof(range));
	if (ret < 0)
		return ret;

	subnode = fdt_first_subnode(blob, node);
	prev_node = 0;

	/* Look for invalid subnodes and delete them */
	while (subnode >= 0) {
		sub_reg = fdt_getprop(blob, subnode, "reg", &len);
		addr = fdt_read_number(sub_reg, 1);
		sub_reg++;
		size = fdt_read_number(sub_reg, 1);
		debug("%s: subnode = %d, addr = 0x%x. size = 0x%x\n", __func__,
		      subnode, addr, size);
		if (addr + size > msmc_size ||
		    !strncmp(fdt_get_name(blob, subnode, &len), "sysfw", 5) ||
		    !strncmp(fdt_get_name(blob, subnode, &len), "l3cache", 7)) {
			fdt_del_node(blob, subnode);
			debug("%s: deleting subnode %d\n", __func__, subnode);
			if (!prev_node)
				subnode = fdt_first_subnode(blob, node);
			else
				subnode = fdt_next_subnode(blob, prev_node);
		} else {
			prev_node = subnode;
			subnode = fdt_next_subnode(blob, prev_node);
		}
	}

	return 0;
}
#endif
