// SPDX-License-Identifier: GPL-2.0-only
/*
 *  EFI application ESRT tables support
 *
 *  Copyright (C) 2021 Arm Ltd.
 */

#include <efi_loader.h>
#include <log.h>
#include <efi_api.h>
#include <malloc.h>

const efi_guid_t efi_esrt_guid = EFI_SYSTEM_RESOURCE_TABLE_GUID;

static struct efi_system_resource_table *esrt;

#define EFI_ESRT_VERSION 1

/**
 * efi_esrt_image_info_to_entry() - copy the information present in a fw image
 * descriptor to a ESRT entry.
 * The function ensures the ESRT entry matches the image_type_id in @img_info.
 * In case of a mismatch we leave the entry unchanged.
 *
 * @img_info:     the source image info descriptor
 * @entry:        pointer to the ESRT entry to be filled
 * @desc_version: the version of the elements in img_info
 * @image_type:   the image type value to be set in the ESRT entry
 * @flags:        the capsule flags value to be set in the ESRT entry
 *
 * Return:
 * - EFI_SUCCESS if the entry is correctly updated
 * - EFI_INVALID_PARAMETER if entry does not match image_type_id in @img_info.
 */
static efi_status_t
efi_esrt_image_info_to_entry(struct efi_firmware_image_descriptor *img_info,
			     struct efi_system_resource_entry *entry,
			     u32 desc_version, u32 image_type, u32 flags)
{
	if (guidcmp(&entry->fw_class, &img_info->image_type_id)) {
		EFI_PRINT("ESRT entry %pUL mismatches img_type_id %pUL\n",
			  &entry->fw_class, &img_info->image_type_id);
		return EFI_INVALID_PARAMETER;
	}

	entry->fw_version = img_info->version;

	entry->fw_type = image_type;
	entry->capsule_flags = flags;

	/*
	 * The field lowest_supported_image_version is only present
	 * on image info structure of version 2 or greater.
	 * See the EFI_FIRMWARE_IMAGE_DESCRIPTOR definition in UEFI.
	 */
	if (desc_version >= 2)
		entry->lowest_supported_fw_version =
			img_info->lowest_supported_image_version;
	else
		entry->lowest_supported_fw_version = 0;

	/*
	 * The fields last_attempt_version and last_attempt_status
	 * are only present on image info structure of version 3 or
	 * greater.
	 * See the EFI_FIRMWARE_IMAGE_DESCRIPTOR definition in UEFI.
	 */
	if (desc_version >= 3) {
		entry->last_attempt_version =
			img_info->last_attempt_version;

		entry->last_attempt_status =
			img_info->last_attempt_status;
	} else {
		entry->last_attempt_version = 0;
		entry->last_attempt_status = LAST_ATTEMPT_STATUS_SUCCESS;
	}

	return EFI_SUCCESS;
}

/**
 * efi_esrt_entries_to_size() - Obtain the bytes used by an ESRT
 * datastructure with @num_entries.
 *
 * @num_entries: the number of entries in the ESRT.
 *
 * Return: the number of bytes an ESRT with @num_entries occupies in memory.
 */
static
inline u32 efi_esrt_entries_to_size(u32 num_entries)
{
	u32 esrt_size = sizeof(struct efi_system_resource_table) +
		num_entries * sizeof(struct efi_system_resource_entry);

	return esrt_size;
}

/**
 * efi_esrt_allocate_install() - Allocates @num_entries for the ESRT and
 * performs basic ESRT initialization.
 *
 * @num_entries: the number of entries that the ESRT will hold.
 *
 * Return:
 * - pointer to the ESRT if successful.
 * - NULL otherwise.
 */
