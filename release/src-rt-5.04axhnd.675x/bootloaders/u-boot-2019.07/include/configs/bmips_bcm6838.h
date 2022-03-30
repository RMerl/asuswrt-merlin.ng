/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Philippe Reynes <philippe.reynes@softathome.com>
 */

#ifndef __CONFIG_BMIPS_BCM6838_H
#define __CONFIG_BMIPS_BCM6838_H

/* CPU */
#define CONFIG_SYS_MIPS_TIMER_FREQ	160000000

/* RAM */
#define CONFIG_SYS_SDRAM_BASE		0x80000000

/* U-Boot */
#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SDRAM_BASE + 0x100000

#if defined(CONFIG_BMIPS_BOOT_RAM)
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SYS_INIT_SP_OFFSET	0x2000
#endif

#endif /* __CONFIG_BMIPS_BCM6838_H */
