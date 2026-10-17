/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Configuration for MediaTek MT8518 SoC
 *
 * Copyright (C) 2019 MediaTek Inc.
 * Author: Mingming Lee <mingming.lee@mediatek.com>
 */

#ifndef __MT8518_H
#define __MT8518_H

/* DRAM definition */
#define CFG_SYS_SDRAM_BASE			0x40000000
#define CFG_SYS_SDRAM_SIZE			0x20000000

/* Uboot definition */

#define ENV_BOOT_READ_IMAGE \
	"boot_rd_img=mmc dev 0" \
	";mmc read ${loadaddr} 0x27400 0x4000" \
	";iminfo ${loadaddr}\0"

/* Console configuration */
#define ENV_DEVICE_SETTINGS \
	"stdin=serial\0" \
	"stdout=serial\0" \
	"stderr=serial\0"

#define ENV_BOOT_CMD \
	"mtk_boot=run boot_rd_img;bootm;\0"

#define ENV_FASTBOOT \
	"serial#=1234567890ABCDEF\0" \
	"board=mt8518\0"

#define CFG_EXTRA_ENV_SETTINGS \
	"fdt_high=0x6c000000\0" \
	ENV_DEVICE_SETTINGS \
	ENV_BOOT_READ_IMAGE \
	ENV_FASTBOOT \
	ENV_BOOT_CMD \
	"bootcmd=run mtk_boot;\0" \

#endif
