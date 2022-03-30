/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 1998  Dan Malek <dmalek@jlc.net>
 * Copyright (C) 1999  Magnus Damm <kieraypc01.p.y.kie.era.ericsson.se>
 * Copyright (C) 2000, 2001,2002 Wolfgang Denk <wd@denx.de>
 * Copyright Freescale Semiconductor, Inc. 2004, 2006, 2008.
 */

/*
 *  U-Boot - Startup Code for MPC83xx PowerPC based Embedded Boards
 */

#include <asm-offsets.h>
#include <config.h>
#include <mpc83xx.h>
#include <version.h>

#define CONFIG_83XX	1		/* needed for Linux kernel header files*/

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#include <asm/cache.h>
#include <asm/mmu.h>
#include <asm/u-boot.h>

#include "hrcw/hrcw.h"
#include "bats/bats.h"
#include "hid/hid.h"

/* We don't want the  MMU yet.
 */
#undef	MSR_KERNEL

/*
 * Floating Point enable, Machine Check and Recoverable Interr.
 */
#ifdef DEBUG
#define MSR_KERNEL (MSR_FP|MSR_RI)
#else
#define MSR_KERNEL (MSR_FP|MSR_ME|MSR_RI)
#endif

#if defined(CONFIG_NAND_SPL) || \
	(defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_INIT_MINIMAL))
#define MINIMAL_SPL
#endif

#if !defined(CONFIG_SPL_BUILD) && !defined(CONFIG_NAND_SPL) && \
	!defined(CONFIG_SYS_RAMBOOT)
#define CONFIG_SYS_FLASHBOOT
#endif

/*
 * Set up GOT: Global Offset Table
 *
 * Use r12 to access the GOT
 */
	START_GOT
	GOT_ENTRY(_GOT2_TABLE_)
	GOT_ENTRY(__bss_start)
	GOT_ENTRY(__bss_end)

#ifndef MINIMAL_SPL
	GOT_ENTRY(_FIXUP_TABLE_)
	GOT_ENTRY(_start)
	GOT_ENTRY(_start_of_vectors)
	GOT_ENTRY(_end_of_vectors)
	GOT_ENTRY(transfer_to_handler)
#endif
	END_GOT

/*
 * The Hard Reset Configuration Word (HRCW) table is in the first 64
 * (0x40) bytes of flash.  It has 8 bytes, but each byte is repeated 8
 * times so the processor can fetch it out of flash whether the flash
 * is 8, 16, 32, or 64 bits wide (hardware trickery).
 */
	.text
#define _HRCW_TABLE_ENTRY(w)		\
	.fill	8,1,(((w)>>24)&0xff);	\
	.fill	8,1,(((w)>>16)&0xff);	\
	.fill	8,1,(((w)>> 8)&0xff);	\
	.fill	8,1,(((w)    )&0xff)

	_HRCW_TABLE_ENTRY(CONFIG_SYS_HRCW_LOW)
	_HRCW_TABLE_ENTRY(CONFIG_SYS_HRCW_HIGH)

/*
 * Magic number and version string - put it after the HRCW since it
 * cannot be first in flash like it is in many other processors.
 */
	.long	0x27051956		/* U-Boot Magic Number */

	.globl	version_string
version_string:
	.ascii U_BOOT_VERSION_STRING, "\0"

	.align 2

	.globl enable_addr_trans
enable_addr_trans:
	/* enable address translation */
	mfmsr	r5
	ori	r5, r5, (MSR_IR | MSR_DR)
	mtmsr	r5
	isync
	blr

	.globl disable_addr_trans
disable_addr_trans:
	/* disable address translation */
	mflr	r4
	mfmsr	r3
	andi.	r0, r3, (MSR_IR | MSR_DR)
	beqlr
	andc	r3, r3, r0
	mtspr	SRR0, r4
	mtspr	SRR1, r3
	rfi

#ifndef CONFIG_DEFAULT_IMMR
#error CONFIG_DEFAULT_IMMR must be defined
#endif /* CONFIG_DEFAULT_IMMR */
#ifndef CONFIG_SYS_IMMR
#define CONFIG_SYS_IMMR CONFIG_DEFAULT_IMMR
#endif /* CONFIG_SYS_IMMR */

/*
 * After configuration, a system reset exception is executed using the
 * vector at offset 0x100 relative to the base set by MSR[IP]. If
 * MSR[IP] is 0, the base address is 0x00000000. If MSR[IP] is 1, the
 * base address is 0xfff00000. In the case of a Power On Reset or Hard
 * Reset, the value of MSR[IP] is determined by the CIP field in the
 * HRCW.
 *
 * Other bits in the HRCW set up the Base Address and Port Size in BR0.
 * This determines the location of the boot ROM (flash or EPROM) in the
 * processor's address space at boot time. As long as the HRCW is set up
 * so that we eventually end up executing the code below when the
 * processor executes the reset exception, the actual values used should
 * not matter.
 *
 * Once we have got here, the address mask in OR0 is cleared so that the
 * bottom 32K of the boot ROM is effectively repeated all throughout the
 * processor's address space, after which we can jump to the absolute
 * address at which the boot ROM was linked at compile time, and proceed
 * to initialise the memory controller without worrying if the rug will
 * be pulled out from under us, so to speak (it will be fine as long as
 * we configure BR0 with the same boot ROM link address).
 */
	. = EXC_OFF_SYS_RESET

	.globl	_start
_start: /* time t 0 */
	lis	r4, CONFIG_DEFAULT_IMMR@h
	nop

	mfmsr	r5			/* save msr contents	*/

	/* 83xx manuals prescribe a specific sequence for updating IMMRBAR. */
	bl	1f
