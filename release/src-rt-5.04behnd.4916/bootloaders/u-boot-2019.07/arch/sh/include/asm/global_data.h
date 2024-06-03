/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2007
 * Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 */

#ifndef	__ASM_SH_GLOBALDATA_H_
#define __ASM_SH_GLOBALDATA_H_

/* Architecture-specific global data */
struct arch_global_data {
};

#include <asm-generic/global_data.h>

#define DECLARE_GLOBAL_DATA_PTR	register gd_t *gd asm ("r13")

#endif /* __ASM_SH_GLOBALDATA_H_ */
