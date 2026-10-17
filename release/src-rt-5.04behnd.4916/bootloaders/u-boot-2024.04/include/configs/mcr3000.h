/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2010-2017 CS Systemes d'Information
 * Christophe Leroy <christophe.leroy@c-s.fr>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */

/* Miscellaneous configurable options */

/* Definitions for initial stack pointer and data area (in DPRAM) */
#define CFG_SYS_INIT_RAM_ADDR	(CONFIG_SYS_IMMR + 0x2800)
#define	CFG_SYS_INIT_RAM_SIZE	(0x2e00 - 0x2800)
#define CFG_SYS_INIT_SP		(CONFIG_SYS_IMMR + 0x3c00)

/* RAM configuration (note that CFG_SYS_SDRAM_BASE must be zero) */
#define	CFG_SYS_SDRAM_BASE		0x00000000

/* FLASH organization */
#define CFG_SYS_FLASH_BASE		CONFIG_TEXT_BASE

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 32 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define	CFG_SYS_BOOTMAPSZ		(32 << 20)

/* Environment Configuration */

/* environment is in FLASH */

/* Ethernet configuration part */

/* NAND configuration part */
#define CFG_SYS_NAND_BASE		0x0C000000

#endif /* __CONFIG_H */