1:	mflr	r7

	lis	r3, CONFIG_SYS_IMMR@h
	ori	r3, r3, CONFIG_SYS_IMMR@l

	lwz	r6, IMMRBAR(r4)
	isync

	stw	r3, IMMRBAR(r4)
	lwz	r6, 0(r7)		/* Arbitrary external load */
	isync

	lwz	r6, IMMRBAR(r3)
	isync

	/* Initialise the E300 processor core		*/
	/*------------------------------------------*/

#if (defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_MPC83XX_WAIT_FOR_NAND)) || \
		defined(CONFIG_NAND_SPL)
	/* The FCM begins execution after only the first page
	 * is loaded.  Wait for the rest before branching
	 * to another flash page.
	 */
1:	lwz	r6, 0x50b0(r3)
	andi.	r6, r6, 1
	beq	1b
#endif

	bl	init_e300_core

#ifdef CONFIG_SYS_FLASHBOOT

	/* Inflate flash location so it appears everywhere, calculate */
	/* the absolute address in final location of the FLASH, jump  */
	/* there and deflate the flash size back to minimal size      */
	/*------------------------------------------------------------*/
	bl map_flash_by_law1
	lis r4, (CONFIG_SYS_MONITOR_BASE)@h
	ori r4, r4, (CONFIG_SYS_MONITOR_BASE)@l
	addi r5, r4, in_flash - _start + EXC_OFF_SYS_RESET
	mtlr r5
	blr
in_flash:
#if 1 /* Remapping flash with LAW0. */
	bl remap_flash_by_law0
#endif
#endif	/* CONFIG_SYS_FLASHBOOT */

	/* setup the bats */
	bl	setup_bats
	sync

	/*
	 * Cache must be enabled here for stack-in-cache trick.
	 * This means we need to enable the BATS.
	 * This means:
	 *   1) for the EVB, original gt regs need to be mapped
	 *   2) need to have an IBAT for the 0xf region,
	 *      we are running there!
	 * Cache should be turned on after BATs, since by default
	 * everything is write-through.
	 * The init-mem BAT can be reused after reloc. The old
	 * gt-regs BAT can be reused after board_init_f calls
	 * board_early_init_f (EVB only).
	 */
	/* enable address translation */
	bl	enable_addr_trans
	sync

	/* enable the data cache */
	bl	dcache_enable
	sync
#ifdef CONFIG_SYS_INIT_RAM_LOCK
	bl	lock_ram_in_cache
	sync
#endif

	/* set up the stack pointer in our newly created
	 * cache-ram; use r3 to keep the new SP for now to
	 * avoid overiding the SP it uselessly */
	lis	r3, (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_GBL_DATA_OFFSET)@h
	ori	r3, r3, (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_GBL_DATA_OFFSET)@l

	/* r4 = end of GD area */
	addi r4, r3, GENERATED_GBL_DATA_SIZE

	/* Zero GD area */
	li	r0, 0
1:
	subi	r4, r4, 1
	stb	r0, 0(r4)
	cmplw	r3, r4
	bne	1b

#if CONFIG_VAL(SYS_MALLOC_F_LEN)

#if CONFIG_VAL(SYS_MALLOC_F_LEN) + GENERATED_GBL_DATA_SIZE > CONFIG_SYS_INIT_RAM_SIZE
#error "SYS_MALLOC_F_LEN too large to fit into initial RAM."
#endif

	/* r3 = new stack pointer / pre-reloc malloc area */
	subi    r3, r3, CONFIG_VAL(SYS_MALLOC_F_LEN)

	/* Set pointer to pre-reloc malloc area in GD */
	stw     r3, GD_MALLOC_BASE(r4)
#endif
	li	r0, 0		/* Make room for stack frame header and	*/
	stwu	r0, -4(r3)	/* clear final stack frame so that	*/
	stwu	r0, -4(r3)	/* stack backtraces terminate cleanly	*/

	/* Finally, actually set SP */
	mr	r1, r3

	/* let the C-code set up the rest	                    */
	/*				                            */
	/* Be careful to keep code relocatable & stack humble   */
	/*------------------------------------------------------*/

	GET_GOT			/* initialize GOT access	*/
	/* Needed for -msingle-pic-base */
	bl	_GLOBAL_OFFSET_TABLE_@local-4
	mflr	r30

	/* r3: IMMR */
	lis	r3, CONFIG_SYS_IMMR@h
	/* run low-level CPU init code (in Flash)*/
	bl	cpu_init_f

	/* run 1st part of board init code (in Flash)*/
	li	r3, 0		/* clear boot_flag for calling board_init_f */
	bl	board_init_f

	/* NOTREACHED - board_init_f() does not return */

#ifndef MINIMAL_SPL
/*
 * Vector Table
 */

	.globl	_start_of_vectors
_start_of_vectors:

/* Machine check */
	STD_EXCEPTION(0x200, MachineCheck, MachineCheckException)

/* Data Storage exception. */
	STD_EXCEPTION(0x300, DataStorage, UnknownException)

/* Instruction Storage exception. */
	STD_EXCEPTION(0x400, InstStorage, UnknownException)

/* External Interrupt exception. */
#ifndef FIXME
	STD_EXCEPTION(0x500, ExtInterrupt, external_interrupt)
#endif

/* Alignment exception. */
	. = 0x600
Alignment:
	EXCEPTION_PROLOG(SRR0, SRR1)
	mfspr	r4,DAR
	stw	r4,_DAR(r21)
	mfspr	r5,DSISR
	stw	r5,_DSISR(r21)
	addi	r3,r1,STACK_FRAME_OVERHEAD
	EXC_XFER_TEMPLATE(Alignment, AlignmentException, MSR_KERNEL, COPY_EE)

/* Program check exception */
	. = 0x700
