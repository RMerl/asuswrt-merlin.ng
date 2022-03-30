/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2004, 2007-2012 Freescale Semiconductor, Inc.
 * Copyright (C) 2003  Motorola,Inc.
 */

/* U-Boot Startup Code for Motorola 85xx PowerPC based Embedded Boards
 *
 * The processor starts at 0xfffffffc and the code is first executed in the
 * last 4K page(0xfffff000-0xffffffff) in flash/rom.
 *
 */

#include <asm-offsets.h>
#include <config.h>
#include <mpc85xx.h>
#include <version.h>

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#include <asm/cache.h>
#include <asm/mmu.h>

#undef	MSR_KERNEL
#define MSR_KERNEL ( MSR_ME )	/* Machine Check */

#define LAW_EN		0x80000000

#if defined(CONFIG_NAND_SPL) || \
	(defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_INIT_MINIMAL))
#define MINIMAL_SPL
#endif

#if !defined(CONFIG_SPL) && !defined(CONFIG_SYS_RAMBOOT) && \
	!defined(CONFIG_SECURE_BOOT) && !defined(CONFIG_SRIO_PCIE_BOOT_SLAVE)
#define NOR_BOOT
#endif

/*
 * Set up GOT: Global Offset Table
 *
 * Use r12 to access the GOT
 */
	START_GOT
	GOT_ENTRY(_GOT2_TABLE_)
	GOT_ENTRY(_FIXUP_TABLE_)

#ifndef MINIMAL_SPL
	GOT_ENTRY(_start)
	GOT_ENTRY(_start_of_vectors)
	GOT_ENTRY(_end_of_vectors)
	GOT_ENTRY(transfer_to_handler)
#endif

	GOT_ENTRY(__init_end)
	GOT_ENTRY(__bss_end)
	GOT_ENTRY(__bss_start)
	END_GOT

/*
 * e500 Startup -- after reset only the last 4KB of the effective
 * address space is mapped in the MMU L2 TLB1 Entry0. The .bootpg
 * section is located at THIS LAST page and basically does three
 * things: clear some registers, set up exception tables and
 * add more TLB entries for 'larger spaces'(e.g. the boot rom) to
 * continue the boot procedure.

 * Once the boot rom is mapped by TLB entries we can proceed
 * with normal startup.
 *
 */

	.section .bootpg,"ax"
	.globl _start_e500

_start_e500:
/* Enable debug exception */
	li	r1,MSR_DE
	mtmsr 	r1

	/*
	 * If we got an ePAPR device tree pointer passed in as r3, we need that
	 * later in cpu_init_early_f(). Save it to a safe register before we
	 * clobber it so that we can fetch it from there later.
	 */
	mr	r24, r3

#ifdef CONFIG_SYS_FSL_ERRATUM_A004510
	mfspr	r3,SPRN_SVR
	rlwinm	r3,r3,0,0xff
	li	r4,CONFIG_SYS_FSL_ERRATUM_A004510_SVR_REV
	cmpw	r3,r4
	beq	1f

#ifdef CONFIG_SYS_FSL_ERRATUM_A004510_SVR_REV2
	li	r4,CONFIG_SYS_FSL_ERRATUM_A004510_SVR_REV2
	cmpw	r3,r4
	beq	1f
#endif

	/* Not a supported revision affected by erratum */
	li	r27,0
	b	2f

1:	li	r27,1	/* Remember for later that we have the erratum */
	/* Erratum says set bits 55:60 to 001001 */
	msync
	isync
	mfspr	r3,SPRN_HDBCR0
	li	r4,0x48
	rlwimi	r3,r4,0,0x1f8
	mtspr	SPRN_HDBCR0,r3
	isync
2:
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A005125
	msync
	isync
	mfspr	r3, SPRN_HDBCR0
	oris	r3, r3, 0x0080
	mtspr	SPRN_HDBCR0, r3
#endif


#if defined(CONFIG_SECURE_BOOT) && defined(CONFIG_E500MC) && \
	!defined(CONFIG_E6500)
	/* ISBC uses L2 as stack.
	 * Disable L2 cache here so that u-boot can enable it later
	 * as part of it's normal flow
	*/

	/* Check if L2 is enabled */
	mfspr	r3, SPRN_L2CSR0
	lis	r2, L2CSR0_L2E@h
	ori	r2, r2, L2CSR0_L2E@l
	and.	r4, r3, r2
	beq	l2_disabled

	mfspr r3, SPRN_L2CSR0
	/* Flush L2 cache */
	lis     r2,(L2CSR0_L2FL)@h
	ori     r2, r2, (L2CSR0_L2FL)@l
	or      r3, r2, r3
	sync
	isync
	mtspr   SPRN_L2CSR0,r3
	isync
1:
	mfspr r3, SPRN_L2CSR0
	and. r1, r3, r2
	bne 1b

	mfspr r3, SPRN_L2CSR0
	lis r2, L2CSR0_L2E@h
	ori r2, r2, L2CSR0_L2E@l
	andc r4, r3, r2
	sync
	isync
	mtspr SPRN_L2CSR0,r4
	isync

l2_disabled:
#endif

/* clear registers/arrays not reset by hardware */

	/* L1 */
	li	r0,2
	mtspr	L1CSR0,r0	/* invalidate d-cache */
	mtspr	L1CSR1,r0	/* invalidate i-cache */

	mfspr	r1,DBSR
	mtspr	DBSR,r1		/* Clear all valid bits */


	.macro	create_tlb1_entry esel ts tsize epn wimg rpn perm phy_high scratch
	lis	\scratch, FSL_BOOKE_MAS0(1, \esel, 0)@h
	ori	\scratch, \scratch, FSL_BOOKE_MAS0(1, \esel, 0)@l
	mtspr	MAS0, \scratch
	lis	\scratch, FSL_BOOKE_MAS1(1, 1, 0, \ts, \tsize)@h
	ori	\scratch, \scratch, FSL_BOOKE_MAS1(1, 1, 0, \ts, \tsize)@l
	mtspr	MAS1, \scratch
	lis	\scratch, FSL_BOOKE_MAS2(\epn, \wimg)@h
	ori	\scratch, \scratch, FSL_BOOKE_MAS2(\epn, \wimg)@l
	mtspr	MAS2, \scratch
	lis	\scratch, FSL_BOOKE_MAS3(\rpn, 0, \perm)@h
	ori	\scratch, \scratch, FSL_BOOKE_MAS3(\rpn, 0, \perm)@l
	mtspr	MAS3, \scratch
	lis	\scratch, \phy_high@h
	ori	\scratch, \scratch, \phy_high@l
	mtspr	MAS7, \scratch
	isync
	msync
	tlbwe
	isync
	.endm

	.macro	create_tlb0_entry esel ts tsize epn wimg rpn perm phy_high scratch
	lis	\scratch, FSL_BOOKE_MAS0(0, \esel, 0)@h
	ori	\scratch, \scratch, FSL_BOOKE_MAS0(0, \esel, 0)@l
	mtspr	MAS0, \scratch
	lis	\scratch, FSL_BOOKE_MAS1(1, 0, 0, \ts, \tsize)@h
	ori	\scratch, \scratch, FSL_BOOKE_MAS1(1, 0, 0, \ts, \tsize)@l
	mtspr	MAS1, \scratch
	lis	\scratch, FSL_BOOKE_MAS2(\epn, \wimg)@h
	ori	\scratch, \scratch, FSL_BOOKE_MAS2(\epn, \wimg)@l
	mtspr	MAS2, \scratch
	lis	\scratch, FSL_BOOKE_MAS3(\rpn, 0, \perm)@h
	ori	\scratch, \scratch, FSL_BOOKE_MAS3(\rpn, 0, \perm)@l
	mtspr	MAS3, \scratch
	lis	\scratch, \phy_high@h
	ori	\scratch, \scratch, \phy_high@l
	mtspr	MAS7, \scratch
	isync
	msync
	tlbwe
	isync
	.endm

	.macro	delete_tlb1_entry esel scratch
	lis	\scratch, FSL_BOOKE_MAS0(1, \esel, 0)@h
	ori	\scratch, \scratch, FSL_BOOKE_MAS0(1, \esel, 0)@l
	mtspr	MAS0, \scratch
	li	\scratch, 0
	mtspr	MAS1, \scratch
	isync
	msync
	tlbwe
	isync
	.endm

	.macro	delete_tlb0_entry esel epn wimg scratch
	lis	\scratch, FSL_BOOKE_MAS0(0, \esel, 0)@h
	ori	\scratch, \scratch, FSL_BOOKE_MAS0(0, \esel, 0)@l
	mtspr	MAS0, \scratch
	li	\scratch, 0
	mtspr	MAS1, \scratch
	lis	\scratch, FSL_BOOKE_MAS2(\epn, \wimg)@h
	ori	\scratch, \scratch, FSL_BOOKE_MAS2(\epn, \wimg)@l
	mtspr	MAS2, \scratch
	isync
	msync
	tlbwe
	isync
	.endm

