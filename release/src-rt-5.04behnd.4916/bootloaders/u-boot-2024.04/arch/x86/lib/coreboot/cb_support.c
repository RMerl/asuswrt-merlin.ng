// SPDX-License-Identifier: GPL-2.0+
/*
 * Support for booting from coreboot
 *
 * Copyright 2021 Google LLC
 */

#include <common.h>
#include <asm/cb_sysinfo.h>
#include <asm/e820.h>

unsigned int cb_install_e820_map(unsigned int max_entries,
				 struct e820_entry *entries)
{
	unsigned int num_entries;
	int i;

	num_entries = min((unsigned int)lib_sysinfo.n_memranges, max_entries);
	if (num_entries < lib_sysinfo.n_memranges) {
		printf("Warning: Limiting e820 map to %d entries\n",
		       num_entries);
	}
	for (i = 0; i < num_entries; i++) {
		struct memrange *memrange = &lib_sysinfo.memrange[i];

		entries[i].addr = memrange->base;
		entries[i].size = memrange->size;

		/*
		 * coreboot has some extensions (type 6 & 16) to the E820 types.
		 * When we detect this, mark it as E820_RESERVED.
		 */
		if (memrange->type == CB_MEM_VENDOR_RSVD ||
		    memrange->type == CB_MEM_TABLE)
			entries[i].type = E820_RESERVED;
		else
			entries[i].type = memrange->type;
	}

	return num_entries;
}
