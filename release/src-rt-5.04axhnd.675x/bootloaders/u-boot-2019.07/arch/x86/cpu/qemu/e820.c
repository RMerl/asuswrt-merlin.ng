// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 */

#include <common.h>
#include <asm/e820.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned int install_e820_map(unsigned int max_entries,
			      struct e820_entry *entries)
{
	entries[0].addr = 0;
	entries[0].size = ISA_START_ADDRESS;
	entries[0].type = E820_RAM;

	entries[1].addr = ISA_START_ADDRESS;
	entries[1].size = ISA_END_ADDRESS - ISA_START_ADDRESS;
	entries[1].type = E820_RESERVED;

	/*
	 * since we use memalign(malloc) to allocate high memory for
	 * storing ACPI tables, we need to reserve them in e820 tables,
	 * otherwise kernel will reclaim them and data will be corrupted
	 */
	entries[2].addr = ISA_END_ADDRESS;
	entries[2].size = gd->relocaddr - TOTAL_MALLOC_LEN - ISA_END_ADDRESS;
	entries[2].type = E820_RAM;

	/* for simplicity, reserve entire malloc space */
	entries[3].addr = gd->relocaddr - TOTAL_MALLOC_LEN;
	entries[3].size = TOTAL_MALLOC_LEN;
	entries[3].type = E820_RESERVED;

	entries[4].addr = gd->relocaddr;
	entries[4].size = gd->ram_size - gd->relocaddr;
	entries[4].type = E820_RESERVED;

	entries[5].addr = CONFIG_PCIE_ECAM_BASE;
	entries[5].size = CONFIG_PCIE_ECAM_SIZE;
	entries[5].type = E820_RESERVED;

	return 6;
}
