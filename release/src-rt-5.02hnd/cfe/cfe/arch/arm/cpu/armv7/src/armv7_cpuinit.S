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
#include "bcm_common.h"


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
	
	mov	r0,#0
	mcr	p15,0,r0,c8,c7,0		/* Invalidate TLB */
	mcr	p15,0,r0,c7,c5,0		/* Invalidate icache */
	mcr     p15,0,r0,c7,c5,6                /* Invalidate entire branch predictor array */
	
	/* Initialize system control register */
	mrc	p15,0,r0,c1,c0,0
	bic	r0,r0,#(CR_C|CR_A|CR_M)		/* Clear C, A, M bits */
	orr	r0,r0,#CR_V			/* Set V bit: high vectors (0xffff0000) */
	orr	r0,r0,#CR_A			/* Set A bit: enable strict alignment */
	orr	r0,r0,#CR_Z			/* Set Z bit: enable program flow prediction */
	orr	r0,r0,#CR_I			/* Set I bit: enable instruction cache */
	mcr	p15,0,r0,c1,c0,0

	isb

#if defined(_BCM947189_)
	/* Enable SMP before cache enabled, for Cortex-A7 series core */
	mrc	p15,0,r1,c1,c0,1
	orr	r1,r1,#0x40
	mcr	p15,0,r1,c1,c0,1
#endif

	mov	lr, ip		/* restore link */
	mov	pc, lr
	
END(armv7_cpuinit)

/*  *********************************************************************
    *  armv7_mmuinit(uint* addr)
    *  
    *  MMU initialization 
    *  
    *  Input parameters: 
    *  	   r0 - physical address of the mmu page table, must be 16KB aligned
    *  	   
    *  Return value:
    *  	   nothing
    *
    *  Registers used:
    *  	   r0 - r5
    ********************************************************************* */

FUNC(armv7_mmuinit)
	
	/* clear mmu table first, total 16K mmu table */
	mov	r1, r0
	ldr	r3, =0x4000
	add	r3, r0, r3
	mov	r2, #0
clearloop:
	str	r2, [r1]
	add     r1, #4
	cmp	r1, r3
	blt	clearloop
	
	mov	r1, #0
	/* 0x1C0E, normal memory cached */
	/* setup maximum 1GB for cfe rom alias test.For cfe ram, set the mmu memory size based on mem detection result */
	/* DDR memory 0x0000-0000 to 0x4000-0000 */
#if defined(CFG_RAMAPP)	&& !defined(IKOS_NO_DDRINIT)
	ldr	r3, =(SECTION_OUTER_WBWA_INNER_WBWA|SECTION_AP|DESC_DOMAIN(0))
	/* read ddr memory size in MEMC */
