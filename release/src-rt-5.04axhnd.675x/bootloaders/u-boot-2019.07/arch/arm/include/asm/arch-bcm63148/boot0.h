/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _63148_BOOT0_H
#define _63148_BOOT0_H

#include <asm-offsets.h>
#include <linux/linkage.h>
#include <asm/system.h>

#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
_bcm_boot:
	mov	r0,#0
	mcr	p15,0,r0,c8,c7,0		/* Invalidate TLB */
	mcr	p15,0,r0,c7,c5,0		/* Invalidate icache */
	
	/* Initialize system control register enable i-cache */
	mrc	p15,0,r0,c1,c0,0
	bic	r0,r0,#(CR_C|CR_A|CR_M)		/* Clear C, A, M bits */
	orr	r0,r0,#CR_I			/* Set I bit: enable instruction cache */
	mcr	p15,0,r0,c1,c0,0

	isb

#include <asm/arch/pmc_ll_init.h>

	/* relocate the code, init'ed data from flash to lmem  */	
relo_image:
	adr		r1, _bcm_boot	/* r1 source address in flash */
	ldr		r0, =_bcm_boot	/* r0 dest address in sram */
	subs	r4, r1, r0		/* r4 relocation offset */
	beq		relo_dtb		/* skip relocation */
	  
	ldr		r0, =__image_copy_start	/* r0 dest address in sram */
	add		r1, r0, r4				/* r1 source address in flash */  
	ldr		r2, =__image_copy_end	/* r2 dest ending address in flash */
relo_loop:
	ldmia	r1!, {r10-r11}
	stmia	r0!, {r10-r11}
	cmp	r0, r2
	blo	relo_loop

	/* if we attached dtb after bss, need to relocate dtb as well */	
relo_dtb:
#if defined(CONFIG_SPL_OF_CONTROL) && defined(CONFIG_OF_SEPARATE)

#ifdef CONFIG_OF_SPL_SEPARATE_BSS
	ldr	r3, =__image_binary_end
#else
	ldr	r3, =__bss_end
#endif
	add	r1, r3, r4       /* r1 source address in flash */
	/* check ftd size ... */
	/* struct fdt_header {
	fdt32_t magic;
	fdt32_t totalsize; */
	ldr	r0, [r1, #4]	/* r0 total size */
	rev	r0, r0          /* byte order from fdt to little endian */
	lsr	r0, r0, #2      /* in the order of 4 bytes aligned */
	add	r0, r0, #1
	lsl	r0, r0, #2 
	add	r2, r1, r0       /* r2 dest ending address in flash */
	mov	r0, r3           /* r0 dest address in sram */

dtb_loop:
	ldmia	r1!, {r10-r11}
	stmia	r0!, {r10-r11}
	cmp	r1, r2
	blo	dtb_loop
#endif
	ldr	r0, =reset
	bx	r0

#endif
	.align(5), 0x0
_start:
	ARM_VECTORS
#endif
