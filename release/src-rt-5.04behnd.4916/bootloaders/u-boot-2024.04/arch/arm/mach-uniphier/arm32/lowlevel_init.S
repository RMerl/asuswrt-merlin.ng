/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012-2015 Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <config.h>
#include <linux/linkage.h>
#include <linux/sizes.h>
#include <asm/system.h>

ENTRY(lowlevel_init)
	mov	r8, lr			@ persevere link reg across call

	/*
	 * The UniPhier Boot ROM loads SPL code to the L2 cache.
	 * But CPUs can only do instruction fetch now because start.S has
	 * cleared C and M bits.
	 * First we need to turn on MMU and Dcache again to get back
	 * data access to L2.
	 */
	mrc	p15, 0, r0, c1, c0, 0	@ SCTLR (System Control Register)
	orr	r0, r0, #(CR_C | CR_M)	@ enable MMU and Dcache
	mcr	p15, 0, r0, c1, c0, 0

#ifdef CONFIG_DEBUG_LL
	bl	debug_ll_init
#endif

	bl	setup_init_ram		@ RAM area for stack and page table

	/*
	 * Now we are using the page table embedded in the Boot ROM.
	 * What we need to do next is to create a page table and switch
	 * over to it.
	 */
	bl	create_page_table
	bl	__v7_flush_dcache_all

	/* Disable MMU and Dcache before switching Page Table */
	mrc	p15, 0, r0, c1, c0, 0	@ SCTLR (System Control Register)
	bic	r0, r0, #(CR_C | CR_M)	@ disable MMU and Dcache
	mcr	p15, 0, r0, c1, c0, 0

	bl	enable_mmu

	mov	lr, r8			@ restore link
	mov	pc, lr			@ back to my caller
ENDPROC(lowlevel_init)

ENTRY(enable_mmu)
	mrc	p15, 0, r0, c2, c0, 2	@ TTBCR (Translation Table Base Control Register)
	bic	r0, r0, #0x37
	orr	r0, r0, #0x20		@ disable TTBR1
	mcr	p15, 0, r0, c2, c0, 2

	orr	r0, r12, #0x8		@ Outer Cacheability for table walks: WBWA
	mcr	p15, 0, r0, c2, c0, 0   @ TTBR0

	mov	r0, #0
	mcr	p15, 0, r0, c8, c7, 0	@ invalidate TLBs

	mov	r0, #-1			@ manager for all domains (No permission check)
	mcr	p15, 0, r0, c3, c0, 0   @ DACR (Domain Access Control Register)

	dsb
	isb
	/*
	 * MMU on:
	 * TLBs was already invalidated in "../start.S"
	 * So, we don't need to invalidate it here.
	 */
	mrc	p15, 0, r0, c1, c0, 0	@ SCTLR (System Control Register)
	orr	r0, r0, #(CR_C | CR_M)	@ MMU and Dcache enable
	mcr	p15, 0, r0, c1, c0, 0

	mov	pc, lr
ENDPROC(enable_mmu)

/*
 * For PH1-Pro4 or older SoCs, the size of WAY is 32KB.
 * It is large enough for tmp RAM.
 */
#define BOOT_RAM_SIZE	(SZ_32K)
#define BOOT_RAM_BASE	((CONFIG_SPL_STACK) - (BOOT_RAM_SIZE))
#define BOOT_RAM_WAYS	(0x00000100)	@ way 8

#define SSCO_BASE		0x506c0000
#define SSCOPE			0x244
#define SSCOQM			0x248
#define SSCOQAD			0x24c
#define SSCOQSZ			0x250
#define SSCOQWN			0x258
#define SSCOPPQSEF		0x25c
#define SSCOLPQS		0x260

ENTRY(setup_init_ram)
	ldr	r1, = SSCO_BASE

	/* Touch to zero for the boot way */
0:	ldr	r0, = 0x00408006	@ touch to zero with address range
	str	r0, [r1, #SSCOQM]
	ldr	r0, = BOOT_RAM_BASE
	str	r0, [r1, #SSCOQAD]
	ldr	r0, = BOOT_RAM_SIZE
	str	r0, [r1, #SSCOQSZ]
	ldr	r0, = BOOT_RAM_WAYS
	str	r0, [r1, #SSCOQWN]
	ldr	r0, [r1, #SSCOPPQSEF]
	cmp	r0, #0			@ check if the command is successfully set
	bne	0b			@ try again if an error occurs

1:	ldr	r0, [r1, #SSCOLPQS]
	cmp	r0, #0x4
	bne	1b			@ wait until the operation is completed
	str	r0, [r1, #SSCOLPQS]	@ clear the complete notification flag

	mov	pc, lr
ENDPROC(setup_init_ram)

#define DEVICE	0x00002002 /* Non-shareable Device */
#define NORMAL	0x0000000e /* Normal Memory Write-Back, No Write-Allocate */

ENTRY(create_page_table)
	ldr	r0, = DEVICE
	ldr	r1, = BOOT_RAM_BASE
	mov	r12, r1			@ r12 is preserved during D-cache flush
0:	str	r0, [r1], #4		@ specify all the sections as Device
	adds	r0, r0, #0x00100000
	bcc	0b

	ldr	r0, = NORMAL
	str	r0, [r12]		@ mark the first section as Normal
	add	r0, r0, #0x00100000
	str	r0, [r12, #4]		@ mark the second section as Normal
	mov	pc, lr
ENDPROC(create_page_table)
