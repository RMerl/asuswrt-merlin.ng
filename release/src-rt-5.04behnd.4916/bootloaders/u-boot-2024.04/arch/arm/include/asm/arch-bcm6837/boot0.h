/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2021 Broadcom Ltd.
 */

#ifndef _6837_BOOT0_H
#define _6837_BOOT0_H

#include <asm-offsets.h>
#include <linux/linkage.h>
#include <asm/system.h>

#define SRAM_BASE       0x82000000
#define SRAM_SIZE       0x80000
#define HIF_WIN_BASE    0x84400000 

#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
_bcm_boot:

	tlbi	alle3			/* Invalidate TLB */
	dsb	sy
	isb

	ic	ialluis			/* Invalidate icache */
	isb

	/* Initialize system control register enable i-cache */
	mrs	x0, sctlr_el3
	movn	x1, #(CR_M | CR_C)
	and	x0, x1, x0
	orr	x0, x0, #CR_I
	msr	sctlr_el3, x0
	isb

	b relo_start

	.org    0x100
	.word   1
	.org    0x140

relo_start:
	ldr x8, =HIF_WIN_BASE //HIF window start
	ldr x9, =0x000FFFFF

	/* Copy the bootcode  SRAM*/
	ldr x2, =SRAM_BASE    /*SRAM destination address */
	ldr x3, =SRAM_SIZE        /*SRAM size */
	add x2, x2, x3
	sub x2, x2, #0x400    /* Drop to SRAM SIZE - 0x400, within the stack area */
	mov x10, x2           /* keep it in x10 for future. */

	ldr x3, =relo_image  
	and x3, x3, x9
	orr x3, x3, x8       /* translate SRAM address to HIF */ 
	ldr x4, =relo_end
	and x4, x4, x9
	orr x4, x4, x8       /* translate SRAM address to HIF */ 

.align 3
copy_2_sram:

	ldp x5, x6, [x3], #16    /* copy from hif source address [x0] */
	stp x5, x6, [x2], #16    /* copy to sram dest address [x1] */
	cmp x3,x4           /* until source end address */

	blo copy_2_sram
	
	ldr x20, =relo_end /* Save the address to continue after copy*/

	ldr x0, =__image_copy_start
	and x0, x0, x9
	orr x0, x0, x8               /* translate SRAM address to HIF */            
	ldr x1, =__image_copy_end
	ldr x2, =__image_copy_start

	isb
	br x10               /* Now jump to an address in SRAM */

/* This is the second stage, now we are running from SRAM tiny code that
   copies all bootloader from flash to SRAM */
.align 3
relo_image:

	ldp x5, x6, [x0], #16    /* copy from source address [x0] */
	stp x5, x6, [x2], #16    /* copy from source address [x1] */
	cmp x1, x2           /* until source end address */
	bge relo_image
	
	isb
	br x20   /* jump to newly copied code */

relo_end:

	/* if we attached dtb after bss, need to relocate dtb as well */	
relo_dtb:
#if defined(CONFIG_SPL_OF_CONTROL) && defined(CONFIG_OF_SEPARATE)

#ifdef CONFIG_OF_SPL_SEPARATE_BSS
	ldr	x3, =__image_binary_end
#else
	ldr	x3, =_end
#endif

    and x1, x3, x9
    orr x1, x1, x8

	mov	x0, xzr
	mov	x2, xzr
	/* check ftd size ... */
	/* struct fdt_header {
	fdt32_t magic;
	fdt32_t totalsize; */
	ldr	w0, [x1, #4]	/* r0 total size */
	rev	w0, w0          /* byte order from fdt to little endian */
	lsr	w0, w0, #2      /* in the order of 4 bytes aligned */
	add	w0, w0, #1
	lsl	w0, w0, #2 
	add	x2, x1, x0       /* r2 dest ending address in flash */
	mov	x0, x3           /* r0 dest address in sram */

dtb_loop:
	ldp	x5, x6, [x1], #16
	stp	x5, x6, [x0], #16
	cmp	x1, x2
	blo	dtb_loop
#endif

/*	mrs	x0, sctlr_el3
	movn	x1, #CR_I
	and	x0, x1, x0
	msr	sctlr_el3, x0
	isb
*/
	ldr	x0, =reset
	br	x0

#else
	b	reset
#endif

#endif