ProgramCheck:
	EXCEPTION_PROLOG(SRR0, SRR1)
	addi	r3,r1,STACK_FRAME_OVERHEAD
	EXC_XFER_TEMPLATE(ProgramCheck, ProgramCheckException,
		MSR_KERNEL, COPY_EE)

	STD_EXCEPTION(0x800, FPUnavailable, UnknownException)

	/* I guess we could implement decrementer, and may have
	 * to someday for timekeeping.
	 */
	STD_EXCEPTION(0x900, Decrementer, timer_interrupt)

	STD_EXCEPTION(0xa00, Trap_0a, UnknownException)
	STD_EXCEPTION(0xb00, Trap_0b, UnknownException)
	STD_EXCEPTION(0xc00, SystemCall, UnknownException)
	STD_EXCEPTION(0xd00, SingleStep, UnknownException)

	STD_EXCEPTION(0xe00, Trap_0e, UnknownException)
	STD_EXCEPTION(0xf00, Trap_0f, UnknownException)

	STD_EXCEPTION(0x1000, InstructionTLBMiss, UnknownException)
	STD_EXCEPTION(0x1100, DataLoadTLBMiss, UnknownException)
	STD_EXCEPTION(0x1200, DataStoreTLBMiss, UnknownException)
#ifdef DEBUG
	. = 0x1300
	/*
	 * This exception occurs when the program counter matches the
	 * Instruction Address Breakpoint Register (IABR).
	 *
	 * I want the cpu to halt if this occurs so I can hunt around
	 * with the debugger and look at things.
	 *
	 * When DEBUG is defined, both machine check enable (in the MSR)
	 * and checkstop reset enable (in the reset mode register) are
	 * turned off and so a checkstop condition will result in the cpu
	 * halting.
	 *
	 * I force the cpu into a checkstop condition by putting an illegal
	 * instruction here (at least this is the theory).
	 *
	 * well - that didnt work, so just do an infinite loop!
	 */
1:	b	1b
#else
	STD_EXCEPTION(0x1300, InstructionBreakpoint, DebugException)
#endif
	STD_EXCEPTION(0x1400, SMI, UnknownException)

	STD_EXCEPTION(0x1500, Trap_15, UnknownException)
	STD_EXCEPTION(0x1600, Trap_16, UnknownException)
	STD_EXCEPTION(0x1700, Trap_17, UnknownException)
	STD_EXCEPTION(0x1800, Trap_18, UnknownException)
	STD_EXCEPTION(0x1900, Trap_19, UnknownException)
	STD_EXCEPTION(0x1a00, Trap_1a, UnknownException)
	STD_EXCEPTION(0x1b00, Trap_1b, UnknownException)
	STD_EXCEPTION(0x1c00, Trap_1c, UnknownException)
	STD_EXCEPTION(0x1d00, Trap_1d, UnknownException)
	STD_EXCEPTION(0x1e00, Trap_1e, UnknownException)
	STD_EXCEPTION(0x1f00, Trap_1f, UnknownException)
	STD_EXCEPTION(0x2000, Trap_20, UnknownException)
	STD_EXCEPTION(0x2100, Trap_21, UnknownException)
	STD_EXCEPTION(0x2200, Trap_22, UnknownException)
	STD_EXCEPTION(0x2300, Trap_23, UnknownException)
	STD_EXCEPTION(0x2400, Trap_24, UnknownException)
	STD_EXCEPTION(0x2500, Trap_25, UnknownException)
	STD_EXCEPTION(0x2600, Trap_26, UnknownException)
	STD_EXCEPTION(0x2700, Trap_27, UnknownException)
	STD_EXCEPTION(0x2800, Trap_28, UnknownException)
	STD_EXCEPTION(0x2900, Trap_29, UnknownException)
	STD_EXCEPTION(0x2a00, Trap_2a, UnknownException)
	STD_EXCEPTION(0x2b00, Trap_2b, UnknownException)
	STD_EXCEPTION(0x2c00, Trap_2c, UnknownException)
	STD_EXCEPTION(0x2d00, Trap_2d, UnknownException)
	STD_EXCEPTION(0x2e00, Trap_2e, UnknownException)
	STD_EXCEPTION(0x2f00, Trap_2f, UnknownException)


	.globl	_end_of_vectors
_end_of_vectors:

	. = 0x3000

/*
 * This code finishes saving the registers to the exception frame
 * and jumps to the appropriate handler for the exception.
 * Register r21 is pointer into trap frame, r1 has new stack pointer.
 */
	.globl	transfer_to_handler
transfer_to_handler:
	stw	r22,_NIP(r21)
	lis	r22,MSR_POW@h
	andc	r23,r23,r22
	stw	r23,_MSR(r21)
	SAVE_GPR(7, r21)
	SAVE_4GPRS(8, r21)
	SAVE_8GPRS(12, r21)
	SAVE_8GPRS(24, r21)
	mflr	r23
	andi.	r24,r23,0x3f00		/* get vector offset */
	stw	r24,TRAP(r21)
	li	r22,0
	stw	r22,RESULT(r21)
	lwz	r24,0(r23)		/* virtual address of handler */
	lwz	r23,4(r23)		/* where to go when done */
	mtspr	SRR0,r24
	mtspr	SRR1,r20
	mtlr	r23
	SYNC
	rfi				/* jump to handler, enable MMU */

