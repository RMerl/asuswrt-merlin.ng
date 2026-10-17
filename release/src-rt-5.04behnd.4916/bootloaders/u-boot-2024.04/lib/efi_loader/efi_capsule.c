// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI Capsule
 *
 *  Copyright (c) 2018 Linaro Limited
 *			Author: AKASHI Takahiro
 */

#define LOG_CATEGORY LOGC_EFI

#include <efi_loader.h>
#include <efi_variable.h>
#include <env.h>
#include <fdtdec.h>
#include <fs.h>
#include <fwu.h>
#include <hang.h>
#include <malloc.h>
#include <mapmem.h>
#include <sort.h>
#include <sysreset.h>
#include <asm/global_data.h>

#include <crypto/pkcs7.h>
#include <crypto/pkcs7_parser.h>
#include <linux/err.h>

DECLARE_GLOBAL_DATA_PTR;

const efi_guid_t efi_guid_capsule_report = EFI_CAPSULE_REPORT_GUID;
static const efi_guid_t efi_guid_firmware_management_capsule_id =
		EFI_FIRMWARE_MANAGEMENT_CAPSULE_ID_GUID;
const efi_guid_t efi_guid_firmware_management_protocol =
		EFI_FIRMWARE_MANAGEMENT_PROTOCOL_GUID;
const efi_guid_t fwu_guid_os_request_fw_revert =
		FWU_OS_REQUEST_FW_REVERT_GUID;
const efi_guid_t fwu_guid_os_request_fw_accept =
		FWU_OS_REQUEST_FW_ACCEPT_GUID;

#define FW_ACCEPT_OS	(u32)0x8000

#ifdef CONFIG_EFI_CAPSULE_ON_DISK
/* for file system access */
static struct efi_file_handle *bootdev_root;
#endif

static __maybe_unused unsigned int get_capsule_index(const u16 *variable_name)
{
	u16 value16[11]; /* "CapsuleXXXX": non-null-terminated */
	char value[5];
	efi_uintn_t size;
	unsigned long index = 0xffff;
	efi_status_t ret;
	int i;

	size = sizeof(value16);
	ret = efi_get_variable_int(variable_name, &efi_guid_capsule_report,
				   NULL, &size, value16, NULL);
	if (ret != EFI_SUCCESS || size != 22 ||
	    u16_strncmp(value16, u"Capsule", 7))
		goto err;
	for (i = 0; i < 4; ++i) {
		u16 c = value16[i + 7];

		if (!c || c > 0x7f)
			goto err;
		value[i] = c;
	}
	value[4] = 0;
	if (strict_strtoul(value, 16, &index))
		index = 0xffff;
err:
	return index;
}

/**
 * get_last_capsule - get the last capsule index
 *
 * Retrieve the index of the capsule invoked last time from "CapsuleLast"
 * variable.
 *
 * Return:
 * * > 0	- the last capsule index invoked
 * * 0xffff	- on error, or no capsule invoked yet
 */
static __maybe_unused unsigned int get_last_capsule(void)
{
	return get_capsule_index(u"CapsuleLast");
}

/**
 * get_max_capsule - get the max capsule index
 *
 * Retrieve the max capsule index value from "CapsuleMax" variable.
 *
 * Return:
 * * > 0	- the max capsule index
 * * 0xffff	- on error, or "CapsuleMax" variable does not exist
 */
static __maybe_unused unsigned int get_max_capsule(void)
{
	return get_capsule_index(u"CapsuleMax");
}

/**
 * set_capsule_result - set a result variable
 * @capsule:		Capsule
 * @return_status:	Return status
 *
 * Create and set a result variable, "CapsuleXXXX", for the capsule,
 * @capsule.
 */
static __maybe_unused
void set_capsule_result(int index, struct efi_capsule_header *capsule,
			efi_status_t return_status)
{
	u16 variable_name16[12];
	struct efi_capsule_result_variable_header result;
	struct efi_time time;
	efi_status_t ret;

	efi_create_indexed_name(variable_name16, sizeof(variable_name16),
				"Capsule", index);
	result.variable_total_size = sizeof(result);
	result.capsule_guid = capsule->capsule_guid;
	ret = EFI_CALL((*efi_runtime_services.get_time)(&time, NULL));
	if (ret == EFI_SUCCESS)
		memcpy(&result.capsule_processed, &time, sizeof(time));
	else
		memset(&result.capsule_processed, 0, sizeof(time));
	result.capsule_status = return_status;
	ret = efi_set_variable_int(variable_name16, &efi_guid_capsule_report,
				   EFI_VARIABLE_NON_VOLATILE |
				   EFI_VARIABLE_BOOTSERVICE_ACCESS |
				   EFI_VARIABLE_RUNTIME_ACCESS,
				   sizeof(result), &result, false);
	if (ret != EFI_SUCCESS) {
		log_err("Setting %ls failed\n", variable_name16);
		return;
	}

	/* Variable CapsuleLast must not include terminating 0x0000 */
	ret = efi_set_variable_int(u"CapsuleLast", &efi_guid_capsule_report,
				   EFI_VARIABLE_READ_ONLY |
				   EFI_VARIABLE_NON_VOLATILE |
				   EFI_VARIABLE_BOOTSERVICE_ACCESS |
				   EFI_VARIABLE_RUNTIME_ACCESS,
				   22, variable_name16, false);
	if (ret != EFI_SUCCESS)
		log_err("Setting %ls failed\n", u"CapsuleLast");
}

