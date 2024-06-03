/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010-2012, NVIDIA CORPORATION.  All rights reserved.
 */

#ifndef _TEGRA30_H_
#define _TEGRA30_H_

#define NV_PA_MC_BASE		0x7000F000
#define NV_PA_SDRAM_BASE	0x80000000	/* 0x80000000 for real T30 */

#include <asm/arch-tegra/tegra.h>

#define TEGRA_USB1_BASE		0x7D000000

#define BCT_ODMDATA_OFFSET	6116	/* 12 bytes from end of BCT */

#define MAX_NUM_CPU		4

#endif	/* TEGRA30_H */
