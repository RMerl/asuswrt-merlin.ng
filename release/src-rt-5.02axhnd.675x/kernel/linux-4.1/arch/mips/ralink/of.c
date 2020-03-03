/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 * Copyright (C) 2008-2009 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (C) 2013 John Crispin <blogic@openwrt.org>
 */

#include <linux/io.h>
#include <linux/clk.h>
#include <linux/init.h>
#include <linux/sizes.h>
#include <linux/of_fdt.h>
#include <linux/kernel.h>
#include <linux/bootmem.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>

#include <asm/reboot.h>
#include <asm/bootinfo.h>
#include <asm/addrspace.h>
#include <asm/prom.h>

#include "common.h"

__iomem void *rt_sysc_membase;
__iomem void *rt_memc_membase;

__iomem void *plat_of_remap_node(const char *node)
{
	struct resource res;
	struct device_node *np;

	np = of_find_compatible_node(NULL, NULL, node);
	if (!np)
		panic("Failed to find %s node", node);

	if (of_address_to_resource(np, 0, &res))
		panic("Failed to get resource for %s", node);

	if ((request_mem_region(res.start,
				resource_size(&res),
				res.name) < 0))
		panic("Failed to request resources for %s", node);

	return ioremap_nocache(res.start, resource_size(&res));
}

void __init device_tree_init(void)
{
	unflatten_and_copy_device_tree();
}

static int memory_dtb;

static int __init early_init_dt_find_memory(unsigned long node,
				const char *uname, int depth, void *data)
{
	if (depth == 1 && !strcmp(uname, "memory@0"))
		memory_dtb = 1;

	return 0;
}

void __init plat_mem_setup(void)
{
	set_io_port_base(KSEG1);

	/*
	 * Load the builtin devicetree. This causes the chosen node to be
	 * parsed resulting in our memory appearing
	 */
	__dt_setup_arch(__dtb_start);

	strlcpy(arcs_cmdline, boot_command_line, COMMAND_LINE_SIZE);

	of_scan_flat_dt(early_init_dt_find_memory, NULL);
	if (memory_dtb)
		of_scan_flat_dt(early_init_dt_scan_memory, NULL);
	else if (soc_info.mem_size)
		add_memory_region(soc_info.mem_base, soc_info.mem_size * SZ_1M,
				  BOOT_MEM_RAM);
	else
		detect_memory_region(soc_info.mem_base,
				     soc_info.mem_size_min * SZ_1M,
				     soc_info.mem_size_max * SZ_1M);
}

static int __init plat_of_setup(void)
{
	__dt_register_buses(soc_info.compatible, "palmbus");

	/* make sure that the reset controller is setup early */
	ralink_rst_init();

	return 0;
}

arch_initcall(plat_of_setup);
