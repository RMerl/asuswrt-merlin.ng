/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) Stefano Babic <sbabic@denx.de>
 */

#ifndef __PCM058_CONFIG_H
#define __PCM058_CONFIG_H

#include "mx6_common.h"

#define PHYS_SDRAM_SIZE		(1u * 1024 * 1024 * 1024)

/* Enable NAND support */

/* Physical Memory Map */
#define PHYS_SDRAM                     MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE          PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE       IRAM_SIZE

/* Environment organization */
#define ENV_MMC \
	"mmcdev=0\0" \
	"mmcpart=2\0" \
	"fitpart=1\0" \
	"mmcrootfstype=ext4\0" \
	"fitname=fitImage\0" \
	"mmcloadfit=load mmc ${mmcdev}:${fitpart} ${loadaddr} ${fitname}\0" \
	"mmcargs=setenv bootargs root=/dev/mmcblk${mmcdev}p${mmcpart} " \
		"rootfstype=${mmcrootfstype} ${optargs}\0" \
	"mmcboot=run mmcloadfit;run mmcargs;bootm ${loadaddr}\0"

#define ENV_NAND \
	"nandroot=ubi0:root ubi.mtd=rootfs\0" \
	"nandrootfstype=ubifs\0" \
	"nandargs=setenv bootargs root=${nandroot} " \
		"rootfstype=${nandrootfstype} ${mtdparts} ${optargs}\0" \
	"nandloadfit=ubi part rootfs;ubi readvol ${loadaddr} fit\0" \
	"nandboot=run nandloadfit;run nandargs;bootm ${loadaddr}\0"

#define CFG_EXTRA_ENV_SETTINGS \
	"bootm_size=0x30000000\0" \
	"optargs=rw rootwait\0" \
	ENV_MMC \
	ENV_NAND
#endif