/* Interrupt vectors do not fit in minimal SPL. */
#if !defined(MINIMAL_SPL)
	/* Setup interrupt vectors */
	lis	r1,CONFIG_SYS_MONITOR_BASE@h
	mtspr	IVPR,r1

	li	r4,CriticalInput@l
	mtspr	IVOR0,r4	/* 0: Critical input */
	li	r4,MachineCheck@l
	mtspr	IVOR1,r4	/* 1: Machine check */
	li	r4,DataStorage@l
	mtspr	IVOR2,r4	/* 2: Data storage */
	li	r4,InstStorage@l
	mtspr	IVOR3,r4	/* 3: Instruction storage */
	li	r4,ExtInterrupt@l
	mtspr	IVOR4,r4	/* 4: External interrupt */
	li	r4,Alignment@l
	mtspr	IVOR5,r4	/* 5: Alignment */
	li	r4,ProgramCheck@l
	mtspr	IVOR6,r4	/* 6: Program check */
	li	r4,FPUnavailable@l
	mtspr	IVOR7,r4	/* 7: floating point unavailable */
	li	r4,SystemCall@l
	mtspr	IVOR8,r4	/* 8: System call */
	/* 9: Auxiliary processor unavailable(unsupported) */
	li	r4,Decrementer@l
	mtspr	IVOR10,r4	/* 10: Decrementer */
	li	r4,IntervalTimer@l
	mtspr	IVOR11,r4	/* 11: Interval timer */
	li	r4,WatchdogTimer@l
	mtspr	IVOR12,r4	/* 12: Watchdog timer */
	li	r4,DataTLBError@l
	mtspr	IVOR13,r4	/* 13: Data TLB error */
	li	r4,InstructionTLBError@l
	mtspr	IVOR14,r4	/* 14: Instruction TLB error */
	li	r4,DebugBreakpoint@l
	mtspr	IVOR15,r4	/* 15: Debug */
#endif

	/* Clear and set up some registers. */
	li      r0,0x0000
	lis	r1,0xffff
	mtspr	DEC,r0			/* prevent dec exceptions */
	mttbl	r0			/* prevent fit & wdt exceptions */
	mttbu	r0
	mtspr	TSR,r1			/* clear all timer exception status */
	mtspr	TCR,r0			/* disable all */
	mtspr	ESR,r0			/* clear exception syndrome register */
	mtspr	MCSR,r0			/* machine check syndrome register */
	mtxer	r0			/* clear integer exception register */

#ifdef CONFIG_SYS_BOOK3E_HV
	mtspr	MAS8,r0			/* make sure MAS8 is clear */
#endif

	/* Enable Time Base and Select Time Base Clock */
	lis	r0,HID0_EMCP@h		/* Enable machine check */
#if defined(CONFIG_ENABLE_36BIT_PHYS)
	ori	r0,r0,HID0_ENMAS7@l	/* Enable MAS7 */
#endif
#ifndef CONFIG_E500MC
	ori	r0,r0,HID0_TBEN@l	/* Enable Timebase */
#endif
	mtspr	HID0,r0

#if !defined(CONFIG_E500MC) && !defined(CONFIG_ARCH_QEMU_E500)
	li	r0,(HID1_ASTME|HID1_ABE)@l	/* Addr streaming & broadcast */
	mfspr	r3,PVR
	andi.	r3,r3, 0xff
	cmpwi	r3,0x50@l	/* if we are rev 5.0 or greater set MBDD */
	blt 1f
	/* Set MBDD bit also */
	ori r0, r0, HID1_MBDD@l
1:
	mtspr	HID1,r0
#endif

#ifdef CONFIG_SYS_FSL_ERRATUM_CPU_A003999
	mfspr	r3,SPRN_HDBCR1
	oris	r3,r3,0x0100
	mtspr	SPRN_HDBCR1,r3
#endif

	/* Enable Branch Prediction */
#if defined(CONFIG_BTB)
	lis	r0,BUCSR_ENABLE@h
	ori	r0,r0,BUCSR_ENABLE@l
	mtspr	SPRN_BUCSR,r0
#endif

#if defined(CONFIG_SYS_INIT_DBCR)
	lis	r1,0xffff
	ori	r1,r1,0xffff
	mtspr	DBSR,r1			/* Clear all status bits */
	lis	r0,CONFIG_SYS_INIT_DBCR@h	/* DBCR0[IDM] must be set */
	ori	r0,r0,CONFIG_SYS_INIT_DBCR@l
	mtspr	DBCR0,r0
#endif

#ifdef CONFIG_ARCH_MPC8569
#define CONFIG_SYS_LBC_ADDR (CONFIG_SYS_CCSRBAR_DEFAULT + 0x5000)
#define CONFIG_SYS_LBCR_ADDR (CONFIG_SYS_LBC_ADDR + 0xd0)

	/* MPC8569 Rev.0 silcon needs to set bit 13 of LBCR to allow elBC to
	 * use address space which is more than 12bits, and it must be done in
	 * the 4K boot page. So we set this bit here.
	 */

	/* create a temp mapping TLB0[0] for LBCR  */
	create_tlb0_entry 0, \
		0, BOOKE_PAGESZ_4K, \
		CONFIG_SYS_LBC_ADDR, MAS2_I|MAS2_G, \
		CONFIG_SYS_LBC_ADDR, MAS3_SW|MAS3_SR, \
		0, r6

	/* Set LBCR register */
	lis     r4,CONFIG_SYS_LBCR_ADDR@h
	ori     r4,r4,CONFIG_SYS_LBCR_ADDR@l

	lis     r5,CONFIG_SYS_LBC_LBCR@h
	ori     r5,r5,CONFIG_SYS_LBC_LBCR@l
	stw     r5,0(r4)
	isync

	/* invalidate this temp TLB */
	lis	r4,CONFIG_SYS_LBC_ADDR@h
	ori	r4,r4,CONFIG_SYS_LBC_ADDR@l
	tlbivax	0,r4
	isync

#endif /* CONFIG_ARCH_MPC8569 */

/*
 * Search for the TLB that covers the code we're executing, and shrink it
 * so that it covers only this 4K page.  That will ensure that any other
 * TLB we create won't interfere with it.  We assume that the TLB exists,
 * which is why we don't check the Valid bit of MAS1.  We also assume
 * it is in TLB1.
 *
 * This is necessary, for example, when booting from the on-chip ROM,
 * which (oddly) creates a single 4GB TLB that covers CCSR and DDR.
 */
	bl	nexti		/* Find our address */
nexti:	mflr	r1		/* R1 = our PC */
	li	r2, 0
	mtspr	MAS6, r2	/* Assume the current PID and AS are 0 */
	isync
	msync
	tlbsx	0, r1		/* This must succeed */

	mfspr	r14, MAS0	/* Save ESEL for later */
	rlwinm	r14, r14, 16, 0xfff

	/* Set the size of the TLB to 4KB */
	mfspr	r3, MAS1
	li	r2, 0xF80
	andc	r3, r3, r2	/* Clear the TSIZE bits */
	ori	r3, r3, MAS1_TSIZE(BOOKE_PAGESZ_4K)@l
	oris	r3, r3, MAS1_IPROT@h
	mtspr	MAS1, r3

	/*
	 * Set the base address of the TLB to our PC.  We assume that
	 * virtual == physical.  We also assume that MAS2_EPN == MAS3_RPN.
	 */
	lis	r3, MAS2_EPN@h
	ori	r3, r3, MAS2_EPN@l	/* R3 = MAS2_EPN */

	and	r1, r1, r3	/* Our PC, rounded down to the nearest page */

	mfspr	r2, MAS2
	andc	r2, r2, r3
	or	r2, r2, r1
