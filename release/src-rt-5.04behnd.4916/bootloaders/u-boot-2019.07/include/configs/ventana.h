/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>
#include "tegra20-common.h"

/* High-level configuration options */
#define CONFIG_TEGRA_BOARD_STRING	"NVIDIA Ventana"

/* Board-specific serial config */
#define CONFIG_TEGRA_ENABLE_UARTD
#define CONFIG_SYS_NS16550_COM1		NV_PA_APB_UARTD_BASE

#define CONFIG_MACH_TYPE		MACH_TYPE_VENTANA

/* Environment in eMMC, at the end of 2nd "boot sector" */
#define CONFIG_ENV_OFFSET (-CONFIG_ENV_SIZE)
#define CONFIG_SYS_MMC_ENV_DEV 0
#define CONFIG_SYS_MMC_ENV_PART 2

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
