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

#if defined(_BCM963381_) || defined(_BCM96848_)

#include "sbmips.h"
#include "mipsmacros.h"
#include "segtable.h"
#include "bcm_map.h"

               .text

/*  *********************************************************************
    *  SETLEDS(a,b,c,d)
    *  SETLEDS1(a,b,c,d)
    *
    *  Sets the on-board LED display (if present).  Two variants
    *  of this routine are provided.  If you're running KSEG1,
    *  call the SETLEDS1 variant, else call SETLEDS.
    *
    *  Input parameters:
    *    a,b,c,d - four ASCII characters (literal constants)
    *
    *  Return value:
    *    a0,k1,ra trashed
    ********************************************************************* */

#define SETLEDS(a,b,c,d)                     \
       li     a0,(((a)<<24)|((b)<<16)|((c)<<8)|(d)) ;    \
       CALLINIT_KSEG0(init_table,R_INIT_SETLEDS)

#define R_PMC_WRITE_BPCM_REG _TBLIDX(0)
#define R_PMC_READ_BPCM_REG  _TBLIDX(1)
#define R_PMC_WRITE_ZONE_REG _TBLIDX(2)
#define R_PMC_READ_ZONE_REG  _TBLIDX(3)


pmcinit_table:
                _LONG_  pmc_write_bpcm_reg                  # [ 0 ] R_PMC_WRITE_BPCM_REG
                _LONG_  pmc_read_bpcm_reg                   # [ 1 ] R_PMC_READ_BPCM_REG
                _LONG_  pmc_write_zone_reg                  # [ 2 ] R_PMC_WRITE_ZONE_REG
                _LONG_  pmc_read_zone_reg                   # [ 3 ] R_PMC_READ_ZONE_REG

/*  *********************************************************************
    *  pmc_write_bpcm_reg()
    *
    *   perform a write to a BPCM register via the PMC message handler
    *
    *
    *  Inputs:
    *   a0 - dev address
    *   a1 - register offset
    *   a2 - register value to write
    *
    *  Outputs:
    *   v0 - return value, pmc error code.
    *     zero - success, non zero - error
    *
    *  Register used:
    *   t0, t1, t2
    ********************************************************************* */
LEAF(pmc_write_bpcm_reg)

        li      t0, PMC_BASE
        add     t0, t0, PMC_DQM_QUEUE_DATA_HOST_TO_PMC
        li      t1, 0xd           /* DQM message word0 cmdId[7:0]=cmdWriteBpcmReg=0x0d */
        sw      t1, 0x0(t0)
        and     t1, a0, 0x3ff
        sll     t1, 10            /* DQM message word1 pmb bus index[19:18]=0, dev addr within bus[17:10] */
        sw      t1, 0x4(t0)
        and     t1, a1,0xff      /* register addr offset within pmbs device  */
        sw      t1, 0x8(t0)
        sw      a2, 0xc(t0)       /* Write register value (the 4th word) intiates the write  */ 

        li      t0, PMC_BASE
        li      t1, 2             /* Only need the status for DQM message queue 1 PMC to Host */
dwait1: lw      t2, PMC_DQM_NOT_EMPTY_STS(t0)
        and     t2, t2,t1          /* get rid of everything except 2nd bit */
        bne     t1, t2, dwait1     /* Repeat until DQM1 is ready with data */

        li      t0, PMC_BASE
        add     t0, t0, PMC_DQM_QUEUE_DATA_PMC_TO_HOST
        lw      t1, 0x0(t0)
        srl     t1, 8
        and     v0, t1, 0xff      /* return the PMC error code */
        lw      t1, 0x4(t0)
        lw      t1, 0x8(t0)
        lw      t1, 0xc(t0)      /* read word 3 ( read is complete DQM1 has been flushed) */ 

        j       ra
END(pmc_write_bpcm_reg)


/*  *********************************************************************
    *  pmc_read_bpcm_reg()
    *
    *   perform a read to a BPCM register via the PMC message handler
    *
    *
    *  Inputs:
    *   a0 - dev address
    *   a1 - register offset
    *
    *  Outputs:
    *   v0 - return value, pmc error code.
    *        zero - success, non zero - error
    *   v1 - register value if return success
    *
    *  Register used:
    *   t0, t1, t2
    ********************************************************************* */
