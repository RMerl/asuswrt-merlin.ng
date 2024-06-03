/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 */

#ifndef __ASM_NIOS2_PTRACE_H_
#define __ASM_NIOS2_PTRACE_H_

struct pt_regs {
	unsigned reg[32];
	unsigned status;
};


#endif /* __ASM_NIOS2_PTRACE_H_ */
