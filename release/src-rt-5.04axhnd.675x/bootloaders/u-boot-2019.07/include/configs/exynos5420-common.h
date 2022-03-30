/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG EXYNOS5420 SoC
 */

#ifndef __CONFIG_EXYNOS5420_H
#define __CONFIG_EXYNOS5420_H

#define CONFIG_EXYNOS5420

#define CONFIG_EXYNOS5_DT

/* Provide the MACH_TYPE value that the vendor kernel requires. */
#define CONFIG_MACH_TYPE		8002

#define CONFIG_VAR_SIZE_SPL

#define CONFIG_IRAM_TOP			0x02074000

#define CONFIG_SPL_MAX_FOOTPRINT	(30 * 1024)

#define CONFIG_DEVICE_TREE_LIST "exynos5800-peach-pi"	\
				"exynos5420-peach-pit exynos5420-smdk5420"

#define CONFIG_PHY_IRAM_BASE		0x02020000

/* Address for relocating helper code (Last 4 KB of IRAM) */
#define CONFIG_EXYNOS_RELOCATE_CODE_BASE	(CONFIG_IRAM_TOP - 0x1000)

/*
 * Low Power settings
 */
#define CONFIG_LOWPOWER_FLAG		0x02020028
#define CONFIG_LOWPOWER_ADDR		0x0202002C

#define CONFIG_USB_XHCI_EXYNOS

#endif	/* __CONFIG_EXYNOS5420_H */
