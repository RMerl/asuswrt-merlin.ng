// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_devicepath
 *
 * Copyright (c) 2017 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This unit test checks the following protocol services:
 * DevicePathToText
 */

#include <efi_selftest.h>

static struct efi_boot_services *boottime;

static efi_handle_t handle1;
static efi_handle_t handle2;
static efi_handle_t handle3;

struct interface {
	void (EFIAPI * inc)(void);
} interface;

static efi_guid_t guid_device_path = EFI_DEVICE_PATH_PROTOCOL_GUID;

static efi_guid_t guid_device_path_to_text_protocol =
	EFI_DEVICE_PATH_TO_TEXT_PROTOCOL_GUID;

static efi_guid_t guid_protocol =
	EFI_GUID(0xdbca4c98, 0x6cb0, 0x694d,
		 0x08, 0x72, 0x81, 0x9c, 0x65, 0x0c, 0xbb, 0x7d);

static efi_guid_t guid_vendor1 =
	EFI_GUID(0xdbca4c98, 0x6cb0, 0x694d,
		 0x08, 0x72, 0x81, 0x9c, 0x65, 0x0c, 0xbb, 0xb1);

static efi_guid_t guid_vendor2 =
	EFI_GUID(0xdbca4c98, 0x6cb0, 0x694d,
		 0x08, 0x72, 0x81, 0x9c, 0x65, 0x0c, 0xbb, 0xa2);

static efi_guid_t guid_vendor3 =
	EFI_GUID(0xdbca4c98, 0x6cb0, 0x694d,
		 0x08, 0x72, 0x81, 0x9c, 0x65, 0x0c, 0xbb, 0xc3);

static u8 *dp1;
static u8 *dp2;
static u8 *dp3;

struct efi_device_path_to_text_protocol *device_path_to_text;

