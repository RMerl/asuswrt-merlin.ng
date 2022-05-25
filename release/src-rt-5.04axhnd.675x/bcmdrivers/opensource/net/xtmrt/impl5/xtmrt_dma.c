/*
<:copyright-BRCM:2011:DUAL/GPL:standard

   Copyright (c) 2011 Broadcom 
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

/* Includes. */
//#define DUMP_DATA

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/rtnetlink.h>
#include <linux/ethtool.h>
#include <linux/if_arp.h>
#include <linux/ppp_channel.h>
#include <linux/ppp_defs.h>
#include <linux/if_ppp.h>
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <linux/atmppp.h>
#include <linux/blog.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/ip.h>
#include <bcmtypes.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>
#include <board.h>
#include "bcmnet.h"
#include "bcm_mm.h"
#include "bcmxtmcfg.h"
#include "bcmxtmrt.h"
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/nbuff.h>
#include "bcmxtmrtimpl.h"
#include "bcmPktDma.h"
#include "xtmrt_dma.h"
#include <linux/bcm_log.h>


/* 32bit context is union of pointer to pdevCtrl and channel number */
#if (ENET_RX_CHANNELS_MAX > 4)
#error "Overlaying channel and pDevCtrl into context param needs rework"
#else
#define BUILD_CONTEXT(pGi,channel) \
            (UINT32)((UINT32)(pGi) | ((UINT32)(channel) & 0x3u))
#endif


/**** Externs ****/

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
extern UINT32             iqos_enable_g;
extern UINT32             iqos_cpu_cong_g;
extern iqos_status_hook_t iqos_xtm_status_hook_g;
#endif


extern int bcmxtmrt_in_init_dev;


/**** Globals ****/



/**** Prototypes ****/

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
static void bcmxtmrt_timer(PBCMXTMRT_GLOBAL_INFO pGi);
#else
static void bcmxtmrt_timer(struct timer_list *timer);
#endif
static inline void AssignRxBuffer(int channel, UINT8 *pData);

static inline void _bcmxtmrt_recycle_fkb(struct fkbuff *pFkb, UINT32 context);


static int ProcDmaTxInfo(struct file *file, char __user *buf,
                        size_t len, loff_t *offset);
static int ProcDmaRxInfo(struct file *file, char __user *buf,
                        size_t len, loff_t *offset);
                         

static struct file_operations dma_tx_info_proc = {
        .read = ProcDmaTxInfo,
};
static struct file_operations dma_rx_info_proc = {
        .read = ProcDmaRxInfo,
};
static struct file_operations tx_bond_info_proc = {
        .read = ProcTxBondInfo,
};

static void freeXmitPkts(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                         volatile BcmPktDma_XtmTxDma *txdma,
                         int forceFree);
static FN_HANDLER_RT bcmxtmrt_rxisr(int nIrq, void *pRxDma);
//static void xtmDmaStatus(int channel, BcmPktDma_XtmTxDma *txdma);


/**** Statics ****/



/*---------------------------------------------------------------------------
 * int bcmxapi_module_init(void)
 * Description:
 *    Called when the driver is loaded.
 * Returns:
 *    0 or error status
 *---------------------------------------------------------------------------
 */
int bcmxapi_module_init(void)
{
   g_pXtmGlobalInfo = &g_GlobalInfo;

   /* create a XTM slab cache for SKBs */
   g_GlobalInfo.xtmSkbCache = kmem_cache_create("bcm_XtmSkbCache",
                                                BCM_SKB_ALIGNED_SIZE,
                                                0, /* align */
                                                SLAB_HWCACHE_ALIGN, /* flags */
                                                NULL); /* ctor */
   if (g_GlobalInfo.xtmSkbCache == NULL)
   {
      BCM_XTM_ERROR("XTM: Unable to create skb cache");
      return -ENOMEM;
   }


#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    /* GBPM status and threshold dump handler hooks */
   gbpm_g.xtm_status = xtm_bpm_status;
   gbpm_g.xtm_thresh = xtm_bpm_dump_txq_thresh;
#endif

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
   iqos_xtm_status_hook_g = xtm_iq_status;
#endif

   return 0;

}  /* bcmxapi_module_init() */


/*---------------------------------------------------------------------------
 * void bcmxapi_module_cleanup(void)
 * Description:
 *    Called when the driver is unloaded.
 * Returns: void
 *---------------------------------------------------------------------------
 */
void bcmxapi_module_cleanup(void)
{
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
   iqos_xtm_status_hook_g = NULL;
#endif

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    /* GBPM status and threshold dump handler hooks */
   gbpm_g.xtm_status = (gbpm_xtm_status_hook_t)NULL;
   gbpm_g.xtm_thresh = (gbpm_xtm_thresh_hook_t)NULL;
#endif

   /* destroy the XTM slab cache for SKBs */
   kmem_cache_destroy(g_GlobalInfo.xtmSkbCache); 

}  /* bcmxapi_module_cleanup() */


/*---------------------------------------------------------------------------
 * int bcmxapi_enable_rx_interrupt(void)
 * Description:
 *
 * Returns:
 *    0 or error code
 *---------------------------------------------------------------------------
 */
int bcmxapi_enable_rx_interrupt(void)
{
   int i;
   BcmXtm_RxDma *rxdma;

   for (i = 0; i < MAX_RECEIVE_QUEUES; i++)
   {
      rxdma = g_GlobalInfo.rxdma[i];
      
      if (rxdma->pktDmaRxInfo.rxBds)
      {
         /* set rxdma channel interrupt mask */
         rxdma->pktDmaRxInfo.rxDma->intMask =
                              DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
         /* enable rxdma channel interrupt */
         bcmPktDma_XtmRxEnable(&rxdma->pktDmaRxInfo);
#if !defined(CONFIG_ARM)
         /* enable rxdma channel hardware interrupt */
         BcmHalInterruptEnable(SAR_RX_INT_ID_BASE + i);
#endif
      }
   }
   return 0;
   
}  /* bcmxapi_enable_rx_interrupt() */


/*---------------------------------------------------------------------------
 * int bcmxapi_disable_rx_interrupt(void)
 * Description:
 *
 * Returns:
 *    0 or error code
 *---------------------------------------------------------------------------
 */
int bcmxapi_disable_rx_interrupt(void)
{
   int i;
   BcmXtm_RxDma *rxdma;
   
   for (i = 0; i < MAX_RECEIVE_QUEUES; i++)
   {
      rxdma = g_GlobalInfo.rxdma[i];
      if (rxdma->pktDmaRxInfo.rxBds)
      {
         bcmPktDma_XtmRxDisable(&rxdma->pktDmaRxInfo);
#if !defined(CONFIG_ARM)
         BcmHalInterruptDisable(SAR_RX_INT_ID_BASE + i);
#endif
      }
   }
   return 0;
   
}  /* bcmxapi_disable_rx_interrupt() */


/*---------------------------------------------------------------------------
 * void AssignRxBuffer(int channel, UINT8 *pData)
 * Description:
 *    Put a data buffer back on to the receive BD ring.
 * Returns: void
 *---------------------------------------------------------------------------
 */
static inline void AssignRxBuffer(int channel, UINT8 *pData)
{
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   BcmXtm_RxDma *rxdma = pGi->rxdma[channel];
   BcmPktDma_XtmRxDma *pktDmaRxInfo_p = &rxdma->pktDmaRxInfo;

#if defined (CONFIG_BCM963138) || defined (CONFIG_BCM963148)
   unsigned long irqFlags;

   spin_lock_irqsave(&pGi->xtmlock_rx, irqFlags);
#else
   spin_lock_bh(&pGi->xtmlock_rx);
#endif

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
   if (pktDmaRxInfo_p->numRxBds - pktDmaRxInfo_p->rxAssignedBds)
   {
      /* Free the data buffer to a specific Rx Queue (ie channel) */
      bcmPktDma_XtmFreeRecvBuf(pktDmaRxInfo_p, (UINT8 *)pData);
   }
   else
   {
      xtm_bpm_free_buf(rxdma, pData);
   }

#else
   /* Free the data buffer to a specific Rx Queue (ie channel) */
   bcmPktDma_XtmFreeRecvBuf(pktDmaRxInfo_p, (UINT8 *)pData);
#endif

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
   /* Update congestion status, once all the buffers have been recycled. */
   if (iqos_cpu_cong_g)
   {
      if (pktDmaRxInfo_p->numRxBds == pktDmaRxInfo_p->rxAssignedBds)
         iqos_set_cong_status(IQOS_IF_XTM, channel, IQOS_CONG_STATUS_LO);
   }
#endif


#if defined (CONFIG_BCM963138) || defined (CONFIG_BCM963148)
   spin_unlock_irqrestore(&pGi->xtmlock_rx, irqFlags);
#else
   spin_unlock_bh(&pGi->xtmlock_rx);
#endif   
   
}  /* AssignRxBuffer() */


