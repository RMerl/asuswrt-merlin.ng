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
#define CFG_TEGRA_BOARD_STRING	"NVIDIA Seaboard"

/* Board-specific serial config */
#define CFG_SYS_NS16550_COM1		NV_PA_APB_UARTD_BASE

/* Environment in eMMC, at the end of 2nd "boot sector" */

/* NAND support */

/* Max number of NAND devices */

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
