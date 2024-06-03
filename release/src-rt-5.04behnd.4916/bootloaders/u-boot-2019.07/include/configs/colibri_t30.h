/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013-2016 Stefan Agner
 *
 * Configuration settings for the Toradex Colibri T30 modules.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

#include "tegra30-common.h"

/* High-level configuration options */

/* Board-specific serial config */
#define CONFIG_TEGRA_ENABLE_UARTA
#define CONFIG_SYS_NS16550_COM1		NV_PA_APB_UARTA_BASE

#define CONFIG_MACH_TYPE		MACH_TYPE_COLIBRI_T30

/* Environment in eMMC, before config block at the end of 1st "boot sector" */
#define CONFIG_ENV_OFFSET		(-CONFIG_ENV_SIZE + \
					 CONFIG_TDX_CFG_BLOCK_OFFSET)
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_SYS_MMC_ENV_PART		1

/* General networking support */
#define CONFIG_TFTP_TSIZE

/* Increase console I/O buffer size */
#undef CONFIG_SYS_CBSIZE
#define CONFIG_SYS_CBSIZE		1024

/* Increase arguments buffer size */
#undef CONFIG_SYS_BARGSIZE
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE

/* Increase maximum number of arguments */
#undef CONFIG_SYS_MAXARGS
#define CONFIG_SYS_MAXARGS		32

#include "tegra-common-usb-gadget.h"
#include "tegra-common-post.h"

#endif /* __CONFIG_H */
