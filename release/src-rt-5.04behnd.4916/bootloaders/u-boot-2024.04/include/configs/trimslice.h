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
#define CFG_TEGRA_BOARD_STRING	"Compulab Trimslice"

/* Board-specific serial config */
#define CFG_SYS_NS16550_COM1		NV_PA_APB_UARTA_BASE

/* SPI */

/* Environment in SPI */
/* 1MiB flash, environment located as high as possible */

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
