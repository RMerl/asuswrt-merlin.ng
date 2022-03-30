/* SPDX-License-Identifier: GPL-2.0 */
/*
 * include/asm-arm/unified.h - Unified Assembler Syntax helper macros
 *
 * Copyright (C) 2008 ARM Limited
 */

#ifndef __ASM_UNIFIED_H
#define __ASM_UNIFIED_H

#if defined(__ASSEMBLY__) && defined(CONFIG_ARM_ASM_UNIFIED)
	.syntax unified
#endif

#ifdef CONFIG_CPU_V7M
#define AR_CLASS(x...)
#define M_CLASS(x...)	x
#else
#define AR_CLASS(x...)	x
#define M_CLASS(x...)
#endif

#ifdef CONFIG_THUMB2_KERNEL

#if __GNUC__ < 4
#error Thumb-2 kernel requires gcc >= 4
#endif

/* The CPSR bit describing the instruction set (Thumb) */
#define PSR_ISETSTATE	PSR_T_BIT

#define ARM(x...)
#define THUMB(x...)	x
#ifdef __ASSEMBLY__
#define W(instr)	instr.w
#else
#define WASM(instr)	#instr ".w"
#endif

#else	/* !CONFIG_THUMB2_KERNEL */

/* The CPSR bit describing the instruction set (ARM) */
#define PSR_ISETSTATE	0

#define ARM(x...)	x
#define THUMB(x...)
#ifdef __ASSEMBLY__
#define W(instr)	instr
#else
#define WASM(instr)	#instr
#endif

#endif	/* CONFIG_THUMB2_KERNEL */

#ifndef CONFIG_ARM_ASM_UNIFIED

/*
 * If the unified assembly syntax isn't used (in ARM mode), these
 * macros expand to an empty string
 */
#ifdef __ASSEMBLY__
	.macro	it, cond
	.endm
	.macro	itt, cond
	.endm
	.macro	ite, cond
	.endm
	.macro	ittt, cond
	.endm
	.macro	itte, cond
	.endm
	.macro	itet, cond
	.endm
	.macro	itee, cond
	.endm
	.macro	itttt, cond
	.endm
	.macro	ittte, cond
	.endm
	.macro	ittet, cond
	.endm
	.macro	ittee, cond
	.endm
	.macro	itett, cond
	.endm
	.macro	itete, cond
	.endm
	.macro	iteet, cond
	.endm
	.macro	iteee, cond
	.endm
#else	/* !__ASSEMBLY__ */
__asm__(
"	.macro	it, cond\n"
"	.endm\n"
"	.macro	itt, cond\n"
"	.endm\n"
"	.macro	ite, cond\n"
"	.endm\n"
"	.macro	ittt, cond\n"
"	.endm\n"
"	.macro	itte, cond\n"
"	.endm\n"
"	.macro	itet, cond\n"
"	.endm\n"
"	.macro	itee, cond\n"
"	.endm\n"
"	.macro	itttt, cond\n"
"	.endm\n"
"	.macro	ittte, cond\n"
"	.endm\n"
"	.macro	ittet, cond\n"
"	.endm\n"
"	.macro	ittee, cond\n"
"	.endm\n"
"	.macro	itett, cond\n"
"	.endm\n"
"	.macro	itete, cond\n"
"	.endm\n"
"	.macro	iteet, cond\n"
"	.endm\n"
"	.macro	iteee, cond\n"
"	.endm\n");
#endif	/* __ASSEMBLY__ */

#endif	/* CONFIG_ARM_ASM_UNIFIED */

#endif	/* !__ASM_UNIFIED_H */
