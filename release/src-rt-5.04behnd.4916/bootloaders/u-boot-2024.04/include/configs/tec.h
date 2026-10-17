/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 *  (C) Copyright 2011-2012
 *  Avionic Design GmbH <www.avionic-design.de>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "tegra20-common.h"

/* High-level configuration options */
#define CFG_TEGRA_BOARD_STRING	"Avionic Design Tamonten Evaluation Carrier"

/* Board-specific serial config */
#define CFG_SYS_NS16550_COM1		NV_PA_APB_UARTD_BASE

/* NAND support */

/* Environment in NAND, aligned to start of last sector */

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
