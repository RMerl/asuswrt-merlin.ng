// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * Derived work from spl_usb.c
 */

#include <common.h>
#include <spl.h>
#include <asm/u-boot.h>
#include <sata.h>
#include <scsi.h>
#include <errno.h>
#include <fat.h>
#include <image.h>

static int spl_sata_load_image(struct spl_image_info *spl_image,
			       struct spl_boot_device *bootdev)
{
	int err;
	struct blk_desc *stor_dev;

	err = init_sata(CONFIG_SPL_SATA_BOOT_DEVICE);
	if (err) {
#ifdef CONFIG_SPL_LIBCOMMON_SUPPORT
		printf("spl: sata init failed: err - %d\n", err);
#endif
		return err;
	} else {
		/* try to recognize storage devices immediately */
		scsi_scan(false);
		stor_dev = blk_get_devnum_by_type(IF_TYPE_SCSI, 0);
		if (!stor_dev)
			return -ENODEV;
	}

#ifdef CONFIG_SPL_OS_BOOT
	if (spl_start_uboot() ||
	    spl_load_image_fat_os(spl_image, stor_dev,
				  CONFIG_SYS_SATA_FAT_BOOT_PARTITION))
#endif
	{
		err = spl_load_image_fat(spl_image, stor_dev,
					CONFIG_SYS_SATA_FAT_BOOT_PARTITION,
				CONFIG_SPL_FS_LOAD_PAYLOAD_NAME);
	}
	if (err) {
		puts("Error loading sata device\n");
		return err;
	}

	return 0;
}
SPL_LOAD_IMAGE_METHOD("SATA", 0, BOOT_DEVICE_SATA, spl_sata_load_image);
