// SPDX-License-Identifier: GPL-2.0+
/*
 *  Root node for system services
 *
 *  Copyright (c) 2018 Heinrich Schuchardt
 */

#include <malloc.h>
#include <efi_dt_fixup.h>
#include <efi_loader.h>

const efi_guid_t efi_u_boot_guid = U_BOOT_GUID;

efi_handle_t efi_root = NULL;

struct efi_root_dp {
	struct efi_device_path_vendor vendor;
	struct efi_device_path end;
} __packed;

/**
 * efi_root_node_register() - create root node
 *
 * Create the root node on which we install all protocols that are
 * not related to a loaded image or a driver.
 *
 * Return:	status code
 */
efi_status_t efi_root_node_register(void)
{
	efi_status_t ret;
	struct efi_root_dp *dp;

	/* Create device path protocol */
	dp = calloc(1, sizeof(*dp));
	if (!dp)
		return EFI_OUT_OF_RESOURCES;

	/* Fill vendor node */
	dp->vendor.dp.type = DEVICE_PATH_TYPE_HARDWARE_DEVICE;
	dp->vendor.dp.sub_type = DEVICE_PATH_SUB_TYPE_VENDOR;
	dp->vendor.dp.length = sizeof(struct efi_device_path_vendor);
	dp->vendor.guid = efi_u_boot_guid;

	/* Fill end node */
	dp->end.type = DEVICE_PATH_TYPE_END;
	dp->end.sub_type = DEVICE_PATH_SUB_TYPE_END;
	dp->end.length = sizeof(struct efi_device_path);

	/* Create root node and install protocols */
	ret = efi_install_multiple_protocol_interfaces
		(&efi_root,
		 /* Device path protocol */
		 &efi_guid_device_path, dp,
#if CONFIG_IS_ENABLED(EFI_DEVICE_PATH_TO_TEXT)
		 /* Device path to text protocol */
		 &efi_guid_device_path_to_text_protocol,
		 &efi_device_path_to_text,
#endif
#if IS_ENABLED(CONFIG_EFI_DEVICE_PATH_UTIL)
		 /* Device path utilities protocol */
		 &efi_guid_device_path_utilities_protocol,
		 &efi_device_path_utilities,
#endif
#if CONFIG_IS_ENABLED(EFI_DT_FIXUP)
		 /* Device-tree fix-up protocol */
		 &efi_guid_dt_fixup_protocol,
		 &efi_dt_fixup_prot,
#endif
#if IS_ENABLED(CONFIG_EFI_UNICODE_COLLATION_PROTOCOL2)
		 &efi_guid_unicode_collation_protocol2,
		 &efi_unicode_collation_protocol2,
#endif
#if IS_ENABLED(CONFIG_EFI_LOADER_HII)
		 /* HII string protocol */
		 &efi_guid_hii_string_protocol,
		 &efi_hii_string,
		 /* HII database protocol */
		 &efi_guid_hii_database_protocol,
		 &efi_hii_database,
#endif
		 NULL);
	efi_root->type = EFI_OBJECT_TYPE_U_BOOT_FIRMWARE;
	return ret;
}
