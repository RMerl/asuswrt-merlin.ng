/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CFG_MALLOC_F_ADDR		0x0010000

/* Size of our emulated memory */
#define SB_CONCAT(x, y) x ## y
#define SB_TO_UL(s) SB_CONCAT(s, UL)
#define CFG_SYS_SDRAM_BASE		0
#define CFG_SYS_SDRAM_SIZE \
		(SB_TO_UL(CONFIG_SANDBOX_RAM_SIZE_MB) << 20)

#define CFG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600,\
					115200}

/* Unused but necessary to build */
#define CFG_SYS_UBOOT_BASE	CONFIG_TEXT_BASE

#endif
