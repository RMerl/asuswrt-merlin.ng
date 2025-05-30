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
 * $Id: startarm-cr4.S 821234 2023-02-06 14:16:52Z $
 */

#include <arminc.h>
#include <sbhndarm.h>
#include <hnd_armtrap.h>
#include <hnd_debug.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <bcmdevs.h>
#ifdef SOCRAM
#include <sbsocram.h>
#endif

/* threadx runs in SVC mode */
#define	ARM_DEFAULT_MODE	PS_SVC

/* IRQ stack size if using a dedicated IRQ stack */
#ifndef IRQ_STACK_SIZE
#define IRQ_STACK_SIZE		512
#endif	/* IRQ_STACK_SIZE */

/* SVC mode stack size, used during system init and by ThreadX, and optionally shared with IRQs */
#ifndef	 SVC_STACK_SIZE
#define	 SVC_STACK_SIZE		512
#endif /* SVC_STACK_SIZE */

/* SVC mode stack guard size for stack overflow checking */
#ifndef SVC_STACK_GUARD_SIZE
#define SVC_STACK_GUARD_SIZE	0	/* Disable */
#endif /* SVC_STACK_GUARD_SIZE */

/* SVC mode stack prefill pattern, same as used for ThreadX stack checking */
#define SVC_STACK_FILL		0xefefefef

	.text

#ifdef	__thumb__
	.thumb
#endif

/* Entry point */
	.section .text.startup
	.global startup
	.global _tx_thread_context_save
	.global __tx_irq_processing_return
	.global threadx_isr
	.global _tx_thread_context_restore

startup:

/* #if defined(__ARM_ARCH_7R__) */

excvtbl:
	/* Total 8 entries */
#ifndef	__thumb__
	.arm
	/* Cortex R4 in ARM mode. */
	ldr	pc,=tr_rst	/* 0 - reset */
	ldr	pc,=tr_und	/* 4 - undefined instruction */
	ldr	pc,=tr_swi	/* 8 - software interrupt */
	ldr	pc,=tr_iab	/* 0xc - prefetch abort */
	ldr	pc,=tr_dab	/* 0x10 - data abort */
	ldr	pc,=tr_bad	/* 0x14 - reserved */
	ldr	pc,=tr_irq	/* 0x18 - external interrupt */
	ldr	pc,=tr_fiq	/* 0x1c - fast interrupt */
#else
	.thumb
	.syntax unified
	/* Cortex R4 in thumb mode. */
	b.w	tr_rst		/* 0 - reset */
	b.w	tr_und		/* 4 - undefined instruction */
	b.w	tr_swi		/* 8 - software interrupt */
	b.w	tr_iab		/* 0xc - prefetch abort */
	b.w	tr_dab		/* 0x10 - data abort */
	b.w	tr_bad		/* 0x14 - reserved */
	b.w	tr_irq		/* 0x18 - external interrupt */
	b.w	tr_fiq		/* 0x1c - fast interrupt */
#endif

/* Reset handler */
#ifdef	__thumb__
	.thumb
#endif

#ifdef FLOPS_SUPPORT

excvtbl_end:

	.section .text.startup_flops
	.global startup_flops
startup_flops:

# now this place is labeled as dup_excvtbl, where the instructions from
# excvtbl are copied using flopscopy script.
dup_excvtbl:

# 32 bytes for duplicate execvtbl and rest till 0x78 reserved.
#if !defined(RAMBASE)
.skip 0x60, 0xFA
#else /* RAMBASE */
.skip 0x54, 0xFA

# Place debug_info at a known location, RAMBase+0x878
debug_info_ptr:
	.global hnd_debug_info
	.word	HND_DEBUG_PTR_PTR_MAGIC		/* 'DBPP' */
	.word	hnd_debug_info
	.word	RAMBASE
#endif /* RAMBASE */

#endif /* FLOPS_SUPPORT */

tr_rst:
#ifdef FLOPS_SUPPORT
# code to copy flops from location labeled dup_excvtbl, to vectors.
	ldr r8, =dup_excvtbl
	mov r9, #0x00
	ldm.w r8!, {r0-r7}
	stm.w r9!, {r0-r7}
#endif /* FLOPS_SUPPORT */

#ifdef	BCMDBG
	ldr	r0,=0xbbadbadd
	mov	r1,r0
	mov	r2,r0
	mov	r3,r0
	mov	r4,r0
	mov	r5,r0
	mov	r6,r0
	mov	r7,r0
	mov	r8,r0
	mov	r9,r0
	mov	r10,r0
	mov	r11,r0
	mov	r12,r0
	mov	r13,r0
	mov	r14,r0
#endif	/* BCMDBG */
	mrs	r0,cpsr
	mov	r1,#(PS_MM)
	bic	r0,r0,r1

#ifdef FIQMODE
	/* Switch to FIQ mode and make sure interrupts are disabled */
	mov	r1,#(PS_I | PS_F | PS_FIQ)
	orr	r0,r0,r1
	msr	cpsr,r0
	ldr	sp, =fiq_stack_end
	mrs	r0,cpsr
	mov	r1,#(PS_MM)
	bic	r0,r0,r1
#endif

#ifdef IRQMODE
	/* Switch to IRQ mode and make sure interrupts are disabled */
	mov 	r1,#(PS_I | PS_F | PS_IRQ)
	orr	r0,r0,r1
	msr	cpsr,r0
	ldr	sp,=irq_stack_end
	mrs	r0,cpsr
	mov	r1,#(PS_MM)
	bic	r0,r0,r1
#endif

	/* Switch to default mode and make sure interrupts are disabled */
	mov	r1,#(PS_I | PS_F | ARM_DEFAULT_MODE)
	orr	r0,r0,r1
	msr	cpsr,r0

	/* Go to the common setup routine! */
	ldr	r0,=setup
	bx	r0

/* Note: hnd_rte_arm.c for EXT_CBALL expects each trap handler to be 16 bytes */
LEAF(__traps)

/* Undefined instruction exception handler */
tr_und:
	srsdb	sp!, #ARM_DEFAULT_MODE	/* use default mode stack */
	cps	#ARM_DEFAULT_MODE

	/* now cr4 is in system mode */
	/* lr has the value before the exception, push it to stack */
	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}	/* save r0-r7 to the stack */
	eor	r0,r0,r0
	add	r0,r0,#TR_UND
	b	trap

