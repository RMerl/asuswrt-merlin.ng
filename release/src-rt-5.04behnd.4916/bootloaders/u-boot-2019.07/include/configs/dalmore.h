/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010-2013, NVIDIA CORPORATION.  All rights reserved.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

#include "tegra114-common.h"

/* High-level configuration options */
#define CONFIG_TEGRA_BOARD_STRING	"NVIDIA Dalmore"

/* Board-specific serial config */
#define CONFIG_TEGRA_ENABLE_UARTD
#define CONFIG_SYS_NS16550_COM1		NV_PA_APB_UARTD_BASE

#define CONFIG_MACH_TYPE		MACH_TYPE_DALMORE

/* Environment in eMMC, at the end of 2nd "boot sector" */
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_SYS_MMC_ENV_PART		2
#define CONFIG_ENV_OFFSET		(-CONFIG_ENV_SIZE)

/* SPI */
#define CONFIG_SPI_FLASH_SIZE          (4 << 20)

#include "tegra-common-usb-gadget.h"
#include "tegra-common-post.h"

#endif /* __CONFIG_H */