#ifdef CONFIG_EFI_CAPSULE_FIRMWARE_MANAGEMENT
/**
 * efi_fmp_find - search for Firmware Management Protocol drivers
 * @image_type:		Image type guid
 * @image_index:	Image Index
 * @instance:		Instance number
 * @handles:		Handles of FMP drivers
 * @no_handles:		Number of handles
 *
 * Search for Firmware Management Protocol drivers, matching the image
 * type, @image_type and the machine instance, @instance, from the list,
 * @handles.
 *
 * Return:
 * * Protocol instance	- on success
 * * NULL		- on failure
 */
static struct efi_firmware_management_protocol *
efi_fmp_find(efi_guid_t *image_type, u8 image_index, u64 instance,
	     efi_handle_t *handles, efi_uintn_t no_handles)
{
	efi_handle_t *handle;
	struct efi_firmware_management_protocol *fmp;
	struct efi_firmware_image_descriptor *image_info, *desc;
	efi_uintn_t info_size, descriptor_size;
	u32 descriptor_version;
	u8 descriptor_count;
	u32 package_version;
	u16 *package_version_name;
	bool found = false;
	int i, j;
	efi_status_t ret;

	for (i = 0, handle = handles; i < no_handles; i++, handle++) {
		struct efi_handler *fmp_handler;

		ret = efi_search_protocol(
				*handle, &efi_guid_firmware_management_protocol,
				&fmp_handler);
		if (ret != EFI_SUCCESS)
			continue;
		fmp = fmp_handler->protocol_interface;

		/* get device's image info */
		info_size = 0;
		image_info = NULL;
		descriptor_version = 0;
		descriptor_count = 0;
		descriptor_size = 0;
		package_version = 0;
		package_version_name = NULL;
		ret = EFI_CALL(fmp->get_image_info(fmp, &info_size,
						   image_info,
						   &descriptor_version,
						   &descriptor_count,
						   &descriptor_size,
						   &package_version,
						   &package_version_name));
		if (ret != EFI_BUFFER_TOO_SMALL)
			goto skip;

		image_info = malloc(info_size);
		if (!image_info)
			goto skip;

		ret = EFI_CALL(fmp->get_image_info(fmp, &info_size,
						   image_info,
						   &descriptor_version,
						   &descriptor_count,
						   &descriptor_size,
						   &package_version,
						   &package_version_name));
		if (ret != EFI_SUCCESS ||
		    descriptor_version != EFI_FIRMWARE_IMAGE_DESCRIPTOR_VERSION)
			goto skip;

		/* matching */
		for (j = 0, desc = image_info; j < descriptor_count;
		     j++, desc = (void *)desc + descriptor_size) {
			log_debug("+++ desc[%d] index: %d, name: %ls\n",
				  j, desc->image_index, desc->image_id_name);
			if (!guidcmp(&desc->image_type_id, image_type) &&
			    (desc->image_index == image_index) &&
			    (!instance ||
			     !desc->hardware_instance ||
			      desc->hardware_instance == instance))
				found = true;
		}

skip:
		efi_free_pool(package_version_name);
		free(image_info);
		if (found)
			return fmp;
	}

	return NULL;
}

/**
 * efi_remove_auth_hdr - remove authentication data from image
 * @image:	Pointer to pointer to Image
 * @image_size:	Pointer to Image size
 *
 * Remove the authentication data from image if possible.
 * Update @image and @image_size.
 *
 * Return:		status code
 */
static efi_status_t efi_remove_auth_hdr(void **image, efi_uintn_t *image_size)
{
	struct efi_firmware_image_authentication *auth_hdr;
	efi_status_t ret = EFI_INVALID_PARAMETER;

	auth_hdr = (struct efi_firmware_image_authentication *)*image;
	if (*image_size < sizeof(*auth_hdr))
		goto out;

	if (auth_hdr->auth_info.hdr.dwLength <=
	    offsetof(struct win_certificate_uefi_guid, cert_data))
		goto out;

	*image = (uint8_t *)*image + sizeof(auth_hdr->monotonic_count) +
		auth_hdr->auth_info.hdr.dwLength;
	*image_size = *image_size - auth_hdr->auth_info.hdr.dwLength -
		sizeof(auth_hdr->monotonic_count);

	ret = EFI_SUCCESS;
out:
	return ret;
}

#if defined(CONFIG_EFI_CAPSULE_AUTHENTICATE)
int efi_get_public_key_data(void **pkey, efi_uintn_t *pkey_len)
{
	const void *fdt_blob = gd->fdt_blob;
	const void *blob;
	const char *cnode_name = "capsule-key";
	const char *snode_name = "signature";
	int sig_node;
	int len;

	sig_node = fdt_subnode_offset(fdt_blob, 0, snode_name);
	if (sig_node < 0) {
		log_err("Unable to get signature node offset\n");

		return -FDT_ERR_NOTFOUND;
	}

	blob = fdt_getprop(fdt_blob, sig_node, cnode_name, &len);

	if (!blob || len < 0) {
		log_err("Unable to get capsule-key value\n");
		*pkey = NULL;
		*pkey_len = 0;

		return -FDT_ERR_NOTFOUND;
	}

	*pkey = (void *)blob;
	*pkey_len = len;

	return 0;
}

