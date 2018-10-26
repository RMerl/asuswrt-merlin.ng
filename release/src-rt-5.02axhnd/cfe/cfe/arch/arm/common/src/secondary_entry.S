/*  *********************************************************************
    *
    <:copyright-BRCM:2018:proprietary:standard
    
       Copyright (c) 2018 Broadcom 
       All Rights Reserved
    
     This program is the proprietary software of Broadcom and/or its
     licensors, and may only be used, duplicated, modified or distributed pursuant
     to the terms and conditions of a separate, written license agreement executed
     between you and Broadcom (an "Authorized License").  Except as set forth in
     an Authorized License, Broadcom grants no license (express or implied), right
     to use, or waiver of any kind with respect to the Software, and Broadcom
     expressly reserves all rights in and to the Software and all intellectual
     property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
     NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
     BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
    
     Except as expressly set forth in the Authorized License,
    
     1. This program, including its structure, sequence and organization,
        constitutes the valuable trade secrets of Broadcom, and you shall use
        all reasonable efforts to protect the confidentiality thereof, and to
        use this information only in connection with your use of Broadcom
        integrated circuit products.
    
     2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
        AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
        WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
        RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
        ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
        FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
        COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
        TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
        PERFORMANCE OF THE SOFTWARE.
    
     3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
        ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
        INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
        WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
        IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
        OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
        SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
        SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
        LIMITED REMEDY.
    :>
    ********************************************************************
*/
#include <arm.h>
#include <armmacros.h>
#include "cpu_config.h"
#include "bsp_config.h"
#include "bcm_map.h"

/*  *********************************************************************
    *  secondary_startup
    *
    *  Eentry point for secondary CPUs. The code must be pic independent
    *  and all subroutine within ths block. This boot code will be relocated to
    *  a reserved, secure access only memory area for Linux to use to dynamically
    *  power on/off cores. So it must NOT do the following:
    *   - call a function into cfe ram code. The function must be embedded in
    *     this file
    *   - use any symbol directly such as ldr r0, =<sybmol>.  Use mov r0, #imm
    *     if possible. Otherwise, use movw and movt pair. If the symbol is in
    *     this file, use adrl instruction
    *
    *  This function copies most main cpu init sequence and inline the
    *  subroutines. Then switch to unsecure world and enter WFI state waiting
    *  linux to wake it up
    ********************************************************************* */	
	
/* This file must use relocatable code. The code is copied to a bootstrap
   area at runtime and keep active in the linux for secondary cpus power up
   and down.  
*/
#if defined(CFG_RAMAPP) && !defined(_BCM947189_)
	.align 3
	.global sec_entry_begin
sec_entry_begin:
	b   secondary_reset
	nop
FUNC(secondary_startup)

cntfrq:
	.word 0x02FAF080
#if defined(_BCM96846_)
biucfgb:
	.word BIUCFG_BASE
#endif
gpiobase:
	.word GPIO_BASE
gicdb:
	.word GICD_BASE
giccb:
	.word GICC_BASE
grpd:
	.word 0xffffffff

secondary_reset:
	mrs	r0, cpsr
	mov	r1, #(PS_MM)
	bic	r0, r0, r1
	/* Switch to system mode and make sure interrupts are disabled */
	mov	r1, #(PS_I | PS_F | PS_SVC)
	orr	r0, r0, r1
	msr	cpsr, r0

	/* Initialize system control register */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #(CR_C|CR_A|CR_M)	/* Clear C, A, M, bits */
	bic	r0, r0, #(CR_Z|CR_I)		/* clear Z, I bits */
	orr	r0, r0, #CR_V			/* Set V bit: high vectors (0xffff0000) */
	mcr	p15, 0, r0, c1, c0, 0
	isb
	
	mov	r0, #0
	mcr	p15, 0, r0, c8, c7, 0	/* Invalidate TLB */
	mcr	p15, 0, r0, c7, c5, 0	/* Invalidate icache */
	mcr	p15, 0, r0, c7, c5, 6	/* Invalidate entire branch predictor array */

	
#if defined(_BCM96846_) || defined(_BCM963148_)
	/* Enable SMP */
	mrc	p15, 0, r1, c1, c0, 1
	orr	r1, r1, #(1<<6)
	mcr	p15, 0, r1, c1, c0, 1
	/* set the system clock (enable timestamp0 counter) */
	adrl	r0, cntfrq
	ldr	r0, [r0]
	mcr	p15, 0, r0, c14, c0, 0
#endif

