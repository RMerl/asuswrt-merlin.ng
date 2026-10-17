// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2016
 * Ladislav Michl <ladis@linux-mips.org>
 */

#include <common.h>
#include <config.h>
#include <image.h>
#include <nand.h>
#include <onenand_uboot.h>
#include <ubispl.h>
#include <spl.h>

int spl_ubi_load_image(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev)
{
	struct legacy_img_hdr *header;
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
		info.peb_size = CFG_SYS_ONENAND_BLOCK_SIZE;
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

#if CONFIG_IS_ENABLED(OS_BOOT)
	if (!spl_start_uboot()) {
		volumes[0].vol_id = CONFIG_SPL_UBI_LOAD_KERNEL_ID;
		volumes[0].load_addr = (void *)CONFIG_SYS_LOAD_ADDR;
		volumes[1].vol_id = CONFIG_SPL_UBI_LOAD_ARGS_ID;
		volumes[1].load_addr = (void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR;

		ret = ubispl_load_volumes(&info, volumes, 2);
		if (!ret) {
			header = (struct legacy_img_hdr *)volumes[0].load_addr;
			spl_parse_image_header(spl_image, bootdev, header);
			puts("Linux loaded.\n");
			goto out;
		}
		puts("Loading Linux failed, falling back to U-Boot.\n");
	}
#endif
	header = spl_get_load_buffer(-sizeof(*header), sizeof(header));
#ifdef CONFIG_SPL_UBI_LOAD_BY_VOLNAME
	volumes[0].vol_id = -1;
	strncpy(volumes[0].name,
		CONFIG_SPL_UBI_LOAD_MONITOR_VOLNAME,
		UBI_VOL_NAME_MAX + 1);
#else
	volumes[0].vol_id = CONFIG_SPL_UBI_LOAD_MONITOR_ID;
#endif
	volumes[0].load_addr = (void *)header;

	ret = ubispl_load_volumes(&info, volumes, 1);
	if (!ret)
		spl_parse_image_header(spl_image, bootdev, header);
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
