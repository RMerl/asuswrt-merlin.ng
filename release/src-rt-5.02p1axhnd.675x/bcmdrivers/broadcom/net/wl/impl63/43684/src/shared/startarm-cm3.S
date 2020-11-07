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
 * $Id: startarm-cm3.S 778425 2019-08-30 12:52:56Z $
 */

#include <arminc.h>
#include <sbhndarm.h>
#include <hnd_armtrap.h>
#include <hnd_debug.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <sbsocram.h>
#include <bcmdevs.h>
#include <hnd_armtrap.h>

#ifndef	__thumb2__
#error __thumb2__ not defined in startarm-cm3.S
#endif // endif

	.text
	.syntax unified

/* Entry point */
	.section .text.startup
	.global startup
startup:

/*
 * Cortex-M3 Exception Vector Table.
 */
	/* Repeat .word <hdlr> <cnt> times */
	.macro _ISR_ hdlr cnt
	.word	\hdlr
	.if \cnt-1
		_ISR_ \hdlr "(\cnt-1)"
	.endif
	.endm

	.global __stack_end__
	.global setup
	.global trap
	.global svc
	.global __tx_SVCallHandler
	.global	__tx_PendSVHandler
	.global	irq

excvtbl:
	/* Total 32 entries */
#ifdef BCMDBG_BOOT
	.word	__stack_end__	/* 0 */
#else
	.word	0		/* 0 */
#endif /* BCMDBG_BOOT */
	.word	setup		/* 4 */
	.word	trap		/* 8 */
	.word	trap		/* 0xc */
	.word	trap		/* 0x10 */
	.word	trap		/* 0x14 */
	.word	trap		/* 0x18 */
	.word	trap		/* 0x1c */
	.word	trap		/* 0x20 */
	.word	trap		/* 0x24 */
	.word	trap		/* 0x28 */
#ifdef BCMDBG_ASSERT
	.word	svc		/* 0x2c */
#else
	.word	__tx_SVCallHandler /* 0x2c */
#endif /* BCMDBG_ASSERT */
	.word	trap		/* 0x30 */
	.word	trap		/* 0x34 */
	.word	__tx_PendSVHandler /* 0x38 */
	.word	trap		/* 0x3c */
	_ISR_	irq,ARMCM3_NUMINTS /* 0x40 - 0x7c */

	/* Put an entry for the TRX to use. This is the first location after
	 * vectors, so its a handy fixed address to put into any Makefile
	 * that is going to build a trx.
	 */
	.global trx_entry
trx_entry:
	/* Go to the common setup routine! */
	ldr	r0,=setup
	bx	r0

	/* Put a pointer to the debug area in an easy-to-find area */
#if defined(RAMBASE)
	.section .text.debugptr
	.global hnd_debug_info
	.word	HND_DEBUG_PTR_PTR_MAGIC		/* 'DBPP' */
	.word	hnd_debug_info
	.word	RAMBASE
#endif	/* RAMBASE */

