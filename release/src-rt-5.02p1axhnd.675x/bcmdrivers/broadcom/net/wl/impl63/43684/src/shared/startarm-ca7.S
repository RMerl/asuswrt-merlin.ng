/*
 * RTE ARM run-time initialization and first level exception handling.
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 * $Id: startarm-ca7.S 779771 2019-10-07 11:26:40Z $
 */

/**
 * Cortex-A7 supports the Thumb-2 instruction set within the ARMv7-A instruction set. Instructions
 * can be either 16 or 32 bits wide.
 */

#include <arminc.h>
#include <sbhndarm.h>
#include <hnd_armtrap.h>
#include <hnd_debug.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <bcmdevs.h>
#include <sbsysmem.h>

/* threadx runs in SVC mode */
#define	ARM_DEFAULT_MODE	PS_SVC

/* IRQ stack */
#ifndef IRQ_STACK_SIZE
#define IRQ_STACK_SIZE		512
#endif	/* IRQ_STACK_SIZE */

	.text

#ifdef	__thumb__
	.thumb
#endif // endif

/* Entry point */
	.section .text.startup
	.global startup
	.global _tx_thread_context_save
	.global __tx_irq_processing_return
	.global threadx_isr
	.global _tx_thread_context_restore
	.global _tx_thread_system_state
	.global _tx_thread_fiq_context_save
	.global __tx_fiq_processing_return
	.global threadx_fiq_isr
	.global _tx_thread_fiq_context_restore

startup:

/* #if defined(__ARM_ARCH_7R__) */

excvtbl:
	/* Total 8 entries */
#ifndef	__thumb__
	.arm
	/* Cortex A7 in ARM mode. */
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
	/* Cortex A7 in thumb mode. */
	b.w	tr_rst		/* 0 - reset */
	b.w	tr_und		/* 4 - undefined instruction */
	b.w	tr_swi		/* 8 - software interrupt */
	b.w	tr_iab		/* 0xc - prefetch abort */
	b.w	tr_dab		/* 0x10 - data abort */
	b.w	tr_bad		/* 0x14 - reserved */
	b.w	tr_irq		/* 0x18 - external interrupt */
	b.w	tr_fiq		/* 0x1c - fast interrupt */
#endif // endif

/* Reset handler */
#ifdef	__thumb__
	.thumb
#endif // endif

#ifdef FLOPS_SUPPORT

excvtbl_end:

	.section .text.startup_flops
	.global startup_flops
startup_flops:

# now this place is labeled as dup_excvtbl, where the instructions from
# excvtbl are copied using flopscopy script.
dup_excvtbl:
#endif /* FLOPS_SUPPORT */

# 32 bytes for duplicate execvtbl and rest till 0x78 reserved.
#if !defined(RAMBASE)
.skip 0x60, 0xFA
#else /* RAMBASE */
.skip 0x4C, 0xFA

# Place RAMSIZE at a known location.
ramsize_ptr:
	.word	HND_RAMSIZE_PTR_MAGIC	/* 'RAMS' */
	.word	RAMSIZE

# Place debug_info at a known location, RAMBase+0x878
debug_info_ptr:
	.global hnd_debug_info
	.word	HND_DEBUG_PTR_PTR_MAGIC		/* 'DBPP' */
	.word	hnd_debug_info
	.word	RAMBASE
#endif /* RAMBASE */

tr_rst:

#ifdef FLOPS_SUPPORT
# code to copy flops from location labeled dup_excvtbl, to vectors.
#ifdef	__thumb__
	ldr r8, =dup_excvtbl
	mov r9, #0x00
	ldm.w r8!, {r0-r7}
	stm.w r9!, {r0-r7}
#endif // endif
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
#endif // endif

