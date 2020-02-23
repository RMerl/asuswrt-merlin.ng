/*  
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
*/
    /* *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *
    *  ARMv8 cache function implementation. Some based on the ARM 
    *  bare-metal examples 
    *
    *********************************************************************  */



#include <armv8.h>
#include <armmacros.h>
#include "cpu_config.h"
#include "bsp_config.h"
#include "cfe_iocb.h"

/*  *********************************************************************
    *  armv8_l1cache_enable_i
    *
    *  l1 i-cache enable. I-cache must be invalid first
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *	   nothing
    *
    *  Registers used:
    *	   x0
    ********************************************************************* */
FUNC(armv8_l1cache_enable_i)
	GET_EXCEPTION_LEVEL
	cmp	x0, EX_LEVEL_3
	beq	el3_l1cache_enable_i

	mrs	x0, SCTLR_EL2
	orr	x0, x0, #SCTLR_ELx_I
	msr	SCTLR_EL2, x0
	isb
	ret
        
el3_l1cache_enable_i:
	mrs	x0, SCTLR_EL3
	orr	x0, x0, #SCTLR_ELx_I
	msr	SCTLR_EL3, x0

	isb

	ret
END(armv8_l1cache_enable_i)

/*  *********************************************************************
    *  armv8_l1cache_disable_i
    *
    *  l1 i-cache disable.
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *	   nothing
    *
    *  Registers used:
    *	   r0
    ********************************************************************* */
FUNC(armv8_l1cache_disable_i)
	GET_EXCEPTION_LEVEL
	cmp	x0, EX_LEVEL_3
	beq	el3_l1cache_disable_i

	mrs	x0, SCTLR_EL2
	bic	x0, x0, #SCTLR_ELx_I
	msr	SCTLR_EL2, x0
	isb
	ret

el3_l1cache_disable_i:
	mrs	x0, SCTLR_EL3
	bic	x0, x0, #SCTLR_ELx_I
	msr	SCTLR_EL3, x0

	isb

	ret
END(armv8_l1cache_disable_i)


/*  *********************************************************************
    *  armv8_l1cache_inval_i
    *
    *  l1 invalid instruction cache
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *	   nothing
    *
    *  Registers used:
    *	   r0
    ********************************************************************* */
FUNC(armv8_l1cache_inval_i)
	ic	ialluis			/* Invalidate icache */
	isb

	ret
END(armv8_l1cache_inval_i)

/*  *********************************************************************
    *  armv8_l1cache_enable_d
    *
    *  l1 d-cache enable. MMU must be initialized and enabled first.
    *	D-cache must invalidate first
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *   nothing
    *
    *  Registers used:
    *   x0
    ********************************************************************* */
FUNC(armv8_l1cache_enable_d)
	GET_EXCEPTION_LEVEL
	cmp	x0, EX_LEVEL_3
	beq	el3_l1cache_enable_d

	mrs	x1, SCTLR_EL2
	orr	x1, x1, #SCTLR_ELx_C
	msr	SCTLR_EL2, x1
	isb
	ret

el3_l1cache_enable_d:
	mrs	x1, SCTLR_EL3
	orr	x1, x1, #SCTLR_ELx_C
	msr	SCTLR_EL3, x1
	isb

	ret
END(armv8_l1cache_enable_d)

/*  *********************************************************************
    *  armv8_l1cache_disable_d
    *
    *  l1 d-cache disable.
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *   nothing
    *
    *  Registers used:
    *   x0
    ********************************************************************* */
FUNC(armv8_l1cache_disable_d)
	GET_EXCEPTION_LEVEL
	cmp	x0, EX_LEVEL_3
	beq	el3_l1cache_disable_d

	mrs	x1, SCTLR_EL2
	bic	x1, x1, #SCTLR_ELx_C
	msr	SCTLR_EL2, x1
	isb
	ret

el3_l1cache_disable_d:
        
	mrs	x1, SCTLR_EL3
	bic	x1, x1, #SCTLR_ELx_C
	msr	SCTLR_EL3, x1
	isb

	ret
END(armv8_l1cache_disable_d)


