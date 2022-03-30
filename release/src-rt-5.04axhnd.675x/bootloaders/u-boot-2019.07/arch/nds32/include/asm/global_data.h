/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2011 Andes Technology Corporation
 * Shawn Lin, Andes Technology Corporation <nobuhiro@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 */

/**************************************************************
 * CAUTION:
 *   - do not implement for NDS32 Arch yet.
 *   - so far no one uses the macros defined in this head file.
 **************************************************************/

#ifndef	__ASM_GBL_DATA_H
#define __ASM_GBL_DATA_H

/* Architecture-specific global data */
struct arch_global_data {
};

#include <asm-generic/global_data.h>

#ifdef CONFIG_GLOBAL_DATA_NOT_REG10
extern volatile gd_t g_gd;
#define DECLARE_GLOBAL_DATA_PTR		static volatile gd_t *gd = &g_gd
#else
#define DECLARE_GLOBAL_DATA_PTR		register volatile gd_t *gd asm ("$r10")
#endif

#endif /* __ASM_GBL_DATA_H */
