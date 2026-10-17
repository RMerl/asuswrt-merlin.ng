/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Marek Vasut <marex@denx.de>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CFG_SYS_SDRAM_BASE		0xa0000000

#define CFG_SYS_INIT_RAM_ADDR	0xbd000000
#define CFG_SYS_INIT_RAM_SIZE	0x8000

/*
 * Serial Port
 */
#define CFG_SYS_NS16550_CLK		40000000

/*
 * Command
 */
/* Miscellaneous configurable options */

/*
 * Diagnostics
 */

#endif  /* __CONFIG_H */
