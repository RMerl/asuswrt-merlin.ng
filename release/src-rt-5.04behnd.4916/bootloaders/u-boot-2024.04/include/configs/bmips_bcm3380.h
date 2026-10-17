/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 */

#ifndef __CONFIG_BMIPS_BCM3380_H
#define __CONFIG_BMIPS_BCM3380_H

#include <linux/sizes.h>

/* RAM */
#define CFG_SYS_SDRAM_BASE		0x80000000

/* U-Boot */

#if defined(CONFIG_BMIPS_BOOT_RAM)
#define CFG_SYS_INIT_SP_OFFSET	SZ_8K
#endif

#endif /* __CONFIG_BMIPS_BCM3380_H */
