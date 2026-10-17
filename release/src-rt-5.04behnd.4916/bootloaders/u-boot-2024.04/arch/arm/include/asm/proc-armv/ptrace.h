/*
 *  linux/include/asm-arm/proc-armv/ptrace.h
 *
 *  Copyright (C) 1996-1999 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __ASM_PROC_PTRACE_H
#define __ASM_PROC_PTRACE_H

#ifdef CONFIG_ARM64

#define PCMASK		0

/*
 * PSR bits
 */
#define PSR_MODE_EL0t	0x00000000
#define PSR_MODE_EL1t	0x00000004
#define PSR_MODE_EL1h	0x00000005
#define PSR_MODE_EL2t	0x00000008
#define PSR_MODE_EL2h	0x00000009
#define PSR_MODE_EL3t	0x0000000c
#define PSR_MODE_EL3h	0x0000000d
#define PSR_MODE_MASK	0x0000000f

/* AArch32 CPSR bits */
#define PSR_MODE32_BIT		0x00000010

/* AArch64 SPSR bits */
#define PSR_F_BIT	0x00000040
#define PSR_I_BIT	0x00000080
#define PSR_A_BIT	0x00000100
#define PSR_D_BIT	0x00000200
#define PSR_BTYPE_MASK	0x00000c00
#define PSR_SSBS_BIT	0x00001000
#define PSR_PAN_BIT	0x00400000
#define PSR_UAO_BIT	0x00800000
#define PSR_DIT_BIT	0x01000000
#define PSR_TCO_BIT	0x02000000
#define PSR_V_BIT	0x10000000
#define PSR_C_BIT	0x20000000
#define PSR_Z_BIT	0x40000000
#define PSR_N_BIT	0x80000000

#define PSR_BTYPE_SHIFT		10

/*
 * Groups of PSR bits
 */
#define PSR_f		0xff000000	/* Flags		*/
#define PSR_s		0x00ff0000	/* Status		*/
#define PSR_x		0x0000ff00	/* Extension		*/
#define PSR_c		0x000000ff	/* Control		*/

/* Convenience names for the values of PSTATE.BTYPE */
#define PSR_BTYPE_NONE		(0b00 << PSR_BTYPE_SHIFT)
#define PSR_BTYPE_JC		(0b01 << PSR_BTYPE_SHIFT)
#define PSR_BTYPE_C		(0b10 << PSR_BTYPE_SHIFT)
#define PSR_BTYPE_J		(0b11 << PSR_BTYPE_SHIFT)

/* SPSR_ELx bits for exceptions taken from AArch32 */
#define PSR_AA32_MODE_MASK	0x0000001f
#define PSR_AA32_MODE_USR	0x00000010
#define PSR_AA32_MODE_FIQ	0x00000011
#define PSR_AA32_MODE_IRQ	0x00000012
#define PSR_AA32_MODE_SVC	0x00000013
#define PSR_AA32_MODE_ABT	0x00000017
#define PSR_AA32_MODE_HYP	0x0000001a
#define PSR_AA32_MODE_UND	0x0000001b
#define PSR_AA32_MODE_SYS	0x0000001f
#define PSR_AA32_T_BIT		0x00000020
#define PSR_AA32_F_BIT		0x00000040
#define PSR_AA32_I_BIT		0x00000080
#define PSR_AA32_A_BIT		0x00000100
#define PSR_AA32_E_BIT		0x00000200
#define PSR_AA32_PAN_BIT	0x00400000
#define PSR_AA32_SSBS_BIT	0x00800000
#define PSR_AA32_DIT_BIT	0x01000000
#define PSR_AA32_Q_BIT		0x08000000
#define PSR_AA32_V_BIT		0x10000000
#define PSR_AA32_C_BIT		0x20000000
#define PSR_AA32_Z_BIT		0x40000000
#define PSR_AA32_N_BIT		0x80000000
#define PSR_AA32_IT_MASK	0x0600fc00	/* If-Then execution state mask */
#define PSR_AA32_GE_MASK	0x000f0000

#ifndef __ASSEMBLY__

/*
 * This struct defines the way the registers are stored
 * on the stack during an exception.
 */
struct pt_regs {
	unsigned long spsr;
	unsigned long elr;
	unsigned long esr;
	unsigned long regs[31];
};

#endif	/* __ASSEMBLY__ */

#else	/* CONFIG_ARM64 */

#define USR26_MODE	0x00
#define FIQ26_MODE	0x01
#define IRQ26_MODE	0x02
#define SVC26_MODE	0x03
#define USR_MODE	0x10
#define FIQ_MODE	0x11
#define IRQ_MODE	0x12
#define SVC_MODE	0x13
#define MON_MODE	0x16
#define ABT_MODE	0x17
#define HYP_MODE	0x1a
#define UND_MODE	0x1b
#define SYSTEM_MODE	0x1f
#define MODE_MASK	0x1f
#define T_BIT		0x20
#define F_BIT		0x40
#define I_BIT		0x80
#define A_BIT		0x100
#define CC_V_BIT	(1 << 28)
#define CC_C_BIT	(1 << 29)
#define CC_Z_BIT	(1 << 30)
#define CC_N_BIT	(1 << 31)
#define PCMASK		0

#ifndef __ASSEMBLY__

/* this struct defines the way the registers are stored on the
   stack during a system call. */

struct pt_regs {
	long uregs[18];
};

#define ARM_cpsr	uregs[16]
#define ARM_pc		uregs[15]
#define ARM_lr		uregs[14]
#define ARM_sp		uregs[13]
#define ARM_ip		uregs[12]
#define ARM_fp		uregs[11]
#define ARM_r10		uregs[10]
#define ARM_r9		uregs[9]
#define ARM_r8		uregs[8]
#define ARM_r7		uregs[7]
#define ARM_r6		uregs[6]
#define ARM_r5		uregs[5]
#define ARM_r4		uregs[4]
#define ARM_r3		uregs[3]
#define ARM_r2		uregs[2]
#define ARM_r1		uregs[1]
#define ARM_r0		uregs[0]
#define ARM_ORIG_r0	uregs[17]

#ifdef __KERNEL__

#define user_mode(regs)	\
	(((regs)->ARM_cpsr & 0xf) == 0)

#if CONFIG_IS_ENABLED(SYS_THUMB_BUILD)
#define thumb_mode(regs) \
	(((regs)->ARM_cpsr & T_BIT))
#else
#define thumb_mode(regs) (0)
#endif

#define processor_mode(regs) \
	((regs)->ARM_cpsr & MODE_MASK)

#define interrupts_enabled(regs) \
	(!((regs)->ARM_cpsr & I_BIT))

#define fast_interrupts_enabled(regs) \
	(!((regs)->ARM_cpsr & F_BIT))

#define condition_codes(regs) \
	((regs)->ARM_cpsr & (CC_V_BIT|CC_C_BIT|CC_Z_BIT|CC_N_BIT))

/* Are the current registers suitable for user mode?
 * (used to maintain security in signal handlers)
 */
static inline int valid_user_regs(struct pt_regs *regs)
{
	if ((regs->ARM_cpsr & 0xf) == 0 &&
	    (regs->ARM_cpsr & (F_BIT|I_BIT)) == 0)
		return 1;

	/*
	 * Force CPSR to something logical...
	 */
	regs->ARM_cpsr &= (CC_V_BIT|CC_C_BIT|CC_Z_BIT|CC_N_BIT|0x10);

	return 0;
}

#endif	/* __KERNEL__ */

#endif	/* __ASSEMBLY__ */

#endif	/* CONFIG_ARM64 */

#endif