#if defined(CFG_NONSEC_BOOT)
	/* setup the GIC for non secure world. group 0 secure state, group 1 non secure state */
	/* enable interrupt for both groups */
	adrl	r0, gicdb
	ldr	r0, [r0]
	mov	r1, #0x3
	str	r1, [r0, #GICD_CTLR_OFFSET]

	/* assign all the interrupt to group 1 */
	ldr	r2, [r0, #GICD_TYPER_OFFSET]
	and	r2, r2, #0x1f
	add	r2, r2, #1
	adrl	r1, grpd
	ldr	r1, [r1]
	add	r0, r0, #GICD_IGROUPR0_OFFSET	
setgroup:
	str	r1, [r0]
	add	r0, #4
	subs	r2, r2, #1
	bne	setgroup
 	

	adrl	r0, giccb
	ldr	r0, [r0]
	/* enable both group interrupt in the cpu interface */	
	ldr	r1, [r0, #GICC_CTLR_OFFSET]
	orr	r1, #0x3
	str	r1, [r0, #GICC_CTLR_OFFSET]

	 /* set cpu priority mask view for nonsecure */
	mov	r1, #0x80
	str	r1, [r0, #GICC_PMR_OFFSET]
	
	/* setup the monitor mode vector */
	adrl	r0, sec_mon_vector	
	mcr	p15, 0, r0, c12, c0, 1
	isb

	/* set non-secure ACR. Allow SMP, L2ERR, CP10 and CP11 and Enable Neon/VFP bit for non-secure mode */
	movw	r0, #0x0c00
	movt	r0, #0x0006
	mcr	p15, 0, r0, c1, c1, 2
#if defined(_BCM963138_)
	/* set FW bit in ACTLR, SMP is set by linux */
	mrc	p15, 0, r1, c1, c0, 1
	orr	r1, r1, #0x1
	mcr	p15, 0, r1, c1, c0, 1

	/* invalidate cache in the secure state. unsecure invalidation
	can't touch the secure cache entry */
	adrl	r0, l1cache_inval_d
	blx	r0
#endif
	
label_smc:
	.arch_extension sec

	/* switch to unsecure mode */
	smc	#0

#else
	/* enable both group interrupt in the cpu interface */
	adrl	r0, giccb
	ldr	r0, [r0]
	ldr	r1, [r0, #GICC_CTLR_OFFSET]
	orr	r1, #0x1
	str	r1, [r0, #GICC_CTLR_OFFSET]

	mov	r1, #0x80
	str	r1, [r0, #GICC_PMR_OFFSET]
#endif

	wfi	 /* wait for linux */
	
	/* read linux entry point */
	adrl	r0, gpiobase
	ldr	r0, [r0]
	ldr	r1, [r0, #GP_OFFSET]

	/* jump to linux */
	mov	pc, r1

/* vector table for secure monitor mode */
	.align	5
sec_mon_vector:
	.word	0
	.word	0
	b	sec_monitor_handler
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0

/*
 * monitor mode exception handler
 * Currently this is only for switching cpu from secure state to nonsecure state
 * when cfe ram boot to linux.
 * It does not save any registers. r0, r1, r2, r3 is passed to kernel as argument 
 * r4 kernel entry point address
 */
sec_monitor_handler:
	mrc	p15, 0, r5, c1, c1, 0
	/* turn on NS bit */
	orr	r5, r5, #0x1
	mcr	p15, 0, r5, c1, c1, 0
	isb

	mov	r5, #(PS_F|PS_I|PS_A)
	tst	lr, #1
	orrne	r5, r5, #PS_T
	orr	r5, r5, #PS_SVC
	msr	spsr_cxfs, r5
	movs	pc, lr				/* ERET to non-secure SVC mode*/

#if defined(_BCM963138_)
l1cache_inval_d:
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
	isb
						@ but we compile with armv5
	mrc	p15, 1, r1, c0, c0, 0		@ read the new csidr
	and	r2, r1, #7			@ extract the length of the cache lines
	add	r2, r2, #4			@ add 4 (line length offset)
	mov	r4, #0
	movw	r4, #0x3ff
	ands	r4, r4, r1, lsr #3		@ find maximum number on the way size
	clz	r5, r4				@ find bit position of way size increment
	mov	r7, #0
	movw	r7, #0x7fff
	ands	r7, r7, r1, lsr #13		@ extract max number of the index size
invloop2:
	mov	r9, r4				@ create working copy of max way size
invloop3:
	orr	r6, r10, r9, lsl r5		@ factor way and cache number into r6
	orr	r6, r6, r7, lsl r2		@ factor index number into r6
	mcr	p15, 0, r6, c7, c6, 2		@ invalidate by set/way
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
	isb
	mov	pc, lr
#endif

END(secondary_startup)
	.global sec_entry_end
sec_entry_end:
#endif

