// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_manageprotocols
 *
 * Copyright (c) 2017 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This unit test checks the following protocol services:
 * InstallProtocolInterface, UninstallProtocolInterface,
 * InstallMultipleProtocolsInterfaces, UninstallMultipleProtocolsInterfaces,
 * HandleProtocol, ProtocolsPerHandle,
 * LocateHandle, LocateHandleBuffer.
 */

#include <efi_selftest.h>

/*
 * The test currently does not actually call the interface function.
 * So this is just a dummy structure.
 */
struct interface {
	void (EFIAPI * inc)(void);
};

static struct efi_boot_services *boottime;
static efi_guid_t guid1 =
	EFI_GUID(0x2e7ca819, 0x21d3, 0x0a3a,
		 0xf7, 0x91, 0x82, 0x1f, 0x7a, 0x83, 0x67, 0xaf);
static efi_guid_t guid2 =
	EFI_GUID(0xf909f2bb, 0x90a8, 0x0d77,
		 0x94, 0x0c, 0x3e, 0xa8, 0xea, 0x38, 0xd6, 0x6f);
static efi_guid_t guid3 =
	EFI_GUID(0x06d641a3, 0xf4e7, 0xe0c9,
		 0xe7, 0x8d, 0x41, 0x2d, 0x72, 0xa6, 0xb1, 0x24);
static efi_handle_t handle1;
static efi_handle_t handle2;
static struct interface interface1;
static struct interface interface2;
static struct interface interface3;
static struct interface interface4;

/*
 * Find a handle in an array.
 *
 * @handle:	handle to find
 * @count:	number of entries in the array
 * @buffer:	array to search
 */
efi_status_t find_in_buffer(efi_handle_t handle, size_t count,
			    efi_handle_t *buffer)
{
	size_t i;

	for (i = 0; i < count; ++i) {
		if (buffer[i] == handle)
			return EFI_SUCCESS;
	}
	return EFI_NOT_FOUND;
}

