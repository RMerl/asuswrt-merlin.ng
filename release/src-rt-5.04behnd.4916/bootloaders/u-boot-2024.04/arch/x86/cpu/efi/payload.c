// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <cpu_func.h>
#include <efi.h>
#include <efi_api.h>
#include <errno.h>
#include <event.h>
#include <init.h>
#include <log.h>
#include <usb.h>
#include <asm/bootparam.h>
#include <asm/e820.h>
#include <asm/global_data.h>
#include <asm/post.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * This function looks for the highest region of memory lower than 4GB which
 * has enough space for U-Boot where U-Boot is aligned on a page boundary.
 * It overrides the default implementation found elsewhere which simply
 * picks the end of ram, wherever that may be. The location of the stack,
 * the relocation address, and how far U-Boot is moved by relocation are
 * set in the global data structure.
 */
phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	struct efi_mem_desc *desc, *end;
	struct efi_entry_memmap *map;
	int ret, size;
	uintptr_t dest_addr = 0;
	struct efi_mem_desc *largest = NULL;

	/*
	 * Find largest area of memory below 4GB. We could
	 * call efi_build_mem_table() for a more accurate picture since it
	 * merges areas together where possible. But that function uses more
	 * pre-relocation memory, and it's not critical that we find the
	 * absolute largest region.
	 */
	ret = efi_info_get(EFIET_MEMORY_MAP, (void **)&map, &size);
	if (ret) {
		/* We should have stopped in dram_init(), something is wrong */
		debug("%s: Missing memory map\n", __func__);
		goto err;
	}

	end = (struct efi_mem_desc *)((ulong)map + size);
	desc = map->desc;
	for (; desc < end; desc = efi_get_next_mem_desc(desc, map->desc_size)) {
		if (desc->type != EFI_CONVENTIONAL_MEMORY ||
		    desc->physical_start >= 1ULL << 32)
			continue;
		if (!largest || desc->num_pages > largest->num_pages)
			largest = desc;
	}

	/* If no suitable area was found, return an error. */
	assert(largest);
	if (!largest || (largest->num_pages << EFI_PAGE_SHIFT) < (2 << 20))
		goto err;

	dest_addr = largest->physical_start + (largest->num_pages <<
			EFI_PAGE_SHIFT);

	return (ulong)dest_addr;
err:
	panic("No available memory found for relocation");
	return 0;
}

int dram_init(void)
{
	struct efi_mem_desc *desc, *end;
	struct efi_entry_memmap *map;
	int size, ret;

	ret = efi_info_get(EFIET_MEMORY_MAP, (void **)&map, &size);
	if (ret) {
		printf("Cannot find EFI memory map tables, ret=%d\n", ret);

		return -ENODEV;
	}

	end = (struct efi_mem_desc *)((ulong)map + size);
	gd->ram_size = 0;
	desc = map->desc;
	for (; desc < end; desc = efi_get_next_mem_desc(desc, map->desc_size)) {
		if (desc->type < EFI_MMAP_IO)
			gd->ram_size += desc->num_pages << EFI_PAGE_SHIFT;
	}

	return 0;
}

int dram_init_banksize(void)
{
	struct efi_mem_desc *desc, *end;
	struct efi_entry_memmap *map;
	int ret, size;
	int num_banks;

	ret = efi_info_get(EFIET_MEMORY_MAP, (void **)&map, &size);
	if (ret) {
		/* We should have stopped in dram_init(), something is wrong */
		debug("%s: Missing memory map\n", __func__);
		return -ENXIO;
	}
	end = (struct efi_mem_desc *)((ulong)map + size);
	desc = map->desc;
	for (num_banks = 0;
	     desc < end && num_banks < CONFIG_NR_DRAM_BANKS;
	     desc = efi_get_next_mem_desc(desc, map->desc_size)) {
		/*
		 * We only use conventional memory and ignore
		 * anything less than 1MB.
		 */
		if (desc->type != EFI_CONVENTIONAL_MEMORY ||
		    (desc->num_pages << EFI_PAGE_SHIFT) < 1 << 20)
			continue;
		gd->bd->bi_dram[num_banks].start = desc->physical_start;
		gd->bd->bi_dram[num_banks].size = desc->num_pages <<
			EFI_PAGE_SHIFT;
		num_banks++;
	}

	return 0;
}

int arch_cpu_init(void)
{
	post_code(POST_CPU_INIT);

	return x86_cpu_init_f();
}

int checkcpu(void)
{
	return 0;
}

int print_cpuinfo(void)
{
	return default_print_cpuinfo();
}

/* Find any available tables and copy them to a safe place */
int reserve_arch(void)
{
	struct efi_info_hdr *hdr;

	debug("table=%lx\n", gd->arch.table);
	if (!gd->arch.table)
		return 0;

	hdr = (struct efi_info_hdr *)gd->arch.table;

	gd->start_addr_sp -= hdr->total_size;
	memcpy((void *)gd->start_addr_sp, hdr, hdr->total_size);
	debug("Stashing EFI table at %lx to %lx, size %x\n",
	      gd->arch.table, gd->start_addr_sp, hdr->total_size);
	gd->arch.table = gd->start_addr_sp;

	return 0;
}