static
efi_status_t efi_esrt_allocate_install(u32 num_entries)
{
	efi_status_t ret;
	struct efi_system_resource_table *new_esrt;
	u32 size = efi_esrt_entries_to_size(num_entries);
	efi_guid_t esrt_guid = efi_esrt_guid;

	/* Reserve memory for ESRT */
	ret = efi_allocate_pool(EFI_BOOT_SERVICES_DATA, size,
				(void **)&new_esrt);

	if (ret != EFI_SUCCESS) {
		EFI_PRINT("ESRT cannot allocate memory for %u entries (%u bytes)\n",
			  num_entries, size);

		return ret;
	}

	new_esrt->fw_resource_count_max = num_entries;
	new_esrt->fw_resource_count = 0;
	new_esrt->fw_resource_version = EFI_ESRT_VERSION;

	/* Install the ESRT in the system configuration table. */
	ret = efi_install_configuration_table(&esrt_guid, (void *)new_esrt);
	if (ret != EFI_SUCCESS) {
		EFI_PRINT("ESRT failed to install the ESRT in the system table\n");
		return ret;
	}

	/* If there was a previous ESRT, deallocate its memory now. */
	if (esrt)
		ret = efi_free_pool(esrt);

	esrt = new_esrt;

	return EFI_SUCCESS;
}

/**
 * esrt_find_entry() - Obtain the ESRT entry for the image with GUID
 * @img_fw_class.
 *
 * If the img_fw_class is not yet present in the ESRT, this function
 * reserves the tail element of the current ESRT as the entry for that fw_class.
 * The number of elements in the ESRT is updated in that case.
 *
 * @img_fw_class: the GUID of the FW image which ESRT entry we want to obtain.
 *
 * Return:
 *  - A pointer to the ESRT entry for the image with GUID img_fw_class,
 *  - NULL if:
 *   - there is no more space in the ESRT,
 *   - ESRT is not initialized,
 */
static
struct efi_system_resource_entry *esrt_find_entry(efi_guid_t *img_fw_class)
{
	u32 filled_entries;
	u32 max_entries;
	struct efi_system_resource_entry *entry;

	if (!esrt) {
		EFI_PRINT("ESRT access before initialized\n");
		return NULL;
	}

	filled_entries = esrt->fw_resource_count;
	entry = esrt->entries;

	/* Check if the image with img_fw_class is already in the ESRT. */
	for (u32 idx = 0; idx < filled_entries; idx++) {
		if (!guidcmp(&entry[idx].fw_class, img_fw_class)) {
			EFI_PRINT("ESRT found entry for image %pUs at index %u\n",
				  img_fw_class, idx);
			return &entry[idx];
		}
	}

	max_entries = esrt->fw_resource_count_max;
	/*
	 * Since the image with img_fw_class is not present in the ESRT, check
	 * if ESRT is full before appending the new entry to it.
	 */
	if (filled_entries == max_entries) {
		EFI_PRINT("ESRT full, this should not happen\n");
		return NULL;
	}

	/*
	 * This is a new entry for a fw image, increment the element
	 * number in the table and set the fw_class field.
	 */
	esrt->fw_resource_count++;
	entry[filled_entries].fw_class = *img_fw_class;
	EFI_PRINT("ESRT allocated new entry for image %pUs at index %u\n",
		  img_fw_class, filled_entries);

	return &entry[filled_entries];
}

/**
 * efi_esrt_add_from_fmp() - Populates a sequence of ESRT entries from the FW
 * images in the FMP.
 *
 * @fmp: the FMP instance from which FW images are added to the ESRT
 *
 * Return:
 * - EFI_SUCCESS if all the FW images in the FMP are added to the ESRT
 * - Error status otherwise
 */
