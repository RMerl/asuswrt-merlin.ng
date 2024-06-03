/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG SMDK5420 board.
 */

#ifndef __CONFIG_SMDK5420_H
#define __CONFIG_SMDK5420_H

#include <configs/exynos5420-common.h>
#include <configs/exynos5-dt-common.h>
#include <configs/exynos5-common.h>

#undef CONFIG_EXYNOS_FB
#undef CONFIG_EXYNOS_DP

#undef CONFIG_KEYBOARD

#define CONFIG_BOARD_COMMON

#define CONFIG_SMDK5420			/* which is in a SMDK5420 */

#define CONFIG_SYS_SDRAM_BASE	0x20000000
#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_IRAM_TOP - 0x800)

/* select serial console configuration */
#define CONFIG_DEFAULT_CONSOLE	"console=ttySAC1,115200n8\0"

#define CONFIG_DEFAULT_CONSOLE		"console=ttySAC1,115200n8\0"

/* USB */
#define CONFIG_USB_XHCI_EXYNOS

/* DRAM Memory Banks */
#define SDRAM_BANK_SIZE		(512UL << 20UL)	/* 512 MB */

#endif	/* __CONFIG_SMDK5420_H */
