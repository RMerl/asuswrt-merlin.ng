/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#ifndef __ASM_SANDBOX_PTRACE_H
#define __ASM_SANDBOX_PTRACE_H

#ifndef __ASSEMBLY__
/* This is not used in the sandbox architecture, but required by U-Boot */
struct pt_regs {
};

#ifdef __KERNEL__
extern void show_regs(struct pt_regs *);

#endif

#endif /* __ASSEMBLY__ */

#endif
