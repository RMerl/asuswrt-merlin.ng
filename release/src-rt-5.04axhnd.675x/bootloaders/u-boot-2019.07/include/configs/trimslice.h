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
#define CONFIG_TEGRA_BOARD_STRING	"Compulab Trimslice"

/* Board-specific serial config */
#define CONFIG_TEGRA_ENABLE_UARTA
#define CONFIG_TEGRA_UARTA_GPU
#define CONFIG_SYS_NS16550_COM1		NV_PA_APB_UARTA_BASE

#define CONFIG_MACH_TYPE		MACH_TYPE_TRIMSLICE

/* SPI */

/* Environment in SPI */
#define CONFIG_ENV_SECT_SIZE		CONFIG_ENV_SIZE
/* 1MiB flash, environment located as high as possible */
#define CONFIG_ENV_OFFSET		(SZ_1M - CONFIG_ENV_SIZE)

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
