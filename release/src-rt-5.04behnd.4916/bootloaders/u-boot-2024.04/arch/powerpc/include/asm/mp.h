/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2009-2010 Freescale Semiconductor, Inc.
 */

#ifndef _ASM_MP_H_
#define _ASM_MP_H_

#include <lmb.h>

void setup_mp(void);
void cpu_mp_lmb_reserve(struct lmb *lmb);
u32 determine_mp_bootpg(unsigned int *pagesize);
int is_core_disabled(int nr);

#ifdef CONFIG_E6500
#define thread_to_core(x) (x >> 1)
#else
#define thread_to_core(x) (x)
#endif

#endif
