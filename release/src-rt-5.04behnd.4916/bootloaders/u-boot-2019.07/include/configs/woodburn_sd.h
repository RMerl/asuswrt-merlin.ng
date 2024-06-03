/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011, Stefano Babic <sbabic@denx.de>
 *
 * (C) Copyright 2008-2010 Freescale Semiconductor, Inc.
 *
 * Configuration for the woodburn board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/imx-regs.h>
#include "woodburn_common.h"

/* Set TEXT in RAM */

/*
 * SPL
 */

#define CONFIG_SPL_MAX_SIZE		(64 * 1024)	/* 8 KB for stack */
#define CONFIG_SPL_STACK		LOW_LEVEL_SRAM_STACK

#define CONFIG_SYS_SPL_MALLOC_START	0x8f000000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x80000
#define CONFIG_SPL_BSS_START_ADDR	0x8f080000 /* end of RAM */
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000

#endif				/* __CONFIG_H */
