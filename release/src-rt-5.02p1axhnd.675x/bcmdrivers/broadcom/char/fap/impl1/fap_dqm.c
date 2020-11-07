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

/******************************************************************************
* File Name  : fapDqm.c                                                       *
*                                                                             *
* Description: This is the DQM implementation for the HOST MIPS core.         *
******************************************************************************/

#include "bcm_OS_Deps.h"
#include <linux/bcm_log.h>
#include "fap.h"
#include "fap_hw.h"
#include "fap_local.h"
#include "fap_task.h"
#include "fap_dqm.h"
#include "fap_dqmHost.h"
#include "fap4ke_memory.h"
#include "fap_protocol.h"
#include "bcmPktDma.h"

#define DQM_MAX_HANDLER_HOST    8

#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30)
static DEFINE_SPINLOCK(fapDqm_lock_g);
#else
static spinlock_t fapDqm_lock_g = SPIN_LOCK_UNLOCKED;
#endif
#define FAP_DQM_LOCK(flags)   spin_lock_irqsave(&fapDqm_lock_g, flags)
#define FAP_DQM_UNLOCK(flags) spin_unlock_irqrestore(&fapDqm_lock_g, flags)

#else

#define FAP_DQM_LOCK(flags)   local_irq_save(flags)
#define FAP_DQM_UNLOCK(flags) local_irq_restore(flags)

#endif

/* queue handler structure */
typedef struct {
   uint32            mask;
   bool              enable;
   dqmHandlerHost_t  handler;
   unsigned long     arg;
} fapHostDqm_handlerInfo_t;

/*
 * fapHost_dqm_t: DQM variables
 */ 
typedef struct{
    /* Queue Handlers */
    uint32 handlerCount;
    fapHostDqm_handlerInfo_t handlerInfo[DQM_MAX_HANDLER_HOST];
} fapHost_dqm_t;

/* host side DQM - global allocated in host DDR */
static fapHost_dqm_t hostDqm;

#define g_hostDqmHandlerInfo  (hostDqm.handlerInfo)
#define g_hostDqmHandlerCount (hostDqm.handlerCount)

#define dqmHandlerEnableHost(_mask)  _dqmHandlerEnableHost(_mask, TRUE)
#define dqmHandlerDisableHost(_mask) _dqmHandlerEnableHost(_mask, FALSE)

void fapIf_recv_dqmhandler(uint32 fapIdx, unsigned long unused);

#ifdef CONFIG_BCM_FAP_GSO_LOOPBACK
void fapGsoLoopBk_recv_dqmhandler(uint32 fapIdx, unsigned long unused);
#endif

/******************************************************************************
* Function: fapDqm_hostInit                                                   *
*                                                                             *
* Description: Host side DQM initialization.                                  *
******************************************************************************/ 
void fapDqm_hostInit(void)
{
    int i;

    BCM_LOG_INFO(BCM_LOG_ID_FAP, "fapDqm_hostInit");

    /* initialize our handler structure */ 
    g_hostDqmHandlerCount = 0;

    for (i = 0; i < DQM_MAX_HANDLER_HOST; i++)
    {
        g_hostDqmHandlerInfo[i].mask = 0;
        g_hostDqmHandlerInfo[i].enable = FALSE;
    }

    /* register our command handlers */

    /* register the enet recv handlers */
    {
        dqmHandlerRegisterHost((1 << (DQM_FAP2HOST_ETH_RX_Q_LOW)),
                               bcm63xx_enet_dqmhandler, 0);
        dqmHandlerEnableHost(1 << (DQM_FAP2HOST_ETH_RX_Q_LOW ));

        dqmHandlerRegisterHost((1 << (DQM_FAP2HOST_ETH_RX_Q_HI)),
                               bcm63xx_enet_dqmhandler, 0);
        dqmHandlerEnableHost(1 << (DQM_FAP2HOST_ETH_RX_Q_HI));
    }

#if (defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE))
    /* register the xtm recv handlers */
    {
        dqmHandlerRegisterHost((1 << (DQM_FAP2HOST_XTM_RX_Q_LOW )),
                               bcm63xx_xtm_dqmhandler, 0);
        dqmHandlerEnableHost(1 << (DQM_FAP2HOST_XTM_RX_Q_LOW ));

        dqmHandlerRegisterHost((1 << (DQM_FAP2HOST_XTM_RX_Q_HI)),
                               bcm63xx_xtm_dqmhandler, 0);
        dqmHandlerEnableHost(1 << (DQM_FAP2HOST_XTM_RX_Q_HI));
    }
#endif

    /* register the host recv handlers */
    dqmHandlerRegisterHost((1 << DQM_FAP2HOST_HOSTIF_Q),
                           fapIf_recv_dqmhandler, 0);
    dqmHandlerEnableHost(1 << DQM_FAP2HOST_HOSTIF_Q);


#ifdef CONFIG_BCM_FAP_GSO_LOOPBACK
    /* register the host gso loopback recv handler */
    dqmHandlerRegisterHost((1 << DQM_FAP2HOST_GSO_LOOPBACK_Q),
                           fapGsoLoopBk_recv_dqmhandler, 0);
    dqmHandlerEnableHost(1 << DQM_FAP2HOST_GSO_LOOPBACK_Q);
#endif

    {
        unsigned long flags;
        uint32 fapIdx;

        local_irq_save(flags);

        for(fapIdx=0; fapIdx<NUM_FAPS; ++fapIdx)
        {
            fapIrq_enable(fapIdx, IRQ_FAP_HOST_DQM);
        }

        local_irq_restore(flags);
    }
}

