// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_set_virtual_address_map.c
 *
 * Copyright (c) 2019 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This test checks the notification of EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE
 * and the following services: SetVirtualAddressMap, ConvertPointer.
 */

#include <efi_selftest.h>

static const struct efi_boot_services *boottime;
static const struct efi_runtime_services *runtime;
static struct efi_event *event;
static struct efi_mem_desc *memory_map;
static efi_uintn_t map_size;
static efi_uintn_t desc_size;
static u32 desc_version;
static u64 page1;
static u64 page2;
static u32 notify_call_count;
static bool convert_pointer_failed;

/**
 * notify() - notification function
 *
 * This function is called when the EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE event
 * occurs. The correct output of ConvertPointer() is checked.
 *
 * @event	notified event
 * @context	pointer to the notification count
 */
static void EFIAPI notify(struct efi_event *event, void *context)
{
	void *addr;
	efi_status_t ret;

	++notify_call_count;

	addr = (void *)(uintptr_t)page1;
	ret = runtime->convert_pointer(0, &addr);
	if (ret != EFI_SUCCESS) {
		efi_st_error("ConvertPointer failed\n");
		convert_pointer_failed = true;
		return;
	}
	if ((uintptr_t)addr != page1 + EFI_PAGE_SIZE) {
		efi_st_error("ConvertPointer wrong address\n");
		convert_pointer_failed = true;
		return;
	}

	addr = (void *)(uintptr_t)page2;
	ret = runtime->convert_pointer(0, &addr);
	if (ret != EFI_SUCCESS) {
		efi_st_error("ConvertPointer failed\n");
		convert_pointer_failed = true;
		return;
	}
	if ((uintptr_t)addr != page2 + 2 * EFI_PAGE_SIZE) {
		efi_st_error("ConvertPointer wrong address\n");
		convert_pointer_failed = true;
	}
}

/**
 * setup() - setup unit test
 *
 * The memory map is read. Boottime only entries are deleted. Two entries for
 * newly allocated pages are added. For these virtual addresses deviating from
 * the physical addresses are set.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * Return:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	efi_uintn_t map_key;
	efi_status_t ret;
	struct efi_mem_desc *end, *pos1, *pos2;

	boottime = systable->boottime;
	runtime = systable->runtime;

	ret = boottime->create_event(EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE,
				     TPL_CALLBACK, notify, NULL,
				     &event);
	if (ret != EFI_SUCCESS) {
		efi_st_error("could not create event\n");
		return EFI_ST_FAILURE;
	}

	ret = boottime->get_memory_map(&map_size, NULL, &map_key, &desc_size,
				       &desc_version);
	if (ret != EFI_BUFFER_TOO_SMALL) {
		efi_st_error(
			"GetMemoryMap did not return EFI_BUFFER_TOO_SMALL\n");
		return EFI_ST_FAILURE;
	}
	/* Allocate extra space for newly allocated memory */
	map_size += 3 * sizeof(struct efi_mem_desc);
	ret = boottime->allocate_pool(EFI_BOOT_SERVICES_DATA, map_size,
				      (void **)&memory_map);
	if (ret != EFI_SUCCESS) {
		efi_st_error("AllocatePool failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->get_memory_map(&map_size, memory_map, &map_key,
				       &desc_size, &desc_version);
	if (ret != EFI_SUCCESS) {
		efi_st_error("GetMemoryMap failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->allocate_pages(EFI_ALLOCATE_ANY_PAGES,
				       EFI_BOOT_SERVICES_DATA, 2, &page1);
	if (ret != EFI_SUCCESS) {
		efi_st_error("AllocatePages failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->allocate_pages(EFI_ALLOCATE_ANY_PAGES,
				       EFI_BOOT_SERVICES_DATA, 3, &page2);
	if (ret != EFI_SUCCESS) {
		efi_st_error("AllocatePages failed\n");
		return EFI_ST_FAILURE;
	}
	/* Remove entries not relevant for runtime from map */
	end = (struct efi_mem_desc *)((u8 *)memory_map + map_size);
	for (pos1 = memory_map, pos2 = memory_map;
	     pos2 < end; ++pos2) {
		switch (pos2->type) {
		case EFI_LOADER_CODE:
		case EFI_LOADER_DATA:
		case EFI_BOOT_SERVICES_CODE:
		case EFI_BOOT_SERVICES_DATA:
		case EFI_CONVENTIONAL_MEMORY:
			continue;
		}
		memcpy(pos1, pos2, desc_size);
		++pos1;
	}

	/*
	 * Add entries with virtual addresses deviating from the physical
	 * addresses. By choosing virtual address ranges within the allocated
	 * physical pages address space collisions are avoided.
	 */
	pos1->type = EFI_RUNTIME_SERVICES_DATA;
	pos1->reserved = 0;
	pos1->physical_start = page1;
	pos1->virtual_start = page1 + EFI_PAGE_SIZE;
	pos1->num_pages = 1;
	pos1->attribute = EFI_MEMORY_RUNTIME;
	++pos1;

	pos1->type = EFI_RUNTIME_SERVICES_DATA;
	pos1->reserved = 0;
	pos1->physical_start = page2;
	pos1->virtual_start = page2 + 2 * EFI_PAGE_SIZE;
	pos1->num_pages = 1;
	pos1->attribute = EFI_MEMORY_RUNTIME;
	++pos1;

	map_size = (u8 *)pos1 - (u8 *)memory_map;

	return EFI_ST_SUCCESS;
}

/**
 * execute() - execute unit test
 *
 * SetVirtualAddressMap() is called with the memory map prepared in setup().
 *
 * The triggering of the EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE event is checked via
 * the call count of the notification function.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	efi_status_t ret;

	ret = runtime->set_virtual_address_map(map_size, desc_size,
					       desc_version, memory_map);
	if (ret != EFI_SUCCESS) {
		efi_st_error("SetVirtualAddressMap failed\n");
		return EFI_ST_FAILURE;
	}
	if (notify_call_count != 1) {
		efi_st_error("EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE triggered %d times\n",
			     notify_call_count);
		return EFI_ST_FAILURE;
	}
	if (convert_pointer_failed)
		return EFI_ST_FAILURE;

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(virtaddrmap) = {
	.name = "virtual address map",
	.phase = EFI_SETTING_VIRTUAL_ADDRESS_MAP,
	.setup = setup,
	.execute = execute,
};