#ifdef CONFIG_SYS_FSL_ERRATUM_A004510
	cmpwi	r27,0
	beq	1f
	andi.	r15, r2, MAS2_I|MAS2_G /* save the old I/G for later */
	rlwinm	r2, r2, 0, ~MAS2_I
	ori	r2, r2, MAS2_G
1:
#endif
	mtspr	MAS2, r2	/* Set the EPN to our PC base address */

	mfspr	r2, MAS3
	andc	r2, r2, r3
	or	r2, r2, r1
	mtspr	MAS3, r2	/* Set the RPN to our PC base address */

	isync
	msync
	tlbwe

/*
 * Clear out any other TLB entries that may exist, to avoid conflicts.
 * Our TLB entry is in r14.
 */
	li	r0, TLBIVAX_ALL | TLBIVAX_TLB0
	tlbivax 0, r0
	tlbsync

	mfspr	r4, SPRN_TLB1CFG
	rlwinm	r4, r4, 0, TLBnCFG_NENTRY_MASK

	li	r3, 0
	mtspr	MAS1, r3
1:	cmpw	r3, r14
	rlwinm	r5, r3, 16, MAS0_ESEL_MSK
	addi	r3, r3, 1
	beq	2f		/* skip the entry we're executing from */

	oris	r5, r5, MAS0_TLBSEL(1)@h
	mtspr	MAS0, r5

	isync
	tlbwe
	isync
	msync

2:	cmpw	r3, r4
	blt	1b

#if defined(CONFIG_SYS_PPC_E500_DEBUG_TLB) && !defined(MINIMAL_SPL) && \
	!defined(CONFIG_SECURE_BOOT)
/*
 * TLB entry for debuggging in AS1
 * Create temporary TLB entry in AS0 to handle debug exception
 * As on debug exception MSR is cleared i.e. Address space is changed
 * to 0. A TLB entry (in AS0) is required to handle debug exception generated
 * in AS1.
 */

#ifdef NOR_BOOT
/*
 * TLB entry is created for IVPR + IVOR15 to map on valid OP code address
 * bacause flash's virtual address maps to 0xff800000 - 0xffffffff.
 * and this window is outside of 4K boot window.
 */
	create_tlb1_entry CONFIG_SYS_PPC_E500_DEBUG_TLB, \
		0, BOOKE_PAGESZ_4M, \
		CONFIG_SYS_MONITOR_BASE & 0xffc00000,  MAS2_I|MAS2_G, \
		0xffc00000, MAS3_SX|MAS3_SW|MAS3_SR, \
		0, r6

#else
/*
 * TLB entry is created for IVPR + IVOR15 to map on valid OP code address
 * because "nexti" will resize TLB to 4K
 */
	create_tlb1_entry CONFIG_SYS_PPC_E500_DEBUG_TLB, \
		0, BOOKE_PAGESZ_256K, \
		CONFIG_SYS_MONITOR_BASE & 0xfffc0000, MAS2_I, \
		CONFIG_SYS_MONITOR_BASE & 0xfffc0000, MAS3_SX|MAS3_SW|MAS3_SR, \
		0, r6
#endif
#endif

/*
 * Relocate CCSR, if necessary.  We relocate CCSR if (obviously) the default
 * location is not where we want it.  This typically happens on a 36-bit
 * system, where we want to move CCSR to near the top of 36-bit address space.
 *
 * To move CCSR, we create two temporary TLBs, one for the old location, and
 * another for the new location.  On CoreNet systems, we also need to create
 * a special, temporary LAW.
 *
 * As a general rule, TLB0 is used for short-term TLBs, and TLB1 is used for
 * long-term TLBs, so we use TLB0 here.
 */
#if (CONFIG_SYS_CCSRBAR_DEFAULT != CONFIG_SYS_CCSRBAR_PHYS)

#if !defined(CONFIG_SYS_CCSRBAR_PHYS_HIGH) || !defined(CONFIG_SYS_CCSRBAR_PHYS_LOW)
#error "CONFIG_SYS_CCSRBAR_PHYS_HIGH and CONFIG_SYS_CCSRBAR_PHYS_LOW) must be defined."
#endif

create_ccsr_new_tlb:
	/*
	 * Create a TLB for the new location of CCSR.  Register R8 is reserved
	 * for the virtual address of this TLB (CONFIG_SYS_CCSRBAR).
	 */
	lis	r8, CONFIG_SYS_CCSRBAR@h
	ori	r8, r8, CONFIG_SYS_CCSRBAR@l
	lis	r9, (CONFIG_SYS_CCSRBAR + 0x1000)@h
	ori	r9, r9, (CONFIG_SYS_CCSRBAR + 0x1000)@l
	create_tlb0_entry 0, \
		0, BOOKE_PAGESZ_4K, \
		CONFIG_SYS_CCSRBAR, MAS2_I|MAS2_G, \
		CONFIG_SYS_CCSRBAR_PHYS_LOW, MAS3_SW|MAS3_SR, \
		CONFIG_SYS_CCSRBAR_PHYS_HIGH, r3
	/*
	 * Create a TLB for the current location of CCSR.  Register R9 is reserved
	 * for the virtual address of this TLB (CONFIG_SYS_CCSRBAR + 0x1000).
	 */
create_ccsr_old_tlb:
	create_tlb0_entry 1, \
		0, BOOKE_PAGESZ_4K, \
		CONFIG_SYS_CCSRBAR + 0x1000, MAS2_I|MAS2_G, \
		CONFIG_SYS_CCSRBAR_DEFAULT, MAS3_SW|MAS3_SR, \
		0, r3 /* The default CCSR address is always a 32-bit number */


	/*
	 * We have a TLB for what we think is the current (old) CCSR.  Let's
	 * verify that, otherwise we won't be able to move it.
	 * CONFIG_SYS_CCSRBAR_DEFAULT is always a 32-bit number, so we only
	 * need to compare the lower 32 bits of CCSRBAR on CoreNet systems.
	 */
verify_old_ccsr:
	lis     r0, CONFIG_SYS_CCSRBAR_DEFAULT@h
	ori     r0, r0, CONFIG_SYS_CCSRBAR_DEFAULT@l
#ifdef CONFIG_FSL_CORENET
	lwz	r1, 4(r9)		/* CCSRBARL */
#else
	lwz	r1, 0(r9)		/* CCSRBAR, shifted right by 12 */
	slwi	r1, r1, 12
#endif

	cmpl	0, r0, r1

	/*
	 * If the value we read from CCSRBARL is not what we expect, then
	 * enter an infinite loop.  This will at least allow a debugger to
	 * halt execution and examine TLBs, etc.  There's no point in going
	 * on.
	 */
infinite_debug_loop:
	bne	infinite_debug_loop

#ifdef CONFIG_FSL_CORENET

#define CCSR_LAWBARH0	(CONFIG_SYS_CCSRBAR + 0x1000)
#define LAW_SIZE_4K	0xb
#define CCSRBAR_LAWAR	(LAW_EN | (0x1e << 20) | LAW_SIZE_4K)
#define CCSRAR_C	0x80000000	/* Commit */