/*---------------------------------------------------------------------------
 * void FlushAssignRxBuffer(int channel, UINT8 *pData, UINT8 *pEnd)
 * Description:
 *    Flush then assign RxBdInfo to the receive BD ring.
 * Returns: void
 *---------------------------------------------------------------------------
 */
void FlushAssignRxBuffer(int channel, UINT8 *pData, UINT8 *pEnd)
{
   cache_invalidate_region(pData, pEnd);
   AssignRxBuffer(channel, pData);
   
}  /* FlushAssignRxBuffer() */


/*---------------------------------------------------------------------------
 * void _bcmxtmrt_recycle_fkb(struct fkbuff *pFkb, UINT32 context)
 * Description:
 *    Put fkb buffer back on to the BD ring.
 * Returns: void
 *---------------------------------------------------------------------------
 */
static inline void _bcmxtmrt_recycle_fkb(struct fkbuff *pFkb, UINT32 context)
{
   UINT8 *pData = (UINT8 *)PFKBUFF_TO_PDATA(pFkb, BCM_PKT_HEADROOM);
   int channel  = FKB_RECYCLE_CONTEXT(pFkb)->channel;

   AssignRxBuffer(channel, pData); /* No cache flush */
    
}  /* _bcmxtmrt_recycle_fkb() */


/*---------------------------------------------------------------------------
 * void bcmxtmrt_recycle_skb_or_data(struct sk_buff *skb, UINT32 context,
 *                                   UINT32 nFlag)
 * Description:
 *    Put socket buffer header back onto the free list or a data
 *    buffer back on to the BD ring.
 * Returns: void
 *---------------------------------------------------------------------------
 */
void bcmxtmrt_recycle_skb_or_data(struct sk_buff *skb, UINT32 context,
                                  UINT32 nFlag)
{
   int channel = RECYCLE_CONTEXT(context)->channel;

   if (nFlag & SKB_RECYCLE)
   {
      xtm_skb_free(g_GlobalInfo.rxdma[channel], skb);
   }
   else
   {
      UINT8 *pData, *pEnd;

      pData = skb->head + BCM_PKT_HEADROOM;
#ifdef XTM_CACHE_SMARTFLUSH
      pEnd = (UINT8 *)(skb_shinfo(skb)) + sizeof(struct skb_shared_info);
#else
      pEnd = pData + BCM_MAX_PKT_LEN;
#endif
      FlushAssignRxBuffer(channel, pData, pEnd);
   }
}  /* bcmxtmrt_recycle_skb_or_data() */


/*---------------------------------------------------------------------------
 * void bcmxtmrt_recycle(pNBuff_t pNBuf, UINT32 context, UINT32 flags)
 * Description:
 *    Recycle a fkb or skb or skb->data.
 * Returns: void
 *---------------------------------------------------------------------------
 */
void bcmxtmrt_recycle(pNBuff_t pNBuf, UINT32 context, UINT32 flags)
{
   if (IS_FKBUFF_PTR(pNBuf))
      _bcmxtmrt_recycle_fkb( PNBUFF_2_FKBUFF(pNBuf), context);
   else
      bcmxtmrt_recycle_skb_or_data( PNBUFF_2_SKBUFF(pNBuf), context, flags);
      
}  /* bcmxtmrt_recycle() */


/*---------------------------------------------------------------------------
 * FN_HANDLER_RT bcmxtmrt_rxisr(int nIrq, void *pRxDma)
 * Description:
 *    Hardware interrupt that is called when a packet is received
 *    on one of the receive queues.
 * Returns:
 *    IRQ_HANDLED
 *---------------------------------------------------------------------------
 */
static FN_HANDLER_RT bcmxtmrt_rxisr(int nIrq, void *pRxDma)
{
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   PBCMXTMRT_DEV_CONTEXT pDevCtx;
   UINT32 i;
   UINT32 scheduled = 0;
   int    channel;
   BcmXtm_RxDma *rxdma;

   channel = CONTEXT_TO_CHANNEL((UINT32)pRxDma);
   rxdma   = pGi->rxdma[channel];

   spin_lock(&pGi->xtmlock_rx_regs);

   for (i = 0; i < MAX_DEV_CTXS; i++)
   {
      if ((pDevCtx = pGi->pDevCtxs[i]) != NULL &&
         pDevCtx->ulOpenState == XTMRT_DEV_OPENED)
      {
         /* set flag to reenable rxdma channel hardware interrupt
          * when RX processing completes.
          */
         pGi->ulIntEnableMask |= 1 << channel;
#if defined(CONFIG_ARM)
         /* Mask off the interrupt */
         rxdma->pktDmaRxInfo.rxDma->intMask = 0;

         /* clear rxdma channel interrupt status */
         bcmPktDma_XtmClrRxIrq_Iudma(&rxdma->pktDmaRxInfo);

         /* Device is open.  Start polling for packets. */
         BCMXTMRT_WAKEUP_RXWORKER(pGi);
#else
         /* Device is open.  Start polling for packets. */
         BCMXTMRT_WAKEUP_RXWORKER(pGi);
         /* clear rxdma channel interrupt status */
         bcmPktDma_XtmClrRxIrq_Iudma(&rxdma->pktDmaRxInfo);
#endif
         
         scheduled = 1;
      }
   }

   if (scheduled == 0 && pGi->ulDrvState == XTMRT_RUNNING)
   {
      /* Device is not open */
      
      /* clear rxdma channel interrupt status */
      bcmPktDma_XtmClrRxIrq_Iudma(&rxdma->pktDmaRxInfo);
#if !defined(CONFIG_ARM)
      /* enable rxdma channel hardware interrupt */
      BcmHalInterruptEnable(SAR_RX_INT_ID_BASE + channel);
#endif
   }

   spin_unlock(&pGi->xtmlock_rx_regs);

   return (IRQ_HANDLED);
    
}  /* bcmxtmrt_rxisr() */


/*---------------------------------------------------------------------------
 * UINT32 bcmxapi_rxtask(UINT32 ulBudget, UINT32 *pulMoreToDo)
 * Description:
 *    xtm receive task called RX thread.
 * Returns:
 *    0 - success, Error code - failure
 *---------------------------------------------------------------------------
 */
