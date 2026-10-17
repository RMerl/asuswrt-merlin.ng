/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) ASPEED Technology Inc.
 */
#include <config.h>
#include <asm/armv7.h>
#include <linux/linkage.h>
#include <asm/arch/scu_ast2600.h>

/* SCU register offsets */
#define SCU_BASE		0x1e6e2000
#define SCU_PROT_KEY1		(SCU_BASE + 0x000)
#define SCU_PROT_KEY2		(SCU_BASE + 0x010)
#define SCU_SMP_BOOT		(SCU_BASE + 0x180)
#define SCU_HWSTRAP1		(SCU_BASE + 0x510)
#define SCU_CA7_PARITY_CHK	(SCU_BASE + 0x820)
#define SCU_CA7_PARITY_CLR	(SCU_BASE + 0x824)
#define SCU_MMIO_DEC		(SCU_BASE + 0xc24)

/* FMC SPI register offsets */
#define FMC_BASE		0x1e620000
#define FMC_CE0_CTRL		(FMC_BASE + 0x010)
#define FMC_SW_RST_CTRL		(FMC_BASE + 0x050)
#define FMC_WDT1_CTRL_MODE	(FMC_BASE + 0x060)
#define FMC_WDT2_CTRL_MODE	(FMC_BASE + 0x064)

/*
 * The SMP mailbox provides a space with few instructions in it
 * for secondary cores to execute on and wait for the signal of
 * SMP core bring up.
 *
 *       SMP mailbox
 * +----------------------+
 * |                      |
 * | mailbox insn. for    |
 * | cpuN polling SMP go  |
 * |                      |
 * +----------------------+ 0xC
 * | mailbox ready signal |
 * +----------------------+ 0x8
 * | cpuN GO signal       |
 * +----------------------+ 0x4
 * | cpuN entrypoint      |
 * +----------------------+ SMP_MAILBOX_BASE
 */
#define SMP_MBOX_BASE		(SCU_SMP_BOOT)
#define SMP_MBOX_FIELD_ENTRY	(SMP_MBOX_BASE + 0x0)
#define SMP_MBOX_FIELD_GOSIGN	(SMP_MBOX_BASE + 0x4)
#define SMP_MBOX_FIELD_READY	(SMP_MBOX_BASE + 0x8)
#define SMP_MBOX_FIELD_POLLINSN	(SMP_MBOX_BASE + 0xc)

.macro scu_unlock
	movw	r0, #(SCU_UNLOCK_KEY & 0xffff)
	movt	r0, #(SCU_UNLOCK_KEY >> 16)

	ldr	r1, =SCU_PROT_KEY1
	str	r0, [r1]
	ldr	r1, =SCU_PROT_KEY2
	str	r0, [r1]
.endm

.macro timer_init
	ldr	r1, =SCU_HWSTRAP1
	ldr	r1, [r1]
	and	r1, #0x700
	lsr	r1, #0x8

	/* 1.2GHz */
	cmp	r1, #0x0
	movweq	r0, #0x8c00
	movteq	r0, #0x4786

	/* 1.6GHz */
	cmp	r1, #0x1
	movweq	r0, #0x1000
	movteq	r0, #0x5f5e

	/* 1.2GHz */
	cmp	r1, #0x2
	movweq	r0, #0x8c00
	movteq	r0, #0x4786

	/* 1.6GHz */
	cmp	r1, #0x3
	movweq	r0, #0x1000
	movteq	r0, #0x5f5e

	/* 800MHz */
	cmp	r1, #0x4
	movwge	r0, #0x0800
	movtge	r0, #0x2faf

	mcr	p15, 0, r0, c14, c0, 0	@; update CNTFRQ
.endm


.globl lowlevel_init

lowlevel_init:
#if defined(CONFIG_SPL) && !defined(CONFIG_SPL_BUILD)
	mov	pc, lr
