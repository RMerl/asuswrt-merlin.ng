/*
 * HND arm trap handling.
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * $Id: hnd_armtrap.h 821234 2023-02-06 14:16:52Z $
 */

#ifndef	_hnd_armtrap_h_
#define	_hnd_armtrap_h_

/* ARM trap handling */

/* Trap types defined by ARM (see arminc.h) */

/* Trap locations in lo memory */
#define	TRAP_STRIDE	4
#define FIRST_TRAP	TR_RST
#define LAST_TRAP	(TR_FIQ * TRAP_STRIDE)

#if defined(__ARM_ARCH_7M__)
#define	MAX_TRAP_TYPE	(TR_ISR + ARMCM3_NUMINTS)
#endif	/* __ARM_ARCH_7M__ */

/* The trap structure is defined here as offsets for assembly */
#define	TR_TYPE		0x00
#define	TR_EPC		0x04
#define	TR_CPSR		0x08
#define	TR_SPSR		0x0c
#define	TR_REGS		0x10
#define	TR_REG(n)	(TR_REGS + (n) * 4)
#define	TR_SP		TR_REG(13)
#define	TR_LR		TR_REG(14)
#define	TR_PC		TR_REG(15)

#define	TRAP_T_SIZE	80
#define ASSERT_TRAP_SVC_NUMBER	255

#ifndef	_LANGUAGE_ASSEMBLY

#include <typedefs.h>

/*
 * defines for cp15 DFSR/ISFR register providing abort details
 *
 * these registers can be in short or long format. Bit 9 is
 * reserved, but... for the short format it is always zero and
 * for long format it is always one.
 */
#define ARM_CP15_FSR_FORMAT_RSVD_MASK		0x200
#define ARM_CP15_FSR_FORMAT_LONG(_fsr)		((_fsr) & ARM_CP15_FSR_FORMAT_RSVD_MASK)
#define ARM_CP15_FSR_FORMAT_SHORT(_fsr)		!ARM_CP15_FSR_FORMAT_LONG(_fsr)
#define ARM_CP15_FSR_LONG_FSTAT_MASK	0x3F
#define ARM_CP15_FSR_SHORT_FSTAT_MSB	0x400
#define ARM_CP15_FSR_SHORT_FSTAT_MASK	0xF
#define ARM_CP15_FSR_FAULT_STATUS(_fsr) \
	(ARM_CP15_FSR_FORMAT_LONG(_fsr) ? ((_fsr) & ARM_CP15_FSR_LONG_FSTAT_MASK) : \
	((((_fsr) & ARM_CP15_FSR_SHORT_FSTAT_MSB) >> 6) | ((_fsr) & ARM_CP15_FSR_SHORT_FSTAT_MASK)))

/* SHORT */
#define ARM_CP15_S_FSTAT_ALIGN		1
#define ARM_CP15_S_FSTAT_DEBUG		2
#define ARM_CP15_S_FSTAT_SEC_ACCESS	3
#define ARM_CP15_S_FSTAT_ICACHE		4
#define ARM_CP15_S_FSTAT_SEC_TRANS	5
#define ARM_CP15_S_FSTAT_PAGE_ACCESS	6
#define ARM_CP15_S_FSTAT_PAGE_TRANS	7
#define ARM_CP15_S_FSTAT_SEA		8
#define ARM_CP15_S_FSTAT_SEA_TLB1	0xC
#define ARM_CP15_S_FSTAT_SEC_PERM	0xD
#define ARM_CP15_S_FSTAT_SEA_TLB2	0xE
#define ARM_CP15_S_FSTAT_PAGE_PERM	0xF
#define ARM_CP15_S_FSTAT_AEA		0x16
/* LONG */
#define ARM_CP15_L_FSTAT_ALIGN		0x21
#define ARM_CP15_L_FSTAT_DEBUG		0x22
#define ARM_CP15_L_FSTAT_SEA		0x10
#define ARM_CP15_L_FSTAT_AEA		0x11
#define ARM_CP15_L_FSTAT_LL_MASK	0x03
/* below have LL bits indicating level */
#define ARM_CP15_L_FSTAT_SEA_TLB	0x14
#define ARM_CP15_L_FSTAT_ACCESS		0x08
#define ARM_CP15_L_FSTAT_TRANS		0x04
#define ARM_CP15_L_FSTAT_PERM		0x0C

typedef struct _trap_struct {
	uint32		type;
	uint32		epc;
	uint32		cpsr;
	uint32		spsr;
	uint32		r0;	/* a1 */
	uint32		r1;	/* a2 */
	uint32		r2;	/* a3 */
	uint32		r3;	/* a4 */
	uint32		r4;	/* v1 */
	uint32		r5;	/* v2 */
	uint32		r6;	/* v3 */
	uint32		r7;	/* v4 */
	uint32		r8;	/* v5 */
	uint32		r9;	/* sb/v6 */
	uint32		r10;	/* sl/v7 */
	uint32		r11;	/* fp/v8 */
	uint32		r12;	/* ip */
	uint32		r13;	/* sp */
	uint32		r14;	/* lr */
	uint32		pc;	/* r15 */
	uint32		abort_fsr;
	uint32		abort_far;
} trap_t;

#endif	/* !_LANGUAGE_ASSEMBLY */

#endif	/* _hnd_armtrap_h_ */