efi_status_t efi_capsule_authenticate(const void *capsule, efi_uintn_t capsule_size,
				      void **image, efi_uintn_t *image_size)
{
	u8 *buf;
	int ret;
	void *fdt_pkey, *pkey;
	efi_uintn_t pkey_len;
	uint64_t monotonic_count;
	struct efi_signature_store *truststore;
	struct pkcs7_message *capsule_sig;
	struct efi_image_regions *regs;
	struct efi_firmware_image_authentication *auth_hdr;
	efi_status_t status;

	status = EFI_SECURITY_VIOLATION;
	capsule_sig = NULL;
	truststore = NULL;
	regs = NULL;

	/* Sanity checks */
	if (capsule == NULL || capsule_size == 0)
		goto out;

	*image = (uint8_t *)capsule;
	*image_size = capsule_size;
	if (efi_remove_auth_hdr(image, image_size) != EFI_SUCCESS)
		goto out;

	auth_hdr = (struct efi_firmware_image_authentication *)capsule;
	if (guidcmp(&auth_hdr->auth_info.cert_type, &efi_guid_cert_type_pkcs7))
		goto out;

	memcpy(&monotonic_count, &auth_hdr->monotonic_count,
	       sizeof(monotonic_count));

	/* data to be digested */
	regs = calloc(sizeof(*regs) + sizeof(struct image_region) * 2, 1);
	if (!regs)
		goto out;

	regs->max = 2;
	efi_image_region_add(regs, (uint8_t *)*image,
			     (uint8_t *)*image + *image_size, 1);

	efi_image_region_add(regs, (uint8_t *)&monotonic_count,
			     (uint8_t *)&monotonic_count + sizeof(monotonic_count),
			     1);

	capsule_sig = efi_parse_pkcs7_header(auth_hdr->auth_info.cert_data,
					     auth_hdr->auth_info.hdr.dwLength
					     - sizeof(auth_hdr->auth_info),
					     &buf);
	if (!capsule_sig) {
		debug("Parsing variable's pkcs7 header failed\n");
		goto out;
	}

	ret = efi_get_public_key_data(&fdt_pkey, &pkey_len);
	if (ret < 0)
		goto out;

	pkey = malloc(pkey_len);
	if (!pkey)
		goto out;

	memcpy(pkey, fdt_pkey, pkey_len);
	truststore = efi_build_signature_store(pkey, pkey_len);
	if (!truststore)
		goto out;

	/* verify signature */
	if (efi_signature_verify(regs, capsule_sig, truststore, NULL)) {
		debug("Verified\n");
	} else {
		debug("Verifying variable's signature failed\n");
		goto out;
	}

	status = EFI_SUCCESS;

out:
	efi_sigstore_free(truststore);
	pkcs7_free_message(capsule_sig);
	free(regs);

	return status;
}
#endif /* CONFIG_EFI_CAPSULE_AUTHENTICATE */

static __maybe_unused bool fwu_empty_capsule(struct efi_capsule_header *capsule)
{
	return !guidcmp(&capsule->capsule_guid,
			&fwu_guid_os_request_fw_revert) ||
		!guidcmp(&capsule->capsule_guid,
			 &fwu_guid_os_request_fw_accept);
}

static __maybe_unused efi_status_t fwu_to_efi_error(int err)
{
	efi_status_t ret;

	switch(err) {
	case 0:
		ret = EFI_SUCCESS;
		break;
	case -ERANGE:
	case -EIO:
		ret = EFI_DEVICE_ERROR;
		break;
	case -EINVAL:
		ret = EFI_INVALID_PARAMETER;
		break;
	case -ENODEV:
		ret = EFI_NOT_FOUND;
		break;
	default:
		ret = EFI_OUT_OF_RESOURCES;
	}

	return ret;
}

static __maybe_unused efi_status_t fwu_empty_capsule_process(
	struct efi_capsule_header *capsule)
{
	int status;
	u32 active_idx;
	efi_guid_t *image_guid;
	efi_status_t ret = EFI_INVALID_PARAMETER;

	if (!guidcmp(&capsule->capsule_guid,
		     &fwu_guid_os_request_fw_revert)) {
		/*
		 * One of the previously updated image has
		 * failed the OS acceptance test. OS has
		 * requested to revert back to the earlier
		 * boot index
		 */
		status = fwu_revert_boot_index();
		ret = fwu_to_efi_error(status);
		if (ret == EFI_SUCCESS)
			log_debug("Reverted the FWU active_index. Recommend rebooting the system\n");
		else
			log_err("Failed to revert the FWU boot index\n");
	} else if (!guidcmp(&capsule->capsule_guid,
			    &fwu_guid_os_request_fw_accept)) {
		/*
		 * Image accepted by the OS. Set the acceptance
		 * status for the image.
		 */
		image_guid = (void *)(char *)capsule +
			capsule->header_size;

		status = fwu_get_active_index(&active_idx);
		ret = fwu_to_efi_error(status);
		if (ret != EFI_SUCCESS) {
			log_err("Unable to get the active_index from the FWU metadata\n");
			return ret;
		}

		status = fwu_accept_image(image_guid, active_idx);
		ret = fwu_to_efi_error(status);
		if (ret != EFI_SUCCESS)
			log_err("Unable to set the Accept bit for the image %pUs\n",
				image_guid);
	}

	return ret;
}

static __maybe_unused void fwu_post_update_checks(
	struct efi_capsule_header *capsule,
	bool *fw_accept_os, bool *capsule_update)
{
	if (fwu_empty_capsule(capsule))
		*capsule_update = false;
	else
		if (!*fw_accept_os)
			*fw_accept_os =
				capsule->flags & FW_ACCEPT_OS ? true : false;
}