/*
 * Setup unit test.
 *
 * Create two handles and install two out of three protocol interfaces on each
 * of them:
 *
 * handle1
 *   guid1 interface1
 *   guid3 interface3
 * handle2
 *   guid1 interface4
 *   guid2 interface2
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 */
static int setup(const efi_handle_t img_handle,
		 const struct efi_system_table *systable)
{
	efi_status_t ret;
	efi_handle_t handle;

	boottime = systable->boottime;

	ret = boottime->install_protocol_interface(&handle1, &guid3,
						   EFI_NATIVE_INTERFACE,
						   &interface3);
	if (ret != EFI_SUCCESS) {
		efi_st_error("InstallProtocolInterface failed\n");
		return EFI_ST_FAILURE;
	}
	if (!handle1) {
		efi_st_error("InstallProtocolInterface failed to create handle\n");
		return EFI_ST_FAILURE;
	}
	handle = handle1;
	ret = boottime->install_protocol_interface(&handle1, &guid1,
						   EFI_NATIVE_INTERFACE,
						   &interface1);
	if (ret != EFI_SUCCESS) {
		efi_st_error("InstallProtocolInterface failed\n");
		return EFI_ST_FAILURE;
	}
	if (handle != handle1) {
		efi_st_error("InstallProtocolInterface failed to use handle\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->install_multiple_protocol_interfaces(&handle2,
			&guid1, &interface4, &guid2, &interface2, NULL);
	if (ret != EFI_SUCCESS) {
		efi_st_error("InstallMultipleProtocolInterfaces failed\n");
		return EFI_ST_FAILURE;
	}
	if (!handle2 || handle1 == handle2) {
		efi_st_error("InstallMultipleProtocolInterfaces failed to create handle\n");
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
	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 */
static int execute(void)
{
	struct interface *interface;
	efi_status_t ret;
	efi_handle_t *buffer;
	size_t buffer_size;
	efi_uintn_t count = 0;
	efi_guid_t **prot_buffer;
	efi_uintn_t prot_count;

	/*
	 * Test HandleProtocol
	 */
	ret = boottime->handle_protocol(handle1, &guid3, (void **)&interface);
	if (ret != EFI_SUCCESS) {
		efi_st_error("HandleProtocol failed to retrieve interface\n");
		return EFI_ST_FAILURE;
	}
	if (interface != &interface3) {
		efi_st_error("HandleProtocol returned wrong interface\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->handle_protocol(handle1, &guid2, (void **)&interface);
	if (ret == EFI_SUCCESS) {
		efi_st_error("HandleProtocol returned not installed interface\n");
		return EFI_ST_FAILURE;
	}

	/*
	 * Test LocateHandleBuffer with AllHandles
	 */
	ret = boottime->locate_handle_buffer(ALL_HANDLES, NULL, NULL,
					     &count, &buffer);
	if (ret != EFI_SUCCESS) {
		efi_st_error("LocateHandleBuffer with AllHandles failed\n");
		return EFI_ST_FAILURE;
	}
	buffer_size = count;
	ret = find_in_buffer(handle1, count, buffer);
	if (ret != EFI_SUCCESS) {
		efi_st_error("LocateHandleBuffer failed to locate new handle\n");
		return EFI_ST_FAILURE;
	}
	ret = find_in_buffer(handle2, count, buffer);
	if (ret != EFI_SUCCESS) {
		efi_st_error("LocateHandleBuffer failed to locate new handle\n");
		return EFI_ST_FAILURE;
	}
	/* Release buffer */
	ret = boottime->free_pool(buffer);
	if (ret != EFI_SUCCESS) {
		efi_st_error("FreePool failed\n");
		return EFI_ST_FAILURE;
	}

	/*
	 * Test error handling in UninstallMultipleProtocols
	 *
	 * These are the installed protocol interfaces on handle 2:
	 *
	 *   guid1 interface4
	 *   guid2 interface2
	 *
	 * Try to uninstall more protocols than there are installed. This
	 * should return an error EFI_INVALID_PARAMETER. All deleted protocols
	 * should be reinstalled.
	 */
	ret = boottime->uninstall_multiple_protocol_interfaces(
						handle2,
						&guid1, &interface4,
						&guid2, &interface2,
						&guid3, &interface3,
						NULL);
	if (ret != EFI_INVALID_PARAMETER) {
		printf("%lx", ret);
		efi_st_error("UninstallMultipleProtocolInterfaces did not catch error\n");
		return EFI_ST_FAILURE;
	}

	/*
	 * Test LocateHandleBuffer with ByProtocol
	 *
	 * These are the handles with a guid1 protocol interface installed:
	 *
	 *	handle1, handle2
	 */
	count = buffer_size;
	ret = boottime->locate_handle_buffer(BY_PROTOCOL, &guid1, NULL,
					     &count, &buffer);
	if (ret != EFI_SUCCESS) {
		efi_st_error("LocateHandleBuffer failed to locate new handles\n");
		return EFI_ST_FAILURE;
	}
	if (count != 2) {
		efi_st_error("UninstallMultipleProtocolInterfaces deleted handle\n");
		return EFI_ST_FAILURE;
	}
	ret = find_in_buffer(handle1, count, buffer);
	if (ret != EFI_SUCCESS) {
		efi_st_error("LocateHandleBuffer failed to locate new handle\n");
		return EFI_ST_FAILURE;
	}
	ret = find_in_buffer(handle2, count, buffer);
	if (ret != EFI_SUCCESS) {
		efi_st_error("LocateHandleBuffer failed to locate new handle\n");
		return EFI_ST_FAILURE;
	}
	/* Clear the buffer, we are reusing it it the next step. */
	boottime->set_mem(buffer, sizeof(efi_handle_t) * buffer_size, 0);

	/*
	 * Test LocateHandle with ByProtocol
	 */
	count = buffer_size * sizeof(efi_handle_t);
	ret = boottime->locate_handle(BY_PROTOCOL, &guid1, NULL,
				      &count, buffer);
	if (ret != EFI_SUCCESS) {
		efi_st_error("LocateHandle with ByProtocol failed\n");
		return EFI_ST_FAILURE;
	}
	if (count / sizeof(efi_handle_t) != 2) {
		efi_st_error("LocateHandle failed to locate new handles\n");
		return EFI_ST_FAILURE;
	}
	buffer_size = count;
	ret = find_in_buffer(handle1, count, buffer);
	if (ret != EFI_SUCCESS) {
		efi_st_error("LocateHandle failed to locate new handles\n");
		return EFI_ST_FAILURE;
	}
	ret = find_in_buffer(handle2, count, buffer);
	if (ret != EFI_SUCCESS) {
		efi_st_error("LocateHandle failed to locate new handles\n");
		return EFI_ST_FAILURE;
	}
	/* Release buffer */
	ret = boottime->free_pool(buffer);
	if (ret != EFI_SUCCESS) {
		efi_st_error("FreePool failed\n");
		return EFI_ST_FAILURE;
	}

	/*
	 * Test LocateProtocol
	 */
	ret = boottime->locate_protocol(&guid1, NULL, (void **)&interface);
	if (ret != EFI_SUCCESS) {
		efi_st_error("LocateProtocol failed\n");
		return EFI_ST_FAILURE;
	}
	if (interface != &interface1 && interface != &interface4) {
		efi_st_error("LocateProtocol failed to locate protocol\n");
		return EFI_ST_FAILURE;
	}

	/*
	 * Test UninstallMultipleProtocols
	 */
	ret = boottime->uninstall_multiple_protocol_interfaces(
						handle2,
						&guid1, &interface4,
						&guid2, &interface2,
						NULL);
	if (ret != EFI_SUCCESS) {
		efi_st_error("UninstallMultipleProtocolInterfaces failed\n");
		return EFI_ST_FAILURE;
	}
	/*
	 * Check that the protocols are really uninstalled.
	 */
	count = buffer_size;
	ret = boottime->locate_handle_buffer(BY_PROTOCOL, &guid1, NULL,
					     &count, &buffer);
	if (ret != EFI_SUCCESS) {
		efi_st_error("LocateHandleBuffer failed\n");
		return EFI_ST_FAILURE;
	}
	if (count != 1) {
		efi_st_error("UninstallMultipleProtocolInterfaces failed to uninstall protocols\n");
		return EFI_ST_FAILURE;
	}
	ret = find_in_buffer(handle1, count, buffer);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to locate new handle\n");
		return EFI_ST_FAILURE;
	}
	boottime->set_mem(buffer, sizeof(efi_handle_t) * buffer_size, 0);

	/*
	 * Test ProtocolsPerHandle
	 */
	ret = boottime->protocols_per_handle(handle1,
					     &prot_buffer, &prot_count);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to get protocols per handle\n");
		return EFI_ST_FAILURE;
	}
	if (prot_count != 2) {
		efi_st_error("Failed to get protocols per handle\n");
		return EFI_ST_FAILURE;
	}
	if (memcmp(prot_buffer[0], &guid1, 16) &&
	    memcmp(prot_buffer[1], &guid1, 16)) {
		efi_st_error("Failed to get protocols per handle\n");
		return EFI_ST_FAILURE;
	}
	if (memcmp(prot_buffer[0], &guid3, 16) &&
	    memcmp(prot_buffer[1], &guid3, 16)) {
		efi_st_error("Failed to get protocols per handle\n");
		return EFI_ST_FAILURE;
	}
	/* Release buffer */
	ret = boottime->free_pool(prot_buffer);
	if (ret != EFI_SUCCESS) {
		efi_st_error("FreePool failed\n");
		return EFI_ST_FAILURE;
	}

	/*
	 * Uninstall remaining protocols
	 */
	ret = boottime->uninstall_protocol_interface(handle1, &guid1,
						     &interface1);
	if (ret != EFI_SUCCESS) {
		efi_st_error("UninstallProtocolInterface failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->handle_protocol(handle1, &guid1, (void **)&interface);
	if (ret == EFI_SUCCESS) {
		efi_st_error("UninstallProtocolInterface failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->uninstall_protocol_interface(handle1, &guid3,
						     &interface3);
	if (ret != EFI_SUCCESS) {
		efi_st_error("UninstallProtocolInterface failed\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(protserv) = {
	.name = "manage protocols",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.teardown = teardown,
};
