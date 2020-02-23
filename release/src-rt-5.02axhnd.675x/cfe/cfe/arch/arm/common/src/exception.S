/*  *********************************************************************
    *  BCM947xx Board Support Package
    *  
    *  Exception Handler			File: exception.S
    *  
    *  Author: 
    *  
    *********************************************************************  
    *
    *  Copyright 2000,2001,2002,2003
    *  Broadcom Corporation. All rights reserved.
    *  
    *  This software is furnished under license and may be used and 
    *  copied only in accordance with the following terms and 
    *  conditions.  Subject to these conditions, you may download, 
    *  copy, install, use, modify and distribute modified or unmodified 
    *  copies of this software in source and/or binary form.  No title 
    *  or ownership is transferred hereby.
    *  
    *  1) Any source code used, modified or distributed must reproduce 
    *     and retain this copyright notice and list of conditions 
    *     as they appear in the source file.
    *  
    *  2) No right is granted to use any trade name, trademark, or 
    *     logo of Broadcom Corporation.  The "Broadcom Corporation" 
    *     name may not be used to endorse or promote products derived 
    *     from this software without the prior written permission of 
    *     Broadcom Corporation.
    *  
    *  3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR
    *     IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED
    *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
    *     PURPOSE, OR NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT 
    *     SHALL BROADCOM BE LIABLE FOR ANY DAMAGES WHATSOEVER, AND IN 
    *     PARTICULAR, BROADCOM SHALL NOT BE LIABLE FOR DIRECT, INDIRECT,
    *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
    *     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    *     GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    *     BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
    *     OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
    *     TORT (INCLUDING NEGLIGENCE OR OTHERWISE), EVEN IF ADVISED OF 
    *     THE POSSIBILITY OF SUCH DAMAGE.
    ********************************************************************* */


#include <arm.h>
#include <armmacros.h>

/*
 * Common code/data start here...
 */
	.data
/* C trap handler */
	DW(trap_hook, 0)

#define EXCEPTION_STACK_SIZE	0x1000
	.globl exc_stack
exc_stack:
	.space  EXCEPTION_STACK_SIZE

	.text

#if defined(CFG_RAMAPP)
/* vector table for secure monitor mode */
	.align	5
mon_vector:
	.word	0
	.word	0
	b	monitor_handler
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
monitor_handler:
	mrc	p15, 0, r5, c1, c1, 0
	/* turn on NS bit */
	orr	r5, r5, #0x1
	mcr	p15, 0, r5, c1, c1, 0
	isb

	mov	lr, r4
	mov	r5, #(PS_F|PS_I|PS_A)
	tst	lr, #1
	orrne	r5, r5, #PS_T
	orr	r5, r5, #PS_SVC
	msr	spsr_cxfs, r5
	movs	pc, lr				/* ERET to non-secure SVC mode*/
#endif

/* Interrupt handlers */
/* Note: hndrte_arm.c for EXT_CBALL expects each trap handler to be 16 bytes */
#define TRAP_T_SIZE     80

	.globl tr_und
	.globl tr_swi
	.globl tr_iab
	.globl tr_dab
	.globl tr_bad
	.globl tr_fiq
	.globl tr_irq

LEAF(__traps)

/* Undefined instruction exception handler */
tr_und:
	srsdb	sp!, #PS_SYS	/* use system mode stack */
	cps	#PS_SYS		

	/* now cr4 is in system mode */
	/* lr has the value before the exception, push it to stack */
	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}	/* save r0-r7 to the stack */
	eor	r0,r0,r0
	add	r0,r0,#TR_UND
	b	trap

/* Software interrupt exception handler */
tr_swi:
	srsdb	sp!, #PS_SYS	/* use system mode stack */
	cps	#PS_SYS

	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}
	eor	r0,r0,r0
	add	r0,r0,#TR_SWI
	b	trap

/* Prefetch abort exception handler */
tr_iab:
	/* adjust lr to the address of abort insturction fetch */
	mov 	sp,lr
	sub	sp,sp,#4
	mov 	lr,sp
	srsdb	sp!, #PS_SYS	/* use system mode stack */
	cps	#PS_SYS

	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}
	eor	r0,r0,r0
	add	r0,r0,#TR_IAB
	b	trap

/* Data abort exception handler */
tr_dab:
	/* adjust lr to the address of aborted data fetch */
	mov 	sp,lr
	sub	sp,sp,#8
	mov 	lr,sp
	srsdb	sp!, #PS_SYS	/* use system mode stack */
	cps	#PS_SYS	

	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}
	eor	r0,r0,r0
	add	r0,r0,#TR_DAB
	b	trap

tr_bad: /* reserved */
	mov 	sp,lr
	sub	sp,sp,#4
	mov 	lr,sp
	srsdb	sp!, #PS_SYS	/* use system mode stack */
	cps	#PS_SYS

	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}
	eor	r0,r0,r0
	add	r0,r0,#TR_BAD
	b	trap

/* Interrupt handler */
tr_irq:
	/* subtract the offset */
	mov 	sp,lr
	sub	sp,sp,#4
	mov 	lr,sp
	srsdb	sp!, #PS_SYS	/* use system mode stack */
	cps	#PS_SYS

	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}
	eor	r0,r0,r0
	add	r0,r0,#TR_IRQ
	b	trap


