/*  *********************************************************************
    *
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
    ********************************************************************* */
	
#include <arm.h>
#include <armmacros.h>
#include "cpu_config.h"
#include "bsp_config.h"
#include "bcm_map.h"
#include "mmu_map.h"

#if defined(CFG_NONSEC_BOOT)
/*  *********************************************************************
    *  armv7_gic_secure_init(void)
    *
    *  .init gic for nonsecure os
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *      nothing
    *
    *  Registers used:
    *     r0, r1, r2
    ********************************************************************* */
FUNC(armv7_gic_secure_init)
	/* setup the GIC for non secure world. group 0 secure state, group 1 non secure state */
	/* enable interrupt for both groups */
	ldr	r0, =GICD_BASE
	ldr	r1, =0x3
	str	r1, [r0, #GICD_CTLR_OFFSET]
	
	/* assign all the interrupt to group 1 */
	ldr	r2, [r0, #GICD_TYPER_OFFSET]
	and     r2, r2, #0x1f
	add	r2, r2, #1
	ldr	r1, =0xffffffff
	add	r0, r0, #GICD_IGROUPR0_OFFSET	
setgroup:
	str	r1, [r0]
	add	r0, #4
	subs	r2, r2, #1
	bne	setgroup

	/* enable both group interrupt in the cpu interface */
	ldr	r0, =GICC_BASE
	ldr	r1, [r0, #GICC_CTLR_OFFSET]
	orr	r1, #0x3
	str	r1, [r0]

	/* set cpu priority mask view for nonsecure */
	mov	r1, #0x80
	str	r1, [r0, #GICC_PMR_OFFSET]

	mov	pc, lr
END(armv7_gic_secure_init)

/*  *********************************************************************
    *  armv7_change_to_nonsecure(void)
    *
    *  drop to NonSecure mode and jump to the entry point
    *
    *  Input parameters:
    *      r0, r1, r2, r3: argment pass to the program
    *      r4: program entry point
    *
    *  Return value:
    *      nothing
    *
    *  Registers used:
    *     r5
    ********************************************************************* */
FUNC(armv7_change_to_nonsecure)
	.arch_extension sec
	/* set non-secure ACR. Allow SMP, L2ERR, CP10 and CP11 and Enable Neon/VFP bit for non-secure mode */
	ldr	r5, =0x60c00
	mcr	p15, 0, r5, c1, c1, 2
#if defined(_BCM963138_)
	/* set FW bit in ACTLR, SMP is set by linux */
	mrc	p15, 0, r5, c1, c0, 1
	orr	r5, r5, #0x1   
	mcr	p15, 0, r5, c1, c0, 1
#endif
	smc	#0
END(armv7_change_to_nonsecure)
#endif

/*  *********************************************************************
    *  armv7_cpuinit(void)
    *  
    *  perform fundemetnal CPU initialization 
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */

FUNC(armv7_cpuinit)
	
	mov	ip, lr		/* persevere link reg across call */

#if defined(_BCM96846_)
	mrc	p15, 0, r0, c1, c0, 0       // Read CP15 System Control register
	bic	r0, r0, #CR_I               // Clear I bit 12 to disable I Cache
	bic	r0, r0, #CR_C               // Clear C bit  2 to disable D Cache
	bic	r0, r0, #CR_M               // Clear M bit  0 to disable MMU
	bic	r0, r0, #CR_Z               // Clear Z bit 11 to disable branch prediction
	bic	r0, r0, #CR_A               // Clear A bit: disable strict alignment
	orr	r0, r0, #CR_V               // Set V bit: high vectors (0xffff0000)
	mcr	p15, 0, r0, c1, c0, 0       // Write value back to CP15 System Control register
	isb

	/* Enable SMP before cache enabled, for Cortex-A7 series core */
	mrc	p15,0,r1,c1,c0,1
	orr	r1,r1,#(1<<6)
	mcr	p15,0,r1,c1,c0,1
 
	mov	r0,#0
	mcr	p15,0,r0,c8,c7,0		/* Invalidate TLB */
	mcr	p15,0,r0,c7,c5,6        /* Invalidate entire branch predictor array */
	isb
#else
	mov	r0,#0
	mcr	p15,0,r0,c8,c7,0		/* Invalidate TLB */
	mcr	p15,0,r0,c7,c5,0		/* Invalidate icache */
	mcr	 p15,0,r0,c7,c5,6                /* Invalidate entire branch predictor array */
	
	/* Initialize system control register */
	mrc	p15,0,r0,c1,c0,0
	bic	r0,r0,#(CR_C|CR_A|CR_M)		/* Clear C, A, M bits */
	orr	r0,r0,#CR_V			/* Set V bit: high vectors (0xffff0000) */
	orr	r0,r0,#CR_A			/* Set A bit: enable strict alignment */
	orr	r0,r0,#CR_Z			/* Set Z bit: enable program flow prediction */
	orr	r0,r0,#CR_I			/* Set I bit: enable instruction cache */
	mcr	p15,0,r0,c1,c0,0

	isb

#if defined(_BCM947189_) || defined(_BCM963148_)
	/* Enable SMP before cache enabled, for Cortex-A7 series core */
	mrc	p15,0,r1,c1,c0,1
	orr	r1,r1,#(1<<6)
	mcr	p15,0,r1,c1,c0,1
#endif
#endif
	mov	lr, ip		/* restore link */
	mov	pc, lr
	
END(armv7_cpuinit)

/*  *********************************************************************
    *  armv7_enable_branch_predictor
    *
    *  Enable branch predictor
    *
    *  Input parameters:
    *      nothing
    *
    *  Return value:
    *      nothing
    *
    *  Registers used:
    *      r0
    ********************************************************************* */
FUNC(armv7_enable_branch_predictor)

	mrc	p15, 0, r0, c1, c0, 0     // Read CP15 System Control register
	orr	r0, r0, #CR_Z             // Set Z bit: enable program flow prediction
	orr	r0, r0, #CR_A             // Set A bit: enable strict alignment
	mcr	p15, 0, r0, c1, c0, 0     // Write CP15 System Control register
	isb

	mov	pc, lr

END(armv7_enable_branch_predictor)


/*  *********************************************************************
    *  armv7_enable_mmu
    *  
    *  Enable MMU 
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    *
    *  Registers used:
    *  	   r0	
    ********************************************************************* */	
FUNC(armv7_enable_mmu)

	mrc	p15, 0, r0, c1, c0, 0     // Read CP15 System Control register
#if defined(_BCM96846_)
	bic	r0, r0, #CR_I             // Clear I bit 12 to disable I Cache
	bic	r0, r0, #CR_D             // Clear C bit  2 to disable D Cache
	bic	r0, r0, #CR_A             // Clear A bit  1 to disable strict alignment fault checking
#endif
	orr	r0, r0, #CR_M             // Set M bit 0 to enable MMU
	mcr	p15, 0, r0, c1, c0, 0     // Write CP15 System Control register
	isb

	mov	pc, lr
	
END(armv7_enable_mmu)

/*  *********************************************************************
    *  armv7_disable_mmu
    *  
    *  Disable MMU 
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    *
    *  Registers used:
    *  	   r0	
    ********************************************************************* */	
FUNC(armv7_disable_mmu)

	mrc	p15, 0, r0, c1, c0, 0 // Read Control Register configuration data
	bic	r0, r0, #CR_M 
	mcr	p15, 0, r0, c1, c0, 0 // Write Control Register configuration data
	isb    


	mov	pc, lr
	
END(armv7_disable_mmu)

/*  *********************************************************************
    *  armv7_l1cache_on(void)
    *  
    *  C interface for turnning on L1 cache.
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    *
    *  Registers used:
    *  	   all reserved	
    ********************************************************************* */	
FUNC(armv7_l1cache_on)
	stmfd	sp!, {r0-r11, lr}

	/* enabled i cache */
	bl	armv7_l1cache_inval_i 
	bl	armv7_l1cache_enable_i

	/* enanble d cache */
	bl	armv7_l1cache_inval_d

        /* mmu page tables are defined 
	in the appropriate board implementation */
	bl	armv7_mmu_map

	bl	armv7_enable_mmu
	bl	armv7_l1cache_enable_d
	
	ldmfd	sp!, {r0-r11, lr}
	mov	pc, lr
END(armv7_l1cache_on)

FUNC(armv7_l1cache_off)
	stmfd	sp!, {r0-r11, lr}

	mov	r0, #0
	bl	_cfe_flushcache

	/*
	 * Disable the D-Cache, MMU and I-Cache bit
	 */
	bl	armv7_l1cache_disable_d
	bl	armv7_disable_mmu
	bl	armv7_l1cache_disable_i
	

	ldmfd	sp!, {r0-r11, lr}
	mov	pc, lr
END(armv7_l1cache_off)


/*  *********************************************************************
    *  armv7_enable_tick(void)
    *  
    *  Enble arm cycle count through PMNC
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    *
    *  Registers used:
    *  	  r1, r2
    ********************************************************************* */
FUNC(armv7_enable_tick)
	mrc	p15,0,r1,c9,c12,0
	ldr	r2,=5           /* enable and reset PMNC */
	orr	r1,r1,r2
	mcr	p15,0,r1,c9,c12,0	
	ldr	r1,=0x80000000
	mcr	p15,0,r1,c9,c12,1

	mov	pc, lr
END(armv7_enable_tick)

/*  *********************************************************************
    *  getticks(void)
    *  
    *  get cycle counter through PMNC
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    *
    *  Registers used:
    *  	  r0
    ********************************************************************* */
FUNC(_getticks)
	mrc	p15,0,r0,c9,c13,0
	mov	pc, lr
END(_getticks)