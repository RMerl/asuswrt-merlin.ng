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

#define CFG_SYS_SPI_BASE	0x12D30000
#define FLASH_SIZE		(4 << 20)

#endif
