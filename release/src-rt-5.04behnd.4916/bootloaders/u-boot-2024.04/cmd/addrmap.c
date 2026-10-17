// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <command.h>
#include <addr_map.h>

static int do_addrmap(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	int i;

	printf("           vaddr            paddr             size\n");
	printf("================ ================ ================\n");

	for (i = 0; i < CONFIG_SYS_NUM_ADDR_MAP; i++) {
		if (address_map[i].size == 0)
			continue;

		printf("%16.8lx %16.8llx %16.8llx\n",
		       address_map[i].vaddr,
		       (unsigned long long)address_map[i].paddr,
		       (unsigned long long)address_map[i].size);
	}

	return 0;
}

U_BOOT_CMD(
	addrmap,	1,	1,	do_addrmap,
	"List non-identity virtual-physical memory mappings for 32-bit CPUs",
	""
);
