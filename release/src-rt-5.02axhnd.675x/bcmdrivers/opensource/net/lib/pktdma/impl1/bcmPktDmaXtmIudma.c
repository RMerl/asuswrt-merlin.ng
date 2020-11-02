/*
 <:copyright-BRCM:2007:DUAL/GPL:standard
 
    Copyright (c) 2007 Broadcom 
    All Rights Reserved
 
 Unless you and Broadcom execute a separate written software license
 agreement governing use of this software, this software is licensed
 to you under the terms of the GNU General Public License version 2
 (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 with the following added to such license:
 
    As a special exception, the copyright holders of this software give
    you permission to link this software with independent modules, and
    to copy and distribute the resulting executable under terms of your
    choice, provided that you also meet, for each linked independent
    module, the terms and conditions of the license of that module.
    An independent module is a module which is not derived from this
    software.  The special exception does not apply to any modifications
    of the software.
 
 Not withstanding the above, under no circumstances may you combine
 this software in any way with any other Broadcom software provided
 under a license other than the GPL, without Broadcom's express prior
 written consent.
 
:>
*/
/*
 *******************************************************************************
 * File Name  : bcmPktDmaXtmIudma.c
 *
 * Description: This file contains the Packet DMA Implementation for the iuDMA
 *              channels of the XTM Controller.
 * Note       : This bcmPktDma code is tied to impl1 of the Xtm Driver
 *
 *******************************************************************************
 */


#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
#include <bcm_intr.h>
#endif

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
#ifdef FAP_4KE
#include "Fap4keOsDeps.h"
#include "fap4ke_local.h"
#include "fap_task.h"
#include "fap_dqm.h"
#include "bcmenet.h"
#include "fap4ke_mailBox.h"
#include "fap4ke_timers.h"
#include "bcmPktDmaHooks.h"
#include "bcmxtmrtimpl.h"
#else /* FAP_4KE */
#include "bcm_OS_Deps.h"
#include <linux/bcm_log.h>
#include "fap4ke_local.h"
#include "fap_task.h"
#include "fap_dqm.h"
#include "fap4ke_mailBox.h"
#include "fap4ke_timers.h"
#include "bcmPktDmaHooks.h"
#endif /* FAP_4KE */
#endif

#include "bcmPktDma.h"

/* fap4ke_local redfines memset to the 4ke lib one - not what we want */
#if defined memset
#undef memset
#endif

#ifndef FAP_4KE
/* Binding with XTMRT */
PBCMXTMRT_GLOBAL_INFO g_pXtmGlobalInfo = (PBCMXTMRT_GLOBAL_INFO)NULL;
#endif /* FAP_4KE */

//#ifndef FAP_4KE
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
const int g_Xtm_rx_iudma_ownership[XTM_RX_CHANNELS_MAX] =
{
    PKTDMA_XTM_RX_OWNERSHIP   /* rx iudma channel ownership */
};

const int g_Xtm_tx_iudma_ownership[XTM_TX_CHANNELS_MAX] =
{
    PKTDMA_XTM_TX_OWNERSHIP   /* tx iudma channel ownership */
};
#endif /* defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE) */
//#endif /* !FAP_4KE */

int bcmPktDma_XtmInitRxChan_Iudma(uint32 bufDescrs,
                                  BcmPktDma_LocalXtmRxDma *pXtmRxDma)
{
    pXtmRxDma->numRxBds = bufDescrs;
    pXtmRxDma->rxAssignedBds = 0;
    pXtmRxDma->rxHeadIndex = 0;
    pXtmRxDma->rxTailIndex = 0;
    pXtmRxDma->xtmrxchannel_isr_enable = 1;
    pXtmRxDma->rxEnabled = 0;

    return 1;
}

int bcmPktDma_XtmUnInitRxChan_Iudma(BcmPktDma_LocalXtmRxDma *pXtmRxDma)
{
	pXtmRxDma->numRxBds = 0;
    pXtmRxDma->rxAssignedBds = 0;
    pXtmRxDma->rxHeadIndex = 0;
    pXtmRxDma->rxTailIndex = 0;
    pXtmRxDma->xtmrxchannel_isr_enable = 0;
    pXtmRxDma->rxEnabled = 0;

    return 1;
}

