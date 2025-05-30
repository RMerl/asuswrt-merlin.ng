/*
 * RTE ARM run-time initialization and first level exception handling.
 *
 * Copyright (C) 2024, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: startarm.S 821234 2023-02-06 14:16:52Z $
 */

#ifndef	__arm__
#error __arm__ is NOT defined
#endif

/* Routine begin/end macro */
#if defined(__thumb__)
#define FUNC(x)	THUMBLEAF(x)
#else
#define FUNC(x)	LEAF(x)
#endif	/* __thumb__ */

/* Debug macro - write a number to memory - use it with caution,
 *  it changes r0 and r1 registers.
 */
#ifdef BCMDBG_BOOT
#define TRACE(x) \
	ldr	r0,=x; \
	ldr	r1,=__watermark; \
	str	r0,[r1]

/* Similar to TRACE but restores r0 and r1 registers
 * BUT stack must be initialized before invoking
 */
#define TRACE_SAFE(x)		\
	push	{r0,r1};	\
	TRACE	(x);		\
	pop	{r0,r1};
#else
#define TRACE(x)
#define TRACE_SAFE(x)
#endif	/* BCMDBG */

/* Instruction macros that hide machine/arch details. */
.macro _LSL_ rd,rm,im
	lsl \rd,\rm,\im
.endm
.macro _LSR_ rd,rm,im
	lsr \rd,\rm,\im
.endm
.macro _ROR_ rd,rm,im
	ror \rd,\rm,\im
.endm
.macro _MUL_ rd,rm,rs
	mov \rd,\rm
	mul \rd,\rd,\rs
.endm

#if defined(__ARM_ARCH_7R__)
#include "startarm-cr4.S"
#elif defined(__ARM_ARCH_7M__)
#include "startarm-cm3.S"
#elif defined(__ARM_ARCH_7A__)
#include "startarm-ca7.S"
#elif defined(__ARM_ARCH_8A__) && defined(__ARM_32BIT_STATE)
/* On v8 ARM but in AArch32 mode, use the CA7 variant for now */
#include "startarm-ca7.S"
#else
#error Unrecognized ARM architecture
#endif