create_temp_law:
	/*
	 * On CoreNet systems, we create the temporary LAW using a special LAW
	 * target ID of 0x1e.  LAWBARH is at offset 0xc00 in CCSR.
	 */
	lis     r0, CONFIG_SYS_CCSRBAR_PHYS_HIGH@h
	ori     r0, r0, CONFIG_SYS_CCSRBAR_PHYS_HIGH@l
	lis     r1, CONFIG_SYS_CCSRBAR_PHYS_LOW@h
	ori     r1, r1, CONFIG_SYS_CCSRBAR_PHYS_LOW@l
	lis     r2, CCSRBAR_LAWAR@h
	ori     r2, r2, CCSRBAR_LAWAR@l

	stw     r0, 0xc00(r9)	/* LAWBARH0 */
	stw     r1, 0xc04(r9)	/* LAWBARL0 */
	sync
	stw     r2, 0xc08(r9)	/* LAWAR0 */

	/*
	 * Read back from LAWAR to ensure the update is complete.  e500mc
	 * cores also require an isync.
	 */
	lwz	r0, 0xc08(r9)	/* LAWAR0 */
	isync

	/*
	 * Read the current CCSRBARH and CCSRBARL using load word instructions.
	 * Follow this with an isync instruction. This forces any outstanding
	 * accesses to configuration space to completion.
	 */
read_old_ccsrbar:
	lwz	r0, 0(r9)	/* CCSRBARH */
	lwz	r0, 4(r9)	/* CCSRBARL */
	isync

	/*
	 * Write the new values for CCSRBARH and CCSRBARL to their old
	 * locations.  The CCSRBARH has a shadow register. When the CCSRBARH
	 * has a new value written it loads a CCSRBARH shadow register. When
	 * the CCSRBARL is written, the CCSRBARH shadow register contents
	 * along with the CCSRBARL value are loaded into the CCSRBARH and
	 * CCSRBARL registers, respectively.  Follow this with a sync
	 * instruction.
	 */
write_new_ccsrbar:
	lis	r0, CONFIG_SYS_CCSRBAR_PHYS_HIGH@h
	ori	r0, r0, CONFIG_SYS_CCSRBAR_PHYS_HIGH@l
	lis	r1, CONFIG_SYS_CCSRBAR_PHYS_LOW@h
	ori	r1, r1, CONFIG_SYS_CCSRBAR_PHYS_LOW@l
	lis	r2, CCSRAR_C@h
	ori	r2, r2, CCSRAR_C@l

	stw	r0, 0(r9)	/* Write to CCSRBARH */
	sync			/* Make sure we write to CCSRBARH first */
	stw	r1, 4(r9)	/* Write to CCSRBARL */
	sync

	/*
	 * Write a 1 to the commit bit (C) of CCSRAR at the old location.
	 * Follow this with a sync instruction.
	 */
	stw	r2, 8(r9)
	sync

	/* Delete the temporary LAW */
delete_temp_law:
	li	r1, 0
	stw	r1, 0xc08(r8)
	sync
	stw	r1, 0xc00(r8)
	stw	r1, 0xc04(r8)
	sync

#else /* #ifdef CONFIG_FSL_CORENET */

write_new_ccsrbar:
	/*
	 * Read the current value of CCSRBAR using a load word instruction
	 * followed by an isync. This forces all accesses to configuration
	 * space to complete.
	 */
	sync
	lwz	r0, 0(r9)
	isync

/* CONFIG_SYS_CCSRBAR_PHYS right shifted by 12 */
#define CCSRBAR_PHYS_RS12 ((CONFIG_SYS_CCSRBAR_PHYS_HIGH << 20) | \
			   (CONFIG_SYS_CCSRBAR_PHYS_LOW >> 12))

	/* Write the new value to CCSRBAR. */
	lis	r0, CCSRBAR_PHYS_RS12@h
	ori	r0, r0, CCSRBAR_PHYS_RS12@l
	stw	r0, 0(r9)
	sync

	/*
	 * The manual says to perform a load of an address that does not
	 * access configuration space or the on-chip SRAM using an existing TLB,
	 * but that doesn't appear to be necessary.  We will do the isync,
	 * though.
	 */
	isync

	/*
	 * Read the contents of CCSRBAR from its new location, followed by
	 * another isync.
	 */
	lwz	r0, 0(r8)
	isync

#endif  /* #ifdef CONFIG_FSL_CORENET */

	/* Delete the temporary TLBs */
delete_temp_tlbs:
	delete_tlb0_entry 0, CONFIG_SYS_CCSRBAR, MAS2_I|MAS2_G, r3
	delete_tlb0_entry 1, CONFIG_SYS_CCSRBAR + 0x1000, MAS2_I|MAS2_G, r3

#endif /* #if (CONFIG_SYS_CCSRBAR_DEFAULT != CONFIG_SYS_CCSRBAR_PHYS) */

#if defined(CONFIG_SYS_FSL_QORIQ_CHASSIS2) && defined(CONFIG_E6500)
create_ccsr_l2_tlb:
	/*
	 * Create a TLB for the MMR location of CCSR
	 * to access L2CSR0 register
	 */
	create_tlb0_entry 0, \
		0, BOOKE_PAGESZ_4K, \
		CONFIG_SYS_CCSRBAR + 0xC20000, MAS2_I|MAS2_G, \
		CONFIG_SYS_CCSRBAR_PHYS_LOW + 0xC20000, MAS3_SW|MAS3_SR, \
		CONFIG_SYS_CCSRBAR_PHYS_HIGH, r3

enable_l2_cluster_l2:
	/* enable L2 cache */
	lis	r3, (CONFIG_SYS_CCSRBAR + 0xC20000)@h
	ori	r3, r3, (CONFIG_SYS_CCSRBAR + 0xC20000)@l
	li	r4, 33	/* stash id */
	stw	r4, 4(r3)
	lis	r4, (L2CSR0_L2FI|L2CSR0_L2LFC)@h
	ori	r4, r4, (L2CSR0_L2FI|L2CSR0_L2LFC)@l
	sync
	stw	r4, 0(r3)	/* invalidate L2 */
	/* Poll till the bits are cleared */
1:	sync
	lwz	r0, 0(r3)
	twi	0, r0, 0
	isync
	and.	r1, r0, r4
	bne	1b

	/* L2PE must be set before L2 cache is enabled */
	lis	r4, (L2CSR0_L2PE)@h
	ori	r4, r4, (L2CSR0_L2PE)@l
	sync
	stw	r4, 0(r3)	/* enable L2 parity/ECC error checking */
	/* Poll till the bit is set */
1:	sync
	lwz	r0, 0(r3)
	twi	0, r0, 0
	isync
	and.	r1, r0, r4
	beq	1b

	lis	r4, (L2CSR0_L2E|L2CSR0_L2PE)@h
	ori	r4, r4, (L2CSR0_L2REP_MODE)@l
	sync
	stw	r4, 0(r3)	/* enable L2 */
	/* Poll till the bit is set */
1:	sync
	lwz	r0, 0(r3)
	twi	0, r0, 0
	isync
	and.	r1, r0, r4
	beq	1b

delete_ccsr_l2_tlb:
	delete_tlb0_entry 0, CONFIG_SYS_CCSRBAR + 0xC20000, MAS2_I|MAS2_G, r3
#endif

	/*
	 * Enable the L1. On e6500, this has to be done
	 * after the L2 is up.
	 */

#ifdef CONFIG_SYS_CACHE_STASHING
	/* set stash id to (coreID) * 2 + 32 + L1 CT (0) */
	li	r2,(32 + 0)
	mtspr	L1CSR2,r2
#endif

	/* Enable/invalidate the I-Cache */
	lis	r2,(L1CSR1_ICFI|L1CSR1_ICLFR)@h
	ori	r2,r2,(L1CSR1_ICFI|L1CSR1_ICLFR)@l
	mtspr	SPRN_L1CSR1,r2
1:
	mfspr	r3,SPRN_L1CSR1
	and.	r1,r3,r2
	bne	1b

	lis	r3,(L1CSR1_CPE|L1CSR1_ICE)@h
	ori	r3,r3,(L1CSR1_CPE|L1CSR1_ICE)@l
	mtspr	SPRN_L1CSR1,r3
	isync
2:
	mfspr	r3,SPRN_L1CSR1
	andi.	r1,r3,L1CSR1_ICE@l
	beq	2b

	/* Enable/invalidate the D-Cache */
	lis	r2,(L1CSR0_DCFI|L1CSR0_DCLFR)@h
	ori	r2,r2,(L1CSR0_DCFI|L1CSR0_DCLFR)@l
	mtspr	SPRN_L1CSR0,r2