/*  *********************************************************************
    *  armv8_l1cache_maintain
    *
    *  l1 flush data cache maintain function
    *
    *  Input parameters:
    *      w0 0: clean
    *         1: invalidate
    *         2: clean+invalid(flush)
    *  Return value:
    *	   nothing
    *
    *  Registers used:
    *	   w0 - w10
    ********************************************************************* */
FUNC(armv8_l1cache_maintenace)

	/* don't invalidate until all prior accesses have been observed */
	dmb	sy

	mov	w10, w0
	mrs	x0, CLIDR_EL1
	ubfx	w2, w0, #24, #3
	cbz	w2, invalidateUDCaches_end

	mov	w1, #0			// w1 = level iterator

flush_level:
	add	w3, w1, w1, lsl #1	// w3 = w1 * 3 (right-shift for cache type)
	lsr	w3, w0, w3		// w3 = w0 >> w3
	ubfx	w3, w3, #0, #3		// w3 = cache type of this level
	cmp	w3, #2			// No cache at this level?
	b.lt	next_level

	lsl	w4, w1, #1
	msr	CSSELR_EL1, x4		// Select current cache level in CSSELR
	isb				// ISB required to reflect new CSSELR
	mrs	x4, CCSIDR_EL1		// x4 = CCSIDR

	ubfx	w3, w4, #0, #3
	add	w3, w3, #4		 // w3 = log2(line size) + 4
	ubfx	w5, w4, #13, #15
	ubfx	w4, w4, #3, #10		// w4 = Way number
	clz	w6, w4			// w6 = 32 - log2(number of ways)

flush_set:
	mov	w8, w4			// w8 = Way number
flush_way:
	lsl	w7, w1, #1		// Fill level field
	lsl	w9, w5, w3
	orr	w7, w7, w9		// Fill index field
	lsl	w9, w8, w6
	orr	w7, w7, w9		// Fill way field
	cmp	w10, #0
	bne	1f
	dc	csw, x7			// Clean by set/way
	b	2f
1:	cmp	w10, #1
	bne	1f
	dc	isw, x7			// Invalidate by set/way
	b	2f
1:	dc	cisw, x7		// Clean and Invalidate by set/way
2:	subs	w8, w8, #1		// Decrement way
	b.ge	flush_way
	subs	w5, w5, #1		// Decrement set
	b.ge	flush_set

next_level:
	add	w1, w1, #1		// Next level
	cmp	w2, w1
	b.gt	flush_level

invalidateUDCaches_end:
	mov	x4, #0			// switch back to cache level 0
	msr	CSSELR_EL1, x4
	
	dsb	sy
	isb
	ret

END(armv8_l1cache_maintenace)

/*  *********************************************************************
    *  armv8_l1cache_clean_d
    *
    *  l1 clean data cache
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *	   nothing
    *
    *  Registers used:
    *	   w0 - w10, x14
    ********************************************************************* */
FUNC(armv8_l1cache_clean_d)
	mov	x14, lr

	mov	w0, #0
	CALLINIT(=armv8_l1cache_maintenace)

	mov	lr, x14
	ret
END(armv8_l1cache_clean_d)

/*  *********************************************************************
    *  armv8_l1cache_invalid_d
    *
    *  l1 invalid data cache
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *	   nothing
    *
    *  Registers used:
    *	   w0 - w10, x14
    ********************************************************************* */
FUNC(armv8_l1cache_inval_d)
	mov	x14, lr

	mov	w0, #1
	CALLINIT(=armv8_l1cache_maintenace)

	mov	lr, x14
	ret
END(armv8_l1cache_inval_d)

/*  *********************************************************************
    *  armv8_l1cache_flush_d
    *
    *  l1 flush data cache (clean + invalidate)
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *	   nothing
    *
    *  Registers used:
    *	   w0 - w10, x14
    ********************************************************************* */
FUNC(armv8_l1cache_flush_d)
	mov	x14, lr

	mov	w0, #2
	CALLINIT(=armv8_l1cache_maintenace)

	mov	lr, x14
	ret
END(armv8_l1cache_flush_d)

/*  *********************************************************************
    *  armv8_l1cache_inval_range_d
    *
    *  l1 invalid data cache in a range
    *
    *  Input parameters:
    *      x1, x2: start and end address of the range
    *
    *  Return value:
    *	   nothing
    *
    *  Registers used:
    *     x1 - x4
    ********************************************************************* */