LEAF(pmc_read_bpcm_reg)

        li      t0, PMC_BASE
        add     t0, t0, PMC_DQM_QUEUE_DATA_HOST_TO_PMC

        li      t1, 0xb           /* DQM message word0 cmdId[7:0]=cmdReadBpcmReg=0x0b */
        sw      t1, 0x0(t0)
        and     t1, a0, 0x3ff
        sll     t1, 10            /* DQM message word1 pmb bus index[19:18]=0, dev addr within bus[17:10] */
        sw      t1, 0x4(t0)
        and     t1, a1,0xff      /* register addr offset within pmbs device  */
        sw      t1, 0x8(t0)
        sw      zero, 0xc(t0)       /* Write 0 value (the 4th word) intiates the write  */

        li      t0, PMC_BASE
        li      t1, 2             /* Only need the status for DQM message queue 1 PMC to Host */
dwait2: lw      t2, PMC_DQM_NOT_EMPTY_STS(t0)
        and     t2, t2,t1          /* get rid of everything except 2nd bit */
        bne     t1, t2, dwait2     /* Repeat until DQM1 is ready with data */

        li      t0, PMC_BASE
        add     t0, t0, PMC_DQM_QUEUE_DATA_PMC_TO_HOST
        lw      t1, 0x0(t0)
        srl     t1, 8
        and     v0, t1, 0xff      /* return the PMC error code */
        lw      t1, 0x4(t0)
        lw      v1, 0x8(t0)       /* register value */
        lw      t1, 0xc(t0)       /* read word 3 ( read is complete ... DQM1 has been flushed) */

        j        ra

END(pmc_read_bpcm_reg)


/*  *********************************************************************
    *  pmc_write_zone_reg()
    *
    *   perform a write to a zone register via the PMC message handler
    *
    *
    *  Inputs:
    *   a0 - dev address
    *   a1 - zone index
    *   a2 - register offset
    *   a3 - register value to write
    *
    *  Outputs:
    *   v0 - return value, pmc error code.
    *        zero - success, non zero - error
    *
    *  Register used:
    *   t0, t1, t2
    ********************************************************************* */
LEAF(pmc_write_zone_reg)

        li      t0, PMC_BASE
        add     t0, t0, PMC_DQM_QUEUE_DATA_HOST_TO_PMC
        li      t1, 0xe           /* DQM message word0 cmdId[7:0]=cmdWriteZoneReg=0x0e */
        sw      t1, 0x0(t0)
        and     t1, a0, 0x3ff
        sll     t1, 10            /* DQM message word1 pmb bus index[19:18]=0, dev addr within bus[17:10] */
        and     t2, a1, 0x3ff     /* DQM message word1 zone index[9:0] */
        or      t1, t1, t2
        sw      t1, 0x4(t0)
        and     t1, a2,0xff      /* register addr offset within pmbs device  */
        sw      t1, 0x8(t0)
        sw      a3, 0xc(t0)       /* Write register value (the 4th word) intiates the write  */ 

        li      t0, PMC_BASE
        li      t1, 2             /* Only need the status for DQM message queue 1 PMC to Host */
dwait3: lw      t2, PMC_DQM_NOT_EMPTY_STS(t0)
        and     t2, t2,t1          /* get rid of everything except 2nd bit */
        bne     t1, t2, dwait3     /* Repeat until DQM1 is ready with data */

        li      t0, PMC_BASE
        add     t0, t0, PMC_DQM_QUEUE_DATA_PMC_TO_HOST
        lw      t1, 0x0(t0)
        srl     t1, 8
        and     v0, t1, 0xff      /* return the PMC error code */
        lw      t1, 0x4(t0)
        lw      t1, 0x8(t0)
        lw      t1, 0xc(t0)       /* read word 3 ( read is complete ... DQM1 has been flushed) */

        j       ra
END(pmc_write_zone_reg)


/*  *********************************************************************
    *  pmc_read_zone_reg()
    *
    *   perform a read to a BPCM register via the PMC message handler
    *
    *
    *  Inputs:
    *   a0 - dev address
    *   a1 - zone index
    *   a2 - register offset
    *
    *  Outputs:
    *   v0 - return value, pmc error code.
    *        zero - success, non zero - error
    *   v1 - register value if return success
    *
    *  Register used:
    *   t0, t1, t2
    ********************************************************************* */