1:
	mfspr	r3,SPRN_L1CSR0
	and.	r1,r3,r2
	bne	1b

	lis	r3,(L1CSR0_CPE|L1CSR0_DCE)@h
	ori	r3,r3,(L1CSR0_CPE|L1CSR0_DCE)@l
	mtspr	SPRN_L1CSR0,r3
	isync
2:
	mfspr	r3,SPRN_L1CSR0
	andi.	r1,r3,L1CSR0_DCE@l
	beq	2b
#ifdef CONFIG_SYS_FSL_ERRATUM_A004510
#define DCSR_LAWBARH0	(CONFIG_SYS_CCSRBAR + 0x1000)
#define LAW_SIZE_1M	0x13
#define DCSRBAR_LAWAR	(LAW_EN | (0x1d << 20) | LAW_SIZE_1M)

	cmpwi	r27,0
	beq	9f

	/*
	 * Create a TLB entry for CCSR
	 *
	 * We're executing out of TLB1 entry in r14, and that's the only
	 * TLB entry that exists.  To allocate some TLB entries for our
	 * own use, flip a bit high enough that we won't flip it again
	 * via incrementing.
	 */

	xori	r8, r14, 32
	lis	r0, MAS0_TLBSEL(1)@h
	rlwimi	r0, r8, 16, MAS0_ESEL_MSK
	lis	r1, FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_16M)@h
	ori	r1, r1, FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_16M)@l
	lis	r7, CONFIG_SYS_CCSRBAR@h
	ori	r7, r7, CONFIG_SYS_CCSRBAR@l
	ori	r2, r7, MAS2_I|MAS2_G
	lis	r3, FSL_BOOKE_MAS3(CONFIG_SYS_CCSRBAR_PHYS_LOW, 0, (MAS3_SW|MAS3_SR))@h
	ori	r3, r3, FSL_BOOKE_MAS3(CONFIG_SYS_CCSRBAR_PHYS_LOW, 0, (MAS3_SW|MAS3_SR))@l
	lis	r4, CONFIG_SYS_CCSRBAR_PHYS_HIGH@h
	ori	r4, r4, CONFIG_SYS_CCSRBAR_PHYS_HIGH@l
	mtspr	MAS0, r0
	mtspr	MAS1, r1
	mtspr	MAS2, r2
	mtspr	MAS3, r3
	mtspr	MAS7, r4
	isync
	tlbwe
	isync
	msync

	/* Map DCSR temporarily to physical address zero */
	li	r0, 0
	lis	r3, DCSRBAR_LAWAR@h
	ori	r3, r3, DCSRBAR_LAWAR@l

	stw	r0, 0xc00(r7)	/* LAWBARH0 */
	stw	r0, 0xc04(r7)	/* LAWBARL0 */
	sync
	stw	r3, 0xc08(r7)	/* LAWAR0 */

	/* Read back from LAWAR to ensure the update is complete. */
	lwz	r3, 0xc08(r7)	/* LAWAR0 */
	isync

	/* Create a TLB entry for DCSR at zero */

	addi	r9, r8, 1
	lis	r0, MAS0_TLBSEL(1)@h
	rlwimi	r0, r9, 16, MAS0_ESEL_MSK
	lis	r1, FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_1M)@h
	ori	r1, r1, FSL_BOOKE_MAS1(1, 1, 0, 0, BOOKE_PAGESZ_1M)@l
	li	r6, 0	/* DCSR effective address */
	ori	r2, r6, MAS2_I|MAS2_G
	li	r3, MAS3_SW|MAS3_SR
	li	r4, 0
	mtspr	MAS0, r0
	mtspr	MAS1, r1
	mtspr	MAS2, r2
	mtspr	MAS3, r3
	mtspr	MAS7, r4
	isync
	tlbwe
	isync
	msync

	/* enable the timebase */
#define CTBENR	0xe2084
	li	r3, 1
	addis	r4, r7, CTBENR@ha
	stw	r3, CTBENR@l(r4)
	lwz	r3, CTBENR@l(r4)
	twi	0,r3,0
	isync

	.macro	erratum_set_ccsr offset value
	addis	r3, r7, \offset@ha
	lis	r4, \value@h
	addi	r3, r3, \offset@l
	ori	r4, r4, \value@l
	bl	erratum_set_value
	.endm

	.macro	erratum_set_dcsr offset value
	addis	r3, r6, \offset@ha
	lis	r4, \value@h
	addi	r3, r3, \offset@l
	ori	r4, r4, \value@l
	bl	erratum_set_value
	.endm

	erratum_set_dcsr 0xb0e08 0xe0201800
	erratum_set_dcsr 0xb0e18 0xe0201800
	erratum_set_dcsr 0xb0e38 0xe0400000
	erratum_set_dcsr 0xb0008 0x00900000
	erratum_set_dcsr 0xb0e40 0xe00a0000
	erratum_set_ccsr 0x18600 CONFIG_SYS_FSL_CORENET_SNOOPVEC_COREONLY
#ifdef  CONFIG_RAMBOOT_PBL
	erratum_set_ccsr 0x10f00 0x495e5000
#else
	erratum_set_ccsr 0x10f00 0x415e5000
#endif
	erratum_set_ccsr 0x11f00 0x415e5000

	/* Make temp mapping uncacheable again, if it was initially */
	bl	2f
2:	mflr	r3
	tlbsx	0, r3
	mfspr	r4, MAS2
	rlwimi	r4, r15, 0, MAS2_I
	rlwimi	r4, r15, 0, MAS2_G
	mtspr	MAS2, r4
	isync
	tlbwe
	isync
	msync

	/* Clear the cache */
	lis	r3,(L1CSR1_ICFI|L1CSR1_ICLFR)@h
	ori	r3,r3,(L1CSR1_ICFI|L1CSR1_ICLFR)@l
	sync
	isync
	mtspr	SPRN_L1CSR1,r3
	isync
2:	sync
	mfspr	r4,SPRN_L1CSR1
	and.	r4,r4,r3
	bne	2b

	lis	r3,(L1CSR1_CPE|L1CSR1_ICE)@h
	ori	r3,r3,(L1CSR1_CPE|L1CSR1_ICE)@l
	sync
	isync
	mtspr	SPRN_L1CSR1,r3
	isync
2:	sync
	mfspr	r4,SPRN_L1CSR1
	and.	r4,r4,r3
	beq	2b

	/* Remove temporary mappings */
	lis	r0, MAS0_TLBSEL(1)@h
	rlwimi	r0, r9, 16, MAS0_ESEL_MSK
	li	r3, 0
	mtspr	MAS0, r0
	mtspr	MAS1, r3
	isync
	tlbwe
	isync
	msync

	li	r3, 0
	stw	r3, 0xc08(r7)	/* LAWAR0 */
	lwz	r3, 0xc08(r7)
	isync

	lis	r0, MAS0_TLBSEL(1)@h
	rlwimi	r0, r8, 16, MAS0_ESEL_MSK
	li	r3, 0
	mtspr	MAS0, r0
	mtspr	MAS1, r3
	isync
	tlbwe
	isync
	msync

	b	9f

	/* r3 = addr, r4 = value, clobbers r5, r11, r12 */
erratum_set_value:
	/* Lock two cache lines into I-Cache */
	sync
	mfspr	r11, SPRN_L1CSR1
	rlwinm	r11, r11, 0, ~L1CSR1_ICUL
	sync
	isync
	mtspr	SPRN_L1CSR1, r11
	isync

	mflr	r12
	bl	5f
5:	mflr	r5
	addi	r5, r5, 2f - 5b
	icbtls	0, 0, r5
	addi	r5, r5, 64

	sync
	mfspr	r11, SPRN_L1CSR1
3:	andi.	r11, r11, L1CSR1_ICUL
	bne	3b

	icbtls	0, 0, r5
	addi	r5, r5, 64

	sync
	mfspr	r11, SPRN_L1CSR1
3:	andi.	r11, r11, L1CSR1_ICUL
	bne	3b

	b	2f
	.align	6
	/* Inside a locked cacheline, wait a while, write, then wait a while */
2:	sync

	mfspr	r5, SPRN_TBRL
	addis	r11, r5, 0x10000@h /* wait 65536 timebase ticks */
