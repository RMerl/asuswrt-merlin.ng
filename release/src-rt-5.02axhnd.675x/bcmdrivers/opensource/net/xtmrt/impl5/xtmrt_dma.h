/*
 Copyright 2002-2010 Broadcom Corp. All Rights Reserved.

 <:label-BRCM:2011:DUAL/GPL:standard    
 
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
#ifndef _BCMXTMRT_DMA_H_
#define _BCMXTMRT_DMA_H_

#include "bcm_intr.h"
#include "bcmPktDma.h"

/* 32-bit recycle context definition */
typedef union {
    struct {
        /* fapQuickFree handling removed - Oct 2010 */
#ifdef __LITTLE_ENDIAN
        UINT32 channel      :  2;
        UINT32 reserved     : 30;
#else
        UINT32 reserved     : 30;
        UINT32 channel      :  2;
#endif        
    };
    UINT32 u32;
} xtm_recycle_context_t;


#define RECYCLE_CONTEXT(_context)   ((xtm_recycle_context_t *)(&(_context)))
#define FKB_RECYCLE_CONTEXT(_pFkb)  RECYCLE_CONTEXT((_pFkb)->recycle_context)

#define CONTEXT_TO_CHANNEL(context) (int)((context) & 0x3u)


#define bcmxapi_XtmCreateDevice(_devId, _encapType, _headerLen, _trailerLen)  \
            bcmPktDma_XtmCreateDevice(_devId, _encapType, _headerLen, _trailerLen)
            
#define bcmxapi_XtmLinkUp(_devId, _matchId)  \
            bcmPktDma_XtmLinkUp(_devId, _matchId)


/**** Externs ****/

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
/* Add code for buffer quick free between enet and xtm - June 2010 */
extern RecycleFuncP enet_recycle_hook;
#endif


/**** Prototypes ****/

void FlushAssignRxBuffer(int channel, UINT8 *pData, UINT8 *pEnd);
void bcmxtmrt_recycle(pNBuff_t pNBuf, UINT32 context, UINT32 flags);
void bcmxtmrt_recycle_skb_or_data(struct sk_buff *skb, UINT32 context,
                                  UINT32 nFlag);

int bcmxapi_module_init(void);
void bcmxapi_module_cleanup(void);
int bcmxapi_enable_rx_interrupt(void);
int bcmxapi_disable_rx_interrupt(void);
UINT32 bcmxapi_rxtask(UINT32 ulBudget, UINT32 *pulMoreToDo);

int bcmxapi_add_proc_files(void);
int bcmxapi_del_proc_files(void);

int bcmxapi_DoGlobInitReq(PXTMRT_GLOBAL_INIT_PARMS pGip);
int bcmxapi_DoGlobUninitReq(void);
int bcmxapi_DoSetTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                         PXTMRT_TRANSMIT_QUEUE_ID pTxQId);
void bcmxapi_ShutdownTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                             volatile BcmPktDma_XtmTxDma *txdma);
void bcmxapi_FlushdownTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                              volatile BcmPktDma_XtmTxDma *txdma);
void bcmxapi_StopTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                              volatile BcmPktDma_XtmTxDma *txdma);
void bcmxapi_StartTxQueue(PBCMXTMRT_DEV_CONTEXT pDevCtx,
                              volatile BcmPktDma_XtmTxDma *txdma);
int  bcmxapi_SetTxPortShaperInfo(PBCMXTMRT_GLOBAL_INFO pGi, 
                             PXTMRT_PORT_SHAPER_INFO pShaperInfo);
void bcmxapi_SetPtmBondPortMask(UINT32 portMask);
void bcmxapi_SetPtmBonding(UINT32 bonding);
void bcmxapi_XtmGetStats(UINT8 vport, UINT32 *rxDropped, UINT32 *txDropped);
void bcmxapi_XtmResetStats(UINT8 vport);
void bcmxapi_blog_ptm_us_bonding(UINT32 ulTxPafEnabled, struct sk_buff *skb) ;
int  bcmxapi_GetPaddingAdjustedLen (int len) ;