static __maybe_unused efi_status_t fwu_post_update_process(bool fw_accept_os)
{
	int status;
	uint update_index;
	efi_status_t ret;

	status = fwu_plat_get_update_index(&update_index);
	if (status < 0) {
		log_err("Failed to get the FWU update_index value\n");
		return EFI_DEVICE_ERROR;
	}

	/*
	 * All the capsules have been updated successfully,
	 * update the FWU metadata.
	 */
	log_debug("Update Complete. Now updating active_index to %u\n",
		  update_index);
	status = fwu_set_active_index(update_index);
	ret = fwu_to_efi_error(status);
	if (ret != EFI_SUCCESS) {
		log_err("Failed to update FWU metadata index values\n");
	} else {
		log_debug("Successfully updated the active_index\n");
		if (fw_accept_os) {
			status = fwu_trial_state_ctr_start();
			if (status < 0)
				ret = EFI_DEVICE_ERROR;
		}
	}

	return ret;
}

/**
 * efi_capsule_update_firmware - update firmware from capsule
 * @capsule_data:	Capsule
 *
 * Update firmware, using a capsule, @capsule_data. Loading any FMP
 * drivers embedded in a capsule is not supported.
 *
 * Return:		status code
 */
static efi_status_t efi_capsule_update_firmware(
		struct efi_capsule_header *capsule_data)
{
	struct efi_firmware_management_capsule_header *capsule;
	struct efi_firmware_management_capsule_image_header *image;
	size_t capsule_size, image_binary_size;
	void *image_binary, *vendor_code;
	efi_handle_t *handles;
	efi_uintn_t no_handles;
	int item;
	struct efi_firmware_management_protocol *fmp;
	u16 *abort_reason;
	efi_guid_t *image_type_id;
	efi_status_t ret = EFI_SUCCESS;
	int status;
	uint update_index;
	bool fw_accept_os;

	if (IS_ENABLED(CONFIG_FWU_MULTI_BANK_UPDATE)) {
		if (fwu_empty_capsule_checks_pass() &&
		    fwu_empty_capsule(capsule_data))
			return fwu_empty_capsule_process(capsule_data);

		if (!fwu_update_checks_pass()) {
			log_err("FWU checks failed. Cannot start update\n");
			return EFI_INVALID_PARAMETER;
		}


		/* Obtain the update_index from the platform */
		status = fwu_plat_get_update_index(&update_index);
		if (status < 0) {
			log_err("Failed to get the FWU update_index value\n");
			return EFI_DEVICE_ERROR;
		}

		fw_accept_os = capsule_data->flags & FW_ACCEPT_OS ? 0x1 : 0x0;
	}

	if (guidcmp(&capsule_data->capsule_guid,
		    &efi_guid_firmware_management_capsule_id)) {
		log_err("Unsupported capsule type: %pUs\n",
			&capsule_data->capsule_guid);
		return EFI_UNSUPPORTED;
	}

	/* sanity check */
	if (capsule_data->header_size < sizeof(*capsule) ||
	    capsule_data->header_size >= capsule_data->capsule_image_size)
		return EFI_INVALID_PARAMETER;

	capsule = (void *)capsule_data + capsule_data->header_size;
	capsule_size = capsule_data->capsule_image_size
			- capsule_data->header_size;

	if (capsule->version != 0x00000001)
		return EFI_UNSUPPORTED;

	handles = NULL;
	ret = EFI_CALL(efi_locate_handle_buffer(
			BY_PROTOCOL,
			&efi_guid_firmware_management_protocol,
			NULL, &no_handles, (efi_handle_t **)&handles));
	if (ret != EFI_SUCCESS)
		return EFI_UNSUPPORTED;

	/* Payload */
	for (item = capsule->embedded_driver_count;
	     item < capsule->embedded_driver_count
		    + capsule->payload_item_count; item++) {
		/* sanity check */
		if ((capsule->item_offset_list[item] + sizeof(*image)
				 >= capsule_size)) {
			log_err("Capsule does not have enough data\n");
			ret = EFI_INVALID_PARAMETER;
			goto out;
		}

		image = (void *)capsule + capsule->item_offset_list[item];

		if (image->version != 0x00000003) {
			ret = EFI_UNSUPPORTED;
			goto out;
		}

		/* find a device for update firmware */
		fmp = efi_fmp_find(&image->update_image_type_id,
				   image->update_image_index,
				   image->update_hardware_instance,
				   handles, no_handles);
		if (!fmp) {
			log_err("FMP driver not found for firmware type %pUs, hardware instance %lld\n",
				&image->update_image_type_id,
				image->update_hardware_instance);
			ret = EFI_UNSUPPORTED;
			goto out;
		}

		/* do update */
		if (IS_ENABLED(CONFIG_EFI_CAPSULE_AUTHENTICATE) &&
		    !(image->image_capsule_support &
				CAPSULE_SUPPORT_AUTHENTICATION)) {
			/* no signature */
			ret = EFI_SECURITY_VIOLATION;
			goto out;
		}

		image_binary = (void *)image + sizeof(*image);
		image_binary_size = image->update_image_size;
		vendor_code = image_binary + image_binary_size;
		if (!IS_ENABLED(CONFIG_EFI_CAPSULE_AUTHENTICATE) &&
		    (image->image_capsule_support &
				CAPSULE_SUPPORT_AUTHENTICATION)) {
			ret = efi_remove_auth_hdr(&image_binary,
						  &image_binary_size);
			if (ret != EFI_SUCCESS)
				goto out;
		}

		abort_reason = NULL;
		ret = EFI_CALL(fmp->set_image(fmp, image->update_image_index,
					      image_binary,
					      image_binary_size,
					      vendor_code, NULL,
					      &abort_reason));
		if (ret != EFI_SUCCESS) {
			log_err("Firmware update failed: %ls\n",
				abort_reason);
			efi_free_pool(abort_reason);
			goto out;
		}

		if (IS_ENABLED(CONFIG_FWU_MULTI_BANK_UPDATE)) {
			image_type_id = &image->update_image_type_id;
			if (!fw_accept_os) {
				/*
				 * The OS will not be accepting the firmware
				 * images. Set the accept bit of all the
				 * images contained in this capsule.
				 */
				status = fwu_accept_image(image_type_id,
							  update_index);
			} else {
				status = fwu_clear_accept_image(image_type_id,
								update_index);
			}
			ret = fwu_to_efi_error(status);
			if (ret != EFI_SUCCESS) {
				log_err("Unable to %s the accept bit for the image %pUs\n",
					fw_accept_os ? "clear" : "set",
					image_type_id);
				goto out;
			}

			log_debug("%s the accepted bit for Image %pUs\n",
				  fw_accept_os ? "Cleared" : "Set",
				  image_type_id);
		}

	}

out:
	efi_free_pool(handles);