UINT32 bcmxapi_rxtask(UINT32 ulBudget, UINT32 *pulMoreToDo)
{
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   PBCMXTMRT_DEV_CONTEXT pDevCtx = NULL;
   BcmXtm_RxDma   *rxdma;
   DmaDesc        dmaDesc;
   UINT8          *pBuf;
   struct sk_buff *skb;
   int    len;
   int    i;
   UINT32 ulCell;
   UINT32 ulVcId;
   UINT32 ulMoreToReceive;
   UINT32 ulRxPktGood = 0;
   UINT32 ulRxPktProcessed = 0;
   UINT32 ulRxPktMax = ulBudget + (ulBudget / 2);
#if defined (CONFIG_BCM963138) || defined (CONFIG_BCM963148)
   UINT32 irqFlags;
#endif   

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
   if (iqos_enable_g)
   {
      for (i = 0; i < MAX_RECEIVE_QUEUES; i++)
         xtm_iq_update_cong_status(i);
   }
#endif

   /* Receive packets from every receive queue in a round robin order until
    * there are no more packets to receive.
    */
   do
   {
      ulMoreToReceive = 0;
      
      /* In case of FAP, we are checking DQMs and not DMAs.
         Checking only one time should be sufficient, as the DQM delivers messages
         for both low/high prios from either of the channels.
         + Rx channel 1 is not used
       */
      for (i = 0; i < MAX_RECEIVE_QUEUES; i++)
      {
         if (ulBudget == 0)
         {
            *pulMoreToDo = 1;
            break;
         }

         rxdma = pGi->rxdma[i];

#if defined (CONFIG_BCM963138) || defined (CONFIG_BCM963148)
         spin_lock_irqsave(&pGi->xtmlock_rx, irqFlags);
#else
         spin_lock_bh(&pGi->xtmlock_rx);
#endif         

         dmaDesc.word0 = bcmPktDma_XtmRecv(&rxdma->pktDmaRxInfo, &pBuf, &len);

#if defined (CONFIG_BCM963138) || defined (CONFIG_BCM963148)
         spin_unlock_irqrestore(&pGi->xtmlock_rx, irqFlags);
#else
         spin_unlock_bh(&pGi->xtmlock_rx);
#endif

         if (dmaDesc.status & DMA_OWN)
         {
            ulRxPktGood |= XTM_POLL_DONE;
            continue;   /* next RxBdInfos */
         }

         ulRxPktProcessed++;
         ulVcId = ((dmaDesc.status>>FSTAT_MATCH_ID_SHIFT) & FSTAT_MATCH_ID_MASK) ;
         pDevCtx = pGi->pDevCtxsByMatchId[ulVcId] ;
         
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
         /* alloc a new buf from bpm */
         {
            if (xtm_bpm_alloc_buf(rxdma) == GBPM_ERROR)
            {
               FlushAssignRxBuffer(rxdma->pktDmaRxInfo.channel, pBuf, pBuf);
               if (pDevCtx)
                  pDevCtx->DevStats.rx_errors++;
               goto drop_pkt;
            }
         }
#endif

#if defined(CONFIG_BCM963138) || defined(COFNIG_BCM963148)
         cache_invalidate_len_outer_first(pBuf, len);
#endif

         ulCell = (dmaDesc.status & FSTAT_PACKET_CELL_MASK) == FSTAT_CELL;

         /* error status, or packet with no pDev */
         if (((dmaDesc.status & FSTAT_ERROR) != 0) ||
             ((dmaDesc.status & (DMA_SOP|DMA_EOP)) != (DMA_SOP|DMA_EOP)) ||
             ((!ulCell) && (pDevCtx == NULL)))   /* packet */
         {
            if (ulVcId == TEQ_DATA_VCID && pGi->pTeqNetDev)
            {
               UINT32 recycle_context = 0;
               unsigned long flags;

#ifdef XTM_CACHE_SMARTFLUSH
               len = dmaDesc.length + SAR_DMA_MAX_BURST_LENGTH;
#else
               len = BCM_MAX_PKT_LEN;
#endif

               skb = xtm_skb_alloc(rxdma);
               if (!skb)
               {
                  FlushAssignRxBuffer(rxdma->pktDmaRxInfo.channel, pBuf, pBuf);
                  if (pDevCtx)
                  {
                     pDevCtx->DevStats.rx_dropped++;
                  }
                  goto drop_pkt;
               }

               RECYCLE_CONTEXT(recycle_context)->channel = rxdma->pktDmaRxInfo.channel;

               skb_headerinit(BCM_PKT_HEADROOM, len, skb, pBuf,
                  (RecycleFuncP)bcmxtmrt_recycle_skb_or_data, recycle_context, NULL);
               __skb_trim(skb, dmaDesc.length);

               /* Sending TEQ data to interface told to us by DSL Diags */
               skb->dev      = pGi->pTeqNetDev;
               skb->protocol = htons(ETH_P_802_3);
               local_irq_save(flags);
               local_irq_enable();
               dev_queue_xmit(skb);
               local_irq_restore(flags);
            }
            else
            {
               DUMP_PKT(pBuf, dmaDesc.length);
               AssignRxBuffer(i, pBuf);
               if (pDevCtx)
                  pDevCtx->DevStats.rx_errors++;
            }
         }
         else if (!ulCell) /* process packet, pDev != NULL */
         {
            /* pBuf is just a pointer to the rx iudma buffer. */
            bcmxtmrt_processRxPkt(pDevCtx, rxdma, pBuf,
                                  dmaDesc.status, dmaDesc.length, 0, 0/*is_mcast_fwd_exp*/);
            
            ulRxPktGood++;
            ulBudget--;
         }
         else                /* process cell */
         {
            bcmxtmrt_processRxCell(pBuf);
            
            /* Put the buffer back onto the BD ring. */
            FlushAssignRxBuffer(rxdma->pktDmaRxInfo.channel,
                                pBuf, pBuf + BCM_MAX_PKT_LEN);
         }
drop_pkt:
         if (ulRxPktProcessed >= ulRxPktMax)
            break;
         else
            ulMoreToReceive = 1; /* more packets to receive on Rx queue? */

      } /* For loop */

   } while (ulMoreToReceive);

   return (ulRxPktGood);

}  /* bcmxtmrt_rxtask() */


/*---------------------------------------------------------------------------
 * int bcmxapi_add_proc_files(void)
 * Description:
 *    Adds proc file system directories and entries.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
int bcmxapi_add_proc_files(struct proc_dir_entry *dir)
{

   struct proc_dir_entry *entry;

   entry = proc_create("txdmainfo", S_IRUGO, dir,
                                &dma_tx_info_proc);
   if (!entry) {
      pr_err("%s: could not create proc entry for txdmainfo",
                __func__);
      return -1;
   }
   entry = proc_create("rxdmainfo", S_IRUGO, dir,
                                &dma_rx_info_proc);
   if (!entry) {
      pr_err("%s: could not create proc entry for rxdmainfo",
                __func__);
      return -1;
   }
   entry = proc_create("txbondinfo", S_IRUGO, dir,
                                &tx_bond_info_proc);
   if (!entry) {
      pr_err("%s: could not create proc entry for txbondinfo",
                __func__);
      return -1;
   }


   return 0;
    
}  /* bcmxapi_add_proc_files() */


/*---------------------------------------------------------------------------
 * int bcmxapi_del_proc_files(void)
 * Description:
 *    Deletes proc file system directories and entries.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
int bcmxapi_del_proc_files(struct proc_dir_entry *dir)
{
   remove_proc_entry("driver/xtm/txschedinfo", NULL);
   remove_proc_entry("driver/xtm/txbondinfo", NULL);
   remove_proc_entry("driver/xtm/txdmainfo", NULL);
   return 0;
    
}  /* bcmxapi_del_proc_files() */


static inline int __get_dma_tx_info(char *page)
{
   PBCMXTMRT_GLOBAL_INFO  pGi = &g_GlobalInfo;
   BcmPktDma_XtmTxDma    *txdma;
   volatile DmaStateRam  *pStRam = (DmaStateRam *)&pGi->dmaCtrl->stram.s;
   UINT32 i;
   int sz = 0;

   for (i = 0; i < MAX_DEV_CTXS; i++)
   {
      UINT32 j ;
      PBCMXTMRT_DEV_CONTEXT  pDevCtx;
      pDevCtx = pGi->pDevCtxs[i];
      if (pDevCtx != (PBCMXTMRT_DEV_CONTEXT) NULL)
      {
         for (j = 0; j < pDevCtx->ulTxQInfosSize; j++)
         {
            txdma = pDevCtx->txdma[j];

            sz += sprintf(page + sz,
                          "Ch %u, NumTxBds: %u, HeadIdx: %u, TailIdx: %u, FreeBds: %u\n",
                          txdma->ulDmaIndex, txdma->ulQueueSize,
                          (UINT32)txdma->txHeadIndex, (UINT32)txdma->txTailIndex,
                          (UINT32)txdma->txFreeBds);

            sz += sprintf(page + sz,
                          "BD RingOffset: 0x%08x, Word1: 0x%08x",                  
                          pStRam[SAR_TX_DMA_BASE_CHAN + txdma->ulDmaIndex].state_data & 0x1fff,
                          pStRam[SAR_TX_DMA_BASE_CHAN + txdma->ulDmaIndex].desc_len_status);

#if !(defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
            sz += sprintf(page + sz,
                          "%s tx_chan_size: %u, tx_chan_filled: %u",
                          pDevCtx->pDev->name, txdma->ulQueueSize,
                          txdma->ulNumTxBufsQdOne);
#endif                      
         }
      }
   }

#if !(defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
   sz += sprintf(page + sz, "ext_buf_size: %u, reserve_buf_size: %u, tx_total_filled: %u\n\n",
                 pGi->ulNumExtBufs, pGi->ulNumExtBufsRsrvd, pGi->ulNumTxBufsQdAll);

   sz += sprintf(page + sz,
                 "queue_condition: %u %u %u, drop_condition: %u %u %u\n",
                 pGi->ulDbgQ1, pGi->ulDbgQ2, pGi->ulDbgQ3,
                 pGi->ulDbgD1, pGi->ulDbgD2, pGi->ulDbgD3);
#endif

   return sz;

}
static inline int __get_dma_rx_info(char *page)
{
    PBCMXTMRT_GLOBAL_INFO  pGi = &g_GlobalInfo;
    BcmPktDma_LocalXtmRxDma *rxdma;
    UINT32 i;
    int sz = 0;

    /* Process all RX queues */
    for (i = 0; i < MAX_RECEIVE_QUEUES; i++)
    {
       /* Use global pointer to get FAP index via RXDMA structure */
       rxdma = (BcmPktDma_LocalXtmRxDma *)&pGi->rxdma[i]->pktDmaRxInfo;


       /* On FAP platforms, we need to get data updated by FAP (referenced in PSM structure) */

       /* Good RXDA pointer? */
       if (rxdma != NULL)
       {
          sz += sprintf(page + sz,
                        "Ch %u, NumRxBds: %u, HeadIdx: %u, TailIdx: %u, AssignedBds: %u\n",
                        i, (UINT32)rxdma->numRxBds, (UINT32)rxdma->rxHeadIndex,
                        (UINT32)rxdma->rxTailIndex, (UINT32)rxdma->rxAssignedBds);

          sz += sprintf(page + sz,
                        "DMA cfg: 0x%08x, intstat: 0x%08x, intmask: 0x%08x",
                        rxdma->rxDma->cfg, rxdma->rxDma->intStat, rxdma->rxDma->intMask);
       }
    }

   return sz;

}

