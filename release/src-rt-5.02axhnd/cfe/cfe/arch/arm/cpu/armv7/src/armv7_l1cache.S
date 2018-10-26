/*  
    <:copyright-BRCM:2012:proprietary:standard
    
       Copyright (c) 2012 Broadcom 
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
*/

    /*********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *
    *  ARMv7 cache function implementation. Some based on the ARM 
    *  bare-metal examples 
    *
    ********************************************************************* */
	
#include <arm.h>
#include <armmacros.h>
#include "cpu_config.h"
#include "bsp_config.h"
#include "cfe_iocb.h"
#include "bcm_map.h"	

/*  *********************************************************************
    *  armv7_branch_predict_enable
    *
    *  enable branch predition.
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *  	   nothing
    *	
    *  Registers used:
    *  	   r0	
    ********************************************************************* */	
FUNC(armv7_branch_predict_enable)
    mov   r0, #0
    mcr   p15, 0, r0, c7, c5, 6                /* Invalidate entire branch predictor array */
    mrc	  p15, 0, r0, c1, c0, 0 
    orr   r0, r0, #CR_Z                   // Enable Prediction
    mcr   p15, 0, r0, c1, c0, 0
    dsb

    mov pc, lr
END(armv7_branch_predict_enable)

/*  *********************************************************************
    *  armv7_branch_predict_disable
    *
    *  disable branch predition.
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *  	   nothing
    *	
    *  Registers used:
    *  	   r0	
    ********************************************************************* */	
FUNC(armv7_branch_predict_disable)
    mrc	  p15, 0, r0, c1, c0, 0 
    bic   r0, r0, #CR_Z                   // disable prediction
    mcr   p15, 0, r0, c1, c0, 0
    dsb

    mov pc, lr
END(armv7_branch_predict_disable)
	
/*  *********************************************************************
    *  armv7_l1cache_enable_i
    *
    *  l1 i-cache enable. I-cache must be invalid first
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *  	   nothing
    *	
    *  Registers used:
    *  	   r0	
    ********************************************************************* */
FUNC(armv7_l1cache_enable_i)
	
    mrc   p15, 0, r0, c1, c0, 0 // Read Control Register configuration data
    orr   r0, r0, #CR_I         // Enable I Cache
    orr   r0, r0, #CR_Z         // Enable Prediction
    mcr   p15, 0, r0, c1, c0, 0 // Write Control Register configuration data
    isb
	
    mov   pc, lr
  
END(armv7_l1cache_enable_i)

/*  *********************************************************************
    *  armv7_l1cache_enable_d
    *
    *  l1 d-cache enable. MMU must be initialized and enabled first.
    *	D-cache must invalidate first
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *  	   nothing
    *
    *  Registers used:
    *  	   r0
    ********************************************************************* */
FUNC(armv7_l1cache_enable_d)
	
    mrc   p15, 0, r0, c1, c0, 0 // Read Control Register configuration data
    orr   r0, r0, #CR_C         // Enable D Cache
    mcr   p15, 0, r0, c1, c0, 0 // Write Control Register configuration data
    isb
	
    mov   pc, lr
  
END(armv7_l1cache_enable_d)

/*  *********************************************************************
    *  armv7_l1cache_disable_i
    *
    *  l1 i-cache disable.
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *  	   nothing
    *
    *  Registers used:
    *  	   r0	
    ********************************************************************* */
FUNC(armv7_l1cache_disable_i)
	
    mrc   p15, 0, r0, c1, c0, 0 // Read Control Register configuration data
    bic   r0, r0, #CR_I         // Disable I Cache
    bic   r0, r0, #CR_Z         // Disable Prediction
    mcr   p15, 0, r0, c1, c0, 0 // Write Control Register configuration data
    isb
	
    mov   pc, lr
  
END(armv7_l1cache_disable_i)

/*  *********************************************************************
    *  armv7_l1cache_disable_d
    *
    *  l1 d-cache disable. 
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *  	   nothing
    *
    *  Registers used:
    *  	   r0	
    ********************************************************************* */
FUNC(armv7_l1cache_disable_d)
	
    mrc   p15, 0, r0, c1, c0, 0 // Read Control Register configuration data
    bic   r0, r0, #CR_C         // Disable D Cache
    mcr   p15, 0, r0, c1, c0, 0 // Write Control Register configuration data
    isb
	
    mov   pc, lr
  
END(armv7_l1cache_disable_d)
	

/*  *********************************************************************
    *  armv7_l1cache_inval_i
    *
    *  l1 invalid instruction cache
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *  	   nothing
    *
    *  Registers used:
    *  	   r0	
    ********************************************************************* */
FUNC(armv7_l1cache_inval_i)
    mov   r0, #0
    mcr   p15, 0, r0, c7, c5, 0 // Invalidate Instruction cache
    isb

    mov   pc, lr

END(armv7_l1cache_inval_i)

/*  *********************************************************************
    *  armv7_l1cache_inval_d
    *
    *  l1 invalid data cache
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *  	   nothing
    *
    *  Registers used:
    *  	   r0 - r7,r9,r10	
    ********************************************************************* */