/******************************************************************************
* Function: dqmHandlerRegisterHost                                            *
*                                                                             *
* Description: register a DQM handler - host side                             *
******************************************************************************/ 
int dqmHandlerRegisterHost(uint32 mask, dqmHandlerHost_t handler,
                           unsigned long arg)
{
    fapHostDqm_handlerInfo_t *handlerInfo_p;

    if (g_hostDqmHandlerCount < DQM_MAX_HANDLER_HOST)
    {
        handlerInfo_p = &g_hostDqmHandlerInfo[g_hostDqmHandlerCount];

        handlerInfo_p->mask = mask;
      
        handlerInfo_p->handler = handler;
        handlerInfo_p->arg = arg;

        handlerInfo_p->enable = FALSE;

        g_hostDqmHandlerCount++;

        BCM_LOG_INFO(BCM_LOG_ID_FAP, "mask: 0x%08x count: %d", 
                     mask, g_hostDqmHandlerCount);
        return 0;
    }
    else
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, 
                      "Exceeded maximum number of DQM IRQ Handlers! (%d)",
                      DQM_MAX_HANDLER);
        return 1;
    }

    return 0;
}

/******************************************************************************
 * Function: dqmIrqHandlerHost                                                 *
 *                                                                             *
 * Description: Host side IRQ handler for the DQM.                             *
 ******************************************************************************/  
void dqmIrqHandlerHost(uint32 fapIdx)
{
    uint32 i, irq_status;
    fapHostDqm_handlerInfo_t *handlerInfo_p;
    unsigned long flags;
 
    irq_status = (FAP_HOST_REG_RD(hostDqmReg(fapIdx)->not_empty_irq_sts) &
                  FAP_HOST_REG_RD(hostDqmReg(fapIdx)->mips_not_empty_irq_msk));

    for (i = 0; i < g_hostDqmHandlerCount; i++)
    {
        handlerInfo_p = &g_hostDqmHandlerInfo[i];

        if ((irq_status & handlerInfo_p->mask) && (handlerInfo_p->enable))
        {
            FAP_DQM_LOCK(flags);
            /* disable this interrupt
               IT SHOULD BE RE-ENABLED BY THE INDIVIDUAL HANDLER */
            FAP_HOST_REG_WR(hostDqmReg(fapIdx)->mips_not_empty_irq_msk,
                            FAP_HOST_REG_RD(hostDqmReg(fapIdx)->mips_not_empty_irq_msk) & ~handlerInfo_p->mask);
            FAP_DQM_UNLOCK(flags);

            handlerInfo_p->handler(fapIdx, handlerInfo_p->arg);
        }
    }

    /* clear the Host DQM interrupt */
    fapIrq_enable(fapIdx, IRQ_FAP_HOST_DQM);
}

/******************************************************************************
* Function: _dqmHandlerEnableHost                                             *
*                                                                             *
* Description: enables or disables a host side DQM handler                    *
*              called by dqmHandlerEnableHost and dqmHandlerDisableHost       *
******************************************************************************/ 
int _dqmHandlerEnableHost(uint32 mask, bool enable)
{
   int i, ret = 1;

   for (i = 0; i < g_hostDqmHandlerCount; i++)
   {
      if (mask == g_hostDqmHandlerInfo[i].mask)
      {
          g_hostDqmHandlerInfo[i].enable = enable;
          ret = 0;
      }
   }

   return ret;
}

/******************************************************************************
* Function: dqmEnableNotEmptyIrqMskHost                                       *
*                                                                             *
* Description: enables the "Not Empty" interrupt of the given DQM             *
*                                                                             *
******************************************************************************/ 
void dqmEnableNotEmptyIrqMskHost(uint32 fapIdx, uint32 dqmBitMask)
{
    unsigned long flags;

    FAP_DQM_LOCK(flags);

    FAP_HOST_REG_WR(hostDqmReg(fapIdx)->mips_not_empty_irq_msk,
                    dqmReadNotEmptyIrqMskHost(fapIdx) | dqmBitMask);

    FAP_DQM_UNLOCK(flags);
}

/******************************************************************************
* Function: dqmEnableLowWtmkIrqMskHost                                        *
*                                                                             *
* Description: enables the "Low Watermark" interrupt of the given DQM         *
*                                                                             *
******************************************************************************/ 
void dqmEnableLowWtmkIrqMskHost(uint32 fapIdx, uint32 dqmBitMask)
{
    unsigned long flags;

    FAP_DQM_LOCK(flags);

    FAP_HOST_REG_WR(hostDqmReg(fapIdx)->mips_low_wtmk_irq_msk,
                    dqmReadLowWtmkIrqMskHost(fapIdx) | dqmBitMask);

    FAP_DQM_UNLOCK(flags);
}
