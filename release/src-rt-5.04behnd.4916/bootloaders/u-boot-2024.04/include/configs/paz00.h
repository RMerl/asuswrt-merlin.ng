/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010-2012 NVIDIA CORPORATION.  All rights reserved.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>
#include "tegra20-common.h"

/* High-level configuration options */
#define CFG_TEGRA_BOARD_STRING	"Compal Paz00"

/* Board-specific serial config */
#define CFG_SYS_NS16550_COM1		NV_PA_APB_UARTA_BASE

/* Environment in eMMC, at the end of 2nd "boot sector" */

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
