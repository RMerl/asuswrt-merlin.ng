// SPDX-License-Identifier: GPL-2.0+
/*
 * EFI Firmware management protocol
 *
 *  Copyright (c) 2020 Linaro Limited
 *			Author: AKASHI Takahiro
 */

#include <charset.h>
#include <dfu.h>
#include <efi_loader.h>
#include <efi_variable.h>
#include <fwu.h>
#include <image.h>
#include <signatures.h>

#include <linux/list.h>

#define FMP_PAYLOAD_HDR_SIGNATURE	SIGNATURE_32('M', 'S', 'S', '1')

/**
 * struct fmp_payload_header - EDK2 header for the FMP payload
 *
 * This structure describes the header which is preprended to the
 * FMP payload by the edk2 capsule generation scripts.
 *
 * @signature:			Header signature used to identify the header
 * @header_size:		Size of the structure
 * @fw_version:			Firmware versions used
 * @lowest_supported_version:	Lowest supported version
 */
struct fmp_payload_header {
	u32 signature;
	u32 header_size;
	u32 fw_version;
	u32 lowest_supported_version;
};

/**
 * struct fmp_state - fmp firmware update state
 *
 * This structure describes the state of the firmware update
 * through FMP protocol.
 *
 * @fw_version:			Firmware versions used
 * @lowest_supported_version:	Lowest supported version
 * @last_attempt_version:	Last attempt version
 * @last_attempt_status:	Last attempt status
 */
struct fmp_state {
	u32 fw_version;
	u32 lowest_supported_version; /* not used */
	u32 last_attempt_version; /* not used */
	u32 last_attempt_status; /* not used */
};

__weak void set_dfu_alt_info(char *interface, char *devstr)
{
	env_set("dfu_alt_info", update_info.dfu_string);
}

/**
 * efi_firmware_get_image_type_id - get image_type_id
 * @image_index:	image index
 *
 * Return the image_type_id identified by the image index.
 *
 * Return:		pointer to the image_type_id, NULL if image_index is invalid
 */
static
efi_guid_t *efi_firmware_get_image_type_id(u8 image_index)
{
	int i;
	struct efi_fw_image *fw_array;

	fw_array = update_info.images;
	for (i = 0; i < update_info.num_images; i++) {
		if (fw_array[i].image_index == image_index)
			return &fw_array[i].image_type_id;
	}

	return NULL;
}

