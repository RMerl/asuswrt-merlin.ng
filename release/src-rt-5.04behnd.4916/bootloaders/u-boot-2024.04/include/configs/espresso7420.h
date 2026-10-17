/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the SAMSUNG ESPRESSO7420 board.
 * Copyright (C) 2016 Samsung Electronics
 * Thomas Abraham <thomas.ab@samsung.com>
 */

#ifndef __CONFIG_ESPRESSO7420_H
#define __CONFIG_ESPRESSO7420_H

#include <configs/exynos7420-common.h>

#define CFG_SYS_SDRAM_BASE		0x40000000

/* DRAM Memory Banks */
#define SDRAM_BANK_SIZE		(256UL << 20UL)	/* 256 MB */

#endif	/* __CONFIG_ESPRESSO7420_H */
