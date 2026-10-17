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

#include <linux/types.h>
#include <asm/smp.h>
#include <asm/u-boot.h>
#include <compiler.h>

/* Architecture-specific global data */
struct arch_global_data {
	long boot_hart;		/* boot hart id */
	phys_addr_t firmware_fdt_addr;
#if CONFIG_IS_ENABLED(RISCV_ACLINT)
	void __iomem *aclint;	/* aclint base address */
#endif
#ifdef CONFIG_ANDES_PLICSW
	void __iomem *plicsw;	/* andes plicsw base address */
#endif
#if CONFIG_IS_ENABLED(SMP)
	struct ipi_data ipi[CONFIG_NR_CPUS];
#endif
#if !CONFIG_IS_ENABLED(XIP)
#ifdef CONFIG_AVAILABLE_HARTS
	ulong available_harts;
#endif
#endif
#if CONFIG_IS_ENABLED(ACPI)
	ulong table_start;		/* Start address of ACPI tables */
	ulong table_end;		/* End address of ACPI tables */
	ulong table_start_high;		/* Start address of high ACPI tables */
	ulong table_end_high;		/* End address of high ACPI tables */
#endif
#ifdef CONFIG_SMBIOS
	ulong smbios_start;		/* Start address of SMBIOS table */
#endif
};

#include <asm-generic/global_data.h>

#define DECLARE_GLOBAL_DATA_PTR register gd_t *gd asm ("gp")

static inline void set_gd(volatile gd_t *gd_ptr)
{
#ifdef CONFIG_64BIT
	asm volatile("ld gp, %0\n" : : "m"(gd_ptr));
#else
	asm volatile("lw gp, %0\n" : : "m"(gd_ptr));
#endif
}

#endif /* __ASM_GBL_DATA_H */
