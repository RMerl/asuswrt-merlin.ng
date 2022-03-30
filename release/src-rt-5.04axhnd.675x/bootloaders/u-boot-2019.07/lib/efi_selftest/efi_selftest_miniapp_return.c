// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_miniapp_return
 *
 * Copyright (c) 2018 Heinrich Schuchardt
 *
 * This EFI application is run by the StartImage selftest.
 * It returns directly without calling the Exit boot service.
 */

#include <common.h>
#include <efi_api.h>

/*
 * Entry point of the EFI application.
 *
 * @handle	handle of the loaded image
 * @systable	system table
 * @return	status code
 */
efi_status_t EFIAPI efi_main(efi_handle_t handle,
			     struct efi_system_table *systable)
{
	struct efi_simple_text_output_protocol *con_out = systable->con_out;

	con_out->output_string(con_out,
			       L"EFI application returning w/o calling Exit\n");

	/* The return value is checked by the calling test */
	return EFI_INCOMPATIBLE_VERSION;
}