FUNC(armv7_l1cache_inval_d)
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
	ldr	r4, =0x3ff
	ands	r4, r4, r1, lsr #3		@ find maximum number on the way size
	clz	r5, r4				@ find bit position of way size increment
	ldr	r7, =0x7fff
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
END(armv7_l1cache_inval_d)
	
/*  *********************************************************************
    *  armv7_l1cache_flush_d
    *
    *  l1 flush data cache (clean+invalid)
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *  	   nothing
    *
    *  Registers used:
    *  	   r0 - r7,r9,r10	
    ********************************************************************* */	
FUNC(armv7_l1cache_flush_d)
	mrc	p15, 1, r0, c0, c0, 1		@ read clidr
	ands	r3, r0, #0x7000000		@ extract loc from clidr
	mov	r3, r3, lsr #23			@ left align loc bit field
	beq	finished			@ if loc is 0, then no need to clean
	mov	r10, #0				@ start clean at cache level 0
loop1:
	add	r2, r10, r10, lsr #1		@ work out 3x current cache level
	mov	r1, r0, lsr r2			@ extract cache type bits from clidr
	and	r1, r1, #7			@ mask of the bits for current cache only
	cmp	r1, #2				@ see what cache we have at this level
	blt	skip				@ skip if no cache, or just i-cache
	mcr	p15, 2, r10, c0, c0, 0		@ select current cache level in cssr
	isb
						@ but we compile with armv5
	mrc	p15, 1, r1, c0, c0, 0		@ read the new csidr
	and	r2, r1, #7			@ extract the length of the cache lines
	add	r2, r2, #4			@ add 4 (line length offset)
	ldr	r4, =0x3ff
	ands	r4, r4, r1, lsr #3		@ find maximum number on the way size
	clz	r5, r4				@ find bit position of way size increment
	ldr	r7, =0x7fff
	ands	r7, r7, r1, lsr #13		@ extract max number of the index size
loop2:
	mov	r9, r4				@ create working copy of max way size
loop3:
	orr	r6, r10, r9, lsl r5		@ factor way and cache number into r6
	orr	r6, r6, r7, lsl r2		@ factor index number into r6
	mcr	p15, 0, r6, c7, c14, 2		@ clean & invalidate by set/way
	subs	r9, r9, #1			@ decrement the way
	bge	loop3
	subs	r7, r7, #1			@ decrement the index
	bge	loop2
skip:
	add	r10, r10, #2			@ increment cache number
	cmp	r3, r10
	bgt	loop1
finished:
	mov	r10, #0				@ swith back to cache level 0
	mcr	p15, 2, r10, c0, c0, 0		@ select current cache level in cssr
	isb
	mov	pc, lr
END(armv7_l1cache_flush_d)

/*  *********************************************************************
    *  armv7_l1cache_inval_range_d
    *
    *  l1 invalid data cache in a range
    *
    *  Input parameters:
    *      r1, r2: start and end address of the range
    *
    *  Return value:
    *  	   nothing
    *
    *  Registers used:
    *  	   r0 - r3
    ********************************************************************* */
FUNC(armv7_l1cache_inval_range_d)
	mrc	p15, 0, r0, c0, c0, 1		@ minimum d cache line size from CTR
	lsr	r0, r0, #16			@ cache line size encoding
	and	r0, r0, #0xf
	mov	r3, #4				@ size offset
	mov	r3, r3, lsl r0			@ actual cache line size

	sub	r0, r3, #1
	tst	r1, r0
	bic	r1, r1, r0
	mcrne	p15, 0, r1, c7, c14, 1		@ if start address is not aligned, clean & invalidate D / U line
						@ first in case the first line contains unflushed data
	tst	r2, r0
	bic	r2, r2, r0
	mcrne	p15, 0, r2, c7, c14, 1		@ if end address is not aligned, clean & invalidate D / U line first
1:
	mcr	p15, 0, r1, c7, c6, 1		@ data cache invalid by MVA
	add	r1, r1, r3
	cmp	r1, r2
	blo	1b
#	dsb
	isb
	mov	pc, lr
END(armv7_l1cache_inval_range_d)

/*  *********************************************************************
    *  armv7_l1cache_flush_range_d
    *
    *  l1 flush data cache in a range(clean+invalid)
    *
    *  Input parameters:
    *      r1, r2: start and end address of the range
    *
    *  Return value:
    *  	   nothing
    *
    *  Registers used:
    *  	   r0 - r3	
    ********************************************************************* */	
FUNC(armv7_l1cache_flush_range_d)
	mrc	p15, 0, r0, c0, c0, 1		@ minimum d cache line size from CTR
	lsr	r0, r0, #16			@ cache line size encoding
	and	r0, r0, #0xf
	mov	r3, #4				@ size offset
	mov	r3, r3, lsl r0			@ actual cache line size
	
	sub	r0, r3, #1
	bic	r1, r1, r0
1:
	mcr	p15, 0, r1, c7, c14, 1		@ data cache clean and invalid by MVA
	add	r1, r1, r3
	cmp	r1, r2
	blo	1b
#	dsb
	isb
	mov	pc, lr
END(armv7_l1cache_flush_range_d)
	

