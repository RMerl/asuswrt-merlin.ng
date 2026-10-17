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
#define CFG_TEGRA_BOARD_STRING	"NVIDIA Harmony"

/* Board-specific serial config */

/* UARTD: keyboard satellite board UART, default */
#define CFG_SYS_NS16550_COM1		NV_PA_APB_UARTD_BASE
#ifdef CONFIG_TEGRA_ENABLE_UARTA
/* UARTA: debug board UART */
#define CFG_SYS_NS16550_COM2		NV_PA_APB_UARTA_BASE
#endif

/* NAND support */

/* Environment in NAND (which is 512M), aligned to start of last sector */

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
