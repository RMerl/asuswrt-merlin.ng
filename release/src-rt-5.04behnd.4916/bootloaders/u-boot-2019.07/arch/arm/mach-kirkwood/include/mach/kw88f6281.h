/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * Header file for Feroceon CPU core 88FR131 Based KW88F6281 SOC.
 */

#ifndef _ASM_ARCH_KW88F6281_H
#define _ASM_ARCH_KW88F6281_H

/* SOC specific definitions */
#define KW88F6281_REGS_PHYS_BASE	0xf1000000
#define KW_REGS_PHY_BASE		KW88F6281_REGS_PHYS_BASE

/* TCLK Core Clock definition */
#ifndef CONFIG_SYS_TCLK
#define CONFIG_SYS_TCLK	200000000 /* 200MHz */
#endif

#endif /* _ASM_ARCH_KW88F6281_H */