LEAF(pmc_read_zone_reg)

        li      t0, PMC_BASE
        add     t0, t0, PMC_DQM_QUEUE_DATA_HOST_TO_PMC
        li      t1, 0xc           /* DQM message word0 cmdId[7:0]=cmdReadZoneReg=0x0c */
        sw      t1, 0x0(t0)
        and     t1, a0, 0x3ff
        sll     t1, 10            /* DQM message word1 pmb bus index[19:18]=0, dev addr within bus[17:10] */
        and     t2, a1, 0x3ff     /* DQM message word1 zone index[9:0] */
        or      t1, t1, t2
        sw      t1, 0x4(t0)
        and     t1, a2,0xff      /* register addr offset within pmbs device  */
        sw      t1, 0x8(t0)
        sw      zero, 0xc(t0)       /* Write 0 value (the 4th word) intiates the write  */

        li      t0, PMC_BASE
        li      t1, 2             /* Only need the status for DQM message queue 1 PMC to Host */
dwait4: lw      t2, PMC_DQM_NOT_EMPTY_STS(t0)
        and     t2, t2,t1          /* get rid of everything except 2nd bit */
        bne     t1, t2, dwait4    /* Repeat until DQM1 is ready with data */

        li      t0, PMC_BASE
        add     t0, t0, PMC_DQM_QUEUE_DATA_PMC_TO_HOST
        lw      t1, 0x0(t0)
        srl     t1, 8
        and     v0, t1, 0xff      /* return the PMC error code */
        lw      t1, 0x4(t0)
        lw      v1, 0x8(t0)       /* register value */
        lw      t1, 0xc(t0)       /* read word 3 ( read is complete ... DQM1 has been flushed) */

        j        ra

END(pmc_read_zone_reg)


/*  *********************************************************************
    *  PMC Init
    *
    *  This function power up any necessary modules that are controlled by
    *  PMC for board to boot such as vdsl.
    *  This is called from KSEG0 when still executing in place on flash
    *  Input parameters:
    *       nothing
    *
    *  Return value:
    *       nothing
    ********************************************************************* */
LEAF(pmc_ll_init)

    move    fp,ra

    SETLEDS('P','M','C','I')

#if defined(_BCM96848_) || defined(_BCM963381_)
    /* check straps if pmc rom and avs are running*/
    li      t0, MISC_BASE
    lw      t1, MISC_STRAP_BUS(t0)
    and     t2, t1, MISC_STRAP_BUS_PMC_BOOT_MASK
    beq     t2, 0, pmc_exit
    nop
    and     t2, t1, MISC_STRAP_BUS_PMC_AVS_MASK
    beq     t2, 0, pmc_exit
    nop
#endif

    /* wait for the PMC rom up running */
    li      t0, PMC_BASE
pmc_wait:
    lw      t1, PMC_CONTROL_HOST_MBOX_IN(t0)
    srl     t1, 24
    and     t1, 0x7
    sub     t1, 2    /* kPMCRunStatAVSCompleteWaitingForImage */
    bgez    t1, pmc_start
    /* put some delay and wait again */
    li      t1, 1000
delay:
    sub     t1, 1
    bgtz    t1, delay
    b       pmc_wait

pmc_start:

#if (INC_BTRM_BOOT==1) && defined(_BCM963381_)
    SETLEDS('P','M','C','S')

    /* based on Mahesh's vdsl_init_b0/a0.cc */
#if defined(_BCM963381_)
    /* afe global power down */
    li      a0, 0x10d     /* vdsl core bus addr[9:8] 0x1, dev addr[7:0] 0x0d */
    li      a1, 0xc       /* vdsl control reg offset in vdsl core */
    CALLINIT_KSEG0(pmcinit_table,R_PMC_READ_BPCM_REG)
    bne     v0, zero, pmc_error_retry
    or      a2, v1, 0x1<<31
    CALLINIT_KSEG0(pmcinit_table,R_PMC_WRITE_BPCM_REG)
    bne     v0, zero, pmc_error_retry

    /* check chip revision */
    li      t0, PERF_BASE
    lw      t1, 0x0(t0)
    and     t1, 0xff
    beq     t1, 0xa0, pll_tuning
    nop
    beq     t1, 0xa1, pll_tuning
    nop
    b       tuning_done