/*---------------------------------------------------------------------------
 * int ProcDmaTxInfo(char *page, char **start, off_t off, int cnt,
 *                   int *eof, void *data)
 * Description:
 *    Displays information about transmit DMA channels for all
 *    network interfaces.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int ProcDmaTxInfo(struct file *file, char __user *buf,
                        size_t len, loff_t *offset)
{
int sz=0;
   if(*offset == 0)
   { 
      *offset=sz=__get_dma_tx_info(buf);
   }
   return sz;
}
/*---------------------------------------------------------------------------
 * int ProcDmaRxInfo(char *page, char **start, off_t off, int cnt,
 *                   int *eof, void *data)
 * Description:
 *    Displays information about receive DMA channels for all
 *    network interfaces.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static int ProcDmaRxInfo(struct file *file, char __user *buf,
                        size_t len, loff_t *offset)
{
int sz=0;
   if(*offset == 0)
   {
      *offset=sz=__get_dma_rx_info(buf);
   }
   return sz;
}


/*---------------------------------------------------------------------------
 * Function Name: bcmxtmrt_timer
 * Description:
 *    Periodic timer that calls the send function to free packets
 *    that have been transmitted.
 * Returns: void
 *---------------------------------------------------------------------------
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
static void bcmxtmrt_timer(PBCMXTMRT_GLOBAL_INFO pGi)
#else
static void bcmxtmrt_timer(struct timer_list *timer)
#endif
{
   UINT32 i;
   UINT32 ulHdrTypeIsPtm = FALSE;
   PBCMXTMRT_DEV_CONTEXT pDevCtx;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
   PBCMXTMRT_GLOBAL_INFO pGi;
   pGi = from_timer(pGi, timer, Timer); 
#endif

   /* Free transmitted buffers. */
   for (i = 0; i < MAX_DEV_CTXS; i++)
   {
      if ((pDevCtx = pGi->pDevCtxs[i]))
      {
         if (pDevCtx->ulTxQInfosSize)
         {
            bcmxtmrt_xmit(PNBUFF_NULL, pGi->pDevCtxs[i]->pDev);
         }
         if (pGi->pDevCtxs[i]->ulHdrType == HT_PTM)
            ulHdrTypeIsPtm = TRUE ;
      }
   }

   if (pGi->pTeqNetDev && ((void *)pGi->ulDevCtxMask == NULL))
   {
      UINT32 ulNotUsed;
      bcmxapi_rxtask( 100, &ulNotUsed );
   }

   /* Restart the timer. */
   pGi->Timer.expires = jiffies + SAR_TIMEOUT;
   add_timer(&pGi->Timer);


}  /* bcmxtmrt_timer() */


/*---------------------------------------------------------------------------
 * int bcmxapi_DoGlobInitReq(PXTMRT_GLOBAL_INIT_PARMS pGip)
 * Description:
 *    Processes an XTMRT_CMD_GLOBAL_INITIALIZATION command.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
int bcmxapi_DoGlobInitReq(PXTMRT_GLOBAL_INIT_PARMS pGip)
{
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   UINT32 i, j = 0 ;
#if !(defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
   UINT32 k, ulSize;
#endif

   BcmXtm_RxDma         *rxdma;
   volatile DmaStateRam *StateRam;

   int nr_tx_bds = bcmPktDma_XtmGetTxBds(0);

   /* Allocate original number of xtm tx BDs in non-FAP builds. BDs are stored in DDR - Apr 2010 */
   g_GlobalInfo.ulNumExtBufs = nr_tx_bds;

   g_GlobalInfo.ulNumExtBufsRsrvd = g_GlobalInfo.ulNumExtBufs / 5;
   g_GlobalInfo.ulNumExtBufs90Pct = (g_GlobalInfo.ulNumExtBufs * 9) / 10;
   g_GlobalInfo.ulNumExtBufs50Pct = g_GlobalInfo.ulNumExtBufs / 2;


   if (pGi->ulDrvState != XTMRT_UNINITIALIZED)
   {
      BCM_XTM_ERROR("Driver not initialized");
      return -EPERM;
   }

   bcmxtmrt_in_init_dev = 1;

   bcmLog_setLogLevel(BCM_LOG_ID_XTM, BCM_LOG_LEVEL_ERROR);

   spin_lock_init(&pGi->xtmlock_tx);
   spin_lock_init(&pGi->xtmlock_rx);
   spin_lock_init(&pGi->xtmlock_rx_regs);

   /* Save MIB counter/Cam registers. */
   pGi->pulMibTxOctetCountBase = pGip->pulMibTxOctetCountBase;
   pGi->ulMibRxClrOnRead       = pGip->ulMibRxClrOnRead;
   pGi->pulMibRxCtrl           = pGip->pulMibRxCtrl;
   pGi->pulMibRxMatch          = pGip->pulMibRxMatch;
   pGi->pulMibRxOctetCount     = pGip->pulMibRxOctetCount;
   pGi->pulMibRxPacketCount    = pGip->pulMibRxPacketCount;
   pGi->pulRxCamBase           = pGip->pulRxCamBase;

   /* allocate rxdma channel structures */
   for (i = 0; i < MAX_RECEIVE_QUEUES; i++)
   {
      pGi->rxdma[i] = (BcmXtm_RxDma *)kzalloc(sizeof(BcmXtm_RxDma), GFP_ATOMIC);

      if (pGi->rxdma[i] == NULL)
      {
         BCM_XTM_ERROR("Unable to allocate memory for rx dma channel structs");
         for (j = 0; j < i; j++)
            kfree(pGi->rxdma[j]);
         return -ENXIO;
      }
      pGi->rxdma[i]->pktDmaRxInfo.channel = i;
   }

   /* alloc space for the rx buffer descriptors */
   for (i = 0; i < MAX_RECEIVE_QUEUES; i++)
   {
      uint32 phy_addr ;
      rxdma = pGi->rxdma[i];

      pGip->ulReceiveQueueSizes[i] = bcmPktDma_XtmGetRxBds(i);
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
      xtm_rx_init_iq_thresh(i);
#endif

      if (pGip->ulReceiveQueueSizes[i] == 0) continue;

      rxdma->pktDmaRxInfo.rxBdsBase = bcmPktDma_XtmAllocRxBds(i, pGip->ulReceiveQueueSizes[i], &phy_addr);
#if defined(CONFIG_ARM)
      rxdma->pktDmaRxInfo.rxBdsPhysBase = (volatile DmaDesc *) phy_addr ;
#endif

      if (rxdma->pktDmaRxInfo.rxBdsBase == NULL)
      {
         BCM_XTM_ERROR("Unable to allocate memory for Rx Descriptors ");
         for (j = 0; j < MAX_RECEIVE_QUEUES; j++)
         {
#if !defined(XTM_RX_BDS_IN_PSM)
#if !defined(CONFIG_ARM)
            if (pGi->rxdma[j]->pktDmaRxInfo.rxBdsBase)
               kfree((void *)pGi->rxdma[j]->pktDmaRxInfo.rxBdsBase);
#else
            if (pGi->rxdma[j]->pktDmaRxInfo.rxBdsBase) {
               uint32_t size32 = (uint32_t) (pGip->ulReceiveQueueSizes[j] * sizeof(DmaDesc) + 0x10) ;
               void *mem = (void *) pGi->rxdma[j]->pktDmaRxInfo.rxBdsBase ;
               void *physMem = (void *) pGi->rxdma[j]->pktDmaRxInfo.rxBdsPhysBase ;
               NONCACHED_FREE (size32, mem, (dma_addr_t)physMem) ;
            }
#endif
#endif
            kfree(pGi->rxdma[j]);
         }
         return -ENOMEM;
      }
#if defined(XTM_RX_BDS_IN_PSM)
      rxdma->pktDmaRxInfo.rxBds = rxdma->pktDmaRxInfo.rxBdsBase;
#else
      /* Align rx BDs on 16-byte boundary - Apr 2010 */
      rxdma->pktDmaRxInfo.rxBds = (volatile DmaDesc *)(((int)rxdma->pktDmaRxInfo.rxBdsBase + 0xF) & ~0xF);
      rxdma->pktDmaRxInfo.rxBds = (volatile DmaDesc *)CACHE_TO_NONCACHE(rxdma->pktDmaRxInfo.rxBds);
#endif

      rxdma->pktDmaRxInfo.numRxBds = pGip->ulReceiveQueueSizes[i];

      BCM_XTM_NOTICE("XTM Init: Ch:%d - %d rx BDs at 0x%x", rxdma->pktDmaRxInfo.channel, rxdma->pktDmaRxInfo.numRxBds,
             (unsigned int)rxdma->pktDmaRxInfo.rxBds);

      rxdma->rxIrq = bcmPktDma_XtmSelectRxIrq(i);
      rxdma->channel = i;
   }

   /*
    * clear RXDMA state RAM
    */
   pGi->dmaCtrl = (DmaRegs *) SAR_DMA_BASE;
   StateRam = (volatile DmaStateRam *)&pGi->dmaCtrl->stram.s;