int_return:
	mfmsr	r28		/* Disable interrupts */
	li	r4,0
	ori	r4,r4,MSR_EE
	andc	r28,r28,r4
	SYNC			/* Some chip revs need this... */
	mtmsr	r28
	SYNC
	lwz	r2,_CTR(r1)
	lwz	r0,_LINK(r1)
	mtctr	r2
	mtlr	r0
	lwz	r2,_XER(r1)
	lwz	r0,_CCR(r1)
	mtspr	XER,r2
	mtcrf	0xFF,r0
	REST_10GPRS(3, r1)
	REST_10GPRS(13, r1)
	REST_8GPRS(23, r1)
	REST_GPR(31, r1)
	lwz	r2,_NIP(r1)	/* Restore environment */
	lwz	r0,_MSR(r1)
	mtspr	SRR0,r2
	mtspr	SRR1,r0
	lwz	r0,GPR0(r1)
	lwz	r2,GPR2(r1)
	lwz	r1,GPR1(r1)
	SYNC
	rfi
#endif /* !MINIMAL_SPL */

/*
 * This code initialises the E300 processor core
 * (conforms to PowerPC 603e spec)
 * Note: expects original MSR contents to be in r5.
 */
	.globl	init_e300_core
init_e300_core: /* time t 10 */
	/* Initialize machine status; enable machine check interrupt */
	/*-----------------------------------------------------------*/

	li	r3, MSR_KERNEL			/* Set ME and RI flags */
	rlwimi	r3, r5, 0, 25, 25	/* preserve IP bit set by HRCW */
#ifdef DEBUG
	rlwimi	r3, r5, 0, 21, 22   /* debugger might set SE & BE bits */
#endif
	SYNC						/* Some chip revs need this... */
	mtmsr	r3
	SYNC
	mtspr	SRR1, r3			/* Make SRR1 match MSR */


	lis	r3, CONFIG_SYS_IMMR@h
#if defined(CONFIG_WATCHDOG)
	/* Initialise the Watchdog values and reset it (if req) */
	/*------------------------------------------------------*/
	lis r4, CONFIG_SYS_WATCHDOG_VALUE
	ori r4, r4, (SWCRR_SWEN | SWCRR_SWRI | SWCRR_SWPR)
	stw r4, SWCRR(r3)

	/* and reset it */

	li	r4, 0x556C
	sth	r4, SWSRR@l(r3)
	li	r4, -0x55C7
	sth	r4, SWSRR@l(r3)
#else
	/* Disable Watchdog  */
	/*-------------------*/
	lwz r4, SWCRR(r3)
	/* Check to see if its enabled for disabling
	   once disabled by SW you can't re-enable */
	andi. r4, r4, 0x4
	beq 1f
	xor r4, r4, r4
	stw r4, SWCRR(r3)
1:
#endif /* CONFIG_WATCHDOG */

#if defined(CONFIG_MASK_AER_AO)
	/* Write the Arbiter Event Enable to mask Address Only traps. */
	/* This prevents the dcbz instruction from being trapped when */
	/* HID0_ABE Address Broadcast Enable is set and the MEMORY    */
	/* COHERENCY bit is set in the WIMG bits, which is often      */
	/* needed for PCI operation.                                  */
	lwz	r4, 0x0808(r3)
	rlwinm	r0, r4, 0, ~AER_AO
	stw	r0, 0x0808(r3)
#endif /* CONFIG_MASK_AER_AO */

	/* Initialize the Hardware Implementation-dependent Registers */
	/* HID0 also contains cache control			*/
	/* - force invalidation of data and instruction caches  */
	/*------------------------------------------------------*/

	lis	r3, CONFIG_SYS_HID0_INIT@h
	ori	r3, r3, (CONFIG_SYS_HID0_INIT | HID0_ICFI | HID0_DCFI)@l
	SYNC
	mtspr	HID0, r3

	lis	r3, CONFIG_SYS_HID0_FINAL@h
	ori	r3, r3, (CONFIG_SYS_HID0_FINAL & ~(HID0_ICFI | HID0_DCFI))@l
	SYNC
	mtspr	HID0, r3

	lis	r3, CONFIG_SYS_HID2@h
	ori	r3, r3, CONFIG_SYS_HID2@l
	SYNC
	mtspr	HID2, r3

	/* Done!						*/
	/*------------------------------*/
	blr

	/* setup_bats - set them up to some initial state */
	.globl	setup_bats
setup_bats:
	addis	r0, r0, 0x0000

	/* IBAT 0 */
	addis	r4, r0, CONFIG_SYS_IBAT0L@h
	ori	r4, r4, CONFIG_SYS_IBAT0L@l
	addis	r3, r0, CONFIG_SYS_IBAT0U@h
	ori	r3, r3, CONFIG_SYS_IBAT0U@l
	mtspr	IBAT0L, r4
	mtspr	IBAT0U, r3

	/* DBAT 0 */
	addis	r4, r0, CONFIG_SYS_DBAT0L@h
	ori	r4, r4, CONFIG_SYS_DBAT0L@l
	addis	r3, r0, CONFIG_SYS_DBAT0U@h
	ori	r3, r3, CONFIG_SYS_DBAT0U@l
	mtspr	DBAT0L, r4
	mtspr	DBAT0U, r3

	/* IBAT 1 */
	addis	r4, r0, CONFIG_SYS_IBAT1L@h
	ori	r4, r4, CONFIG_SYS_IBAT1L@l
	addis	r3, r0, CONFIG_SYS_IBAT1U@h
	ori	r3, r3, CONFIG_SYS_IBAT1U@l
	mtspr	IBAT1L, r4
	mtspr	IBAT1U, r3

	/* DBAT 1 */
	addis	r4, r0, CONFIG_SYS_DBAT1L@h
	ori	r4, r4, CONFIG_SYS_DBAT1L@l
	addis	r3, r0, CONFIG_SYS_DBAT1U@h
	ori	r3, r3, CONFIG_SYS_DBAT1U@l
	mtspr	DBAT1L, r4
	mtspr	DBAT1U, r3

	/* IBAT 2 */
	addis	r4, r0, CONFIG_SYS_IBAT2L@h
	ori	r4, r4, CONFIG_SYS_IBAT2L@l
	addis	r3, r0, CONFIG_SYS_IBAT2U@h
	ori	r3, r3, CONFIG_SYS_IBAT2U@l
	mtspr	IBAT2L, r4
	mtspr	IBAT2U, r3

	/* DBAT 2 */
	addis	r4, r0, CONFIG_SYS_DBAT2L@h
	ori	r4, r4, CONFIG_SYS_DBAT2L@l
	addis	r3, r0, CONFIG_SYS_DBAT2U@h
	ori	r3, r3, CONFIG_SYS_DBAT2U@l
	mtspr	DBAT2L, r4
	mtspr	DBAT2U, r3

	/* IBAT 3 */
	addis	r4, r0, CONFIG_SYS_IBAT3L@h
	ori	r4, r4, CONFIG_SYS_IBAT3L@l
	addis	r3, r0, CONFIG_SYS_IBAT3U@h
	ori	r3, r3, CONFIG_SYS_IBAT3U@l
	mtspr	IBAT3L, r4
	mtspr	IBAT3U, r3

	/* DBAT 3 */
	addis	r4, r0, CONFIG_SYS_DBAT3L@h
	ori	r4, r4, CONFIG_SYS_DBAT3L@l
	addis	r3, r0, CONFIG_SYS_DBAT3U@h
	ori	r3, r3, CONFIG_SYS_DBAT3U@l
	mtspr	DBAT3L, r4
	mtspr	DBAT3U, r3