4:	mfspr	r5, SPRN_TBRL
	subf.	r5, r5, r11
	bgt	4b

	stw	r4, 0(r3)

	mfspr	r5, SPRN_TBRL
	addis	r11, r5, 0x10000@h /* wait 65536 timebase ticks */
4:	mfspr	r5, SPRN_TBRL
	subf.	r5, r5, r11
	bgt	4b

	sync

	/*
	 * Fill out the rest of this cache line and the next with nops,
	 * to ensure that nothing outside the locked area will be
	 * fetched due to a branch.
	 */
	.rept 19
	nop
	.endr

	sync
	mfspr	r11, SPRN_L1CSR1
	rlwinm	r11, r11, 0, ~L1CSR1_ICUL
	sync
	isync
	mtspr	SPRN_L1CSR1, r11
	isync

	mtlr	r12
	blr

9:
#endif

create_init_ram_area:
	lis     r6,FSL_BOOKE_MAS0(1, 15, 0)@h
	ori     r6,r6,FSL_BOOKE_MAS0(1, 15, 0)@l

#ifdef NOR_BOOT
	/* create a temp mapping in AS=1 to the 4M boot window */
	create_tlb1_entry 15, \
		1, BOOKE_PAGESZ_4M, \
		CONFIG_SYS_MONITOR_BASE & 0xffc00000, MAS2_I|MAS2_G, \
		0xffc00000, MAS3_SX|MAS3_SW|MAS3_SR, \
		0, r6

#elif !defined(CONFIG_SYS_RAMBOOT) && defined(CONFIG_SECURE_BOOT)
	/* create a temp mapping in AS = 1 for Flash mapping
	 * created by PBL for ISBC code
	 */
	create_tlb1_entry 15, \
		1, BOOKE_PAGESZ_1M, \
		CONFIG_SYS_MONITOR_BASE & 0xfff00000, MAS2_I|MAS2_G, \
		CONFIG_SYS_PBI_FLASH_WINDOW & 0xfff00000, MAS3_SX|MAS3_SW|MAS3_SR, \
		0, r6

/*
 * For Targets without CONFIG_SPL like P3, P5
 * and for targets with CONFIG_SPL like T1, T2, T4, only for
 * u-boot-spl i.e. CONFIG_SPL_BUILD
 */
#elif defined(CONFIG_RAMBOOT_PBL) && defined(CONFIG_SECURE_BOOT) && \
	(!defined(CONFIG_SPL) || defined(CONFIG_SPL_BUILD))
	/* create a temp mapping in AS = 1 for mapping CONFIG_SYS_MONITOR_BASE
	 * to L3 Address configured by PBL for ISBC code
	 */
	create_tlb1_entry 15, \
		1, BOOKE_PAGESZ_1M, \
		CONFIG_SYS_MONITOR_BASE & 0xfff00000, MAS2_I|MAS2_G, \
		CONFIG_SYS_INIT_L3_ADDR & 0xfff00000, MAS3_SX|MAS3_SW|MAS3_SR, \
		0, r6

#else
	/*
	 * create a temp mapping in AS=1 to the 1M CONFIG_SYS_MONITOR_BASE space, the main
	 * image has been relocated to CONFIG_SYS_MONITOR_BASE on the second stage.
	 */
	create_tlb1_entry 15, \
		1, BOOKE_PAGESZ_1M, \
		CONFIG_SYS_MONITOR_BASE & 0xfff00000, MAS2_I|MAS2_G, \
		CONFIG_SYS_MONITOR_BASE & 0xfff00000, MAS3_SX|MAS3_SW|MAS3_SR, \
		0, r6
#endif

	/* create a temp mapping in AS=1 to the stack */
#if defined(CONFIG_SYS_INIT_RAM_ADDR_PHYS_LOW) && \
    defined(CONFIG_SYS_INIT_RAM_ADDR_PHYS_HIGH)
	create_tlb1_entry 14, \
		1, BOOKE_PAGESZ_16K, \
		CONFIG_SYS_INIT_RAM_ADDR, 0, \
		CONFIG_SYS_INIT_RAM_ADDR_PHYS_LOW, MAS3_SX|MAS3_SW|MAS3_SR, \
		CONFIG_SYS_INIT_RAM_ADDR_PHYS_HIGH, r6

#else
	create_tlb1_entry 14, \
		1, BOOKE_PAGESZ_16K, \
		CONFIG_SYS_INIT_RAM_ADDR, 0, \
		CONFIG_SYS_INIT_RAM_ADDR, MAS3_SX|MAS3_SW|MAS3_SR, \
		0, r6
#endif

	lis	r6,MSR_IS|MSR_DS|MSR_DE@h
	ori	r6,r6,MSR_IS|MSR_DS|MSR_DE@l
	lis	r7,switch_as@h
	ori	r7,r7,switch_as@l

	mtspr	SPRN_SRR0,r7
	mtspr	SPRN_SRR1,r6
	rfi

switch_as:
/* L1 DCache is used for initial RAM */

	/* Allocate Initial RAM in data cache.
	 */
	lis	r3,CONFIG_SYS_INIT_RAM_ADDR@h
	ori	r3,r3,CONFIG_SYS_INIT_RAM_ADDR@l
	mfspr	r2, L1CFG0
	andi.	r2, r2, 0x1ff
	/* cache size * 1024 / (2 * L1 line size) */
	slwi	r2, r2, (10 - 1 - L1_CACHE_SHIFT)
	mtctr	r2
	li	r0,0
1:
	dcbz	r0,r3
#ifdef CONFIG_E6500	/* Lock/unlock L2 cache long with L1 */
	dcbtls	2, r0, r3
	dcbtls	0, r0, r3
#else
	dcbtls	0, r0, r3
#endif
	addi	r3,r3,CONFIG_SYS_CACHELINE_SIZE
	bdnz	1b

	/* Jump out the last 4K page and continue to 'normal' start */
#if defined(CONFIG_SYS_RAMBOOT) || defined(CONFIG_SPL)
	/* We assume that we're already running at the address we're linked at */
	b	_start_cont
#else
	/* Calculate absolute address in FLASH and jump there		*/
	/*--------------------------------------------------------------*/
	lis	r3,CONFIG_SYS_MONITOR_BASE@h
	ori	r3,r3,CONFIG_SYS_MONITOR_BASE@l
	addi	r3,r3,_start_cont - _start
	mtlr	r3
	blr
#endif

	.text
	.globl	_start
_start:
	.long	0x27051956		/* U-BOOT Magic Number */
	.globl	version_string
version_string:
	.ascii U_BOOT_VERSION_STRING, "\0"

	.align	4
	.globl	_start_cont
_start_cont:
	/* Setup the stack in initial RAM,could be L2-as-SRAM or L1 dcache*/
	lis	r3,(CONFIG_SYS_INIT_RAM_ADDR)@h
	ori	r3,r3,((CONFIG_SYS_INIT_SP_OFFSET-16)&~0xf)@l /* Align to 16 */

#if CONFIG_VAL(SYS_MALLOC_F_LEN)
#if CONFIG_VAL(SYS_MALLOC_F_LEN) + GENERATED_GBL_DATA_SIZE > CONFIG_SYS_INIT_RAM_SIZE
#error "SYS_MALLOC_F_LEN too large to fit into initial RAM."
#endif

	/* Leave 16+ byte for back chain termination and NULL return address */
	subi	r3,r3,((CONFIG_VAL(SYS_MALLOC_F_LEN)+16+15)&~0xf)
#endif

	/* End of RAM */
	lis	r4,(CONFIG_SYS_INIT_RAM_ADDR)@h
	ori	r4,r4,(CONFIG_SYS_INIT_RAM_SIZE)@l

	li	r0,0

1: 	subi 	r4,r4,4
	stw 	r0,0(r4)
	cmplw 	r4,r3
	bne	1b

#if CONFIG_VAL(SYS_MALLOC_F_LEN)
	lis	r4,(CONFIG_SYS_INIT_RAM_ADDR)@h
	ori	r4,r4,(CONFIG_SYS_GBL_DATA_OFFSET)@l

	addi	r3,r3,16	/* Pre-relocation malloc area */
	stw	r3,GD_MALLOC_BASE(r4)
	subi	r3,r3,16