	return ret;
}
#else
static efi_status_t efi_capsule_update_firmware(
		struct efi_capsule_header *capsule_data)
{
	return EFI_UNSUPPORTED;
}
#endif /* CONFIG_EFI_CAPSULE_FIRMWARE_MANAGEMENT */

/**
 * efi_update_capsule() - process information from operating system
 * @capsule_header_array:	Array of virtual address pointers
 * @capsule_count:		Number of pointers in capsule_header_array
 * @scatter_gather_list:	Array of physical address pointers
 *
 * This function implements the UpdateCapsule() runtime service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return:			status code
 */
efi_status_t EFIAPI efi_update_capsule(
		struct efi_capsule_header **capsule_header_array,
		efi_uintn_t capsule_count,
		u64 scatter_gather_list)
{
	struct efi_capsule_header *capsule;
	unsigned int i;
	efi_status_t ret;

	EFI_ENTRY("%p, %zu, %llu\n", capsule_header_array, capsule_count,
		  scatter_gather_list);

	if (!capsule_count) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	ret = EFI_SUCCESS;
	for (i = 0, capsule = *capsule_header_array; i < capsule_count;
	     i++, capsule = *(++capsule_header_array)) {
		/* sanity check */
		if (capsule->header_size < sizeof(*capsule) ||
		    capsule->capsule_image_size < sizeof(*capsule)) {
			log_err("Capsule does not have enough data\n");
			continue;
		}

		log_debug("Capsule[%d] (guid:%pUs)\n",
			  i, &capsule->capsule_guid);
		ret  = efi_capsule_update_firmware(capsule);
		if (ret != EFI_SUCCESS)
			goto out;
	}

	if (IS_ENABLED(CONFIG_EFI_ESRT)) {
		/* Rebuild the ESRT to reflect any updated FW images. */
		ret = efi_esrt_populate();
		if (ret != EFI_SUCCESS)
			log_warning("ESRT update failed\n");
	}
out:

	return EFI_EXIT(ret);
}

/**
 * efi_query_capsule_caps() - check if capsule is supported
 * @capsule_header_array:	Array of virtual pointers
 * @capsule_count:		Number of pointers in capsule_header_array
 * @maximum_capsule_size:	Maximum capsule size
 * @reset_type:			Type of reset needed for capsule update
 *
 * This function implements the QueryCapsuleCapabilities() runtime service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return:			status code
 */
efi_status_t EFIAPI efi_query_capsule_caps(
		struct efi_capsule_header **capsule_header_array,
		efi_uintn_t capsule_count,
		u64 *maximum_capsule_size,
		u32 *reset_type)
{
	struct efi_capsule_header *capsule __attribute__((unused));
	unsigned int i;
	efi_status_t ret;

	EFI_ENTRY("%p, %zu, %p, %p\n", capsule_header_array, capsule_count,
		  maximum_capsule_size, reset_type);

	if (!maximum_capsule_size) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	*maximum_capsule_size = U64_MAX;
	*reset_type = EFI_RESET_COLD;

	ret = EFI_SUCCESS;
	for (i = 0, capsule = *capsule_header_array; i < capsule_count;
	     i++, capsule = *(++capsule_header_array)) {
		/* TODO */
	}
out:
	return EFI_EXIT(ret);
}

/**
 * efi_load_capsule_drivers - initialize capsule drivers
 *
 * Generic FMP drivers backed by DFU
 *
 * Return:	status code
 */
efi_status_t __weak efi_load_capsule_drivers(void)
{
	__maybe_unused efi_handle_t handle;
	efi_status_t ret = EFI_SUCCESS;

	if (IS_ENABLED(CONFIG_EFI_CAPSULE_FIRMWARE_FIT)) {
		handle = NULL;
		ret = efi_install_multiple_protocol_interfaces(&handle,
							       &efi_guid_firmware_management_protocol,
							       &efi_fmp_fit,
							       NULL);
	}

	if (IS_ENABLED(CONFIG_EFI_CAPSULE_FIRMWARE_RAW)) {
		handle = NULL;
		ret = efi_install_multiple_protocol_interfaces(&handle,
							       &efi_guid_firmware_management_protocol,
							       &efi_fmp_raw,
							       NULL);
	}

	return ret;
}

#ifdef CONFIG_EFI_CAPSULE_ON_DISK
/**
 * get_dp_device - retrieve a device  path from boot variable
 * @boot_var:	Boot variable name
 * @device_dp	Device path
 *
 * Retrieve a device patch from boot variable, @boot_var.
 *
 * Return:	status code
 */
static efi_status_t get_dp_device(u16 *boot_var,
				  struct efi_device_path **device_dp)
{
	void *buf = NULL;
	efi_uintn_t size;
	struct efi_load_option lo;
	struct efi_device_path *file_dp;
	efi_status_t ret;

