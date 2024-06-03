/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016 Imagination Technologies
 */

#ifndef __BOARD_BOSTON_REGS_H__
#define __BOARD_BOSTON_REGS_H__

#include <asm/addrspace.h>

#define BOSTON_PLAT_BASE		CKSEG1ADDR(0x17ffd000)
#define BOSTON_LCD_BASE			CKSEG1ADDR(0x17fff000)

/*
 * Platform Register Definitions
 */
#define BOSTON_PLAT_CORE_CL		(BOSTON_PLAT_BASE + 0x04)

#define BOSTON_PLAT_DDR3STAT		(BOSTON_PLAT_BASE + 0x14)
# define BOSTON_PLAT_DDR3STAT_CALIB	(1 << 2)

#define BOSTON_PLAT_DDRCONF0		(BOSTON_PLAT_BASE + 0x38)
# define BOSTON_PLAT_DDRCONF0_SIZE	(0xf << 0)

#endif /* __BOARD_BOSTON_REGS_H__ */
