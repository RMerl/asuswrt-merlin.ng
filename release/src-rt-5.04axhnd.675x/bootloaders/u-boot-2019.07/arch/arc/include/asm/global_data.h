/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 */

#ifndef	__ASM_ARC_GLOBAL_DATA_H
#define __ASM_ARC_GLOBAL_DATA_H

#include <config.h>

#ifndef __ASSEMBLY__
/* Architecture-specific global data */
struct arch_global_data {
	int l1_line_sz;
#if defined(CONFIG_ISA_ARCV2)
	int slc_line_sz;
#endif
};
#endif /* __ASSEMBLY__ */

#include <asm-generic/global_data.h>

#define DECLARE_GLOBAL_DATA_PTR		register volatile gd_t *gd asm ("r25")

#endif /* __ASM_ARC_GLOBAL_DATA_H */