#ifdef CONFIG_HIGH_BATS
	/* IBAT 4 */
	addis   r4, r0, CONFIG_SYS_IBAT4L@h
	ori     r4, r4, CONFIG_SYS_IBAT4L@l
	addis   r3, r0, CONFIG_SYS_IBAT4U@h
	ori     r3, r3, CONFIG_SYS_IBAT4U@l
	mtspr   IBAT4L, r4
	mtspr   IBAT4U, r3

	/* DBAT 4 */
	addis   r4, r0, CONFIG_SYS_DBAT4L@h
	ori     r4, r4, CONFIG_SYS_DBAT4L@l
	addis   r3, r0, CONFIG_SYS_DBAT4U@h
	ori     r3, r3, CONFIG_SYS_DBAT4U@l
	mtspr   DBAT4L, r4
	mtspr   DBAT4U, r3

	/* IBAT 5 */
	addis   r4, r0, CONFIG_SYS_IBAT5L@h
	ori     r4, r4, CONFIG_SYS_IBAT5L@l
	addis   r3, r0, CONFIG_SYS_IBAT5U@h
	ori     r3, r3, CONFIG_SYS_IBAT5U@l
	mtspr   IBAT5L, r4
	mtspr   IBAT5U, r3

	/* DBAT 5 */
	addis   r4, r0, CONFIG_SYS_DBAT5L@h
	ori     r4, r4, CONFIG_SYS_DBAT5L@l
	addis   r3, r0, CONFIG_SYS_DBAT5U@h
	ori     r3, r3, CONFIG_SYS_DBAT5U@l
	mtspr   DBAT5L, r4
	mtspr   DBAT5U, r3

	/* IBAT 6 */
	addis   r4, r0, CONFIG_SYS_IBAT6L@h
	ori     r4, r4, CONFIG_SYS_IBAT6L@l
	addis   r3, r0, CONFIG_SYS_IBAT6U@h
	ori     r3, r3, CONFIG_SYS_IBAT6U@l
	mtspr   IBAT6L, r4
	mtspr   IBAT6U, r3

	/* DBAT 6 */
	addis   r4, r0, CONFIG_SYS_DBAT6L@h
	ori     r4, r4, CONFIG_SYS_DBAT6L@l
	addis   r3, r0, CONFIG_SYS_DBAT6U@h
	ori     r3, r3, CONFIG_SYS_DBAT6U@l
	mtspr   DBAT6L, r4
	mtspr   DBAT6U, r3

	/* IBAT 7 */
	addis   r4, r0, CONFIG_SYS_IBAT7L@h
	ori     r4, r4, CONFIG_SYS_IBAT7L@l
	addis   r3, r0, CONFIG_SYS_IBAT7U@h
	ori     r3, r3, CONFIG_SYS_IBAT7U@l
	mtspr   IBAT7L, r4
	mtspr   IBAT7U, r3

	/* DBAT 7 */
	addis   r4, r0, CONFIG_SYS_DBAT7L@h
	ori     r4, r4, CONFIG_SYS_DBAT7L@l
	addis   r3, r0, CONFIG_SYS_DBAT7U@h
	ori     r3, r3, CONFIG_SYS_DBAT7U@l
	mtspr   DBAT7L, r4
	mtspr   DBAT7U, r3
#endif

	isync

	/* invalidate all tlb's
	 *
	 * From the 603e User Manual: "The 603e provides the ability to
	 * invalidate a TLB entry. The TLB Invalidate Entry (tlbie)
	 * instruction invalidates the TLB entry indexed by the EA, and
	 * operates on both the instruction and data TLBs simultaneously
	 * invalidating four TLB entries (both sets in each TLB). The
	 * index corresponds to bits 15-19 of the EA. To invalidate all
	 * entries within both TLBs, 32 tlbie instructions should be
	 * issued, incrementing this field by one each time."
	 *
	 * "Note that the tlbia instruction is not implemented on the
	 * 603e."
	 *
	 * bits 15-19 correspond to addresses 0x00000000 to 0x0001F000
	 * incrementing by 0x1000 each time. The code below is sort of
	 * based on code in "flush_tlbs" from arch/powerpc/kernel/head.S
	 *
	 */
	lis	r3, 0
	lis	r5, 2