static
efi_status_t efi_esrt_add_from_fmp(struct efi_firmware_management_protocol *fmp)
{
	struct efi_system_resource_entry *entry = NULL;
	size_t info_size = 0;
	struct efi_firmware_image_descriptor *img_info = NULL;
	u32 desc_version;
	u8 desc_count;
	size_t desc_size;
	u32 package_version;
	u16 *package_version_name;
	efi_status_t ret = EFI_SUCCESS;

	/*
	 * TODO: set the field image_type depending on the FW image type
	 * defined in a platform basis.
	 */
	u32 image_type = ESRT_FW_TYPE_UNKNOWN;

	/* TODO: set the capsule flags as a function of the FW image type. */
	u32 flags = 0;

	ret = EFI_CALL(fmp->get_image_info(fmp, &info_size, img_info,
					   &desc_version, &desc_count,
					   &desc_size, NULL, NULL));

	if (ret != EFI_BUFFER_TOO_SMALL) {
		/*
		 * An input of info_size=0 should always lead
		 * fmp->get_image_info to return BUFFER_TO_SMALL.
		 */
		EFI_PRINT("Erroneous FMP implementation\n");
		return EFI_INVALID_PARAMETER;
	}

	ret = efi_allocate_pool(EFI_BOOT_SERVICES_DATA, info_size,
				(void **)&img_info);
	if (ret != EFI_SUCCESS) {
		EFI_PRINT("ESRT failed to allocate memory for image info.\n");
		return ret;
	}

	ret = EFI_CALL(fmp->get_image_info(fmp, &info_size, img_info,
					   &desc_version, &desc_count,
					   &desc_size, &package_version,
					   &package_version_name));
	if (ret != EFI_SUCCESS) {
		EFI_PRINT("ESRT failed to obtain the FMP image info\n");
		goto out;
	}

	/*
	 * Iterate over all the FW images in the FMP.
	 */
	for (u32 desc_idx = 0; desc_idx < desc_count; desc_idx++) {
		struct efi_firmware_image_descriptor *cur_img_info =
			(struct efi_firmware_image_descriptor *)
			((uintptr_t)img_info + desc_idx * desc_size);

		/*
		 * Obtain the ESRT entry for the FW image with fw_class
		 * equal to cur_img_info->image_type_id.
		 */
		entry = esrt_find_entry(&cur_img_info->image_type_id);

		if (entry) {
			ret = efi_esrt_image_info_to_entry(cur_img_info, entry,
							   desc_version,
							   image_type, flags);
			if (ret != EFI_SUCCESS)
				EFI_PRINT("ESRT entry mismatches image_type\n");

		} else {
			EFI_PRINT("ESRT failed to add entry for %pUs\n",
				  &cur_img_info->image_type_id);
			continue;
		}
	}

out:
	efi_free_pool(img_info);
	return EFI_SUCCESS;
}

/**
 * efi_esrt_populate() - Populates the ESRT entries from the FMP instances
 * present in the system.
 * If an ESRT already exists, the old ESRT is replaced in the system table.
 * The memory of the old ESRT is deallocated.
 *
 * Return:
 * - EFI_SUCCESS if the ESRT is correctly created
 * - error code otherwise.
 */
