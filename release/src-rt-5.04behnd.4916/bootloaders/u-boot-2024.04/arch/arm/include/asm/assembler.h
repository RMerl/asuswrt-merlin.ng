/*
 *  arch/arm/include/asm/assembler.h
 *
 *  Copyright (C) 1996-2000 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  This file contains arm architecture specific defines
 *  for the different processors.
 *
 *  Do not include any C declarations in this file - it is included by
 *  assembler source.
 */

#include <asm/unified.h>

/*
 * Endian independent macros for shifting bytes within registers.
 */
#ifndef __ARMEB__
#define lspull		lsr
#define lspush		lsl
#define get_byte_0	lsl #0
#define get_byte_1	lsr #8
#define get_byte_2	lsr #16
#define get_byte_3	lsr #24
#define put_byte_0	lsl #0
#define put_byte_1	lsl #8
#define put_byte_2	lsl #16
#define put_byte_3	lsl #24
#else
#define lspull		lsl
#define lspush		lsr
#define get_byte_0	lsr #24
#define get_byte_1	lsr #16
#define get_byte_2	lsr #8
#define get_byte_3      lsl #0
#define put_byte_0	lsl #24
#define put_byte_1	lsl #16
#define put_byte_2	lsl #8
#define put_byte_3      lsl #0
#endif

/*
 * Data preload for architectures that support it
 */
#if defined(__ARM_ARCH_5E__) || defined(__ARM_ARCH_5TE__) || \
	defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || \
	defined(__ARM_ARCH_6T2__) || defined(__ARM_ARCH_6Z__) || \
	defined(__ARM_ARCH_6ZK__) || defined(__ARM_ARCH_7A__) || \
	defined(__ARM_ARCH_7R__)
#define PLD(code...)	code
#else
#define PLD(code...)
#endif

/*
 * Use 'bx lr' everywhere except ARMv4 (without 'T') where only 'mov pc, lr'
 * works
 */
	.irp	c,,eq,ne,cs,cc,mi,pl,vs,vc,hi,ls,ge,lt,gt,le,hs,lo
	.macro	ret\c, reg

	/* ARMv4- don't know bx lr but the assembler fails to see that */
#ifdef __ARM_ARCH_4__
	mov\c	pc, \reg
#else
	.ifeqs	"\reg", "lr"
	bx\c	\reg
	.else
	mov\c	pc, \reg
	.endif
#endif
	.endm
	.endr

/*
 * Cache aligned, used for optimized memcpy/memset
 * In the kernel this is only enabled for Feroceon CPU's...
 * We disable it especially for Thumb builds since those instructions
 * are not made in a Thumb ready way...
 */
#if CONFIG_IS_ENABLED(SYS_THUMB_BUILD)
#define CALGN(code...)
#else
#define CALGN(code...) code
#endif
