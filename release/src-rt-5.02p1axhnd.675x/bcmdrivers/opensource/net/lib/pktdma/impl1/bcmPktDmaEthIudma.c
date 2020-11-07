/*
 <:copyright-BRCM:2007:GPL/GPL:standard
 
    Copyright (c) 2007 Broadcom 
    All Rights Reserved
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License, version 2, as published by
 the Free Software Foundation (the "GPL").
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 
 A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA.
 
:>
*/
/*
 *******************************************************************************
 * File Name  : bcmPktDmaEthIudma.c
 *
 * Description: This file contains the Packet DMA Implementation for the iuDMA
 *              channels of the Ethernet Controller.
 * Note       : This bcmPktDma code is tied to impl3 of the Eth Driver
 *
 *******************************************************************************
 */

#include <bcm_intr.h>

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
#ifdef FAP_4KE
#include "Fap4keOsDeps.h"
#include "fap4ke_local.h"
#include "fap_task.h"
#include "fap_dqm.h"
#include "fap4ke_memory.h"
#include "fap4ke_packet.h"
#include "bcmenet.h"
#include "bcmPktDmaHooks.h"
#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
#include "bcmxtmrtimpl.h"
#endif
#else /* FAP_4KE */
#include "bcm_OS_Deps.h"
#include <linux/bcm_log.h>
#include "fap4ke_local.h"
#include "fap_task.h"
#include "fap_dqm.h"
#include "fap4ke_memory.h"
#include "bcmPktDmaHooks.h"
#endif /* FAP_4KE */
#endif /* defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE) */

#include "bcmPktDma.h"

/* Uncomment this define to turn on error checking and logging in the bcmPktDma driver */
//#define ENABLE_BCMPKTDMA_IUDMA_ERROR_CHECKING

/* fap4ke_local redfines memset to the 4ke lib one - not what we want */
#if defined memset
#undef memset
#endif

/* Globals initialized in the Enet Driver */
#ifndef FAP_4KE
BcmEnet_devctrl *   g_pEnetDevCtrl    = (BcmEnet_devctrl *) NULL;   /* Binding with Switch ENET */
#endif /* FAP_4KE */

//#ifndef FAP_4KE
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
const int g_Eth_rx_iudma_ownership[ENET_RX_CHANNELS_MAX] =
{
    PKTDMA_ETH_RX_OWNERSHIP   /* rx iudma channel ownership */
};

const int g_Eth_tx_iudma_ownership[ENET_TX_CHANNELS_MAX] =
{
    PKTDMA_ETH_TX_OWNERSHIP   /* tx iudma channel ownership */
};

#endif /* defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE) */
//#endif /* !FAP_4KE */

int bcmPktDma_EthInitRxChan_Iudma( uint32 bufDescrs,
                                   BcmPktDma_LocalEthRxDma *pEthRxDma)
{
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    /* NOTE: Only needed by the FAP */
    pEthRxDma->numRxBds = bufDescrs;
    pEthRxDma->rxAssignedBds = 0;
    pEthRxDma->rxHeadIndex = 0;
    pEthRxDma->rxTailIndex = 0;
    pEthRxDma->enetrxchannel_isr_enable = 1;  /* Must be worked out */

    /* Copy over pointers from passed in structure */
    // Already initialized
    //g_pEthRxDma[channel]->rxBds = (volatile DmaDesc *)(pEthRxDma->rxBds);
    //g_pEthRxDma[channel]->rxDma = (volatile DmaChannelCfg *)(pEthRxDma->rxDma);

    pEthRxDma->rxEnabled = 1;
#endif

    return 1;

}

int bcmPktDma_EthInitTxChan_Iudma( uint32 bufDescrs,
                                     BcmPktDma_LocalEthTxDma *pEthTxDma)
{
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    /* NOTE: Only needed by the FAP */
    pEthTxDma->txHeadIndex = 0;
    pEthTxDma->txTailIndex = 0;
    pEthTxDma->txEnabled = 1;

    /* Copy over pointers from Eth Driver */
    // Already initialized
    //pEthTxDma->txBds = pEthTxDma->txBds;
    //pEthTxDma->txDma = (volatile DmaChannelCfg *)pEthTxDma->txDma;
    //pEthTxDma->txRecycle = pEthTxDma->txRecycle;
#endif

    return 1;

}