/*  *********************************************************************
    *  ARMV7_CACHEOPS
    *  
    *  Perform various cache operations on a armv7 cpu core. Must be called
    *  from relocated code with stack setup. Exception made for btrm only. 
    *  See below BTRM directive 
    *  
    *  Input parameters: 
    *  	   r0 - flag bits (CFE_CACHE_xxx)
    *  	   r1 - start address for range operation
    *      r2 - end address+1 for range operation
    *	
    *  Return value:
    *  	   nothing
    *  	   
    *  Registers used:
    *  	   r0 - r7,r9,r10, if BTRM - r11 
    *      
    ********************************************************************* */

FUNC(armv7_cacheops)

	mov	r12, lr		/* persevere link reg across call */

	/* save input parameters */
	mov	r4, r0
	mov	r5, r1
	mov	r6, r2
	
	/*
	 * With no flags, we flush L1D and invalid L1I
	 */
	cmp	r4, #0
	bne     1f
	mov	r4, #(CFE_CACHE_FLUSH_D | CFE_CACHE_INVAL_I)
1:


	/*
	 * Invalidate the I-Cache, so that addresses in the program
	 * region will miss and need to be filled from the data we 
	 * just flushed above.
	 */
	tst	r4, #CFE_CACHE_INVAL_I
	beq	2f
	bl	armv7_l1cache_inval_i	      
2:
	
	/*
	 * Invalidate d cache range
	 */
	tst	r4, #CFE_CACHE_INVAL_RANGE
	beq	2f

	mov	r1, r5
	mov	r2, r6
        bl	armv7_l1cache_inval_range_d	      

2:	
	/*
	 * Flush cache range
	 */
	tst	r4, #CFE_CACHE_FLUSH_RANGE
	beq	2f

	mov	r1, r5
	mov	r2, r6
        bl	armv7_l1cache_flush_range_d	      
		
2:
	/*
	 * Invalid the D-Cache, since the program we loaded is "data".
	 */
	tst	r4, #CFE_CACHE_INVAL_D
	beq	2f

	mov	r11,r4
        bl	armv7_l1cache_inval_d
	mov	r4,r11
2:	
	/*
	 * Flush the D-Cache, since the program we loaded is "data".
	 */
	tst	r4, #CFE_CACHE_FLUSH_D
	beq	2f

	mov	r11,r4
        bl	armv7_l1cache_flush_d
	mov	r4,r11

2:	
	mov	lr, r12		/* restore link */
	mov	pc, lr

END(armv7_cacheops)

#if defined(CFG_NONSEC_BOOT)	
#if defined(_BCM963138_)
FUNC(armv7_l2c_secure_init)

#define L2C_AUX_CONTROL_VAL (L2C_AUX_CONTROL_I_PREF_EN | L2C_AUX_CONTROL_NS_LOCK_EN | L2C_AUX_CONTROL_NS_INT_EN \
	| L2C_AUX_CONTROL_SHARED_OVERRIDE_EN | L2C_AUX_CONTROL_EARLY_BRESP_EN | L2C_AUX_CONTROL_WAY_SIZE_32KB | L2C_AUX_CONTROL_ASSOCIATIVITY_16WAY )
#define L2C_AUX_CONTROL_MASK ~(L2C_AUX_CONTROL_I_PREF_EN | L2C_AUX_CONTROL_NS_LOCK_EN | L2C_AUX_CONTROL_NS_INT_EN \
	| L2C_AUX_CONTROL_SHARED_OVERRIDE_EN | L2C_AUX_CONTROL_EARLY_BRESP_EN | L2C_AUX_CONTROL_WAY_SIZE_MASK | L2C_AUX_CONTROL_ASSOCIATIVITY_16WAY )
	
	ldr	r0, =L2C_BASE
	ldr	r1, =L2C_AUX_CONTROL_MASK
	ldr	r2, [r0, #L2C_AUX_CONTROL_OFFSET]
	and	r2, r2, r1
	ldr	r1, =L2C_AUX_CONTROL_VAL	
	orr	r1, r1, r2
	str	r1, [r0, #L2C_AUX_CONTROL_OFFSET]

	// invalidate by way all cache entries
	ldr	r1, =0xffff
	str	r1, [r0, #L2C_INVALID_WAY_OFFSET]
enable_L2C_310_inv_way_loop:
	ldr	r2, [r0, #L2C_INVALID_WAY_OFFSET]
	tst	r2, r1
	bne	enable_L2C_310_inv_way_loop

	// clear spurious interrupts
	ldr	r1, =0x000001ff
	str	r1, [r0, #L2C_INTR_CLEAR_OFFSET]

	// enable the controller.
	mov	r1, #1
	str	r1, [r0, #L2C_CONTROL_OFFSET]
	dsb

	mov	pc, lr
END(armv7_l2c_secure_init)

FUNC(armv7_scu_secure_init)
	/* set scu non-secure access */
	ldr	r0, =SCU_BASE
	ldr	r1, =0xfff
	str	r1, [r0, #SCU_SNSAC_OFFSET]
	mov	pc, lr
END(armv7_scu_secure_init)
#endif
#endif