/**** Inline functions ****/

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
#include "fap_task.h"
#include "fap_dqm.h"
#endif
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
#include "xtmrt_iq.h"
#endif
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#include "xtmrt_bpm.h"
#endif

static inline struct sk_buff *xtm_skb_alloc(BcmXtm_RxDma *rxdma);
static inline void xtm_skb_free(BcmXtm_RxDma *rxdma, struct sk_buff *skb); 

static inline struct sk_buff *bcmxapi_skb_alloc(void *rxdma, pNBuff_t pNBuf, 
                                                int delLen, int trailerDelLen);
static inline FkBuff_t *bcmxapi_fkb_qinit(UINT8 *pBuf, UINT8 *pData,
                                          UINT32 len, void *rxdma);
static inline void bcmxapi_rx_pkt_drop(void *rxdma, UINT8 *pBuf, int len);
static inline void bcmxapi_free_xmit_packets(PBCMXTMRT_DEV_CONTEXT pDevCtx);
static inline UINT32 bcmxapi_xmit_available(void *txdma, UINT32 skbMark);
static inline int bcmxapi_queue_packet(PTXQINFO pTqi, UINT32 isAtmCell);
#ifdef CONFIG_BLOG
static inline void bcmxapi_blog_emit (pNBuff_t pNBuf, struct net_device *dev,
                           PBCMXTMRT_DEV_CONTEXT pDevCtx,
                           BcmPktDma_XtmTxDma *txdma,
                           UINT32 rfc2684Type, UINT16 bufStatus);
#endif
static inline int bcmxapi_xmit_packet(pNBuff_t *ppNBuf, UINT8 **ppData, UINT32 *pLen,
                                      BcmPktDma_XtmTxDma *txdma, UINT32 txdmaIdx,
                                      UINT16 bufStatus, UINT32 skbMark, int is_spdsvc_setup_packet,
                                      void *pTcpSpdTstInfo);
static inline void bcmxapi_clear_xtmrxint(UINT32 mask);


/*---------------------------------------------------------------------------
 * struct sk_buff *xtm_skb_alloc(BcmXtm_RxDma *rxdma)
 * Description:
 *    Allocate an SKB either from SKB pool or dynamically
 * Returns:
 *    skb
 *---------------------------------------------------------------------------
 */
static inline struct sk_buff *xtm_skb_alloc(BcmXtm_RxDma *rxdma)
{
   struct sk_buff *skb;
   spinlock_t     *xtmlock_rx = &g_GlobalInfo.xtmlock_rx;
      
#if defined (CONFIG_BCM963138) || defined (CONFIG_BCM963148)
   UINT32   irqFlags;
      
   spin_lock_irqsave(xtmlock_rx, irqFlags);
#else
   spin_lock_bh(xtmlock_rx);   
#endif
      
   if (rxdma->freeSkbList)
   {
      skb = rxdma->freeSkbList;
      rxdma->freeSkbList = rxdma->freeSkbList->next_free;
   }
   else
      skb = kmem_cache_alloc(g_GlobalInfo.xtmSkbCache, GFP_ATOMIC);

#if defined (CONFIG_BCM963138) || defined (CONFIG_BCM963148)
   spin_unlock_irqrestore(xtmlock_rx, irqFlags);
#else
   spin_unlock_bh(xtmlock_rx);   
#endif
      
   return skb;
   
}  /* xtm_skb_alloc() */


/*---------------------------------------------------------------------------
 * void xtm_skb_free(BcmXtm_RxDma *rxdma, struct sk_buff *skb)
 * Description:
 *    Free an SKB from wherever it was allocated from SKB pool or dynamically
 * Returns: void
 *---------------------------------------------------------------------------
 */
