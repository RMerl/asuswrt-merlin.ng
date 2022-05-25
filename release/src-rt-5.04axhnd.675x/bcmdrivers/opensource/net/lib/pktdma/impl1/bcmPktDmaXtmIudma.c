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


#include <bcm_intr.h>


#include "bcmPktDma.h"

/* fap4ke_local redfines memset to the 4ke lib one - not what we want */
#if defined memset
#undef memset
#endif

/* Binding with XTMRT */
PBCMXTMRT_GLOBAL_INFO g_pXtmGlobalInfo = (PBCMXTMRT_GLOBAL_INFO)NULL;

//#ifndef FAP_4KE
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
            
            mdelay(100);  /* 100 msec */
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

        /* The delay only works when called from Linux */
        __udelay(20);
    }

    return 1;
}



/* Return non-aligned, cache-based pointer to caller - Apr 2010 */
DmaDesc *bcmPktDma_XtmAllocTxBds(int channel, int numBds, uint32 *phy_addr)
{
    /* Allocate space for pKeyPtr, pTxSource and pTxAddress as well as BDs. */
    int size = sizeof(DmaDesc) + sizeof(BcmPktDma_txRecycle_t);

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