/* Generic exception handler */
FUNC(trap)
	/* The CPU saves eight registers on the stack automatically so that sp is
	 * now pointing at a block containing: r0, r1, r2, r3, r12, lr, pc, xPSR.
	 * Save exception block pointer in r0 so we can push more on the stack.
	 */
	mrs	r0,psp

	/* Push the EXC_RETURN value on the stack so that the end of this function
	 * can call pop {pc}.  EXC_RETURN is not the actual lr.  It is typically a
	 * value like 0xfffffff9.
	 *
	 * Allocate and fill out a trap_t structure on the stack by pushing its
	 * fields in reverse order.
	 */

	ldr	r3,[r0,#CM3_TROFF_PC]
	ldr	r1,[r0,#CM3_TROFF_LR]
	add	r0, #32			/* Account for regs pushed by CM3 */
	push	{r0,r1,r3,lr}		/* EXC_RETURN, TR_PC, TR_LR, TR_SP */
	sub	r0, #32

	ldr	r3,[r0,#CM3_TROFF_R12]
	mov	r2,r11
	mov	r1,r10
	push	{r1,r2,r3}		/* TR_REG(12) - TR_REG(10) */

	mov	r2,r9
	mov	r1,r8
	push	{r1,r2}			/* TR_REG(9) - TR_REG(8) */

	ldr	r3,[r0,#CM3_TROFF_R3]
	ldr	r2,[r0,#CM3_TROFF_R2]
	ldr	r1,[r0,#CM3_TROFF_R1]
	push	{r1-r7}			/* TR_REG(7) - TR_REG(1) */

	ldr	r3,[r0,#CM3_TROFF_R0]
	ldr	r2,[r0,#CM3_TROFF_xPSR]
	mrs	r1,psr
	push	{r1,r2,r3}		/* TR_REG(0), TR_SPSR, TR_CPSR */

	ldr	r2,[r0,#CM3_TROFF_PC]
	mrs	r1,ipsr
	push	{r1,r2}			/* TR_EPC, TR_TYPE */

	/* Call trap_hook, passing trap_t as first argument */
	ldr	r0,=trap_hook
	ldr	r1,[r0]
	cmp	r1,#0
	/* Halt if trap_hook is null */
	beq	.
	mov	r0,sp
	blx	r1

	/* Pop trap_t off the stack */
	add	sp,sp,#TRAP_T_SIZE

	/* Return from exception */
	pop	{pc}
END(trap)

	.global _tx_thread_context_save
	.global threadx_isr
	.global _tx_thread_context_restore

/* Interrupt handler */
FUNC(irq)
	push    {lr}
	blx	_tx_thread_context_save
	mrs	r0, ipsr
	push	{r0}
	mov	r0, sp
	blx	threadx_isr
	pop	{r0}
	b	_tx_thread_context_restore
END(irq)

#ifdef BCMDBG_ASSERT
	.global hnd_svctrap_handler
/* SVC handler */
FUNC(svc)
	mrs r0, psp
	b hnd_svctrap_handler
END(svc)
#endif /* BCMDBG_ASSERT */

/* The following all can be done in C */
/* Enable/Disable ARM IRQ(s) */
FUNC(enable_arm_irq)
	cpsie	i
	bx	lr
END(enable_arm_irq)

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

FUNC(enable_nvic_ints)
	ldr	r1,=ARMCM3_INTALL
	and	r0,r0,r1
	cmp	r0,#0
	beq	1f
	ldr	r1,=CM3_NVIC_IRQ_SET_EN0
	ldr	r2,[r1,#0]
	orr	r2,r2,r0
	str	r2,[r1,#0]
1:	bx	lr
END(enable_nvic_ints)

FUNC(disable_nvic_ints)
	ldr	r1,=ARMCM3_INTALL
	and	r0,r0,r1
	cmp	r0,#0
	beq	1f
	ldr	r1,=CM3_NVIC_IRQ_CLR_EN0
	ldr	r2,[r1,#0]
	orr	r2,r2,r0
	str	r2,[r1,#0]
1:	bx	lr
END(disable_nvic_ints)

/* Cycle count */
FUNC(get_arm_cyclecount)
	ldr	r1,=arm_regs
	ldr	r1,[r1,#0]
	ldr	r2,=ARMCM3_CYCLECNT
	ldr	r0,[r1,r2]
	bx	lr
END(get_arm_cyclecount)

FUNC(set_arm_cyclecount)
	ldr	r1,=arm_regs
	ldr	r1,[r1,#0]
	ldr	r2,=ARMCM3_CYCLECNT
	str	r0,[r1,r2]
	bx	lr
END(set_arm_cyclecount)

/* Interval timer expiration time */
FUNC(get_arm_inttimer)
	ldr	r1,=arm_regs
	ldr	r1,[r1,#0]
	ldr	r2,=ARMCM3_INTTIMER
	ldr	r0,[r1,r2]
	bx	lr
END(get_arm_inttimer)

FUNC(set_arm_inttimer)
	ldr	r1,=arm_regs
	ldr	r1,[r1,#0]
	ldr	r2,=ARMCM3_INTTIMER
	str	r0,[r1,r2]
	bx	lr
END(set_arm_inttimer)

/* Interval timer interrupt mask */
FUNC(get_arm_intmask)
	ldr	r1,=arm_regs
	ldr	r1,[r1,#0]
	ldr	r2,=ARMCM3_INTMASK
	ldr	r0,[r1,r2]
	bx	lr
END(get_arm_intmask)

FUNC(set_arm_intmask)
	ldr	r1,=arm_regs
	ldr	r1,[r1,#0]
	ldr	r2,=ARMCM3_INTMASK
	str	r0,[r1,r2]
	bx	lr
END(set_arm_intmask)

/* Interval timer interrupt status */
FUNC(get_arm_intstatus)
	ldr	r1,=arm_regs
	ldr	r1,[r1,#0]
	ldr	r2,=ARMCM3_INTSTATUS
	ldr	r0,[r1,r2]
	bx	lr
END(get_arm_intstatus)

FUNC(set_arm_intstatus)
	ldr	r1,=arm_regs
	ldr	r1,[r1,#0]
	ldr	r2,=ARMCM3_INTSTATUS
	str	r0,[r1,r2]
	bx	lr
END(set_arm_intstatus)

/*
 * Common code/data start here...
 */
#ifdef CONFIG_XIP
	.data
#endif // endif

/* Debug/Trace */
	DW(__watermark,	0xbbadbadd)

/* to save reset vec for CRC */
	DW(orig_rst, 0)

/* C trap handler */
	DW(trap_hook, 0)

/* chiptype */
	DW(chiptype, 0)

/* arm core regs and wrapper */
	DW(arm_regs, 0)
	DW(arm_wrap, 0)

/* socram core regs and corerev */
	DW(socram_regs, 0)
	DW(socram_rev, 0)

/* stack bottom */
	DW(_stackbottom, 0)

#ifdef BCMDBG_BOOT
/* stack for debug */
        .balign 8
__stack_start__:
        .rept 16
        .byte 0
        .endr
__stack_end__ = .
#endif // endif

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

/*
 * Turn remap off and then jump to an given address
 */
FUNC(arm_jumpto)
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
#if defined(BCMSDIODEV) || defined(BCMHOSTVARS)
	.word	nvram_array
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
 *	- turn off pll ASAP, it may come up without knowing the xtal freq
 *	- wait till OTP is ready, it is used to repair/patch the RAM
 *	- copy the data section to RAM if running from ROM (XIP)
 *	- copy the code and data to RAM if booting from FLASH and run from RAM
 *	- set up stack for C code
 */
FUNC(setup)

	/* clear pending INT */
	ldr r2,=0x3F
	ldr r3,=0xE000E280
	str r2,[r3]

#ifdef VTABLE_RELLOC
	/* First - move ISR VTABLE to SRAM  */
	ldr	r2,=text_start
	ldr	r1,=CM3_VTOFF
	str	r2,[r1,#0]
#endif // endif
	/* Figure out if we have an SB or AI chip */
	ldr	r0,=SI_ENUM_BASE_DEFAULT		/* r0:	core regs SI base address */

	/* reset pmutimer to 0 */
	ldr	r1,=PMU_TIMER
	mov	r2,#0
	str	r2,[r1,r0]

	ldr	r1,[r0,#CC_CHIPID]
	ldr	r2,=(CID_ID_MASK)
	and	r2,r2,r1
#ifdef BCMCHIPID
#if (BCMCHIPID == BCM43236_CHIP_ID)

	/* Double check in case chipid does not match */
	ldr	r3,=BCMCHIPID
	cmp	r2,r3
	bne	1f

	/* Boot from flash if sflash is present */
	/* Check if we should boot from sflash in a 43236 */
#if (BCMCHIPID == BCM43236_CHIP_ID)
	ldr	r4,=CST43236_SFLASH_MASK
#endif // endif
	ldr	r2,[r0,#CC_CHIPST]		/* 1: Is sflash present? */
	tst	r2,r4
	beq	1f

	ldr	r2,=SI_FLASH2			/* 2: Is first word 0? */
	ldr	r3,[r2]
	tst	r3,r3
	bne	1f

	ldr	r3,=0xff000000			/* 3: Are we in rom? */
	mov	r4,pc
	ldr	r2,=SI_ARM_ROM
	and	r3,r3,r4
	cmp	r2,r3
	bne	1f                              /* We are in ROM. Go to boot. */

	/* OK, boot from flash */
	/* First change the remap */
#if BCMCHIPID == BCM43236_CHIP_ID
	ldr	r1,=(0x18102000 + AI_IOCTRL)
#else
	ldr	r1,=(0x18103000 + AI_IOCTRL)
#endif // endif
	ldr	r3,[r1]
	ldr	r4,=~SICF_REMAP_MSK
	and	r3,r3,r4
	ldr	r4,=SIFC_REMAP_FLASH
	orr	r3,r3,r4

	ldr	r2,=SI_FLASH2			/* load the SFLASH address */
	ldr	r0,[r2,#4]			/* Pick up boot vector */
	orr	r0,r0,r2
	str	r3,[r1]
	bx	r0				/* And go there */

#endif /* (BCMCHIPID == BCM43236_CHIP_ID) */
#endif /* BCMCHIPID */

1:
	ldr	r7,=CID_TYPE_MASK
	and	r7,r7,r1			/* r7:	0 if SB, non-zero if AI */

/*
 * Find arm core and socram core
 * Input:
 *	r0 - chipc base address.
 *	r1 - chip id
 *	r7 - (chip type << CID_TYPE_SHIFT)
 * Output:
 *	r0 - arm regs
 *	r8 - arm regs
 *	r9 - arm wrapper (sbconfig or ai dmp)
 *	r10 - socram regs
 *	r11 - socram wrapper (sbconfig or ai dmp)
 *	r12 - socram corerev
 * Changed:
 *	r0, r1, r2, r3, r4, r5, r8, r9, r10, r11, r12
 */
findcore:
#if !defined(BCMCHIPTYPE) || (BCMCHIPTYPE == SOCI_AI)
	tst	r7,r7
	bne	ai_findcore
#endif // endif
#if !defined(BCMCHIPTYPE) || (BCMCHIPTYPE == SOCI_SB)
	mov	r3,#0
	mov	r8,r3
	mov	r10,r3

	/* Find cores in Siliconbackplane chips */
	ldr	r2,=CID_CC_MASK
	and	r2,r2,r1
	ldr	r1,=CID_CC_SHIFT
	_LSR_	r2,r2,r1			/* r2:	core count */
	ldr	r1,=(SBCONFIGOFF + SBIDHIGH)	/* r1:	sbconfig.sbidhigh offset */
1:	ldr	r3,[r0,r1]
	ldr	r4,=SBIDH_CC_MASK
	and	r4,r4,r3
	ldr	r5,=(ARM_CORE_ID << SBIDH_CC_SHIFT)
	cmp	r4,r5
	bne	2f

	/* Found arm, save regs & sbconfig */
	mov	r8,r0
	ldr	r5,=SBCONFIGOFF
	add	r5,r0,r5
	mov	r9,r5
	b	3f

2:	ldr	r5,=(SOCRAM_CORE_ID << SBIDH_CC_SHIFT)
	cmp	r4,r5
	bne	3f

	/* Found socram, save regs & sbconfig */
	mov	r10,r0
	ldr	r5,=SBCONFIGOFF
	add	r5,r0,r5
	mov	r11,r5

	/* Also figure out corerev and save it */
	ldr	r4,=SBIDH_RC_MASK
	mov	r5,r3
	and	r3,r3,r4
	ldr	r4,=SBIDH_RCE_MASK
	and	r5,r5,r4
	_LSR_	r5,r5,#SBIDH_RCE_SHIFT
	orr	r3,r3,r5
	mov	r12,r3

3:	mov 	r3, #0
	cmp	r8,r3
	beq	4f
	cmp	r10,r3
	bne	chk_cores
4:	ldr	r3,=SI_CORE_SIZE
	add	r0,r0,r3
	sub	r2,r2,#1
	bne	1b
	b	panic
#endif // endif
	/* Done, check we have both arm and socram cores */
chk_cores:
	mov	r0,r10
	tst	r0,r0
	beq	panic

	mov	r0,r8
	tst	r0,r0
	bne	alpclk

panic:	b	panic

#if !defined(BCMCHIPTYPE) || (BCMCHIPTYPE == SOCI_AI)
	/* findcores for AMBA Interconnect chips */
ai_findcore:
	ldr	r1,=CC_EROMPTR
	ldr	r1,[r0,r1]			/* r1:	erom pointer */
1:	ldr	r2,[r1]				/* r2:	erom entry */
	ldr	r3,=(ER_TAG | ER_VALID)
	and	r3,r3,r2
	cmp	r3,#(ER_END | ER_VALID)		/* Is this the END marker? */
	beq	chk_cores			/* Yes: done */

	cmp	r3,#(ER_CI | ER_VALID)
	beq	2f

	add	r1,r1,#4			/* Next erom entry */
	b	1b

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
	ldr	r4,=(ARM_CORE_ID << CIA_CID_SHIFT)
	cmp	r3,r4
	bne	3f

	/* Found arm, get regs and wrapper and save them */
	bl	ai_get_slave
	mov	r8,r0

	bl	ai_get_wrapper
	mov	r9,r0
	b	1b

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

	bl	ai_get_wrapper
	mov	r11,r0
	b	1b
#endif /* !defined(BCMCHIPTYPE) || (BCMCHIPTYPE == SOCI_AI) */

/*
 * Change to run on ALP until PLL is configured.
 * Input:
 *	r0 - arm registers
 * Output:
 * Changed:
 *	r1, r2
 */
alpclk:
	/* Stop HT and force ALP and run on ALP clock until PLL is configured */
	ldr	r1,=SI_CLK_CTL_ST
	ldr	r2,=(CCS_FORCEHWREQOFF | CCS_FORCEALP)
	str	r2,[r0,r1]

/*
 * Wait until OTP is ready if OTP is present and powered up
 * Input:
 * Output:
 * Changed:
 *	r1, r2, r3, r4
 */
waitotp:
	ldr	r2,=SI_ENUM_BASE_DEFAULT			/* r2:	Chipc base */

	/* PMU rev must be >= 1 */
	ldr	r1,=PMU_CAP
	ldr	r1,[r1,r2]
	ldr	r3,=PCAP_REV_MASK
	tst	r1,r3
	beq	isflrom
	/* OTP must be present */
	ldr	r1,[r2,#CC_CAPABILITIES]
	ldr	r3,=CC_CAP_OTPSIZE
	tst	r1,r3
	beq	isflrom
	/* Check OTP power down strapping option first */
	ldr	r1,[r2,#CC_CHIPID]
	ldr	r3,=CID_ID_MASK
	and	r1,r1,r3				/* r1:	chipid */
	ldr	r3,[r2,#CC_CHIPST]
	/* Assume we don't need to wait for OTP ready */
	b	isflrom
	/* Don't wait if OTP is strapped to powered down */
1:	cmp	r3,r4
	beq	isflrom
2:	nop
	/* Assume we don't need to wait for OTP ready */
	b	isflrom
	/* Don't wait if OTP is not requested to power up */
1:	ldr	r1,=PMU_MIN_RES_MASK
	ldr	r1,[r1,r2]
	tst	r1,r3
	beq	isflrom
	/* Wait until OTP is powered up */
2:	ldr	r1,=PMU_RES_STATE
	ldr	r1,[r1,r2]
	and	r1,r1,r3
	cmp	r1,r3
	bne	2b
	/* Wait until OTP is ready */
	ldr	r3,=OTPS_READY
3:	ldr	r1,[r2,#CC_OTPST]
	tst	r1,r3
	beq	3b

/*
 * Check if flash or ROM is remapped.
 * Input:
 *	r9 - arm registers
 *	r7 - (chip type << CID_TYPE_SHIFT)
 * Changed:
 *	r3, r4
 */
isflrom:
#ifndef BCMNOFLROM
	mov	r1,r9				/* Get arm wrapper */
	tst	r7,r7
	bne	1f

	/* For SB chips, get sbtmstatelow and shift it */
	ldr	r3,=SBTMSTATELOW
	ldr	r3,[r1,r3]
	ldr	r4,=SBTML_SICF_SHIFT
	_LSR_	r3,r3,r4
	b	2f

	/* For AI chips, just get ioctrl register */
1:	ldr	r3,=AI_IOCTRL
	ldr	r3,[r1,r3]

	/* Figure out where we are running from */
2:	ldr	r4,=SICF_REMAP_MSK
	and	r3,r3,r4
	cmp	r3,#SICF_REMAP_NONE
	beq	inram

	/* Go take care of the flash/rom cases */
	bl	inflrom
#endif /* !BCMNOFLROM */

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

	/* Save socram regs and corerev */
	mov	r0,r10
	ldr	r1,=socram_regs
	str	r0,[r1]
	mov	r0,r12
	ldr	r1,=socram_rev
	str	r0,[r1]

	/* Also save chip type */
	ldr	r1,=chiptype
	str	r7,[r1]

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
	TRACE(1)

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
	beq	.
	b	6f

	/* corerev >= 8 memory size calculation */
3:
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
#endif /* MEMSIZE */

6:
	/* Save memory size */
	ldr	r1,=_memsize
	str	r5,[r1]

/* Clear BSS */
clearbss:
#ifndef EXT_CBALL
	TRACE(2)
	ldr	r0,=bss_start
	ldr	r1,=bss_end
	mov	r2,#0
1:	stmia	r0!,{r2}
	cmp	r1,r0
	bhi	1b
#endif	/* EXT_CBALL */

/*
 * Setup stack pointer.
 * Input:
 *	r5 - memsize.
 */
setsp:
	/* Set up stack pointer */
	TRACE(3)
#ifdef	CONFIG_XIP
	ldr	r2,=SI_ARM_SRAM2
	add	r5,r5,r2
#endif // endif
	sub	r5,r5,#4
#ifdef VTABLE_RELLOC
	ldr r1,=text_start
	add r5,r5,r1
#endif // endif
	mov	sp,r5

#if defined(BCMSDIODEV) || defined(BCMHOSTVARS)
	/*
	 * Check if the host wrote nvram vars at the end of memory.  If so,
	 * record their location/size and move them to a reserved (reclaimable) area.
	 * If valid, the vars length is in words (4 bytes) a multiple of 4 encoded in the last
	 * word of memory as (~len << 16 | len).  The vars precede the length.
	 * If invalid, the stack pointer starts at end of memory minus 4.
	 * During the init sequence of SDIO firmware, the last 4 bytes of memory are
	 * overwritten with the address of the sdpcm_shared structure.
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
#endif /* BCMSDIODEV || BCMHOSTVARS */

/* bootloader supports nvram dl: shift stack pointer beyond nvram */
#ifdef DL_NVRAM
	ldr	r5,=_dlvarsz
	str	r3,[r5]
	ldr	r0,=DL_NVRAM
	sub	r3,r3,r0
	ldr	r5,=_dlvars
	str	r3,[r5]
	sub	r3,r3,#4
	mov	sp,r3
#endif /* DL_NVRAM */

#ifdef SFLASH_BOOTLOADER
	bl sf_bootloader
#endif /* SFLASH_BOOTLOADER */

#if SR_MEMSIZE && !defined(SAVERESTORE_DISABLED)
    /* Free up top of memory for save/restore data */
    /* First: decrease memsize */
    ldr r3,=SR_MEMSIZE
    ldr r1,=_memsize
    ldr r5,[r1]
    sub r5,r5,r3
    str r5,[r1]
    /* Next: update SP */
    mov r4,sp
    sub r4,r4,r3
    mov sp,r4
#endif /* SR_MEMSIZE */

	mov     r0,sp
	_LSR_   r0,r0,#4
	_LSL_   r0,r0,#4
	mov     sp,r0

	/* set the stack bottom */
	mov	r4,sp
	ldr	r5,=_stackbottom
	str	r4,[r5]

/* Switch to C code main function */

	/* lable it assembly-to-c */
	TRACE(0xA2C)

	/* Call c_main and should never come back! */
	blx	c_main

/* Halt in case we ever come out of c_main! */
rfm:	TRACE(0xbaad)
	b	.

END(setup)

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
	mov	pc,lr
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
	mov	pc,lr
END(ai_get_wrapper)

#ifdef BCMTRAPTEST
/* Add console command 'tr' that causes a trap to test trap handling */
FUNC(traptest)
traptest:
	ldr	r1, =0x01010101
	ldr	r2, =0x02020202
	ldr	r3, =0x03030303
	ldr	r4, =0x04040404
	ldr	r5, =0x05050505
	ldr	r6, =0x06060606
	ldr	r7, =0x07070707
	ldr	r0, =0x08080808; mov	r8, r0
	ldr	r0, =0x09090909; mov	r9, r0
	ldr	r0, =0x0a0a0a0a; mov	r10, r0
	ldr	r0, =0x0b0b0b0b; mov	r11, r0
	ldr	r0, =0x0c0c0c0c; mov	r12, r0
	ldr	r0, =0x0d0d0d0d; //	mov	r13, r0
	ldr	r0, =0x0e0e0e0e; mov	r14, r0
	mov	r0, #0
	udiv	r0, r0, r0	// <== divide by 0 exception (comment out to
	str	r5, [r0]	// <== test null pointer store exception)
	bx	lr
END(traptest)
#endif /* BCMTRAPTEST */

#ifdef CONFIG_XIP
#ifdef ROMTABLE
	.word	ROMTABLE
#endif // endif
#endif	/* CONFIG_XIP */

	.data

/* Data section */

        .section .data._memsize
	DW(_memsize, 0)

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

#ifdef SFLASH_BOOTLOADER
FUNC(sf_bootloader)
	TRACE(0xA2D)
	/* for serial flash bootloader, XIP is too slow, copy to itself to RAM and run from there */
	ldr	r5,=SF_BOOTLOADER_RAM_ADDRESS
	mov	r3,r5

	/* the run version of the bootloader is stored in flash after the boot version */
	ldr 	r2,=rodata_end
	ldr	r1,=text_start
	sub	r0,r2,r1

	ldr 	r2,=data_end
	ldr	r1,=data_start
	sub	r2,r2,r1
	add	r0,r0,r2
        ldr     r1,=SF_BOOTLOADER_END_ADDRESS
	/* find the location of 2nd bootload in flash */
	ldr     r2,=text_start
	add	r2,r2,r0

1:      ldmia   r2!,{r4-r7}
     	stmia   r3!,{r4-r7}
        cmp     r1,r3
        bhi     1b

	/*
	 * copy exception vector table, bootloader running in poll mode, just in case exception happens
	 */
	ldr	r4,=trx_entry
	ldr	r2,=text_start
	sub	r1,r4,r2
	add	r2,r2,r0
	ldr	r3,=SI_ARM_SRAM2

	add	r1,r1,r3
2:      ldmia   r2!,{r4}
     	stmia   r3!,{r4}
        cmp     r1,r3
        bhi     2b

	ldr	r4,=trx_entry
	ldr 	r2,=text_start
	sub	r4,r4,r2
	ldr	r5,=SF_BOOTLOADER_RAM_ADDRESS
	add	r3,r5,r4
	ldr	r2,=SI_ARM_SRAM2
	sub	r3,r3,r2
	add	r3,r3,#1									/* set bit0 for thumb mode */

	/* jump to address of setup in RAM */
	mov	r0,r3
	bl	arm_jumpto									/* jump to trx_entry() in RAM */
/* Halt in case we ever back! */
rfm1:	TRACE(0x7777)
	b	.
END(sf_bootloader)
#endif /* SFLASH_BOOTLOADER */

#ifdef BCMDBG_ASSERT
/* toplevel SVC handler */
.global __tx_PendSVHandler
/*
 * Stack contains:
 * r0, r1, r2, r3, r12, r14, the return address and xPSR
 * First argument args[0] is (r0)
 */

FUNC(hnd_svctrap_handler)
	push	{r0}
	ldr	r0, [r0, #24]
	ldrb.w	r0, [r0, #-2]
	cmp	r0, #ASSERT_TRAP_SVC_NUMBER
	pop	{r0}
	bne.n	svc_trap
	b	trap
svc_trap:
	b	__tx_PendSVHandler
END(hnd_svctrap_handler)

#endif /* BCMDBG_ASSERT */
