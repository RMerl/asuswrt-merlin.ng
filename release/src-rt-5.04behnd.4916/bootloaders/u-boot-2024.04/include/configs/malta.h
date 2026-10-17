/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2013 Gabor Juhos <juhosg@openwrt.org>
 */

#ifndef _MALTA_CONFIG_H
#define _MALTA_CONFIG_H

/*
 * System configuration
 */

/*
 * CPU Configuration
 */

/*
 * Memory map
 */

#ifdef CONFIG_64BIT
# define CFG_SYS_SDRAM_BASE		0xffffffff80000000
#else
# define CFG_SYS_SDRAM_BASE		0x80000000
#endif
#define CFG_SYS_SDRAM_SIZE		0x10000000	/* 256 MiB */

#define CFG_SYS_INIT_SP_OFFSET	0x400000

/*
 * Serial driver
 */

/*
 * Flash configuration
 */
#ifdef CONFIG_64BIT
# define CFG_SYS_FLASH_BASE		0xffffffffbe000000
#else
# define CFG_SYS_FLASH_BASE		0xbe000000
#endif

/*
 * Environment
 */

/*
 * IDE/ATA
 */

/*
 * Commands
 */

#endif /* _MALTA_CONFIG_H */
