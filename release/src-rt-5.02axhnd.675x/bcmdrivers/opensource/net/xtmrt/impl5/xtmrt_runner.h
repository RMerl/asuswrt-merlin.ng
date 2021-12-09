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
#ifndef _BCMXTM_RUNNER_H_
#define _BCMXTM_RUNNER_H_

#include "xtmrt_runner_ex.h"
#if (defined(CONFIG_BCM_SPDTEST) || defined(CONFIG_BCM_SPDTEST_MODULE))
#include "linux/bcm_log.h"
#include "tcpspdtest_defs.h"
extern bcmFun_t *xtmrt_tcpspdtest_connect;
#endif

#if defined(CONFIG_BCM_XRDP)
#define RDPA_XTM_CPU_RX_QUEUE_SIZE  1024
#else
#define RDPA_XTM_CPU_RX_QUEUE_SIZE  256
#endif

#define bcmxapi_XtmCreateDevice(_devId, _encapType, _headerLen, _trailerLen)
#define bcmxapi_XtmLinkUp(_devId, _matchId)

#define XTM_RDPA_CPU_TC_LOW      0
#define XTM_RDPA_CPU_TC_HIGH     1

#define XTM_RNR_CPU_RX_QUEUES    2

/**** Prototypes ****/

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


/**** Inline functions ****/

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
                  (RecycleFuncP)bdmf_sysb_recycle,
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
   /* Fkb already initialized in rdpa_get_pkt_from_ring(). Just update
      pData, len and the recycle fields */
   FkBuff_t *pFkb = PDATA_TO_PFKBUFF(pBuf, BCM_PKT_HEADROOM); 
      
   pFkb->data = pData;
   pFkb->len = len;
   pFkb->dirty_p = NULL;
   pFkb->recycle_hook = (RecycleFuncP)bdmf_sysb_recycle;
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
   return 1;
}

/* No pre or post Rx processing needed on runner platforms */
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
   UINT32 ctType;
   UINT16 wanChannelFlow;

   ctType  = ((UINT32)bufStatus & FSTAT_CT_MASK) >> FSTAT_CT_SHIFT ;
   wanChannelFlow = (MAX_TRANSMIT_QUEUES * txdma->ulDmaIndex) + ctType ;

   blog_emit(pNBuf, dev, pDevCtx->ulEncapType, wanChannelFlow,
             BLOG_SET_PHYHDR(rfc2684Type, BLOG_XTMPHY));
}
#endif

/*---------------------------------------------------------------------------
 * int bcmxapi_xmit_packet(pNBuff_t *ppNBuf, UINT8 **ppData, UINT32 *pLen,
 *                         BcmPktDma_XtmTxDma *txdma, UINT32 txdmaIdx,
 *                         UINT16 bufStatus, UINT32 skbMark,
 *                         int is_spdsvc_setup_packet, void *pTcpSpdTstInfo) 
 * Description:
 *    Enqueue the packet to the tx queue specified by txdma for transmission.
 *    Function to suit in runner based architecture.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
static inline int bcmxapi_xmit_packet(pNBuff_t *ppNBuf, UINT8 **ppData, UINT32 *pLen,
                                      BcmPktDma_XtmTxDma *txdma, UINT32 txdmaIdx,
                                      UINT16 bufStatus, UINT32 skbMark, int is_spdsvc_setup_packet,
                                      void *pTcpSpdTstInfo)
{
   int rc;
   bdmf_sysb  pDataBuf ;
   UINT32 ctType;
   UINT16 wanFlow;
   rdpa_cpu_tx_extra_info_t extra_info;
   extra_info.u32 = 0;

   ctType  = ((UINT32)bufStatus & FSTAT_CT_MASK) >> FSTAT_CT_SHIFT;
   wanFlow = (MAX_TRANSMIT_QUEUES * txdmaIdx) + ctType;
      
   pDataBuf = *ppNBuf;

   extra_info.is_spdsvc_setup_packet = is_spdsvc_setup_packet;

   rc = rdpa_cpu_tx_port_enet_or_dsl_wan(pDataBuf, txdmaIdx, (rdpa_flow) wanFlow, rdpa_wan_type_to_if (rdpa_wan_dsl), extra_info);
   if (rc != 0)
   {
      printk(KERN_NOTICE "rdpa_cpu_tx_port_enet_or_dsl_wan() for XTM port "
          "returned %d (wan_flow: %d queue_id: %u)\n", rc, wanFlow, (unsigned int) txdmaIdx);
          
      /* Buffer is already released by rdpa_cpu_tx_port_enet_or_dsl_wan() */
      return -1;
   }
   
   return rc;
   
}  /* bcmxapi_xmit_packet() */


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
   rdpa_cpu_int_enable(XTM_RDPA_CPU_PORT, RDPA_XTM_CPU_HI_RX_QUEUE_ID);
   rdpa_cpu_int_enable(XTM_RDPA_CPU_PORT, RDPA_XTM_CPU_LO_RX_QUEUE_ID);
}  /* bcmxapi_clear_xtmrxint() */


#endif /* _BCMXTM_RUNNER_H_ */