FUNC(armv8_l1cache_inval_range_d)
	mrs	x4, ctr_el0
	ubfm	x4, x4, #16, #19	/* get system minimum d cache line size */
	mov	x3, #4
	lsl	x3, x3, x4		
	sub	x4, x3, #1

	tst	x2, x4			/*  perform clean+invalid if start and end cache line not aligned */
	bic	x2, x2, x4
	b.eq	1f
	dc	civac, x2
1:	tst	x1, x4		
	bic	x1, x1, x4
	b.eq	2f
	dc	civac, x1
	b	3f
2:	dc	ivac, x1
3:	add	x1, x1, x3
	cmp	x1, x2
	b.lo	2b
	dsb	sy
	isb

	ret
END(armv8_l1cache_inval_range_d)

/*  *********************************************************************
    *  armv8_l1cache_flush_range_d
    *
    *  l1 flush data cache in a range(clean+invalid)
    *
    *  Input parameters:
    *      x1, x2: start and end address of the range
    *
    *  Return value:
    *   nothing
    *
    *  Registers used:
    *     x1 - x4
    ********************************************************************* */
FUNC(armv8_l1cache_flush_range_d)
	mrs	x4, ctr_el0
	ubfm	x4, x4, #16, #19	/* get system minimum d cache line size */
	mov	x3, #4
	lsl	x3, x3, x4
	sub	x4, x3, #1

	bic	x1, x1, x4
1:	dc	civac, x1
	add	x1, x1, x3
	cmp	x1, x2
	b.lo	1b
	dsb	sy
	isb

	ret
END(armv8_l1cache_flush_range_d)


/*  *********************************************************************
    *  ARMV8_CACHEOPS
    *
    *  Perform various cache operations on a armv8 cpu core. Must be called
    *  from relocated code with stack setup.
    *
    *  Input parameters:
    *	   w0 - flag bits (CFE_CACHE_xxx)
    *	   x1 - start address for range operation
    *      x2 - end address+1 for range operation
    *
    *  Return value:
    *	   nothing
    *
    *  Registers used:
    *	   x0 - x11, x14
    ********************************************************************* */

FUNC(armv8_cacheops)

	mov	x14, lr		/* persevere link reg across call */

	/* save input parameters */
	mov	w4, w0
	mov	x5, x1
	mov	x6, x2

	/*
	 * With no flags, we flush L1D and invalid L1I
	 */
	cmp	w4, #0
	bne     1f
	mov	w4, #(CFE_CACHE_FLUSH_D | CFE_CACHE_INVAL_I)
1:

	/*
	 * Invalidate the I-Cache, so that addresses in the program
	 * region will miss and need to be filled from the data we
	 * just flushed above.
	 */
	tst	w4, #CFE_CACHE_INVAL_I
	beq	2f
	bl	armv8_l1cache_inval_i
2:

	/*
	 * Invalidate d cache range
	 */
	tst	w4, #CFE_CACHE_INVAL_RANGE
	beq	2f

	mov	w11, w4
	mov	x1, x5
	mov	x2, x6
	bl	armv8_l1cache_inval_range_d
	mov	w4, w11

2:
	/*
	 * Flush cache range
	 */
	tst	w4, #CFE_CACHE_FLUSH_RANGE
	beq	2f

	mov	w11, w4
	mov	x1, x5
	mov	x2, x6
	bl	armv8_l1cache_flush_range_d
	mov	w4, w11

2:
	/*
	 * Invalid the D-Cache, since the program we loaded is "data".
	 */
	tst	w4, #CFE_CACHE_INVAL_D
	beq	2f

	mov	w11, w4
	mov	w0, #1
	bl	armv8_l1cache_maintenace
	mov	w4, w11
2:

	/*
	 * Flush the D-Cache, since the program we loaded is "data".
	 */
	tst	w4, #CFE_CACHE_FLUSH_D
	beq	2f
	mov	w0, #2
	bl	armv8_l1cache_maintenace

2:
	mov	lr, x14		/* restore link */
	ret

END(armv8_cacheops)