#if !defined(_BCM947189_)
	ldr     r2, =MEMC_BASE 
	ldr     r2, [r2, #MEMC_GLB_GCFG]
	and     r2, #MEMC_GLB_GCFG_DRAM_SIZE1_MASK
#else
	/* For 47189: Hardcode the memory size for now (128MB) 0x0000-0000 to 0x0800-0000*/
        mov     r2, #7
#endif
	mov     r5, #1
	lsl     r5, r5, r2
#else
	/* cfe rom maps DDR as uncached because we need to alias test when init the ddr */ 
	ldr	r3, =(SECTION_OUTER_NC_INNER_NC|SECTION_AP|DESC_DOMAIN(0))
	ldr     r5, =1024
#endif	
mmuloop:	
	mov	r2, r1, lsl #20
	orr	r2, r2, r3
	str	r2, [r0, r1, lsl #2]
	add	r1, r1, #1
	cmp	r1, r5
	blt	mmuloop

#if defined(CFG_RAMAPP)	
	/* 0x1C0E, map normal memory uncached */
	/* virtual address 0x4000-0000 to 0x8000-0000 -> 0x0000-0000 to 0x4000-0000 physcial uncached*/
	/* For 47189, this is 0x4000-0000 to 0x4800-0000 -> 0x0000-0000 to 0x0800-0000 */
	ldr     r1, =0x400
	mov     r2, #0
	ldr	r3, =(SECTION_OUTER_NC_INNER_NC|SECTION_AP|DESC_DOMAIN(0))
loop2:
	mov	r4, r2, lsl #20
	orr	r4, r4, r3
	str	r4, [r0, r1, lsl #2]
	add	r1, r1, #1
	add     r2, r2, #1
	cmp	r2, r5
	blt	loop2
#endif

#if !defined(_BCM947189_)
	/* entry for APM at 0x801x-xxxx */
	ldr	r1, =0x801
	mov	r2, r1, lsl #20
	ldr	r3, =(SECTION_OUTER_NSD_INNER_NSD|SECTION_AP|DESC_DOMAIN(0))
	orr	r2, r2, r3
	str	r2, [r0, r1, lsl #2]
	
	/* entry for RDP at 0x802x-xxxx */
	ldr	r1, =0x802
	mov	r2, r1, lsl #20
	ldr	r3, =(SECTION_OUTER_NSD_INNER_NSD|SECTION_AP|DESC_DOMAIN(0))
	orr	r2, r2, r3
	str	r2, [r0, r1, lsl #2]

	/* entry for PMC at 0x804x-xxxx */
	ldr	r1, =0x804
	mov	r2, r1, lsl #20
	ldr	r3, =(SECTION_OUTER_NSD_INNER_NSD|SECTION_AP|DESC_DOMAIN(0))
	orr	r2, r2, r3
	str	r2, [r0, r1, lsl #2]

	/* entry for onchip memory at 0x807x-xxxx */
	ldr	r1, =0x807
	mov	r2, r1, lsl #20
	ldr	r3, =(SECTION_OUTER_WBWA_INNER_WBWA|SECTION_AP|DESC_DOMAIN(0)) 
	orr	r2, r2, r3
	str	r2, [r0, r1, lsl #2]

	/* entry for MEMC, ARM UART at 0x800x-xxxx */
	mov	r1, #0x800
	mov	r2, r1, lsl #20
	ldr	r3, =(SECTION_OUTER_NSD_INNER_NSD|SECTION_AP|DESC_DOMAIN(0))
	orr	r2, r2, r3
	str	r2, [r0, r1, lsl #2]

	/* entry for PERIPHERAL at 0xfffx-xxxx */
	ldr	r1, =0xfff
	mov	r2, r1, lsl #20
	ldr	r3, =(SECTION_OUTER_NSD_INNER_NSD|SECTION_AP|DESC_DOMAIN(0))
	orr	r2, r2, r3
	str	r2, [r0, r1, lsl #2]

	/* entry for the direct mapped SPI flash. Set to cache memory to improve the performance for sim. */
	ldr	r1, =0xffd
	mov	r2, r1, lsl #20
	ldr	r3, =(SECTION_OUTER_NC_INNER_NC|SECTION_AP|DESC_DOMAIN(0))
	orr	r2, r2, r3
	str	r2, [r0, r1, lsl #2]

	/* entry for the direct mapped NAND flash. Set to cache memory to improve the performance for sim. */
	ldr	r1, =0xffe
	mov	r2, r1, lsl #20
	ldr	r3, =(SECTION_OUTER_NC_INNER_NC|SECTION_AP|DESC_DOMAIN(0))
	orr	r2, r2, r3
	str	r2, [r0, r1, lsl #2]
#else
	/* 47189 memory mapped registers 0x18xx-xxxx */
	mov	r5, #0x10
	ldr	r1, =0x180
	mov	r2, r1, lsl #20
	ldr	r3, =(SECTION_OUTER_NSD_INNER_NSD|SECTION_AP|DESC_DOMAIN(0))
loop3:
	orr	r2, r2, r3
	str	r2, [r0, r1, lsl #2]
	add	r1, r1, #1
	mov	r2, r1, lsl #20
	subs	r5, #1
	bne	loop3

	/* 47189 Serial Flash 0x1cxx-xxxx */
	mov     r5, #0x10
	ldr	r1, =0x1c0
	mov     r2, r1, lsl #20
	ldr	r3, =(SECTION_OUTER_NC_INNER_NC|SECTION_AP|DESC_DOMAIN(0))
loop4:
	orr	r2, r2, r3
	str	r2, [r0, r1, lsl #2]
	add	r1, r1, #1
	mov     r2, r1, lsl #20
	subs	r5, #1
	bne	loop4
#endif

	mov	r1, #0
	mcr	p15, 0, r1, c8, c7, 0	/* Invalidate TLB */
	
	// Set the access control to all-supervisor 
	ldr	r1, =0xffffffff
	mcr	p15, 0, r1, c3, c0, 0 // Write Domain Access Control Register
    
	// Set Translation Table base address. r0 must be preserved
	mcr	p15, 0, r0, c2, c0, 0 // Write Translation Table Base Register 0
	isb
	dsb
	
	mov	pc, lr	
END(armv7_mmuinit)


/*  *********************************************************************
    *  armv7_btrm_mmuinit(uint* addr)
    *  
    *  MMU initialization 
    *  
    *  Input parameters: 
    *  	   r0 - physical address of the mmu page table, must be 16KB aligned
    *  	   
    *  Return value:
    *  	   nothing
    *
    *  Registers used:
    *  	   r0 - r3	
    ********************************************************************* */

FUNC(armv7_btrm_mmuinit)
        mov     r1, #0
        /* 0x1C0E, normal memory */
        ldr     r3, =(SECTION_OUTER_WBWA_INNER_WBWA|SECTION_AP|DESC_DOMAIN(0)) 
mmuloop1:
        mov     r2, r1, lsl #20         /* r2 contains which 1 MB chunk we are dealing with (top 3 nibbles (0xfff00000) ) */
        orr     r2, r2, r3
        str     r2, [r0, r1, lsl #2]
        add     r1, r1, #1
        cmp     r1, #8
        blt     mmuloop1

        /* 0x2C02, nonsharable device */
        ldr     r3, =(SECTION_OUTER_NSD_INNER_NSD|SECTION_AP|DESC_DOMAIN(0)) 
mmuloop2:
        mov     r2, r1, lsl #20         /* r2 contains which 1 MB chunk we are dealing with (top 3 nibbles (0xfff00000) ) */
        orr     r2, r2, r3
        str     r2, [r0, r1, lsl #2]
        add     r1, r1, #1
        cmp     r1, #4096
        blt     mmuloop2

        /* Set the mmu entry for internal vdsl lmem to be 0x1C0E, normal memory */
        ldr     r3, =(SECTION_OUTER_WBWA_INNER_WBWA|SECTION_AP|DESC_DOMAIN(0)) 
        ldr     r2, =0x80700000         /* top three nibbles of beginning of internal vdsl lmem */
        orr     r2, r2, r3
        ldr     r1, =0x807              /* mmu table offset: ie table[0x807]  */
        str     r2, [r0, r1, lsl #2]

        // Set the access control to all-supervisor
        ldr     r1, =0xffffffff
        mcr     p15, 0, r1, c3, c0, 0 // Write Domain Access Control Register

        // Set Translation Table base address. r0 must be preserved
        mcr      p15, 0, r0, c2, c0, 0 // Write Translation Table Base Register 0
        isb

        mov     pc, lr

END(armv7_btrm_mmuinit)

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

	mrc	p15, 0, r0, c1, c0, 0 // Read Control Register configuration data
	orr	r0, r0, #CR_M 
	mcr	p15, 0, r0, c1, c0, 0 // Write Control Register configuration data
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

        /*16KB mmu page table right below the text code */
  	ldr	r0, =text_start
	sub	r0, r0, #16*1024

#if (INC_BTRM_BUILD==1)
	bl	armv7_btrm_mmuinit
#else
	bl	armv7_mmuinit
#endif

	bl	armv7_enable_mmu
	bl	armv7_l1cache_enable_d
	
	ldmfd	sp!, {r0-r11, lr}
	mov pc, lr
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
	mov pc, lr
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
	ldr	r2,=5                    /* enable and reset PMNC */
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
	bx	lr
END(_getticks)