#if 0
   /* The following does not work in ARM. Keep this for records.*/
   printk ("bcmxtmrt: Setting SAR Rx DMA CHANNEL \n") ;
   printk ("bcmxtmrt: IO_ADDR DMA_BASE=%x, PHYS_BASE=%x \n", SAR_DMA_BASE, SAR_DMA_PHYS_BASE) ;
   printk ("bcmxtmrt: Stram - %x \n", (unsigned int) &StateRam[SAR_RX_DMA_BASE_CHAN]) ;
   printk ("bcmxtmrt: Before memset base-%x, state_data-%x, desc_len_status-%x, desc_base_bufptr-%x \n",
            (unsigned int) StateRam->baseDescPtr, (unsigned int) StateRam->state_data, (unsigned int) StateRam->desc_len_status,
            (unsigned int) StateRam->desc_base_bufptr) ;
            
   printk ("size = %d Channels = %d \n", sizeof (DmaStateRam), NR_SAR_RX_DMA_CHANS) ;
   memset((char *) &StateRam[SAR_RX_DMA_BASE_CHAN], 0x00, sizeof(DmaStateRam) * NR_SAR_RX_DMA_CHANS);

   printk ("bcmxtmrt: After memset base-%x, state_data-%x, desc_len_status-%x, desc_base_bufptr-%x \n",
            (unsigned int) StateRam->baseDescPtr, (unsigned int) StateRam->state_data, (unsigned int) StateRam->desc_len_status,
            (unsigned int) StateRam->desc_base_bufptr) ;
#else
   /* Explicitly assign the StateRam values */
   StateRam->baseDescPtr = 0 ;
   StateRam->state_data = 0 ;
   StateRam->desc_len_status = 0 ;
   StateRam->desc_base_bufptr = 0 ;
#endif

   /* setup the RX DMA channels */
   for (i = 0; i < MAX_RECEIVE_QUEUES; i++)
   {
      BcmXtm_RxDma *rxdma;

      rxdma = pGi->rxdma[i];

      rxdma->pktDmaRxInfo.rxDma = &pGi->dmaCtrl->chcfg[SAR_RX_DMA_BASE_CHAN + i];
      rxdma->pktDmaRxInfo.rxDma->cfg = 0;

      if( pGip->ulReceiveQueueSizes[i] == 0 ) continue;

      rxdma->pktDmaRxInfo.rxDma->maxBurst = SAR_DMA_MAX_BURST_LENGTH;
      rxdma->pktDmaRxInfo.rxDma->intMask = 0;   /* mask all ints */
      rxdma->pktDmaRxInfo.rxDma->intStat = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
      rxdma->pktDmaRxInfo.rxDma->intMask = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
#if !defined(CONFIG_ARM)
      pGi->dmaCtrl->stram.s[SAR_RX_DMA_BASE_CHAN + i].baseDescPtr =
                        (UINT32)VIRT_TO_PHYS((UINT32 *)rxdma->pktDmaRxInfo.rxBds);
#else
      pGi->dmaCtrl->stram.s[SAR_RX_DMA_BASE_CHAN + i].baseDescPtr =
                        (UINT32)((UINT32 *)rxdma->pktDmaRxInfo.rxBdsPhysBase);
#endif

      /* register the RX ISR */
      if (rxdma->rxIrq)
      {
#if !defined(CONFIG_ARM)
         BcmHalInterruptDisable(rxdma->rxIrq);
#endif
         BcmHalMapInterrupt(bcmxtmrt_rxisr,
                            (void*)(BUILD_CONTEXT(pGi,i)), rxdma->rxIrq);
      }

      bcmPktDma_XtmInitRxChan(rxdma->pktDmaRxInfo.numRxBds, &rxdma->pktDmaRxInfo);

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
      xtm_rx_set_iq_thresh( i );
#endif

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
      bcmPktDma_XtmSetRxChanBpmThresh(&rxdma->pktDmaRxInfo,
                (rxdma->pktDmaRxInfo.numRxBds * BPM_XTM_ALLOC_TRIG_PCT/100),
                BPM_XTM_BULK_ALLOC_COUNT);

      BCM_XTM_DEBUG( "Xtm: BPM Rx allocTrig=%d bulkAlloc=%d",
            (int) (rxdma->pktDmaRxInfo.allocTrig),
            (int) rxdma->pktDmaRxInfo.bulkAlloc );
#endif
   }

   /* Allocate receive socket buffers and data buffers. */
   for (i = 0; i < MAX_RECEIVE_QUEUES; i++)
   {
#if !(defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
      const UINT32 ulRxAllocSize = BCM_PKTBUF_SIZE;
      const UINT32 ulBlockSize = (64 * 1024);
      const UINT32 ulBufsPerBlock = ulBlockSize / ulRxAllocSize;
      UINT32 ulAllocAmt;
      UINT8 *pFkBuf;
      UINT8 *data;
#endif
      UINT32 BufsToAlloc;
      UINT32 context = 0;

      rxdma = pGi->rxdma[i];
      j = 0;

      rxdma->pktDmaRxInfo.rxAssignedBds = 0;
      rxdma->pktDmaRxInfo.rxHeadIndex = rxdma->pktDmaRxInfo.rxTailIndex = 0;

      BufsToAlloc = rxdma->pktDmaRxInfo.numRxBds;

      RECYCLE_CONTEXT(context)->channel = rxdma->pktDmaRxInfo.channel;

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
      if ((xtm_bpm_alloc_buf_ring(rxdma, BufsToAlloc)) == GBPM_ERROR)
      {
         BCM_XTM_ERROR("Failed to alloc BPM buffer for RX ring");
         /* release all allocated receive buffers */
         xtm_bpm_free_buf_ring(rxdma);
         kfree(pGi->rxdma[i]);
         return -ENOMEM;
      }

      gbpm_resv_rx_buf(GBPM_PORT_XTM, i, rxdma->pktDmaRxInfo.numRxBds,
                (rxdma->pktDmaRxInfo.numRxBds * BPM_XTM_ALLOC_TRIG_PCT/100));
#else
      while (BufsToAlloc)
      {
         ulAllocAmt = (ulBufsPerBlock < BufsToAlloc) ? ulBufsPerBlock : BufsToAlloc;

         ulSize = ulAllocAmt * ulRxAllocSize;
         ulSize = (ulSize + 0x0f) & ~0x0f;

         if ((j >= MAX_BUFMEM_BLOCKS) ||
             ((data = kmalloc(ulSize, GFP_ATOMIC)) == NULL))
         {
            /* release all allocated receive buffers */
            BCM_XTM_NOTICE(CARDNAME": Low memory.");
            for (k = 0; k < MAX_BUFMEM_BLOCKS; k++)
            {
               if (rxdma->buf_pool[k])
               {
                  kfree(rxdma->buf_pool[k]);
                  rxdma->buf_pool[k] = NULL;
               }
            }
            for (k = 0; k < MAX_RECEIVE_QUEUES; k++)
            {
#if !defined(XTM_RX_BDS_IN_PSM)
#if !defined(CONFIG_ARM)
               if (pGi->rxdma[k]->pktDmaRxInfo.rxBdsBase)
                  kfree((void *)pGi->rxdma[k]->pktDmaRxInfo.rxBdsBase);
#else
               if (pGi->rxdma[k]->pktDmaRxInfo.rxBdsBase) {
                  uint32_t size32 = (uint32_t) (pGip->ulReceiveQueueSizes[k] * sizeof(DmaDesc) + 0x10) ;
                  void *mem = (void *) pGi->rxdma[k]->pktDmaRxInfo.rxBdsBase ;
                  void *physMem = (void *) pGi->rxdma[k]->pktDmaRxInfo.rxBdsPhysBase ;
                  NONCACHED_FREE (size32, mem, (dma_addr_t) physMem) ;
               }
#endif
#endif
               kfree(pGi->rxdma[k]);
            }
            return -ENOMEM;
         }

         rxdma->buf_pool[j++] = data;
         memset(data, 0x00, ulSize);
         cache_flush_len(data, ulSize);

         data = (UINT8 *) (((UINT32) data + 0x0f) & ~0x0f);
         for (k = 0, pFkBuf = data; k < ulAllocAmt; k++)
         {
            /* Place a FkBuff_t object at the head of pFkBuf */
            fkb_preinit(pFkBuf, (RecycleFuncP)bcmxtmrt_recycle, context);
            FlushAssignRxBuffer(i,
                                PFKBUFF_TO_PDATA(pFkBuf, BCM_PKT_HEADROOM),
                                (UINT8 *)pFkBuf + BCM_PKTBUF_SIZE);

            pFkBuf += BCM_PKTBUF_SIZE;
         }
         BufsToAlloc -= ulAllocAmt;
      }
#endif
      {
         int s;
         unsigned char *pSkbuff;

         if ((rxdma->skbs_p =
                  kmalloc((rxdma->pktDmaRxInfo.numRxBds * BCM_SKB_ALIGNED_SIZE) + 0x10,
                          GFP_ATOMIC)) == NULL) {
            BCM_XTM_ERROR("Failed to allocate memory for skbs");
            return -ENOMEM;
         }

         memset(rxdma->skbs_p, 0,
                (rxdma->pktDmaRxInfo.numRxBds * BCM_SKB_ALIGNED_SIZE) + 0x10);

         /* Chain socket skbs */
         for (s = 0, pSkbuff = (UINT8 *)(((UINT32)rxdma->skbs_p + 0x0f) & ~0x0f);
              s < rxdma->pktDmaRxInfo.numRxBds;
              s++, pSkbuff += BCM_SKB_ALIGNED_SIZE)
         {
            ((struct sk_buff *) pSkbuff)->next_free = rxdma->freeSkbList;
            rxdma->freeSkbList = (struct sk_buff *) pSkbuff;
         }
      }

      rxdma->end_skbs_p = rxdma->skbs_p + 
            (rxdma->pktDmaRxInfo.numRxBds * BCM_SKB_ALIGNED_SIZE) + 0x10; 
   }
   pGi->bondConfig.uConfig = pGip->bondConfig.uConfig ;
   if ((pGi->bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE) ||
       (pGi->bondConfig.sConfig.atmBond == BC_ATM_BONDING_ENABLE))
      BCM_XTM_NOTICE(CARDNAME ": PTM/ATM Bonding Mode configured in system");
   else
      BCM_XTM_NOTICE(CARDNAME ": PTM/ATM Non-Bonding Mode configured in system");

   pGi->atmBondSidMode = ATMBOND_ASM_MESSAGE_TYPE_NOSID ;

   /* Initialize a timer function to free transmit buffers. */
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4,19,0))
   init_timer(&pGi->Timer);
   pGi->Timer.function = (void *)bcmxtmrt_timer;