#if defined(CONFIG_BCM_GMAC)
int bcmPktDma_EthUnInitRxChan_Iudma(BcmPktDma_LocalEthRxDma *pEthRxDma)
{
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    /* NOTE: Only needed by the FAP */
    pEthRxDma->numRxBds = 0;
    pEthRxDma->rxAssignedBds = 0;
    pEthRxDma->rxHeadIndex = 0;
    pEthRxDma->rxTailIndex = 0;
    pEthRxDma->enetrxchannel_isr_enable = 0;  /* Must be worked out */
#endif

    return 1;
}

int bcmPktDma_EthUnInitTxChan_Iudma(BcmPktDma_LocalEthTxDma *pEthTxDma)
{
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    /* NOTE: Only needed by the FAP */
    pEthTxDma->txHeadIndex = 0;
    pEthTxDma->txTailIndex = 0;
#endif

    return 1;
}
#endif



/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthSelectRxIrq
 Purpose: Return IRQ number to be used for bcmPkt Rx on a specific channel
-------------------------------------------------------------------------- */
int bcmPktDma_EthSelectRxIrq_Iudma(int channel)
{
#if defined(CONFIG_BCM_ENET_4908_GMAC)
    return INTERRUPT_ID_GMAC_DMA_0;
#else
    switch (channel)
    {
         case 0: return INTERRUPT_ID_ENETSW_RX_DMA_0; break;
         case 1: return INTERRUPT_ID_ENETSW_RX_DMA_1; break;
         case 2: return INTERRUPT_ID_ENETSW_RX_DMA_2; break;
         case 3: return INTERRUPT_ID_ENETSW_RX_DMA_3; break;
#if defined(CONFIG_BCM_GMAC)
         case 4: return INTERRUPT_ID_GMAC_DMA_0; break;
#endif
         default: return INTERRUPT_ID_ENETSW_RX_DMA_0; break;
    }
#endif
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthTxEnable_Iudma
 Purpose: Coordinate with FAP for tx enable
  Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */
int bcmPktDma_EthTxEnable_Iudma( BcmPktDma_LocalEthTxDma *  txdma )
{
    txdma->txEnabled = 1;
    return 1;
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthTxDisable_Iudma
 Purpose: Coordinate with FAP for tx disable
  Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */
 int bcmPktDma_EthTxDisable_Iudma( BcmPktDma_LocalEthTxDma *  txdma )
{
    txdma->txEnabled = 0;
#if defined(CONFIG_BCM947189)
    txdma->txDma->control &= ~DMA_EN;
#else
    txdma->txDma->cfg = 0;
#endif

    return 1;
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthRxEnable
 Purpose: Enable rx interrupts for the given channel.
  Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */
int bcmPktDma_EthRxEnable_Iudma( BcmPktDma_LocalEthRxDma * rxdma )
{
#if defined(CONFIG_BCM947189)
    rxdma->rxDma->control |= DMA_EN;
#else
    rxdma->rxDma->cfg |= DMA_ENABLE;
#endif
    rxdma->rxEnabled = 1;

    return 1;
}

/* --------------------------------------------------------------------------
    Name: bcmPktDma_EthRxDisable
 Purpose: Disable rx interrupts for the given channel
  Return: 1 on success; 0 otherwise
-------------------------------------------------------------------------- */
int bcmPktDma_EthRxDisable_Iudma( BcmPktDma_LocalEthRxDma * rxdma )
{
    int                       i;

    rxdma->rxEnabled = 0;

#if defined(CONFIG_BCM947189)
    rxdma->rxDma->control &= ~DMA_EN;
    for(i = 0; rxdma->rxDma->control & DMA_EN; i++)
    {
        rxdma->rxDma->control &= ~DMA_EN;

        if (i >= 100)
            break;
    }
#else
    rxdma->rxDma->cfg &= ~DMA_ENABLE;
    for(i = 0; rxdma->rxDma->cfg & DMA_ENABLE; i++)
    {
        rxdma->rxDma->cfg &= ~DMA_ENABLE;

        if (i >= 100)
            break;
#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
        /* The delay only works when called from Linux */
        udelay(20);
#endif
    }
#endif

  return 1;
}

#ifndef FAP_4KE
/* Return non-aligned, cache-based pointer to caller - Apr 2010 */
#if !( defined(CONFIG_ARM) || defined(CONFIG_ARM64) )
DmaDesc * bcmPktDma_EthAllocTxBds(int channel, int numBds)
#else
DmaDesc * bcmPktDma_EthAllocTxBds(struct device *dev, int channel, int numBds, uint32 *phy_addr)
#endif
{
    /* Allocate space for pKeyPtr, pTxSource and pTxAddress aa well as BDs. */
    int size = sizeof(DmaDesc) + sizeof(BcmPktDma_txRecycle_t);

#if defined(ENET_TX_BDS_IN_PSM) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
#if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
    if(g_Eth_tx_iudma_ownership[channel] == HOST_OWNED)
    {
        void * p;

        /* Tx Descriptors are allocated in DDR */
        /* Leave room for alignment by caller - Apr 2010 */
        if ((p = kmalloc(numBds * size + 0x10, GFP_ATOMIC))) {
            memset(p, 0, numBds * size + 0x10);
            cache_flush_len(p, numBds * size + 0x10);
        }
        return( (DmaDesc *)p );   /* tx bd ring + pKeyPtr, pTxSource and pTxAddress */
    }
    else
#endif
{
    uint8 * pMem;
    static uint8 * txBdAllocation[ENET_TX_CHANNELS_MAX] = {NULL};
    static int txNumBds[ENET_TX_CHANNELS_MAX] = {0};
    uint32 fapIdx;

    /* Restore previous BD allocation pointer if any */
    pMem = txBdAllocation[channel];

    if (pMem)
    {
        if(txNumBds[channel] != numBds)
        {
            printk("ERROR: Tried to allocate a different number of txBDs (was %d, attempted %d)\n",
                    txNumBds[channel], numBds);
            printk("       Eth tx BD allocation rejected!!\n");
            return( NULL );
        }
        memset(pMem, 0, numBds * size);
        return((DmaDesc *)pMem);   /* tx bd ring + pKeyPtr, pTxSource and pTxAddress */
    }

    /* Try to allocate Tx Descriptors in PSM. Use Host-side addressing here. */
    /* fapDrv_psmAlloc guarantees byte alignment. */

    /* channel represents iudma in this instance */
    fapIdx = getFapIdxFromEthTxIudma(channel);
    if ( !isValidFapIdx(fapIdx) )
    {
        printk("ERROR: bcmPktDma_psmAlloc for non-FAP channel (%d / %d)!!\n", channel, g_Eth_tx_iudma_ownership[channel]);
        return NULL;
    }

    pMem = bcmPktDma_psmAlloc(fapIdx, numBds * size);
    if(pMem != FAP4KE_OUT_OF_PSM)
    {
        memset(pMem, 0, numBds * size);
        txBdAllocation[channel] = pMem;
        txNumBds[channel] = numBds;
        return((DmaDesc *)pMem);   /* tx bd ring + pKeyPtr, pTxSource and pTxAddress */
    }

    printk("ERROR: Out of PSM. Eth tx BD allocation rejected!!\n");
    return( NULL );
}
#else  /* !defined(ENET_TX_BDS_IN_PSM) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)) */
{
#if !( defined(CONFIG_ARM) || defined(CONFIG_ARM64) )
    void * p;

    /* Allocate Tx Descriptors in DDR */
    /* Leave room for alignment by caller - Apr 2010 */
    if ((p = kmalloc(numBds * size + 0x10, GFP_KERNEL))) {
        memset(p, 0, numBds * size + 0x10);
        cache_flush_len(p, numBds * size + 0x10);
    }
    return( (DmaDesc *)p );   /* tx bd ring + pKeyPtr, pTxSource and pTxAddress */
#else
   dma_addr_t phys_addr;
   void * p;
   /* Override the size - only allocate DmaDesc as uncached */
    size = (uint32_t) (numBds * sizeof(DmaDesc) + BCM_DCACHE_LINE_LEN) ;
   /* memory allocation of NONCACHE_MALLOC for ARM is aligned to page size which is aligned to cache */
   if ((p = dma_alloc_coherent(dev, size, &phys_addr, GFP_KERNEL))) {
       memset(p, 0, size);
   }

   *phy_addr = (uint32_t) phys_addr ;
   return ( (DmaDesc *) p );   /* tx bd ring + pKeyPtr, pTxSource and pTxAddress */
#endif
}
#endif   /* defined(ENET_TX_BDS_IN_PSM) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)) */
}

/* Return non-aligned, cache-based pointer to caller - Apr 2010 */
#if !( defined(CONFIG_ARM) || defined(CONFIG_ARM64) )
DmaDesc * bcmPktDma_EthAllocRxBds(int channel, int numBds)
#else
DmaDesc * bcmPktDma_EthAllocRxBds(struct device *dev, int channel, int numBds, uint32 *phy_addr)
#endif
{
#if defined(ENET_RX_BDS_IN_PSM) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
{
    uint8 * pMem;
    static uint8 * rxBdAllocation[ENET_RX_CHANNELS_MAX] = {NULL};
    static int rxNumBds[ENET_RX_CHANNELS_MAX] = {0};
    uint32 fapIdx;

    /* Restore previous BD allocation pointer if any */
    pMem = rxBdAllocation[channel];

    if (pMem)
    {
        if(rxNumBds[channel] != numBds)
        {
            printk("ERROR: Tried to allocate a different number of rxBDs (was %d, attempted %d)\n",
                    rxNumBds[channel], numBds);
            printk("       Eth rx BD allocation rejected!!\n");
            return( NULL );
        }
        memset(pMem, 0, numBds * sizeof(DmaDesc));
        return((DmaDesc *)pMem);   /* rx bd ring */
    }

    /* Try to allocate Rx Descriptors in PSM. Use Host-side addressing here. */
    /* fapDrv_psmAlloc guarantees byte alignment. */
    /* channel is iudma in this instance */
    fapIdx = getFapIdxFromEthRxIudma(channel);
    if ( fapIdx == FAP_INVALID_IDX )
    {
        printk("ERROR: bcmPktDma_psmAlloc for non-FAP channel (%d / %d)!!\n", channel, g_Eth_tx_iudma_ownership[channel]);
        return NULL;
    }

    pMem = bcmPktDma_psmAlloc(fapIdx, numBds * sizeof(DmaDesc));
    if(pMem != FAP4KE_OUT_OF_PSM)
    {
        memset(pMem, 0, numBds * sizeof(DmaDesc));
        rxBdAllocation[channel] = pMem;
        rxNumBds[channel] = numBds;
        return((DmaDesc *)pMem);   /* rx bd ring */
    }

    printk("ERROR: Out of PSM. Eth rx BD allocation rejected!!\n");
    return( NULL );
}
#else   /* !defined(ENET_RX_BDS_IN_PSM) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)) */
{
#if !( defined(CONFIG_ARM) || defined(CONFIG_ARM64) )
    void * p;

    /* Allocate Rx Descriptors in DDR */
    /* Leave room for alignment by caller - Apr 2010 */
    if ((p = kmalloc(numBds * sizeof(DmaDesc) + 0x10, GFP_KERNEL))) {
        memset(p, 0, numBds * sizeof(DmaDesc) + 0x10);
        cache_flush_len(p, numBds * sizeof(DmaDesc) + 0x10);
    }
    return((DmaDesc *)p);   /* rx bd ring */
#else
   dma_addr_t phys_addr;
   uint32_t size32 = (uint32_t) (numBds * sizeof(DmaDesc) + BCM_DCACHE_LINE_LEN) ;
   /* memory allocation of NONCACHE_MALLOC for ARM is aligned to page size which is aligned to cache */
   void *p = dma_alloc_coherent(dev, size32, &phys_addr, GFP_KERNEL);

   if (p != NULL) {
      memset(p, 0, size32) ;
   }

   *phy_addr = (uint32_t) phys_addr ;
   return ( (DmaDesc *) p );   /* tx bd ring + pKeyPtr, pTxSource and pTxAddress */
#endif
}
#endif   /* defined(ENET_RX_BDS_IN_PSM) && (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)) */
}

/* Only called from the Host MIPS - Sept 2010 */
void bcmPktDma_BcmHalInterruptEnable(int channel, int irq)
{

#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
#if defined(CONFIG_BCM947189)
    volatile EnetCoreMisc *reg;
    if (channel == 0)
        reg = ENET_CORE0_MISC;
    else
        reg = ENET_CORE1_MISC;
    reg->intmask = I_RI;    
#else
    BcmHalInterruptEnable(irq);
#endif
#else
#endif
    return;
}

/* Only called from the Host MIPS - Sept 2010 */
void bcmPktDma_BcmHalInterruptDisable(int channel, int irq)
{

#if !(defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    BcmHalInterruptDisable(irq);
#else
#endif
    return;
}
 
#if defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE)
int bcmPktDma_EthSetIqThresh_Iudma( BcmPktDma_LocalEthRxDma * rxdma,
                                    uint16 loThresh,
                                    uint16 hiThresh)
{
    rxdma->iqLoThresh = loThresh;
    rxdma->iqHiThresh = hiThresh;
    rxdma->iqDropped  = 0;
    return 1;
}
#endif
int bcmPktDma_EthSetRxChanBpmThresh_dqm( BcmPktDma_LocalEthRxDma * rxdma,
                                         uint16 allocTrig,
                                         uint16 bulkAlloc);


#if defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)
int bcmPktDma_EthSetRxChanBpmThresh_Iudma( BcmPktDma_LocalEthRxDma * rxdma,
                                           uint16 allocTrig,
                                           uint16 bulkAlloc )
{
    rxdma->allocTrig = allocTrig;
    rxdma->bulkAlloc = bulkAlloc;

    return 1;
}

int bcmPktDma_EthSetTxChanBpmThresh_Iudma( BcmPktDma_LocalEthTxDma * txdma,
                                           uint16 *txDropThr )
{
    int q;

    for (q=0; q < ENET_TX_EGRESS_QUEUES_MAX; q++)
        txdma->txDropThr[q] = *(txDropThr + q);

    return 1;
}
#endif

void bcmPktDma_EthGetStats_Iudma(uint8 vport, uint32 *rxDrop_p, uint32 *txDrop_p)
{
    *rxDrop_p = 0;
    *txDrop_p = 0;
}

void bcmPktDma_EthResetStats_Iudma(uint8 vport)
{
    return;
}

EXPORT_SYMBOL(g_pEnetDevCtrl);

EXPORT_SYMBOL(bcmPktDma_EthInitRxChan_Iudma);
EXPORT_SYMBOL(bcmPktDma_EthInitTxChan_Iudma);

#if defined(CONFIG_BCM_GMAC)
EXPORT_SYMBOL(bcmPktDma_EthUnInitRxChan_Iudma);
EXPORT_SYMBOL(bcmPktDma_EthUnInitTxChan_Iudma);
#endif

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
EXPORT_SYMBOL(g_Eth_rx_iudma_ownership);
EXPORT_SYMBOL(g_Eth_tx_iudma_ownership);
#endif
EXPORT_SYMBOL(bcmPktDma_EthSelectRxIrq_Iudma);
EXPORT_SYMBOL(bcmPktDma_EthAllocTxBds);
EXPORT_SYMBOL(bcmPktDma_EthAllocRxBds);
EXPORT_SYMBOL(bcmPktDma_EthTxEnable_Iudma);
EXPORT_SYMBOL(bcmPktDma_EthTxDisable_Iudma);
EXPORT_SYMBOL(bcmPktDma_EthRxEnable_Iudma);
EXPORT_SYMBOL(bcmPktDma_EthRxDisable_Iudma);
EXPORT_SYMBOL(bcmPktDma_BcmHalInterruptEnable);
EXPORT_SYMBOL(bcmPktDma_BcmHalInterruptDisable);

#if defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE)
EXPORT_SYMBOL(bcmPktDma_EthSetIqThresh_Iudma);
#endif
#if defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)
EXPORT_SYMBOL(bcmPktDma_EthSetRxChanBpmThresh_Iudma);
EXPORT_SYMBOL(bcmPktDma_EthSetTxChanBpmThresh_Iudma);
#endif
EXPORT_SYMBOL(bcmPktDma_EthGetStats_Iudma);
EXPORT_SYMBOL(bcmPktDma_EthResetStats_Iudma);
#endif /* !FAP_4KE */

#if !defined(FAP_4KE) && !defined(CONFIG_BCM_ENET_4908_GMAC)
MODULE_LICENSE("GPL");
#endif
