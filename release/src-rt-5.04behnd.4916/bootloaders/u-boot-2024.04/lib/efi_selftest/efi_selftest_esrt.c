// SPDX-License-Identifier: GPL-2.0-only
/*
 *  Test ESRT tables support
 *
 *  Copyright (C) 2021 Arm Ltd.
 */
#include <efi_loader.h>
#include <efi_selftest.h>

// This value must not exceed 255.
// An FMP cannot contain more than 255 FW images.
#define TEST_ESRT_NUM_ENTRIES 255

static
struct efi_firmware_image_descriptor static_img_info[TEST_ESRT_NUM_ENTRIES];

static const struct efi_system_table *local_systable;

static efi_handle_t fmp_handle;

static const efi_guid_t efi_fmp_guid =
		EFI_FIRMWARE_MANAGEMENT_PROTOCOL_GUID;

static void efi_test_esrt_init_info(void)
{
	for (int idx = 0; idx < TEST_ESRT_NUM_ENTRIES; idx++) {
		static_img_info[idx].image_index = idx;

		// Note: the 16 byte value present in
		// static_img_info[idx].image_type_id is not strictly a GUID.
		// The value is used for the sake of code testing.
		static_img_info[idx].image_type_id.b[0] = idx;

		static_img_info[idx].image_id = 0;
		static_img_info[idx].image_id_name = NULL;
		static_img_info[idx].version = 0;
		static_img_info[idx].version_name = NULL;
		static_img_info[idx].size = 0;
		static_img_info[idx].lowest_supported_image_version = 1;
		static_img_info[idx].last_attempt_version = 2;
		static_img_info[idx].last_attempt_status = 3;
		static_img_info[idx].hardware_instance = 1;
	}
}

static efi_status_t
EFIAPI efi_test_fmp_get_image_info(struct efi_firmware_management_protocol *this,
				   efi_uintn_t *image_info_size,
				   struct efi_firmware_image_descriptor *image_info,
				   u32 *descriptor_version,
				   u8 *descriptor_count,
				   efi_uintn_t *descriptor_size,
				   u32 *package_version,
				   u16 **package_version_name)
{
	efi_status_t ret = EFI_SUCCESS;

	if (!image_info_size)
		return EFI_INVALID_PARAMETER;

	if (descriptor_version)
		*descriptor_version = EFI_FIRMWARE_IMAGE_DESCRIPTOR_VERSION;
	if (descriptor_count)
		*descriptor_count = TEST_ESRT_NUM_ENTRIES;
	if (descriptor_size)
		*descriptor_size = sizeof(*image_info);
	if (package_version)
		*package_version = 0xffffffff;
	if (package_version_name)
		*package_version_name = NULL;

	if (*image_info_size < sizeof(*image_info)) {
		*image_info_size = *descriptor_size * *descriptor_count;
		return EFI_BUFFER_TOO_SMALL;
	}

	for (int idx = 0; idx < TEST_ESRT_NUM_ENTRIES; idx++)
		image_info[idx] = static_img_info[idx];

	return ret;
}

static struct efi_firmware_management_protocol efi_test_fmp = {
	.get_image_info = efi_test_fmp_get_image_info,
	.get_image = NULL,
	.set_image = NULL,
	.check_image = NULL,
	.get_package_info = NULL,
	.set_package_info = NULL,
};

static void *lib_test_get_esrt(void)
{
	for (int idx = 0; idx < local_systable->nr_tables; idx++)
		if (!guidcmp(&efi_esrt_guid, &local_systable->tables[idx].guid))
			return local_systable->tables[idx].table;

	return NULL;
}

/**
 * lib_test_check_uuid_entry: Find an ESRT entry for which the fw_calss field matches
 * the image_type_id in the @img_info.
 * Ensure that all of the field in the ESRT entry have the same value as the corresponding
 * fields in the @img_info.
 *
 * @esrt: pointer to the ESRT
 * @img_info: an image_info_descriptor output by the FMP get_image_info
 *
 * Return: true if matching ESRT entry is found and if all the ESRT entry fields match the
 * corresponding @img_info fields.
 */
