/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002 - 2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef	__ASM_GBL_DATA_H
#define __ASM_GBL_DATA_H

/* Architecture-specific global data */
struct arch_global_data {
#ifdef CONFIG_SYS_I2C_FSL
	unsigned long	i2c1_clk;
	unsigned long	i2c2_clk;
#endif
#ifdef CONFIG_EXTRA_CLOCK
	unsigned long inp_clk;
	unsigned long vco_clk;
	unsigned long flb_clk;
#endif
#ifdef CONFIG_MCF5441x
	unsigned long sdhc_clk;
#endif
};

#include <asm-generic/global_data.h>

#if 0
extern gd_t *global_data;
#define DECLARE_GLOBAL_DATA_PTR     gd_t *gd = global_data
#else
#define DECLARE_GLOBAL_DATA_PTR     register volatile gd_t *gd asm ("d7")
#endif

#endif /* __ASM_GBL_DATA_H */
