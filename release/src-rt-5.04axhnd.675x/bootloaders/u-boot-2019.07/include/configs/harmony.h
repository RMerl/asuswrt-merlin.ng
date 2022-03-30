/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  (C) Copyright 2010-2012
 *  NVIDIA Corporation <www.nvidia.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>
#include "tegra20-common.h"

/* High-level configuration options */
#define CONFIG_TEGRA_BOARD_STRING	"NVIDIA Harmony"

/* Board-specific serial config */
#define CONFIG_TEGRA_ENABLE_UARTD

/* UARTD: keyboard satellite board UART, default */
#define CONFIG_SYS_NS16550_COM1		NV_PA_APB_UARTD_BASE
#ifdef CONFIG_TEGRA_ENABLE_UARTA
/* UARTA: debug board UART */
#define CONFIG_SYS_NS16550_COM2		NV_PA_APB_UARTA_BASE
#endif

#define CONFIG_MACH_TYPE		MACH_TYPE_HARMONY

/* NAND support */
#define CONFIG_TEGRA_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1

/* Environment in NAND (which is 512M), aligned to start of last sector */
#define CONFIG_ENV_OFFSET	(SZ_512M - SZ_128K) /* 128K sector size */

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