#ifdef IRQMODE
	/* Switch to IRQ mode and make sure interrupts are disabled */
	mov 	r1,#(PS_I | PS_F | PS_IRQ)
	orr	r0,r0,r1
	msr	cpsr,r0
	ldr	sp,=irq_stack_end

	mrs	r0,cpsr
	mov	r1,#(PS_MM)
	bic	r0,r0,r1
#endif // endif

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

	/* now ca7 is in system mode */
	/* lr has the value before the exception, push it to stack */
	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}	/* save r0-r7 to the stack */
	eor	r0,r0,r0
	add	r0,r0,#TR_UND
	b	trap

/*
 * Software interrupt (SVC) exception handler. Entered on Reset, and on execution of a Supervisor
 * Call (SVC) instruction.
 */
tr_swi:
	srsdb	sp!, #ARM_DEFAULT_MODE  /* StoreReturnStateDecrBefore, use default mode stack */
	cps	#ARM_DEFAULT_MODE       /* ChangeProcessorState */
	push 	{r0}	/* hold the place for r15 (pc) */
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
	mov 	sp,lr
	sub	sp,sp,#8
	mov 	lr,sp
	srsdb	sp!, #ARM_DEFAULT_MODE	/* use default mode stack */
	cps	#ARM_DEFAULT_MODE

	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}
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
	ldr	r0,=threadx_isr
	blx	r0
	b	_tx_thread_context_restore

	/* swich back to THUMB mode */
	.thumb

/* Fast interrupt handler */
tr_fiq:
	push	{r0}
	ldr	r0,=tr_threadx_arm_fiq
	bx	r0

	/* this block of code runs in ARM mode as threadx context/restore
	 * functions are only supported in ARM mode
	 */
	.align 2
	.arm
tr_threadx_arm_fiq:
	pop	{r0}
	b	_tx_thread_fiq_context_save
__tx_fiq_processing_return:
	ldr	r0,=threadx_fiq_isr
	blx	r0

	/* Generate a trap only if FIQ handler returns TRUE */
	cmp	r0,#0
	bne	1f
	b	_tx_thread_fiq_context_restore

1:
	/* Following code does the same as _tx_thread_fiq_context_restore except restoring pc */

	/* Disable IRQ and FIQ interrupts */
	cpsID	if

	/* Determine if interrupts are nested. */
	ldr	r3,=_tx_thread_system_state	/* Pickup address of system state variable */
	ldr	r2,[r3]		/* Pickup system state */
	sub	r2,r2,#1	/* Decrement the counter */
	str	r2,[r3]		/* Store the counter */
	cmp	r2,#0		/* Was this the first interrupt */
	beq	2f			/* __tx_thread_fiq_not_nested_restore */

	/* Interrupts are nested. */
	ldmia	sp!,{r0, r10, r12, lr}	/* Recover SPSR, POI, and scratch regs */
	b	3f

2:
	ldmia	sp!,{r0, lr}	/* Recover SPSR, POI, and scratch regs */

3:
	msr	SPSR_cxsf,r0	/* Put SPSR back */
	ldmia	sp!,{r0-r3}	/* Recover r0-r3 */

	/* Now we are on original state */
	srsdb	sp!,#ARM_DEFAULT_MODE	/* use default mode stack */
	cpsID	if,#ARM_DEFAULT_MODE	/* disable IRQ/FIQ */

	push	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push	{r0-r7}
	eor	r0,r0,r0
	add	r0,r0,#TR_FIQ

	/* branch from ARM mode the THUMB mode */
	ldr	r4,=trap
	orr	r4,r4,#1
	blx	r4

	/* swich back to THUMB mode */
	.thumb