pll_tuning:
    /* 63381 A0 or A1 require additional tuning for AFE pll to lock */
    SETLEDS('P','L','L','T')
    li      a0, 0x10e     /* afe pll bus addr[9:8] 0x1, dev addr[7:0] 0x0e */
    li      a1, 0x5       /* cfg0 offset in afe pll */
    CALLINIT_KSEG0(pmcinit_table,R_PMC_READ_BPCM_REG)
    bne     v0, zero, pmc_error_retry
    or      a2, v1, 0x1|0x1<<27 /* bit 0 and bit 27, dco bias boot and fdco ctrl bypass bits */ 
    CALLINIT_KSEG0(pmcinit_table,R_PMC_WRITE_BPCM_REG)
    bne     v0, zero, pmc_error_retry

    li      a0, 0x10e     /* afe pll bus addr[9:8] 0x1, dev addr[7:0] 0x0e */
    li      a1, 0x6       /* cfg1 offset in afe pll */
    CALLINIT_KSEG0(pmcinit_table,R_PMC_READ_BPCM_REG)
    bne     v0, zero, pmc_error_retry
    and     v1, 0xfffff9ff
    or      a2, v1, 0x2<<9 /* bit[10:9] port reset mode = 2 */
    CALLINIT_KSEG0(pmcinit_table,R_PMC_WRITE_BPCM_REG)
    bne     v0, zero, pmc_error_retry

    li      a0, 0x10e     /* afe pll bus addr[9:8] 0x1, dev addr[7:0] 0x0e */
    li      a1, 0x9       /* loop0 offset in afe pll */
    CALLINIT_KSEG0(pmcinit_table,R_PMC_READ_BPCM_REG)
    bne     v0, zero, pmc_error_retry
    and     v1, 0xffff08ff
    or      a2, v1, 0x5<<12|0x3<<8 /* ki bit[10:8]= 3, kp bit[15:12]=5 */
    CALLINIT_KSEG0(pmcinit_table,R_PMC_WRITE_BPCM_REG)
    bne     v0, zero, pmc_error_retry

tuning_done:
#endif

    //SETLEDS('P','M','C','A')
    /* set afe pll control */
    li      a0, 0x10e     /* afe pll bus addr[9:8] 0x1, dev addr[7:0] 0x0e */
    li      a1, 0x4       /* pll_reset reg offset in afe pll */
    CALLINIT_KSEG0(pmcinit_table,R_PMC_READ_BPCM_REG)
    bne     v0, zero, pmc_error_retry
    or      a2, v1, 0x3
    CALLINIT_KSEG0(pmcinit_table,R_PMC_WRITE_BPCM_REG)
    bne     v0, zero, pmc_error_retry

    /* wait for pll lock */
waitlock:
    li      a0, 0x10e     /* afe pll bus addr[9:8] 0x1, dev addr[7:0] 0x0e */
    li      a1, 0xa       /* pll_loop1 reg offset in afe pll */
    CALLINIT_KSEG0(pmcinit_table,R_PMC_READ_BPCM_REG)
    bne     v0, zero, pmc_error_retry
    and     v1, 0x80000000
    beq     v1, zero, waitlock
    SETLEDS('A','F','E','L')

#if defined(_BCM963381_)
    /* de-assert afe global power down */
    li      a0, 0x10d     /* vdsl core bus addr[9:8] 0x1, dev addr[7:0] 0x0d */
    li      a1, 0xc       /* vdsl control reg offset in vdsl core */
    CALLINIT_KSEG0(pmcinit_table,R_PMC_READ_BPCM_REG)
    bne     v0, zero, pmc_error_retry
    and     a2, v1, ~(0x1<<31)
    CALLINIT_KSEG0(pmcinit_table,R_PMC_WRITE_BPCM_REG)
    bne     v0, zero, pmc_error_retry