static inline void xtm_skb_free(BcmXtm_RxDma *rxdma, struct sk_buff *skb) 
{
   spinlock_t  *xtmlock_rx = &g_GlobalInfo.xtmlock_rx;
      
#if defined (CONFIG_BCM963138) || defined (CONFIG_BCM963148)
   UINT32   irqFlags;
      
   spin_lock_irqsave(xtmlock_rx, irqFlags);
#else
   spin_lock_bh(xtmlock_rx);   
#endif
      
   if ((UINT8 *)skb < rxdma->skbs_p || (UINT8 *)skb >= rxdma->end_skbs_p)
   {
      kmem_cache_free(g_GlobalInfo.xtmSkbCache, skb);
   }
   else
   {
      skb->next_free     = rxdma->freeSkbList;
      rxdma->freeSkbList = skb;
   }
   
#if defined (CONFIG_BCM963138) || defined (CONFIG_BCM963148)
   spin_unlock_irqrestore(xtmlock_rx, irqFlags);
#else
   spin_unlock_bh(xtmlock_rx);   
#endif
}  /* xtm_skb_free() */


/*---------------------------------------------------------------------------
 * struct sk_buff *bcmxapi_skb_alloc(void *rxdma, pNBuff_t pNBuf, 
 *                                   int delLen, int trailerDelLen)
 * Description:
 *
 * Returns:
 *    skb
 *---------------------------------------------------------------------------
 */
static inline struct sk_buff *bcmxapi_skb_alloc(void *rxdma, pNBuff_t pNBuf, 
                                                int delLen, int trailerDelLen)
{
   struct sk_buff *skb  = NULL;
   FkBuff_t       *pFkb = PNBUFF_2_FKBUFF(pNBuf);
   
   if (rxdma && pFkb)
   {
      skb = xtm_skb_alloc((BcmXtm_RxDma *)rxdma);
      if (!skb)
      {
         FlushAssignRxBuffer(((BcmXtm_RxDma *)rxdma)->pktDmaRxInfo.channel,
                             pFkb->data, pFkb->data);
      }
      else
      {
         UINT32 recycle_context = 0;

         RECYCLE_CONTEXT(recycle_context)->channel =
                              ((BcmXtm_RxDma * )rxdma)->pktDmaRxInfo.channel;

         skb_headerinit(BCM_PKT_HEADROOM,
#ifdef XTM_CACHE_SMARTFLUSH
                        pFkb->len+delLen+trailerDelLen+SAR_DMA_MAX_BURST_LENGTH,
#else
                        BCM_MAX_PKT_LEN,
#endif
                        skb, pFkb->data,
                        (RecycleFuncP)bcmxtmrt_recycle_skb_or_data,
                        recycle_context, pFkb->blog_p);

         if (delLen)
            __skb_pull(skb, delLen);

         __skb_trim(skb, pFkb->len);
      }
   }
   return skb;
   
}  /* bcmxapi_skb_alloc() */


/*---------------------------------------------------------------------------
 * FkBuff_t *bcmxapi_fkb_qinit(pNBuff_t pNBuf, UINT8 *pData,
 *                             UINT32 len, void *rxdma)
 * Description:
 *    Initialize the FKB context for a received packet.
 * Returns:
 *    fkb
 *---------------------------------------------------------------------------
 */
static inline FkBuff_t *bcmxapi_fkb_qinit(UINT8 *pBuf, UINT8 *pData,
                                          UINT32 len, void *rxdma)
{
   FkBuff_t *pFkb;
     
   pFkb = fkb_qinit(pBuf, BCM_PKT_HEADROOM, pData, len, (UINT32)rxdma);
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)) || (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
   {
      UINT32 context = 0;

      RECYCLE_CONTEXT(context)->channel = ((BcmXtm_RxDma *)rxdma)->pktDmaRxInfo.channel;

      pFkb->recycle_hook    = (RecycleFuncP)bcmxtmrt_recycle;
      pFkb->recycle_context = context;
   }
#endif
   return pFkb;

}  /* bcmxapi_fkb_qinit() */


/*---------------------------------------------------------------------------
 * void bcmxapi_rx_pkt_drop(void *rxdma, UINT8 *pBuf, int len)
 * Description:
 *
 * Returns: void
 *---------------------------------------------------------------------------
 */