1:
	tlbie	r3
	addi	r3, r3, 0x1000
	cmp	0, 0, r3, r5
	blt	1b

	blr

/* Cache functions.
 *
 * Note: requires that all cache bits in
 * HID0 are in the low half word.
 */
#ifndef MINIMAL_SPL
	.globl	icache_enable
icache_enable:
	mfspr	r3, HID0
	ori	r3, r3, HID0_ICE
	li	r4, HID0_ICFI|HID0_ILOCK
	andc	r3, r3, r4
	ori	r4, r3, HID0_ICFI
	isync
	mtspr	HID0, r4    /* sets enable and invalidate, clears lock */
	isync
	mtspr	HID0, r3	/* clears invalidate */
	blr

	.globl	icache_disable
icache_disable:
	mfspr	r3, HID0
	lis	r4, 0
	ori	r4, r4, HID0_ICE|HID0_ICFI|HID0_ILOCK
	andc	r3, r3, r4
	isync
	mtspr	HID0, r3	/* clears invalidate, enable and lock */
	blr

	.globl	icache_status
icache_status:
	mfspr	r3, HID0
	rlwinm	r3, r3, (31 - HID0_ICE_SHIFT + 1), 31, 31
	blr
#endif	/* !MINIMAL_SPL */

	.globl	dcache_enable
dcache_enable:
	mfspr	r3, HID0
	li	r5, HID0_DCFI|HID0_DLOCK
	andc	r3, r3, r5
	ori	r3, r3, HID0_DCE
	sync
	mtspr	HID0, r3		/* enable, no invalidate */
	blr

	.globl	dcache_disable
dcache_disable:
	mflr	r4
	bl	flush_dcache		/* uses r3 and r5 */
	mfspr	r3, HID0
	li	r5, HID0_DCE|HID0_DLOCK
	andc	r3, r3, r5
	ori	r5, r3, HID0_DCFI
	sync
	mtspr	HID0, r5	/* sets invalidate, clears enable and lock */
	sync
	mtspr	HID0, r3	/* clears invalidate */
	mtlr	r4
	blr

	.globl	dcache_status
dcache_status:
	mfspr	r3, HID0
	rlwinm	r3, r3, (31 - HID0_DCE_SHIFT + 1), 31, 31
	blr

	.globl	flush_dcache
flush_dcache:
	lis	r3, 0
	lis	r5, CONFIG_SYS_CACHELINE_SIZE
1:	cmp	0, 1, r3, r5
	bge	2f
	lwz	r5, 0(r3)
	lis	r5, CONFIG_SYS_CACHELINE_SIZE
	addi	r3, r3, 0x4
	b	1b
2:	blr

/*-------------------------------------------------------------------*/

/*
 * void relocate_code (addr_sp, gd, addr_moni)
 *
 * This "function" does not return, instead it continues in RAM
 * after relocating the monitor code.
 *
 * r3 = dest
 * r4 = src
 * r5 = length in bytes
 * r6 = cachelinesize
 */
	.globl	relocate_code
relocate_code:
	mr	r1,  r3		/* Set new stack pointer	*/
	mr	r9,  r4		/* Save copy of Global Data pointer */
	mr	r10, r5		/* Save copy of Destination Address */

	GET_GOT
	mr	r3,  r5				/* Destination Address */
	lis	r4, CONFIG_SYS_MONITOR_BASE@h		/* Source      Address */
	ori	r4, r4, CONFIG_SYS_MONITOR_BASE@l
	lwz	r5, GOT(__bss_start)
	sub	r5, r5, r4
	li	r6, CONFIG_SYS_CACHELINE_SIZE		/* Cache Line Size */

	/*
	 * Fix GOT pointer:
	 *
	 * New GOT-PTR = (old GOT-PTR - CONFIG_SYS_MONITOR_BASE)
	 *		+ Destination Address
	 *
	 * Offset:
	 */
	sub	r15, r10, r4

	/* First our own GOT */
	add	r12, r12, r15
	/* then the one used by the C code */
	add	r30, r30, r15

	/*
	 * Now relocate code
	 */

	cmplw	cr1,r3,r4
	addi	r0,r5,3
	srwi.	r0,r0,2
	beq	cr1,4f		/* In place copy is not necessary */
	beq	7f		/* Protect against 0 count	  */
	mtctr	r0
	bge	cr1,2f
	la	r8,-4(r4)
	la	r7,-4(r3)

	/* copy */
1:	lwzu	r0,4(r8)
	stwu	r0,4(r7)
	bdnz	1b

	addi	r0,r5,3
	srwi.	r0,r0,2
	mtctr	r0
	la	r8,-4(r4)
	la	r7,-4(r3)

	/* and compare */
20:	lwzu	r20,4(r8)
	lwzu	r21,4(r7)
	xor. r22, r20, r21
	bne  30f
	bdnz	20b
	b 4f

	/* compare failed */
30:	li r3, 0
	blr

2:	slwi	r0,r0,2 /* re copy in reverse order ... y do we needed it? */
	add	r8,r4,r0
	add	r7,r3,r0
3:	lwzu	r0,-4(r8)
	stwu	r0,-4(r7)
	bdnz	3b

/*
 * Now flush the cache: note that we must start from a cache aligned
 * address. Otherwise we might miss one cache line.
 */
4:	cmpwi	r6,0
	add	r5,r3,r5
	beq	7f		/* Always flush prefetch queue in any case */
	subi	r0,r6,1
	andc	r3,r3,r0
	mr	r4,r3
5:	dcbst	0,r4
	add	r4,r4,r6
	cmplw	r4,r5
	blt	5b
	sync			/* Wait for all dcbst to complete on bus */
	mr	r4,r3
6:	icbi	0,r4
	add	r4,r4,r6
	cmplw	r4,r5
	blt	6b
