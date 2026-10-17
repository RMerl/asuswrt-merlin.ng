/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2010-2011 Calxeda, Inc.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CFG_SYS_BOOTMAPSZ		(16 << 20)

#define CFG_PL011_CLOCK		150000000

/*
 * Miscellaneous configurable options
 */

#define CFG_SYS_SDRAM_BASE		0x00000000

#define CFG_EXTRA_ENV_SETTINGS				\
	"fdt_high=0x20000000\0"					\
	"initrd_high=0x20000000\0"

#endif