/* Software interrupt exception handler */
tr_swi:
	srsdb	sp!, #ARM_DEFAULT_MODE	/* use default mode stack */
	cps	#ARM_DEFAULT_MODE

	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}
	eor	r0,r0,r0
	add	r0,r0,#TR_SWI
	b	trap

/* Prefetch abort exception handler */
tr_iab:
	/* adjust lr to the address of abort insturction fetch */
	mov 	sp,lr
	sub	sp,sp,#4
	mov 	lr,sp
	srsdb	sp!, #ARM_DEFAULT_MODE	/* use default mode stack */
	cps	#ARM_DEFAULT_MODE

	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}
	eor	r0,r0,r0
	add	r0,r0,#TR_IAB
	b	trap

/* Data abort exception handler */
tr_dab:
	/* adjust lr to the address of aborted data fetch */
	mov	sp,lr
	sub	sp,sp,#8
	mov	lr,sp
	srsdb	sp!, #ARM_DEFAULT_MODE	/* use default mode stack */
	cps	#ARM_DEFAULT_MODE

	push	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push	{r0-r7}
	eor	r0,r0,r0
	add	r0,r0,#TR_DAB
	b	trap

tr_bad: /* reserved */
	mov 	sp,lr
	sub	sp,sp,#4
	mov 	lr,sp
	srsdb	sp!, #ARM_DEFAULT_MODE	/* use default mode stack */
	cps	#ARM_DEFAULT_MODE

	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}
	eor	r0,r0,r0
	add	r0,r0,#TR_BAD
	b	trap

/* Interrupt handler */
tr_irq:
/* threadx interrupt handler */
	push	{r0}
	ldr	r0,=tr_threadx_arm_irq
	bx	r0

	/* this block of code runs in ARM mode as threadx context/restore
 	 * functions are only supported in ARM mode
 	 */
	.align 2
	.arm
tr_threadx_arm_irq:
	pop	{r0}
	b	_tx_thread_context_save
__tx_irq_processing_return:
	mov	r0, sp
	blx	threadx_isr
	b	_tx_thread_context_restore

	/* swich back to THUMB mode */
	.thumb

/* Fast interrupt handler */
tr_fiq:
#ifdef FIQ_USE_COMMON_TRAP_HDLR
	mov 	sp,lr
	sub	sp,sp,#4
	mov 	lr,sp
	srsdb	sp!, #ARM_DEFAULT_MODE	/* use default mode stack */
	cpsID	if,#ARM_DEFAULT_MODE	/* disable IRQ/FIQ */

	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}
	eor	r0,r0,r0
	add	r0,r0,#TR_FIQ
	b	trap
#elif defined(FIQMODE)
	push	{r0-r7,lr}
	subs	r0,lr,#4
	mov	r1,lr
	mov	r2,sp
	mrs	r3,cpsr

	/* If trap_hook is null, go to the end */
	ldr	r4,=fiqtrap_hook
	ldr	r4,[r4]
	cmp	r4,#0
3:	beq	3b
	/* else call (*fiqtrap_hook)(epc,lr,sp,cpsr) */
	blx	r4
	pop	{r0-r7,lr}
	subs	pc,lr,#4
#endif /* FIQ_USE_COMMON_TRAP_HDLR*/

/* Generic exception handler */
trap:
	/*
	 * construt the trap structure in system mode stack (SVC mode for threadx)
	*/
	/* save trap type, epc, cpsr and spsr */
	mov 	r4, sp	/* sp is now at r0 */
	add	r4,r4,#64
	ldmia	r4!, {r1,r3} /* r1<-epc, r3<-spsr */
	mrs	r2, cpsr
	push	{r0,r1,r2,r3}	/* trap type(r0), epc(r1), cpsr,spsr */

	/* fix the value of pc in trap structure */
	sub	r4,r4,#12
	str	r1,[r4]	/* save the value or epc as r15 into the stack */
	and	r1,r3,#PS_F	/* save old FIQ enable flag */

	/* save r8 - r13 */
	mov	r7,sp	/* sp is at the being of the trap structure now */
	add	r7,r7,#TRAP_T_SIZE + 8	/* sp value before the exception */
	mov	r6,r12
	mov	r5,r11
	mov	r4,r10
	mov	r3,r9
	mov	r2,r8
	add	sp,sp,#72 /* move sp to r14 */
	push	{r2-r7}	/* save r8-r13 to stack */

	sub	sp,sp,#48 /* move sp back to the top of trap structure */

	cmp	r0,#TR_IRQ
	bne	2f
	cmp	r1,#PS_F	/* IRQ is disabled before entering exception */
	beq	2f
	cpsIE	f		/* Reenable FIQ for IRQ exception */
2:

	/* If trap_hook is null, go to the end */
	ldr	r4,=trap_hook
	ldr	r4,[r4]
	cmp	r4,#0
1:	beq	1b
	/* else call (*trap_hook)(trap_t) */
	mov	r0,sp
	blx	r4

/* Restore the state from the trap_t */
rfe:
	/* pop r8-r14 */
	cpsID	if	/* Disable IRQ/FIQ during stack manipulation */
	add	sp,sp,#48
	pop	{r0-r6} /* r8 - r14 */
	mov	r8,r0
	mov	r9,r1
	mov	r10,r2
	mov	r11,r3
	mov	r12,r4
	mov	lr,r6	/* restore lr */

	sub	sp,sp,#60 /* move sp to point to r0 */
	pop	{r0-r7}

	/* move sp to point to epc and spsr saved by srs */
	add	sp,sp,#32

	/* Restore cpsr, sp and return */
	rfeia	sp!	/* done! */
END(__traps)

	.thumb
/* Enable ARM master interrupt */
FUNC(enable_arm_irq)
	cpsie	i
	bx	lr
END(enable_arm_irq)

FUNC(enable_arm_dab)
        cpsie   a
        bx      lr
END(enable_arm_dab)

FUNC(enable_arm_fiq)
	cpsie	f
	bx	lr
END(enable_arm_fiq)

FUNC(disable_arm_irq)
	cpsid	i
	bx	lr
END(disable_arm_irq)

FUNC(disable_arm_fiq)
	cpsid	i
	bx	lr
END(disable_arm_fiq)

/* Cycle & performance counters */
FUNC(get_arm_perfcount_enable)
	mrc	p15,0,r0,c9,c12,1
	bx	lr
