// SPDX-License-Identifier: GPL-2.0+
/*
 * QEMU x86 specific E820 table generation
 *
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 * (C) Copyright 2019 Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <env_internal.h>
#include <malloc.h>
#include <asm/e820.h>
#include <asm/arch/qemu.h>
#include <asm/global_data.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned int install_e820_map(unsigned int max_entries,
			      struct e820_entry *entries)
{
	u64 high_mem_size;
	int n = 0;

	entries[n].addr = 0;
	entries[n].size = ISA_START_ADDRESS;
	entries[n].type = E820_RAM;
	n++;

	entries[n].addr = ISA_START_ADDRESS;
	entries[n].size = ISA_END_ADDRESS - ISA_START_ADDRESS;
	entries[n].type = E820_RESERVED;
	n++;

	/*
	 * since we use memalign(malloc) to allocate high memory for
	 * storing ACPI tables, we need to reserve them in e820 tables,
	 * otherwise kernel will reclaim them and data will be corrupted
	 */
	entries[n].addr = ISA_END_ADDRESS;
	entries[n].size = gd->relocaddr - TOTAL_MALLOC_LEN - ISA_END_ADDRESS;
	entries[n].type = E820_RAM;
	n++;

	/* for simplicity, reserve entire malloc space */
	entries[n].addr = gd->relocaddr - TOTAL_MALLOC_LEN;
	entries[n].size = TOTAL_MALLOC_LEN;
	entries[n].type = E820_RESERVED;
	n++;

	entries[n].addr = gd->relocaddr;
	entries[n].size = qemu_get_low_memory_size() - gd->relocaddr;
	entries[n].type = E820_RESERVED;
	n++;

	entries[n].addr = CONFIG_PCIE_ECAM_BASE;
	entries[n].size = CONFIG_PCIE_ECAM_SIZE;
	entries[n].type = E820_RESERVED;
	n++;

	high_mem_size = qemu_get_high_memory_size();
	if (high_mem_size) {
		entries[n].addr = SZ_4G;
		entries[n].size = high_mem_size;
		entries[n].type = E820_RAM;
		n++;
	}

	return n;
}
