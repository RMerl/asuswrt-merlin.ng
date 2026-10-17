/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011-2014 Pierrick Hascoet, Abilis Systems
 */

#ifndef _CONFIG_TB100_H_
#define _CONFIG_TB100_H_

#include <linux/sizes.h>

/*
 * Memory configuration
 */

#define CFG_SYS_DDR_SDRAM_BASE	0x80000000
#define CFG_SYS_SDRAM_BASE		CFG_SYS_DDR_SDRAM_BASE
#define CFG_SYS_SDRAM_SIZE		SZ_128M

/*
 * UART configuration
 */
#define CFG_SYS_NS16550_CLK		166666666

#endif /* _CONFIG_TB100_H_ */
