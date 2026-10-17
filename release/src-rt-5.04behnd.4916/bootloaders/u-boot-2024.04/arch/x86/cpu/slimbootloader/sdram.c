// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Intel Corporation <www.intel.com>
 */

#include <common.h>
#include <init.h>
#include <asm/global_data.h>
#include <linux/sizes.h>
#include <asm/e820.h>
#include <asm/arch/slimbootloader.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * This returns a data pointer of memory map info from the guid hob.
 *
 * @return: A data pointer of memory map info hob
 */
static struct sbl_memory_map_info *get_memory_map_info(void)
{
	struct sbl_memory_map_info *data;
	const efi_guid_t guid = SBL_MEMORY_MAP_INFO_GUID;

	if (!gd->arch.hob_list)
		return NULL;

	data = hob_get_guid_hob_data(gd->arch.hob_list, NULL, &guid);
	if (!data)
		panic("memory map info hob not found\n");
	if (!data->count)
		panic("invalid number of memory map entries\n");

	return data;
}

#define for_each_if(condition) if (!(condition)) {} else

#define for_each_memory_map_entry_reversed(iter, entries) \
	for (iter = entries->count - 1; iter >= 0; iter--) \
		for_each_if(entries->entry[iter].type == E820_RAM)

/**
 * This is to give usable memory region information for u-boot relocation.
 * so search usable memory region lower than 4GB.
 * The memory map entries from Slim Bootloader hob are already sorted.
 *
 * @total_size: The memory size that u-boot occupies
 * Return:    : The top available memory address lower than 4GB
 */
phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	struct sbl_memory_map_info *data;
	int i;
	u64 addr_start;
	u64 addr_end;
	ulong ram_top;

	data = get_memory_map_info();

	/**
	 * sorted memory map entries from Slim Bootloader based on physical
	 * start memory address, from low to high. So do reversed search to
	 * get highest usable, suitable size, 4KB aligned available memory
	 * under 4GB.
	 */
	ram_top = 0;
	for_each_memory_map_entry_reversed(i, data) {
		addr_start = data->entry[i].addr;
		addr_end = addr_start + data->entry[i].size;

		if (addr_start > SZ_4G)
			continue;

		if (addr_end > SZ_4G)
			addr_end = SZ_4G;

		if (addr_end < total_size)
			continue;

		/* to relocate u-boot at 4K aligned memory */
		addr_end = rounddown(addr_end - total_size, SZ_4K);
		if (addr_end >= addr_start) {
			ram_top = (ulong)addr_end + total_size;
			break;
		}
	}

	if (!ram_top)
		panic("failed to find available memory for relocation!");

	return ram_top;
}

/**
 * The memory initialization has already been done in previous Slim Bootloader
 * stage thru FSP-M. Instead, this sets the ram_size from the memory map info
 * hob.
 */
int dram_init(void)
{
	struct sbl_memory_map_info *data;
	int i;
	u64 ram_size;

	data = get_memory_map_info();

	/**
	 * sorted memory map entries from Slim Bootloader based on physical
	 * start memory address, from low to high. So do reversed search to
	 * simply get highest usable memory address as RAM size
	 */
	ram_size = 0;
	for_each_memory_map_entry_reversed(i, data) {
		/* simply use the highest usable memory address as RAM size */
		ram_size = data->entry[i].addr + data->entry[i].size;
		break;
	}

	if (!ram_size)
		panic("failed to detect memory size");

	gd->ram_size = ram_size;
	return 0;
}

int dram_init_banksize(void)
{
	if (!CONFIG_NR_DRAM_BANKS)
		return 0;

	/* simply use a single bank to have whole size for now */
	gd->bd->bi_dram[0].start = 0;
	gd->bd->bi_dram[0].size = gd->ram_size;
	return 0;
}

unsigned int install_e820_map(unsigned int max_entries,
			      struct e820_entry *entries)
{
	struct sbl_memory_map_info *data;
	unsigned int i;

	data = get_memory_map_info();

	for (i = 0; i < data->count; i++) {
		entries[i].addr = data->entry[i].addr;
		entries[i].size = data->entry[i].size;
		entries[i].type = data->entry[i].type;
	}

	return i;
}