#endif

    SETLEDS('P','W','R','Z')
    /* Power on VDSL zone 0 */
    li      a0, 0x10d       /* vdsl core bus addr[9:8] 0x1, dev addr[7:0] 0x0d */        
    li      a1, 0x10         /* zone 0 reg offset in vdsl core */
    CALLINIT_KSEG0(pmcinit_table,R_PMC_READ_BPCM_REG)
    bne     v0, zero, pmc_error_retry
    or      a2, v1, 0x1d00
    CALLINIT_KSEG0(pmcinit_table,R_PMC_WRITE_BPCM_REG)
    bne     v0, zero, pmc_error_retry

    SETLEDS('M','E','M','L')
    /* soft reset vdsl */
    li      a0, 0x10d       /* vdsl core bus addr[9:8] 0x1, dev addr[7:0] 0x0d */
    li      a1, 0xa         /* sr_control reg offset in vdsl core */
    CALLINIT_KSEG0(pmcinit_table,R_PMC_READ_BPCM_REG)
    bne     v0, zero, pmc_error_retry
    or      a2, v1, 0xffffff01
    CALLINIT_KSEG0(pmcinit_table,R_PMC_WRITE_BPCM_REG)
    bne     v0, zero, pmc_error_retry

    /* enable vdsl, mips and afe clock */
    //SETLEDS('P','M','C','C')
    li      a0, 0x10d       /* vdsl core bus addr[9:8] 0x1, dev addr[7:0] 0x0d */
    li      a1, 0xc         /* vdsl_phy_control reg offset in vdsl core */
    CALLINIT_KSEG0(pmcinit_table,R_PMC_READ_BPCM_REG)
    bne     v0, zero, pmc_error_retry
    or      a2, v1, 0x3
    CALLINIT_KSEG0(pmcinit_table,R_PMC_WRITE_BPCM_REG)
    bne     v0, zero, pmc_error_retry

    //SETLEDS('P','M','C','R')
    /* out of reset vdsl? */
    li      a0, 0x10d       /* vdsl core bus addr[9:8] 0x1, dev addr[7:0] 0x0d */
    li      a1, 0xa         /* sr_control reg offset in vdsl core */
    CALLINIT_KSEG0(pmcinit_table,R_PMC_READ_BPCM_REG)
    bne     v0, zero, pmc_error_retry
    or      a2, v1, 0xffffff03
    CALLINIT_KSEG0(pmcinit_table,R_PMC_WRITE_BPCM_REG)
    bne     v0, zero, pmc_error_retry


    //SETLEDS('P','M','C','K')
    /* clkrst control */
    li      a0, 0x5       /* clkrst bus addr[9:8] 0x0, dev addr[7:0] 0x05 */
    li      a1, 0xe       /* xtal_control reg offset in clkrst */
    CALLINIT_KSEG0(pmcinit_table,R_PMC_READ_BPCM_REG)
    bne     v0, zero, pmc_error_retry
    or      a2, v1, 0x80000
    CALLINIT_KSEG0(pmcinit_table,R_PMC_WRITE_BPCM_REG)
    bne     v0, zero, pmc_error_retry

    /* some delay needed */
    li      t1, 5000
delay2:
    sub     t1, 1
    bgtz    t1, delay2
    nop

    b       pmc_done

pmc_error_retry:
    li      a0, 0x45525200   /* ERR0 */
    add     a0, a0, v0
    CALLINIT_KSEG0(init_table,R_INIT_SETLEDS)

    SETLEDS('P','M','C','E')
    b       pmc_start

pmc_done:
    SETLEDS('P','M','C','D')
#elif defined(_BCM963381_)
pmcm:
    SETLEDS('P','M','C','M')
    /* adjust margins to 75,75 */

    li      t1, 34/* DQM message word0 cmdId[7:0]=cmdCloseAVS=34 */
    sw      t1, PMC_DQM_QUEUE_DATA_HOST_TO_PMC+0x0(t0)
    li      t1, 0
    sw      t1, PMC_DQM_QUEUE_DATA_HOST_TO_PMC+0x4(t0)
    li      t1, 75
    sw      t1, PMC_DQM_QUEUE_DATA_HOST_TO_PMC+0x8(t0)
    li      t1, 75
    sw      t1, PMC_DQM_QUEUE_DATA_HOST_TO_PMC+0xc(t0)        /* Write register value (the 4th word) intiates the write  */ 

pmcw:
    lw      t1, PMC_DQM_NOT_EMPTY_STS(t0)
    and     t1, 2
    sub     t1, 2
    bnez    t1, pmcw

    lw      t1, PMC_DQM_QUEUE_DATA_PMC_TO_HOST+0x0(t0)
    lw      t1, PMC_DQM_QUEUE_DATA_PMC_TO_HOST+0x4(t0)
    lw      t1, PMC_DQM_QUEUE_DATA_PMC_TO_HOST+0x8(t0)
    lw      t1, PMC_DQM_QUEUE_DATA_PMC_TO_HOST+0xc(t0)        /* read word 3 ( read is complete ... DQM1 has been flushed) */
#endif

pmc_exit:
    move    ra,fp
    j       ra

END(pmc_ll_init)

#endif
