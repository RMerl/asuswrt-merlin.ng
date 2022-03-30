/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _6813_BOOT0_H
#define _6813_BOOT0_H

#include <asm-offsets.h>
#include <linux/linkage.h>
#include <asm/system.h>

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

	/* relocate the code, init'ed data from flash to lmem  */	
relo_image:
	adr     x1, _bcm_boot		/* x1 source address in flash */
	ldr	x0, =__image_copy_start	/* x0 dest address in sram */
	subs	x4, x1, x0		/* x4 relocation offset */
	beq	relo_dtb		/* skip relocation */
	ldr	x2, =__image_copy_end	/* x2 dest ending address in flash */

relo_loop:
	ldp	x5, x6, [x1], #16
	stp	x5, x6, [x0], #16
	cmp	x0, x2
	blo	relo_loop

	/* if we attached dtb after bss, need to relocate dtb as well */	
relo_dtb:
#if defined(CONFIG_SPL_OF_CONTROL) && defined(CONFIG_OF_SEPARATE)

#ifdef CONFIG_OF_SPL_SEPARATE_BSS
	ldr	x3, =__image_binary_end
#else
	ldr	x3, =_end
#endif
	add	x1, x3, x4       /* r1 source address in flash */
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
	ldr	x0, =reset
	br	x0

#else
	b	reset
#endif

#endif
