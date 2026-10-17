/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016 Imagination Technologies
 */

#ifndef __CONFIGS_BOSTON_H__
#define __CONFIGS_BOSTON_H__

/*
 * General board configuration
 */

/*
 * CPU
 */

/*
 * PCI
 */

/*
 * Memory map
 */
#ifdef CONFIG_64BIT
# define CFG_SYS_SDRAM_BASE		0xffffffff80000000
#else
# define CFG_SYS_SDRAM_BASE		0x80000000
#endif

#define CFG_SYS_INIT_SP_OFFSET	0x400000

/*
 * Console
 */

/*
 * Environment
 */

#endif /* __CONFIGS_BOSTON_H__ */