efi_status_t efi_esrt_populate(void)
{
	efi_handle_t *base_handle = NULL;
	efi_handle_t *it_handle;
	efi_uintn_t no_handles = 0;
	struct efi_firmware_management_protocol *fmp;
	efi_status_t ret;
	u32 num_entries = 0;
	struct efi_handler *handler;

	/*
	 * Obtain the number of registered FMP handles.
	 */
	ret = EFI_CALL(efi_locate_handle_buffer(BY_PROTOCOL,
						&efi_guid_firmware_management_protocol,
						NULL, &no_handles,
						(efi_handle_t **)&base_handle));

	if (ret != EFI_SUCCESS) {
		EFI_PRINT("ESRT There are no FMP instances\n");

		ret = efi_esrt_allocate_install(0);
		if (ret != EFI_SUCCESS) {
			EFI_PRINT("ESRT failed to create table with 0 entries\n");
			return ret;
		}
		return EFI_SUCCESS;
	}

	EFI_PRINT("ESRT populate esrt from (%zd) available FMP handles\n",
		  no_handles);

	/*
	 * Iterate over all FMPs to determine an upper bound on the number of
	 * ESRT entries.
	 */
	it_handle = base_handle;
	for (u32 idx = 0; idx < no_handles; idx++, it_handle++) {
		struct efi_firmware_image_descriptor *img_info = NULL;
		size_t info_size = 0;
		u32 desc_version = 0;
		u8 desc_count = 0;
		size_t desc_size = 0;
		u32 package_version;
		u16 *package_version_name;

		ret = efi_search_protocol(*it_handle,
					  &efi_guid_firmware_management_protocol,
					  &handler);

		if (ret != EFI_SUCCESS) {
			EFI_PRINT("ESRT Unable to find FMP handle (%u)\n",
				  idx);
			continue;
		}
		fmp = handler->protocol_interface;

		ret = EFI_CALL(fmp->get_image_info(fmp, &info_size, NULL,
						   &desc_version, &desc_count,
						   &desc_size, &package_version,
						   &package_version_name));

		if (ret != EFI_BUFFER_TOO_SMALL) {
			/*
			 * An input of info_size=0 should always lead
			 * fmp->get_image_info to return BUFFER_TO_SMALL.
			 */
			EFI_PRINT("ESRT erroneous FMP implementation\n");
			continue;
		}

		ret = efi_allocate_pool(EFI_BOOT_SERVICES_DATA, info_size,
					(void **)&img_info);
		if (ret != EFI_SUCCESS) {
			EFI_PRINT("ESRT failed to allocate memory for image info\n");
			continue;
		}

		/*
		 * Calls to a FMP get_image_info method do not return the
		 * desc_count value if the return status differs from EFI_SUCCESS.
		 * We need to repeat the call to get_image_info with a properly
		 * sized buffer in order to obtain the real number of images
		 * handled by the FMP.
		 */
		ret = EFI_CALL(fmp->get_image_info(fmp, &info_size, img_info,
						   &desc_version, &desc_count,
						   &desc_size, &package_version,
						   &package_version_name));

		if (ret != EFI_SUCCESS) {
			EFI_PRINT("ESRT failed to obtain image info from FMP\n");
			efi_free_pool(img_info);
			continue;
		}

		num_entries += desc_count;

		efi_free_pool(img_info);
	}

	/* error occurs in fmp->get_image_info() if num_entries is 0 here */
	if (!num_entries) {
		EFI_PRINT("Error occurs, num_entries should not be 0\n");
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	EFI_PRINT("ESRT create table with %u entries\n", num_entries);
	/*
	 * Allocate an ESRT with the sufficient number of entries to accommodate
	 * all the FMPs in the system.
	 */
	ret = efi_esrt_allocate_install(num_entries);
	if (ret != EFI_SUCCESS) {
		EFI_PRINT("ESRT failed to initialize table\n");
		goto out;
	}

	/*
	 * Populate the ESRT entries with all existing FMP.
	 */
	it_handle = base_handle;
	for (u32 idx = 0; idx < no_handles; idx++, it_handle++) {
		ret = efi_search_protocol(*it_handle,
					  &efi_guid_firmware_management_protocol,
					  &handler);

		if (ret != EFI_SUCCESS) {
			EFI_PRINT("ESRT unable to find FMP handle (%u)\n",
				  idx);
			continue;
		}
		fmp = handler->protocol_interface;

		ret = efi_esrt_add_from_fmp(fmp);
		if (ret != EFI_SUCCESS)
			EFI_PRINT("ESRT failed to add FMP to the table\n");
	}

out:

	efi_free_pool(base_handle);

	return ret;
}

/**
 * efi_esrt_new_fmp_notify() - Callback for the EVT_NOTIFY_SIGNAL event raised
 * when a new FMP protocol instance is registered in the system.
 */
static void EFIAPI efi_esrt_new_fmp_notify(struct efi_event *event,
					   void *context)
{
	efi_status_t ret;

	EFI_ENTRY();

	ret = efi_esrt_populate();
	if (ret != EFI_SUCCESS)
		EFI_PRINT("ESRT failed to populate ESRT entry\n");

	EFI_EXIT(ret);
}

/**
 * efi_esrt_register() - Install the ESRT system table.
 *
 * Return: status code
 */
efi_status_t efi_esrt_register(void)
{
	struct efi_event *ev = NULL;
	void *registration;
	efi_status_t ret;

	EFI_PRINT("ESRT creation start\n");

	ret = efi_esrt_populate();
	if (ret != EFI_SUCCESS) {
		EFI_PRINT("ESRT failed to initiate the table\n");
		return ret;
	}

	ret = efi_create_event(EVT_NOTIFY_SIGNAL, TPL_CALLBACK,
			       efi_esrt_new_fmp_notify, NULL, NULL, &ev);
	if (ret != EFI_SUCCESS) {
		EFI_PRINT("ESRT failed to create event\n");
		return ret;
	}

	ret = EFI_CALL(efi_register_protocol_notify(&efi_guid_firmware_management_protocol,
						    ev, &registration));
	if (ret != EFI_SUCCESS) {
		EFI_PRINT("ESRT failed to register FMP callback\n");
		return ret;
	}

	EFI_PRINT("ESRT table created\n");

	return ret;
}
