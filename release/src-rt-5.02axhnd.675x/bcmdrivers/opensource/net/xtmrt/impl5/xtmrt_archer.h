/*
 Copyright 2018 Broadcom Corp. All Rights Reserved.

 <:label-BRCM:2018:DUAL/GPL:standard    
 
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
#ifndef _XTMRT_ARCHER_H_
#define _XTMRT_ARCHER_H_

#define XTM_CPU_RX_QUEUE_SIZE  512    // chosen to match the interface queue size
#if ((XTM_CPU_RX_QUEUE_SIZE & (XTM_CPU_RX_QUEUE_SIZE-1)) != 0)
#error "XTM_CPU_RX_QUEUE_SIZE is not power of 2"
#endif

#define XTM_CPU_TX_QUEUE_SIZE  512
#if ((XTM_CPU_TX_QUEUE_SIZE & (XTM_CPU_TX_QUEUE_SIZE-1)) != 0)
#error "XTM_CPU_TX_QUEUE_SIZE is not power of 2"
#endif

#define XTM_CPU_RECYCLE_Q_SIZE 512
#if ((XTM_CPU_RECYCLE_Q_SIZE & (XTM_CPU_RECYCLE_Q_SIZE-1)) != 0)
#error "XTM_CPU_RECYCLE_Q_SIZE is not power of 2"
#endif


/**** Prototypes ****/

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

int bcmxapi_XtmCreateDevice (uint32_t devId, uint32_t encapType, uint32_t headerLen, uint32_t trailerLen);
int bcmxapi_XtmLinkUp (uint32_t devId, uint32_t matchId);

void bcmxtmrt_recycle_handler (pNBuff_t pNBuff, unsigned long context, uint32_t flag);

extern struct sk_buff *skb_header_alloc(void);
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
   struct sk_buff *skb;
   FkBuff_t       *pFkb = PNBUFF_2_FKBUFF(pNBuf);

   /* allocate skb structure*/
   skb = skb_header_alloc();
   if(!skb)
   {
       return NULL;
   }

   /* initialize the skb */
   skb_headerinit(BCM_PKT_HEADROOM,
#ifdef XTM_CACHE_SMARTFLUSH
                  pFkb->len+delLen+trailerDelLen+BCM_SKB_TAILROOM,
#else
                  BCM_MAX_PKT_LEN,
#endif
                  skb, pFkb->data,
                  (RecycleFuncP)bcmxtmrt_recycle_skb_or_data,
                  0, pFkb->blog_p);

   if (delLen)
       __skb_pull(skb, delLen);

   __skb_trim(skb, pFkb->len);    
   skb->recycle_flags &= SKB_NO_RECYCLE;/* no skb recycle,just do data recyle */

   return skb;
}


/*---------------------------------------------------------------------------
 * FkBuff_t *bcmxapi_fkb_qinit(UINT8 *pBuf, UINT8 *pData,
 *                             UINT32 len, void *rxdma)
 * Description:
 *
 * Returns:
 *    fkb
 *---------------------------------------------------------------------------
 */
static inline FkBuff_t *bcmxapi_fkb_qinit(UINT8 *pBuf, UINT8 *pData,
                                          UINT32 len, void *rxdma)
{
   /* Unused parameters: len, rxdma */
   /* Fkb already initialized. Just update
      pData, len and the recycle fields */
   FkBuff_t *pFkb = PDATA_TO_PFKBUFF(pBuf, BCM_PKT_HEADROOM); 
      
   pFkb->data = pData;
   pFkb->len = len;
   pFkb->dirty_p = NULL;
   pFkb->recycle_hook = (RecycleFuncP)bcmxtmrt_recycle_handler;
   pFkb->recycle_context = 0;
   
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
   return;
}

/*---------------------------------------------------------------------------
 * void bcmxapi_free_xmit_packets(PBCMXTMRT_DEV_CONTEXT pDevCtx)
 * Description:
 *    Free packets that have been transmitted.
 * Returns: void
 *---------------------------------------------------------------------------
 */
static inline void bcmxapi_free_xmit_packets(PBCMXTMRT_DEV_CONTEXT pDevCtx)
{
    return;
}

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
    return 1;
}


/* No pre or post Rx processing needed on Archer platforms */
#define bcmxapi_preRxProcessing(param) 
#define bcmxapi_postRxProcessing(param)

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
 * void bcmxapi_clear_xtmrxint(UINT32 mask)
 * Description:
 *    Clear xtm receive interrupt.
 * Returns: void
 *---------------------------------------------------------------------------
 */
static inline void bcmxapi_clear_xtmrxint(UINT32 mask)
{
   /* Clear interrupts on HI & LO queues */
   //rdpa_cpu_int_enable(RDPA_XTM_CPU_PORT, RDPA_XTM_CPU_HI_RX_QUEUE_ID);
   //rdpa_cpu_int_enable(RDPA_XTM_CPU_PORT, RDPA_XTM_CPU_LO_RX_QUEUE_ID);
}  /* bcmxapi_clear_xtmrxint() */

int bcmxapi_queue_packet(PTXQINFO pTqi, UINT32 isAtmCell);
int bcmxapi_xmit_packet(pNBuff_t *ppNBuf, UINT8 **ppData, UINT32 *pLen,
                        BcmPktDma_XtmTxDma *txdma, UINT32 txdmaIdx,
                        UINT16 bufStatus, UINT32 skbMark, int is_spdsvc_setup_packet,
                        void *pTcpSpdTstInfo);

#endif /* _XTMRT_ARCHER_H_ */

