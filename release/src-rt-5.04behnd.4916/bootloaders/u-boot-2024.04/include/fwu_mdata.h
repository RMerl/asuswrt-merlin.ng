/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2022, Linaro Limited
 */

#if !defined _FWU_MDATA_H_
#define _FWU_MDATA_H_

#include <linux/compiler_attributes.h>
#include <efi.h>

/**
 * struct fwu_image_bank_info - firmware image information
 * @image_uuid: Guid value of the image in this bank
 * @accepted: Acceptance status of the image
 * @reserved: Reserved
 *
 * The structure contains image specific fields which are
 * used to identify the image and to specify the image's
 * acceptance status
 */
struct fwu_image_bank_info {
	efi_guid_t  image_uuid;
	uint32_t accepted;
	uint32_t reserved;
} __packed;

/**
 * struct fwu_image_entry - information for a particular type of image
 * @image_type_uuid: Guid value for identifying the image type
 * @location_uuid: Guid of the storage volume where the image is located
 * @img_bank_info: Array containing properties of images
 *
 * This structure contains information on various types of updatable
 * firmware images. Each image type then contains an array of image
 * information per bank.
 */
struct fwu_image_entry {
	efi_guid_t image_type_uuid;
	efi_guid_t location_uuid;
	struct fwu_image_bank_info img_bank_info[CONFIG_FWU_NUM_BANKS];
} __packed;

/**
 * struct fwu_mdata - FWU metadata structure for multi-bank updates
 * @crc32: crc32 value for the FWU metadata
 * @version: FWU metadata version
 * @active_index: Index of the bank currently used for booting images
 * @previous_active_inde: Index of the bank used before the current bank
 *                        being used for booting
 * @img_entry: Array of information on various firmware images that can
 *             be updated
 *
 * This structure is used to store all the needed information for performing
 * multi bank updates on the platform. This contains info on the bank being
 * used to boot along with the information needed for identification of
 * individual images
 */
struct fwu_mdata {
	uint32_t crc32;
	uint32_t version;
	uint32_t active_index;
	uint32_t previous_active_index;

	struct fwu_image_entry img_entry[CONFIG_FWU_NUM_IMAGES_PER_BANK];
} __packed;

#endif /* _FWU_MDATA_H_ */
