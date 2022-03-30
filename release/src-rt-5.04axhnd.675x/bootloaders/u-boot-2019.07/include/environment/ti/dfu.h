/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com
 *
 * Environment variable definitions for DFU on TI boards.
 */

#ifndef __TI_DFU_H
#define __TI_DFU_H

#define DFU_ALT_INFO_MMC \
	"dfu_alt_info_mmc=" \
	"boot part 0 1;" \
	"rootfs part 0 2;" \
	"MLO fat 0 1;" \
	"MLO.raw raw 0x100 0x100;" \
	"u-boot.img.raw raw 0x300 0x1000;" \
	"u-env.raw raw 0x1300 0x200;" \
	"spl-os-args.raw raw 0x1500 0x200;" \
	"spl-os-image.raw raw 0x1700 0x6900;" \
	"spl-os-args fat 0 1;" \
	"spl-os-image fat 0 1;" \
	"u-boot.img fat 0 1;" \
	"uEnv.txt fat 0 1\0"

#define DFU_ALT_INFO_EMMC \
	"dfu_alt_info_emmc=" \
	"rawemmc raw 0 3751936;" \
	"boot part 1 1;" \
	"rootfs part 1 2;" \
	"MLO fat 1 1;" \
	"MLO.raw raw 0x100 0x100;" \
	"u-boot.img.raw raw 0x300 0x1000;" \
	"u-env.raw raw 0x1300 0x200;" \
	"spl-os-args.raw raw 0x1500 0x200;" \
	"spl-os-image.raw raw 0x1700 0x6900;" \
	"spl-os-args fat 1 1;" \
	"spl-os-image fat 1 1;" \
	"u-boot.img fat 1 1;" \
	"uEnv.txt fat 1 1\0"

#ifdef CONFIG_NAND
#define DFU_ALT_INFO_NAND \
	"dfu_alt_info_nand=" \
	"SPL part 0 1;" \
	"SPL.backup1 part 0 2;" \
	"SPL.backup2 part 0 3;" \
	"SPL.backup3 part 0 4;" \
	"u-boot part 0 5;" \
	"u-boot-spl-os part 0 6;" \
	"kernel part 0 8;" \
	"rootfs part 0 9\0"
#else
#define DFU_ALT_INFO_NAND ""
#endif

#define DFU_ALT_INFO_RAM \
	"dfu_alt_info_ram=" \
	"kernel ram 0x80200000 0x4000000;" \
	"fdt ram 0x80f80000 0x80000;" \
	"ramdisk ram 0x81000000 0x4000000\0"

#define DFU_ALT_INFO_QSPI_XIP \
	"dfu_alt_info_qspi=" \
	"u-boot.bin raw 0x0 0x080000;" \
	"u-boot.backup raw 0x080000 0x080000;" \
	"u-boot-spl-os raw 0x100000 0x010000;" \
	"u-boot-env raw 0x110000 0x010000;" \
	"u-boot-env.backup raw 0x120000 0x010000;" \
	"kernel raw 0x130000 0x800000\0"

#define DFU_ALT_INFO_QSPI \
	"dfu_alt_info_qspi=" \
	"MLO raw 0x0 0x040000;" \
	"u-boot.img raw 0x040000 0x0100000;" \
	"u-boot-spl-os raw 0x140000 0x080000;" \
	"u-boot-env raw 0x1C0000 0x010000;" \
	"u-boot-env.backup raw 0x1D0000 0x010000;" \
	"kernel raw 0x1E0000 0x800000\0"

#endif /* __TI_DFU_H */