#endif
	li	r0,0
	stw	r0,0(r3)	/* Terminate Back Chain */
	stw	r0,+4(r3)	/* NULL return address. */
	mr	r1,r3		/* Transfer to SP(r1) */

	GET_GOT
	/* Needed for -msingle-pic-base */
	bl	_GLOBAL_OFFSET_TABLE_@local-4
	mflr	r30

	/* Pass our potential ePAPR device tree pointer to cpu_init_early_f */
	mr	r3, r24

	bl	cpu_init_early_f

	/* switch back to AS = 0 */
	lis	r3,(MSR_CE|MSR_ME|MSR_DE)@h
	ori	r3,r3,(MSR_CE|MSR_ME|MSR_DE)@l
	mtmsr	r3
	isync

	bl	cpu_init_f	/* return boot_flag for calling board_init_f */
	bl	board_init_f
	isync

	/* NOTREACHED - board_init_f() does not return */

#ifndef MINIMAL_SPL
	.globl	_start_of_vectors
_start_of_vectors:

/* Critical input. */
	CRIT_EXCEPTION(0x0100, CriticalInput, CritcalInputException)

/* Machine check */
	MCK_EXCEPTION(0x200, MachineCheck, MachineCheckException)

/* Data Storage exception. */
	STD_EXCEPTION(0x0300, DataStorage, UnknownException)

/* Instruction Storage exception. */
	STD_EXCEPTION(0x0400, InstStorage, UnknownException)

/* External Interrupt exception. */
	STD_EXCEPTION(0x0500, ExtInterrupt, ExtIntException)

/* Alignment exception. */
Alignment:
	EXCEPTION_PROLOG(SRR0, SRR1)
	mfspr	r4,DAR
	stw	r4,_DAR(r21)
	mfspr	r5,DSISR
	stw	r5,_DSISR(r21)
	addi	r3,r1,STACK_FRAME_OVERHEAD
	EXC_XFER_TEMPLATE(0x600, Alignment, AlignmentException,
		MSR_KERNEL, COPY_EE)

/* Program check exception */
ProgramCheck:
	EXCEPTION_PROLOG(SRR0, SRR1)
	addi	r3,r1,STACK_FRAME_OVERHEAD
	EXC_XFER_TEMPLATE(0x700, ProgramCheck, ProgramCheckException,
		MSR_KERNEL, COPY_EE)

	/* No FPU on MPC85xx.  This exception is not supposed to happen.
	*/
	STD_EXCEPTION(0x0800, FPUnavailable, UnknownException)
	STD_EXCEPTION(0x0900, SystemCall, UnknownException)
	STD_EXCEPTION(0x0a00, Decrementer, timer_interrupt)
	STD_EXCEPTION(0x0b00, IntervalTimer, UnknownException)
	STD_EXCEPTION(0x0c00, WatchdogTimer, UnknownException)

	STD_EXCEPTION(0x0d00, DataTLBError, UnknownException)
	STD_EXCEPTION(0x0e00, InstructionTLBError, UnknownException)

	CRIT_EXCEPTION(0x0f00, DebugBreakpoint, DebugException )

	.globl	_end_of_vectors
_end_of_vectors:


	. = . + (0x100 - ( . & 0xff ))	/* align for debug */

/*
 * This code finishes saving the registers to the exception frame
 * and jumps to the appropriate handler for the exception.
 * Register r21 is pointer into trap frame, r1 has new stack pointer.
 * r23 is the address of the handler.
 */
	.globl	transfer_to_handler
transfer_to_handler:
	SAVE_GPR(7, r21)
	SAVE_4GPRS(8, r21)
	SAVE_8GPRS(12, r21)
	SAVE_8GPRS(24, r21)

	li	r22,0
	stw	r22,RESULT(r21)
	mtspr	SPRG2,r22		/* r1 is now kernel sp */

	mtctr	r23			/* virtual address of handler */
	mtmsr	r20
	bctrl

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

/* Cache functions.
*/
.globl flush_icache
flush_icache:
.globl invalidate_icache
invalidate_icache:
	mfspr	r0,L1CSR1
	ori	r0,r0,L1CSR1_ICFI
	msync
	isync
	mtspr	L1CSR1,r0
	isync
	blr				/* entire I cache */

.globl invalidate_dcache
invalidate_dcache:
	mfspr	r0,L1CSR0
	ori	r0,r0,L1CSR0_DCFI
	msync
	isync
	mtspr	L1CSR0,r0
	isync
	blr

	.globl	icache_enable
icache_enable:
	mflr	r8
	bl	invalidate_icache
	mtlr	r8
	isync
	mfspr	r4,L1CSR1
	ori	r4,r4,(L1CSR1_CPE | L1CSR1_ICE)@l
	oris	r4,r4,(L1CSR1_CPE | L1CSR1_ICE)@h
	mtspr	L1CSR1,r4
	isync
	blr

	.globl	icache_disable
icache_disable:
	mfspr	r0,L1CSR1
	lis	r3,0
	ori	r3,r3,L1CSR1_ICE
	andc	r0,r0,r3
	mtspr	L1CSR1,r0
	isync
	blr

	.globl	icache_status
icache_status:
	mfspr	r3,L1CSR1
	andi.	r3,r3,L1CSR1_ICE
	blr

	.globl	dcache_enable
dcache_enable:
	mflr	r8
	bl	invalidate_dcache
	mtlr	r8
	isync
	mfspr	r0,L1CSR0
	ori	r0,r0,(L1CSR0_CPE |  L1CSR0_DCE)@l
	oris	r0,r0,(L1CSR0_CPE |  L1CSR0_DCE)@h
	msync
	isync
	mtspr	L1CSR0,r0
	isync
	blr

	.globl	dcache_disable
dcache_disable:
	mfspr	r3,L1CSR0
	lis	r4,0
	ori	r4,r4,L1CSR0_DCE
	andc	r3,r3,r4
	mtspr	L1CSR0,r3
	isync
	blr

	.globl	dcache_status
dcache_status:
	mfspr	r3,L1CSR0
	andi.	r3,r3,L1CSR0_DCE
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 in8 */
/* Description:	 Input 8 bits */
/*------------------------------------------------------------------------------- */
	.globl	in8
in8:
	lbz	r3,0x0000(r3)
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 out8 */
/* Description:	 Output 8 bits */
/*------------------------------------------------------------------------------- */
	.globl	out8
out8:
	stb	r4,0x0000(r3)
	sync
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 out16 */
/* Description:	 Output 16 bits */
/*------------------------------------------------------------------------------- */
	.globl	out16
out16:
	sth	r4,0x0000(r3)
	sync
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 out16r */
/* Description:	 Byte reverse and output 16 bits */
/*------------------------------------------------------------------------------- */
	.globl	out16r
out16r:
	sthbrx	r4,r0,r3
	sync
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 out32 */
/* Description:	 Output 32 bits */
/*------------------------------------------------------------------------------- */
	.globl	out32
out32:
	stw	r4,0x0000(r3)
	sync
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 out32r */
/* Description:	 Byte reverse and output 32 bits */
/*------------------------------------------------------------------------------- */
	.globl	out32r
out32r:
	stwbrx	r4,r0,r3
	sync
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 in16 */
/* Description:	 Input 16 bits */
/*------------------------------------------------------------------------------- */
	.globl	in16
in16:
	lhz	r3,0x0000(r3)
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 in16r */
/* Description:	 Input 16 bits and byte reverse */
/*------------------------------------------------------------------------------- */
	.globl	in16r
in16r:
	lhbrx	r3,r0,r3
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 in32 */
/* Description:	 Input 32 bits */
/*------------------------------------------------------------------------------- */
	.globl	in32
in32:
	lwz	3,0x0000(3)
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 in32r */
/* Description:	 Input 32 bits and byte reverse */
/*------------------------------------------------------------------------------- */
	.globl	in32r
in32r:
	lwbrx	r3,r0,r3
	blr
#endif  /* !MINIMAL_SPL */