#else
   timer_setup(&pGi->Timer, bcmxtmrt_timer, 0);
#endif
   pGi->Timer.data = (UINT32)pGi;

   /* This was not done before. Is done in impl1 - Apr 2010 */
   pGi->dmaCtrl->controller_cfg |= DMA_MASTER_EN;

   pGi->ulDrvState = XTMRT_INITIALIZED;

   bcmxtmrt_in_init_dev = 0;

   return 0;
    
}  /* bcmxapi_DoGlobInitReq() */


/*---------------------------------------------------------------------------
 * int bcmxapi_DoGlobUninitReq(void)
 * Description:
 *    Processes an XTMRT_CMD_GLOBAL_UNINITIALIZATION command.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
int bcmxapi_DoGlobUninitReq(void)
{
   int nRet = 0;
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   UINT32 i;
#if !(defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
   UINT32 j;
#endif

   if (pGi->ulDrvState == XTMRT_UNINITIALIZED)
   {
      nRet = -EPERM;
   }
   else
   {
      pGi->ulDrvState = XTMRT_UNINITIALIZED;

      for (i = 0; i < MAX_RECEIVE_QUEUES; i++)
      {
#if !defined(CONFIG_ARM)
         BcmHalInterruptDisable(SAR_RX_INT_ID_BASE + i);
#endif

#if !defined(XTM_RX_BDS_IN_PSM)
#if !defined(CONFIG_ARM)
         if (pGi->rxdma[i]->pktDmaRxInfo.rxBdsBase)
            kfree((void *)pGi->rxdma[i]->pktDmaRxInfo.rxBdsBase);
#else
            if (pGi->rxdma[i]->pktDmaRxInfo.rxBdsBase) {
               uint32_t size32 = (uint32_t) (bcmPktDma_XtmGetRxBds(i) * sizeof(DmaDesc) + 0x10) ;
               void *mem = (void *) pGi->rxdma[i]->pktDmaRxInfo.rxBdsBase ;
               void *physMem = (void *) pGi->rxdma[i]->pktDmaRxInfo.rxBdsPhysBase ;
               NONCACHED_FREE (size32, mem, (dma_addr_t) physMem) ;
            }
#endif
#endif
		 bcmPktDma_XtmUnInitRxChan(&pGi->rxdma[i]->pktDmaRxInfo);

            /* Free space for receive socket buffers and data buffers */
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
         xtm_bpm_free_buf_ring(pGi->rxdma[i]);
#else
         if (pGi->rxdma[i]->skbs_p)
            kfree(pGi->rxdma[i]->skbs_p);

         for (j = 0; j < MAX_BUFMEM_BLOCKS; j++)
         {
            if (pGi->rxdma[i]->buf_pool[j])
            {
               kfree(pGi->rxdma[i]->buf_pool[j]);
               pGi->rxdma[i]->buf_pool[j] = NULL;
            }
         }
#endif
         if (pGi->rxdma[i])
            kfree((void *)(pGi->rxdma[i]));

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
         gbpm_unresv_rx_buf( GBPM_PORT_XTM, i );
#endif
      }

      del_timer_sync(&pGi->Timer);
   }
   return (nRet);

}  /* bcmxapi_DoGlobUninitReq() */


/*---------------------------------------------------------------------------
 * int bcmxapi_DoSetTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
 *                          PXTMRT_TRANSMIT_QUEUE_ID pTxQId)
 * Description:
 *    Allocate memory for and initialize a transmit queue.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
int bcmxapi_DoSetTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                         PXTMRT_TRANSMIT_QUEUE_ID pTxQId)
{
   UINT32 ulQueueSize, ulPort;
   BcmPktDma_XtmTxDma  *txdma;
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   UINT32 phy_addr ;

   BCM_XTM_DEBUG("DoSetTxQueue");

   local_bh_enable();  /* needed to avoid kernel error */

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
   xtm_bpm_txq_thresh(pDevCtx, pTxQId);
#endif

   txdma = (BcmPktDma_XtmTxDma *)kzalloc(sizeof(BcmPktDma_XtmTxDma), GFP_ATOMIC);

   local_bh_disable();

   if (txdma == NULL)
   {
      BCM_XTM_ERROR("Unable to allocate memory for tx dma info");
      return -ENOMEM;
   }

   /* Increment channels per dev context */
   pDevCtx->txdma[pDevCtx->ulTxQInfosSize++] = txdma;

   txdma->ulDmaIndex = pTxQId->ulQueueIndex;
   
   /* Set every transmit queue size to the number of external buffers.
    * The QueuePacket function will control how many packets are queued.
    */
   ulQueueSize = pTxQId->usQueueSize; // pGi->ulNumExtBufs;

   local_bh_enable();  // needed to avoid kernel error - Apr 2010
   /* allocate and assign tx buffer descriptors */
   txdma->txBdsBase = bcmPktDma_XtmAllocTxBds(txdma->ulDmaIndex, ulQueueSize, &phy_addr);
#if defined(CONFIG_ARM)
   txdma->txBdsPhysBase = (volatile DmaDesc *) phy_addr ;
#endif

   local_bh_disable();

   if (txdma->txBdsBase == NULL)
   {
      BCM_XTM_ERROR("Unable to allocate memory for Tx Descriptors ");
      return -ENOMEM;
   }
#if defined(XTM_TX_BDS_IN_PSM)
   txdma->txBds = txdma->txBdsBase;
#else
   /* Align to 16 byte boundary - Apr 2010 */
   txdma->txBds = (volatile DmaDesc *)(((int)txdma->txBdsBase + 0xF) & ~0xF);
   txdma->txBds = (volatile DmaDesc *)CACHE_TO_NONCACHE(txdma->txBds);