END(get_arm_perfcount_enable)

FUNC(set_arm_perfcount_enable)
	mcr	p15,0,r0,c9,c12,1
	bx	lr
END(set_arm_perfcount_enable)

FUNC(set_arm_perfcount_disable)
	mcr	p15,0,r0,c9,c12,2
	bx	lr
END(set_arm_perfcount_disable)

FUNC(get_arm_perfcount_sel)
	mrc	p15,0,r0,c9,c12,5
	bx	lr
END(get_arm_perfcount_sel)

FUNC(set_arm_perfcount_sel)
	mcr	p15,0,r0,c9,c12,5
	bx	lr
END(set_arm_perfcount_sel)

FUNC(get_arm_perfcount_event)
	mrc	p15,0,r0,c9,c13,1
	bx	lr
END(get_arm_perfcount_event)

FUNC(set_arm_perfcount_event)
	mcr	p15,0,r0,c9,c13,1
	bx	lr
END(set_arm_perfcount_event)

FUNC(get_arm_perfcount)
	mrc	p15,0,r0,c9,c13,2
	bx	lr
END(get_arm_perfcount)

FUNC(set_arm_perfcount)
	mcr	p15,0,r0,c9,c13,2
	bx	lr
END(set_arm_perfcount)

FUNC(enable_arm_cyclecount)
	mrc	p15,0,r1,c9,c12,0
	ldr	r2,=1
	orr	r1,r1,r2
	mcr	p15,0,r1,c9,c12,0	/* Set enable bit in PMNC */
	ldr	r1,=0x80000000
	mcr	p15,0,r1,c9,c12,1
	bx	lr
END(enable_arm_cyclecount)

FUNC(disable_arm_cyclecount)
	ldr	r1,=0x80000000
	mcr	p15,0,r1,c9,c12,2
	bx	lr
END(disable_arm_cyclecount)

FUNC(get_arm_cyclecount)
	.type get_arm_cyclecount, %function
	mrc	p15,0,r0,c9,c13,0
	bx	lr
END(get_arm_cyclecount)

FUNC(set_arm_cyclecount)
	mcr	p15,0,r0,c9,c13,0
	bx	lr
END(set_arm_cyclecount)

FUNC(get_arm_data_fault_status)
	mrc     p15,0,r0,c5,c0,0
	bx      lr
END(get_arm_data_fault_status)

FUNC(get_arm_data_fault_address)
	mrc     p15,0,r0,c6,c0,0
	bx      lr
END(get_arm_data_fault_address)

FUNC(get_arm_instruction_fault_status)
	mrc     p15,0,r0,c5,c0,1
	bx      lr
END(get_arm_instruction_fault_status)

FUNC(get_arm_instruction_fault_address)
	mrc     p15,0,r0,c6,c0,2
	bx      lr
END(get_arm_instruction_fault_address)

