// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2016
 * Ladislav Michl <ladis@linux-mips.org>
 */

#include <common.h>
#include <config.h>
#include <nand.h>
#include <onenand_uboot.h>
#include <ubispl.h>
#include <spl.h>

int spl_ubi_load_image(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev)
{
	struct image_header *header;
	struct ubispl_info info;
	struct ubispl_load volumes[2];
	int ret = 1;

	switch (bootdev->boot_device) {
#ifdef CONFIG_SPL_NAND_SUPPORT
	case BOOT_DEVICE_NAND:
		nand_init();
		info.read = nand_spl_read_block;
		info.peb_size = CONFIG_SYS_NAND_BLOCK_SIZE;
		break;
#endif
#ifdef CONFIG_SPL_ONENAND_SUPPORT
	case BOOT_DEVICE_ONENAND:
		info.read = onenand_spl_read_block;
		info.peb_size = CONFIG_SYS_ONENAND_BLOCK_SIZE;
		break;
#endif
	default:
		goto out;
	}
	info.ubi = (struct ubi_scan_info *)CONFIG_SPL_UBI_INFO_ADDR;
	info.fastmap = IS_ENABLED(CONFIG_MTD_UBI_FASTMAP);

	info.peb_offset = CONFIG_SPL_UBI_PEB_OFFSET;
	info.vid_offset = CONFIG_SPL_UBI_VID_OFFSET;
	info.leb_start = CONFIG_SPL_UBI_LEB_START;
	info.peb_count = CONFIG_SPL_UBI_MAX_PEBS - info.peb_offset;

#ifdef CONFIG_SPL_OS_BOOT
	if (!spl_start_uboot()) {
		volumes[0].vol_id = CONFIG_SPL_UBI_LOAD_KERNEL_ID;
		volumes[0].load_addr = (void *)CONFIG_SYS_LOAD_ADDR;
		volumes[1].vol_id = CONFIG_SPL_UBI_LOAD_ARGS_ID;
		volumes[1].load_addr = (void *)CONFIG_SYS_SPL_ARGS_ADDR;

		ret = ubispl_load_volumes(&info, volumes, 2);
		if (!ret) {
			header = (struct image_header *)volumes[0].load_addr;
			spl_parse_image_header(spl_image, header);
			puts("Linux loaded.\n");
			goto out;
		}
		puts("Loading Linux failed, falling back to U-Boot.\n");
	}
#endif
	header = spl_get_load_buffer(-sizeof(*header), sizeof(header));
	volumes[0].vol_id = CONFIG_SPL_UBI_LOAD_MONITOR_ID;
	volumes[0].load_addr = (void *)header;

	ret = ubispl_load_volumes(&info, volumes, 1);
	if (!ret)
		spl_parse_image_header(spl_image, header);
out:
#ifdef CONFIG_SPL_NAND_SUPPORT
	if (bootdev->boot_device == BOOT_DEVICE_NAND)
		nand_deselect();
#endif
	return ret;
}
/* Use priorty 0 so that Ubi will override NAND and ONENAND methods */
SPL_LOAD_IMAGE_METHOD("NAND", 0, BOOT_DEVICE_NAND, spl_ubi_load_image);
SPL_LOAD_IMAGE_METHOD("OneNAND", 0, BOOT_DEVICE_ONENAND, spl_ubi_load_image);
