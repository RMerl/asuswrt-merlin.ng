/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Lei Wen <leiwen@marvell.com>
 */

/*
 * This file should be included in board config header file.
 *
 * It supports common definitions for MVEBU platforms
 */

#ifndef _MVEBU_CONFIG_H
#define _MVEBU_CONFIG_H

#include <asm/arch/soc.h>

#if defined(CONFIG_ARMADA_XP) || defined(CONFIG_ARMADA_375) \
	|| defined(CONFIG_ARMADA_38X) || defined(CONFIG_ARMADA_MSYS)
/*
 * Set this for the common xor register definitions needed in dram.c
 * for A38x as well here.
 */
#define MV88F78X60 /* for the DDR training bin_hdr code */
#endif

#define MV_UART_CONSOLE_BASE		MVEBU_UART0_BASE

#endif /* __MVEBU_CONFIG_H */