static inline void bcmxapi_rx_pkt_drop(void *rxdma, UINT8 *pBuf, int len)
{
   FlushAssignRxBuffer(((BcmXtm_RxDma *)rxdma)->pktDmaRxInfo.channel,
                       pBuf, pBuf+len);
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
#if defined(CC_IQ_STATS)
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
   ((BcmXtm_RxDma *)rxdma)->pktDmaRxInfo.iqDroppedDqm++;
#else
   ((BcmXtm_RxDma *)rxdma)->pktDmaRxInfo.iqDropped++;
#endif
#endif
#endif
}  /* bcmxapi_rx_pkt_drop() */ 


/*---------------------------------------------------------------------------
 * void bcmxapi_free_xmit_packets(PBCMXTMRT_DEV_CONTEXT pDevCtx)
 * Description:
 *    Free packets from iudma that have been transmitted.
 * Returns: void
 *---------------------------------------------------------------------------
 */
static inline void bcmxapi_free_xmit_packets(PBCMXTMRT_DEV_CONTEXT pDevCtx)
{
   int      i;
   UINT32   txSource;
   UINT32   txAddr;
   UINT32   rxChannel;
   pNBuff_t nbuff_reclaim_p;
   BcmPktDma_XtmTxDma *txdma   = NULL;
   spinlock_t         *xtmlock = &g_GlobalInfo.xtmlock_tx;

   /* Free packets that have been transmitted. */
   for (i = 0; i < pDevCtx->ulTxQInfosSize; i++)
   {
      txdma = pDevCtx->txdma[i];
      while (bcmPktDma_XtmFreeXmitBufGet(txdma, (UINT32 *)&nbuff_reclaim_p,
                                         &txSource, &txAddr, &rxChannel,
                                         TXDMATYPE(pDevCtx), 0x0) == TRUE)
      {
         if (nbuff_reclaim_p != PNBUFF_NULL)
         {
            BCM_XTM_TX_DEBUG("Host bcmPktDma_XtmFreeXmitBufGet TRUE! (xmit) key 0x%x\n",
                             (int)nbuff_reclaim_p);
            spin_unlock_bh(xtmlock);
            nbuff_free(nbuff_reclaim_p);
            spin_lock_bh(xtmlock);
         }
      }
   }
}  /* bcmxapi_free_xmit_packets() */


/*---------------------------------------------------------------------------
 * UINT32 bcmxapi_xmit_available(void *txdma, UINT32 skbMark)
 * Description:
 *    Determine if there are free resources for the xmit.
 * Returns:
 *    0 - resource is not available
 *    1 - resource is available 
 *---------------------------------------------------------------------------
 */
static inline UINT32 bcmxapi_xmit_available(void *txdma, UINT32 skbMark)
{
#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
   /* Send the high priority tarffic HOST2FAP HIGH priorty DQM's and
    * other traffic through LOW priority HOST2FAP DQM 
    * classification is based on (skb/fkb)->mark 
    */
   UINT32 dqm;
   
   if ((SKBMARK_GET_FLOW_ID(skbMark)) && ((skbMark & 0x7) == 0x7))
   {
      dqm = DQM_HOST2FAP_XTM_XMIT_Q_HI;
   }
   else 
   {
      dqm = DQM_HOST2FAP_XTM_XMIT_Q_LOW;
   }
   return bcmPktDma_XtmXmitAvailable((BcmPktDma_XtmTxDma *)txdma, dqm);
#else   
   return bcmPktDma_XtmXmitAvailable((BcmPktDma_XtmTxDma *)txdma, 0);
#endif
   
   
}  /* bcmxapi_xmit_available() */ 


