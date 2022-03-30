/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007, Tensilica Inc.
 */

#ifndef	_XTENSA_GBL_DATA_H
#define _XTENSA_GBL_DATA_H

/* Architecture-specific global data */

struct arch_global_data {
	unsigned long cpu_clk;
};

#include <asm-generic/global_data.h>

#define DECLARE_GLOBAL_DATA_PTR     extern gd_t *gd

#endif	/* _XTENSA_GBL_DATA_H */