	size = 0;
	ret = efi_get_variable_int(boot_var, &efi_global_variable_guid,
				   NULL, &size, NULL, NULL);
	if (ret == EFI_BUFFER_TOO_SMALL) {
		buf = malloc(size);
		if (!buf)
			return EFI_OUT_OF_RESOURCES;
		ret = efi_get_variable_int(boot_var, &efi_global_variable_guid,
					   NULL, &size, buf, NULL);
	}
	if (ret != EFI_SUCCESS)
		return ret;

	efi_deserialize_load_option(&lo, buf, &size);

	if (lo.attributes & LOAD_OPTION_ACTIVE) {
		efi_dp_split_file_path(lo.file_path, device_dp, &file_dp);
		efi_free_pool(file_dp);

		ret = EFI_SUCCESS;
	} else {
		ret = EFI_NOT_FOUND;
	}

	free(buf);

	return ret;
}

/**
 * device_is_present_and_system_part - check if a device exists
 *
 * Check if a device pointed to by the device path, @dp, exists and is
 * located in UEFI system partition.
 *
 * @dp		device path
 * Return:	true - yes, false - no
 */
static bool device_is_present_and_system_part(struct efi_device_path *dp)
{
	efi_handle_t handle;
	struct efi_device_path *rem;

	/* Check device exists */
	handle = efi_dp_find_obj(dp, NULL, NULL);
	if (!handle)
		return false;

	/* Check device is on system partition */
	handle = efi_dp_find_obj(dp, &efi_system_partition_guid, &rem);
	if (!handle)
		return false;

	return true;
}

/**
 * find_boot_device - identify the boot device
 *
 * Identify the boot device from boot-related variables as UEFI
 * specification describes and put its handle into bootdev_root.
 *
 * Return:	status code
 */
static efi_status_t find_boot_device(void)
{
	char boot_var[9];
	u16 boot_var16[9], *p, bootnext, *boot_order = NULL;
	efi_uintn_t size;
	int i, num;
	struct efi_simple_file_system_protocol *volume;
	struct efi_device_path *boot_dev = NULL;
	efi_status_t ret;

	/* find active boot device in BootNext */
	bootnext = 0;
	size = sizeof(bootnext);
	ret = efi_get_variable_int(u"BootNext",
				   (efi_guid_t *)&efi_global_variable_guid,
				   NULL, &size, &bootnext, NULL);
	if (ret == EFI_SUCCESS || ret == EFI_BUFFER_TOO_SMALL) {
		/* BootNext does exist here */
		if (ret == EFI_BUFFER_TOO_SMALL || size != sizeof(u16)) {
			log_err("BootNext must be 16-bit integer\n");
			goto skip;
		}
		sprintf((char *)boot_var, "Boot%04X", bootnext);
		p = boot_var16;
		utf8_utf16_strcpy(&p, boot_var);

		ret = get_dp_device(boot_var16, &boot_dev);
		if (ret == EFI_SUCCESS) {
			if (device_is_present_and_system_part(boot_dev)) {
				goto found;
			} else {
				efi_free_pool(boot_dev);
				boot_dev = NULL;
			}
		}
	}

skip:
	/* find active boot device in BootOrder */
	size = 0;
	ret = efi_get_variable_int(u"BootOrder", &efi_global_variable_guid,
				   NULL, &size, NULL, NULL);
	if (ret == EFI_BUFFER_TOO_SMALL) {
		boot_order = malloc(size);
		if (!boot_order) {
			ret = EFI_OUT_OF_RESOURCES;
			goto out;
		}

		ret = efi_get_variable_int(u"BootOrder",
					   &efi_global_variable_guid,
					   NULL, &size, boot_order, NULL);
	}
	if (ret != EFI_SUCCESS)
		goto out;

	/* check in higher order */
	num = size / sizeof(u16);
	for (i = 0; i < num; i++) {
		sprintf((char *)boot_var, "Boot%04X", boot_order[i]);
		p = boot_var16;
		utf8_utf16_strcpy(&p, boot_var);
		ret = get_dp_device(boot_var16, &boot_dev);
		if (ret != EFI_SUCCESS)
			continue;

		if (device_is_present_and_system_part(boot_dev))
			break;

		efi_free_pool(boot_dev);
		boot_dev = NULL;
	}
found:
	if (boot_dev) {
		log_debug("Boot device %pD\n", boot_dev);

		volume = efi_fs_from_path(boot_dev);
		if (!volume)
			ret = EFI_DEVICE_ERROR;
		else
			ret = EFI_CALL(volume->open_volume(volume,
							   &bootdev_root));
		efi_free_pool(boot_dev);
	} else {
		ret = EFI_NOT_FOUND;
	}
out:
	free(boot_order);

	return ret;
}

/**
 * efi_capsule_scan_dir - traverse a capsule directory in boot device
 * @files:	Array of file names
 * @num:	Number of elements in @files
 *
 * Traverse a capsule directory in boot device.
 * Called by initialization code, and returns an array of capsule file
 * names in @files.
 *
 * Return:	status code
 */