static bool lib_test_check_uuid_entry(struct efi_system_resource_table *esrt,
				      struct efi_firmware_image_descriptor
				      *img_info)
{
	const u32 filled_entries = esrt->fw_resource_count;
	struct efi_system_resource_entry *entry = esrt->entries;

	for (u32 idx = 0; idx < filled_entries; idx++) {
		if (!guidcmp(&entry[idx].fw_class, &img_info->image_type_id)) {
			if (entry[idx].fw_version != img_info->version) {
				efi_st_error("ESRT field mismatch for entry with fw_class=%pU\n",
					     &img_info->image_type_id);
				return false;
			}

			if (entry[idx].lowest_supported_fw_version !=
				img_info->lowest_supported_image_version) {
				efi_st_error("ESRT field mismatch for entry with fw_class=%pU\n",
					     &img_info->image_type_id);
				return false;
			}

			if (entry[idx].last_attempt_version !=
				img_info->last_attempt_version) {
				efi_st_error("ESRT field mismatch for entry with fw_class=%pU\n",
					     &img_info->image_type_id);
				return false;
			}

			if (entry[idx].last_attempt_status !=
				img_info->last_attempt_status) {
				efi_st_error("ESRT field mismatch for entry with fw_class=%pU\n",
					     &img_info->image_type_id);
				return false;
			}

			/*
			 * The entry with fw_class = img_uuid matches with the
			 * remainder fmp input.
			 */
			return true;
		}
	}

	/* There exists no entry with fw_class equal to img_uuid in the ESRT. */
	efi_st_error("ESRT no entry with fw_class= %pUl\n", &img_info->image_type_id);

	return false;
}

/*
 * Setup unit test.
 *
 * Initialize the test FMP datastructure.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * Return:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	local_systable = systable;

	efi_test_esrt_init_info();

	return EFI_ST_SUCCESS;
}

/*
 * Tear down unit test.
 *
 * Uninstall the test FMP.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int teardown(void)
{
	efi_status_t ret = EFI_SUCCESS;
	struct efi_boot_services *bt;

	bt = local_systable->boottime;

	if (!bt) {
		efi_st_error("Cannot find boottime services structure\n");
		return EFI_ST_FAILURE;
	}

	ret = bt->uninstall_multiple_protocol_interfaces
		(fmp_handle, &efi_fmp_guid,
		 &efi_test_fmp, NULL);

	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to uninstall FMP\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

/*
 * Perform the test
 *
 * The test consists of the following steps:
 *
 * 1) Obtain the ESRT
 * 2) Record the number of ESRT entries prior to test start
 * 3) Install the test FMP
 * 4) Re-obtain the ESRT (the ESRT pointer may have changed with the FMP install)
 * 5) verify that the ESRT entries have increased by the number of entries in the
 *     test FMP.
 * 6) Traverse all the elements used as the test FMP input and verify that each
 *     has a corresponding ESRT entry and that the fields are correctly set.
 *
 * The failure of any of the above steps results in a test failure.
 *
 */
static int execute(void)
{
	struct efi_system_resource_table *esrt;
	efi_status_t ret = EFI_SUCCESS;
	u32 base_entry_count;
	u32 entry_delta;
	struct efi_boot_services *bt;

	bt = local_systable->boottime;

	if (!bt) {
		efi_st_error("Cannot find boottime services structure\n");
		return EFI_ST_FAILURE;
	}

	esrt = lib_test_get_esrt();
	if (!esrt) {
		efi_st_error("ESRT table not present\n");
		return EFI_ST_FAILURE;
	}
	base_entry_count = esrt->fw_resource_count;

	ret = bt->install_multiple_protocol_interfaces(&fmp_handle,
						       &efi_fmp_guid,
						       &efi_test_fmp,
						       NULL);

	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to install FMP\n");
		return EFI_ST_FAILURE;
	}

	esrt = lib_test_get_esrt();
	if (!esrt) {
		efi_st_error("ESRT table not present\n");
		return EFI_ST_FAILURE;
	}

	entry_delta = esrt->fw_resource_count - base_entry_count;
	if (entry_delta != TEST_ESRT_NUM_ENTRIES) {
		efi_st_error("ESRT mismatch in new entry count (%d), expected (%d).\n",
			     entry_delta, TEST_ESRT_NUM_ENTRIES);
		return EFI_ST_FAILURE;
	}

	for (u32 idx = 0; idx < TEST_ESRT_NUM_ENTRIES; idx++)
		if (!lib_test_check_uuid_entry(esrt, &static_img_info[idx])) {
			efi_st_error("ESRT entry mismatch\n");
			return EFI_ST_FAILURE;
		}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(esrt) = {
	.name = "esrt",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.teardown = teardown,
};