/* Interval timer expiration time */
FUNC(get_arm_inttimer)
	ldr	r1,=arm_regs
	ldr	r1,[r1,#0]
	ldr	r2,=ARMCR4_INTTIMER
	ldr	r0,[r1,r2]
	bx	lr
END(get_arm_inttimer)

FUNC(set_arm_inttimer)
	ldr	r1,=arm_regs
	ldr	r1,[r1,#0]
	ldr	r2,=ARMCR4_INTTIMER
	str	r0,[r1,r2]
	bx	lr
END(set_arm_inttimer)

/* Interval timer interrupt mask */
FUNC(get_arm_intmask)
	ldr	r1,=arm_regs
	ldr	r1,[r1,#0]
	ldr	r2,=ARMCR4_INTMASK
	ldr	r0,[r1,r2]
	bx	lr
END(get_arm_intmask)

FUNC(set_arm_intmask)
	ldr	r1,=arm_regs
	ldr	r1,[r1,#0]
	ldr	r2,=ARMCR4_INTMASK
	str	r0,[r1,r2]
	bx	lr
END(set_arm_intmask)

/* Interval timer interrupt status */
FUNC(get_arm_intstatus)
	ldr	r1,=arm_regs
	ldr	r1,[r1,#0]
	ldr	r2,=ARMCR4_INTSTATUS
	ldr	r0,[r1,r2]
	bx	lr
END(get_arm_intstatus)

FUNC(set_arm_intstatus)
	ldr	r1,=arm_regs
	ldr	r1,[r1,#0]
	ldr	r2,=ARMCR4_INTSTATUS
	str	r0,[r1,r2]
	bx	lr
END(set_arm_intstatus)

/* Interval timer interrupt status */
FUNC(get_arm_firqstatus)
	ldr	r1,=arm_regs
	ldr	r1,[r1,#0]
	ldr	r2,=ARMCR4_FIQRSTATUS
	ldr	r0,[r1,r2]
	bx	lr
END(get_arm_firqstatus)

FUNC(set_arm_firqstatus)
	ldr	r1,=arm_regs
	ldr	r1,[r1,#0]
	ldr	r2,=ARMCR4_FIQRSTATUS
	str	r0,[r1,r2]
	bx	lr
END(set_arm_firqstatus)

/*
 * Setup the trap handler.
 */
FUNC(hnd_set_trap)
	ldr	r2,=trap_hook
	ldr	r1,[r2]
	str	r0,[r2]
	mov	r0,r1
	bx	lr
END(hnd_set_trap)

#ifdef FIQMODE
FUNC(hnd_set_fiqtrap)
	ldr	r2,=fiqtrap_hook
	ldr	r1,[r2]
	str	r0,[r2]
	mov	r0,r1
	bx	lr
END(hnd_set_fiqtrap)
#endif

/*
 * Turn remap off and then jump to an given address
 */
FUNC(arm_jumpto)
#ifndef FLOPS_SUPPORT
	ldr	r2,=chiptype
	ldr	r1,=arm_wrap
	ldr	r2,[r2]
	ldr	r1,[r1]
	tst	r2,r2
	bne	1f

	/* For SB chips, get sbtmstatelow and shift it */
	ldr	r2,=SBTMSTATELOW
	ldr	r3,[r1,r2]
	ldr	r5,=SBTML_SICF_SHIFT
	_LSR_	r3,r3,r5
	b	2f

	/* For AI chips, its just the ioctrl register */
1:	ldr	r2,=AI_IOCTRL
	ldr	r3,[r1,r2]
	eor	r5,r5,r5				/* No shift needed in AI */

2:	ldr	r4,=SICF_REMAP_MSK
	bic	r3,r3,r4
	ldr	r4,=SICF_REMAP_NONE
	orr	r3,r3,r4
	_LSL_	r3,r3,r5
3:	str	r3,[r1,r2]
	/* read back to wait for the posted write */
	ldr	r3,[r1,r2]
	nop
	nop
	/* Jump to addr in r0 */
#endif
	bx	r0
END(arm_jumpto)

/* Embedded nvram section */
#ifndef CONFIG_XIP
	/* Record the size of the binary */
	.org	BISZ_OFFSET

	.word	BISZ_MAGIC
	.word	text_start
	.word	text_end
	.word	data_start
	.word	data_end
	.word	bss_start
	.word	bss_end
#if defined(BCMHOSTVARS)
	.extern	nvram_array
#endif
	.word	0

	/* Embedded NVRAM */
	.global	embedded_nvram
embedded_nvram:
	.fill	0x100,4,~0x48534C46	/* 'FLSH' */
	.long   0x4c5a4d41              /* LZMA NVRAM Supported */
#endif	/* !CONFIG_XIP */

/*
 * Run-time initialization:
 *	- Initialize MPU
 *	- turn off pll ASAP, it may come up without knowing the xtal freq
 *	- wait till OTP is ready, it is used to repair/patch the RAM
 *	- copy the data section to RAM if running from ROM (XIP)
 *	- copy the code and data to RAM if booting from FLASH and run from RAM
 *	- set up stack for C code
 */
FUNC(setup)
	/* Initialize MPU */
	bl	mpu_init
	/* Figure out if we have an SB or AI chip */
	ldr	r0,=SI_ENUM_BASE_DEFAULT		/* r0:	core regs SI base address */
	ldr	r1,[r0,#CC_CHIPID]
	ldr	r2,=(CID_ID_MASK & ~0x7)
	and	r2,r2,r1

/*
 * Find arm core
 * Input:
 *	r0 - chipc base address.
 *	r1 - chip id
 * Output:
 *	r0 - arm regs
 *	r8 - arm regs
 *	r9 - arm wrapper (sbconfig or ai dmp)
 *      r10 - socram regs
 *	r11 - arm core index
 * Changed:
 *	r0, r1, r2, r3, r4, r5, r8, r9, r10, r11
 */
findcore:
#ifdef SOCRAM
	mov	r5,#0
	mov	r8,r5
	mov	r10,r5
#endif
	ldr	r1,=CC_EROMPTR
	ldr	r1,[r0,r1]			/* r1:	erom pointer */
1:	ldr	r2,[r1]				/* r2:	erom entry */
	ldr	r3,=(ER_TAG | ER_VALID)
	and	r3,r3,r2
	cmp	r3,#(ER_END | ER_VALID)		/* Is this the END marker? */
	beq	panic				/* Yes: done and did not find cr4 */

	cmp	r3,#(ER_CI | ER_VALID)
	beq	2f

	add	r1,r1,#4			/* Next erom entry */
	b	1b

panic:	b	.

	/* This code is taking some liberties. It is possible for it to
	 * mis-parse what would be a correct EROM (It will certanly
	 * misbehave if the EROM is not correct, but that shouldn't
	 * happen since a bad EROM can be detected long before tapeout).
	 * A bulletproof version would have to always check for the
	 * valid bit ER_VALID, and it would be more carefull to recognize
	 * Address Descriptors with 64bit addresses or sizes other
	 * that 4/16/64K so it can skip those entries and not be possibly
	 * fooled by them.
	 */

	/* Found a CIA */
2:	add	r1,r1,#8			/* Skip both CIA & CIB */
	ldr	r3,=CIA_CID_MASK
	and	r3,r3,r2
	ldr	r4,=(ARMCR4_CORE_ID << CIA_CID_SHIFT)
	cmp	r3,r4
#ifdef SOCRAM
	bne	3f
#else
	bne	1b
#endif

#ifdef SECOND_ARM
	/* The Apps core is at second entry */
	cmp	r5,#1
	mov	r5,#1
	bne	1b
	/* Save ARM core index to r11 */
	mov	r11,r5
#endif
	/* Found arm, get regs and wrapper and save them */
	bl	ai_get_slave
	mov	r8,r0

	bl	ai_get_wrapper
	mov	r9,r0
#ifdef SOCRAM
	b	4f

3:	ldr	r4,=(SOCRAM_CORE_ID << CIA_CID_SHIFT)
	cmp	r3,r4
	bne	1b

	/* Found socram, get corerev, regs and wrapper and save them */
	sub	r3,r1,#4			/* Back up pointer to CIB */
	ldr	r3,[r3]				/* Get CIB */
	ldr	r4,=CIB_REV_MASK
	and	r3,r3,r4
	ldr	r4,=CIB_REV_SHIFT
	_LSR_	r3,r3,r4
	mov	r12,r3				/* r12:	Core rev */

	bl	ai_get_slave
	mov	r10,r0

4:	mov 	r3, #0
	cmp	r8,r3
	beq	1b
	cmp	r10,r3
	beq	1b
chk_cores:
	mov	r0,r10
	tst	r0,r0
	beq	panic

	mov	r0,r8
	tst	r0,r0
	beq	panic
#endif
/*
 * Check if flash or ROM is remapped.
 * Input:
 *	r9 - arm registers
 * Changed:
 *	r3, r4
 */
isflrom:
	mov	r1,r9				/* Get arm wrapper */

	/* Get ioctrl register */
1:	ldr	r3,=AI_IOCTRL
	ldr	r3,[r1,r3]

	/* Figure out where we are running from */
#ifndef FLOPS_SUPPORT
	/* legacy code using remap method */
2:	ldr	r4,=SICF_REMAP_MSK
	and	r3,r3,r4
	cmp	r3,#SICF_REMAP_NONE
	beq	inram
#else /* FLOPS_SUPPORT */
	/* for flops support, there is no remap. So, we figure out
	 * if we are in ROM by checking the pc address to be less
	 * than atcm ram base.
	 */
	bl	cr4_mem_info

	/* here r6 contains atcm ram base address */
	/* r15 - r6 & check carry for jump */
	mov r1, r15
	cmp r1, r6

	/* if positive, remain in ram */
	bpl inram

#endif /* FLOPS_SUPPORT */

	/* Go take care of the flash/rom cases */
	bl	inflrom

/*
 * The data section is in RAM now so we can read/write memory!
 */
inram:
	/* Save arm regs & wrapper */
	TRACE(0)
	mov	r0,r8
	ldr	r1,=arm_regs
	str	r0,[r1]
	mov	r0,r9
	ldr	r1,=arm_wrap
	str	r0,[r1]

#ifdef SOCRAM
	/* Save socram regs and corerev */
	mov	r0,r10
	ldr	r1,=socram_regs
	str	r0,[r1]
	mov	r0,r12
	ldr	r1,=socram_rev
	str	r0,[r1]
#endif
#ifdef SECOND_ARM
	/* Save ARM core index */
	mov	r0,r11
	ldr	r1,=_armcr4_idx
	str	r0,[r1]
#endif
/* get memory info. So here onwards,
 *	r5 - ram size [atcm + btcm]
 *	r6 - atcm ram base address
 *	r7 - RAM bottom/stack start
 */
	bl	cr4_mem_info

	/* Save memory size */
	ldr	r1,=_memsize
	str	r5,[r1]

	/* save atcm ram base address */
	ldr	r1,=_atcmrambase
	str	r6,[r1]

	/* save ram bottom [essentially stk bottom] */
	ldr	r1,=_rambottom
	str	r7,[r1]

	/* r5 = r7 [instructions below were using r5. so keeping them intact] */
	mov r5, r7

/* Clear BSS */
clearbss:
	TRACE(3)
	ldr	r0,=bss_start
	ldr	r1,=bss_end
	mov	r2,#0
1:	stmia	r0!,{r2}
	cmp	r1,r0
	bhi	1b
/*
 * Setup stack pointer.
 * Input:
 *	r5 - memsize.
 */
setsp:
	/* Set up stack pointer */
	TRACE(2)
#if defined(BCM_BOOTLOADER)
#if defined(BCMTRXV2)
	/* Space for TRXV2 header on top of RAM */
	sub	r5,r5,#36
	sub	r5,r5,#4
	mov	sp,r5
#endif /* defined(BCMTRXV2) */

#ifdef DL_NVRAM
	/* bootloader supports nvram dl: shift stack pointer beyond nvram */
	sub	r7,r7,#4
	ldr	r3,=_dlvarsz
	str	r7,[r3]

	ldr	r0,=DL_NVRAM
	sub	r5,r5,r0

	ldr	r3,=_dlvars
	str	r5,[r3]
	mov	sp,r5
#endif /* DL_NVRAM */

#else /* !defined(BCM_BOOTLOADER) */

#if defined(BCMUSBDEV_ENABLED) && defined(BCMTRXV2)
	/* Space for TRXV2 header on top of RAM */
	sub	r5,r5,#4
	sub	r5,r5,#36
	mov	sp,r5

	/* check if there is V1 style _varsz */
	sub	r7,r7,#4	/* _varsz */
	ldr	r5,[r7]
	mvn	r4,r5
	mov	r3,#16
	_ROR_	r5,r5,r3
	cmp	r4,r5
	bne	donewithnvram

	mov	sp,r7
	b	vars_present
#endif /* BCMUSBDEV_ENABLED && BCMTRXV2 */

	sub	r5,r5,#4	/* _varsz */
	mov	sp,r5
	/* esp for sdio case */
#if defined(BCMHOSTVARS)
	/*
	 * Common for usb and sdio for non-TRXV2 case.
	 * Check if the host wrote nvram vars at the end of memory.  If so,
	 * record their location/size and start the stack pointer below them.
	 * If valid, the vars length is in words (4 bytes) a multiple of 4 encoded in the last
	 * word of memory as (~len << 16 | len).  The vars precede the length.
	 * If invalid, the stack pointer starts at end of memory minus 4.
	 * During the init sequence of SDIO or PCIe firmware, the last 4 bytes of memory are
	 * overwritten with the address of the sdpcm_shared / pcie_shared structure.
	 */
	ldr	r5,[sp]
	mvn	r4,r5
	mov	r3,#16
	_ROR_	r5,r5,r3
	cmp	r4,r5
	bne	donewithnvram
vars_present:
	/* Vars present */
	/* calculate and set _varsz, keep size in r4 */
	_LSR_	r4,r4,#16
	_LSL_	r4,r4,#2
	ldr	r5,=_varsz
	str	r4,[r5]
	/* calculate start of NVRAM, keep in r3 */
	mov	r3,sp
	sub	r3,r3,r4
	/* point _vars to nvram_array */
	ldr	r6,=nvram_array
	ldr	r5,=_vars
	str	r6,[r5]
	/* check _varsz boundaries */
	cmp	r4,#0
	beq	donewithnvram
	/*
	 * Should also ensure r4 <= NVRAM_ARRAY_MAXSIZE, but don't have access to it
	 * here. Therefore, this is taken care of in rtecdc.c instead.
	 *
	 * Now copy NVRAM area to target (nvram_array)
	 */

	ldr r5,=nvram_array
	add r6, r5, r4
copy_vars:
	ldmia r3!, {r7}
	stmia r5!, {r7}
	cmp r6, r5
	bgt copy_vars

donewithnvram:
	/* move SP to free up last word in memory */
	sub	sp,sp,#4
#endif /* BCMHOSTVARS */
#endif /* !BCM_BOOTLOADER */

#ifdef THREADX_BTCM_STACK
	sub	sp, sp, #(HND_STACK_SIZE)
#endif /* THREADX_BTCM_STACK */
	mov     r0,sp
	lsr     r0,r0,4
	lsl     r0,r0,4
	mov     sp,r0

	/* set the stack bottom */
	mov	r0,sp
	ldr	r4,=_stackbottom
	str	r0,[r4]

	/* set the stack top */
	sub	r1,r0,#SVC_STACK_SIZE
	ldr	r4,=_stacktop
	str	r1,[r4]

	/* set the stack guard */
	sub	r1,r1,#SVC_STACK_GUARD_SIZE
	ldr	r4,=_stackguard
	str	r1,[r4]

	/* fill the stack and guard with a known pattern */
	ldr     r4,=SVC_STACK_FILL
1:
	stmia   r1!,{r4}
	cmp     r1, r0
	bne     1b

#ifdef IRQMODE_SHARED_STACK
	/* similar to IRQMODE but IRQ uses the same stack as the default mode */

	/* _stackbottom in r4 */
	mov	r4,r0

	/* initialize IRQ stack with same stack as default mode stack (from _stackbottom) */
	mrs	r0,cpsr
	mov	r1,#(PS_MM)
	bic	r0,r0,r1
	mov	r1,#(PS_I | PS_F | PS_IRQ)
	orr	r0,r0,r1
	msr	cpsr,r0
	mov	sp,r4

	/* switch back to default mode */
	mrs	r0,cpsr
	mov	r1,#(PS_MM)
	bic	r0,r0,r1
	mov	r1,#(PS_I | PS_F | ARM_DEFAULT_MODE)
	orr	r0,r0,r1
	msr	cpsr,r0
#endif	/* IRQMODE_SHARED_STACK */

	/* label it assembly-to-c */
call_main:
	/* lable it assembly-to-c */
	TRACE(0xA2C)

	ldr	r4,=enable_arm_cyclecount
	blx	r4
	ldr	r4,=c_main
	blx	r4

/* Halt in case we ever come out of c_main! */
rfm:	TRACE(0xbaad)
	b	.

END(setup)

/*
 * diable MPU
 * Input:
 *	None
 * Output:
 *	None
 * Changed:
 *	r0
 */
FUNC(disable_mpu)
	/* disable MPU */
	mrc p15, 0, r0, c1, c0, 0
	bic r0, r0, #0x1
	dsb
	isb
	mcr	p15,0,r0,c1,c0,0
	dsb
	isb

	bx lr
	nop
END(disable_mpu)

/*
 * enable MPU
 * Input:
 *	None
 * Output:
 *	None
 * Changed:
 *	r0
 */
FUNC(enable_mpu)
	/* enable mpu */
	mrc	p15,0,r0,c1,c0,0
	orr	r0,r0,#0x01
	dsb
	isb
	mcr	p15,0,r0,c1,c0,0
	dsb
	isb
	nop

	bx lr
	nop
END(enable_mpu)

/* input parameters to mpu_set_region */
/* r0 - region number	*/
/* r1 - region base		*/
/* r2 - region size		*/
/* r3 - region control  */

FUNC(mpu_set_region)
mpu_set_region:

	stmfd sp!,{r0,lr}
	bl disable_mpu
	ldmfd sp!,{r0}

	/* init MPU region */
	mcr p15, 0, r0, c6, c2, 0	/* region number */
	mcr p15, 0, r1, c6, c1, 0	/* region base */
	orr r2,r2,#0x01				/* enable memory region */
	mcr p15, 0, r2, c6, c1, 2	/* region size + enable*/
	mcr p15, 0, r3, c6, c1, 4 ; /* region control */

	bl enable_mpu

	ldmfd sp!,{pc}
	nop
END(mpu_set_region)

/* input parameters to disable_mpu_region */
/* r0 - region number	*/
FUNC(disable_mpu_region)
disable_mpu_region:

	stmfd sp!, {r0,lr}
	bl disable_mpu
	ldmfd sp!, {r0}

	mcr p15, 0, r0, c6, c2, 0	/* set region number */
	mrc p15, 0, r0, c6, c1, 2	/* read region size + enable*/
	and r0,r0,#0xFFFFFFFE		/* disable region */
	mcr p15, 0, r0, c6, c1, 2	/* write region size + enable*/

	bl enable_mpu

	ldmfd sp!, {pc}
	nop
END(disable_mpu_region)

/*
 * Initialize MPU with 2 Data regions
 * Region-0: 0x0000_0000 - 0xffff_ffff Device Shared Region		(4GB)
 * Region-1: 0x0000_0000 - 0x0fff_ffff Normal Non-Shared Region	(256MB)
 *
 * Changes:
 *	r1, r2

 * Note: No IRegions used as of now
 */
FUNC(mpu_init)

	mrc	p15,0,r0,c0,c0,4	/* get MPUIR co-processor reg */
	ubfx	r1,r0,#8,#8		/* No of DRegions */
	cmp	r1,#0			/* Regions implemented ? */
	beq	nompu			/* Skip mpu init */

	mov r2,lr

	bl disable_mpu

	mov lr,r2

	ldr	r4,=_mpudcfg		/* mpu cfg table */
	eor	r0,r0				/* DRegion index */
1:
	ldmia	r4!, {r1,r2,r3}
	cmp	r1,#0			/* Last entry has size=0 */
	beq	mpu_enab
	mcr	p15,0,r0,c6,c2,0	/* Select DRegion */
	mcr	p15,0,r1,c6,c1,2	/* Set size:enable */
	mcr	p15,0,r2,c6,c1,0	/* Set base addr */
	mcr	p15,0,r3,c6,c1,4	/* set C,B,TEX[2:0],AP */
	add	r0,#1
	b	1b

mpu_enab:
	/* Enable MPU */
	cmp	r0,#0			/* no Dregions? skip MPU enable */
	beq	nompu
	mov r2,lr
	bl enable_mpu
	mov lr,r2
nompu:
	bx lr
	nop

	/* MPU DRegion config table */
	.align 4
_mpudcfg:
	.word	0x3f		/* DRegion-0 size(4G) */
	.word	0x00000000	/* DRegion-0 base_addr(0x0000_0000) */
	.word	0x005		/* C:0 B:1 TEX[2:0]:0 AP:0x0 (NO) */
	.word	0xc02b		/* DRegion-1 size(4M), subreg 7,8 disable 0x0 - 0x300000  read/write enable */
	.word	0x00000000	/* DRegion-1 base_addr(0x0000_0000) */
#ifdef RTE_CACHED
	.word	0x332		/* for cache access:C:1,B:0, TEX[2:0]:110, write-through, no-write-allocate */
#else
	.word	0x30f		/* C:1 B:1 TEX[2:0]:1 AP:0x3 (RW) */
#endif
	.word	0xe129		/* DRegion-2 size(2M) subregion 1,6,7,8 disabled 0x4000 - 0x140000 read only */
	.word	0x00000000	/* DRegion-2 base_addr(0x0000_0000) */
	.word	0x60f		/* C:1 B:1 TEX[2:0]:1 AP:0x6 (RO) */
	.word	0x35		/* DRegion-3 size(128M) */
	.word	0x18000000	/* DRegion-3 base_addr(0x1800_0000) */
	.word	0x305		/* C:0 B:1 TEX[2:0]:0 AP:0x3 */
	.word	0x35		/* DRegion-4 size(128M) */
	.word	0xE8000000	/* DRegion-4 base_addr(0xE800_0000) */
	.word	0x305		/* C:0 B:1 TEX[2:0]:0 AP:0x3 */
	.word	0			/* DRegion table  END */
	.word	0
	.word	0
END(mpu_init)

/* Find Memory Size of different regions in CR4.
 * Input:
 *	r8 - arm registers
 *	r9 - arm corerev
 * Output:
 *	r5 - ram size [atcm + btcm]
 *	r6 - atcm ram base address
 *	r7 - RAM bottom/stack start
 */
FUNC(cr4_mem_info)

#ifdef SOCRAM
/*
 * Calculate memory size in bytes.
 * Input:
 *	r7 - (chip type << CID_TYPE_SHIFT)
 *	r10 - socram registers
 *	r12 - socram corerev
 * Output:
 *	r5 - memsize.
 */
memsz:
	/* Find ram size */
	TRACE(0x35)

	/* Can hardcode MEMSIZE in dongle makefile to save ~112 bytes */
#ifdef MEMSIZE
	ldr	r5,=MEMSIZE
#else /* !MEMSIZE */
	mov	r0,r12
	mov	r1,r10

	/* Find number of blocks */
	ldr	r2,=SR_COREINFO
	ldr	r2,[r1,r2]
	mov	r3,r2				/* r2:	coreinfo */
	ldr	r1,=SRCI_SRNB_MASK
	and	r3,r3,r1
	_LSR_	r3,r3,#SRCI_SRNB_SHIFT
	cmp	r0,#8
	TRACE(0x37)
	bge	3f
	/* Take the last bank out of the calculation if socram >= 3 */
	cmp	r0,#3
	blo	1f
	mov	r6,r2
	ldr	r1,=SRCI_LSS_MASK
	and	r6,r6,r1
	ldr	r1,=SRCI_LSS_SHIFT
	_LSR_	r6,r6,r1
	cmp	r6,#0
	beq	1f
	sub	r3,r3,#1
	/* Find block size */
1:	ldr	r1,=SRCI_SRBSZ_MASK
	and	r2,r2,r1
	add	r2,r2,#SR_BSZ_BASE
	mov	r4,#1
	_LSL_	r4,r4,r2
	_MUL_	r5,r4,r3
	/* Add the last bank to the total memory size if socram >= 3 */
	cmp	r0,#3
	blo	2f
	cmp	r6,#0
	beq	2f
	sub	r6,r6,#1
	add	r6,r6,#SR_BSZ_BASE
	mov	r4,#1
	_LSL_	r4,r4,r6
	add	r5,r5,r4
	/* Halt if no memory is found */
2:	cmp	r5,#0
	TRACE(0x36)
	beq	.
	b	6f

	/* corerev >= 8 memory size calculation */
3:
	TRACE(0x38)
	/* r3 are number of banks, r10 are the regs */
	ldr	r0,=0
	ldr	r5,=0
	ldr	r2,=SOCRAM_BANKIDX_REG
	ldr	r7,=SOCRAM_BANKINFO_REG
	ldr	r6,=SOCRAM_BANKINFO_SZMASK
	ldr	r4,=SOCRAM_BANKSIZE_SHIFT
4:
	mov	r1,r10
	str	r0,[r1, r2]
	ldr	r1,[r1, r7]
	and	r1,r6
	add	r1,#1
	_LSL_	r1,r1,r4
	add	r5,r1
	add	r0,#1
	cmp	r0,r3
	bne	4b
6:
	ldr	r6,=SOCRAM_BASE
	add	r7,r5,r6
	bx lr
#endif /* MEMSIZE */
#else
memsz:
	/* Find ram size */
	TRACE(1)

	mov	r1,r8

	/* Find number of blocks */
	ldr	r2,[r1,#ARMCR4_CORECAP]		/* r2:	corecapabilities */
	mov	r3,r2
	ldr	r4,=ARMCR4_TCBBNB_MASK
	and	r3,r3,r4
	_LSR_	r3,r3,#ARMCR4_TCBBNB_SHIFT	/* r3: TCBB banks */
	ldr	r4,=ARMCR4_TCBANB_MASK
	and	r2,r2,r4			/* r2: TCBA banks */
	mov r6, r2				/* r6 = r2 = # of atcm ram banks [non-rom] */
	add	r2,r2,r3			/* r2: Total number of banks */
	ldr	r0,=ARMCR4_MT_RAM
	mov	r5,#0
1:	str	r0,[r1,#ARMCR4_BANKIDX]		/* Select RAM bank i */
	ldr	r3,[r1,#ARMCR4_BANKINFO]	/* Read bank info */
	ldr	r4,=ARMCR4_BSZ_MASK
	and	r3,r3,r4
	add	r3,r3,#1
	ldr	r4,=ARMCR4_BSZ_MULT
	_MUL_	r3,r3,r4			/* r3: Bank size */
	add	r5,r5,r3
	add	r0,r0,#1				/* Incr i */
	cmp	r0,r2
	blt	1b

	/* Halt if no memory is found */
	cmp	r5,#0
	beq	panic
#ifdef MEMSIZE
	ldr	r5,=MEMSIZE
#endif /* MEMSIZE */
/* ###### donot modify r5 [_memsize] in this function further ###### */

membase:
	mov	r1, r8		/* r1= r8 = arm registers */
	mov r2, r6		/* r2 = r6, # of ATCM ram banks [non-rom] */
	mov r6, #0		/* r6=0, will contain sum of ATCM ram's */
	ldr	r0,=ARMCR4_MT_RAM
1:	str	r0,[r1,#ARMCR4_BANKIDX]		/* Select RAM bank i */
	ldr	r3,[r1,#ARMCR4_BANKINFO]	/* Read bank info */
	ldr	r4,=ARMCR4_BSZ_MASK
	and	r3,r3,r4
	add	r3,r3,#1
	ldr	r4,=ARMCR4_BSZ_MULT
	_MUL_	r3,r3,r4			/* r3: Bank size */
	add	r6,r6,r3
	add	r0,r0,#1				/* Incr i */
	cmp	r0,r2
	blt	1b

	/* HERE, r6 contains ATCM RAM sz */

	/* r2 = BTCM base address. */
	mrc p15, 0, r2, c9, c1, 0

	ldr	r1,=#0xFFFFF000 /* BTCM base mask */

	/* r2 = r2 & BTCM base mask */
	and r2, r2, r1

	/*	r2 = ATCM base address = BTCM base - [r6] */
	sub r2, r2, r6

membottom:
	/* RAM bottom = ATCMbase + total mem size */
	add r7, r5, r2

	/* r6 = r2 = atcm base address, so r2 can be freed */
	mov r6, r2

	/* return */
	bx lr
#endif
END(cr4_mem_info)

/*
 * Find the first slave address for a core in the AI EROM
 * Input:
 *	r1 - pointer to EROM after CIB
 * Output:
 *	r0 - slave address
 * Changed:
 *	r0, r1, r2
 */
FUNC(ai_get_slave)
1:	ldr	r0,[r1]
	ldr	r2,=(ER_TAG | ER_VALID)
	add	r1,r1,#4			/* Next erom entry */
	and	r2,r2,r0
	cmp	r2,#(ER_ADD | ER_VALID)
	bne	1b

2:	ldr	r2,=AD_ADDR_MASK
	and	r0,r0,r2
	bx	lr
END(ai_get_slave)

/*
 * Find the first wrapper address for a core in the AI EROM
 * Input:
 *	r1 - pointer to EROM after first slave ADD
 * Output:
 *	r0 - wrapper address
 * Changed:
 *	r0, r1, r2, r3
 */
FUNC(ai_get_wrapper)
1:	ldr	r0,[r1]
	ldr	r2,=(ER_TAG | ER_VALID)
	add	r1,r1,#4			/* Next erom entry */
	and	r2,r2,r0
	cmp	r2,#(ER_ADD | ER_VALID)
	bne	1b

	/* An address descriptor, is it a wrapper? */
	ldr	r2,=AD_ST_SWRAP			/* We test just for the SWRAP bit set, */
	tst	r0,r2				/*  that gets both Master and Slave */
	beq	1b				/*  wrappers. */

	/* It is, isolate address and return */
	ldr	r2,=AD_ADDR_MASK
	and	r0,r0,r2
	bx	lr
END(ai_get_wrapper)

/* Add console command 'tr' that causes a trap to test trap handling */
FUNC(traptest)
traptest:
	mov	r0, #0
	str	r5, [r0]		// <== test null pointer store exception)
	bx	lr
END(traptest)

#ifdef FLOPS_SUPPORT
/* section for flops support in case the startup is moved out, this section
 * fills the gap. And the cut/dd logic [Makerules] will copy the startup
 * binary in place of these.
 */
FUNC(startup_flops_dup_ram)
.skip 0x20, 0xEE
END(startup_flops_dup_ram)
#endif /* FLOPS_SUPPORT */

#ifdef RTE_CACHED
/*
 * Enable instruction and data cache
 */
FUNC(caches_on)
        /*Enable CACHE*/
	stmfd	sp!, {r0-r7, r9-r11, lr}
        /* Read System Control Register configuration data */
        mrc p15, 0, r1, c1, c0, 0
        /* instruction cache enable */
                ldr           r2,=(0x1<<12)
                orr          r1,r1,r2
        /* data cache enable */
                ldr           r2,=(0x1<<2)
                orr          r1,r1,r2
        dsb
	mov r0, #0
        /* Invalidate entire data cache */
        mcr p15, 0, r0, c15, c5, 0
        /* Invalidate entire instruction cache */
        mcr p15, 0, r0, c7, c5, 0
        /* enabled cache RAMs */
        mcr p15, 0, r1, c1, c0, 0
        isb
	ldmfd	sp!, {r0-r7, r9-r11, lr}
	mov	pc, lr
END(caches_on)
#endif /* RTE_CACHED */

	.data

/* Debug/Trace */
	DW(__watermark,	0xbbadbadd)

/* HSIC sdr debugging purpose */
#ifdef HSIC_SDR_DEBUG
	DW(__chipsdrenable,	0xbbadbadd)
	DW(__nopubkeyinotp,	0xbbadbadd)
	DW(__imagenotdigsigned,	0xbbadbadd)
	DW(__pubkeyunavail,	0xbbadbadd)
	DW(__rsaimageverify, 0xbbadbadd)
#endif

/* to save reset vec for CRC */
	DW(orig_rst, 0)

/* C trap handler */
	DW(trap_hook, 0)

/* chiptype */
	DW(chiptype, 1)

/* arm core regs and wrapper */
	DW(arm_regs, 0)
	DW(arm_wrap, 0)

/* stack top, bottom and guard */
	DW(_stackguard, 0)
	DW(_stacktop, 0)
	DW(_stackbottom, 0)

#ifdef SOCRAM
/* socram core regs and corerev */
	DW(socram_regs, 0)
	DW(socram_rev, 0)
#else
/* No socram core in cr4, init to -1 */
	DW(socram_regs, -1)
	DW(socram_rev, -1)
#endif

/* Data section */

        .section .data._memsize
	.global	_memsize
	.type	_memsize, %object
	.size _memsize, 4
	.align 2
_memsize:
	.word	0

    	.section .data._atcmrambase
	.global	_atcmrambase
	.type _atcmrambase, %object
	.size _atcmrambase, 4
	.align 2
_atcmrambase:
	.word	0xbbadbadd

    	.section .data._rambottom
	.global	_rambottom
	.type _rambottom, %object
	.size _rambottom, 4
	.align 2
_rambottom:
	.word	0xbbadbadd

        .section .data._armcr4_idx
	DW(_armcr4_idx, 0)

#if defined(BCMHOSTVARS)
        .section .data._vars
	DW(_vars, 0)

        .section .data._varsz
	DW(_varsz, 0)
#endif

#ifdef DL_NVRAM
        .section .data._dlvars
	DW(_dlvars, 0)

        .section .data._dlvarsz
	DW(_dlvarsz, 0)
#endif /* DL_NVRAM */

#ifdef FIQMODE
        .balign 16
fiq_stack:
        .rept 96
        .byte 0
        .endr
fiq_stack_end = .
/* C fiq trap handler */
	DW(fiqtrap_hook, 0)
#endif

#if defined(IRQMODE) && !defined(IRQMODE_SHARED_STACK)
        .balign 16
irq_stack:
        .rept IRQ_STACK_SIZE
        .byte 0
        .endr
irq_stack_end = .
#endif /* IRQMODE && !IRQMODE_SHARED_STACK */