#else
	/* setup ARM arch timer frequency */
	timer_init

	/* reset SMP mailbox as early as possible */
	mov	r0, #0x0
	ldr	r1, =SMP_MBOX_FIELD_READY
	str	r0, [r1]

	/* set ACTLR.SMP to enable cache use */
	mrc	p15, 0, r0, c1, c0, 1
	orr	r0, #0x40
	mcr	p15, 0, r0, c1, c0, 1

	/*
	 * we treat cpu0 as the primary core and
	 * put secondary core (cpuN) to sleep
	 */
	mrc   p15, 0, r0, c0, c0, 5	@; Read CPU ID register
	ands  r0, #0xff			@; Mask off, leaving the CPU ID field
	movw  r2, #0xab00
	movt  r2, #0xabba
	orr   r2, r0

	beq   do_primary_core_setup

	/* hold cpuN until mailbox is ready */
poll_mailbox_ready:
	wfe
	ldr	r0, =SMP_MBOX_FIELD_READY
	ldr	r0, [r0]
	movw	r1, #0xcafe
	movt	r1, #0xbabe
	cmp	r1, r0
	bne	poll_mailbox_ready

	/* parameters for relocated SMP go polling insn. */
	ldr	r0, =SMP_MBOX_FIELD_GOSIGN
	ldr	r1, =SMP_MBOX_FIELD_ENTRY

	/* no return */
	ldr	pc, =SMP_MBOX_FIELD_POLLINSN

do_primary_core_setup:
	scu_unlock

	/* MMIO decode setting */
	ldr	r0, =SCU_MMIO_DEC
	mov	r1, #0x2000
	str	r1, [r0]

	/* enable CA7 cache parity check */
	mov	r0, #0
	ldr	r1, =SCU_CA7_PARITY_CLR
	str	r0, [r1]

	mov	r0, #0x1
	ldr	r1, =SCU_CA7_PARITY_CHK
	str	r0, [r1]

	/* do not fill FMC50[1] if boot from eMMC */
	ldr	r0, =SCU_HWSTRAP1
	ldr	r1, [r0]
	ands	r1, #0x04
	bne	skip_fill_wip_bit

	/* fill FMC50[1] for waiting WIP idle */
	mov	r0, #0x02
	ldr	r1, =FMC_SW_RST_CTRL
	str	r0, [r1]

skip_fill_wip_bit:
	/* disable FMC WDT for SPI address mode detection */
	mov	r0, #0
	ldr	r1, =FMC_WDT1_CTRL_MODE
	str	r0, [r1]

	/* relocate mailbox insn. for cpuN polling SMP go signal */
	adrl	r0, mailbox_insn
	adrl	r1, mailbox_insn_end

	ldr	r2, =#SMP_MBOX_FIELD_POLLINSN

relocate_mailbox_insn:
	ldr	r3, [r0], #0x4
	str	r3, [r2], #0x4
	cmp	r0, r1
	bne	relocate_mailbox_insn

	/* reset SMP go sign */
	mov	r0, #0
	ldr	r1, =SMP_MBOX_FIELD_GOSIGN
	str	r0, [r1]

	/* notify cpuN mailbox is ready */
	movw	r0, #0xCAFE
	movt	r0, #0xBABE
	ldr	r1, =SMP_MBOX_FIELD_READY
	str	r0, [r1]
	sev

	/* back to arch calling code */
	mov	pc, lr

/*
 * insn. inside mailbox to poll SMP go signal.
 *
 * Note that as this code will be relocated, any
 * pc-relative assembly should NOT be used.
 */
mailbox_insn:
	/*
	 * r0 ~ r3 are parameters:
	 *  r0 = SMP_MBOX_FIELD_GOSIGN
	 *  r1 = SMP_MBOX_FIELD_ENTRY
	 *  r2 = per-cpu go sign value
	 *  r3 = no used now
	 */
poll_mailbox_smp_go:
	wfe
	ldr	r4, [r0]
	cmp	r2, r4
	bne	poll_mailbox_smp_go

	/* SMP GO signal confirmed, release cpuN */
	ldr	pc, [r1]

mailbox_insn_end:
	/* should never reach */
	b	.

#endif
