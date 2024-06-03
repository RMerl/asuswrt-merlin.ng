// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_loaded_image
 *
 * Copyright (c) 2018 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This unit test checks the Loaded Image Protocol.
 */

#include <efi_selftest.h>

static efi_guid_t loaded_image_protocol_guid =
	EFI_GUID(0x5b1b31a1, 0x9562, 0x11d2,
		 0x8e, 0x3f, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b);
static struct efi_boot_services *boottime;
efi_handle_t image_handle;

/*
 * Setup unit test.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 */
static int setup(const efi_handle_t img_handle,
		 const struct efi_system_table *systable)
{
	boottime = systable->boottime;
	image_handle = img_handle;

	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * Verify that the loaded image protocol is installed on the image handle.
 * Verify that the loaded image protocol points to the system table.
 */
static int execute(void)
{
	efi_status_t ret;
	efi_uintn_t i, protocol_buffer_count = 0;
	efi_guid_t **protocol_buffer = NULL;
	bool found = false;
	struct efi_loaded_image *loaded_image_protocol;

	/*
	 * Get the GUIDs of all protocols installed on the handle.
	 */
	ret = boottime->protocols_per_handle(image_handle, &protocol_buffer,
					     &protocol_buffer_count);
	if (ret != EFI_SUCCESS) {
		efi_st_error("ProtocolsPerHandle failed\n");
		return EFI_ST_FAILURE;
	}
	if (!protocol_buffer_count || !protocol_buffer) {
		efi_st_error("ProtocolsPerHandle returned no protocol\n");
		return EFI_ST_FAILURE;
	}
	efi_st_printf("%u protocols installed on image handle\n",
		      (unsigned int)protocol_buffer_count);
	for (i = 0; i < protocol_buffer_count; ++i) {
		if (memcmp(protocol_buffer[i], &loaded_image_protocol_guid,
			   sizeof(efi_guid_t)))
			found = true;
	}
	if (!found) {
		efi_st_printf("LoadedImageProtocol not found\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->free_pool(protocol_buffer);
	if (ret != EFI_SUCCESS) {
		efi_st_error("FreePool failed\n");
		return EFI_ST_FAILURE;
	}

	/*
	 * Open the loaded image protocol.
	 */
	ret = boottime->open_protocol(image_handle, &loaded_image_protocol_guid,
				      (void **)&loaded_image_protocol, NULL,
				      NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (ret != EFI_SUCCESS) {
		efi_st_error("OpenProtocol failed\n");
		return EFI_ST_FAILURE;
	}
	if (loaded_image_protocol->revision !=
	    EFI_LOADED_IMAGE_PROTOCOL_REVISION) {
		efi_st_printf("Incorrect revision\n");
		return EFI_ST_FAILURE;
	}
	if (!loaded_image_protocol->system_table ||
	    loaded_image_protocol->system_table->hdr.signature !=
	    EFI_SYSTEM_TABLE_SIGNATURE) {
		efi_st_printf("System table reference missing\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(loadedimage) = {
	.name = "loaded image",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
};
