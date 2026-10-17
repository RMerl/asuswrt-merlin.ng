// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_memory
 *
 * Copyright (c) 2018 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This unit test checks the following boottime services:
 * AllocatePages, FreePages, GetMemoryMap
 *
 * The memory type used for the device tree is checked.
 */

#include <efi_selftest.h>

#define EFI_ST_NUM_PAGES 8

static const efi_guid_t fdt_guid = EFI_FDT_GUID;
static struct efi_boot_services *boottime;
static u64 fdt_addr;

/**
 * setup() - setup unit test
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * Return:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	size_t i;

	boottime = systable->boottime;

	for (i = 0; i < systable->nr_tables; ++i) {
		if (!memcmp(&systable->tables[i].guid, &fdt_guid,
			    sizeof(efi_guid_t))) {
			if (fdt_addr) {
				efi_st_error("Duplicate device tree\n");
				return EFI_ST_FAILURE;
			}
			fdt_addr = (uintptr_t)systable->tables[i].table;
		}
	}
	return EFI_ST_SUCCESS;
}

/**
 * find_in_memory_map() - check matching memory map entry exists
 *
 * @memory_map:		memory map
 * @desc_size:		number of memory map entries
 * @addr:		physical address to find in the map
 * @type:		expected memory type
 * Return:		EFI_ST_SUCCESS for success
 */
static int find_in_memory_map(efi_uintn_t map_size,
			      struct efi_mem_desc *memory_map,
			      efi_uintn_t desc_size,
			      u64 addr, int memory_type)
{
	efi_uintn_t i;
	bool found = false;

	for (i = 0; map_size; ++i, map_size -= desc_size) {
		struct efi_mem_desc *entry = &memory_map[i];

		if (entry->physical_start != entry->virtual_start) {
			efi_st_error("Physical and virtual addresses do not match\n");
			return EFI_ST_FAILURE;
		}

		if (addr >= entry->physical_start &&
		    addr < entry->physical_start +
			    (entry->num_pages << EFI_PAGE_SHIFT)) {
			if (found) {
				efi_st_error("Duplicate memory map entry\n");
				return EFI_ST_FAILURE;
			}
			found = true;
			if (memory_type != entry->type) {
				efi_st_error
					("Wrong memory type %d, expected %d\n",
					 entry->type, memory_type);
				return EFI_ST_FAILURE;
			}
		}
	}
	if (!found) {
		efi_st_error("Missing memory map entry\n");
		return EFI_ST_FAILURE;
	}
	return EFI_ST_SUCCESS;
}

/*
 * execute() - execute unit test
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	u64 p1;
	u64 p2;
	efi_uintn_t map_size = 0;
	efi_uintn_t map_key;
	efi_uintn_t desc_size;
	u32 desc_version;
	struct efi_mem_desc *memory_map;
	efi_status_t ret;

	/* Allocate two page ranges with different memory type */
	ret = boottime->allocate_pages(EFI_ALLOCATE_ANY_PAGES,
				       EFI_RUNTIME_SERVICES_CODE,
				       EFI_ST_NUM_PAGES, &p1);
	if (ret != EFI_SUCCESS) {
		efi_st_error("AllocatePages did not return EFI_SUCCESS\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->allocate_pages(EFI_ALLOCATE_ANY_PAGES,
				       EFI_RUNTIME_SERVICES_DATA,
				       EFI_ST_NUM_PAGES, &p2);
	if (ret != EFI_SUCCESS) {
		efi_st_error("AllocatePages did not return EFI_SUCCESS\n");
		return EFI_ST_FAILURE;
	}

	/* Load memory map */
	ret = boottime->get_memory_map(&map_size, NULL, &map_key, &desc_size,
				       &desc_version);
	if (ret != EFI_BUFFER_TOO_SMALL) {
		efi_st_error
			("GetMemoryMap did not return EFI_BUFFER_TOO_SMALL\n");
		return EFI_ST_FAILURE;
	}
	/* Allocate extra space for newly allocated memory */
	map_size += sizeof(struct efi_mem_desc);
	ret = boottime->allocate_pool(EFI_BOOT_SERVICES_DATA, map_size,
				      (void **)&memory_map);
	if (ret != EFI_SUCCESS) {
		efi_st_error("AllocatePool did not return EFI_SUCCESS\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->get_memory_map(&map_size, memory_map, &map_key,
				       &desc_size, &desc_version);
	if (ret != EFI_SUCCESS) {
		efi_st_error("GetMemoryMap did not return EFI_SUCCESS\n");
		return EFI_ST_FAILURE;
	}

	/* Check memory map entries */
	if (find_in_memory_map(map_size, memory_map, desc_size, p1,
			       EFI_RUNTIME_SERVICES_CODE) != EFI_ST_SUCCESS)
		return EFI_ST_FAILURE;
	if (find_in_memory_map(map_size, memory_map, desc_size, p2,
			       EFI_RUNTIME_SERVICES_DATA) != EFI_ST_SUCCESS)
		return EFI_ST_FAILURE;

	/* Free memory */
	ret = boottime->free_pages(p1, EFI_ST_NUM_PAGES);
	if (ret != EFI_SUCCESS) {
		efi_st_error("FreePages did not return EFI_SUCCESS\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->free_pages(p2, EFI_ST_NUM_PAGES);
	if (ret != EFI_SUCCESS) {
		efi_st_error("FreePages did not return EFI_SUCCESS\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->free_pool(memory_map);
	if (ret != EFI_SUCCESS) {
		efi_st_error("FreePool did not return EFI_SUCCESS\n");
		return EFI_ST_FAILURE;
	}

	/* Check memory reservation for the device tree */
	if (fdt_addr &&
	    find_in_memory_map(map_size, memory_map, desc_size, fdt_addr,
			       EFI_ACPI_RECLAIM_MEMORY) != EFI_ST_SUCCESS) {
		efi_st_error
			("Device tree not marked as ACPI reclaim memory\n");
		return EFI_ST_FAILURE;
	}
	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(memory) = {
	.name = "memory",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
};
