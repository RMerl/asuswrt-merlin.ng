/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Google, Inc
 *
 * Configuration settings for generic Exynos 5 board
 */

#ifndef __CONFIG_EXYNOS5_DT_COMMON_H
#define __CONFIG_EXYNOS5_DT_COMMON_H

/* Console configuration */
#undef EXYNOS_DEVICE_SETTINGS
#define EXYNOS_DEVICE_SETTINGS \
		"stdin=serial,cros-ec-keyb\0" \
		"stdout=serial,vidconsole\0" \
		"stderr=serial,vidconsole\0"

#define CONFIG_EXYNOS5_DT

#define CONFIG_SYS_SPI_BASE	0x12D30000
#define FLASH_SIZE		(4 << 20)
#define CONFIG_ENV_OFFSET	(FLASH_SIZE - CONFIG_ENV_SECT_SIZE)
#define CONFIG_SPI_BOOTING

#define CONFIG_BOARD_COMMON

/* Display */
#ifdef CONFIG_LCD
#define CONFIG_EXYNOS_FB
#define CONFIG_EXYNOS_DP
#define LCD_BPP			LCD_COLOR16
#endif

/* Enable keyboard */
#define CONFIG_KEYBOARD

#endif
