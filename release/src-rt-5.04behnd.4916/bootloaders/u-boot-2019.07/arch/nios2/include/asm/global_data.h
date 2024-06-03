/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 */
#ifndef	__ASM_NIOS2_GLOBALDATA_H_
#define __ASM_NIOS2_GLOBALDATA_H_

/* Architecture-specific global data */
struct arch_global_data {
	u32 dcache_line_size;
	u32 icache_line_size;
	u32 dcache_size;
	u32 icache_size;
	u32 reset_addr;
	u32 exception_addr;
	int has_initda;
	int has_mmu;
	u32 io_region_base;
	u32 mem_region_base;
	u32 physaddr_mask;
};

#include <asm-generic/global_data.h>

#define DECLARE_GLOBAL_DATA_PTR     register gd_t *gd asm ("gp")

#endif /* __ASM_NIOS2_GLOBALDATA_H_ */
