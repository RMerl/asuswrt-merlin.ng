/*
 * include/asm-microblaze/ptrace.h -- Access to CPU registers
 *
 *  Copyright (C) 2003       John Williams <jwilliams@itee.uq.edu.au>
 *  Copyright (C) 2001,2002  NEC Corporation
 *  Copyright (C) 2001,2002  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 * Microblaze port by John Williams
 */

#ifndef __MICROBLAZE_PTRACE_H__
#define __MICROBLAZE_PTRACE_H__


/* Microblaze general purpose registers with special meanings.  */
#define GPR_ZERO	0	/* constant zero */
#define GPR_ASM		18	/* reserved for assembler */
#define GPR_SP		1	/* stack pointer */
#define GPR_GP		2	/* global data pointer */
#define GPR_EP		30	/* `element pointer' */
#define GPR_LP		15	/* link pointer (current return address) */

/* These aren't official names, but they make some code more descriptive.  */
#define GPR_ARG0	5
#define GPR_ARG1	6
#define GPR_ARG2	7
#define GPR_ARG3	8
#define GPR_ARG4	9
#define GPR_ARG5	10
#define GPR_RVAL0	3
#define GPR_RVAL1	4
#define GPR_RVAL	GPR_RVAL0

#define NUM_GPRS	32

/* `system' registers.  */
/* Note these are old v850 values, microblaze has many fewer */
#define SR_EIPC		0
#define SR_EIPSW	1
#define SR_FEPC		2
#define SR_FEPSW	3
#define SR_ECR		4
#define SR_PSW		5
#define SR_CTPC		16
#define SR_CTPSW	17
#define SR_DBPC		18
#define SR_DBPSW	19
#define SR_CTBP		20
#define SR_DIR		21
#define SR_ASID		23


#ifndef __ASSEMBLY__

typedef unsigned long microblaze_reg_t;

/* How processor state is stored on the stack during a syscall/signal.
   If you change this structure, change the associated assembly-language
   macros below too (PT_*)!  */
struct pt_regs
{
	/* General purpose registers.  */
	microblaze_reg_t gpr[NUM_GPRS];

	microblaze_reg_t pc;		/* program counter */
	microblaze_reg_t psw;		/* program status word */

	microblaze_reg_t kernel_mode;	/* 1 if in `kernel mode', 0 if user mode */
	microblaze_reg_t single_step;	/* 1 if in single step mode */
};


#define instruction_pointer(regs)	((regs)->pc)
#define user_mode(regs)			(!(regs)->kernel_mode)

/* When a struct pt_regs is used to save user state for a system call in
   the kernel, the system call is stored in the space for R0 (since it's
   never used otherwise, R0 being a constant 0).  Non-system-calls
   simply store 0 there.  */
#define PT_REGS_SYSCALL(regs)		(regs)->gpr[0]
#define PT_REGS_SET_SYSCALL(regs, val)	((regs)->gpr[0] = (val))

#endif /* !__ASSEMBLY__ */


/* The number of bytes used to store each register.  */
#define _PT_REG_SIZE	4

/* Offset of a general purpose register in a stuct pt_regs.  */
#define PT_GPR(num)	((num) * _PT_REG_SIZE)

/* Offsets of various special registers & fields in a struct pt_regs.  */
#define NUM_SPECIAL	4
#define PT_PC		((NUM_GPRS + 0) * _PT_REG_SIZE)
#define PT_PSW		((NUM_GPRS + 1) * _PT_REG_SIZE)
#define PT_KERNEL_MODE	((NUM_GPRS + 2) * _PT_REG_SIZE)
#define PT_SINGLESTEP	((NUM_GPRS + 3) * _PT_REG_SIZE)

#define PT_SYSCALL	PT_GPR(0)

/* Size of struct pt_regs, including alignment.  */
#define PT_SIZE		((NUM_GPRS + NUM_SPECIAL) * _PT_REG_SIZE)

/* These are `magic' values for PTRACE_PEEKUSR that return info about where
   a process is located in memory.  */
#define PT_TEXT_ADDR    (PT_SIZE + 1)
#define PT_TEXT_LEN     (PT_SIZE + 2)
#define PT_DATA_ADDR    (PT_SIZE + 3)
#define PT_DATA_LEN     (PT_SIZE + 4)

#endif /* __MICROBLAZE_PTRACE_H__ */