/*
 * Setup unit test.
 *
 * Create three handles. Install a new protocol on two of them and
 * provide device paths.
 *
 * handle1
 *   guid interface
 * handle2
 *   guid interface
 * handle3
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 */
static int setup(const efi_handle_t img_handle,
		 const struct efi_system_table *systable)
{
	struct efi_device_path_vendor vendor_node;
	struct efi_device_path end_node;
	efi_status_t ret;

	boottime = systable->boottime;

	ret = boottime->locate_protocol(&guid_device_path_to_text_protocol,
					NULL, (void **)&device_path_to_text);
	if (ret != EFI_SUCCESS) {
		device_path_to_text = NULL;
		efi_st_error(
			"Device path to text protocol is not available.\n");
		return EFI_ST_FAILURE;
	}

	ret = boottime->allocate_pool(EFI_LOADER_DATA,
				      sizeof(struct efi_device_path_vendor) +
				      sizeof(struct efi_device_path),
				      (void **)&dp1);
	if (ret != EFI_SUCCESS)
		goto out_of_memory;

	ret = boottime->allocate_pool(EFI_LOADER_DATA, 2 *
				      sizeof(struct efi_device_path_vendor) +
				      sizeof(struct efi_device_path),
				      (void **)&dp2);
	if (ret != EFI_SUCCESS)
		goto out_of_memory;

	ret = boottime->allocate_pool(EFI_LOADER_DATA, 3 *
				      sizeof(struct efi_device_path_vendor) +
				      sizeof(struct efi_device_path),
				      (void **)&dp3);
	if (ret != EFI_SUCCESS)
		goto out_of_memory;

	vendor_node.dp.type = DEVICE_PATH_TYPE_HARDWARE_DEVICE;
	vendor_node.dp.sub_type = DEVICE_PATH_SUB_TYPE_VENDOR;
	vendor_node.dp.length = sizeof(struct efi_device_path_vendor);

	boottime->copy_mem(&vendor_node.guid, &guid_vendor1,
			   sizeof(efi_guid_t));
	boottime->copy_mem(dp1, &vendor_node,
			   sizeof(struct efi_device_path_vendor));
	boottime->copy_mem(dp2, &vendor_node,
			   sizeof(struct efi_device_path_vendor));
	boottime->copy_mem(dp3, &vendor_node,
			   sizeof(struct efi_device_path_vendor));

	boottime->copy_mem(&vendor_node.guid, &guid_vendor2,
			   sizeof(efi_guid_t));
	boottime->copy_mem(dp2 + sizeof(struct efi_device_path_vendor),
			   &vendor_node, sizeof(struct efi_device_path_vendor));
	boottime->copy_mem(dp3 + sizeof(struct efi_device_path_vendor),
			   &vendor_node, sizeof(struct efi_device_path_vendor));

	boottime->copy_mem(&vendor_node.guid, &guid_vendor3,
			   sizeof(efi_guid_t));
	boottime->copy_mem(dp3 + 2 * sizeof(struct efi_device_path_vendor),
			   &vendor_node, sizeof(struct efi_device_path_vendor));

	end_node.type = DEVICE_PATH_TYPE_END;
	end_node.sub_type = DEVICE_PATH_SUB_TYPE_END;
	end_node.length = sizeof(struct efi_device_path);
	boottime->copy_mem(dp1 + sizeof(struct efi_device_path_vendor),
			   &end_node, sizeof(struct efi_device_path));
	boottime->copy_mem(dp2 + 2 * sizeof(struct efi_device_path_vendor),
			   &end_node, sizeof(struct efi_device_path));
	boottime->copy_mem(dp3 + 3 * sizeof(struct efi_device_path_vendor),
			   &end_node, sizeof(struct efi_device_path));

	ret = boottime->install_protocol_interface(&handle1,
						   &guid_device_path,
						   EFI_NATIVE_INTERFACE,
						   dp1);
	if (ret != EFI_SUCCESS) {
		efi_st_error("InstallProtocolInterface failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->install_protocol_interface(&handle1,
						   &guid_protocol,
						   EFI_NATIVE_INTERFACE,
						   &interface);
	if (ret != EFI_SUCCESS) {
		efi_st_error("InstallProtocolInterface failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->install_protocol_interface(&handle2,
						   &guid_device_path,
						   EFI_NATIVE_INTERFACE,
						   dp2);
	if (ret != EFI_SUCCESS) {
		efi_st_error("InstallProtocolInterface failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->install_protocol_interface(&handle2,
						   &guid_protocol,
						   EFI_NATIVE_INTERFACE,
						   &interface);
	if (ret != EFI_SUCCESS) {
		efi_st_error("InstallProtocolInterface failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->install_protocol_interface(&handle3,
						   &guid_device_path,
						   EFI_NATIVE_INTERFACE,
						   dp3);
	if (ret != EFI_SUCCESS) {
		efi_st_error("InstallProtocolInterface failed\n");
		return EFI_ST_FAILURE;
	}
	return EFI_ST_SUCCESS;

out_of_memory:
	efi_st_error("Out of memory\n");
	return EFI_ST_FAILURE;
}

/*
 * Tear down unit test.
 *
 */
static int teardown(void)
{
	efi_status_t ret;

	ret = boottime->uninstall_protocol_interface(handle1,
						     &guid_device_path,
						     dp1);
	if (ret != EFI_SUCCESS) {
		efi_st_error("UninstallProtocolInterface failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->uninstall_protocol_interface(handle1,
						     &guid_protocol,
						     &interface);
	if (ret != EFI_SUCCESS) {
		efi_st_error("UninstallProtocolInterface failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->uninstall_protocol_interface(handle2,
						     &guid_device_path,
						     dp2);
	if (ret != EFI_SUCCESS) {
		efi_st_error("UninstallProtocolInterface failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->uninstall_protocol_interface(handle2,
						     &guid_protocol,
						     &interface);
	if (ret != EFI_SUCCESS) {
		efi_st_error("UninstallProtocolInterface failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->uninstall_protocol_interface(handle3,
						     &guid_device_path,
						     dp3);
	if (ret != EFI_SUCCESS) {
		efi_st_error("UninstallProtocolInterface failed\n");
		return EFI_ST_FAILURE;
	}
	if (dp1) {
		ret = boottime->free_pool(dp1);
		if (ret != EFI_SUCCESS) {
			efi_st_error("FreePool failed\n");
			return EFI_ST_FAILURE;
		}
	}
	if (dp2) {
		ret = boottime->free_pool(dp2);
		if (ret != EFI_SUCCESS) {
			efi_st_error("FreePool failed\n");
			return EFI_ST_FAILURE;
		}
	}
	if (dp3) {
		ret = boottime->free_pool(dp3);
		if (ret != EFI_SUCCESS) {
			efi_st_error("FreePool failed\n");
			return EFI_ST_FAILURE;
		}
	}
	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 */
static int execute(void)
{
	struct efi_device_path *remaining_dp;
	efi_handle_t handle;
	/*
	 * This device path node ends with the letter 't' of 'u-boot'.
	 * The following '.bin' does not belong to the node but is
	 * helps to test the correct truncation.
	 */
	struct {
		struct efi_device_path dp;
		u16 text[12];
	} __packed dp_node = {
			{ DEVICE_PATH_TYPE_MEDIA_DEVICE,
			  DEVICE_PATH_SUB_TYPE_FILE_PATH,
			  sizeof(struct efi_device_path) + 12},
			L"u-boot.bin",
		};
	u16 *string;
	efi_status_t ret;
	efi_uintn_t i, no_handles;
	efi_handle_t *handles;
	struct efi_device_path *dp;

	/* Display all available device paths */
	ret = boottime->locate_handle_buffer(BY_PROTOCOL,
					     &guid_device_path,
					     NULL, &no_handles, &handles);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Cannot retrieve device path protocols.\n");
		return EFI_ST_FAILURE;
	}

	efi_st_printf("Installed device path protocols:\n");
	for (i = 0; i < no_handles; ++i) {
		ret = boottime->open_protocol(handles[i], &guid_device_path,
					      (void **)&dp, NULL, NULL,
					      EFI_OPEN_PROTOCOL_GET_PROTOCOL);
		if (ret != EFI_SUCCESS) {
			efi_st_error("Cannot open device path protocol.\n");
			return EFI_ST_FAILURE;
		}
		string = device_path_to_text->convert_device_path_to_text(
					dp, true, false);
		if (!string) {
			efi_st_error("ConvertDevicePathToText failed\n");
			return EFI_ST_FAILURE;
		}
		efi_st_printf("%ps\n", string);
		ret = boottime->free_pool(string);
		if (ret != EFI_SUCCESS) {
			efi_st_error("FreePool failed\n");
			return EFI_ST_FAILURE;
		}
		/*
		 * CloseProtocol cannot be called without agent handle.
		 * There is no need to close the device path protocol.
		 */
	}
	ret = boottime->free_pool(handles);
	if (ret != EFI_SUCCESS) {
		efi_st_error("FreePool failed\n");
		return EFI_ST_FAILURE;
	}

	/* Test ConvertDevicePathToText */
	string = device_path_to_text->convert_device_path_to_text(
			(struct efi_device_path *)dp2, true, false);
	if (!string) {
		efi_st_error("ConvertDevicePathToText failed\n");
		return EFI_ST_FAILURE;
	}
	if (efi_st_strcmp_16_8(
		string,
		"/VenHw(dbca4c98-6cb0-694d-0872-819c650cbbb1)/VenHw(dbca4c98-6cb0-694d-0872-819c650cbba2)")
	    ) {
		efi_st_printf("dp2: %ps\n", string);
		efi_st_error("Incorrect text from ConvertDevicePathToText\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->free_pool(string);
	if (ret != EFI_SUCCESS) {
		efi_st_error("FreePool failed\n");
		return EFI_ST_FAILURE;
	}

	/* Test ConvertDeviceNodeToText */
	string = device_path_to_text->convert_device_node_to_text(
			(struct efi_device_path *)&dp_node, true, false);
	if (!string) {
		efi_st_error("ConvertDeviceNodeToText failed\n");
		return EFI_ST_FAILURE;
	}
	if (efi_st_strcmp_16_8(string, "u-boot")) {
		efi_st_printf("dp_node: %ps\n", string);
		efi_st_error(
			"Incorrect conversion by ConvertDeviceNodeToText\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->free_pool(string);
	if (ret != EFI_SUCCESS) {
		efi_st_error("FreePool failed\n");
		return EFI_ST_FAILURE;
	}

	/* Test LocateDevicePath */
	remaining_dp = (struct efi_device_path *)dp3;
	ret = boottime->locate_device_path(&guid_protocol, &remaining_dp,
					   &handle);
	if (ret != EFI_SUCCESS) {
		efi_st_error("LocateDevicePath failed\n");
		return EFI_ST_FAILURE;
	}
	if (handle != handle2) {
		efi_st_error("LocateDevicePath returned wrong handle\n");
		return EFI_ST_FAILURE;
	}
	string = device_path_to_text->convert_device_path_to_text(remaining_dp,
								  true, false);
	if (!string) {
		efi_st_error("ConvertDevicePathToText failed\n");
		return EFI_ST_FAILURE;
	}
	if (efi_st_strcmp_16_8(string,
			       "/VenHw(dbca4c98-6cb0-694d-0872-819c650cbbc3)")
	    ) {
		efi_st_printf("remaining device path: %ps\n", string);
		efi_st_error("LocateDevicePath: wrong remaining device path\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->free_pool(string);
	if (ret != EFI_SUCCESS) {
		efi_st_error("FreePool failed\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(devicepath) = {
	.name = "device path",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.teardown = teardown,
};