/*---------------------------------------------------------------------------
 * int bcmxapi_queue_packet(PTXQINFO pTqi, UINT32 isAtmCell)
 * Description:
 *    Determines whether to queue a packet for transmission based
 *    on the number of total external (ie Ethernet) buffers and
 *    buffers already queued.
 *    For all ATM cells (ASM, OAM which are locally originated and
 *    mgmt based), we allow them to get queued as they are critical
 *    & low frequency based.
 *    For ex., if we drop sucessive ASM cels during congestion (the whole
 *    bonding layer will be reset end to end). So, the criteria here should
 *    be applied more for data packets than for mgmt cells.
 * Returns:
 *    1 to queue packet, 0 to drop packet
 *---------------------------------------------------------------------------
 */
static inline int bcmxapi_queue_packet(PTXQINFO pTqi, UINT32 isAtmCell)
{
   PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
   int nRet = 0; /* default to drop packet */

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
   UINT32 thresh;

   if (gbpm_get_dyn_buf_lvl())
      thresh = pTqi->ulHiThresh;
   else
      thresh = pTqi->ulLoThresh;

   if ((pTqi->ulNumTxBufsQdOne < thresh) || (isAtmCell))
   {
      nRet = 1; /* queue packet */
      pGi->ulDbgQ1++;
   }
   else
   {
      pGi->ulDbgD1++;
      pTqi->ulDropped++;
   }
#else
   if (pGi->ulNumTxQs == 1)
   {
      /* One total transmit queue.  Allow up to 90% of external buffers to
       * be queued on this transmit queue.
       */
      if ((pTqi->ulNumTxBufsQdOne < pGi->ulNumExtBufs90Pct) || isAtmCell)
      {
         nRet = 1; /* queue packet */
         pGi->ulDbgQ1++;
      }
      else
         pGi->ulDbgD1++;
   }
   else
   {
      if (pGi->ulNumExtBufs - pGi->ulNumTxBufsQdAll > pGi->ulNumExtBufsRsrvd)
      {
         /* The available number of external buffers is greater than the
          * reserved value.  Allow up to 50% of external buffers to be
          * queued on this transmit queue.
          */
         if ((pTqi->ulNumTxBufsQdOne < pGi->ulNumExtBufs50Pct) || isAtmCell)
         {
            nRet = 1; /* queue packet */
            pGi->ulDbgQ2++;
         }
         else
            pGi->ulDbgD2++;
      }
      else
      {
         /* Divide the reserved number of external buffers evenly among all
          * of the transmit queues.
          */
         if ((pTqi->ulNumTxBufsQdOne < pGi->ulNumExtBufsRsrvd / pGi->ulNumTxQs)
             || isAtmCell)
         {
            nRet = 1; /* queue packet */
            pGi->ulDbgQ3++;
         }
         else
            pGi->ulDbgD3++;
      }
   }
#endif

   return (nRet);
    
}  /* bcmxapi_queue_packet() */