/* Place holder; not supported */
static
efi_status_t EFIAPI efi_firmware_get_image_unsupported(
	struct efi_firmware_management_protocol *this,
	u8 image_index,
	void *image,
	efi_uintn_t *image_size)
{
	EFI_ENTRY("%p %d %p %p\n", this, image_index, image, image_size);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

/* Place holder; not supported */
static
efi_status_t EFIAPI efi_firmware_check_image_unsupported(
	struct efi_firmware_management_protocol *this,
	u8 image_index,
	const void *image,
	efi_uintn_t *image_size,
	u32 *image_updatable)
{
	EFI_ENTRY("%p %d %p %p %p\n", this, image_index, image, image_size,
		  image_updatable);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

/* Place holder; not supported */
static
efi_status_t EFIAPI efi_firmware_get_package_info_unsupported(
	struct efi_firmware_management_protocol *this,
	u32 *package_version,
	u16 **package_version_name,
	u32 *package_version_name_maxlen,
	u64 *attributes_supported,
	u64 *attributes_setting)
{
	EFI_ENTRY("%p %p %p %p %p %p\n", this, package_version,
		  package_version_name, package_version_name_maxlen,
		  attributes_supported, attributes_setting);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

/* Place holder; not supported */
static
efi_status_t EFIAPI efi_firmware_set_package_info_unsupported(
	struct efi_firmware_management_protocol *this,
	const void *image,
	efi_uintn_t *image_size,
	const void *vendor_code,
	u32 package_version,
	const u16 *package_version_name)
{
	EFI_ENTRY("%p %p %p %p %x %p\n", this, image, image_size, vendor_code,
		  package_version, package_version_name);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

/**
 * efi_firmware_get_lsv_from_dtb - get lowest supported version from dtb
 * @image_index:	Image index
 * @image_type_id:	Image type id
 * @lsv:		Pointer to store the lowest supported version
 *
 * Read the firmware version information from dtb.
 */
static void efi_firmware_get_lsv_from_dtb(u8 image_index,
					  efi_guid_t *image_type_id, u32 *lsv)
{
	const void *fdt = gd->fdt_blob;
	const fdt32_t *val;
	const char *guid_str;
	int len, offset, index;
	int parent, ret;

	*lsv = 0;

	parent = fdt_subnode_offset(fdt, 0, "firmware-version");
	if (parent < 0)
		return;

	fdt_for_each_subnode(offset, fdt, parent) {
		efi_guid_t guid;

		guid_str = fdt_getprop(fdt, offset, "image-type-id", &len);
		if (!guid_str)
			continue;
		ret = uuid_str_to_bin(guid_str, guid.b, UUID_STR_FORMAT_GUID);
		if (ret < 0) {
			log_warning("Wrong image-type-id format.\n");
			continue;
		}

		val = fdt_getprop(fdt, offset, "image-index", &len);
		if (!val)
			continue;
		index = fdt32_to_cpu(*val);

		if (!guidcmp(&guid, image_type_id) && index == image_index) {
			val = fdt_getprop(fdt, offset,
					  "lowest-supported-version", &len);
			if (val)
				*lsv = fdt32_to_cpu(*val);
		}
	}
}

/**
 * efi_firmware_fill_version_info - fill the version information
 * @image_info:		Image information
 * @fw_array:		Pointer to size of new image
 *
 * Fill the version information into image_info strucrure.
 *
 */
static
void efi_firmware_fill_version_info(struct efi_firmware_image_descriptor *image_info,
				    struct efi_fw_image *fw_array)
{
	u16 varname[13]; /* u"FmpStateXXXX" */
	efi_status_t ret;
	efi_uintn_t size, expected_size;
	uint num_banks = 1;
	uint active_index = 0;
	struct fmp_state *var_state;

	efi_firmware_get_lsv_from_dtb(fw_array->image_index,
				      &fw_array->image_type_id,
				      &image_info->lowest_supported_image_version);

	image_info->version_name = NULL; /* not supported */
	image_info->last_attempt_version = 0;
	image_info->last_attempt_status = LAST_ATTEMPT_STATUS_SUCCESS;
	image_info->version = 0;

	/* get the fw_version */
	efi_create_indexed_name(varname, sizeof(varname), "FmpState",
				fw_array->image_index);
	if (IS_ENABLED(CONFIG_FWU_MULTI_BANK_UPDATE)) {
		ret = fwu_get_active_index(&active_index);
		if (ret)
			return;

		num_banks = CONFIG_FWU_NUM_BANKS;
	}

	size = num_banks * sizeof(*var_state);
	expected_size = size;
	var_state = calloc(1, size);
	if (!var_state)
		return;

	ret = efi_get_variable_int(varname, &fw_array->image_type_id,
				   NULL, &size, var_state, NULL);
	if (ret == EFI_SUCCESS && expected_size == size)
		image_info->version = var_state[active_index].fw_version;

	free(var_state);
}

/**
 * efi_fill_image_desc_array - populate image descriptor array
 * @image_info_size:		Size of @image_info
 * @image_info:			Image information
 * @descriptor_version:		Pointer to version number
 * @descriptor_count:		Image count
 * @descriptor_size:		Pointer to descriptor size
 * @package_version:		Package version
 * @package_version_name:	Package version's name
 *
 * Return information about the current firmware image in @image_info.
 * @image_info will consist of a number of descriptors.
 * Each descriptor will be created based on efi_fw_image array.
 *
 * Return		status code
 */
static efi_status_t efi_fill_image_desc_array(
	efi_uintn_t *image_info_size,
	struct efi_firmware_image_descriptor *image_info,
	u32 *descriptor_version,
	u8 *descriptor_count,
	efi_uintn_t *descriptor_size,
	u32 *package_version,
	u16 **package_version_name)
{
	size_t total_size;
	struct efi_fw_image *fw_array;
	int i;

	total_size = sizeof(*image_info) * update_info.num_images;

	if (*image_info_size < total_size) {
		*image_info_size = total_size;

		return EFI_BUFFER_TOO_SMALL;
	}
	*image_info_size = total_size;

	fw_array = update_info.images;
	*descriptor_count = update_info.num_images;
	*descriptor_version = EFI_FIRMWARE_IMAGE_DESCRIPTOR_VERSION;
	*descriptor_size = sizeof(*image_info);
	*package_version = 0xffffffff; /* not supported */
	*package_version_name = NULL; /* not supported */

	for (i = 0; i < update_info.num_images; i++) {
		image_info[i].image_index = fw_array[i].image_index;
		image_info[i].image_type_id = fw_array[i].image_type_id;
		image_info[i].image_id = fw_array[i].image_index;
		image_info[i].image_id_name = fw_array[i].fw_name;

		efi_firmware_fill_version_info(&image_info[i], &fw_array[i]);

		image_info[i].size = 0;
		image_info[i].attributes_supported =
			IMAGE_ATTRIBUTE_IMAGE_UPDATABLE |
			IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED;
		image_info[i].attributes_setting =
				IMAGE_ATTRIBUTE_IMAGE_UPDATABLE;

		/* Check if the capsule authentication is enabled */
		if (IS_ENABLED(CONFIG_EFI_CAPSULE_AUTHENTICATE))
			image_info[0].attributes_setting |=
				IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED;

		image_info[i].hardware_instance = 1;
		image_info[i].dependencies = NULL;
	}

	return EFI_SUCCESS;
}

/**
 * efi_firmware_capsule_authenticate - authenticate the capsule if enabled
 * @p_image:		Pointer to new image
 * @p_image_size:	Pointer to size of new image
 *
 * Authenticate the capsule if authentication is enabled.
 * The image pointer and the image size are updated in case of success.
 *
 * Return:		status code
 */
static
efi_status_t efi_firmware_capsule_authenticate(const void **p_image,
					       efi_uintn_t *p_image_size)
{
	const void *image = *p_image;
	efi_uintn_t image_size = *p_image_size;
	void *capsule_payload;
	efi_status_t status;
	efi_uintn_t capsule_payload_size;

	if (IS_ENABLED(CONFIG_EFI_CAPSULE_AUTHENTICATE)) {
		capsule_payload = NULL;
		capsule_payload_size = 0;
		status = efi_capsule_authenticate(image, image_size,
						  &capsule_payload,
						  &capsule_payload_size);

		if (status == EFI_SECURITY_VIOLATION) {
			printf("Capsule authentication check failed. Aborting update\n");
			return status;
		} else if (status != EFI_SUCCESS) {
			return status;
		}

		debug("Capsule authentication successful\n");
		image = capsule_payload;
		image_size = capsule_payload_size;
	} else {
		debug("Capsule authentication disabled. ");
		debug("Updating capsule without authenticating.\n");
	}

	*p_image = image;
	*p_image_size = image_size;
	return EFI_SUCCESS;
}

/**
 * efi_firmware_set_fmp_state_var - set FmpStateXXXX variable
 * @state:		Pointer to fmp state
 * @image_index:	image index
 *
 * Update the FmpStateXXXX variable with the firmware update state.
 *
 * Return:		status code
 */
static
efi_status_t efi_firmware_set_fmp_state_var(struct fmp_state *state, u8 image_index)
{
	u16 varname[13]; /* u"FmpStateXXXX" */
	efi_status_t ret;
	uint num_banks = 1;
	uint update_bank = 0;
	efi_uintn_t size;
	efi_guid_t *image_type_id;
	struct fmp_state *var_state;

	image_type_id = efi_firmware_get_image_type_id(image_index);
	if (!image_type_id)
		return EFI_INVALID_PARAMETER;

	efi_create_indexed_name(varname, sizeof(varname), "FmpState",
				image_index);

	if (IS_ENABLED(CONFIG_FWU_MULTI_BANK_UPDATE)) {
		ret = fwu_plat_get_update_index(&update_bank);
		if (ret)
			return EFI_INVALID_PARAMETER;

		num_banks = CONFIG_FWU_NUM_BANKS;
	}

	size = num_banks * sizeof(*var_state);
	var_state = malloc(size);
	if (!var_state)
		return EFI_OUT_OF_RESOURCES;

	/*
	 * GetVariable may fail, EFI_NOT_FOUND is returned if FmpState
	 * variable has not been set yet.
	 */
	ret = efi_get_variable_int(varname, image_type_id, NULL, &size,
				   var_state, NULL);
	if (ret != EFI_SUCCESS)
		memset(var_state, 0, num_banks * sizeof(*var_state));

	/*
	 * Only the fw_version is set here.
	 * lowest_supported_version in FmpState variable is ignored since
	 * it can be tampered if the file based EFI variable storage is used.
	 */
	var_state[update_bank].fw_version = state->fw_version;

	size = num_banks * sizeof(*var_state);
	ret = efi_set_variable_int(varname, image_type_id,
				   EFI_VARIABLE_READ_ONLY |
				   EFI_VARIABLE_NON_VOLATILE |
				   EFI_VARIABLE_BOOTSERVICE_ACCESS |
				   EFI_VARIABLE_RUNTIME_ACCESS,
				   size, var_state, false);

	free(var_state);

	return ret;
}

/**
 * efi_firmware_get_fw_version - get fw_version from FMP payload header
 * @p_image:		Pointer to new image
 * @p_image_size:	Pointer to size of new image
 * @state:		Pointer to fmp state
 *
 * Parse the FMP payload header and fill the fmp_state structure.
 * If no FMP payload header is found, fmp_state structure is not updated.
 *
 */
static void efi_firmware_get_fw_version(const void **p_image,
					efi_uintn_t *p_image_size,
					struct fmp_state *state)
{
	const struct fmp_payload_header *header;
	u32 fmp_hdr_signature = FMP_PAYLOAD_HDR_SIGNATURE;

	header = *p_image;
	if (header->signature == fmp_hdr_signature) {
		/* FMP header is inserted above the capsule payload */
		state->fw_version = header->fw_version;

		*p_image += header->header_size;
		*p_image_size -= header->header_size;
	}
}

/**
 * efi_firmware_verify_image - verify image
 * @p_image:		Pointer to new image
 * @p_image_size:	Pointer to size of new image
 * @image_index:	Image index
 * @state:		Pointer to fmp state
 *
 * Verify the capsule authentication and check if the fw_version
 * is equal or greater than the lowest supported version.
 *
 * Return:		status code
 */
static
efi_status_t efi_firmware_verify_image(const void **p_image,
				       efi_uintn_t *p_image_size,
				       u8 image_index,
				       struct fmp_state *state)
{
	u32 lsv;
	efi_status_t ret;
	efi_guid_t *image_type_id;

	ret = efi_firmware_capsule_authenticate(p_image, p_image_size);
	if (ret != EFI_SUCCESS)
		return ret;

	efi_firmware_get_fw_version(p_image, p_image_size, state);

	image_type_id = efi_firmware_get_image_type_id(image_index);
	if (!image_type_id)
		return EFI_INVALID_PARAMETER;

	efi_firmware_get_lsv_from_dtb(image_index, image_type_id, &lsv);
	if (state->fw_version < lsv) {
		log_err("Firmware version %u too low. Expecting >= %u. Aborting update\n",
			state->fw_version, lsv);
		return EFI_INVALID_PARAMETER;
	}

	return ret;
}

/**
 * efi_firmware_get_image_info - return information about the current
 *				     firmware image
 * @this:			Protocol instance
 * @image_info_size:		Size of @image_info
 * @image_info:			Image information
 * @descriptor_version:		Pointer to version number
 * @descriptor_count:		Pointer to number of descriptors
 * @descriptor_size:		Pointer to descriptor size
 * @package_version:		Package version
 * @package_version_name:	Package version's name
 *
 * Return information bout the current firmware image in @image_info.
 * @image_info will consist of a number of descriptors.
 * Each descriptor will be created based on "dfu_alt_info" variable.
 *
 * Return		status code
 */
static
efi_status_t EFIAPI efi_firmware_get_image_info(
	struct efi_firmware_management_protocol *this,
	efi_uintn_t *image_info_size,
	struct efi_firmware_image_descriptor *image_info,
	u32 *descriptor_version,
	u8 *descriptor_count,
	efi_uintn_t *descriptor_size,
	u32 *package_version,
	u16 **package_version_name)
{
	efi_status_t ret;

	EFI_ENTRY("%p %p %p %p %p %p %p %p\n", this,
		  image_info_size, image_info,
		  descriptor_version, descriptor_count, descriptor_size,
		  package_version, package_version_name);

	if (!image_info_size)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	if (*image_info_size &&
	    (!image_info || !descriptor_version || !descriptor_count ||
	     !descriptor_size || !package_version || !package_version_name))
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	ret = efi_fill_image_desc_array(image_info_size, image_info,
					descriptor_version, descriptor_count,
					descriptor_size, package_version,
					package_version_name);

	return EFI_EXIT(ret);
}

#ifdef CONFIG_EFI_CAPSULE_FIRMWARE_FIT
/*
 * This FIRMWARE_MANAGEMENT_PROTOCOL driver provides a firmware update
 * method with existing FIT image format, and handles
 *   - multiple regions of firmware via DFU
 * but doesn't support
 *   - versioning of firmware image
 *   - package information
 */

/**
 * efi_firmware_fit_set_image - update the firmware image
 * @this:		Protocol instance
 * @image_index:	Image index number
 * @image:		New image
 * @image_size:		Size of new image
 * @vendor_code:	Vendor-specific update policy
 * @progress:		Function to report the progress of update
 * @abort_reason:	Pointer to string of abort reason
 *
 * Update the firmware to new image, using dfu. The new image should
 * have FIT image format commonly used in U-Boot.
 * @vendor_code, @progress and @abort_reason are not supported.
 *
 * Return:		status code
 */
static
efi_status_t EFIAPI efi_firmware_fit_set_image(
	struct efi_firmware_management_protocol *this,
	u8 image_index,
	const void *image,
	efi_uintn_t image_size,
	const void *vendor_code,
	efi_status_t (*progress)(efi_uintn_t completion),
	u16 **abort_reason)
{
	efi_status_t status;
	struct fmp_state state = { 0 };

	EFI_ENTRY("%p %d %p %zu %p %p %p\n", this, image_index, image,
		  image_size, vendor_code, progress, abort_reason);

	if (!image || image_index != 1)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	status = efi_firmware_verify_image(&image, &image_size, image_index,
					   &state);
	if (status != EFI_SUCCESS)
		return EFI_EXIT(status);

	if (fit_update(image))
		return EFI_EXIT(EFI_DEVICE_ERROR);

	efi_firmware_set_fmp_state_var(&state, image_index);

	return EFI_EXIT(EFI_SUCCESS);
}

const struct efi_firmware_management_protocol efi_fmp_fit = {
	.get_image_info = efi_firmware_get_image_info,
	.get_image = efi_firmware_get_image_unsupported,
	.set_image = efi_firmware_fit_set_image,
	.check_image = efi_firmware_check_image_unsupported,
	.get_package_info = efi_firmware_get_package_info_unsupported,
	.set_package_info = efi_firmware_set_package_info_unsupported,
};
#endif /* CONFIG_EFI_CAPSULE_FIRMWARE_FIT */

#ifdef CONFIG_EFI_CAPSULE_FIRMWARE_RAW
/*
 * This FIRMWARE_MANAGEMENT_PROTOCOL driver provides a firmware update
 * method with raw data.
 */

/**
 * efi_firmware_raw_set_image - update the firmware image
 * @this:		Protocol instance
 * @image_index:	Image index number
 * @image:		New image
 * @image_size:		Size of new image
 * @vendor_code:	Vendor-specific update policy
 * @progress:		Function to report the progress of update
 * @abort_reason:	Pointer to string of abort reason
 *
 * Update the firmware to new image, using dfu. The new image should
 * be a single raw image.
 * @vendor_code, @progress and @abort_reason are not supported.
 *
 * Return:		status code
 */
static
efi_status_t EFIAPI efi_firmware_raw_set_image(
	struct efi_firmware_management_protocol *this,
	u8 image_index,
	const void *image,
	efi_uintn_t image_size,
	const void *vendor_code,
	efi_status_t (*progress)(efi_uintn_t completion),
	u16 **abort_reason)
{
	int ret;
	u8 dfu_alt_num;
	efi_status_t status;
	struct fmp_state state = { 0 };

	EFI_ENTRY("%p %d %p %zu %p %p %p\n", this, image_index, image,
		  image_size, vendor_code, progress, abort_reason);

	if (!image)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	status = efi_firmware_verify_image(&image, &image_size, image_index,
					   &state);
	if (status != EFI_SUCCESS)
		return EFI_EXIT(status);

	/*
	 * dfu_alt_num is assigned from 0 while image_index starts from 1.
	 * dfu_alt_num is calculated by (image_index - 1) when multi bank update
	 * is not used.
	 */
	dfu_alt_num = image_index - 1;
	if (IS_ENABLED(CONFIG_FWU_MULTI_BANK_UPDATE)) {
		/*
		 * Based on the value of update bank, derive the
		 * image index value.
		 */
		ret = fwu_get_dfu_alt_num(image_index, &dfu_alt_num);
		if (ret) {
			log_debug("Unable to get FWU image_index\n");
			return EFI_EXIT(EFI_DEVICE_ERROR);
		}
	}

	if (dfu_write_by_alt(dfu_alt_num, (void *)image, image_size,
			     NULL, NULL))
		return EFI_EXIT(EFI_DEVICE_ERROR);

	efi_firmware_set_fmp_state_var(&state, image_index);

	return EFI_EXIT(EFI_SUCCESS);
}

const struct efi_firmware_management_protocol efi_fmp_raw = {
	.get_image_info = efi_firmware_get_image_info,
	.get_image = efi_firmware_get_image_unsupported,
	.set_image = efi_firmware_raw_set_image,
	.check_image = efi_firmware_check_image_unsupported,
	.get_package_info = efi_firmware_get_package_info_unsupported,
	.set_package_info = efi_firmware_set_package_info_unsupported,
};
#endif /* CONFIG_EFI_CAPSULE_FIRMWARE_RAW */
