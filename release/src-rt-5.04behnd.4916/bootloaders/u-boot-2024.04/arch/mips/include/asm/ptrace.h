/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 1994, 95, 96, 97, 98, 99, 2000 by Ralf Baechle
 * Copyright (C) 1999, 2000 Silicon Graphics, Inc.
 */
#ifndef _ASM_PTRACE_H
#define _ASM_PTRACE_H

#include <linux/compiler.h>
#include <linux/types.h>
#include <asm/isadep.h>

/*
 * This struct defines the way the registers are stored on the stack during a
 * system call/exception. As usual the registers k0/k1 aren't being saved.
 *
 * If you add a register here, also add it to regoffset_table[] in
 * arch/mips/kernel/ptrace.c.
 */
struct pt_regs {
#ifdef CONFIG_32BIT
	/* Pad bytes for argument save space on the stack. */
	unsigned long pad0[8];
#endif

	/* Saved main processor registers. */
	unsigned long regs[32];

	/* Saved special registers. */
	unsigned long cp0_status;
	unsigned long hi;
	unsigned long lo;
#ifdef CONFIG_CPU_HAS_SMARTMIPS
	unsigned long acx;
#endif
	unsigned long cp0_badvaddr;
	unsigned long cp0_cause;
	unsigned long cp0_epc;
#ifdef CONFIG_CPU_CAVIUM_OCTEON
	unsigned long long mpl[6];        /* MTM{0-5} */
	unsigned long long mtp[6];        /* MTP{0-5} */
#endif
	unsigned long __last[0];
} __aligned(8);

static inline unsigned long kernel_stack_pointer(struct pt_regs *regs)
{
	return regs->regs[31];
}

/*
 * Don't use asm-generic/ptrace.h it defines FP accessors that don't make
 * sense on MIPS.  We rather want an error if they get invoked.
 */

static inline void instruction_pointer_set(struct pt_regs *regs,
						unsigned long val)
{
	regs->cp0_epc = val;
}

/* Query offset/name of register from its name/offset */
extern int regs_query_register_offset(const char *name);
#define MAX_REG_OFFSET (offsetof(struct pt_regs, __last))

/**
 * regs_get_register() - get register value from its offset
 * @regs:       pt_regs from which register value is gotten.
 * @offset:     offset number of the register.
 *
 * regs_get_register returns the value of a register. The @offset is the
 * offset of the register in struct pt_regs address which specified by @regs.
 * If @offset is bigger than MAX_REG_OFFSET, this returns 0.
 */
static inline unsigned long regs_get_register(struct pt_regs *regs,
						unsigned int offset)
{
	if (unlikely(offset > MAX_REG_OFFSET))
		return 0;

	return *(unsigned long *)((unsigned long)regs + offset);
}

/*
 * Does the process account for user or for system time?
 */
#define user_mode(regs) (((regs)->cp0_status & KU_MASK) == KU_USER)

#define instruction_pointer(regs) ((regs)->cp0_epc)
#define profile_pc(regs) instruction_pointer(regs)

/* Helpers for working with the user stack pointer */

static inline unsigned long user_stack_pointer(struct pt_regs *regs)
{
	return regs->regs[29];
}

static inline void user_stack_pointer_set(struct pt_regs *regs,
						unsigned long val)
{
	regs->regs[29] = val;
}

#endif /* _ASM_PTRACE_H */
