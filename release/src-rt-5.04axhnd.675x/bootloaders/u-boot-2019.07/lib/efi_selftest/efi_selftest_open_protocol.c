// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_open_protocol
 *
 * Copyright (c) 2019 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This unit test checks that open protocol information is correctly updated
 * when calling:
 * HandleProtocol, OpenProtocol, OpenProtocolInformation, CloseProtocol.
 */

#include <efi_selftest.h>

/*
 * The test currently does not actually call the interface function.
 * So this is just a dummy structure.
 */
struct interface {
	void (EFIAPI *inc)(void);
};

static struct efi_boot_services *boottime;
static efi_guid_t guid1 =
	EFI_GUID(0x492a0e38, 0x1442, 0xf819,
		 0x14, 0xaa, 0x4b, 0x8d, 0x09, 0xfe, 0x5a, 0xb9);
static efi_handle_t handle1;
static struct interface interface1;

/*
 * Setup unit test.
 *
 * Create a handle and install a protocol interface on it.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 */
static int setup(const efi_handle_t img_handle,
		 const struct efi_system_table *systable)
{
	efi_status_t ret;

	boottime = systable->boottime;

	ret = boottime->install_protocol_interface(&handle1, &guid1,
						   EFI_NATIVE_INTERFACE,
						   &interface1);
	if (ret != EFI_SUCCESS) {
		efi_st_error("InstallProtocolInterface failed\n");
		return EFI_ST_FAILURE;
	}
	if (!handle1) {
		efi_st_error
			("InstallProtocolInterface failed to create handle\n");
		return EFI_ST_FAILURE;
	}
	return EFI_ST_SUCCESS;
}

/*
 * Tear down unit test.
 *
 */
static int teardown(void)
{
	efi_status_t ret;

	if (handle1) {
		ret = boottime->uninstall_protocol_interface(handle1, &guid1,
							     &interface1);
		if (ret != EFI_SUCCESS) {
			efi_st_error("UninstallProtocolInterface failed\n");
			return EFI_ST_FAILURE;
		}
	}
	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * Open the installed protocol twice via HandleProtocol() and once via
 * OpenProtocol(EFI_OPEN_PROTOCOL_GET_PROTOCOL). Read the open protocol
 * information and check the open counts. Finally close the protocol and
 * check again.
 */
static int execute(void)
{
	void *interface;
	struct efi_open_protocol_info_entry *entry_buffer;
	efi_uintn_t entry_count;
	efi_handle_t firmware_handle;
	efi_status_t ret;

	ret = boottime->handle_protocol(handle1, &guid1, &interface);
	if (ret != EFI_SUCCESS) {
		efi_st_error("HandleProtocol failed\n");
		return EFI_ST_FAILURE;
	}
	if (interface != &interface1) {
		efi_st_error("HandleProtocol returned wrong interface\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->open_protocol_information(handle1, &guid1,
						  &entry_buffer, &entry_count);
	if (ret != EFI_SUCCESS) {
		efi_st_error("OpenProtocolInformation failed\n");
		return EFI_ST_FAILURE;
	}
	if (entry_count != 1) {
		efi_st_error("Incorrect OpenProtocolInformation count\n");
		efi_st_printf("Expected 1, got %u\n",
			      (unsigned int)entry_count);
		return EFI_ST_FAILURE;
	}
	ret = boottime->free_pool(entry_buffer);
	if (ret != EFI_SUCCESS) {
		efi_st_error("FreePool failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->handle_protocol(handle1, &guid1, &interface);
	if (ret != EFI_SUCCESS) {
		efi_st_error("HandleProtocol failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->open_protocol_information(handle1, &guid1,
						  &entry_buffer, &entry_count);
	if (ret != EFI_SUCCESS) {
		efi_st_error("OpenProtocolInformation failed\n");
		return EFI_ST_FAILURE;
	}
	if (entry_count != 1) {
		efi_st_error("Incorrect OpenProtocolInformation count\n");
		efi_st_printf("Expected 1, got %u\n",
			      (unsigned int)entry_count);
		return EFI_ST_FAILURE;
	}
	if (entry_buffer[0].open_count != 2) {
		efi_st_error("Incorrect open count: expected 2 got %u\n",
			     entry_buffer[0].open_count);
		return EFI_ST_FAILURE;
	}
	firmware_handle = entry_buffer[0].agent_handle;
	ret = boottime->free_pool(entry_buffer);
	if (ret != EFI_SUCCESS) {
		efi_st_error("FreePool failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->open_protocol(handle1, &guid1, &interface,
				      firmware_handle, NULL,
				      EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (ret != EFI_SUCCESS) {
		efi_st_error("OpenProtocol failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->open_protocol_information(handle1, &guid1,
						  &entry_buffer, &entry_count);
	if (ret != EFI_SUCCESS) {
		efi_st_error("OpenProtocolInformation failed\n");
		return EFI_ST_FAILURE;
	}
	if (entry_count != 2) {
		efi_st_error("Incorrect OpenProtocolInformation count\n");
		efi_st_printf("Expected 2, got %u\n",
			      (unsigned int)entry_count);
		return EFI_ST_FAILURE;
	}
	if (entry_buffer[0].open_count + entry_buffer[1].open_count != 3) {
		efi_st_error("Incorrect open count: expected 3 got %u\n",
			     entry_buffer[0].open_count +
			     entry_buffer[1].open_count);
		return EFI_ST_FAILURE;
	}
	ret = boottime->free_pool(entry_buffer);
	if (ret != EFI_SUCCESS) {
		efi_st_error("FreePool failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->close_protocol(handle1, &guid1, firmware_handle, NULL);
	if (ret != EFI_SUCCESS) {
		efi_st_error("CloseProtocol failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->open_protocol_information(handle1, &guid1,
						  &entry_buffer, &entry_count);
	if (ret != EFI_SUCCESS) {
		efi_st_error("OpenProtocolInformation failed\n");
		return EFI_ST_FAILURE;
	}
	if (entry_count) {
		efi_st_error("Incorrect OpenProtocolInformation count\n");
		efi_st_printf("Expected 0, got %u\n",
			      (unsigned int)entry_count);
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(openprot) = {
	.name = "open protocol",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.teardown = teardown,
};