/* Fast interrupt handler */
tr_fiq:
	mov 	sp,lr
	sub	sp,sp,#4
	mov 	lr,sp
	srsdb	sp!, #PS_SYS	/* use system mode stack */
	cps	#PS_SYS

	push 	{r0}	/* hold the place for r15 */
	push	{lr}
	sub	sp,sp,#24	/* skip r8-r13 now */
	push 	{r0-r7}
	eor	r0,r0,r0
	add	r0,r0,#TR_FIQ
	b	trap

/* Generic exception handler */
trap:
	/*
	 * construt the trap structure in system mode stack 
	*/
	/* save trap type, epc, cpsr and spsr */
	mov 	r4, sp	/* sp is now at r0 */
	add	r4,r4,#64
	ldmia	r4!, {r1,r3} /* r1<-epc, r3<-spsr */
	mrs	r2, cpsr
	push	{r0,r1,r2,r3}	/* trap type(r0), epc(r1), cpsr,spsr */

	/* fix the value of pc in trap structure */
	sub	r4,r4,#12
	str	r1,[r4]	/* save the value or epc as r15 into the stack */ 

	/* switch back to mode where exception from
	   to get the sp and lr */
	msr	cpsr,r3
	
	/* fix the value of lr in trap structure */
	sub	r4,r4,#4
	str	lr,[r4]	/* lr=epc */ 
	mov	r7, sp
        /* switch back to sys mode */
	msr	cpsr,r2
	
	/* save r8 - r13 */
	/* r7 is the original mode sp pointer */
	mov 	r6,r12
	mov	r5,r11
	mov	r4,r10
	mov	r3,r9
	mov	r2,r8
	add	sp,sp,#72 /* move sp to r14 */
	push	{r2-r7}	/* save r8-r13 to stack */		

	sub	sp,sp,#48 /* move sp back to the top of trap structure */	

	/* If trap_hook is null, go to the end */
	ldr	r4,=trap_hook
	ldr	r4,[r4]
	cmp	r4,#0
1:	beq	1b
	/* else call (*trap_hook)(trap_t) */
	mov	r0,sp
	blx	r4

/* Restore the state from the trap_t */
rfe:	
	/* pop r8-r14 */
	add	sp,sp,#48
	pop	{r0-r6} /* r8 - r14 */
	mov	r8,r0
	mov	r9,r1
	mov	r10,r2
	mov	r11,r3
	mov	r12,r4
	mov	lr,r6	/* restore lr */

	sub	sp,sp,#60 /* move sp to point to r0 */
	pop	{r0-r7}

	/* move sp to point to epc and spsr saved by srs */
	add	sp,sp,#32

	/* Restore cpsr, sp and return */
	rfeia	sp!	/* done! */
END(__traps)

/* Dummy function just for link happy */
FUNC(raise)

	mov	r0, #0
	mov	pc, lr

END(raise)

FUNC(__aeabi_unwind_cpp_pr0)

	mov pc, lr

END(__aeabi_unwind_cpp_pr0)

/*
 * Setup the trap handler.
 */
FUNC(_exc_set_trap)

	ldr	r2,=trap_hook
	ldr	r1,[r2]
	str	r0,[r2]
	mov	r0,r1
	bx	lr

END(_exc_set_trap)

/*
 * Turn remap off and then jump to an given address
 */
FUNC(arm_jumpto)

	bx	r0

END(arm_jumpto)


/*  *********************************************************************
    *  armv7_exception_init(void)
    *  
    *  exception handler initialization. setup base address, exception mode
    *  stack pointer	
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */

FUNC(armv7_exception_init)
	/*  change the default hw 0xffff-0000 exception vector base to image
	    exception vector address*/
	ldr	r0, =startup		/* this address must align to 32 bytes */	
	mcr	p15,0,r0,c12,c0,0	/* Write R0 to VBAR */
	mrc	p15,0,r0,c1,c0,0
	bic	r0,r0,#CR_V		/* Clear V bits, using remapped vector base, in SCTLR */
	mcr	p15,0,r0,c1,c0,0

	/* setup stack for sys mode where exception handler is running */
	mrs	r0,cpsr			/* save the original mode */
	mov	r1,#PS_MM
	bic	r1,r1,r0
	mov 	r2,#PS_SYS
	orr	r1,r1,r2
	msr	cpsr,r1

	/* now we are in sys mode */
	ldr	r1,=exc_stack
	add	r1, #EXCEPTION_STACK_SIZE
	sub	r1,r1,#8
	mov	sp, r1
	bic	sp,sp,#7		/* 8-byte alignment for ABI compliance */

	/* switch back to original mode */
	msr	cpsr,r0

	/* setup the monitor mode vector for cfe ram */
#if defined(CFG_RAMAPP)
	ldr	r0, =mon_vector
	mcr	p15, 0, r0, c12, c0, 1
	isb
#endif	
	mov	pc,lr
END(armv7_exception_init)


/*  *********************************************************************
    *  _exc_restart()
    *  
    *  Restart the firmware at the boot address
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */

FUNC(_exc_restart)

	ldr	lr, =0xFFFF0000		/* ROM restart vector */
	mov	pc, lr

END(_exc_restart)

/*  *********************************************************************
    *  CFE_WARMSTART
    *
    *  Restart the command interpreter
    *
    *  Input parameters:
    *      A0 - command status
    *  	   nothing (GP has already been set up for us)
    *
    *  Return value:
    *  	   nothing
    ********************************************************************* */

FUNC(cfe_warmstart)

	mov	pc,lr

END(cfe_warmstart)

/*  *********************************************************************
    *  End
    ********************************************************************* */
