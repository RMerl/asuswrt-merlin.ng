/*  *********************************************************************
    *
    <:copyright-BRCM:2015:proprietary:standard
    
       Copyright (c) 2015 Broadcom 
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
    ********************************************************************* */

#include <armv8.h>
#include <armmacros.h>
#include <exception.h>

/*
 * Common code/data start here...
 */	
	.data
/* C trap handler */
	DDW(trap_hook, 0)
	
	.text

/*  *********************************************************************
    *  armv8_exception_init(void)
    *  
    *  exception handler initialization. setup base address, exception mode
    *  stack use and etc	
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */

FUNC(armv8_exception_init)
	ldr	x0, =vectors_base
	msr	VBAR_EL3, x0

	msr	SPSel, #1	/* use SP_ELn for except stack */
	
	ret
END(armv8_exception_init)

FUNC(cfe_warmstart)
	mrs	x0, RVBAR_EL3
	br	x0

END(cfe_warmstart)

/*
 * Setup the trap handler.
 */
FUNC(_exc_set_trap)
	ldr	x2, =trap_hook
	ldr	x1, [x2]
	str	x0, [x2]
	mov	x0, x1

	ret
END(_exc_set_trap)


/*  *********************************************************************
    *  _exc_restart()
    *
    *  Restart the firmware at the boot address
    *
    *  Input parameters:
    *	   nothing
    *
    *  Return value:
    *	   nothing
    ********************************************************************* */

FUNC(_exc_restart)
	mrs	x0, RVBAR_EL3
	br	x0

END(_exc_restart)

#define prepare_trap \
	/* push x0-x7 to stack first so they can used as scratchpad */	\
	sub	sp, sp, #(TRAP_T_SIZE-TR_REGS);				\
	stp	x0, x1, [sp], #16;					\
	stp	x2, x3, [sp], #16;					\
	stp	x4, x5, [sp], #16;					\
	stp	x6, x7, [sp], #16;					\
	/* restore sp value */						\
	add	sp, sp, #(TRAP_T_SIZE-TR_REG(8))


	.globl vectors_base

	.align 11
vectors_base:

	/* current EL with SP0 */
sp0_vector_sync:
	prepare_trap
	adr	x0, sp0_vector_sync	/* x0 exc entry point */
	mov	x1, sp
	b	trap

	.align 7
sp0_vector_irq:
	b	sp0_vector_irq

	.align 7
sp0_vector_fiq:
	b	sp0_vector_fiq

	.align 7
sp0_vector_serr:
	prepare_trap
	adr	x0, sp0_vector_serr	/* x0 exc entry point */
	mov	x1, sp
	b	trap

	/* current EL with SPx */
	.align 7
spx_vector_sync:
	prepare_trap
	adr	x0, spx_vector_sync	/* x0 exc entry point */
	mov	x1, sp
	b	trap

	.align 7
spx_vector_irq:
	b	spx_vector_irq

	.align 7
spx_vector_fiq:
	b	spx_vector_fiq

	.align 7
spx_vector_serr:
	prepare_trap
	adr	x0, spx_vector_serr	/* x0 exc entry point */
	mov	x1, sp
	b	trap

	/* from lower EL using Aarch64 */
	.align 7
l64_vector_sync:
	prepare_trap
	adr	x0, l64_vector_sync	/* x0 exc entry point */
	mov	x1, sp
	b	trap

	.align 7
l64_vector_irq:
	b	l64_vector_irq

	.align 7
l64_vector_fiq:
	b	l64_vector_fiq

	.align 7
l64_vector_serr:
	prepare_trap
	adr	x0, l64_vector_serr	/* x0 exc entry point */
	mov	x1, sp
	b	trap

	/* from lower EL using Aarch32 */
	.align 7
l32_vector_sync:
	prepare_trap
	adr	x0, l32_vector_sync	/* x0 exc entry point */
	mov	x1, sp
	b	trap

	.align 7
l32_vector_irq:
	b	l32_vector_irq

	.align 7
l32_vector_fiq:
	b	l32_vector_fiq

	.align 7
l32_vector_serr:
	prepare_trap
	adr	x0, l32_vector_serr	/* x0 exc entry point */
	mov	x1, sp
	b	trap


	/* Generic exception handler */
trap:
	/* construt the trap structure in SP_ELx stack */
	sub	sp, sp, #TRAP_T_SIZE
	str	x1, [sp, #TR_SP]
	ldr	x1, =vectors_base
	sub	x0, x0, x1
	lsr	x0, x0, #7 	/* each vector entry is 128 bytes */
	str	w0, [sp, #TR_TYPE]
	mrs	x0, ESR_EL3
	str	w0, [sp, #TR_ESR]
	mrs	x0, SPSR_EL3
	str	w0, [sp, #TR_SPSR]
	/* construct the CSPR from PSTATE */
	mov	x0, xzr
	mrs	x1, NZCV
	orr	x0, x0, x1
	mrs	x1, SPSel
	orr	x0, x0, x1
	mrs	x1, CurrentEL
	orr	x0, x0, x1
	mrs	x1, DAIF
	orr	x0, x0, x1
	str	w0, [sp, #TR_CPSR]
	mrs	x0, ELR_EL3
	str	x0, [sp, #TR_ELR]
	mrs	x0, FAR_EL3
	str	x0, [sp, #TR_FAR]
	
	/* save x8 to x30 */
	stp	x8, x9, [sp, #TR_REG(8)]
	stp	x10, x11, [sp, #TR_REG(10)]
	stp	x12, x13, [sp, #TR_REG(12)]
	stp	x14, x15, [sp, #TR_REG(14)]
	stp	x16, x17, [sp, #TR_REG(16)]
	stp	x18, x19, [sp, #TR_REG(18)]
	stp	x20, x21, [sp, #TR_REG(20)]
	stp	x22, x23, [sp, #TR_REG(22)]
	stp	x24, x25, [sp, #TR_REG(24)]
	stp	x26, x27, [sp, #TR_REG(26)]
	stp	x28, x29, [sp, #TR_REG(28)]
	str	x30, [sp, #TR_REG(30)]

	/* If trap_hook is null, stay in a dead loop */
	ldr	x1, =trap_hook
	ldr	x1, [x1]
	cmp	x1, #0
1:	beq	1b
	/* else call (*trap_hook)(trap_t) */
	mov	x0, sp
	blr	x1

	/* restore x0 to x30 */
	ldp	x0, x1, [sp, #TR_REG(0)]
	ldp	x2, x3, [sp, #TR_REG(2)]
	ldp	x4, x5, [sp, #TR_REG(4)]
	ldp	x6, x7, [sp, #TR_REG(6)]
	ldp	x8, x9, [sp, #TR_REG(8)]
	ldp	x10, x11, [sp, #TR_REG(10)]
	ldp	x12, x13, [sp, #TR_REG(12)]
	ldp	x14, x15, [sp, #TR_REG(14)]
	ldp	x16, x17, [sp, #TR_REG(16)]
	ldp	x18, x19, [sp, #TR_REG(18)]
	ldp	x20, x21, [sp, #TR_REG(20)]
	ldp	x22, x23, [sp, #TR_REG(22)]
	ldp	x24, x25, [sp, #TR_REG(24)]
	ldp	x26, x27, [sp, #TR_REG(26)]
	ldp	x28, x29, [sp, #TR_REG(28)]
	ldr	x30, [sp, #TR_REG(30)]

	/* restore SP and return to ELR */
	add	sp, sp, #TRAP_T_SIZE
	eret
