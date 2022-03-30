/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _BCMBCA_SDK_H
#define _BCMBCA_SDK_H

#include <mmc.h>
#include "boot_flash.h"

/* volume ID assignments for UBI volumes
 * 1 - metadata_copy1
 * 2 - metadata_copy2 
 * 3 - bootfs_1
 * 4 - rootfs_1
 * 5 - bootfs_2
 * 6 - rootfs_2
 * 7-9 RESERVED  
 * 10 - data
 * 11 - defaults (it is possible that uboot will need to access data and defaults during provisioning)
 * Volume numbers > 11 would not need to be accessed by number and can be assigned at will
 */

#define METADATA_SIZE 256
#define MAX_METADATA_SIZE 2048

#define ACTIVE_IMGIDX_BOOTSTRAP	0
#define ACTIVE_IMGIDX_1	1
#define ACTIVE_IMGIDX_2	2
#define ACTIVE_IMGIDX_MAX	ACTIVE_IMGIDX_2

/* UBI volumes defines */
#define IMAGE_VOL_ID_1 3
#define IMAGE_VOL_ID_2 5
#define METADATA_VOL_ID_1 1
#define METADATA_VOL_ID_2 2

/* GPT Partitions defines */
#define IMAGE_PART_ID_1 IMAGE_VOL_ID_1
#define IMAGE_PART_ID_2 IMAGE_VOL_ID_2
#define METADATA_PART_ID_1 METADATA_VOL_ID_1 
#define METADATA_PART_ID_2 METADATA_VOL_ID_2

#define BOOT_MAGIC_MAGIC 0x75456e76

enum boot_device {
	BCA_DEV_NAND,
	BCA_DEV_SPINAND,
	BCA_DEV_MMC,
	BCA_DEV_NOR,
	BCA_DEV_NETBOOT,
	BCA_DEV_UNKNOWN = 0
};

struct bcasdk_ctx {
	enum boot_device boot_device;
	enum boot_device image_device;
	struct ubispl_info *ubispl;
	struct mmc *mmc;
	struct blk_desc *mmcboot_blk;
	int active_image;
	int last_reset_reason;
	/* FIXME -- What is the "handle" for the GPT table where we find the image partitions? */
};

/*Define the partition name for spi nor flash*/
#define LOADER_PART "loader"
/*partition name for fitimage*/
#define BOOTFS_PART "bootfs"
#define ROOTFS_PART "rootfs"
#define DATA_PART "data"

int get_img_index_for_upgrade(int flag);
int flash_upgrade_img_bundle( ulong bundle_addr , int img_index, const char * conf_name);
int commit_image ( int img_index );

int env_override_import(void *ep);

#endif