#ifdef CONFIG_BLOG
/*---------------------------------------------------------------------------
 * void bcmxapi_blog_emit (pNBuff_t pNBuf, struct net_device *dev,
 *                         PBCMXTMRT_DEV_CONTEXT pDevCtx,
 *                         BcmPktDma_XtmTxDma *txdma,
 *                         UINT32 rfc2684Type, UINT16 bufStatus)
 * Description:
 *    Configure BLOG with the egress WAN channel flow information for forwarding.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static inline void bcmxapi_blog_emit (pNBuff_t pNBuf, struct net_device *dev,
                                      PBCMXTMRT_DEV_CONTEXT pDevCtx, 
                                      BcmPktDma_XtmTxDma *txdma, 
                                      UINT32 rfc2684Type, UINT16 bufStatus)
{
   blog_emit(pNBuf, dev, pDevCtx->ulEncapType, txdma->ulDmaIndex,
             BLOG_SET_PHYHDR(rfc2684Type, BLOG_XTMPHY));
}
#endif

/*---------------------------------------------------------------------------
 * int bcmxapi_xmit_packet(pNBuff_t *ppNBuf, UINT8 **ppData, UINT32 *pLen,
 *                         BcmPktDma_XtmTxDma *txdma, UINT32 txdmaIdx,
 *                         UINT16 bufStatus, UINT32 skbMark)
 * Description:
 *    Enqueue the packet to the tx queue specified by txdma for transmission.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static inline int bcmxapi_xmit_packet(pNBuff_t *ppNBuf, UINT8 **ppData, UINT32 *pLen,
                                      BcmPktDma_XtmTxDma *txdma, UINT32 txdmaIdx,
                                      UINT16 bufStatus, UINT32 skbMark, int is_spdsvc_setup_packet,
                                      void *pTcpSpdTstInfo)
{
   int   rc = 0;
   pNBuff_t pNBuf = *ppNBuf ;
   UINT8 *pData = *ppData ;
   UINT32 len = *pLen ;
#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
   UINT32 dqm = 0;
#endif

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
   /* Send the high priority tarffic HOST2FAP HIGH priorty DQM's and other traffic
    * through LOW priority HOST2FAP DQM 
    * classification is based on (skb/fkb)->mark 
    */

   nbuff_flush(pNBuf, pData, len);

   bufStatus |= DMA_SOP | DMA_EOP | DMA_OWN;

   if ((SKBMARK_GET_FLOW_ID(skbMark)) && ((skbMark & 0x7) == 0x7))
   {
      dqm = DQM_HOST2FAP_XTM_XMIT_Q_HI;
   }
   else 
   {
      dqm = DQM_HOST2FAP_XTM_XMIT_Q_LOW;
   }
   
   if (bcmPktDma_tmXtmCheckHighPrio(txdma->ulDmaIndex, SKBMARK_GET_TC_ID(skbMark)) == 1)
   {
      /* this does not look that clear, but it is the quickest way for us to pass
       * the extra high priority information from this level to the real transmit
       * function */
      dqm |= DQM_HOST2FAP_XTM_XMIT_HIGHPRIO;
   }

   if (IS_FKBUFF_PTR(pNBuf))
   {
      FkBuff_t *pFkb = PNBUFF_2_FKBUFF(pNBuf);
      
      /* We can only use the recycle context if this is an xtm or enet buffer */
      if ((pFkb->recycle_hook == (RecycleFuncP)bcmxtmrt_recycle) ||
          (pFkb->recycle_hook == (RecycleFuncP)enet_recycle_hook))
      {
         UINT32 key;
         UINT32 bufSource;
         UINT8  rxChannel = FKB_RECYCLE_CONTEXT(pFkb)->channel;

         if (pFkb->recycle_hook == (RecycleFuncP)bcmxtmrt_recycle)
         {
            /* FKB from XTM */

            key = (UINT32)pNBuf;
#if NUM_FAPS > 1
            /* This assumes that the XTM RX and XTM TX are split between two
               different FAPs. */
            bufSource = HOST_VIA_DQM;
#else
            bufSource = FAP_XTM_RX;
#endif
         }
         else
         {
            /* FKB from Ethernet */

            key = (UINT32)PFKBUFF_TO_PDATA(pFkb, BCM_PKT_HEADROOM);
            
#if NUM_FAPS > 1
            if (g_Eth_rx_iudma_ownership[rxChannel] !=
                              g_Xtm_tx_iudma_ownership[txdmaIdx])
            {
               /* Return buffer to Host for recycling - Jan 24 */
               bufSource = HOST_VIA_DQM;
               /* modify key when free will be done by Host not FAP */
               key = (UINT32)pNBuf;
            }
            else
            {
               bufSource = FAP_ETH_RX;
            }
#else
            bufSource = FAP_ETH_RX;
#endif
         }

         //DUMP_PKT(pData, 32);
         rc = bcmPktDma_XtmXmit(txdma, pData, len, bufSource, bufStatus,
                                key, rxChannel, TXDMATYPE(pDevCtx), 0, dqm,
                                is_spdsvc_setup_packet);
      }
      else
      {
         /* IS FKB, but not an xtm or enet buffer */
        // DUMP_PKT(pData, 32);
         rc = bcmPktDma_XtmXmit(txdma, pData, len, HOST_VIA_DQM, bufStatus,
                                (UINT32)pNBuf, 0, TXDMATYPE(pDevCtx), 0, dqm,
                                is_spdsvc_setup_packet);
      }
   }
   else
   {
      /* IS SKB PTR */
      //DUMP_PKT(pData, 32);
      rc = bcmPktDma_XtmXmit(txdma, pData, len, HOST_VIA_DQM, bufStatus,
                             (UINT32)pNBuf, 0, TXDMATYPE(pDevCtx), 0, dqm,
                             is_spdsvc_setup_packet);
   }
