/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2004 Atmark Techno, Inc.
 *
 * Yasushi SHOJI <yashi@atmark-techno.com>
 */

#ifndef	__ASM_GBL_DATA_H
#define __ASM_GBL_DATA_H

#include <asm/cpuinfo.h>

/* Architecture-specific global data */
struct arch_global_data {
	struct microblaze_cpuinfo cpuinfo;
};

#include <asm-generic/global_data.h>

#define DECLARE_GLOBAL_DATA_PTR     register volatile gd_t *gd asm ("r31")

#define gd_cpuinfo()	((struct microblaze_cpuinfo *)&gd->arch.cpuinfo)

#endif /* __ASM_GBL_DATA_H */
