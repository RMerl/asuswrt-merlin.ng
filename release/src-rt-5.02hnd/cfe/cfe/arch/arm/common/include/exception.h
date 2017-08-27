/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  Exception/trap handler defs		File: exception.h
    *  
    *  This module describes the exception handlers, exception
    *  trap frames, and dispatch.
    *  
    *  Author:  Mitch Lichtenberg (mpl@broadcom.com)
    *  
    *********************************************************************  
    *
    *  Copyright 2000,2001,2002,2003
    *  Broadcom Corporation. All rights reserved.
    *  
    *  This software is furnished under license and may be used and 
    *  copied only in accordance with the following terms and 
    *  conditions.  Subject to these conditions, you may download, 
    *  copy, install, use, modify and distribute modified or unmodified 
    *  copies of this software in source and/or binary form.  No title 
    *  or ownership is transferred hereby.
    *  
    *  1) Any source code used, modified or distributed must reproduce 
    *     and retain this copyright notice and list of conditions 
    *     as they appear in the source file.
    *  
    *  2) No right is granted to use any trade name, trademark, or 
    *     logo of Broadcom Corporation.  The "Broadcom Corporation" 
    *     name may not be used to endorse or promote products derived 
    *     from this software without the prior written permission of 
    *     Broadcom Corporation.
    *  
    *  3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR
    *     IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED
    *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
    *     PURPOSE, OR NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT 
    *     SHALL BROADCOM BE LIABLE FOR ANY DAMAGES WHATSOEVER, AND IN 
    *     PARTICULAR, BROADCOM SHALL NOT BE LIABLE FOR DIRECT, INDIRECT,
    *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
    *     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    *     GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    *     BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
    *     OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
    *     TORT (INCLUDING NEGLIGENCE OR OTHERWISE), EVEN IF ADVISED OF 
    *     THE POSSIBILITY OF SUCH DAMAGE.
    ********************************************************************* */
#ifndef	__EXCEPTION_H__
#define __EXCEPTION_H__

#ifdef CFG_ARMV8_AARCH64
/*  *********************************************************************
    *  Exception frame definitions.
    ********************************************************************* */

/*
 * The exception frame is divided up into pieces, representing the different
 * parts of the processor that the data comes from:
 *
 * General Purpose Regs: 31 dwords
 * Stack pointer : 1 dwords
 * Special register: 4 words + 2 dwords
 * Total size:  72 words(must be multiple of 8 bytes)
 */

#define EXCEPTION_SIZE  288

/* The trap structure is defined here as offsets for assembly */
#define	TR_TYPE		0x00
#define	TR_ESR		0x04
#define	TR_CPSR		0x08
#define	TR_SPSR		0x0c
#define	TR_ELR		0x10
#define	TR_FAR		0x18
#define	TR_REGS		0x20
#define	TR_REG(n)	(TR_REGS + (n) * REGSIZE)
#define	TR_LR		TR_REG(30)
#define	TR_SP		TR_REG(31)


#define	TRAP_T_SIZE	EXCEPTION_SIZE	

#ifndef	__ASSEMBLER__

#include <lib_types.h>

typedef struct _trap_struct {
	uint32_t	type;
	uint32_t	esr;  /* exception syndrome reg */
	uint32_t	cpsr;
	uint32_t	spsr;
	uint64_t	elr;  /* exception link reg */
	uint64_t	far;  /* fault address reg */
	uint64_t	x0;
	uint64_t	x1;
	uint64_t	x2;
	uint64_t	x3;
	uint64_t	x4;
	uint64_t	x5;
	uint64_t	x6;
	uint64_t	x7;
	uint64_t	x8;
	uint64_t	x9;
	uint64_t	x10;
	uint64_t	x11;
	uint64_t	x12;
	uint64_t	x13;
	uint64_t	x14;
	uint64_t	x15;
	uint64_t	x16;
	uint64_t	x17;
	uint64_t	x18;
	uint64_t	x19;
	uint64_t	x20;
	uint64_t	x21;
	uint64_t	x22;
	uint64_t	x23;
	uint64_t	x24;
	uint64_t	x25;
	uint64_t	x26;
	uint64_t	x27;
	uint64_t	x28;
	uint64_t	x29; /* frame pointer */
	uint64_t	x30; /* procedure link reg */
	uint64_t	sp;
} trap_t;
#endif

#else

/*  *********************************************************************
    *  Exception frame definitions.
    ********************************************************************* */

/*
 * The exception frame is divided up into pieces, representing the different
 * parts of the processor that the data comes from:
 *
 * Int Regs:	Words 16
 * Special regs:Words 4
 * Total size:  20 words
 */

#define EXCEPTION_SIZE  80

/* ARM trap handling */

/* Trap types defined by ARM (see arminc.h) */

/* Trap locations in lo memory */
#define	TRAP_STRIDE	4
#define FIRST_TRAP	TR_RST
#define LAST_TRAP	(TR_FIQ * TRAP_STRIDE)

#if defined(__ARM_ARCH_4T__)
#define	MAX_TRAP_TYPE	(TR_FIQ + 1)
#elif defined(__ARM_ARCH_7M__)
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

#define	TRAP_T_SIZE	EXCEPTION_SIZE

#ifndef	__ASSEMBLER__

#include <lib_types.h>
#include <bcmtypes.h>


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
} trap_t;

#endif
#endif

#ifndef	__ASSEMBLER__

extern void _exc_restart(void);
extern unsigned long _exc_set_trap(void* hook);
void cfe_exception(trap_t *tr);
void cfe_setup_exceptions(void);
void cfe_boot_second_cpu(unsigned long vector);
#endif
#endif