#endif

   /* pKeyPtr, pTxSource, pTxAddress now relative to txBds - Apr 2010 */
   txdma->txRecycle = (BcmPktDma_txRecycle_t *)((UINT32)txdma->txBds + (pGi->ulNumExtBufs * sizeof(DmaDesc)));
#if !defined(XTM_TX_BDS_IN_PSM)
   txdma->txRecycle = (BcmPktDma_txRecycle_t *)NONCACHE_TO_CACHE(txdma->txRecycle);
#endif
   BCM_XTM_NOTICE("XTM Init: Ch:%d - %d tx BDs at 0x%x", (int)txdma->ulDmaIndex, (int)pGi->ulNumExtBufs, (unsigned int)txdma->txBds);

   ulPort = PORTID_TO_PORT(pTxQId->ulPortId);

   if ((ulPort < MAX_PHY_PORTS) && (pTxQId->ucSubPriority < MAX_SUB_PRIORITIES))
   {
     UINT32 ulPtmPrioIdx = PTM_FLOW_PRI_LOW;
     volatile DmaRegs *pDmaCtrl = TXDMACTRL(pDevCtx);
     volatile DmaStateRam *pStRam = pDmaCtrl->stram.s;
     UINT32 i, ulTxQs;

     txdma->ulPort        = ulPort;
     txdma->ulPtmPriority = pTxQId->ulPtmPriority;
     txdma->ulSubPriority = pTxQId->ucSubPriority;
     txdma->ulAlg         = (UINT16)pTxQId->ucWeightAlg;
     txdma->ulWeightValue = (UINT16)pTxQId->ulWeightValue;
     txdma->ulQueueSize   = ulQueueSize;
//     txdma->ulDmaIndex = pTxQId->ulQueueIndex;
     txdma->txEnabled     = 1;
     txdma->ulNumTxBufsQdOne = 0;

     txdma->txDma = &pDmaCtrl->chcfg[SAR_TX_DMA_BASE_CHAN + txdma->ulDmaIndex];
     txdma->txDma->cfg      = 0;
     txdma->txDma->maxBurst = SAR_DMA_MAX_BURST_LENGTH;
     txdma->txDma->intMask  = 0;   /* mask all ints */
     txdma->txStateRam = (volatile DmaStateRam *) &pStRam[SAR_TX_DMA_BASE_CHAN + txdma->ulDmaIndex] ;

#if 0
     /* The following does not take effect in ARM. Keep it for records. */
     memset ((UINT8 *)txdma->txStateRam, 0x00, sizeof(DmaStateRam));
#else
     /* Explicitly assign the StateRam values. */
     txdma->txStateRam->baseDescPtr = 0 ;
     txdma->txStateRam->state_data = 0 ;
     txdma->txStateRam->desc_len_status = 0 ;
     txdma->txStateRam->desc_base_bufptr = 0 ;
#endif

#if !defined(CONFIG_ARM)
     txdma->txStateRam->baseDescPtr = (UINT32) VIRT_TO_PHYS(txdma->txBds);
#else
     txdma->txStateRam->baseDescPtr = (UINT32) txdma->txBdsPhysBase ;
#endif

     txdma->txBds[txdma->ulQueueSize - 1].status |= DMA_WRAP;
     txdma->txFreeBds   = txdma->ulQueueSize;
     txdma->txHeadIndex = txdma->txTailIndex;

     if (pDevCtx->Addr.ulTrafficType == TRAFFIC_TYPE_PTM)
        ulPtmPrioIdx = (txdma->ulPtmPriority == PTM_PRI_HIGH)?
                       PTM_FLOW_PRI_HIGH : PTM_FLOW_PRI_LOW;

     pDevCtx->pTxPriorities[ulPtmPrioIdx][ulPort][txdma->ulSubPriority] = txdma;
     pDevCtx->pTxQids[pTxQId->ucQosQId] = txdma;

     if (pDevCtx->pHighestPrio == NULL ||
         pDevCtx->pHighestPrio->ulSubPriority < txdma->ulSubPriority)
        pDevCtx->pHighestPrio = txdma;


     /* Count the total number of transmit queues used across all device
      * interfaces.
      */
     for (i = 0, ulTxQs = 0; i < MAX_DEV_CTXS; i++)
     {
        if (pGi->pDevCtxs[i])
           ulTxQs += pGi->pDevCtxs[i]->ulTxQInfosSize;
     }
     pGi->ulNumTxQs = ulTxQs;

   }

   bcmPktDma_XtmInitTxChan(txdma->ulQueueSize, txdma, TXDMATYPE(pDevCtx));

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
   bcmPktDma_XtmSetTxChanBpmThresh (txdma, pTxQId->ulLoThresh, pTxQId->ulHiThresh, XTM_HW_DMA);

   BCM_XTM_DEBUG("XTM TxQid ulLoThresh=%d, ulHiThresh=%d",
      (int) pTxQId->ulLoThresh, (int) pTxQId->ulHiThresh);
#endif

   bcmPktDma_XtmSetTxChanDropAlg(txdma, pTxQId->ucDropAlg,
                                 100, pTxQId->ucLoMinThresh, pTxQId->ucLoMaxThresh,
                                 100, pTxQId->ucHiMinThresh, pTxQId->ucHiMaxThresh);

   bcmPktDma_XtmTxEnable(txdma, NULL, TXDMATYPE(pDevCtx));

   return 0;
    
}  /* bcmxapi_DoSetTxQueue() */


/*---------------------------------------------------------------------------
 * void bcmxapi_ShutdownTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
 *                              volatile BcmPktDma_XtmTxDma *txdma)
 * Description:
 *    Shutdown and clean up a transmit queue by waiting for it to
 *    empty, clearing state ram and free memory allocated for it.
 * Returns: void
 *---------------------------------------------------------------------------
 */
void bcmxapi_ShutdownTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                             volatile BcmPktDma_XtmTxDma *txdma)
{
   UINT32 ulPtmPrioIdx = txdma->ulPtmPriority ? txdma->ulPtmPriority - PTM_PRI_LOW : 0;
   volatile DmaStateRam *pStRam   = txdma->txStateRam ;


   bcmPktDma_XtmTxDisable((BcmPktDma_LocalXtmTxDma *)txdma, TXDMATYPE(pDevCtx),
                          NULL, ulPtmPrioIdx);


   freeXmitPkts(pDevCtx, txdma, XTMFREE_FORCE_FREE);

   if ((txdma->txDma->cfg & DMA_ENABLE) == DMA_ENABLE)
      BCM_XTM_NOTICE("HOST XTM tx ch %d NOT disabled. Force disable.",
             (int)txdma->ulDmaIndex);
   else
      BCM_XTM_NOTICE("HOST XTM tx ch %d disabled.", (int)txdma->ulDmaIndex);

   pStRam->baseDescPtr      = 0;
   pStRam->state_data       = 0;
   pStRam->desc_len_status  = 0;
   pStRam->desc_base_bufptr = 0;

   txdma->txStateRam = NULL ;

   /* update total number of tx queues used by the system */
   g_GlobalInfo.ulNumTxQs--;

#if !defined(XTM_TX_BDS_IN_PSM)
   /* remove the tx bd ring */
   if (txdma->txBdsBase) {
#if !defined(CONFIG_ARM)
      bcmPktDma_XtmFreeTxBds ((DmaDesc *) txdma->txBdsBase, NULL, txdma->ulQueueSize) ;
#else
      bcmPktDma_XtmFreeTxBds ((DmaDesc *) txdma->txBdsBase, (DmaDesc *) txdma->txBdsPhysBase, txdma->ulQueueSize) ;
#endif
   }

   txdma->txFreeBds = txdma->ulQueueSize = 0; /* this MUST be Last after Q free */

   txdma->txBdsBase = txdma->txBds = NULL;
#if defined(CONFIG_ARM)
   txdma->txBdsPhysBase =  NULL ;
#endif
#endif
}  /* bcmxapi_ShutdownTxQueue() */


/*---------------------------------------------------------------------------
 * void bcmxapi_FlushdownTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
 *                               volatile BcmPktDma_XtmTxDma *txdma)
 * Description:
 *    Flushes and clean up a transmit queue by waiting for it to empty,
 * Returns: void
 *---------------------------------------------------------------------------
 */
