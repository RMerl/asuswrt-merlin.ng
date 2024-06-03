/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (c) 2017 Microsemi Corporation.
 * Padmarao Begari, Microsemi Corporation <padmarao.begari@microsemi.com>
 */

#ifndef	__ASM_GBL_DATA_H
#define __ASM_GBL_DATA_H

#include <asm/smp.h>

/* Architecture-specific global data */
struct arch_global_data {
	long boot_hart;		/* boot hart id */
#ifdef CONFIG_SIFIVE_CLINT
	void __iomem *clint;	/* clint base address */
#endif
#ifdef CONFIG_ANDES_PLIC
	void __iomem *plic;	/* plic base address */
#endif
#ifdef CONFIG_ANDES_PLMT
	void __iomem *plmt;	/* plmt base address */
#endif
#ifdef CONFIG_SMP
	struct ipi_data ipi[CONFIG_NR_CPUS];
#endif
#ifndef CONFIG_XIP
	ulong available_harts;
#endif
};

#include <asm-generic/global_data.h>

#define DECLARE_GLOBAL_DATA_PTR register gd_t *gd asm ("gp")

#endif /* __ASM_GBL_DATA_H */
