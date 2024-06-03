/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef	__ASM_GBL_DATA_H
#define __ASM_GBL_DATA_H

#include <asm/regdef.h>

/* Architecture-specific global data */
struct arch_global_data {
#ifdef CONFIG_DYNAMIC_IO_PORT_BASE
	unsigned long io_port_base;
#endif
#ifdef CONFIG_ARCH_ATH79
	unsigned long id;
	unsigned long soc;
	unsigned long rev;
	unsigned long ver;
#endif
#ifdef CONFIG_SYS_CACHE_SIZE_AUTO
	unsigned short l1i_line_size;
	unsigned short l1d_line_size;
#endif
#ifdef CONFIG_MIPS_L2_CACHE
	unsigned short l2_line_size;
#endif
};

#include <asm-generic/global_data.h>

#define DECLARE_GLOBAL_DATA_PTR     register volatile gd_t *gd asm ("k0")

#endif /* __ASM_GBL_DATA_H */
