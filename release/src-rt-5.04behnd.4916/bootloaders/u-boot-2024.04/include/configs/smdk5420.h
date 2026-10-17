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

#define CFG_SYS_SDRAM_BASE	0x20000000

/* DRAM Memory Banks */
#define SDRAM_BANK_SIZE		(512UL << 20UL)	/* 512 MB */

#endif	/* __CONFIG_SMDK5420_H */
