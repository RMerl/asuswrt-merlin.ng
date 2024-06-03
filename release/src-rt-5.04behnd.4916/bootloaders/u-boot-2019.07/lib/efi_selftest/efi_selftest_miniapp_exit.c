// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_miniapp_exit
 *
 * Copyright (c) 2018 Heinrich Schuchardt
 *
 * This EFI application is run by the StartImage selftest.
 * It uses the Exit boot service to return.
 */

#include <common.h>
#include <efi_selftest.h>

static efi_guid_t loaded_image_protocol_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;

/**
 * check_loaded_image_protocol() - check image_base/image_size
 *
 * Try to open the loaded image protocol. Check that this function is located
 * between image_base and image_base + image_size.
 *
 * @image_handle:	handle of the loaded image
 * @systable:		system table
 * @return:		status code
 */
static efi_status_t EFIAPI check_loaded_image_protocol
		(efi_handle_t image_handle, struct efi_system_table *systable)
{
	struct efi_simple_text_output_protocol *cout = systable->con_out;
	struct efi_boot_services *boottime = systable->boottime;
	struct efi_loaded_image *loaded_image_protocol;
	efi_status_t ret;

	/*
	 * Open the loaded image protocol.
	 */
	ret = boottime->open_protocol
				(image_handle, &loaded_image_protocol_guid,
				 (void **)&loaded_image_protocol, NULL,
				  NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (ret != EFI_SUCCESS) {
		cout->output_string(cout,
				    L"Could not open loaded image protocol");
		return ret;
	}
	if ((void *)check_loaded_image_protocol <
	    loaded_image_protocol->image_base ||
	    (void *)check_loaded_image_protocol >=
	    loaded_image_protocol->image_base +
	    loaded_image_protocol->image_size) {
		cout->output_string(cout,
				    L"Incorrect image_base or image_size\n");
		return EFI_NOT_FOUND;
	}
	return EFI_SUCCESS;
}

/**
 * Entry point of the EFI application.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * @return:	status code
 */
efi_status_t EFIAPI efi_main(efi_handle_t handle,
			     struct efi_system_table *systable)
{
	struct efi_simple_text_output_protocol *con_out = systable->con_out;
	efi_status_t ret;
	u16 text[] = EFI_ST_SUCCESS_STR;

	con_out->output_string(con_out, L"EFI application calling Exit\n");

	if (check_loaded_image_protocol(handle, systable) != EFI_SUCCESS) {
		con_out->output_string(con_out,
				       L"Loaded image protocol missing\n");
		ret = EFI_NOT_FOUND;
		goto out;
	}

	/* This return value is expected by the calling test */
	ret = EFI_UNSUPPORTED;
out:
	systable->boottime->exit(handle, ret, sizeof(text), text);

	/*
	 * This statement should not be reached.
	 * To enable testing use a different return value.
	 */
	return EFI_SUCCESS;
}
