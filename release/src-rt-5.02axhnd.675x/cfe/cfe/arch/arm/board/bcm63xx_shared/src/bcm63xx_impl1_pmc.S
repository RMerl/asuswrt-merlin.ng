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
#include "arm.h"
#include "bsp_config.h"

#include "bcm_cpu.h"
#include "bcm_map.h"
	
#include "bcm_hwdefs.h"
#include "boardparms.h"
#include "armmacros.h"
	
#include "pmc_drv.h" 

#ifndef INC_BTRM_BUILD
#define INC_BTRM_BUILD 0
#endif

      .text
	
/*  *********************************************************************
    *  pmc_send_cmd()
    *
    *   send pmc cmd to DQM queue and wait for response
    *   
    *	 
    *  Inputs:
    *   r0 - DQM input word0 
    *   r1 - DQM input word1
    *   r2 - DQM input word2
    *   r3 - DQM input word3
    *	
    *  Outputs:
    *   r0 - DQM output word0 
    *   r1 - DQM output word1
    *   r2 - DQM output word2
    *   r3 - DQM output word3
    *	
    *  Register used:
    *   r0, r1, r2, r3, r4
    ********************************************************************* */
FUNC(pmc_send_cmd)

        ldr     r4, =PMC_DQM_QUEUE_DATA_BASE
	add     r4, #PMC_DQM_QUEUE_DATA_HOST_TO_PMC 
	str     r0, [r4, #0x0]
        str     r1, [r4, #0x4]       
        str     r2, [r4, #0x8]       
        str     r3, [r4, #0xc]     /* Write register value (the 4th word) intiates the write  */ 

        ldr     r4, =PMC_DQM_BASE
        mov     r1, #2             /* Only need the status for DQM message queue 1 PMC to Host */
pmcw:   ldr     r2, [r4, #PMC_DQM_NOT_EMPTY_STS]
        and     r2, r2, r1          /* get rid of everything except 2nd bit  */
	cmp     r2, #0 
        beq     pmcw             /* Repeat until DQM1 is ready with data  */
	
        ldr     r4, =PMC_DQM_QUEUE_DATA_BASE
	add     r4, #PMC_DQM_QUEUE_DATA_PMC_TO_HOST
        ldr     r0, [r4, #0x0]
        ldr     r1, [r4, #0x4]
        ldr     r2, [r4, #0x8]
        ldr     r3, [r4, #0xc]     /* read word 3 ( read is complete ... DQM1 has been flushed) */

	mov  pc, lr
END(pmc_send_cmd)

/*  *********************************************************************
    *  pmb_send_cmd()
    *
    *   register access using power mamagement bus directly
    *   
    *	 
    *  Inputs:
    *   r0 - dev address 
    *   r1 - register offset 
    *   r2 - register value for write, ignore for read
    *	r3 - 0 for read, 1 for write
    *  Outputs:
    *   r0 - return value
    * 	     zero - success, 1 - time out, 2 - slave error
    *   r1 - read value 
    *	
    *  Register used:
    *   r0, r1, r2, r3, r4
    ********************************************************************* */
FUNC(pmb_send_cmd)
	
	mov     r4, #3             /* check which power management bus */
        ands    r4, r0, LSR #8
	ldreq   r4, =PMBM0_BASE
        ldrne   r4, =PMBM1_BASE

	and     r0, #0xff
	orr     r1, r0, LSL #12
	orr     r1, r3, LSL #PMB_CNTRL_CMD_SHIFT

	/* make sure PMBM is not busy */
pmbw1:
	mov     r3, #1
	ldr     r0, [r4, #PMB_CNTRL]
	ands    r3, r0, LSR #PMB_CNTRL_BUSY_SHIFT
	bne     pmbw1

	/* store the data if for write */
	mov     r3, r1, LSR #PMB_CNTRL_CMD_SHIFT
	ands    r3, #0x1
	beq     scmd
        str     r2, [r4, #PMB_WR_DATA]
scmd:	
	/* send the cmd */
        str     r1, [r4, #PMB_CNTRL]
	mov     r3, #1
	orr     r1, r3, LSL #PMB_CNTRL_START_SHIFT
        str     r1, [r4, #PMB_CNTRL]

	/* make sure the cmd is done */
pmbw2:
	mov     r3, #1
	ldr     r0, [r4, #PMB_CNTRL]
	ands    r3, r0, LSR #PMB_CNTRL_START_SHIFT
	bne     pmbw2

	/* check if any error */
	mov     r3, #1
	ands    r3, r0, LSR #PMB_CNTRL_TIMEOUT_ERR_SHIFT
	movne   r0, #1
	bne     pmb_done
	
	mov     r3, #1
	ands    r3, r0, LSR #PMB_CNTRL_SLAVE_ERR_SHIFT
	movne   r0, #2
	bne     pmb_done

	mov     r0, #0
	mov     r3, r1, LSR #PMB_CNTRL_CMD_SHIFT
	ands    r3, #0x1
	bne     pmb_done
        ldr     r1, [r4, #PMB_RD_DATA]	
	
pmb_done:	
	mov  pc, lr
	
END(pmb_send_cmd)
	
/*  *********************************************************************
    *  pmc_write_bpcm_reg()
    *
    *   perform a write to a BPCM register via the PMC message handler
    *   
    *	 
    *  Inputs:
    *   r0 - dev address 
    *   r1 - register offset 
    *   r2 - register value to write
    *	
    *  Outputs:
    *   r0 - return value, pmc error code.
    * 	     zero - success, non zero - error
    *	
    *  Register used:
    *   r0, r1, r2, r3, r4, r10
    ********************************************************************* */
FUNC(pmc_write_bpcm_reg)
	mov	r10, lr		   /* persevere link reg across call */

#if (INC_BTRM_BUILD == 0)

	ldr     r4, =MISC_BASE     /* check if PMC ROM is enabled or not */
	ldr     r4, [r4, #MISC_STRAP_BUS]   
	lsr     r4, #MISC_STRAP_BUS_PMCROM_BOOT_SHIFT
	ands    r4, #0x1
	beq     nopmc1
	
#endif

        mov     r3, r2
        and     r2, r1, #0xff      /* register addr offset within pmbs device  */
	ldr     r4, =0x3ff
        and     r1, r0, r4
	lsl     r1, #10            /* DQM message word1 pmb bus index[19:18]=0, dev addr within bus[17:10] */
	mov     r0, #0xd           /* DQM message word0 cmdId[7:0]=cmdWriteBpcmReg=0x0d */

        CALLINIT(=pmc_send_cmd)
	
	lsr     r0, #8
	and     r0, #0xff         /* return the PMC error code */
	b       write_reg_done
	
nopmc1:                           /* use PMB direct access */
        mov     r3, #1            /* 1 for write */
	CALLINIT(=pmb_send_cmd)
	
write_reg_done:	
	mov	lr, r10		/* restore link */
	mov	pc, lr
END(pmc_write_bpcm_reg)


/*  *********************************************************************
    *  pmc_read_bpcm_reg()
    *
    *   perform a read to a BPCM register via the PMC message handler
    *   
    *	 
    *  Inputs:
    *   r0 - dev address 
    *   r1 - register offset 
    *	
    *  Outputs:
    *   r0 - return value, pmc error code.
    * 	     zero - success, non zero - error
    *	r1 - register value if return success
    *	
    *  Register used:
    *   r0, r1, r2, r3, r4, r10
    ********************************************************************* */
LEAF(pmc_read_bpcm_reg)

	mov	r10, lr		   /* persevere link reg across call */

#if (INC_BTRM_BUILD == 0)

	ldr     r4, =MISC_BASE     /* check if PMC ROM is enabled or not */
	ldr     r4, [r4, #MISC_STRAP_BUS]   
	lsr     r4, #MISC_STRAP_BUS_PMCROM_BOOT_SHIFT
	ands    r4, #0x1
	beq     nopmc2
	
#endif

        mov     r3, #0
        and     r2, r1, #0xff      /* register addr offset within pmbs device  */
	ldr     r4, =0x3ff
        and     r1, r0, r4
	lsl     r1, #10            /* DQM message word1 pmb bus index[19:18]=0, dev addr within bus[17:10] */
	mov     r0, #0xb           /* DQM message word0 cmdId[7:0]=cmdReadBpcmReg=0x0b */

        CALLINIT(=pmc_send_cmd)
	
	lsr     r0, #8
	and     r0, #0xff      /* return the PMC error code */
        mov     r1, r2
	b       read_reg_done
	
nopmc2:                           /* use PMB direct access */
        mov     r3, #0            /* 0 for write */
	CALLINIT(=pmb_send_cmd)
	
read_reg_done:	
	mov	lr, r10		/* restore link */
	mov	pc, lr

END(pmc_read_bpcm_reg)

	
#ifdef USE_ZONE_API
/*  *********************************************************************
    *  pmc_write_zone_reg()
    *
    *   perform a write to a zone register via the PMC message handler
    *   
    *	 
    *  Inputs:
    *   r0 - dev address
    *   r1 - zone index	
    *   r2 - zone register offset 
    *   r3 - register value to write
    *	
    *  Outputs:
    *   r0 - return value, pmc error code.
    * 	     zero - success, non zero - error
    *	
    *  Register used:
    *   r0, r1, r2, r3, r4, r10
    ********************************************************************* */
FUNC(pmc_write_zone_reg)

	mov	r10, lr		   /* persevere link reg across call */

#if (INC_BTRM_BUILD == 0)

	ldr     r4, =MISC_BASE     /* check if PMC ROM is enabled or not */
	ldr     r4, [r4, #MISC_STRAP_BUS]   
	lsr     r4, #MISC_STRAP_BUS_PMCROM_BOOT_SHIFT
	ands    r4, #0x1
	beq     nopmc3
	
#endif

        mov     r3, r3            
        and     r2, r2, #0xff      /* register addr offset within pmbs device  */
	ldr     r4, =0x3ff
        and     r0, r0, r4
	lsl     r0, #10            /* DQM message word1 pmb bus index[19:18]=0, dev addr within bus[17:10] */
        and     r1, r1, r4	
	orr     r1, r0
	mov     r0, #0xe           /* DQM message word0 cmdId[7:0]=cmdWriteZoneReg=0x0e */

        CALLINIT(=pmc_send_cmd)
	
	lsr     r0, #8
	and     r0, #0xff      /* return the PMC error code */
	b       write_zone_done
	
nopmc3:                           /* use PMB direct access */
	/* convert zone indext to reg offset, zone register start from reg off 16 and each zone has 4 reg, see bpcm.h for details */
	add     r2, r1, LSL #2
	add     r1, r2, #0x10
	mov     r2, r3
        mov     r3, #1            /* 1 for write */
	CALLINIT(=pmb_send_cmd)
	
write_zone_done:	
	mov	lr, r10       /* restore link */
	mov	pc, lr
	
END(pmc_write_zone_reg)


/*  *********************************************************************
    *  pmc_read_zone_reg()
    *
    *   perform a read to a BPCM register via the PMC message handler
    *   
    *	 
    *  Inputs:
    *   r0 - dev address
    *   r1 - zone index	
    *   r2 - zone register offset 
    *	
    *  Outputs:
    *   r0 - return value, pmc error code.
    * 	     zero - success, non zero - error
    *	r1 - register value if return success
    *	
    *  Register used:
    *   r0, r1, r2, r3, r4, r10
    ********************************************************************* */
FUNC(pmc_read_zone_reg)

	mov	r10, lr		   /* persevere link reg across call */

#if (INC_BTRM_BUILD == 0)

	ldr     r4, =MISC_BASE     /* check if PMC ROM is enabled or not */
	ldr     r4, [r4, #MISC_STRAP_BUS]   
	lsr     r4, #MISC_STRAP_BUS_PMCROM_BOOT_SHIFT
	ands    r4, #0x1
	beq     nopmc4
	
#endif

        mov     r3, #0
        and     r2, r2, #0xff      /* register addr offset within pmbs device  */
	ldr     r4, =0x3ff
        and     r0, r0, r4
	lsl     r0, #10            /* DQM message word1 pmb bus index[19:18]=0, dev addr within bus[17:10] */
	and     r1, r1, r4     /* DQM message word1 zone index[9:0] */
	orr     r1, r0
	mov     r0, #0xc           /* DQM message word0 cmdId[7:0]=cmdReadZoneReg=0x0e */

        CALLINIT(=pmc_send_cmd)
	
	lsr     r0, #8
	and     r0, #0xff      /* return the PMC error code */
        mov     r1, r2
	b       read_zone_done
	
nopmc4:                           /* use PMB direct access */
	/* convert zone indext to reg offset, zone register start from reg off 16 and each zone has 4 reg, see bpcm.h for details */
	add     r2, r1, LSL #2
	add     r1, r2, #0x10
	mov     r2, #0
        mov     r3, #0            /* 0 for read */
	CALLINIT(=pmb_send_cmd)

read_zone_done:	
	mov	lr, r10		/* restore link */
	mov	pc, lr

END(pmc_read_zone_reg)
#endif

	
/*  *********************************************************************
    *  PMC Init
    *  
    *  This function power up any necessary modules that are controlled by
    *  PMC for board to boot such as vdsl.
    *  This is called when still executing in place on flash
	
    *  Input parameters: 
    *  	   nothing
    *  	   
    *   r0 - return value
    * 	     zero - success, non zero - error
    ********************************************************************* */
FUNC(pmc_ll_init)

    mov	    r9, lr		   /* persevere link reg across call */

#if (INC_BTRM_BUILD == 0)

    ldr     r0, =MISC_BASE     /* check if PMC ROM is enabled or not */
    ldr     r0, [r0, #MISC_STRAP_BUS]   
    lsr     r0, #MISC_STRAP_BUS_PMCROM_BOOT_SHIFT
    ands    r0, #0x1
    beq     pmbd

#endif

    SETLEDS('P','M','C','S')
    b       pmcs	
pmbd:
    SETLEDS('P','M','B','S')
pmcs:
	
/* workaround for the high temp lock issue. no need for 148 because these setting are already in the chip */
#if defined(_BCM963138_)  
    /* config AFE PLL */	
    mov     r0, #AFEPLL_PMB_ADDR_VDSL3_CORE
    mov     r1, #0x5         /*  cfg[0] reg offset in PLL_BPCM_REGS */ 
    CALLINIT(=pmc_read_bpcm_reg)
    cmp     r0, #0
    bne     pmc_error

    mov     r0, #1
    lsl     r0, #27
    orr     r2, r1, r0
    mov     r0, #AFEPLL_PMB_ADDR_VDSL3_CORE
    mov     r1, #0x5         /* cfg[0] reg offset in PLL_BPCM_REGS */
    CALLINIT(=pmc_write_bpcm_reg)
    cmp     r0, #0
    bne     pmc_error

    /* overwrite ndiv and pdiv */
    ldr     r2, =0x80000001
    mov     r0, #AFEPLL_PMB_ADDR_VDSL3_CORE
    mov     r1, #0x12         /* pdiv reg offset in PLL_BPCM_REGS */
    CALLINIT(=pmc_write_bpcm_reg)
    cmp     r0, #0
    bne     pmc_error	

    ldr     r2, =0x80000030
    mov     r0, #AFEPLL_PMB_ADDR_VDSL3_CORE
    mov     r1, #0x11         /* ndiv reg offset in PLL_BPCM_REGS */
    CALLINIT(=pmc_write_bpcm_reg)
    cmp     r0, #0
    bne     pmc_error	
#endif

    /* start AFE PLL */	
    mov     r0, #AFEPLL_PMB_ADDR_VDSL3_CORE
    mov     r1, #0x4         /* resets reg offset in PLL_BPCM_REGS */ 
    CALLINIT(=pmc_read_bpcm_reg)
    cmp     r0, #0
    bne     pmc_error
	
    orr     r2, r1, #0x3
    mov     r0, #AFEPLL_PMB_ADDR_VDSL3_CORE
    mov     r1, #0x4         /* resets reg offset in PLL_BPCM_REGS */
    CALLINIT(=pmc_write_bpcm_reg)
    cmp     r0, #0
    bne     pmc_error

    /* wait AFE PLL to lock */
afel:
    mov     r0, #AFEPLL_PMB_ADDR_VDSL3_CORE
    mov     r1, #0xa         
    CALLINIT(=pmc_read_bpcm_reg)
    cmp     r0, #0
    bne     pmc_error
	
    /* Wait for AFE PLL lock: repeat read until bit 31 (AFE PLL lock bit) is set                         */
    ldr	    r0, =0x80000000
    and	    r1, r0
    cmp	    r1, #0	      /* if bit 31 is not one, repeat read of reg 0x1700a                        */
    beq	    afel
	
    /* AFE is locked, commence LMEM init                                                                 */
    /* Enable VDSL step 0. Power on zone 0, 1 and 2 */
#ifdef USE_ZONE_API
    mov     r5, #0
pwr_zone_vdsl:	
    mov     r0, #PMB_ADDR_VDSL3_CORE
    mov     r1, r5
    mov     r2, #0
    CALLINIT(=pmc_read_zone_reg)
    cmp     r0, #0
    bne     pmc_error
	
    ldr     r0, =0x1d00
    orr     r3, r0, r1
    mov     r0, #PMB_ADDR_VDSL3_CORE
    mov     r1, r5
    mov     r2, #0	
    CALLINIT(=pmc_write_zone_reg)
    cmp     r0, #0
    bne     pmc_error

    add     r5, #1
    cmp     r5, #3
    bne     pwr_zone_vdsl
#else
    mov     r5, #0x10
pwr_zone_vdsl:	
    mov     r0, #PMB_ADDR_VDSL3_CORE
    mov     r1, r5
    CALLINIT(=pmc_read_bpcm_reg)
    cmp     r0, #0
    bne     pmc_error
	
    ldr     r0, =0x1d00
    orr     r2, r0, r1
    mov     r0, #PMB_ADDR_VDSL3_CORE
    mov     r1, r5         
    CALLINIT(=pmc_write_bpcm_reg)
    cmp     r0, #0
    bne     pmc_error

    add     r5, #4
    /* zone 0 starts from 0x10 offset */	
    cmp     r5, #(0x10+4*PMB_ZONES_VDSL3_CORE)
    bne     pwr_zone_vdsl
#endif
    /* Enable VDSL step 1: initiate a read of register 0x1600a via the PMC message handler,              */
    mov     r0, #PMB_ADDR_VDSL3_CORE
    mov     r1, #0xa
    CALLINIT(=pmc_read_bpcm_reg)
    cmp     r0, #0
    bne     pmc_error
	
    /* Enable VDSL step 1: or data with 0xffffff01 and write back into 0x1600a                           */ 
    ldr     r0, =0xffffff01
    orr     r2, r0, r1
    mov     r0, #PMB_ADDR_VDSL3_CORE
    mov     r1, #0xa         
    CALLINIT(=pmc_write_bpcm_reg)
    cmp     r0, #0
    bne     pmc_error

    /* Enable VDSL step 2   : initiate a read of register 0x1600c via the PMC message handler            */
    mov     r0, #PMB_ADDR_VDSL3_CORE
    mov     r1, #0xc
    CALLINIT(=pmc_read_bpcm_reg)
    cmp     r0, #0
    bne     pmc_error

    /* Enable VDSL step 2   : set the bottom two bits high and rewrite back into 0x1600c                 */
    mov     r0, #0x3
    orr     r2, r0, r1
    mov     r0, #PMB_ADDR_VDSL3_CORE
    mov     r1, #0xc         
    CALLINIT(=pmc_write_bpcm_reg)
    cmp     r0, #0
    bne     pmc_error

    /* Enable VDSL step 3: initiate a read of register 0x1600a via the PMC message handler,              */
    mov     r0, #PMB_ADDR_VDSL3_CORE
    mov     r1, #0xa
    CALLINIT(=pmc_read_bpcm_reg)
    cmp     r0, #0
    bne     pmc_error

    /* Enable VDSL step 3   : write to reg 0x1600a                                                       */
    ldr     r0, =0xffffff03
    orr     r2, r0, r1
    mov     r0, #PMB_ADDR_VDSL3_CORE
    mov     r1, #0xa         
    CALLINIT(=pmc_write_bpcm_reg)
    cmp     r0, #0
    bne     pmc_error

    /* Enable PCM_BMU zones */
    mov     r5, #0x10
pwr_zone_apm:
    mov     r0, #PMB_ADDR_APM
    mov     r1, r5
    CALLINIT(=pmc_read_bpcm_reg)
    cmp     r0, #0
    bne     pmc_error

    ldr     r0, =0x1d00
    orr     r2, r0, r1
    mov     r0, #PMB_ADDR_APM
    mov     r1, r5         
    CALLINIT(=pmc_write_bpcm_reg)
    cmp     r0, #0
    bne     pmc_error

    add     r5, #4
    /* zone 0 starts from 0x10 offset */	
    cmp     r5, #(0x10+4*PMB_ZONES_APM)
    bne     pwr_zone_apm

   /* Move LDO reference to make it settle to the right voltage */	
    ldr     r0, =0x80100130
    ldr     r1, [r0]
    //set bit[15] high
    ldr     r2, =0x8000
    orr     r1, r2
    str     r1, [r0]

#if defined(_BCM963138_)
    /* 63148 does not need to deassert */
    //Wait 550usec de-assert bit[15].
    ldr     r2, =110000
w2:
    sub     r2, #1
    cmp     r2, #0
    bne     w2
    ldr     r1, [r0]
    bic     r1, #0x8000
    str     r1, [r0]
#endif

    b       pmc_done
	
pmc_error:
    ldr     r1, =0x45525230   /* ERR0 */
    add     r0, r1
    CALLINIT(=board_setleds)
    SETLEDS('P','M','C','E')
    mov     r0, #1
    b       pmc_exit
	
pmc_done:
    SETLEDS('P','M','C','D')
    mov     r0, #0
	
pmc_exit:	
    mov	    lr, r9		/* restore link */
    mov	    pc, lr

END(pmc_ll_init)
