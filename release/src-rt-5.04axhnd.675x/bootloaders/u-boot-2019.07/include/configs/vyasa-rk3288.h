/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Amarula Solutions
 *
 * Configuration settings for Amarula Vyasa RK3288.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define ROCKCHIP_DEVICE_SETTINGS
#include <configs/rk3288_common.h>

#undef BOOT_TARGET_DEVICES

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \

#define CONFIG_SYS_MMC_ENV_DEV 1
#undef CONFIG_CMD_USB_MASS_STORAGE

#ifndef CONFIG_TPL_BUILD

#define CONFIG_SPL_OS_BOOT

/* Falcon Mode */
#define CONFIG_SPL_FS_LOAD_ARGS_NAME	"args"
#define CONFIG_SPL_FS_LOAD_KERNEL_NAME	"uImage"
#define CONFIG_CMD_SPL
#define CONFIG_SYS_SPL_ARGS_ADDR	0x0ffe5000
#define CONFIG_CMD_SPL_WRITE_SIZE      (128 * SZ_1K)

/* Falcon Mode - MMC support: args@16MB kernel@17MB */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR		0x8000	/* 16MB */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS		(CONFIG_CMD_SPL_WRITE_SIZE / 512)
#define CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR		0x8800	/* 17MB */
#endif

#endif
