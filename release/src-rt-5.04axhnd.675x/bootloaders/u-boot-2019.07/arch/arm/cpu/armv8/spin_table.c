// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <linux/libfdt.h>
#include <asm/spin_table.h>

int spin_table_update_dt(void *fdt)
{
	int cpus_offset, offset;
	const char *prop;
	int ret;
	unsigned long rsv_addr = (unsigned long)&spin_table_reserve_begin;
	unsigned long rsv_size = &spin_table_reserve_end -
						&spin_table_reserve_begin;

	cpus_offset = fdt_path_offset(fdt, "/cpus");
	if (cpus_offset < 0)
		return -ENODEV;

	for (offset = fdt_first_subnode(fdt, cpus_offset);
	     offset >= 0;
	     offset = fdt_next_subnode(fdt, offset)) {
		prop = fdt_getprop(fdt, offset, "device_type", NULL);
		if (!prop || strcmp(prop, "cpu"))
			continue;

		/*
		 * In the first loop, we check if every CPU node specifies
		 * spin-table.  Otherwise, just return successfully to not
		 * disturb other methods, like psci.
		 */
		prop = fdt_getprop(fdt, offset, "enable-method", NULL);
		if (!prop || strcmp(prop, "spin-table"))
			return 0;
	}

	for (offset = fdt_first_subnode(fdt, cpus_offset);
	     offset >= 0;
	     offset = fdt_next_subnode(fdt, offset)) {
		prop = fdt_getprop(fdt, offset, "device_type", NULL);
		if (!prop || strcmp(prop, "cpu"))
			continue;

		ret = fdt_setprop_u64(fdt, offset, "cpu-release-addr",
				(unsigned long)&spin_table_cpu_release_addr);
		if (ret)
			return -ENOSPC;
	}

	ret = fdt_add_mem_rsv(fdt, rsv_addr, rsv_size);
	if (ret)
		return -ENOSPC;

	printf("   Reserved memory region for spin-table: addr=%lx size=%lx\n",
	       rsv_addr, rsv_size);

	return 0;
}
