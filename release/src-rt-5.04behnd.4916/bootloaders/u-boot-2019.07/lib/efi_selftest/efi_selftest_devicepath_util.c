// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_devicepath_util
 *
 * Copyright (c) 2018 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This unit test checks the device path utilities protocol.
 */

#include <efi_selftest.h>

static struct efi_boot_services *boottime;

static efi_guid_t guid_device_path_utilities_protocol =
	EFI_DEVICE_PATH_UTILITIES_PROTOCOL_GUID;

struct efi_device_path_utilities_protocol *dpu;

/*
 * Setup unit test.
 *
 * Locate the device path utilities protocol.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 */
static int setup(const efi_handle_t img_handle,
		 const struct efi_system_table *systable)
{
	int ret;

	boottime = systable->boottime;

	ret = boottime->locate_protocol(&guid_device_path_utilities_protocol,
					NULL, (void **)&dpu);
	if (ret != EFI_SUCCESS) {
		dpu = NULL;
		efi_st_error(
			"Device path to text protocol is not available.\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

/*
 * Create a device path consisting of a single media device node followed by an
 * end node.
 *
 * @length:	length of the media device node
 * @dp:		device path
 * @return:	status code
 */
static int create_single_node_device_path(unsigned int length,
					  struct efi_device_path **dp)
{
	struct efi_device_path *node;
	efi_uintn_t len;
	int ret;

	node = dpu->create_device_node(DEVICE_PATH_TYPE_MEDIA_DEVICE,
				       DEVICE_PATH_SUB_TYPE_FILE_PATH, length);
	if (!node) {
		efi_st_error("CreateDeviceNode failed\n");
		return EFI_ST_FAILURE;
	}
	*dp = dpu->append_device_node(NULL, node);
	if (!*dp) {
		efi_st_error("AppendDeviceNode failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->free_pool(node);
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("FreePool failed\n");
		return EFI_ST_FAILURE;
	}
	len = dpu->get_device_path_size(*dp);
	if (len != length + 4) {
		efi_st_error("Wrong device path length %u, expected %u\n",
			     (unsigned int)len, length);
		return EFI_ST_FAILURE;
	}
	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * In the test device paths are created, copied, and concatenated. The device
 * path length is used as a measure of success.
 */
static int execute(void)
{
	struct efi_device_path *dp1;
	struct efi_device_path *dp2;
	struct efi_device_path *dp3;

	efi_uintn_t len;
	int ret;

	/* IsDevicePathMultiInstance(NULL) */
	if (dpu->is_device_path_multi_instance(NULL)) {
		efi_st_error("IsDevicePathMultiInstance(NULL) returned true\n");
		return EFI_ST_FAILURE;
	}
	/* GetDevicePathSize(NULL) */
	len = dpu->get_device_path_size(NULL);
	if (len) {
		efi_st_error("Wrong device path length %u, expected 0\n",
			     (unsigned int)len);
		return EFI_ST_FAILURE;
	}
	/* DuplicateDevicePath(NULL) */
	dp1 = dpu->duplicate_device_path(NULL);
	if (dp1) {
		efi_st_error("DuplicateDevicePath(NULL) failed\n");
		return EFI_ST_FAILURE;
	}
	/* AppendDevicePath(NULL, NULL) */
	dp1 = dpu->append_device_path(NULL, NULL);
	if (!dp1) {
		efi_st_error("AppendDevicePath(NULL, NULL) failed\n");
		return EFI_ST_FAILURE;
	}
	len = dpu->get_device_path_size(dp1);
	if (len != 4) {
		efi_st_error("Wrong device path length %u, expected 4\n",
			     (unsigned int)len);
		return EFI_ST_FAILURE;
	}
	ret = boottime->free_pool(dp1);
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("FreePool failed\n");
		return EFI_ST_FAILURE;
	}
	/* CreateDeviceNode */
	ret = create_single_node_device_path(21, &dp1);
	if (ret != EFI_ST_SUCCESS)
		return ret;
	ret = create_single_node_device_path(17, &dp2);
	if (ret != EFI_ST_SUCCESS)
		return ret;
	/* AppendDevicePath */
	dp3 = dpu->append_device_path(dp1, dp2);
	if (!dp3) {
		efi_st_error("AppendDevicePath failed\n");
		return EFI_ST_FAILURE;
	}
	if (dp3 == dp1 || dp3 == dp2) {
		efi_st_error("AppendDevicePath reused buffer\n");
		return EFI_ST_FAILURE;
	}
	len = dpu->get_device_path_size(dp3);
	/* 21 + 17 + 4 */
	if (len != 42) {
		efi_st_error("Wrong device path length %u, expected 42\n",
			     (unsigned int)len);
		return EFI_ST_FAILURE;
	}
	ret = boottime->free_pool(dp2);
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("FreePool failed\n");
		return EFI_ST_FAILURE;
	}
	/* AppendDeviceNode */
	dp2 = dpu->append_device_node(dp1, dp3);
	if (!dp2) {
		efi_st_error("AppendDevicePath failed\n");
		return EFI_ST_FAILURE;
	}
	len = dpu->get_device_path_size(dp2);
	/* 21 + 21 + 4 */
	if (len != 46) {
		printf("%s(%d) %s\n", __FILE__, __LINE__, __func__);
		efi_st_error("Wrong device path length %u, expected 46\n",
			     (unsigned int)len);
		return EFI_ST_FAILURE;
	}
	ret = boottime->free_pool(dp1);
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("FreePool failed\n");
		return EFI_ST_FAILURE;
	}
	/* IsDevicePathMultiInstance */
	if (dpu->is_device_path_multi_instance(dp2)) {
		printf("%s(%d) %s\n", __FILE__, __LINE__, __func__);
		efi_st_error("IsDevicePathMultiInstance returned true\n");
		return EFI_ST_FAILURE;
	}
	/* AppendDevicePathInstance */
	dp1 = dpu->append_device_path_instance(dp2, dp3);
	if (!dp1) {
		efi_st_error("AppendDevicePathInstance failed\n");
		return EFI_ST_FAILURE;
	}
	len = dpu->get_device_path_size(dp1);
	/* 46 + 42 */
	if (len != 88) {
		efi_st_error("Wrong device path length %u, expected 88\n",
			     (unsigned int)len);
		return EFI_ST_FAILURE;
	}
	/* IsDevicePathMultiInstance */
	if (!dpu->is_device_path_multi_instance(dp1)) {
		efi_st_error("IsDevicePathMultiInstance returned false\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->free_pool(dp2);
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("FreePool failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->free_pool(dp3);
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("FreePool failed\n");
		return EFI_ST_FAILURE;
	}
	/* GetNextDevicePathInstance */
	dp3 = dp1;
	dp2 = dpu->get_next_device_path_instance(&dp1, &len);
	if (!dp2) {
		efi_st_error("GetNextDevicePathInstance failed\n");
		return EFI_ST_FAILURE;
	}
	if (!dp1) {
		efi_st_error("GetNextDevicePathInstance no 2nd instance\n");
		return EFI_ST_FAILURE;
	}
	if (len != 46) {
		efi_st_error("Wrong device path length %u, expected 46\n",
			     (unsigned int)len);
		return EFI_ST_FAILURE;
	}
	len = dpu->get_device_path_size(dp1);
	if (len != 42) {
		efi_st_error("Wrong device path length %u, expected 42\n",
			     (unsigned int)len);
		return EFI_ST_FAILURE;
	}
	ret = boottime->free_pool(dp2);
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("FreePool failed\n");
		return EFI_ST_FAILURE;
	}
	dp2 = dpu->get_next_device_path_instance(&dp1, &len);
	if (!dp2) {
		efi_st_error("GetNextDevicePathInstance failed\n");
		return EFI_ST_FAILURE;
	}
	if (len != 42) {
		efi_st_error("Wrong device path length %u, expected 46\n",
			     (unsigned int)len);
		return EFI_ST_FAILURE;
	}
	if (dp1) {
		efi_st_error("GetNextDevicePathInstance did not signal end\n");
		return EFI_ST_FAILURE;
	}

	/* Clean up */
	ret = boottime->free_pool(dp2);
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("FreePool failed\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->free_pool(dp3);
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("FreePool failed\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(dputil) = {
	.name = "device path utilities protocol",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
};