static int last_stage_init(void)
{
	/* start usb so that usb keyboard can be used as input device */
	if (IS_ENABLED(CONFIG_USB_KEYBOARD))
		usb_init();

	return 0;
}
EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, last_stage_init);

unsigned int install_e820_map(unsigned int max_entries,
			      struct e820_entry *entries)
{
	struct efi_mem_desc *desc, *end;
	struct efi_entry_memmap *map;
	int size, ret;
	efi_physical_addr_t last_end_addr = 0;
	struct e820_entry *last_entry = NULL;
	__u32 e820_type;
	unsigned int num_entries = 0;

	ret = efi_info_get(EFIET_MEMORY_MAP, (void **)&map, &size);
	if (ret) {
		printf("Cannot find EFI memory map tables, ret=%d\n", ret);

		return -ENODEV;
	}

	end = (struct efi_mem_desc *)((ulong)map + size);
	for (desc = map->desc; desc < end;
	     desc = efi_get_next_mem_desc(desc, map->desc_size)) {
		if (desc->num_pages == 0)
			continue;

		switch (desc->type) {
		case EFI_LOADER_CODE:
		case EFI_LOADER_DATA:
		case EFI_BOOT_SERVICES_CODE:
		case EFI_BOOT_SERVICES_DATA:
		case EFI_CONVENTIONAL_MEMORY:
			e820_type = E820_RAM;
			break;

		case EFI_RESERVED_MEMORY_TYPE:
		case EFI_RUNTIME_SERVICES_CODE:
		case EFI_RUNTIME_SERVICES_DATA:
		case EFI_MMAP_IO:
		case EFI_MMAP_IO_PORT:
		case EFI_PAL_CODE:
			e820_type = E820_RESERVED;
			break;

		case EFI_ACPI_RECLAIM_MEMORY:
			e820_type = E820_ACPI;
			break;

		case EFI_ACPI_MEMORY_NVS:
			e820_type = E820_NVS;
			break;

		case EFI_UNUSABLE_MEMORY:
			e820_type = E820_UNUSABLE;
			break;

		default:
			printf("Invalid EFI memory descriptor type (0x%x)!\n",
			       desc->type);
			continue;
		}

		if (last_entry != NULL && last_entry->type == e820_type &&
		    desc->physical_start == last_end_addr) {
			last_entry->size += (desc->num_pages << EFI_PAGE_SHIFT);
			last_end_addr += (desc->num_pages << EFI_PAGE_SHIFT);
		} else {
			if (num_entries >= E820MAX)
				break;

			entries[num_entries].addr = desc->physical_start;
			entries[num_entries].size = desc->num_pages;
			entries[num_entries].size <<= EFI_PAGE_SHIFT;
			entries[num_entries].type = e820_type;
			last_entry = &entries[num_entries];
			last_end_addr = last_entry->addr + last_entry->size;
			num_entries++;
		}
	}

	return num_entries;
}

void setup_efi_info(struct efi_info *efi_info)
{
	struct efi_entry_systable *table;
	struct efi_entry_memmap *map;
	char *signature;
	int size, ret;

	memset(efi_info, 0, sizeof(struct efi_info));

	ret = efi_info_get(EFIET_SYS_TABLE, (void **)&table, &size);
	if (ret) {
		printf("Cannot find EFI system table, ret=%d\n", ret);
		return;
	}
	efi_info->efi_systab = (u32)(table->sys_table);

	ret = efi_info_get(EFIET_MEMORY_MAP, (void **)&map, &size);
	if (ret) {
		printf("Cannot find EFI memory map tables, ret=%d\n", ret);
		return;
	}
	efi_info->efi_memdesc_size = map->desc_size;
	efi_info->efi_memdesc_version = map->version;
	efi_info->efi_memmap = (ulong)(map->desc);
	efi_info->efi_memmap_size = size - sizeof(struct efi_entry_memmap);

#ifdef CONFIG_EFI_STUB_64BIT
	efi_info->efi_systab_hi = table->sys_table >> 32;
	efi_info->efi_memmap_hi = (u64)(ulong)map->desc >> 32;
	signature = EFI64_LOADER_SIGNATURE;
#else
	signature = EFI32_LOADER_SIGNATURE;
#endif
	memcpy(&efi_info->efi_loader_signature, signature, 4);
}

void efi_show_bdinfo(void)
{
	struct efi_entry_systable *table = NULL;
	struct efi_system_table *sys_table;
	int size, ret;

	ret = efi_info_get(EFIET_SYS_TABLE, (void **)&table, &size);
	if (!ret) {
		bdinfo_print_num_l("efi_table", table->sys_table);
		sys_table = (struct efi_system_table *)(uintptr_t)
			table->sys_table;
		bdinfo_print_num_l(" revision", sys_table->fw_revision);
	}
}
