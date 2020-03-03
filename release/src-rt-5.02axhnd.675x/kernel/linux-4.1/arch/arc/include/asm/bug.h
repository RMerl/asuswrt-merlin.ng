/*
 * Copyright (C) 2004, 2007-2010, 2011-2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _ASM_ARC_BUG_H
#define _ASM_ARC_BUG_H

#ifndef __ASSEMBLY__

#include <asm/ptrace.h>

struct task_struct;

void show_regs(struct pt_regs *regs);
void show_stacktrace(struct task_struct *tsk, struct pt_regs *regs);
void show_kernel_fault_diag(const char *str, struct pt_regs *regs,
			    unsigned long address);
void die(const char *str, struct pt_regs *regs, unsigned long address);

#define BUG()	do {								\
	pr_warn("BUG: failure at %s:%d/%s()!\n", __FILE__, __LINE__, __func__); \
	dump_stack();								\
} while (0)

#define HAVE_ARCH_BUG

#include <asm-generic/bug.h>

#endif	/* !__ASSEMBLY__ */

#endif