#else
   bufStatus |= DMA_SOP | DMA_EOP | DMA_OWN;

  // DUMP_PKT(pData, 32);

   nbuff_flush(pNBuf, pData, len);

   rc = bcmPktDma_XtmXmit(txdma, pData, len, HOST_VIA_LINUX, bufStatus,
                          (UINT32)pNBuf, 0, TXDMATYPE(pDevCtx), 0, 0, 0);
#endif   /* #if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE) */

   if (rc == 1)
      rc = 0;
   else
      rc = -EIO; 
      
   return rc;
   
}  /* bcmxapi_xmit_packet() */


/*---------------------------------------------------------------------------
 * void bcmxapi_preRxProcessing(UINT32 *pMask)
 * Description:
 *    Disable DQM interrupts and store mask for enabling later.
 * Returns: void
 *---------------------------------------------------------------------------
 */
static inline void bcmxapi_preRxProcessing(UINT32 *pMask)
{
   unsigned long flags;

   /* There are new packets to process.  Capture all enabled interrupts
      and then clear them in the global structure. */
   spin_lock_irqsave(&g_GlobalInfo.xtmlock_rx_regs, flags);
   *pMask = g_GlobalInfo.ulIntEnableMask;
   g_GlobalInfo.ulIntEnableMask = 0;       /* Disable interrupts */
   spin_unlock_irqrestore(&g_GlobalInfo.xtmlock_rx_regs, flags);
}


/*---------------------------------------------------------------------------
 * void bcmxapi_postRxProcessing(UINT32 mask)
 * Description:
 *    Restore interrupt mask so that interrupts can be enabled later.
 * Returns: void
 *---------------------------------------------------------------------------
 */
static inline void bcmxapi_postRxProcessing(UINT32 mask)
{
   unsigned long flags;

   /* Make sure all enabled interrupts are restored to global structure */
   spin_lock_irqsave(&g_GlobalInfo.xtmlock_rx_regs, flags);
   g_GlobalInfo.ulIntEnableMask |= mask;       /* Enable interrupts */
   spin_unlock_irqrestore(&g_GlobalInfo.xtmlock_rx_regs, flags);
}


/*---------------------------------------------------------------------------
 * void bcmxapi_clear_xtmrxint(UINT32 mask)
 * Description:
 *    Clear xtm receive interrupt.
 * Returns: void
 *---------------------------------------------------------------------------
 */
static inline void bcmxapi_clear_xtmrxint(UINT32 mask)
{
   int i;
   BcmXtm_RxDma *rxdma;
   
   for (i = 0; mask && i < MAX_RECEIVE_QUEUES; i++, mask >>= 1)
   {
      if ((mask & 0x01) == 0x01)
      {
         rxdma = g_GlobalInfo.rxdma[i];

#if defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE)
         /* clear and enable fap2host dqm interrupt */
         bcmPktDma_XtmClrRxIrq(&rxdma->pktDmaRxInfo);
#else
         /* reenable rxdma hardware interrupt that was disabled for
          * bcmxtmrt_rxisr handling */
#if defined(CONFIG_ARM)
         rxdma->pktDmaRxInfo.rxDma->intMask = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
#else
         BcmHalInterruptEnable(SAR_RX_INT_ID_BASE+rxdma->pktDmaRxInfo.channel);
#endif
#endif
      }
   }
}  /* bcmxapi_clear_xtmrxint() */



#endif /* _BCMXTMRT_DMA_H_ */