/* Generic exception handler */
trap:
	/*
	 * construct the trap structure 'trap_t' in system mode stack (SVC mode for threadx)
	*/
	/* save trap type, epc, cpsr and spsr */
	mov 	r4, sp	/* precondition: sp points at the saved r0 value */
	add	r4,r4,#64
	ldmia	r4!, {r1,r3} /* r1<-epc, r3<-spsr */
	mrs	r2, cpsr
	push	{r0,r1,r2,r3}	/* trap_t: type(r0), epc(r1), cpsr, spsr struct members */

	/* fix the value of pc in trap structure */
	sub	r4,r4,#12
	str	r1,[r4]	/* save the value or epc as r15 into the stack */
	and	r1,r3,#PS_F	/* save old FIQ enable flag */

	/* save r8 - r13 */
	mov 	r7,sp	/* sp is at the being of the trap structure now */
	add	r7,r7,#TRAP_T_SIZE + 8	/* sp value before the exception */
	mov 	r6,r12	/* r6<-r12 */
	mov	r5,r11
	mov	r4,r10
	mov	r3,r9
	mov	r2,r8
	add	sp,sp,#72 /* move sp to r14 */
	push	{r2-r7}	/* save r8-r13 to stack, constructing part of a 'trap_t' on the stack */
	sub	sp,sp,#48 /* move sp back to the top of trap structure */

	cmp	r0,#TR_IRQ
	bne	2f
	cmp 	r1,#PS_F	/* IRQ is disabled before entering exception */
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
	cpsie	a
	bx	lr
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

/*
 * Common code/data start here...
 */
#ifdef CONFIG_XIP
	.data
#endif // endif

/* Debug/Trace */
	DW(__watermark,	0xbbadbadd)

/* HSIC sdr debugging purpose */
#ifdef HSIC_SDR_DEBUG
	DW(__chipsdrenable,	0xbbadbadd)
	DW(__nopubkeyinotp,	0xbbadbadd)
	DW(__imagenotdigsigned,	0xbbadbadd)
	DW(__pubkeyunavail,	0xbbadbadd)
	DW(__rsaimageverify, 0xbbadbadd)
#endif // endif

/* to save reset vec for CRC */
	DW(orig_rst, 0)

/* C trap handler */
	DW(trap_hook, 0)

/* chiptype */
	DW(chiptype, 1)

/* arm core regs and wrapper */
	DW(arm_regs, 0)
	DW(arm_wrap, 0)

/* stack bottom */
	DW(_stackbottom, 0)

/* sysmem core regs and corerev */
	DW(sysmem_regs, 0)
	DW(sysmem_rev, 0)

#ifdef	CONFIG_XIP
	.text
