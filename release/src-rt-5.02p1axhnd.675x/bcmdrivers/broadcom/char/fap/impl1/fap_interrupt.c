/*
<:copyright-BRCM:2009:proprietary:standard

   Copyright (c) 2009 Broadcom 
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
/*
 *******************************************************************************
 * File Name  : fapDriver.c
 *
 * Description: This file contains Linux character device driver entry points
 *              for the BCM63268 FAP Driver.
 *******************************************************************************
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include <bcm_intr.h>

#include "fap.h"
#include "bcm_OS_Deps.h"
#include <linux/bcm_log.h>
#include "fap_hw.h"
#include "fap_local.h"
#include "fap_dqm.h"
#include "fap_dqmHost.h"
#include "bcmPktDma.h"

static spinlock_t fapIrq_lock;

void fapMailBox_irqHandler(uint32 fapIdx);

static irqreturn_t fapIrq_mainHandler(int irq, void *dev_id)
{
    uint32 fapIdx;
    unsigned long flags;

    fapEnetStats_interrupts();

    fapIdx = getFapIdxFromFapIrq(irq);

    if(!isValidFapIdx(fapIdx))
        return IRQ_HANDLED;

#if 0
    printk("***************** FAP%lu INTERRUPT *****************\n", fapIdx);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), irq_4ke_mask);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), irq_4ke_status);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), irq_mips_mask);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), irq_mips_status);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), gp_mask);
    FAP_PRINT_REG(hostRegCntrl(fapIdx), gp_status);
#endif

    spin_lock_irqsave(&fapIrq_lock, flags);

    if(FAP_HOST_REG_RD(hostRegCntrl(fapIdx)->irq_mips_status) & IRQ_FAP_HOST_MBOX_IN)
    {
        uint32 irq_mips_mask = FAP_HOST_REG_RD(hostRegCntrl(fapIdx)->irq_mips_mask);

        /* mask mailbox interrupt */
        FAP_HOST_REG_WR(hostRegCntrl(fapIdx)->irq_mips_mask, irq_mips_mask & ~IRQ_FAP_HOST_MBOX_IN);

#if 0
        if(FAP_HOST_REG_RD(hostRegCntrl(fapIdx)->irq_mips_mask) !=
           (irq_mips_mask & ~IRQ_FAP_HOST_MBOX_IN))
        {
            printk("\n\tirq_mips_mask IRQ_FAP_HOST_MBOX_IN corrupted!\n");
        }
#endif

        /* call mailbox handler */
        fapMailBox_irqHandler(fapIdx);
    }

    if(FAP_HOST_REG_RD(hostRegCntrl(fapIdx)->irq_mips_status) & IRQ_FAP_HOST_DQM)
    {
        uint32 irq_mips_mask = FAP_HOST_REG_RD(hostRegCntrl(fapIdx)->irq_mips_mask);

        /* mask the DQM interrupt */
        FAP_HOST_REG_WR(hostRegCntrl(fapIdx)->irq_mips_mask, irq_mips_mask & ~IRQ_FAP_HOST_DQM);

#if 0
        if(FAP_HOST_REG_RD(hostRegCntrl(fapIdx)->irq_mips_mask) !=
           (irq_mips_mask & ~IRQ_FAP_HOST_DQM))
        {
            printk("\n\tirq_mips_mask IRQ_FAP_HOST_DQM corrupted!\n");
        }
#endif

        /* call the DQM handler */
        dqmIrqHandlerHost(fapIdx);
    }

    BcmHalInterruptEnable(FAP_INTERRUPT_ENS[fapIdx]);

    spin_unlock_irqrestore(&fapIrq_lock, flags);

    return IRQ_HANDLED;
}

int fapIrq_init(uint32 fapIdx, uint32 isFirstTime)
{
    int ret;

    spin_lock_init(&fapIrq_lock);

    /* initialize interrupt registers */
    FAP_HOST_REG_WR(hostRegCntrl(fapIdx)->irq_4ke_mask, 0);
//    FAP_HOST_REG_WR(hostRegCntrl(fapIdx)->irq_4ke_status, 0xFFFFF);

    FAP_HOST_REG_WR(hostRegCntrl(fapIdx)->irq_mips_mask, 0);
//    FAP_HOST_REG_WR(hostRegCntrl(fapIdx)->irq_mips_status, 0xFFFFF);

    FAP_HOST_REG_WR(hostRegCntrl(fapIdx)->gp_mask, 0);
    FAP_HOST_REG_WR(hostRegCntrl(fapIdx)->gp_status, 0xFFFF9);


    if (isFirstTime)
    {
    	char fap_intr_name[32]={0};
    	snprintf(fap_intr_name, sizeof(fap_intr_name), "fap%d", (int) fapIdx);
        /* register the handler, intrs and bh must be enabled when calling BcmHalMapInterrupt  */
        ret = BcmHalMapInterruptEx((FN_HANDLER)fapIrq_mainHandler,
                         (void*)NULL, FAP_INTERRUPT_ENS[fapIdx],
                         fap_intr_name, INTR_REARM_NO, INTR_AFFINITY_DEFAULT);
        if(ret)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Could not map FAP interrupt <%d>", (int)FAP_INTERRUPT_ENS[fapIdx]);

            return -1;
        }
    }

    BcmHalInterruptEnable(FAP_INTERRUPT_ENS[fapIdx]);

    return 0;
}

MODULE_LICENSE("Proprietary");
