/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG Arndale board.
 */

#ifndef __CONFIG_ARNDALE_H
#define __CONFIG_ARNDALE_H

#define EXYNOS_FDTFILE_SETTING \
	"fdtfile=exynos5250-arndale.dtb\0"

#include "exynos5250-common.h"
#include <configs/exynos5-common.h>

/* Miscellaneous configurable options */

#define CFG_SMP_PEN_ADDR	0x02020000

#endif	/* __CONFIG_H */