static efi_status_t efi_capsule_scan_dir(u16 ***files, unsigned int *num)
{
	struct efi_file_handle *dirh;
	struct efi_file_info *dirent;
	efi_uintn_t dirent_size, tmp_size;
	unsigned int count;
	u16 **tmp_files;
	efi_status_t ret;

	ret = find_boot_device();
	if (ret == EFI_NOT_FOUND) {
		log_debug("Boot device is not set\n");
		*num = 0;
		return EFI_SUCCESS;
	} else if (ret != EFI_SUCCESS) {
		return EFI_DEVICE_ERROR;
	}

	/* count capsule files */
	ret = EFI_CALL((*bootdev_root->open)(bootdev_root, &dirh,
					     EFI_CAPSULE_DIR,
					     EFI_FILE_MODE_READ, 0));
	if (ret != EFI_SUCCESS) {
		*num = 0;
		return EFI_SUCCESS;
	}

	dirent_size = 256;
	dirent = malloc(dirent_size);
	if (!dirent)
		return EFI_OUT_OF_RESOURCES;

	count = 0;
	while (1) {
		tmp_size = dirent_size;
		ret = EFI_CALL((*dirh->read)(dirh, &tmp_size, dirent));
		if (ret == EFI_BUFFER_TOO_SMALL) {
			struct efi_file_info *old_dirent = dirent;

			dirent = realloc(dirent, tmp_size);
			if (!dirent) {
				dirent = old_dirent;
				ret = EFI_OUT_OF_RESOURCES;
				goto err;
			}
			dirent_size = tmp_size;
			ret = EFI_CALL((*dirh->read)(dirh, &tmp_size, dirent));
		}
		if (ret != EFI_SUCCESS)
			goto err;
		if (!tmp_size)
			break;

		if (!(dirent->attribute & EFI_FILE_DIRECTORY))
			count++;
	}

	ret = EFI_CALL((*dirh->setpos)(dirh, 0));
	if (ret != EFI_SUCCESS)
		goto err;

	/* make a list */
	tmp_files = malloc(count * sizeof(*tmp_files));
	if (!tmp_files) {
		ret = EFI_OUT_OF_RESOURCES;
		goto err;
	}

	count = 0;
	while (1) {
		tmp_size = dirent_size;
		ret = EFI_CALL((*dirh->read)(dirh, &tmp_size, dirent));
		if (ret != EFI_SUCCESS)
			goto err;
		if (!tmp_size)
			break;

		if (!(dirent->attribute & EFI_FILE_DIRECTORY) &&
		    u16_strcmp(dirent->file_name, u".") &&
		    u16_strcmp(dirent->file_name, u".."))
			tmp_files[count++] = u16_strdup(dirent->file_name);
	}
	/* ignore an error */
	EFI_CALL((*dirh->close)(dirh));

	/*
	 * Capsule files are applied in case insensitive alphabetic order
	 *
	 * TODO: special handling of rightmost period
	 */
	qsort(tmp_files, count, sizeof(*tmp_files),
	      (int (*)(const void *, const void *))u16_strcasecmp);
	*files = tmp_files;
	*num = count;
	ret = EFI_SUCCESS;
err:
	free(dirent);

	return ret;
}

/**
 * efi_capsule_read_file - read in a capsule file
 * @filename:	File name
 * @capsule:	Pointer to buffer for capsule
 *
 * Read a capsule file and put its content in @capsule.
 *
 * Return:	status code
 */
static efi_status_t efi_capsule_read_file(const u16 *filename,
					  struct efi_capsule_header **capsule)
{
	struct efi_file_handle *dirh, *fh;
	struct efi_file_info *file_info = NULL;
	struct efi_capsule_header *buf = NULL;
	efi_uintn_t size;
	efi_status_t ret;

	ret = EFI_CALL((*bootdev_root->open)(bootdev_root, &dirh,
					     EFI_CAPSULE_DIR,
					     EFI_FILE_MODE_READ, 0));
	if (ret != EFI_SUCCESS)
		return ret;
	ret = EFI_CALL((*dirh->open)(dirh, &fh, (u16 *)filename,
				     EFI_FILE_MODE_READ, 0));
	/* ignore an error */
	EFI_CALL((*dirh->close)(dirh));
	if (ret != EFI_SUCCESS)
		return ret;

	/* file size */
	size = 0;
	ret = EFI_CALL((*fh->getinfo)(fh, &efi_file_info_guid,
				      &size, file_info));
	if (ret == EFI_BUFFER_TOO_SMALL) {
		file_info = malloc(size);
		if (!file_info) {
			ret = EFI_OUT_OF_RESOURCES;
			goto err;
		}
		ret = EFI_CALL((*fh->getinfo)(fh, &efi_file_info_guid,
					      &size, file_info));
	}
	if (ret != EFI_SUCCESS)
		goto err;
	size = file_info->file_size;
	free(file_info);
	buf = malloc(size);
	if (!buf) {
		ret = EFI_OUT_OF_RESOURCES;
		goto err;
	}

	/* fetch data */
	ret = EFI_CALL((*fh->read)(fh, &size, buf));
	if (ret == EFI_SUCCESS) {
		if (size >= buf->capsule_image_size) {
			*capsule = buf;
		} else {
			free(buf);
			ret = EFI_INVALID_PARAMETER;
		}
	} else {
		free(buf);
	}
err:
	EFI_CALL((*fh->close)(fh));

	return ret;
}

/**
 * efi_capsule_delete_file - delete a capsule file
 * @filename:	File name
 *
 * Delete a capsule file from capsule directory.
 *
 * Return:	status code
 */
static efi_status_t efi_capsule_delete_file(const u16 *filename)
{
	struct efi_file_handle *dirh, *fh;
	efi_status_t ret;

	ret = EFI_CALL((*bootdev_root->open)(bootdev_root, &dirh,
					     EFI_CAPSULE_DIR,
					     EFI_FILE_MODE_READ, 0));
	if (ret != EFI_SUCCESS)
		return ret;
	ret = EFI_CALL((*dirh->open)(dirh, &fh, (u16 *)filename,
				     EFI_FILE_MODE_READ, 0));
	/* ignore an error */
	EFI_CALL((*dirh->close)(dirh));

	if (ret == EFI_SUCCESS)
		ret = EFI_CALL((*fh->delete)(fh));

	return ret;
}

