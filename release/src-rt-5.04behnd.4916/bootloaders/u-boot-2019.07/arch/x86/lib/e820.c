// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <efi_loader.h>
#include <asm/e820.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Install a default e820 table with 4 entries as follows:
 *
 *	0x000000-0x0a0000	Useable RAM
 *	0x0a0000-0x100000	Reserved for ISA
 *	0x100000-gd->ram_size	Useable RAM
 *	CONFIG_PCIE_ECAM_BASE	PCIe ECAM
 */
__weak unsigned int install_e820_map(unsigned int max_entries,
				     struct e820_entry *entries)
{
	entries[0].addr = 0;
	entries[0].size = ISA_START_ADDRESS;
	entries[0].type = E820_RAM;
	entries[1].addr = ISA_START_ADDRESS;
	entries[1].size = ISA_END_ADDRESS - ISA_START_ADDRESS;
	entries[1].type = E820_RESERVED;
	entries[2].addr = ISA_END_ADDRESS;
	entries[2].size = gd->ram_size - ISA_END_ADDRESS;
	entries[2].type = E820_RAM;
	entries[3].addr = CONFIG_PCIE_ECAM_BASE;
	entries[3].size = CONFIG_PCIE_ECAM_SIZE;
	entries[3].type = E820_RESERVED;

	return 4;
}

#if CONFIG_IS_ENABLED(EFI_LOADER)
void efi_add_known_memory(void)
{
	struct e820_entry e820[E820MAX];
	unsigned int i, num;
	u64 start, pages;
	int type;

	num = install_e820_map(ARRAY_SIZE(e820), e820);

	for (i = 0; i < num; ++i) {
		start = e820[i].addr;
		pages = ALIGN(e820[i].size, EFI_PAGE_SIZE) >> EFI_PAGE_SHIFT;

		switch (e820[i].type) {
		case E820_RAM:
			type = EFI_CONVENTIONAL_MEMORY;
			break;
		case E820_RESERVED:
			type = EFI_RESERVED_MEMORY_TYPE;
			break;
		case E820_ACPI:
			type = EFI_ACPI_RECLAIM_MEMORY;
			break;
		case E820_NVS:
			type = EFI_ACPI_MEMORY_NVS;
			break;
		case E820_UNUSABLE:
		default:
			type = EFI_UNUSABLE_MEMORY;
			break;
		}

		efi_add_memory_map(start, pages, type, false);
	}
}
#endif /* CONFIG_IS_ENABLED(EFI_LOADER) */
