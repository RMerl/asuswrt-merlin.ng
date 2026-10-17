/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Board specific setup info
 *
 * (C) Copyright 2004, ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 */

#include <config.h>
#include <armcoremodule.h>

	/* Reset using CM control register */
.global reset_cpu
reset_cpu:
	mov	r0, #CM_BASE
	ldr	r1,[r0,#OS_CTRL]
	orr	r1,r1,#CMMASK_RESET
	str	r1,[r0,#OS_CTRL]

reset_failed:
	b	reset_failed

/* Set up the platform, once the cpu has been initialized */
.globl lowlevel_init
lowlevel_init:
	/* If U-Boot has been run after the ARM boot monitor
	 * then all the necessary actions have been done
	 * otherwise we are running from user flash mapped to 0x00000000
	 * --- DO NOT REMAP BEFORE THE CODE HAS BEEN RELOCATED --
	 * Changes to the (possibly soft) reset defaults of the processor
	 * itself should be performed in cpu/arm<>/start.S
	 * This function affects only the core module or board settings
	 */

#ifdef CONFIG_CM_INIT
	/* CM has an initialization register
	 * - bits in it are wired into test-chip pins to force
	 *   reset defaults
	 * - may need to change its contents for U-Boot
	 */

	/* set the desired CM specific value */
	mov	r2,#CMMASK_LOWVEC	/* Vectors at 0x00000000 for all */

#if !defined (CONFIG_CM920T) && !defined (CONFIG_CM920T_ETM) && \
     !defined (CONFIG_CM940T)

#ifdef	CONFIG_CM_MULTIPLE_SSRAM
	/* set simple mapping			*/
	and	r2,r2,#CMMASK_MAP_SIMPLE
#endif /* #ifdef CONFIG_CM_MULTIPLE_SSRAM	*/

#ifdef	CONFIG_CM_TCRAM
	/* disable TCRAM			*/
	and	r2,r2,#CMMASK_TCRAM_DISABLE
#endif /* #ifdef CONFIG_CM_TCRAM		*/

#if defined (CONFIG_CM926EJ_S) || defined (CONFIG_CM1026EJ_S) || \
     defined (CONFIG_CM1136JF_S)

	and	r2,r2,#CMMASK_LE

#endif /* cpu with little endian initialization */

	orr	r2,r2,#CMMASK_CMxx6_COMMON

#endif /* CMxx6 code */

	/* read CM_INIT		 */
	mov	r0, #CM_BASE
	ldr	r1, [r0, #OS_INIT]
	/* check against desired bit setting */
	and	r3,r1,r2
	cmp	r3,r2
	beq	init_reg_OK

	/* lock for change */
	mov	r3, #CMVAL_LOCK1
	add	r3,r3,#CMVAL_LOCK2
	str	r3, [r0, #OS_LOCK]
	/* set desired value */
	orr	r1,r1,r2
	/* write & relock CM_INIT */
	str	r1, [r0, #OS_INIT]
	mov	r1, #CMVAL_UNLOCK
	str	r1, [r0, #OS_LOCK]

	/* soft reset so new values used */
	b	reset_cpu

init_reg_OK:

#endif /* CONFIG_CM_INIT */

	mov	pc, lr

#ifdef	CONFIG_CM_SPD_DETECT
	/* Fast memory is available for the DRAM data
	 * - ensure it has been transferred, then summarize the data
	 *   into a CM register
	 */
.globl dram_query
dram_query:
	stmfd	r13!,{r4-r6,lr}
	/* set up SDRAM info					*/
	/* - based on example code from the CM User Guide */
	mov	r0, #CM_BASE

readspdbit:
	ldr	r1, [r0, #OS_SDRAM]	/* read the SDRAM register	*/
	and	r1, r1, #0x20		/* mask SPD bit (5)		*/
	cmp	r1, #0x20		/* test if set			*/
	bne	readspdbit

setupsdram:
	add	r0, r0, #OS_SPD		/* address the copy of the SDP data	*/
	ldrb	r1, [r0, #3]		/* number of row address lines		*/
	ldrb	r2, [r0, #4]		/* number of column address lines	*/
	ldrb	r3, [r0, #5]		/* number of banks			*/
	ldrb	r4, [r0, #31]		/* module bank density			*/
	mul	r5, r4, r3		/* size of SDRAM (MB divided by 4)	*/
	mov	r5, r5, ASL#2		/* size in MB				*/
	mov	r0, #CM_BASE		/* reload for later code		*/
	cmp	r5, #0x10		/* is it 16MB?				*/
	bne	not16
	mov	r6, #0x2		/* store size and CAS latency of 2	*/
	b	writesize

not16:
	cmp	r5, #0x20		/* is it  32MB? */
	bne	not32
	mov	r6, #0x6
	b	writesize

not32:
	cmp	r5, #0x40		/* is it  64MB? */
	bne	not64
	mov	r6, #0xa
	b	writesize

not64:
	cmp	r5, #0x80		/* is it 128MB? */
	bne	not128
	mov	r6, #0xe
	b	writesize

not128:
	/* if it is none of these sizes then it is either 256MB, or
	 * there is no SDRAM fitted so default to 256MB
	 */
	mov	r6, #0x12

writesize:
	mov	r1, r1, ASL#8		/* row addr lines from SDRAM reg */
	orr	r2, r1, r2, ASL#12	/* OR in column address lines	 */
	orr	r3, r2, r3, ASL#16	/* OR in number of banks	 */
	orr	r6, r6, r3		/* OR in size and CAS latency	 */
	str	r6, [r0, #OS_SDRAM]	/* store SDRAM parameters	 */

#endif /* #ifdef CONFIG_CM_SPD_DETECT */

	ldmfd	r13!,{r4-r6,pc}			/* back to caller */

#ifdef	CONFIG_CM_REMAP
	/* CM remap bit is operational
	 * - use it to map writeable memory at 0x00000000, in place of flash
	 */
.globl cm_remap
cm_remap:
	stmfd	r13!,{r4-r10,lr}

	mov	r0, #CM_BASE
	ldr	r1, [r0, #OS_CTRL]
	orr	r1, r1, #CMMASK_REMAP	/* set remap and led bits */
	str	r1, [r0, #OS_CTRL]

	/* Now 0x00000000 is writeable, replace the vectors	*/
	ldr	r0, =_start	/* r0 <- start of vectors	*/
	add	r2, r0, #64	/* r2 <- past vectors	*/
	sub	r1,r1,r1		/* destination 0x00000000	*/

copy_vec:
	ldmia	r0!, {r3-r10}		/* copy from source address [r0]	*/
	stmia	r1!, {r3-r10}		/* copy to	 target address [r1]	*/
	cmp	r0, r2			/* until source end address [r2]	*/
	ble	copy_vec

	ldmfd	r13!,{r4-r10,pc}	/* back to caller			*/

#endif /* #ifdef CONFIG_CM_REMAP */