7:	sync			/* Wait for all icbi to complete on bus	*/
	isync

/*
 * We are done. Do not return, instead branch to second part of board
 * initialization, now running from RAM.
 */
	addi	r0, r10, in_ram - _start + EXC_OFF_SYS_RESET
	mtlr	r0
	blr

in_ram:

	/*
	 * Relocation Function, r12 point to got2+0x8000
	 *
	 * Adjust got2 pointers, no need to check for 0, this code
	 * already puts a few entries in the table.
	 */
	li	r0,__got2_entries@sectoff@l
	la	r3,GOT(_GOT2_TABLE_)
	lwz	r11,GOT(_GOT2_TABLE_)
	mtctr	r0
	sub	r11,r3,r11
	addi	r3,r3,-4
1:	lwzu	r0,4(r3)
	cmpwi	r0,0
	beq-	2f
	add	r0,r0,r11
	stw	r0,0(r3)
2:	bdnz	1b

#ifndef MINIMAL_SPL
	/*
	 * Now adjust the fixups and the pointers to the fixups
	 * in case we need to move ourselves again.
	 */
	li	r0,__fixup_entries@sectoff@l
	lwz	r3,GOT(_FIXUP_TABLE_)
	cmpwi	r0,0
	mtctr	r0
	addi	r3,r3,-4
	beq	4f
3:	lwzu	r4,4(r3)
	lwzux	r0,r4,r11
	cmpwi	r0,0
	add	r0,r0,r11
	stw	r4,0(r3)
	beq-	5f
	stw	r0,0(r4)
5:	bdnz	3b
4:
#endif

clear_bss:
	/*
	 * Now clear BSS segment
	 */
	lwz	r3,GOT(__bss_start)
	lwz	r4,GOT(__bss_end)

	cmplw	0, r3, r4
	beq	6f

	li	r0, 0
5:
	stw	r0, 0(r3)
	addi	r3, r3, 4
	cmplw	0, r3, r4
	bne	5b
6:

	mr	r3, r9		/* Global Data pointer		*/
	mr	r4, r10		/* Destination Address		*/
	bl	board_init_r

#ifndef MINIMAL_SPL
	/*
	 * Copy exception vector code to low memory
	 *
	 * r3: dest_addr
	 * r7: source address, r8: end address, r9: target address
	 */
	.globl	trap_init
trap_init:
	mflr	r4		/* save link register */
	GET_GOT
	lwz	r7, GOT(_start)
	lwz	r8, GOT(_end_of_vectors)

	li	r9, 0x100	/* reset vector always at 0x100 */

	cmplw	0, r7, r8
	bgelr			/* return if r7>=r8 - just in case */
1:
	lwz	r0, 0(r7)
	stw	r0, 0(r9)
	addi	r7, r7, 4
	addi	r9, r9, 4
	cmplw	0, r7, r8
	bne	1b

	/*
	 * relocate `hdlr' and `int_return' entries
	 */
	li	r7, .L_MachineCheck - _start + EXC_OFF_SYS_RESET
	li	r8, Alignment - _start + EXC_OFF_SYS_RESET
2:
	bl	trap_reloc
	addi	r7, r7, 0x100		/* next exception vector */
	cmplw	0, r7, r8
	blt	2b

	li	r7, .L_Alignment - _start + EXC_OFF_SYS_RESET
	bl	trap_reloc

	li	r7, .L_ProgramCheck - _start + EXC_OFF_SYS_RESET
	bl	trap_reloc

	li	r7, .L_FPUnavailable - _start + EXC_OFF_SYS_RESET
	li	r8, SystemCall - _start + EXC_OFF_SYS_RESET
3:
	bl	trap_reloc
	addi	r7, r7, 0x100		/* next exception vector */
	cmplw	0, r7, r8
	blt	3b

	li	r7, .L_SingleStep - _start + EXC_OFF_SYS_RESET
	li	r8, _end_of_vectors - _start + EXC_OFF_SYS_RESET
4:
	bl	trap_reloc
	addi	r7, r7, 0x100		/* next exception vector */
	cmplw	0, r7, r8
	blt	4b

	mfmsr	r3			/* now that the vectors have */
	lis	r7, MSR_IP@h		/* relocated into low memory */
	ori	r7, r7, MSR_IP@l	/* MSR[IP] can be turned off */
	andc	r3, r3, r7		/* (if it was on) */
	SYNC				/* Some chip revs need this... */
	mtmsr	r3
	SYNC

	mtlr	r4			/* restore link register    */
	blr

#endif /* !MINIMAL_SPL */

#ifdef CONFIG_SYS_INIT_RAM_LOCK
lock_ram_in_cache:
	/* Allocate Initial RAM in data cache.
	 */
	lis	r3, (CONFIG_SYS_INIT_RAM_ADDR & ~31)@h
	ori	r3, r3, (CONFIG_SYS_INIT_RAM_ADDR & ~31)@l
	li	r4, ((CONFIG_SYS_INIT_RAM_SIZE & ~31) + \
		     (CONFIG_SYS_INIT_RAM_ADDR & 31) + 31) / 32
	mtctr	r4
1:
	dcbz	r0, r3
	addi	r3, r3, 32
	bdnz	1b

	/* Lock the data cache */
	mfspr	r0, HID0
	ori	r0, r0, HID0_DLOCK
	sync
	mtspr	HID0, r0
	sync
	blr

#ifndef MINIMAL_SPL
.globl unlock_ram_in_cache
unlock_ram_in_cache:
	/* invalidate the INIT_RAM section */
	lis	r3, (CONFIG_SYS_INIT_RAM_ADDR & ~31)@h
	ori	r3, r3, (CONFIG_SYS_INIT_RAM_ADDR & ~31)@l
	li	r4, ((CONFIG_SYS_INIT_RAM_SIZE & ~31) + \
		     (CONFIG_SYS_INIT_RAM_ADDR & 31) + 31) / 32
	mtctr	r4
