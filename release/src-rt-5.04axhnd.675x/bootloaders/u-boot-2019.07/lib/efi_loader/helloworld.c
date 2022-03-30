// SPDX-License-Identifier: GPL-2.0+
/*
 * EFI hello world
 *
 * Copyright (c) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * This program demonstrates calling a boottime service.
 * It writes a greeting and the load options to the console.
 */

#include <common.h>
#include <efi_api.h>

static const efi_guid_t loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
static const efi_guid_t fdt_guid = EFI_FDT_GUID;
static const efi_guid_t acpi_guid = EFI_ACPI_TABLE_GUID;
static const efi_guid_t smbios_guid = SMBIOS_TABLE_GUID;

/**
 * efi_main() - entry point of the EFI application.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * @return:	status code
 */
efi_status_t EFIAPI efi_main(efi_handle_t handle,
			     struct efi_system_table *systable)
{
	struct efi_simple_text_output_protocol *con_out = systable->con_out;
	struct efi_boot_services *boottime = systable->boottime;
	struct efi_loaded_image *loaded_image;
	efi_status_t ret;
	efi_uintn_t i;
	u16 rev[] = L"0.0.0";

	/* UEFI requires CR LF */
	con_out->output_string(con_out, L"Hello, world!\r\n");

	/* Print the revision number */
	rev[0] = (systable->hdr.revision >> 16) + '0';
	rev[4] = systable->hdr.revision & 0xffff;
	for (; rev[4] >= 10;) {
		rev[4] -= 10;
		++rev[2];
	}
	/* Third digit is only to be shown if non-zero */
	if (rev[4])
		rev[4] += '0';
	else
		rev[3] = 0;

	con_out->output_string(con_out, L"Running on UEFI ");
	con_out->output_string(con_out, rev);
	con_out->output_string(con_out, L"\r\n");

	/* Get the loaded image protocol */
	ret = boottime->handle_protocol(handle, &loaded_image_guid,
					(void **)&loaded_image);
	if (ret != EFI_SUCCESS) {
		con_out->output_string
			(con_out, L"Cannot open loaded image protocol\r\n");
		goto out;
	}
	/* Find configuration tables */
	for (i = 0; i < systable->nr_tables; ++i) {
		if (!memcmp(&systable->tables[i].guid, &fdt_guid,
			    sizeof(efi_guid_t)))
			con_out->output_string
					(con_out, L"Have device tree\r\n");
		if (!memcmp(&systable->tables[i].guid, &acpi_guid,
			    sizeof(efi_guid_t)))
			con_out->output_string
					(con_out, L"Have ACPI 2.0 table\r\n");
		if (!memcmp(&systable->tables[i].guid, &smbios_guid,
			    sizeof(efi_guid_t)))
			con_out->output_string
					(con_out, L"Have SMBIOS table\r\n");
	}
	/* Output the load options */
	con_out->output_string(con_out, L"Load options: ");
	if (loaded_image->load_options_size && loaded_image->load_options)
		con_out->output_string(con_out,
				       (u16 *)loaded_image->load_options);
	else
		con_out->output_string(con_out, L"<none>");
	con_out->output_string(con_out, L"\r\n");

out:
	boottime->exit(handle, ret, 0, NULL);

	/* We should never arrive here */
	return ret;
}
