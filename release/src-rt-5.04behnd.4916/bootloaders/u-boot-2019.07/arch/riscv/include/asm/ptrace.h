/*
 * Copyright (c) 2017 Microsemi Corporation.
 * Copyright (c) 2017 Padmarao Begari <Padmarao.Begari@microsemi.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __ASM_RISCV_PTRACE_H
#define __ASM_RISCV_PTRACE_H

struct pt_regs {
	unsigned long sepc;
	unsigned long ra;
	unsigned long sp;
	unsigned long gp;
	unsigned long tp;
	unsigned long t0;
	unsigned long t1;
	unsigned long t2;
	unsigned long s0;
	unsigned long s1;
	unsigned long a0;
	unsigned long a1;
	unsigned long a2;
	unsigned long a3;
	unsigned long a4;
	unsigned long a5;
	unsigned long a6;
	unsigned long a7;
	unsigned long s2;
	unsigned long s3;
	unsigned long s4;
	unsigned long s5;
	unsigned long s6;
	unsigned long s7;
	unsigned long s8;
	unsigned long s9;
	unsigned long s10;
	unsigned long s11;
	unsigned long t3;
	unsigned long t4;
	unsigned long t5;
	unsigned long t6;
	/* Supervisor CSRs */
	unsigned long sstatus;
	unsigned long sbadaddr;
	unsigned long scause;
};

#ifdef CONFIG_64BIT
#define REG_FMT "%016lx"
#else
#define REG_FMT "%08lx"
#endif

#define user_mode(regs) (((regs)->sstatus & SR_PS) == 0)

/* Helpers for working with the instruction pointer */
#define GET_IP(regs) ((regs)->sepc)
#define SET_IP(regs, val) (GET_IP(regs) = (val))

static inline unsigned long instruction_pointer(struct pt_regs *regs)
{
	return GET_IP(regs);
}

static inline void instruction_pointer_set(struct pt_regs *regs, ulong val)
{
	SET_IP(regs, val);
}

#define profile_pc(regs) instruction_pointer(regs)

/* Helpers for working with the user stack pointer */
#define GET_USP(regs) ((regs)->sp)
#define SET_USP(regs, val) (GET_USP(regs) = (val))

static inline unsigned long user_stack_pointer(struct pt_regs *regs)
{
	return GET_USP(regs);
}

static inline void user_stack_pointer_set(struct pt_regs *regs, ulong val)
{
	SET_USP(regs, val);
}

/* Helpers for working with the frame pointer */
#define GET_FP(regs) ((regs)->s0)
#define SET_FP(regs, val) (GET_FP(regs) = (val))

static inline unsigned long frame_pointer(struct pt_regs *regs)
{
	return GET_FP(regs);
}

static inline void frame_pointer_set(struct pt_regs *regs, ulong val)
{
	SET_FP(regs, val);
}

#endif /* __ASM_RISCV_PTRACE_H */