void bcmxapi_FlushdownTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                              volatile BcmPktDma_XtmTxDma *txdma)
{
   UINT32 j ;
   UINT32 ulPtmPrioIdx;


   ulPtmPrioIdx = txdma->ulPtmPriority ? txdma->ulPtmPriority - PTM_PRI_LOW : 0 ;

   if (bcmPktDma_XtmTxDisable((BcmPktDma_LocalXtmTxDma *)txdma, TXDMATYPE(pDevCtx),
         NULL, ulPtmPrioIdx) == 0)
      BCM_XTM_ERROR("XTM Tx ch %d NOT disabled. Fatal", (int)txdma->ulDmaIndex) ;

   for (j = 1; (j < 2000) && ((txdma->txDma->cfg & DMA_ENABLE) == DMA_ENABLE); j++)
   {
      __udelay (10000) ;
      if ((j%100) == 0)
         BCM_XTM_ERROR("bcmxtmrt: Warning!! HOST XTM Tx Ch %d NOT disabled for the last %d sec(s)....",
                 (int)txdma->ulDmaIndex, (int)(j/100));
   }

   if ((txdma->txDma->cfg & DMA_ENABLE) == DMA_ENABLE)
   {
      BCM_XTM_ERROR("XTM Tx ch %d NOT flushed. Fatal", (int)txdma->ulDmaIndex);
   }
   else
   {
      BCM_XTM_NOTICE("HOST XTM Tx ch %d Flush success. time > %d ms <= %d ms",
             (int)txdma->ulDmaIndex, 
             ((j==0) ? 0 : (int)((j-1)*10)), ((j==0) ? 1 : (int)(j*10)));
   }
   freeXmitPkts (pDevCtx, txdma, XTMFREE_FORCE_FREE) ;

   BCM_XTM_NOTICE("bcmxtmrt: Current DMA Q Size %u", (unsigned int)txdma->ulNumTxBufsQdOne);

   BCM_XTM_NOTICE("bcmxtmrt: Enable Tx ch %d", (int)txdma->ulDmaIndex);
   bcmPktDma_XtmTxEnable((BcmPktDma_LocalXtmTxDma *)txdma, NULL, TXDMATYPE(pDevCtx));
}  /* bcmxapi_FlushdownTxQueue() */

void bcmxapi_StopTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                              volatile BcmPktDma_XtmTxDma *txdma)

{
}

void bcmxapi_StartTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                              volatile BcmPktDma_XtmTxDma *txdma)

{
}

/*---------------------------------------------------------------------------
 * int bcmxapi_SetPortShaperInfo (PBCMXTMRT_GLOBAL_INFO pGi)
 * Description:
 *    Set/UnSet the global port shaper information.
 * Returns: void
 * Notes:
 *    pDevCtx is not used.
 *---------------------------------------------------------------------------
 */
int bcmxapi_SetTxPortShaperInfo(PBCMXTMRT_GLOBAL_INFO pGi, PXTMRT_PORT_SHAPER_INFO pShaperInfo)
{
   return (0) ;
}  /* bcmxapi_SetTxPortShaperInfo () */

/*---------------------------------------------------------------------------
 * void bcmxapi_SetPtmBondPortMask(UINT32 portMask)
 * Description:
 *    Set the value of portMask in ptmBondInfo data structure.
 * Returns: void
 *---------------------------------------------------------------------------
 */
void bcmxapi_SetPtmBondPortMask(UINT32 portMask)
{
   g_GlobalInfo.ptmBondInfo.portMask = portMask;
   
}  /* bcmxapi_SetPtmBondPortMask() */


/*---------------------------------------------------------------------------
 * void bcmxapi_SetPtmBonding(UINT32 bonding)
 * Description:
 *    Set the value of bonding in ptmBondInfo data structure.
 * Returns: void
 *---------------------------------------------------------------------------
 */
void bcmxapi_SetPtmBonding(UINT32 bonding)
{
   g_GlobalInfo.ptmBondInfo.bonding = bonding;
   
}  /* bcmxapi_SetPtmBonding() */


/*---------------------------------------------------------------------------
 * void bcmxapi_XtmGetStats(UINT8 vport, UINT32 *rxDropped, UINT32 *txDropped)
 * Description:
 *
 * Returns: void
 *---------------------------------------------------------------------------
 */
void bcmxapi_XtmGetStats(PBCMXTMRT_DEV_CONTEXT pDevCtx, UINT32 *rxDropped, UINT32 *txDropped)
{
   uint8_t vport = 0;
   bcmPktDma_XtmGetStats(vport, rxDropped, txDropped);
   
}  /* bcmxapi_XtmGetStats() */


/*---------------------------------------------------------------------------
 * void bcmxapi_XtmResetStats(UINT8 vport)
 * Description:
 *
 * Returns: void
 *---------------------------------------------------------------------------
 */
void bcmxapi_XtmResetStats(UINT8 vport)
{
   bcmPktDma_XtmResetStats(vport);
   
}  /* bcmxapi_XtmGetStats() */

/*---------------------------------------------------------------------------
 * void bcmxapi_blog_ptm_us_bonding (UINT32 ulTxPafEnabled, sk_buff *skb)
 * Description:
 *
 * Returns: void
 *---------------------------------------------------------------------------
 */
void bcmxapi_blog_ptm_us_bonding(UINT32 ulTxPafEnabled, struct sk_buff *skb)
{
   blog_ptm_us_bonding (skb, BLOG_PTM_US_BONDING_ENABLED) ;
}  /* bcmxapi_blog_ptm_us_bonding() */

/*---------------------------------------------------------------------------
 * void freeXmitPkts(PBCMXTMRT_DEV_CONTEXT pDevCtx,
 *                   volatile BcmPktDma_XtmTxDma *txdma,
 *                   int forceFree)
 * Description:
 *
 * Returns: void
 *---------------------------------------------------------------------------
 */
static void freeXmitPkts(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                         volatile BcmPktDma_XtmTxDma *txdma,
                         int forceFree)
{
   int ret;
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   UINT32 txAddr;
   UINT32 txSource, rxChannel;
   pNBuff_t nbuff_reclaim_p;

   /* Free transmitted packets. */
   while (1)
   {
      if (forceFree)
         ret = bcmPktDma_XtmForceFreeXmitBufGet((BcmPktDma_LocalXtmTxDma *)txdma,
                  (UINT32 *)&nbuff_reclaim_p, &txSource, &txAddr, &rxChannel,
                  TXDMATYPE(pDevCtx), 0x0);
      else
         ret = bcmPktDma_XtmFreeXmitBufGet((BcmPktDma_LocalXtmTxDma *)txdma,
                  (UINT32 *)&nbuff_reclaim_p, &txSource, &txAddr, &rxChannel,
                  TXDMATYPE(pDevCtx), 0x0);

      if (!ret)
         break ;
      else {
         if (nbuff_reclaim_p != PNBUFF_NULL)
         {
            spin_unlock_bh(&pGi->xtmlock_tx);
            nbuff_free(nbuff_reclaim_p);
            spin_lock_bh(&pGi->xtmlock_tx);
         }
      }
   } /* while 1 */
   
   if(forceFree && (txdma->ulNumTxBufsQdOne > 0))
      BCM_XTM_ERROR("HOST XTM tx ch %d force free failed. Remaining queue len=%d",
             (int)txdma->ulDmaIndex, (int)txdma->ulNumTxBufsQdOne);
      
}  /* freeXmitPkts() */


#if 0
/*---------------------------------------------------------------------------
 * void xtmDmaStatus(int channel, BcmPktDma_XtmTxDma *txdma)
 * Description:
 *    Dumps information about the status of the XTM IUDMA channel
 * Returns: void
 *---------------------------------------------------------------------------
 */
static void xtmDmaStatus(int channel, BcmPktDma_XtmTxDma *txdma)
{
   BcmPktDma_LocalXtmRxDma *rxdma;

   rxdma = (BcmPktDma_LocalXtmRxDma *)&(g_GlobalInfo.rxdma[channel]->pktDmaRxInfo);

   printk("XTM IUDMA INFO CH %d\n", channel);

   printk("RXDMA STATUS: HeadIndex: %d TailIndex: %d numRxBds: %d rxAssignedBds: %d\n",
               rxdma->rxHeadIndex, rxdma->rxTailIndex,
               rxdma->numRxBds, rxdma->rxAssignedBds);

   printk("RXDMA CFG: cfg: 0x%lx intStat: 0x%lx intMask: 0x%lx\n",
               rxdma->rxDma->cfg,
               rxdma->rxDma->intStat,
               rxdma->rxDma->intMask);

   printk("TXDMA STATUS: HeadIndex: %d TailIndex: %d txFreeBds: %d\n",
               txdma->txHeadIndex,
               txdma->txTailIndex,
               txdma->txFreeBds);

   printk("TXDMA CFG: cfg: 0x%lx intStat: 0x%lx intMask: 0x%lx\n",
               txdma->txDma->cfg,
               txdma->txDma->intStat,
               txdma->txDma->intMask);

}  /* xtmDmaStatus() */
#endif
