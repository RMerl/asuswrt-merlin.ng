/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#include <linux/linkage.h>
#include <asm/arch/pmc.h>
#include <asm/arch/misc.h>

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
ENTRY(pmc_send_cmd)

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
ENDPROC(pmc_send_cmd)

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
ENTRY(pmb_send_cmd)
	
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
	
ENDPROC(pmb_send_cmd)
	
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
ENTRY(pmc_write_bpcm_reg)
	mov	r10, lr		   /* persevere link reg across call */

	ldr     r4, =MISC_BASE     /* check if PMC ROM is enabled or not */
	ldr     r4, [r4, #MISC_STRAP_BUS]   
	lsr     r4, #MISC_STRAP_BUS_PMC_ROM_BOOT_SHIFT
	ands    r4, #0x1
	beq     nopmc1
	
        mov     r3, r2
        and     r2, r1, #0xff      /* register addr offset within pmbs device  */
	ldr     r4, =0x3ff
        and     r1, r0, r4
	lsl     r1, #10            /* DQM message word1 pmb bus index[19:18]=0, dev addr within bus[17:10] */
	mov     r0, #0xd           /* DQM message word0 cmdId[7:0]=cmdWriteBpcmReg=0x0d */

	bl      pmc_send_cmd
	
	lsr     r0, #8
	and     r0, #0xff         /* return the PMC error code */
	b       write_reg_done
	
nopmc1:                           /* use PMB direct access */
        mov     r3, #1            /* 1 for write */
	bl      pmb_send_cmd
	
write_reg_done:	
	mov	lr, r10		/* restore link */
	mov	pc, lr
ENDPROC(pmc_write_bpcm_reg)


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
ENTRY(pmc_read_bpcm_reg)

	mov	r10, lr		   /* persevere link reg across call */

	ldr     r4, =MISC_BASE     /* check if PMC ROM is enabled or not */
	ldr     r4, [r4, #MISC_STRAP_BUS]   
	lsr     r4, #MISC_STRAP_BUS_PMC_ROM_BOOT_SHIFT
	ands    r4, #0x1
	beq     nopmc2

        mov     r3, #0
        and     r2, r1, #0xff      /* register addr offset within pmbs device  */
	ldr     r4, =0x3ff
        and     r1, r0, r4
	lsl     r1, #10            /* DQM message word1 pmb bus index[19:18]=0, dev addr within bus[17:10] */
	mov     r0, #0xb           /* DQM message word0 cmdId[7:0]=cmdReadBpcmReg=0x0b */

        bl      pmc_send_cmd
	
	lsr     r0, #8
	and     r0, #0xff      /* return the PMC error code */
        mov     r1, r2
	b       read_reg_done
	
nopmc2:                           /* use PMB direct access */
        mov     r3, #0            /* 0 for write */
	bl      pmb_send_cmd
	
read_reg_done:	
	mov	lr, r10		/* restore link */
	mov	pc, lr

ENDPROC(pmc_read_bpcm_reg)

	