/**
 * efi_capsule_scan_done - reset a scan help function
 *
 * Reset a scan help function
 */
static void efi_capsule_scan_done(void)
{
	EFI_CALL((*bootdev_root->close)(bootdev_root));
	bootdev_root = NULL;
}

/**
 * check_run_capsules() - check whether capsule update should run
 *
 * The spec says OsIndications must be set in order to run the capsule update
 * on-disk.  Since U-Boot doesn't support runtime SetVariable, allow capsules to
 * run explicitly if CONFIG_EFI_IGNORE_OSINDICATIONS is selected
 *
 * Return:	EFI_SUCCESS if update to run, EFI_NOT_FOUND otherwise
 */
static efi_status_t check_run_capsules(void)
{
	u64 os_indications = 0x0;
	efi_uintn_t size;
	efi_status_t r;

	size = sizeof(os_indications);
	r = efi_get_variable_int(u"OsIndications", &efi_global_variable_guid,
				 NULL, &size, &os_indications, NULL);
	if (!IS_ENABLED(CONFIG_EFI_IGNORE_OSINDICATIONS) &&
	    (r != EFI_SUCCESS || size != sizeof(os_indications)))
		return EFI_NOT_FOUND;

	if (os_indications &
	    EFI_OS_INDICATIONS_FILE_CAPSULE_DELIVERY_SUPPORTED) {
		os_indications &=
			~EFI_OS_INDICATIONS_FILE_CAPSULE_DELIVERY_SUPPORTED;
		r = efi_set_variable_int(u"OsIndications",
					 &efi_global_variable_guid,
					 EFI_VARIABLE_NON_VOLATILE |
					 EFI_VARIABLE_BOOTSERVICE_ACCESS |
					 EFI_VARIABLE_RUNTIME_ACCESS,
					 sizeof(os_indications),
					 &os_indications, false);
		if (r != EFI_SUCCESS)
			log_err("Setting %ls failed\n", L"OsIndications");
		return EFI_SUCCESS;
	} else if (IS_ENABLED(CONFIG_EFI_IGNORE_OSINDICATIONS)) {
		return EFI_SUCCESS;
	} else {
		return EFI_NOT_FOUND;
	}
}

/**
 * efi_launch_capsule - launch capsules
 *
 * Launch all the capsules in system at boot time.
 * Called by efi init code
 *
 * Return:	status codde
 */
efi_status_t efi_launch_capsules(void)
{
	struct efi_capsule_header *capsule = NULL;
	u16 **files;
	unsigned int nfiles, index, index_max, i;
	efi_status_t ret;
	bool capsule_update = true;
	bool update_status = true;
	bool fw_accept_os = false;

	if (check_run_capsules() != EFI_SUCCESS)
		return EFI_SUCCESS;

	index_max = get_max_capsule();
	index = get_last_capsule();

	/*
	 * Find capsules on disk.
	 * All the capsules are collected at the beginning because
	 * capsule files will be removed instantly.
	 */
	nfiles = 0;
	files = NULL;
	ret = efi_capsule_scan_dir(&files, &nfiles);
	if (ret != EFI_SUCCESS)
		return ret;
	if (!nfiles)
		return EFI_SUCCESS;

	/* Launch capsules */
	for (i = 0, ++index; i < nfiles; i++, index++) {
		log_debug("Applying %ls\n", files[i]);
		if (index > index_max)
			index = 0;
		ret = efi_capsule_read_file(files[i], &capsule);
		if (ret == EFI_SUCCESS) {
			ret = efi_capsule_update_firmware(capsule);
			if (ret != EFI_SUCCESS) {
				log_err("Applying capsule %ls failed.\n",
					files[i]);
				update_status = false;
			} else {
				log_info("Applying capsule %ls succeeded.\n",
					 files[i]);
				if (IS_ENABLED(CONFIG_FWU_MULTI_BANK_UPDATE)) {
					fwu_post_update_checks(capsule,
							       &fw_accept_os,
							       &capsule_update);
				}
			}

			/* create CapsuleXXXX */
			set_capsule_result(index, capsule, ret);

			free(capsule);
		} else {
			log_err("Reading capsule %ls failed\n", files[i]);
			update_status = false;
		}
		/* delete a capsule either in case of success or failure */
		ret = efi_capsule_delete_file(files[i]);
		if (ret != EFI_SUCCESS)
			log_err("Deleting capsule %ls failed\n",
				files[i]);
	}

	efi_capsule_scan_done();

	if (IS_ENABLED(CONFIG_FWU_MULTI_BANK_UPDATE)) {
		if (capsule_update == true && update_status == true) {
			ret = fwu_post_update_process(fw_accept_os);
		} else if (capsule_update == true && update_status == false) {
			log_err("All capsules were not updated. Not updating FWU metadata\n");
		}
	}

	for (i = 0; i < nfiles; i++)
		free(files[i]);
	free(files);

	/*
	 * UEFI spec requires to reset system after complete processing capsule
	 * update on the storage.
	 */
	log_info("Reboot after firmware update.\n");
	/* Cold reset is required for loading the new firmware. */
	sysreset_walk_halt(SYSRESET_COLD);
	hang();
	/* not reach here */

	return 0;
}
#endif /* CONFIG_EFI_CAPSULE_ON_DISK */