#endif // endif

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
#endif // endif

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
#endif // endif
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
#endif // endif
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
	TRACE(0x43650001)
	/* Initialize MPU */
	/* bl	mpu_init */
	/* Figure out if we have an SB or AI chip */
	ldr	r0,=SI_ENUM_BASE_DEFAULT		/* r0:	core regs SI base address */
	ldr	r1,[r0,#CC_CHIPID]
	ldr	r2,=(CID_ID_MASK & ~0x7)
	and	r2,r2,r1

#ifdef SI_CCI500_S1_CTL
/* CCI500 S1 port not fully reset by WDReset. Manually clear it */
reset_cci500_s1:
	ldr	r3,=SI_CCI500_S1_CTL	/* r3: address of CCI500_S1_SNOOP_CTL */
	mov	r4,#0
	str	r4,[r3]
#else
/* CCI400 S3 port not fully reset by WDReset. Manually clear it */
reset_cci400_s3:
	add	r3,r0,#SI_CCI400_S3_CTL_OFFSET	/* r3: address of CCI400_S3_SNOOP_CTL */
	mov	r4,#0
	str	r4,[r3]
#endif // endif

/*
 * Find arm core
 * Input:
 *	r0 - chipc base address.
 *	r1 - chip id
 * Output:
 *	r0 - arm regs
 *	r8 - arm regs
 *	r9 - arm wrapper (sbconfig or ai dmp)
 *      r10 - socram/sysmem regs
 *	r11 - arm core index
 * Changed:
 *	r0, r1, r2, r3, r4, r5, r8, r9, r10, r11
 */
findcore:
	mov	r5,#0
	mov	r8,r5
	mov	r10,r5
	ldr	r1,=CC_EROMPTR
	ldr	r1,[r0,r1]			/* r1:	erom pointer */
1:	ldr	r2,[r1]				/* r2:	erom entry */
	ldr	r3,=(ER_TAG | ER_VALID)
	and	r3,r3,r2
	cmp	r3,#(ER_END | ER_VALID)		/* Is this the END marker? */
	beq	panic				/* Yes: done and did not find ca7 */

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
	ldr	r4,=(ARMCA7_CORE_ID << CIA_CID_SHIFT)
	cmp	r3,r4
	bne	3f

	/* Found arm, get regs and wrapper and save them */
	bl	ai_get_slave
	mov	r8,r0

	bl	ai_get_wrapper
	mov	r9,r0
	b	4f

3:
	ldr	r4,=(SYSMEM_CORE_ID << CIA_CID_SHIFT)
	cmp	r3,r4
	bne	1b

	/* Found socram/sysmem, get corerev, regs and wrapper and save them */
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
	bl	ca7_mem_info

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

	/* Save sysmem regs and corerev */
	mov	r0,r10
	ldr	r1,=sysmem_regs
	str	r0,[r1]
	mov	r0,r12
	ldr	r1,=sysmem_rev
	str	r0,[r1]

#ifndef NO_ONTHEFLY_FREQ_CHANGE
force_ht:
	ldr	r3,=SI_ENUM_BASE_DEFAULT /* core regs SI base address */
	add	r0,r3,#0x1e0		/* clk_ctl_st */
	ldr	r1,[r0]
	orr	r1,#0x2			/* ForceHT */
	str	r1,[r0]
1:	ldr	r1,[r0]
	ldr	r2,=(0x1 << 19)
	tst	r1,r2
	beq	1b
#endif // endif

#ifdef SECOND_ARM
	/* Save ARM core index */
	mov	r0,r11
	ldr	r1,=_armca7_idx
	str	r0,[r1]
#endif // endif
/* get memory info. So here onwards,
 *	r5 - ram size [atcm + btcm]
 *	r6 - atcm ram base address
 *	r7 - RAM bottom/stack start
 */
	bl	ca7_mem_info

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
	bne	call_main

	mov	sp,r7
	b	vars_present
#endif /* BCMUSBDEV_ENABLED && BCMTRXV2 */

	TRACE(0x43650002)
	sub	r5,r5,#4	/* _varsz */
	mov	sp,r5
	TRACE(0x43650003)
	/* esp for sdio case */
#if defined(BCMHOSTVARS)
	TRACE(0x43650004)
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
	TRACE(0x43650005)
#endif /* BCMHOSTVARS */
#endif /* !BCM_BOOTLOADER */

	/* Align the stack on a 16 byte boundary */
	mov	r0,sp
	lsr	r0,r0,4
	lsl	r0,r0,4
	mov	sp,r0

	/* set the stack bottom */
	mov	r0,sp
	ldr	r4,=_stackbottom
	str	r0,[r4]

#ifdef IRQMODE_SHARED_STACK
	/* similar to IRQMODE but IRQ uses that same stack as the default mode */

	/* _stackbottom in r4 */
	mov	r4,r0

	/* initiaze IRQ stack with same stack as default mode stack (from _stackbottom) */
	mrs	r0,cpsr
	mov	r1,#(PS_MM)
	bic	r0,r0,r1
	mov 	r1,#(PS_I | PS_F | PS_IRQ)
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
rfm:
	TRACE(0xbaad)
	b	.

END(setup)

/* Find Memory Size of different regions in CA7.
 * Input:
 *	r8 - arm registers
 *	r9 - arm corerev
 * Output:
 *	r5 - ram size [atcm + btcm]
 *	r6 - atcm ram base address
 *	r7 - RAM bottom/stack start
 */
FUNC(ca7_mem_info)

/*
 * Calculate memory size in bytes.
 * Input:
 *	r7 - (chip type << CID_TYPE_SHIFT)
 *	r10 - socram/sysmem registers
 *	r12 - socram/sysmem corerev
 * Output:
 *	r5 - memsize.
 */
memsz:
	/* Find ram size */
	TRACE(0x35)

	/* Can hardcode MEMSIZE in dongle makefile to save ~112 bytes */
#ifdef MEMSIZE
	ldr	r5,=MEMSIZE
	ldr	r6,=SI_ARMCA7_RAM
	add	r7,r5,r6
	bx lr
#else /* !MEMSIZE */
	mov	r0,r12
	mov	r1,r10

	/* Find number of blocks */
	ldr	r2,=SR_COREINFO
	ldr	r2,[r1,r2]
	mov	r3,r2				/* r2:	coreinfo */
	ldr	r1,=SYSMEM_SRCI_SRNB_MASK
	and	r3,r3,r1
	_LSR_	r3,r3,#SYSMEM_SRCI_SRNB_SHIFT
	TRACE(0x38)
	/* r3 are number of banks, r10 are the regs */
	ldr	r0,=0
	ldr	r5,=0
	ldr	r2,=SYSMEM_BANKIDX_REG
	ldr	r7,=SYSMEM_BANKINFO_REG
	ldr	r6,=SYSMEM_BANKINFO_SZMASK
	ldr	r4,=SYSMEM_BANKSIZE_SHIFT
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
	ldr	r6,=SI_ARMCA7_RAM
	add	r7,r5,r6
	bx lr
#endif /* MEMSIZE */
END(ca7_mem_info)

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
	udiv	r0, r0, r0	// <== divide by 0 exception (comment out to
	str	r5, [r0]	// <== test null pointer store exception)
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
/* Flush the whole D-cache */
FUNC(cpu_flush_dcache_all)
	mrc	p15, 1, r0, c0, c0, 1		@ read clidr
	ands	r3, r0, #0x7000000		@ extract loc from clidr
	mov	r3, r3, lsr #23			@ left align loc bit field
	beq	finished			@ if loc is 0, then no need to clean
	mov	r10, #0				@ start clean at cache level 0
loop1:
	add	r2, r10, r10, lsr #1		@ work out 3x current cache level
	mov	r1, r0, lsr r2			@ extract cache type bits from clidr
	and	r1, r1, #7			@ mask of the bits for current cache only
	cmp	r1, #2				@ see what cache we have at this level
	blt	skip				@ skip if no cache, or just i-cache
	mcr	p15, 2, r10, c0, c0, 0		@ select current cache level in cssr
	mcr	p15, 0, r10, c7, c5, 4		@ 'isb',
	mrc	p15, 1, r1, c0, c0, 0		@ read the new csidr
	and	r2, r1, #7			@ extract the length of the cache lines
	add	r2, r2, #4			@ add 4 (line length offset)
	ldr	r4, =0x3ff
	ands	r4, r4, r1, lsr #3		@ find maximum number on the way size
	clz	r5, r4				@ find bit position of way size increment
	ldr	r7, =0x7fff
	ands	r7, r7, r1, lsr #13		@ extract max number of the index size
loop2:
	mov	r9, r4				@ create working copy of max way size
loop3:
	lsl	r6, r9, r5
	orr	r11, r10, r6			@ factor way and cache number into r11
	lsl	r6, r7, r2
	orr	r11, r11, r6			@ factor index number into r11
	mcr	p15, 0, r11, c7, c14, 2		@ clean & invalidate by set/way
	subs	r9, r9, #1			@ decrement the way
	bge	loop3
	subs	r7, r7, #1			@ decrement the index
	bge	loop2
skip:
	add	r10, r10, #2			@ increment cache number
	cmp	r3, r10
	bgt	loop1
finished:
	mov	r10, #0				@ swith back to cache level 0
	mcr	p15, 2, r10, c0, c0, 0		@ select current cache level in cssr
	mcr	p15, 0, r10, c7, c5, 4		@ 'isb',
	mov	pc, lr
END(cpu_flush_dcache_all)

FUNC(cpu_flush_cache_all)
	stmfd	sp!, {r0-r7, r9-r11, lr}
	bl	cpu_flush_dcache_all
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 0		@ I+BTB cache invalidate
	ldmfd	sp!, {r0-r7, r9-r11, lr}
	mov	pc, lr
END(cpu_flush_cache_all)

/* Invalidate the whole D-cache */
cpu_inv_dcache_all:
	mrc	p15, 1, r0, c0, c0, 1		@ read clidr
	ands	r3, r0, #0x7000000		@ extract loc from clidr
	mov	r3, r3, lsr #23			@ left align loc bit field
	beq	invfinished			@ if loc is 0, then no need to clean
	mov	r10, #0				@ start clean at cache level 0
invloop1:
	add	r2, r10, r10, lsr #1		@ work out 3x current cache level
	mov	r1, r0, lsr r2			@ extract cache type bits from clidr
	and	r1, r1, #7			@ mask of the bits for current cache only
	cmp	r1, #2				@ see what cache we have at this level
	blt	invskip				@ skip if no cache, or just i-cache
	mcr	p15, 2, r10, c0, c0, 0		@ select current cache level in cssr
	mcr	p15, 0, r10, c7, c5, 4		@ 'isb',
	mrc	p15, 1, r1, c0, c0, 0		@ read the new csidr
	and	r2, r1, #7			@ extract the length of the cache lines
	add	r2, r2, #4			@ add 4 (line length offset)
	ldr	r4, =0x3ff
	ands	r4, r4, r1, lsr #3		@ find maximum number on the way size
	clz	r5, r4				@ find bit position of way size increment
	ldr	r7, =0x7fff
	ands	r7, r7, r1, lsr #13		@ extract max number of the index size
invloop2:
	mov	r9, r4				@ create working copy of max way size
invloop3:
	lsl	r6, r9, r5
	orr	r11, r10, r6			@ factor way and cache number into r11
	lsl	r6, r7, r2
	orr	r11, r11, r6			@ factor index number into r11
	mcr	p15, 0, r11, c7, c6, 2		@ invalidate by set/way
	subs	r9, r9, #1			@ decrement the way
	bge	invloop3
	subs	r7, r7, #1			@ decrement the index
	bge	invloop2
invskip:
	add	r10, r10, #2			@ increment cache number
	cmp	r3, r10
	bgt	invloop1
invfinished:
	mov	r10, #0				@ swith back to cache level 0
	mcr	p15, 2, r10, c0, c0, 0		@ select current cache level in cssr
	mcr	p15, 0, r10, c7, c5, 4		@ 'isb',
	mov	pc, lr

FUNC(cpu_inv_cache_all)
	stmfd	sp!, {r0-r7, r9-r11, lr}
	bl	cpu_inv_dcache_all
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 0		@ I+BTB cache invalidate
	ldmfd	sp!, {r0-r7, r9-r11, lr}
	mov	pc, lr
END(cpu_inv_cache_all)

#endif /* RTE_CACHED */

	.data

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

        .section .data._armca7_idx
	DW(_armca7_idx, 0)

#if defined(BCMSDIODEV) || defined(BCMHOSTVARS)
        .section .data._vars
	DW(_vars, 0)

        .section .data._varsz
	DW(_varsz, 0)
#endif /* BCMSDIODEV || BCMHOSTVARS */

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
fiq_stack_end:
        .word 0
/* C fiq trap handler */
	DW(fiqtrap_hook, 0)
#endif // endif
#ifdef IRQMODE
        .balign 16
irq_stack:
        .rept IRQ_STACK_SIZE
        .byte 0
        .endr
irq_stack_end:
        .word 0
        .balign 16
#endif // endif
