// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI application ACPI tables support
 *
 *  Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <efi_loader.h>
#include <log.h>
#include <mapmem.h>
#include <acpi/acpi_table.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

static const efi_guid_t acpi_guid = EFI_ACPI_TABLE_GUID;

/*
 * Install the ACPI table as a configuration table.
 *
 * Return:	status code
 */
efi_status_t efi_acpi_register(void)
{
	ulong addr, start, end;
	efi_status_t ret;

	/* Mark space used for tables */
	start = ALIGN_DOWN(gd->arch.table_start, EFI_PAGE_MASK);
	end = ALIGN(gd->arch.table_end, EFI_PAGE_MASK);
	ret = efi_add_memory_map(start, end - start, EFI_ACPI_RECLAIM_MEMORY);
	if (ret != EFI_SUCCESS)
		return ret;
	if (gd->arch.table_start_high) {
		start = ALIGN_DOWN(gd->arch.table_start_high, EFI_PAGE_MASK);
		end = ALIGN(gd->arch.table_end_high, EFI_PAGE_MASK);
		ret = efi_add_memory_map(start, end - start,
					 EFI_ACPI_RECLAIM_MEMORY);
		if (ret != EFI_SUCCESS)
			return ret;
	}

	addr = gd_acpi_start();
	printf("EFI using ACPI tables at %lx\n", addr);

	/* And expose them to our EFI payload */
	return efi_install_configuration_table(&acpi_guid,
					       (void *)(ulong)addr);
}
