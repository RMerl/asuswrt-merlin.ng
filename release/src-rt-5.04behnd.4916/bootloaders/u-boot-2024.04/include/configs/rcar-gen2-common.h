/* SPDX-License-Identifier: GPL-2.0 */
/*
 * include/configs/rcar-gen2-common.h
 *
 * Copyright (C) 2013,2014 Renesas Electronics Corporation
 */

#ifndef __RCAR_GEN2_COMMON_H
#define __RCAR_GEN2_COMMON_H

#include <asm/arch/renesas.h>

/* console */
#define CFG_SYS_BAUDRATE_TABLE	{ 38400, 115200 }

#define CFG_SYS_SDRAM_BASE		(RCAR_GEN2_SDRAM_BASE)
#define CFG_SYS_SDRAM_SIZE		(RCAR_GEN2_UBOOT_SDRAM_SIZE)

/* Timer */
#define CFG_SYS_TIMER_COUNTER	(TMU_BASE + 0xc)	/* TCNT0 */
#define CFG_SYS_TIMER_RATE		(get_board_sys_clk() / 8)

#endif	/* __RCAR_GEN2_COMMON_H */