/*------------------------------------------------------------------------------*/

/*
 * void write_tlb(mas0, mas1, mas2, mas3, mas7)
 */
	.globl	write_tlb
write_tlb:
	mtspr	MAS0,r3
	mtspr	MAS1,r4
	mtspr	MAS2,r5
	mtspr	MAS3,r6
#ifdef CONFIG_ENABLE_36BIT_PHYS
	mtspr	MAS7,r7
#endif
	li	r3,0
#ifdef CONFIG_SYS_BOOK3E_HV
	mtspr	MAS8,r3
#endif
	isync
	tlbwe
	msync
	isync
	blr

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
	mr	r1,r3		/* Set new stack pointer		*/
	mr	r9,r4		/* Save copy of Init Data pointer	*/
	mr	r10,r5		/* Save copy of Destination Address	*/

	GET_GOT
#ifndef CONFIG_SPL_SKIP_RELOCATE
	mr	r3,r5				/* Destination Address	*/
	lis	r4,CONFIG_SYS_MONITOR_BASE@h		/* Source      Address	*/
	ori	r4,r4,CONFIG_SYS_MONITOR_BASE@l
	lwz	r5,GOT(__init_end)
	sub	r5,r5,r4
	li	r6,CONFIG_SYS_CACHELINE_SIZE		/* Cache Line Size	*/

	/*
	 * Fix GOT pointer:
	 *
	 * New GOT-PTR = (old GOT-PTR - CONFIG_SYS_MONITOR_BASE) + Destination Address
	 *
	 * Offset:
	 */
	sub	r15,r10,r4

	/* First our own GOT */
	add	r12,r12,r15
	/* the the one used by the C code */
	add	r30,r30,r15

	/*
	 * Now relocate code
	 */

	cmplw	cr1,r3,r4
	addi	r0,r5,3
	srwi.	r0,r0,2
	beq	cr1,4f		/* In place copy is not necessary	*/
	beq	7f		/* Protect against 0 count		*/
	mtctr	r0
	bge	cr1,2f

	la	r8,-4(r4)
	la	r7,-4(r3)
1:	lwzu	r0,4(r8)
	stwu	r0,4(r7)
	bdnz	1b
	b	4f

2:	slwi	r0,r0,2
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
7:	sync			/* Wait for all icbi to complete on bus */
	isync

/*
 * We are done. Do not return, instead branch to second part of board
 * initialization, now running from RAM.
 */

	addi	r0,r10,in_ram - _start

	/*
	 * As IVPR is going to point RAM address,
	 * Make sure IVOR15 has valid opcode to support debugger
	 */
	mtspr	IVOR15,r0

	/*
	 * Re-point the IVPR at RAM
	 */
	mtspr	IVPR,r10

	mtlr	r0
	blr				/* NEVER RETURNS! */
#endif
	.globl	in_ram
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
clear_bss:
	/*
	 * Now clear BSS segment
	 */
	lwz	r3,GOT(__bss_start)
	lwz	r4,GOT(__bss_end)

	cmplw	0,r3,r4
	beq	6f

	li	r0,0
5:
	stw	r0,0(r3)
	addi	r3,r3,4
	cmplw	0,r3,r4
	blt	5b
6:

	mr	r3,r9		/* Init Data pointer		*/
	mr	r4,r10		/* Destination Address		*/
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
	mflr	r11
	bl	_GLOBAL_OFFSET_TABLE_-4
	mflr	r12

	/* Update IVORs as per relocation */
	mtspr	IVPR,r3

	lwz	r4,CriticalInput@got(r12)
	mtspr	IVOR0,r4	/* 0: Critical input */
	lwz	r4,MachineCheck@got(r12)
	mtspr	IVOR1,r4	/* 1: Machine check */
	lwz	r4,DataStorage@got(r12)
	mtspr	IVOR2,r4	/* 2: Data storage */
	lwz	r4,InstStorage@got(r12)
	mtspr	IVOR3,r4	/* 3: Instruction storage */
	lwz	r4,ExtInterrupt@got(r12)
	mtspr	IVOR4,r4	/* 4: External interrupt */
	lwz	r4,Alignment@got(r12)
	mtspr	IVOR5,r4	/* 5: Alignment */
	lwz	r4,ProgramCheck@got(r12)
	mtspr	IVOR6,r4	/* 6: Program check */
	lwz	r4,FPUnavailable@got(r12)
	mtspr	IVOR7,r4	/* 7: floating point unavailable */
	lwz	r4,SystemCall@got(r12)
	mtspr	IVOR8,r4	/* 8: System call */
	/* 9: Auxiliary processor unavailable(unsupported) */
	lwz	r4,Decrementer@got(r12)
	mtspr	IVOR10,r4	/* 10: Decrementer */
	lwz	r4,IntervalTimer@got(r12)
	mtspr	IVOR11,r4	/* 11: Interval timer */
	lwz	r4,WatchdogTimer@got(r12)
	mtspr	IVOR12,r4	/* 12: Watchdog timer */
	lwz	r4,DataTLBError@got(r12)
	mtspr	IVOR13,r4	/* 13: Data TLB error */
	lwz	r4,InstructionTLBError@got(r12)
	mtspr	IVOR14,r4	/* 14: Instruction TLB error */
	lwz	r4,DebugBreakpoint@got(r12)
	mtspr	IVOR15,r4	/* 15: Debug */

	mtlr	r11
	blr

.globl unlock_ram_in_cache
unlock_ram_in_cache:
	/* invalidate the INIT_RAM section */
	lis	r3,(CONFIG_SYS_INIT_RAM_ADDR & ~(CONFIG_SYS_CACHELINE_SIZE-1))@h
	ori	r3,r3,(CONFIG_SYS_INIT_RAM_ADDR & ~(CONFIG_SYS_CACHELINE_SIZE-1))@l
	mfspr	r4,L1CFG0
	andi.	r4,r4,0x1ff
	slwi	r4,r4,(10 - 1 - L1_CACHE_SHIFT)
	mtctr	r4
1:	dcbi	r0,r3
#ifdef CONFIG_E6500	/* lock/unlock L2 cache long with L1 */
	dcblc	2, r0, r3
	dcblc	0, r0, r3
#else
	dcblc	r0,r3
#endif
	addi	r3,r3,CONFIG_SYS_CACHELINE_SIZE
	bdnz	1b
	sync

	/* Invalidate the TLB entries for the cache */
	lis	r3,CONFIG_SYS_INIT_RAM_ADDR@h
	ori	r3,r3,CONFIG_SYS_INIT_RAM_ADDR@l
	tlbivax	0,r3
	addi	r3,r3,0x1000
	tlbivax	0,r3
	addi	r3,r3,0x1000
	tlbivax	0,r3
	addi	r3,r3,0x1000
	tlbivax	0,r3
	isync
	blr

.globl flush_dcache
flush_dcache:
	mfspr	r3,SPRN_L1CFG0

	rlwinm	r5,r3,9,3	/* Extract cache block size */
	twlgti	r5,1		/* Only 32 and 64 byte cache blocks
				 * are currently defined.
				 */
	li	r4,32
	subfic	r6,r5,2		/* r6 = log2(1KiB / cache block size) -
				 *      log2(number of ways)
				 */
	slw	r5,r4,r5	/* r5 = cache block size */

	rlwinm	r7,r3,0,0xff	/* Extract number of KiB in the cache */
	mulli	r7,r7,13	/* An 8-way cache will require 13
				 * loads per set.
				 */
	slw	r7,r7,r6

	/* save off HID0 and set DCFA */
	mfspr	r8,SPRN_HID0
	ori	r9,r8,HID0_DCFA@l
	mtspr	SPRN_HID0,r9
	isync

	lis	r4,0
	mtctr	r7

1:	lwz	r3,0(r4)	/* Load... */
	add	r4,r4,r5
	bdnz	1b

	msync
	lis	r4,0
	mtctr	r7

1:	dcbf	0,r4		/* ...and flush. */
	add	r4,r4,r5
	bdnz	1b

	/* restore HID0 */
	mtspr	SPRN_HID0,r8
	isync

	blr
#endif /* !MINIMAL_SPL */
