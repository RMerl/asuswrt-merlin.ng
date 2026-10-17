/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013-2014
 * NVIDIA Corporation <www.nvidia.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

#include "tegra124-common.h"

/* High-level configuration options */
#define CFG_TEGRA_BOARD_STRING	"NVIDIA Venice2"

/* Board-specific serial config */
#define CFG_SYS_NS16550_COM1		NV_PA_APB_UARTA_BASE

/* Environment in eMMC, at the end of 2nd "boot sector" */

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