int bcmPktDma_XtmInitTxChan_Iudma(uint32 bufDescrs,
                                  BcmPktDma_LocalXtmTxDma *pXtmTxDma,
                                  uint32 dmaType)
{
   //printk("bcmPktDma_XtmInitTxChan_Iudma ch: %ld bufs: %ld txdma: %p\n",
   //        pXtmTxDma->ulDmaIndex, bufDescrs, pXtmTxDma);


   pXtmTxDma->txHeadIndex = 0;
   pXtmTxDma->txTailIndex = 0;
   pXtmTxDma->txEnabled = 0;


    return 1;
}

#ifndef FAP_4KE
/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmSelectRxIrq
 Purpose: Return IRQ number to be used for bcmPkt Rx on a specific channel
-------------------------------------------------------------------------- */
int	bcmPktDma_XtmSelectRxIrq_Iudma(int channel)
{
    return (SAR_RX_INT_ID_BASE + channel);
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmRecvRingSize
 Purpose: Receive ring size (Currently queued Rx Buffers) of the associated DMA desc.
-------------------------------------------------------------------------- */
uint32 bcmPktDma_XtmRecv_RingSize_Iudma(BcmPktDma_LocalXtmRxDma * rxdma)
{
    int ringSize ;

    if (rxdma->rxTailIndex < rxdma->rxHeadIndex) {
       ringSize = rxdma->numRxBds - rxdma->rxHeadIndex ;
       ringSize += rxdma->rxTailIndex ;
    }
    else {
       ringSize = rxdma->rxTailIndex - rxdma->rxHeadIndex ;
    }

    return (ringSize) ;
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmRecv
 Purpose: Receive a packet on a specific channel,
          returning the associated DMA desc
-------------------------------------------------------------------------- */
uint32 bcmPktDma_XtmRecv_Iudma(BcmPktDma_LocalXtmRxDma * rxdma, unsigned char **pBuf, int * pLen)
{
    DmaDesc dmaDesc;

    FAP4KE_IUDMA_PMON_DECLARE();
    FAP4KE_IUDMA_PMON_BEGIN(FAP4KE_PMON_ID_IUDMA_RECV);

    dmaDesc.word0 = 0;

    if (rxdma->rxAssignedBds != 0)
    {
        /* Get the status from the current Rx BD */
        dmaDesc.word0 = rxdma->rxBds[rxdma->rxHeadIndex].word0;

        /* If no more rx packets, we are done for this channel */
        if ((dmaDesc.status & DMA_OWN) == 0)
        {
            int nextIndex = rxdma->rxHeadIndex;
            
            /* Wrap around nextIndex */
            if (++nextIndex == rxdma->numRxBds)
                nextIndex = 0;

#if defined(CONFIG_BCM963381)
            /* Issue seen with 63381/63138 as well */
            /* 6318-A0,B0: CRIUDMA-9
             * After being disabled, an IUDMA RX channel will write out
             * a SW owned descriptor that SW should discarded. The problem
             * is that when reenabled, the channel will not move to the next
             * descriptor. Instead, it re-reads the descriptor that it
             * just wrote out. This gets SW and HW out of sync.
             * This bug might happen on all ubus 2.0 based chip.
             * Here is a work-around for this bug.
             */
            
            /* If the next rx BD is DMA_OWN and
             * the current rx BD is the same rx BD pointed to by iudma,
             * then the current rx BD is a bogus packet. Put the current
             * rx BD back to DMA_OWN, and don't change rxHeadIndex and
             * rxAssignedBds.
             *
             * This work-around should cover the following cases:
             * 1) rx iudma channel is disabled when swBdIdx == hwBdIdx and
             *    the entire rx BD ring is DMA_OWN (ring empty).
             *    Example:
             *        a) Before channel disabled, swBdIdx == hwBdIdx == BD8
             *        b) When channel is disabled, rx iudma will send out a bogus
             *           rx packet at BD8 because BD8 is DMA_OWN. hwBdIdx remains
             *           at BD8.
             *        c) In this case, since the next BD9 is DMA_OWN, and current
             *           swBdIdx(BD8) == hwBdIdx(BD8), the driver will ignore BD8,
             *           and put it back to DMA_OWN. swBdIdx remains at BD8.
             * 2) rx iudma channel is disabled when swBdIdx == hwBdIdx and
             *    the entire rx BD ring is SW_OWN (ring full).
             *    Example:
             *        a) Before channel disabled, swBdIdx == hwBdIdx == BD8
             *        b) When channel is disabled, rx iudma will NOT send out a bogus
             *           rx packet at BD8 because BD8 is SW_OWN. hwBdIdx remains
             *           at BD8.
             *        c) In this case, since the next BD9 is SW_OWN, the driver will
             *           continue process rx BDs until BD7. This time, the next BD8
             *           is DMA_OWN (already processed), but current
             *           swBDIdx(BD7) != hwBDIdx(BD8), the driver will process BD7.
             *           There is no bogus packet to be ignored.
             * 3) rx iudma channel is disabled when swBdIdx is before hwBdIdx in the
             *    ring.
             *    Example:
             *        a) Before channel disabled, swBdIdx = BD5; hwBdIdx = BD24.
             *           i.e. BD5 to BD23 is SW_OWN;  BD24 to BD4 is DMA_OWN.
             *        b) When channel is disabled, rx iudma will send out a bogus
             *           rx packet at BD24 because BD24 is DMA_OWN. hwBdIdx remains
             *           at BD24.
             *        c) In this case, since the next BD6 is SW_OWN, the driver will
             *           continue process rx BDs until BD24. This time, the next BD25
             *           is DMA_OWN, and current swBDIdx(BD24) == hwBDIdx(BD24), the
             *           driver will ignore BD24 and put it back to DMA_OWN. swBDIdx
             *           remains at BD24.
             */ 
            if((g_pXtmGlobalInfo->ulDrvState == XTMRT_INITIALIZED) &&
                ((rxdma->rxBds[nextIndex].status & DMA_OWN) &&
                rxdma->rxHeadIndex ==
                    (SAR_DMA->stram.s[SAR_RX_DMA_BASE_CHAN + rxdma->channel].state_data & 0x1FFF)))
            {
                printk("Warning: SAR Channel %d Rx Bogus Event\n", rxdma->channel);
                dmaDesc.length  = BCM_MAX_PKT_LEN;
                dmaDesc.status &= DMA_WRAP;    /* keep the wrap bit */
                dmaDesc.status |= DMA_OWN;
                rxdma->rxBds[rxdma->rxHeadIndex].word0 = dmaDesc.word0;

                /* In the normal case, rxdma channel is re-enabled when rx buffer is freed.
                 * In this case, since we put the current rx BD back to DMA_OWN,
                 * there is no buffer to free. Therefore, we re-enable rxdma channel here.
                 */                
                if (rxdma->rxEnabled)
                    rxdma->rxDma->cfg = DMA_ENABLE;
                
                /* don't change rxHeadIndex and rxAssignedBds */
            }
            else
#endif /* for 63381*/
            {
                *pBuf = (unsigned char *)
                       (phys_to_virt(rxdma->rxBds[rxdma->rxHeadIndex].address));
                *pLen = (int) dmaDesc.length;

                rxdma->rxHeadIndex = nextIndex;
                rxdma->rxAssignedBds--;
            }
        }
    }
    else   /* out of buffers! */
       return (uint32)0xFFFF;

    //printk("XtmRecv_Iudma end ch: %d head: %d tail: %d assigned: %d\n", rxdma->channel, rxdma->rxHeadIndex, rxdma->rxTailIndex, rxdma->rxAssignedBds);

    FAP4KE_IUDMA_PMON_END(FAP4KE_PMON_ID_IUDMA_RECV);

    return dmaDesc.word0;
}
#endif /* FAP_4KE */
/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmXmitAvailable
 Purpose: Determine if there are free resources for the xmit
   Notes: channel in XTM mode refers to a specific TXQINFO struct of a
          specific XTM Context
-------------------------------------------------------------------------- */
int bcmPktDma_XtmXmitAvailable_Iudma(BcmPktDma_LocalXtmTxDma *txdma)
{
    if (txdma->txFreeBds != 0)  return 1;

    return 0;
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmTxEnable_Iudma
 Purpose: Coordinate with FAP for tx enable
  Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */
int bcmPktDma_XtmTxEnable_Iudma( BcmPktDma_XtmTxDma * txdma, PDEV_PARAMS unused, uint32 dmaType )
{
    //printk("bcmPktDma_XtmTxEnable_Iudma ch: %d\n", txdma->ulDmaIndex);

    txdma->txEnabled = 1;

    /* The other SW entity which reads from this DMA in case of SW_DMA,
     * will always look at this bit to start processing anything from
     * the DMA queue.
     */
    if (dmaType == XTM_SW_DMA)
       txdma->txDma->cfg = DMA_ENABLE ;
    return 1;
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmTxDisable_Iudma
 Purpose: Coordinate with FAP for tx disable
  Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */
int bcmPktDma_XtmTxDisable_Iudma( BcmPktDma_LocalXtmTxDma * txdma, uint32 dmaType, void (*func) (uint32 param1,
         BcmPktDma_XtmTxDma *txswdma), uint32 param1)
{
    /* Changing txEnabled to 0 prevents any more packets
     * from being queued on a transmit DMA channel.  Allow all currenlty
     * queued transmit packets to be transmitted before disabling the DMA.
     */
    txdma->txEnabled = 0;

    if (dmaType == XTM_HW_DMA) {

        int j;

        for (j = 0; (j < 40) && ((txdma->txDma->cfg & DMA_ENABLE) == DMA_ENABLE); j++)
        {
            /* Request the iuDMA to disable at the end of the next tx pkt - Jan 2011 */
            txdma->txDma->cfg = DMA_PKT_HALT;
            
#ifdef FAP_4KE
            {
                uint32 prevJiffies = fap4keTmr_jiffiesLoRes + (FAPTMR_HZ_LORES / 10); /* 100 msec */

                while(!fap4keTmr_isTimeAfter(fap4keTmr_jiffiesLoRes, prevJiffies));
            
                /* send a keep alive message to Host */
                fapMailBox_4keSendKeepAlive();
            }
#else
            mdelay(100);  /* 100 msec */
#endif
        }
        txdma->txDma->cfg = 0;
        if((txdma->txDma->cfg & DMA_ENABLE) == DMA_ENABLE)
        {
            /* This should not happen, unless there is a HW/Phy issue. */
           //printk ("bcmxtmrt: warning!!! txdma cfg is DMA_ENABLE \n") ;
           return 0;    /* return so caller can handle the failure */
        }
    } /* if DMA is HW Type */
    else {

       /* No blocking wait for SW DMAs */
       (*func)(param1, txdma) ;
    }

    return 1;
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmRxEnable
    Purpose: Enable rx DMA for the given channel.
    Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */

int bcmPktDma_XtmRxEnable_Iudma( BcmPktDma_LocalXtmRxDma * rxdma )
{

    //printk("bcmPktDma_XtmRxEnable_Iudma: channel=%d\n", rxdma->channel);

    rxdma->rxDma->cfg |= DMA_ENABLE;
    rxdma->rxEnabled = 1;

    return 1;
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_XtmRxDisable
 Purpose: Disable rx interrupts for the given channel
  Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */

int bcmPktDma_XtmRxDisable_Iudma( BcmPktDma_LocalXtmRxDma * rxdma )
{
    int  i;

    //printk("bcmPktDma_XtmRxDisable_Iudma: channel=%d\n", rxdma->channel);

    rxdma->rxEnabled = 0;

    rxdma->rxDma->cfg &= ~DMA_ENABLE;
    for (i = 0; rxdma->rxDma->cfg & DMA_ENABLE; i++)
    {
        rxdma->rxDma->cfg &= ~DMA_ENABLE;

        if (i >= 100)
        {
            //printk("Failed to disable RX DMA?\n");
            return 0;
        }

#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
        /* The delay only works when called from Linux */
        __udelay(20);
#else
        {
            uint32 prevJiffies = fap4keTmr_jiffiesLoRes;

            while(!fap4keTmr_isTimeAfter(fap4keTmr_jiffiesLoRes, prevJiffies));

            if((rxdma->rxDma->cfg & DMA_ENABLE) == DMA_ENABLE)
            {
                return 0;    /* return so caller can handle the failure */
            }
        }
#endif
    }

    return 1;
}


#ifndef FAP_4KE

/* Return non-aligned, cache-based pointer to caller - Apr 2010 */
DmaDesc *bcmPktDma_XtmAllocTxBds(int channel, int numBds, uint32 *phy_addr)
{
    /* Allocate space for pKeyPtr, pTxSource and pTxAddress as well as BDs. */
    int size = sizeof(DmaDesc) + sizeof(BcmPktDma_txRecycle_t);

#if defined(XTM_TX_BDS_IN_PSM) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
{
    uint8 * pMem;
    static uint8 * txBdAllocation[MAX_TRANSMIT_QUEUES] = {NULL};
    static int txNumBds[MAX_TRANSMIT_QUEUES] = {0};
    uint32 fapIdx;

    /* channel is iudma in this instance */
    fapIdx = getFapIdxFromXtmTxIudma(channel);
    if (!isValidFapIdx(fapIdx))
    {
        printk("ERROR: bcmPktDma_XtmAllocTxBds: Tried to allocate using bad fapIdx (%d / %d)\n",
                channel, fapIdx);
        return (NULL);
    }

    /* Restore previous BD allocation pointer if any */
    pMem = txBdAllocation[channel];

    *phy_addr = 0 ;
    if (pMem)
    {
        if(txNumBds[channel] != numBds)
        {
            printk("ERROR: Tried to allocate a different number of txBDs (was %d, attempted %d)\n",
                    txNumBds[channel], numBds);
            printk("       Xtm tx BD allocation rejected!!\n");
            return( NULL );
}
        memset(pMem, 0, numBds * size);
        return((DmaDesc *)pMem);   /* tx bd ring + pKeyPtr, pTxSource and pTxAddress */
    }

    /* Try to allocate Tx Descriptors in PSM. Use Host-side addressing here. */
    /* fapDrv_psmAlloc guarantees 8 byte alignment. */
    pMem = bcmPktDma_psmAlloc(fapIdx, numBds * size);
    if(pMem != FAP4KE_OUT_OF_PSM)
    {
        memset(pMem, 0, numBds * size);
        txBdAllocation[channel] = pMem;
        txNumBds[channel] = numBds;
        return((DmaDesc *)pMem);   /* tx bd ring + pKeyPtr, pTxSource and pTxAddress */
}

    printk("ERROR: Out of PSM. Xtm tx BD allocation rejected!!\n");
    return(NULL);
}
#else  /* !defined(XTM_TX_BDS_IN_PSM) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)) */
#if !defined(CONFIG_ARM)
{
    void * p;

    /* Allocate Tx Descriptors in DDR */
    /* Leave room for alignment by caller - Apr 2010 */
    p = kmalloc(numBds * size + 0x10, GFP_ATOMIC) ;
    if (p !=NULL) {
        memset(p, 0, numBds * size + 0x10);
        cache_flush_len(p, numBds * size + 0x10);
    }
    *phy_addr = 0 ;
    return( (DmaDesc *)p );   /* tx bd ring + pKeyPtr, pTxSource and pTxAddress */
}
#else
{
   uint32_t size32 = (uint32_t) (numBds * size + 0x10) ;
   dma_addr_t phys_addr ;
   /* memory allocation of NONCACHE_MALLOC for ARM is aligned to page size which is aligned to cache */
   volatile uint32_t *mem = (volatile uint32_t *)NONCACHED_MALLOC(size32, &phys_addr);
   
   if (mem != NULL) {
      memset((char *) mem, 0, size32) ;
   }
   *phy_addr = (uint32_t) phys_addr ;
   return ( (DmaDesc *) mem );   /* tx bd ring + pKeyPtr, pTxSource and pTxAddress */
}
#endif
#endif   /* defined(XTM_TX_BDS_IN_PSM) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)) */
}

void bcmPktDma_XtmFreeTxBds(DmaDesc *mem, DmaDesc *phy_addr, int numBds)
{
#if !defined(CONFIG_ARM)
      kfree ((void *) mem) ;
#else
{
   int size = sizeof(DmaDesc) + sizeof(BcmPktDma_txRecycle_t) ;

   uint32_t size32 = (uint32_t) (numBds * size + 0x10) ;
   NONCACHED_FREE (size32, mem, (dma_addr_t) phy_addr) ;
}
#endif
}

/* Return non-aligned, cache-based pointer to caller - Apr 2010 */
DmaDesc *bcmPktDma_XtmAllocRxBds(int channel, int numBds, uint32 *phy_addr)
{
#if defined(XTM_RX_BDS_IN_PSM) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
{
    uint8 * pMem;
    static uint8 * rxBdAllocation[MAX_RECEIVE_QUEUES] = {NULL};
    static int rxNumBds[MAX_RECEIVE_QUEUES] = {0};
    uint32 fapIdx;

    /* Restore previous BD allocation pointer if any */
    pMem = rxBdAllocation[channel];
    fapIdx = getFapIdxFromXtmRxIudma(channel);
    *phy_addr = 0 ;
    if (!isValidFapIdx(fapIdx))
    {
        printk("ERROR: bcmPktDma_XtmAllocRxBds: Invalid Fap Index (channel=%d, fapIdx=%d)\n", channel, fapIdx);
    }

    if (pMem)
    {
        if(rxNumBds[channel] != numBds)
        {
            printk("ERROR: Tried to allocate a different number of rxBDs (was %d, attempted %d)\n",
                    rxNumBds[channel], numBds);
            printk("       Xtm rx BD allocation rejected!!\n");
            return( NULL );
        }
        memset(pMem, 0, numBds * sizeof(DmaDesc));
        return((DmaDesc *)pMem);   /* rx bd ring */
    }

    /* Try to allocate Rx Descriptors in PSM. Use Host-side addressing here. */
    /* fapDrv_psmAlloc guarantees 8 byte alignment. */
    pMem = bcmPktDma_psmAlloc(fapIdx, numBds * sizeof(DmaDesc));
    if(pMem != FAP4KE_OUT_OF_PSM)
    {
        memset(pMem, 0, numBds * sizeof(DmaDesc));
        rxBdAllocation[channel] = pMem;
        rxNumBds[channel] = numBds;
        return((DmaDesc *)pMem);   /* rx bd ring */
    }

    printk("ERROR: Out of PSM. Xtm rx BD allocation rejected!!\n");
    return( NULL );
}
#else   /* !defined(XTM_RX_BDS_IN_PSM) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)) */
#if !defined(CONFIG_ARM)
{
    void * p;

    /* Allocate Rx Descriptors in DDR */
    /* Leave room for alignment by caller - Apr 2010 */
    p = kmalloc(numBds * sizeof(DmaDesc) + 0x10, GFP_ATOMIC) ;
    if (p != NULL) {
        memset(p, 0, numBds * sizeof(DmaDesc) + 0x10);
        cache_flush_len(p, numBds * sizeof(DmaDesc) + 0x10);
    }
    *phy_addr = 0 ;
    return((DmaDesc *)p);   /* rx bd ring */
}
#else
{
   dma_addr_t phys_addr;
   uint32_t size32 = (uint32_t) (numBds * sizeof(DmaDesc) + 0x10) ;
   /* memory allocation of NONCACHE_MALLOC for ARM is aligned to page size which is aligned to cache */
   volatile uint32_t *mem = (volatile uint32_t *)NONCACHED_MALLOC(size32, &phys_addr);

   if (mem != NULL) {
      memset((char *) mem, 0, size32) ;
      //FLUSH_RANGE (mem, size32) ;
   }

   *phy_addr = (uint32_t) phys_addr ;
   return ( (DmaDesc *) mem );   /* tx bd ring + pKeyPtr, pTxSource and pTxAddress */
}
#endif
#endif   /* defined(XTM_RX_BDS_IN_PSM) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)) */
}

#if defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE)
int bcmPktDma_XtmSetIqThresh_Iudma( BcmPktDma_LocalXtmRxDma * rxdma,
                                    uint16 loThresh,
                                    uint16 hiThresh)
{
    rxdma->iqLoThresh = loThresh;
    rxdma->iqHiThresh = hiThresh;
    rxdma->iqDropped  = 0;
    return 1;
}
#endif

#if defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)
int bcmPktDma_XtmSetRxChanBpmThresh_Iudma( BcmPktDma_LocalXtmRxDma * rxdma,
                                           uint16 allocTrig,
                                           uint16 bulkAlloc )
{
   rxdma->allocTrig = allocTrig;
   rxdma->bulkAlloc = bulkAlloc;

    return 1;
}


int bcmPktDma_XtmSetTxChanBpmThresh_Iudma( BcmPktDma_LocalXtmTxDma * txdma,
                                           uint16 loThresh,
                                           uint16 hiThresh,
                                           uint32 dmaType)
{
   txdma->ulLoThresh = loThresh;
   txdma->ulHiThresh = hiThresh;
   txdma->ulDropped  = 0;

    return 1;
}
#endif

int bcmPktDma_XtmSetTxChanDropAlg_Iudma( BcmPktDma_LocalXtmTxDma * txdma, int dropAlgorithm,
                                         int dropProbabilityLo, int minThresholdLo, int maxThresholdLo,
                                         int dropProbabilityHi, int minThresholdHi, int maxThresholdHi)
{
    /* WRED of Iudma version is not supported. */
    return 1;
}

void bcmPktDma_XtmGetStats_Iudma(uint8 vport, uint32 *rxDrop_p, uint32 *txDrop_p)
{
    *rxDrop_p = 0;
    *txDrop_p = 0;
}

void bcmPktDma_XtmResetStats_Iudma(uint8 vport)
{
    return;
}

EXPORT_SYMBOL(g_pXtmGlobalInfo);

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
EXPORT_SYMBOL(g_Xtm_rx_iudma_ownership);
EXPORT_SYMBOL(g_Xtm_tx_iudma_ownership);
#endif

EXPORT_SYMBOL(bcmPktDma_XtmInitRxChan_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmUnInitRxChan_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmInitTxChan_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmSelectRxIrq_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmClrRxIrq_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmRecv_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmXmitAvailable_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmXmit_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmTxEnable_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmTxDisable_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmRxEnable_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmRxDisable_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmForceFreeXmitBufGet_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmRecv_RingSize_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmAllocTxBds);
EXPORT_SYMBOL(bcmPktDma_XtmFreeTxBds);
EXPORT_SYMBOL(bcmPktDma_XtmAllocRxBds);

#if defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE)
EXPORT_SYMBOL(bcmPktDma_XtmSetIqThresh_Iudma);
#endif
#if defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)
EXPORT_SYMBOL(bcmPktDma_XtmSetRxChanBpmThresh_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmSetTxChanBpmThresh_Iudma);
#endif
EXPORT_SYMBOL(bcmPktDma_XtmSetTxChanDropAlg_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmGetStats_Iudma);
EXPORT_SYMBOL(bcmPktDma_XtmResetStats_Iudma);
#endif /* FAP_4KE */