1:	icbi	r0, r3
	dcbi	r0, r3
	addi	r3, r3, 32
	bdnz	1b
	sync			/* Wait for all icbi to complete on bus	*/
	isync

	/* Unlock the data cache and invalidate it */
	mfspr   r3, HID0
	li	r5, HID0_DLOCK|HID0_DCFI
	andc	r3, r3, r5		/* no invalidate, unlock */
	ori	r5, r3, HID0_DCFI	/* invalidate, unlock */
	sync
	mtspr	HID0, r5		/* invalidate, unlock */
	sync
	mtspr	HID0, r3		/* no invalidate, unlock */
	blr
#endif /* !MINIMAL_SPL */
#endif /* CONFIG_SYS_INIT_RAM_LOCK */

#ifdef CONFIG_SYS_FLASHBOOT
map_flash_by_law1:
	/* When booting from ROM (Flash or EPROM), clear the  */
	/* Address Mask in OR0 so ROM appears everywhere      */
	/*----------------------------------------------------*/
	lis	r3, (CONFIG_SYS_IMMR)@h  /* r3 <= CONFIG_SYS_IMMR    */
	lwz	r4, OR0@l(r3)
	li	r5, 0x7fff        /* r5 <= 0x00007FFFF */
	and	r4, r4, r5
	stw	r4, OR0@l(r3)     /* OR0 <= OR0 & 0x00007FFFF */

	/* As MPC8349E User's Manual presented, when RCW[BMS] is set to 0,
	 * system will boot from 0x0000_0100, and the LBLAWBAR0[BASE_ADDR]
	 * reset value is 0x00000; when RCW[BMS] is set to 1, system will boot
	 * from 0xFFF0_0100, and the LBLAWBAR0[BASE_ADDR] reset value is
	 * 0xFF800.  From the hard resetting to here, the processor fetched and
	 * executed the instructions one by one.  There is not absolutely
	 * jumping happened.  Laterly, the u-boot code has to do an absolutely
	 * jumping to tell the CPU instruction fetching component what the
	 * u-boot TEXT base address is.  Because the TEXT base resides in the
	 * boot ROM memory space, to garantee the code can run smoothly after
	 * that jumping, we must map in the entire boot ROM by Local Access
	 * Window.  Sometimes, we desire an non-0x00000 or non-0xFF800 starting
	 * address for boot ROM, such as 0xFE000000.  In this case, the default
	 * LBIU Local Access Widow 0 will not cover this memory space.  So, we
	 * need another window to map in it.
	 */
	lis r4, (CONFIG_SYS_FLASH_BASE)@h
	ori r4, r4, (CONFIG_SYS_FLASH_BASE)@l
	stw r4, LBLAWBAR1(r3) /* LBLAWBAR1 <= CONFIG_SYS_FLASH_BASE */

	/* Store 0x80000012 + log2(CONFIG_SYS_FLASH_SIZE) into LBLAWAR1 */
	lis r4, (0x80000012)@h
	ori r4, r4, (0x80000012)@l
	li r5, CONFIG_SYS_FLASH_SIZE
1:	srawi. r5, r5, 1	/* r5 = r5 >> 1 */
	addi r4, r4, 1
	bne 1b

	stw r4, LBLAWAR1(r3) /* LBLAWAR1 <= 8MB Flash Size */
	/* Wait for HW to catch up */
	lwz r4, LBLAWAR1(r3)
	twi 0,r4,0
	isync
	blr

	/* Though all the LBIU Local Access Windows and LBC Banks will be
	 * initialized in the C code, we'd better configure boot ROM's
	 * window 0 and bank 0 correctly at here.
	 */
remap_flash_by_law0:
	/* Initialize the BR0 with the boot ROM starting address. */
	lwz r4, BR0(r3)
	li  r5, 0x7FFF
	and r4, r4, r5
	lis r5, (CONFIG_SYS_FLASH_BASE & 0xFFFF8000)@h
	ori r5, r5, (CONFIG_SYS_FLASH_BASE & 0xFFFF8000)@l
	or  r5, r5, r4
	stw r5, BR0(r3) /* r5 <= (CONFIG_SYS_FLASH_BASE & 0xFFFF8000) | (BR0 & 0x00007FFF) */

	lwz r4, OR0(r3)
	lis r5, ~((CONFIG_SYS_FLASH_SIZE << 4) - 1)
	or r4, r4, r5
	stw r4, OR0(r3)

	lis r4, (CONFIG_SYS_FLASH_BASE)@h
	ori r4, r4, (CONFIG_SYS_FLASH_BASE)@l
	stw r4, LBLAWBAR0(r3) /* LBLAWBAR0 <= CONFIG_SYS_FLASH_BASE */

	/* Store 0x80000012 + log2(CONFIG_SYS_FLASH_SIZE) into LBLAWAR0 */
	lis r4, (0x80000012)@h
	ori r4, r4, (0x80000012)@l
	li r5, CONFIG_SYS_FLASH_SIZE
1:	srawi. r5, r5, 1 /* r5 = r5 >> 1 */
	addi r4, r4, 1
	bne 1b
	stw r4, LBLAWAR0(r3) /* LBLAWAR0 <= Flash Size */


	xor r4, r4, r4
	stw r4, LBLAWBAR1(r3)
	stw r4, LBLAWAR1(r3) /* Off LBIU LAW1 */
	/* Wait for HW to catch up */
	lwz r4, LBLAWAR1(r3)
	twi 0,r4,0
	isync
	blr
#endif /* CONFIG_SYS_FLASHBOOT */
