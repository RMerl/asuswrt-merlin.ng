// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI application ACPI tables support
 *
 *  Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <efi_loader.h>
#include <asm/acpi_table.h>

static const efi_guid_t acpi_guid = EFI_ACPI_TABLE_GUID;

/*
 * Install the ACPI table as a configuration table.
 *
 * @return	status code
 */
efi_status_t efi_acpi_register(void)
{
	/* Map within the low 32 bits, to allow for 32bit ACPI tables */
	u64 acpi = U32_MAX;
	efi_status_t ret;

	/* Reserve 64kiB page for ACPI */
	ret = efi_allocate_pages(EFI_ALLOCATE_MAX_ADDRESS,
				 EFI_RUNTIME_SERVICES_DATA, 16, &acpi);
	if (ret != EFI_SUCCESS)
		return ret;

	/*
	 * Generate ACPI tables - we know that efi_allocate_pages() returns
	 * a 4k-aligned address, so it is safe to assume that
	 * write_acpi_tables() will write the table at that address.
	 */
	assert(!(acpi & 0xf));
	write_acpi_tables(acpi);

	/* And expose them to our EFI payload */
	return efi_install_configuration_table(&acpi_guid,
					       (void *)(uintptr_t)acpi);
}
