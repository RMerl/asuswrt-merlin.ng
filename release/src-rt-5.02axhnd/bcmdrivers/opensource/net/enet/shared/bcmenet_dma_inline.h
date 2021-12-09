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
#ifndef _BCMENET_DMA_INLINE_H_
#define _BCMENET_DMA_INLINE_H_

#if defined(_CONFIG_BCM_FAP)
#include <linux/bcm_skb_defines.h>
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0))
#include "kmap_skb.h"
#else
#include "linux/highmem.h"
#endif

#if defined(_CONFIG_ENET_BCM_TM)
#include "bcm_tm_defs.h"
static bcmFun_t *enet_bcm_tm_enqueue = NULL;
#endif

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
#include "spdsvc_defs.h"
static bcmFun_t *enet_spdsvc_transmit = NULL;
#endif

#ifdef _BCMENET_LOCAL_   /* Local definitions not to be used outside Ethernet driver */
/*---------------------------------------------------------------------*/
/* specify number of BDs and buffers to use                            */
/*---------------------------------------------------------------------*/
/* In order for ATM shaping to work correctly,
 * the number of receive BDS/buffers = # tx queues * # buffers per tx queue
 * (80 ENET buffers = 8 tx queues * 10 buffers per tx queue)
 */
#define ENET_CACHE_SMARTFLUSH

/* misc. configuration */
#define DMA_FC_THRESH_LO        5
#define DMA_FC_THRESH_HI        (NR_RX_BDS_MIN / 2)

int phy_port_to_logic_port(int port, int unit);

/*
 * device context
 */ 

/* Accessor functions */

#define DO_DELAYED_RECLAIM(arrayIdx, arrayPtr) \
    do { \
        uint32 tmp_idx=0; \
        while (tmp_idx < (arrayIdx)) { \
            nbuff_free((pNBuff_t) (arrayPtr)[tmp_idx]); \
            tmp_idx++; } \
    } while (0)

#if ENET_RX_CHANNELS_MAX > 1   /* No need to select NEXT_CHANNEL if there is only one channel */
#define  SELECT_NEXT_CHANNEL()                                  \
{                                                               \
    if (scheduling == SP_SCHEDULING) {                          \
        /* Channel-X is done, so move on to channel X-1 */            \
        /* The strict priority is Ch3, Ch2, Ch1, and Ch0 */              \
        global_channel--;                                       \
        if (global_channel < 0) {                               \
            *rxpktgood |= ENET_POLL_DONE;                       \
            return BCMEAPI_CTRL_BREAK;                          \
        }                                                       \
        return BCMEAPI_CTRL_CONTINUE;                           \
    } else if (scheduling == FAP_SCHEDULING) {                  \
        /* we only look at one channel */                            \
        *rxpktgood |= ENET_POLL_DONE;                            \
        return BCMEAPI_CTRL_BREAK;                              \
    } else {    /* WRR */                                                \
        if(--active_channels <= 0) { \
            wrr_rx_reset(); /* Reset WRR for the next NAPI cycle */ \
            *rxpktgood |= ENET_POLL_DONE;                        \
            return BCMEAPI_CTRL_BREAK;                          \
        }                                                       \
        if(--channels_tbd <= 0) { \
            wrr_rx_reset(); /* Reset WRR for the next WRR cycle in the same NAPI cycle */ \
            return BCMEAPI_CTRL_SKIP; \
        } \
        /* Replace the channel done with the last channel in the                  \
                  channels to be done */                                   \
        next_channel[channel_ptr] = next_channel[channels_tbd]; \
        pending_ch_tbd--;   \
        pending_channel[channel_ptr] = pending_channel[pending_ch_tbd]; \
        /* goto next_channel;  */                                    \
        bcm_next_channel(); \
    }                                                           \
}
#else  /* Only one or less RX channels */
#define  SELECT_NEXT_CHANNEL()    { *rxpktgood |= ENET_POLL_DONE; return BCMEAPI_CTRL_BREAK;}
#endif 

#if defined(_CONFIG_BCM_FAP)
inline static void bcm63xx_enet_recycle_skb_or_data_wl_tx_chain(struct sk_buff *skb,
														 uint32 context, uint32 free_flag);
#endif
inline static void bcm63xx_enet_recycle(pNBuff_t pNBuff, uint32 context, uint32 flags);
static inline int bcmeapi_pkt_xmt_dispatch(EnetXmitParams *pParam);
static inline void bcmeapi_xmit_unlock_exit_post(EnetXmitParams *pXmitParams);
static inline void bcmeapi_xmit_unlock_drop_exit_post(EnetXmitParams *pXmitParams);
static inline void bcmeapi_prepare_rx(void);
static inline void bcmeapi_kfree_buf_irq(BcmEnet_devctrl *pDevCtrl, struct fkbuff  *pFkb, unsigned char *pBuf);
#if !defined(CONFIG_BCM947189)
static inline void bcmeapiPhyIntEnable(int enable);
#endif
void bcmeapi_enet_poll_timer(void);
static inline void bcmeapi_select_tx_def_queue(EnetXmitParams *param);
static inline void bcmeapi_select_tx_nodef_queue(EnetXmitParams *param);
static inline void bcmeapi_get_tx_queue(EnetXmitParams *pParam);
static inline void bcmeapi_buf_reclaim(EnetXmitParams *pParam);
static inline int bcmeapi_queue_select(EnetXmitParams *pParam);
static inline void bcmeapi_config_tx_queue(EnetXmitParams *pParam);
static inline void bcmeapi_update_rx_queue(BcmEnet_devctrl *pDevCtrl);
static inline void bcmeapi_napi_leave(struct BcmEnet_devctrl *pDevCtrl);
/* Defined only if more than one channel and FAP supported */
#if ENET_RX_CHANNELS_MAX > 1 && defined(_CONFIG_BCM_FAP)
static inline int bcmeapi_prepare_next_rx(uint32 *rxpktgood);
#else
#define bcmeapi_prepare_next_rx(rxpktgood) BCMEAPI_CTRL_CONTINUE
#endif
static inline void bcmeapi_rx_post(uint32 *rxpktgood);
#if !(defined(_CONFIG_BCM_BPM) || defined(_CONFIG_BCM_FAP))
#define   bcmeapi_set_fkb_recycle_hook(pFkb)
#else
static inline void bcmeapi_set_fkb_recycle_hook(FkBuff_t * pFkb);
#endif
static inline void bcmeapi_blog_drop(BcmEnet_devctrl *pDevCtrl, struct fkbuff  *pFkb, unsigned char *pBuf);
static inline void bcmeapi_buf_alloc(BcmEnet_devctrl *pDevCtrl);
static inline int bcmeapi_alloc_skb(BcmEnet_devctrl *pDevCtrl, struct sk_buff **skb);
static inline int bcmeapi_skb_headerinit(int len, BcmEnet_devctrl *pDevCtrl, struct sk_buff *skb, 
    FkBuff_t * pFkb, unsigned char *pBuf);
static inline void flush_assign_rx_buffer(BcmEnet_devctrl *pDevCtrl, int channel,
                                   uint8 * pData, uint8 * pEnd);


/*
 * Recycling context definition
 */
typedef union {
    struct {
        /* fapQuickFree handling removed - Oct 2010 */
#if defined(CONFIG_BCM_GMAC)
        uint32 reserved     : 29;
        uint32 channel      :  3;
#else
        uint32 reserved     : 30;
        uint32 channel      :  2;
#endif
    };
    uint32 u32;
} enet_recycle_context_t;
#define RECYCLE_CONTEXT(_context)  ( (enet_recycle_context_t *)(&(_context)) )
#define FKB_RECYCLE_CONTEXT(_pFkb) RECYCLE_CONTEXT((_pFkb)->recycle_context)

#if defined(_CONFIG_BCM_BPM)
static inline int enet_bpm_alloc_buf(BcmEnet_devctrl *pDevCtrl, int channel);
static void * eth_alloc_buf_addr[BPM_ENET_BULK_ALLOC_COUNT];
/* Allocates BPM_ENET_BULK_ALLOC_COUNT number of bufs and assigns to the
 * DMA ring of an RX channel. The allocation is done in groups for
 * optimization.
 */
static inline int enet_bpm_alloc_buf(BcmEnet_devctrl *pDevCtrl, int channel)
{
    unsigned char *pData;
    FkBuff_t *pFkBuf ;
    int buf_ix;
    void **pDataBufs = eth_alloc_buf_addr;
    BcmPktDma_EthRxDma *rxdma = &pDevCtrl->rxdma[channel]->pktDmaRxInfo;

    if((rxdma->numRxBds - rxdma->rxAssignedBds) >= rxdma->allocTrig)
    { /* number of used buffers has crossed the trigger threshold */
        if ( (gbpm_alloc_mult_buf( rxdma->bulkAlloc, (void **)pDataBufs ) )
                == GBPM_ERROR)
        {
            /* may be temporarily global buffer pool is depleted.
             * Later try again */
            return GBPM_ERROR;
        }

        rxdma->alloc += rxdma->bulkAlloc;

        for (buf_ix=0; buf_ix < rxdma->bulkAlloc; buf_ix++)
        {
            pData  = (uint8_t *)pDataBufs[buf_ix];
            pFkBuf = PDATA_TO_PFKBUFF(pData, BCM_PKT_HEADROOM);

            flush_assign_rx_buffer(pDevCtrl, channel, pData,
                    (uint8_t*)pFkBuf + BCM_PKTBUF_SIZE);
        }
    }

    return GBPM_SUCCESS;
}

#endif

static inline void _assign_rx_buffer(BcmEnet_devctrl *pDevCtrl, int channel, uint8 * pData);
/*
 * flush_assign_rx_buffer: Cache invalidates before assigning buffer to RxBd
 * Subtle point: flush means write back and invalidate.  Doing invalidate
 * is not good enough because the dirty bit in the cache line tag can still
 * be set, and MIPS will still want to write back that line even though the
 * valid bit has been cleared.
 *   pData: Points to rx DMAed buffer
 *   pEnd : demarcates the end of the buffer that may have cache lines that
 *          need to be invalidated.
 *  if ( round_down_cacheline(pData) == round_up_cacheline(pEnd) ) no flush.
 */
static inline void flush_assign_rx_buffer(BcmEnet_devctrl *pDevCtrl, int channel,
                                   uint8 * pData, uint8 * pEnd)
{
    cache_flush_region(pData, pEnd);
    _assign_rx_buffer( pDevCtrl, channel, pData );
}

#define DELAYED_RECLAIM_ARRAY_LEN 8

#if defined(_CONFIG_BCM_FAP)
static inline int fapTxChannelFkb(FkBuff_t *pFkb)
{
    int txChannel;

#if defined(CONFIG_BCM963268)
    FkBuff_t *pFkbMaster = _get_fkb_master_ptr_(pFkb);

    if(pFkbMaster->recycle_hook == (RecycleFuncP)bcm63xx_enet_recycle)
    {
        /* from ETH (from LAN or WAN) */
        /* Always transmit an ethernet packet back to the same iuDMA channel it was
           received from. This is needed to avoid buffer recycling across FAPs */
        txChannel = FKB_RECYCLE_CONTEXT(pFkb)->channel;
    }
#if defined (_CONFIG_BCM_XTMCFG)
    else if(pFkbMaster->recycle_hook == xtm_fkb_recycle_hook)
    {
        /* from XTM (from WAN) */
        txChannel = PKTDMA_ETH_DS_IUDMA;
    }
#endif
    else
    {
        /* unknown source, e.g. USB (from LAN) */
        txChannel = PKTDMA_ETH_US_IUDMA;
    }

#endif

    return txChannel;
}

static inline int fapTxChannelSkb(struct sk_buff *skb)
{
    int txChannel;

#if defined(CONFIG_BCM963268)

    if(skb->recycle_hook == (RecycleFuncP)bcm63xx_enet_recycle_skb_or_data)
    {
        /* from ETH (from LAN or WAN) */
        /* Always transmit an ethernet packet back to the same iuDMA channel it was
           received from. This is needed to avoid buffer recycling across FAPs */
        txChannel = RECYCLE_CONTEXT(skb->recycle_context)->channel;
    }
#if defined (_CONFIG_BCM_XTMCFG)
    else if(skb->recycle_hook == xtm_skb_recycle_hook)
    {
        /* from XTM (from WAN) */
        txChannel = PKTDMA_ETH_DS_IUDMA;
    }
#endif
    else
    {
        /* unknown source, e.g. USB (from LAN) */
        txChannel = PKTDMA_ETH_US_IUDMA;
    }

#endif

    return txChannel;
}
#endif /* defined(_CONFIG_BCM_FAP) */

static inline void bcmeapi_enet_prepare_xmit(struct net_device *dev, uint32_t *mark)
{
}

static inline void bcmeapi_get_tx_queue(EnetXmitParams *pParam)
{
#if defined(_CONFIG_BCM_FAP)
    if(IS_FKBUFF_PTR(pParam->pNBuff))
    {
        pParam->channel = fapTxChannelFkb(pParam->pFkb);
    }
    else
    {
        pParam->channel = fapTxChannelSkb(pParam->skb);
    }
#endif

#if defined(CONFIG_BCM_GMAC)
#if defined(_CONFIG_BCM_FAP)
    /* If WAN Port send to US TX IuDMA channel */
    if ( IsLogPortWan(pParam->port_id) )
    {
        pParam->channel = PKTDMA_ETH_US_TX_IUDMA;
    }
    else
    {
        pParam->channel = PKTDMA_ETH_DS_TX_IUDMA;
    }
#else
    if ( IsGmacInfoActive && IsGmacPort( pParam->port_id ) )
    {
        pParam->channel = GMAC_LOG_CHAN;
    }
#endif
#endif
#if defined(CONFIG_BCM947189)
    pParam->channel = pParam->pDevPriv->emac_core;
#endif
    BCM_ENET_TX_DEBUG("chan=%d port_id=%d\n", pParam->channel, pParam->port_id);
    BCM_ENET_TX_DEBUG("Send len=%d \n", pParam->len);
}

static inline void bcmeapi_xmit_unlock_exit_post(EnetXmitParams *pXmitParams)
{
    ENET_TX_UNLOCK();
    DO_DELAYED_RECLAIM(pXmitParams->reclaim_idx, pXmitParams->delayed_reclaim_array);
}

static inline void bcmeapi_xmit_unlock_drop_exit_post(EnetXmitParams *pXmitParams)
{
    ENET_TX_UNLOCK();
    DO_DELAYED_RECLAIM(pXmitParams->reclaim_idx, pXmitParams->delayed_reclaim_array);
    nbuff_flushfree(pXmitParams->pNBuff);
}

#if !defined(CONFIG_BCM947189)
static inline void bcmeapiPhyIntEnable(int enable)
{
    if(enable)
    {
        BcmHalInterruptEnable(INTERRUPT_ID_EPHY);
#if defined(CONFIG_BCM963268)
        BcmHalInterruptEnable(INTERRUPT_ID_GPHY);
#endif
#if defined(CONFIG_BCM_GMAC)
        BcmHalInterruptEnable(INTERRUPT_ID_GMAC);
#endif
    }
    else
    {
        BcmHalInterruptDisable(INTERRUPT_ID_EPHY);
#if defined(CONFIG_BCM963268)
        BcmHalInterruptDisable(INTERRUPT_ID_GPHY);
#endif
#if defined(CONFIG_BCM_GMAC)
        BcmHalInterruptDisable(INTERRUPT_ID_GMAC);
#endif
    }
}
#endif

static inline int bcmeapi_skb_headerinit(int len, BcmEnet_devctrl *pDevCtrl, struct sk_buff *skb, 
    FkBuff_t * pFkb, unsigned char *pBuf)
{
    uint32 recycle_context = 0;

    RECYCLE_CONTEXT(recycle_context)->channel = global_channel;

    skb_headerinit(BCM_PKT_HEADROOM,
#if defined(ENET_CACHE_SMARTFLUSH)
            SKB_DATA_ALIGN(len+BCM_SKB_TAILROOM),
#else
            BCM_MAX_PKT_LEN,
#endif
            skb, pBuf, (RecycleFuncP)bcm63xx_enet_recycle_skb_or_data,
            recycle_context,(void*)pFkb->blog_p);

    skb_trim(skb, len - ETH_CRC_LEN);

    return BCMEAPI_CTRL_TRUE;
}

static inline void bcmeapi_buf_alloc(BcmEnet_devctrl *pDevCtrl)
{
#if defined(_CONFIG_BCM_BPM)
    //alloc a new buf from bpm
#if defined(_CONFIG_BCM_FAP)
#else
    enet_bpm_alloc_buf( pDevCtrl, global_channel );
#endif
#endif
}

static inline int bcmeapi_free_skb(BcmEnet_devctrl *pDevCtrl, 
    struct sk_buff *skb, int free_flag, int channel)
{
    BcmEnet_RxDma * rxdma;

    if( !(free_flag & SKB_RECYCLE ))
    {
        return BCMEAPI_CTRL_FALSE;
    }

    ENET_SKBLIST_LOCK();

    rxdma = pDevCtrl->rxdma[channel];
    if ((unsigned char *)skb < rxdma->skbs_p || (unsigned char *)skb >= rxdma->end_skbs_p)
    {
        kmem_cache_free(enetSkbCache, skb);
    }
    else
    {
        skb->next_free = rxdma->freeSkbList;
        rxdma->freeSkbList = skb;      
    }

    ENET_SKBLIST_UNLOCK();

    return BCMEAPI_CTRL_TRUE;
}

static inline void bcmeapi_select_tx_def_queue(EnetXmitParams *pParam)
{
#if !defined(_CONFIG_BCM_FAP)
    if (use_tx_dma_channel_for_priority)
    {
        pParam->channel = channel_for_queue[pParam->egress_queue] % cur_txdma_channels;
    } else {
        pParam->channel = SKBMARK_GET_Q_CHANNEL(pParam->mark) % cur_txdma_channels;
    }
    BCM_ENET_TX_DEBUG("The Tx channel is %d \n", pParam->channel);
#endif
}

static inline void bcmeapi_select_tx_nodef_queue(EnetXmitParams *pParam)
{
#if !defined(_CONFIG_BCM_FAP)
    pParam->channel = SKBMARK_GET_Q_CHANNEL(pParam->mark) % cur_txdma_channels;
#endif
}

#if defined(_CONFIG_BCM_INGQOS)
extern uint32_t iqos_enable_g;
extern uint32_t iqos_cpu_cong_g;
extern uint32_t iqos_debug_g;

inline static void enet_iq_update_cong_status(BcmEnet_devctrl *pDevCtrl)
{
    int chnl;
    int thrOfst;
    DmaDesc  dmaDesc;
    volatile DmaDesc *rxBd_pv;
    BcmPktDma_EthRxDma  *rxdma;

    for (chnl = 0; chnl < cur_rxdma_channels; chnl++)
    {
        rxdma = &pDevCtrl->rxdma[chnl]->pktDmaRxInfo;

#if defined(_CONFIG_BCM_FAP)
        if (g_Eth_rx_iudma_ownership[chnl] != HOST_OWNED)
            continue;
#endif

        if (iqos_get_cong_status(IQOS_IF_ENET, chnl) == IQOS_CONG_STATUS_HI)
        {
            /* calculate low threshold ring offset */
            thrOfst = rxdma->rxTailIndex + rxdma->iqLoThresh;

            if (thrOfst >= rxdma->numRxBds)
                thrOfst %= rxdma->numRxBds;

            /* Get the status from Rx BD */
            rxBd_pv = &rxdma->rxBds[thrOfst];
            dmaDesc.word0 = rxBd_pv->word0;

            if ((dmaDesc.status & DMA_OWN) == DMA_OWN)
            { /* low thresh crossed on downside */
                iqos_set_cong_status(IQOS_IF_ENET, chnl, IQOS_CONG_STATUS_LO);
            }
        }
        else
        {
            /* calculate high threshold ring offset */
            thrOfst = rxdma->rxTailIndex + rxdma->iqHiThresh;

            if (thrOfst >= rxdma->numRxBds)
                thrOfst %= rxdma->numRxBds;

            /* Get the status from Rx BD */
            rxBd_pv = &rxdma->rxBds[thrOfst];
            dmaDesc.word0 = rxBd_pv->word0;

            if ((dmaDesc.status & DMA_OWN) == 0)
            {/* high thresh crossed on upside */
                iqos_set_cong_status(IQOS_IF_ENET, chnl, IQOS_CONG_STATUS_HI);
            }
        }
    }
}
/* Update CPU congestion status based on the DQM IQ thresholds */
/* FAP get Eth DQM queue length handler hook */
#if defined(_CONFIG_BCM_FAP)
extern iqos_fap_ethRxDqmQueue_hook_t iqos_fap_ethRxDqmQueue_hook_g;
inline static void enet_iq_dqm_update_cong_status(BcmEnet_devctrl *pDevCtrl)
{
    int chnl;
    int iqDepth;

    if (iqos_fap_ethRxDqmQueue_hook_g == NULL)
        return;

    for (chnl = 0; chnl < cur_rxdma_channels; chnl++)
    {
        BcmPktDma_EthRxDma  *rxdma = &pDevCtrl->rxdma[chnl]->pktDmaRxInfo;

        if (g_Eth_rx_iudma_ownership[chnl] == HOST_OWNED)
            continue;

        /* get the DQM queue depth */
        iqDepth = iqos_fap_ethRxDqmQueue_hook_g( chnl );

        if (iqDepth >= rxdma->iqHiThreshDqm)
        {/* high thresh crossed on upside */
            iqos_set_cong_status(IQOS_IF_ENET, chnl, IQOS_CONG_STATUS_HI);
        }
        else if (iqDepth < rxdma->iqLoThreshDqm)
        {/* low thresh crossed on downside */
            iqos_set_cong_status(IQOS_IF_ENET, chnl, IQOS_CONG_STATUS_LO);
        }
        /* else donot change the congestion status */
    }
}

#endif
#endif

static inline void bcmeapi_update_rx_queue(BcmEnet_devctrl *pDevCtrl)
{
#if defined(_CONFIG_BCM_INGQOS)
    if (iqos_enable_g)
    {
        /* update the CPU congestion status
         * FAP    : use the DQM queue length
         * non-FAP: use the RX DMA ring length
         */
#if defined(_CONFIG_BCM_FAP)
        enet_iq_dqm_update_cong_status(pDevCtrl);
#endif
        enet_iq_update_cong_status(pDevCtrl);
    }
#endif
}

static inline void bcmeapi_napi_leave(struct BcmEnet_devctrl *pDevCtrl)
{
#if defined(_CONFIG_BCM_FAP)
    int channel;
    BcmEnet_RxDma *rxdma;

    ENET_RX_LOCK();

    /* Add check for 1 iuDMA channel limiting others by hitting the no BDs condition - Sept 2010 */
    for (channel = 0; channel < cur_rxdma_channels; channel++)
    {

        rxdma = pDevCtrl->rxdma[channel];
    }

    ENET_RX_UNLOCK();
#endif
}

#if (defined(_CONFIG_BCM_BPM) || defined(_CONFIG_BCM_FAP))
/* Why should this be compilation dependent - TBD ? */
static inline void bcmeapi_set_fkb_recycle_hook(FkBuff_t * pFkb)
{
    {
        uint32 context = 0;

        RECYCLE_CONTEXT(context)->channel = global_channel;

        pFkb->recycle_hook = (RecycleFuncP)bcm63xx_enet_recycle;
        pFkb->recycle_context = context;
    }
}
#endif

#if defined(_CONFIG_BCM_BPM)
/* Frees a buffer for an Eth RX channel to global BPM */
static inline int enet_bpm_free_buf(BcmEnet_devctrl *pDevCtrl, int channel,
        uint8 *pData)
{
    BcmPktDma_EthRxDma *rxdma = &pDevCtrl->rxdma[channel]->pktDmaRxInfo;
    gbpm_free_buf((void *)pData);
    rxdma->free++;

    return GBPM_SUCCESS;
}
#endif  /* defined(_CONFIG_BCM_BPM)  */

extern int bcmenet_in_init_dev;
/* _assign_rx_buffer: Reassigns a free data buffer to RxBD. No flushing !!! */
static inline void _assign_rx_buffer(BcmEnet_devctrl *pDevCtrl, int channel, uint8 * pData)
{
    BcmPktDma_EthRxDma *pktDmaRxInfo_p =
                                &pDevCtrl->rxdma[channel]->pktDmaRxInfo;

    int buf_freed = 0;

    ENET_RX_LOCK();

#if defined(_CONFIG_BCM_BPM)
#if defined(_CONFIG_BCM_FAP)
    if(0)
#endif
    {
        if (pktDmaRxInfo_p->numRxBds == pktDmaRxInfo_p->rxAssignedBds)
        {
            buf_freed = 1;
            enet_bpm_free_buf(pDevCtrl, channel, pData);
        }
    }
#endif
    if (buf_freed == 0)
    {
        bcmPktDma_EthFreeRecvBuf(pktDmaRxInfo_p, pData);
    }

#if defined(_CONFIG_BCM_INGQOS)
    /* Update congestion status, once all the buffers have been recycled. */
    if (iqos_cpu_cong_g)
    {
        if (pktDmaRxInfo_p->numRxBds == pktDmaRxInfo_p->rxAssignedBds)
            iqos_set_cong_status(IQOS_IF_ENET, channel, IQOS_CONG_STATUS_LO);
    }
#endif

#if defined(_CONFIG_BCM_FAP)
    /* Delay is only needed when the free is actually being done by the FAP */
    if(bcmenet_in_init_dev)
       udelay(20);
#endif

    ENET_RX_UNLOCK();
}

/* Callback: fkb and data recycling */
static inline void __bcm63xx_enet_recycle_fkb(struct fkbuff * pFkb,
                                              uint32 context)
{
    int channel = FKB_RECYCLE_CONTEXT(pFkb)->channel;
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)netdev_priv(vnet_dev[0]);
    uint8 *pData = PFKBUFF_TO_PDATA(pFkb,BCM_PKT_HEADROOM);

    _assign_rx_buffer(pDevCtrl, channel, pData); /* No cache flush */
}

/* Common recycle callback for fkb, skb or data */
static inline void bcm63xx_enet_recycle(pNBuff_t pNBuff, uint32 context, uint32 flags)
{
    if ( IS_FKBUFF_PTR(pNBuff) ) {
        __bcm63xx_enet_recycle_fkb(PNBUFF_2_FKBUFF(pNBuff), context);
    } else { /* IS_SKBUFF_PTR(pNBuff) */
        bcm63xx_enet_recycle_skb_or_data(PNBUFF_2_SKBUFF(pNBuff),context,flags);
    }
}

/*
 * This function is exact copy of bcm63xx_enet_recycle_skb_or_data_wl_tx_chain; Any bug fixes should be done in both 
 */
static inline void bcm63xx_enet_recycle_skb_or_data(struct sk_buff *skb,
                                             uint32 context, uint32 free_flag)
{
    int channel  = RECYCLE_CONTEXT(context)->channel;
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)netdev_priv(vnet_dev[0]);

    if (bcmeapi_free_skb(pDevCtrl, skb, free_flag, channel) != BCMEAPI_CTRL_TRUE)
    { // free data
        uint8 *pData = skb->head + BCM_PKT_HEADROOM;
        uint8 *pEnd;
#if defined(ENET_CACHE_SMARTFLUSH)
        uint8 *dirty_p = skb_shinfo(skb)->dirty_p;
        uint8 *shinfoBegin = (uint8 *)skb_shinfo(skb);
        uint8 *shinfoEnd;
        if (skb_shinfo(skb)->nr_frags == 0) {
            // no frags was used on this skb, so can shorten amount of data
            // flushed on the skb_shared_info structure
            shinfoEnd = shinfoBegin + offsetof(struct skb_shared_info, frags);
        }
        else {
            shinfoEnd = shinfoBegin + sizeof(struct skb_shared_info);
        }
        cache_flush_region(shinfoBegin, shinfoEnd);

        // If driver returned this buffer to us with a valid dirty_p,
        // then we can shorten the flush length.
        if (dirty_p) {
            if ((dirty_p < skb->head) || (dirty_p > shinfoBegin)) {
                printk("invalid dirty_p detected: %p valid=[%p %p]\n",
                        dirty_p, skb->head, shinfoBegin);
                pEnd = shinfoBegin;
            } else {
                pEnd = (dirty_p < pData) ? pData : dirty_p;
            }
        } else {
            pEnd = shinfoBegin;
        }
#else
        pEnd = pData + BCM_MAX_PKT_LEN;
#endif
        flush_assign_rx_buffer(pDevCtrl, channel, pData, pEnd);
    }
}

#if defined(_CONFIG_BCM_FAP)
/*
 * This function is exact copy of bcm63xx_enet_recycle_skb_or_data; Any bug fixes should be done in both 
 */
inline static void bcm63xx_enet_recycle_skb_or_data_wl_tx_chain(struct sk_buff *skb,
														 uint32 context, uint32 free_flag)
{
    int channel  = RECYCLE_CONTEXT(context)->channel;
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)netdev_priv(vnet_dev[0]);

    if (bcmeapi_free_skb(pDevCtrl, skb, free_flag, channel) != BCMEAPI_CTRL_TRUE)
    { // free data
        uint8 *pData = skb->head + BCM_PKT_HEADROOM;
        uint8 *pEnd;
#if defined(ENET_CACHE_SMARTFLUSH)
        uint8 *dirty_p = skb_shinfo(skb)->dirty_p;
        uint8 *shinfoBegin = (uint8 *)skb_shinfo(skb);
        uint8 *shinfoEnd;
        if (skb_shinfo(skb)->nr_frags == 0) {
            // no frags was used on this skb, so can shorten amount of data
            // flushed on the skb_shared_info structure
            shinfoEnd = shinfoBegin + offsetof(struct skb_shared_info, frags);
        }
        else {
            shinfoEnd = shinfoBegin + sizeof(struct skb_shared_info);
        }
        cache_flush_region(shinfoBegin, shinfoEnd);

        // If driver returned this buffer to us with a valid dirty_p,
        // then we can shorten the flush length.
        if (dirty_p) {
            if ((dirty_p < skb->head) || (dirty_p > shinfoBegin)) {
                printk("invalid dirty_p detected: %p valid=[%p %p]\n",
                        dirty_p, skb->head, shinfoBegin);
                pEnd = shinfoBegin;
            } else {
                pEnd = (dirty_p < pData) ? pData : dirty_p;
            }
        } else {
            pEnd = shinfoBegin;
        }
#else
        pEnd = pData + BCM_MAX_PKT_LEN;
#endif
		/* Flush the start of headroom used by TX chaining logic */
		cache_flush_region(skb->head, skb->head+L1_CACHE_BYTES);
        flush_assign_rx_buffer(pDevCtrl, channel, pData, pEnd);
    }
}
#endif

static inline void wrr_rx_reset(void)
{
    int i;
#ifdef BCM_ENET_DEBUG_BUILD
    ch_serviced[NEXT_INDEX(dbg_index)] = WRR_RELOAD;
#endif
    /* reload the pending_weight_pkts[] array */
    /* reload the next_channel[] array */
    for(i = 0; i < cur_rxdma_channels; i++) {
        pending_weight_pkts[i] = weight_pkts[i];
        pending_channel[i] = i;
        next_channel[i] = pending_channel[i];
    }
    /* reset the other scheduling variables */
    global_channel = channel_ptr = loop_index = 0;
    active_channels = channels_tbd = pending_ch_tbd = cur_rxdma_channels;
    pending_channels_mask = channels_mask;
}

static inline void bcmeapi_rx_post(uint32 *rxpktgood)
{
#if defined(CC_FAP4KE_ENET_STATS) && defined(_CONFIG_BCM_FAP)
    pHostPsmGbl->stats.Q7rxCount = rxpktprocessed;
    if(rxpktprocessed > pHostPsmGbl->stats.Q7rxHighWm)
    {
        pHostPsmGbl->stats.Q7rxHighWm = rxpktprocessed;
    }
#endif

    if(scheduling == SP_SCHEDULING) {
        global_channel = cur_rxdma_channels - 1;
    } else if (scheduling == FAP_SCHEDULING) {
        global_channel = 0;
    } 
}

#if ENET_RX_CHANNELS_MAX > 1   /* No need to select NEXT_CHANNEL if there is only one channel */
inline static void bcm_next_channel(void)
{
    BCM_ENET_RX_DEBUG("Selecting next channel for WRR scheduling");
    /* Get the array index for next channel and remember it. We need this
       index for replacing the channel done in next_channel array. */
    channel_ptr = (++loop_index) % channels_tbd;
    /* Select the next channel to be serviced from the next_channel array
       using the loop_index. Any scheduling alogirthms across channels can
       be implemented by chaging the logic on selecting the next channel.*/

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
    /* Need to disable uninitialized warnings here as compiler incorrectly thinks
       scheduiling can change over the course of this function */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#endif
    if (scheduling == WRR_SCHEDULING) {
        global_channel = next_channel[channel_ptr];
    }
    else {
        global_channel = 0;
    }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
#pragma GCC diagnostic pop
#endif
}
#endif /* ENET_RX_CHANNELS_MAX > 1 */

/* Defined only if more than one channel and FAP supported */
#if ENET_RX_CHANNELS_MAX > 1 && defined(_CONFIG_BCM_FAP)
static inline int bcmeapi_prepare_next_rx(uint32 *rxpktgood)
{
#ifdef BCM_ENET_DEBUG_BUILD
    ch_pkts[global_channel]++;
    ch_serviced[dbg_index % NUM_ELEMS] |= (1<<31);
#endif

    /* Update the pending weights and channels_tbd */
    if (scheduling == WRR_SCHEDULING) {
        pending_weight_pkts[global_channel]--;
        if (pending_weight_pkts[global_channel] <= 0) {
            if (pending_channels_mask & (1<<global_channel)) {
                pending_channels_mask &= (~(1<<global_channel));
                BCM_ENET_RX_DEBUG("pending_channels_mask = %x",
                        pending_channels_mask);
                /* If channels_tbd is less than or equal to 0, we are done.
                   So, get out of here */
                if ((--channels_tbd) <= 0) {
                    wrr_rx_reset();
                    return BCMEAPI_CTRL_CONTINUE;   /* Continue the Rx loop */
                    /* Continue the budge cycle
                    *rxpktgood |= ENET_POLL_DONE;
                    return BCMEAPI_CTRL_BREAK;
                    */
                }
                else
                    /* Replace the channel done with the last channel in the
                       channels to be done */
                    next_channel[channel_ptr] = next_channel[channels_tbd];
            }
            else
            {
                // Sampling the same ch again.
                // Time to reset the pending variables - Mar 2011
                *rxpktgood |= ENET_POLL_DONE;
            }
            /* WRR - Check for next packet on new channel - Jan 2011 */
            bcm_next_channel();
        }
    } else {
        /* SP or FAP Scheduling. Continue servicing the same channel */
        BCM_ENET_RX_DEBUG("SP Sched: Continue Rx on the same channel");
    }
    /* WRR - Check for next packet, same channel. Line moved - Jan 2011 */
    return BCMEAPI_CTRL_CONTINUE;
}
#endif /* #if ENET_RX_CHANNELS_MAX > 1 && defined(_CONFIG_BCM_FAP) */

static inline int bcmeapi_alloc_skb(BcmEnet_devctrl *pDevCtrl, struct sk_buff **skb)
{
	BcmEnet_RxDma *rxdma = pDevCtrl->rxdma[global_channel];

    ENET_SKBLIST_LOCK();
	if (rxdma->freeSkbList) {
		*skb = rxdma->freeSkbList;
		rxdma->freeSkbList = rxdma->freeSkbList->next_free;
	}
	else {
		*skb = kmem_cache_alloc(enetSkbCache, GFP_ATOMIC);

		if (!(*skb)) {
            ENET_SKBLIST_UNLOCK();
			return BCMEAPI_CTRL_FALSE;
		}
	}

    ENET_SKBLIST_UNLOCK();
	return BCMEAPI_CTRL_TRUE;
}

static inline void bcmeapi_kfree_buf_irq(BcmEnet_devctrl *pDevCtrl, struct fkbuff  *pFkb, unsigned char *pBuf)
{
    flush_assign_rx_buffer(pDevCtrl, global_channel, pBuf, pBuf);
}

static inline void bcmeapi_blog_drop(BcmEnet_devctrl *pDevCtrl, struct fkbuff  *pFkb, unsigned char *pBuf)
{
#if defined(_CONFIG_BCM_INGQOS) && defined(CC_IQ_STATS)
    BcmEnet_RxDma *rxdma = pDevCtrl->rxdma[global_channel];
#endif /* _CONFIG_BCM_FAP */


    /* CPU is congested and fcache has identified the packet
     * as low prio, and needs to be dropped */
    bcmeapi_kfree_buf_irq(pDevCtrl, NULL, pBuf);
#if defined(_CONFIG_BCM_INGQOS) && defined(CC_IQ_STATS)
#if defined(_CONFIG_BCM_FAP)
    rxdma->pktDmaRxInfo.iqDroppedDqm++;
#else
    rxdma->pktDmaRxInfo.iqDropped++;
#endif
#endif
}




static inline volatile DmaRegs *get_dmaCtrl( int channel )
{
    volatile DmaRegs *dmaCtrl;
#if !defined(CONFIG_BCM947189)

#if defined(CONFIG_BCM_GMAC)
    if ( IsGmacInfoActive && (channel == GMAC_LOG_CHAN ) )
        dmaCtrl= (DmaRegs *)(GMAC_DMA_BASE);
    else
#endif
        dmaCtrl = (DmaRegs *)(SWITCH_DMA_BASE);
#else

    if (channel == 0)
        dmaCtrl= (DmaRegs *)(ENET_CORE0_DMA);
    else
        dmaCtrl= (DmaRegs *)(ENET_CORE1_DMA);
#endif

    return dmaCtrl;
}

static inline int get_phy_chan( int channel )
{
    int phy_chan;

#if defined(CONFIG_BCM_GMAC)
    if ( IsGmacInfoActive && (channel == GMAC_LOG_CHAN ) )
        phy_chan = GMAC_PHY_CHAN;
    else
#endif
        phy_chan = channel;

    return phy_chan;
}

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
/*
 * Switch the RX DMA channel b/w standby ring and main ring
 */
inline static void switch_rx_ring(BcmEnet_devctrl *pDevCtrl, int channel, int toStdBy)
{
    int i = 0, status = 0, index = 0;
    DmaStateRam *StateRam;
    BcmEnet_RxDma *rxdma = pDevCtrl->rxdma[channel];
    volatile DmaRegs *dmaCtrl = get_dmaCtrl( channel );
    int phy_chan = get_phy_chan( channel );

    BCM_ENET_RX_DEBUG("Head = %d; Assigned BDs = %d \n",
        rxdma->pktDmaRxInfo.rxHeadIndex, rxdma->pktDmaRxInfo.rxAssignedBds);

    /* Stop DMA channel */
    rxdma->pktDmaRxInfo.rxDma->cfg = DMA_PKT_HALT;
    while(rxdma->pktDmaRxInfo.rxDma->cfg & DMA_ENABLE) {
    }
    bcmPktDma_EthRxDisable(&rxdma->pktDmaRxInfo);

	/* Clear State RAM */
    StateRam = (DmaStateRam *)&dmaCtrl->stram.s[phy_chan*2];
    memset(StateRam, 0, sizeof(DmaStateRam));

    /* Setup rx dma channel */
    if (toStdBy) {
        BCM_ENET_RX_DEBUG("switch_rx_ring: changing to stdby ring\n");
        rxdma->pktDmaRxInfo.rxDma->maxBurst |= DMA_THROUGHPUT_TEST_EN;
        dmaCtrl->stram.s[phy_chan * 2].baseDescPtr =
            (uint32)VIRT_TO_PHYS((uint32 *)rxdma->rxBdsStdBy);
    } else {
        BCM_ENET_RX_DEBUG("switch_rx_ring: changing to main ring\n");
        rxdma->pktDmaRxInfo.rxDma->maxBurst = DMA_MAX_BURST_LENGTH;
        dmaCtrl->stram.s[phy_chan * 2].baseDescPtr =
            (uint32)VIRT_TO_PHYS((uint32 *)rxdma->pktDmaRxInfo.rxBds);
        /* The head*/

        for (i = 0; i < rxdma->pktDmaRxInfo.rxAssignedBds; i++) {
            index = (rxdma->pktDmaRxInfo.rxHeadIndex + i) % rxdma->pktDmaRxInfo.numRxBds;
            status = rxdma->pktDmaRxInfo.rxBds[index].status;
            if (!(status & DMA_OWN)) {
                rxdma->pktDmaRxInfo.rxBds[index].length  = BCM_MAX_PKT_LEN;
                if (index == (rxdma->pktDmaRxInfo.numRxBds - 1)) {
                    rxdma->pktDmaRxInfo.rxBds[index].status = DMA_OWN | DMA_WRAP;
                } else {
                    rxdma->pktDmaRxInfo.rxBds[index].status = DMA_OWN;
                }
            } else {
                break;
            }
        }

        dmaCtrl->stram.s[phy_chan * 2].state_data = rxdma->pktDmaRxInfo.rxHeadIndex;
    }
    rxdma->pktDmaRxInfo.rxDma->intMask = 0;   /* mask all ints */
    rxdma->pktDmaRxInfo.rxDma->intStat = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
    rxdma->pktDmaRxInfo.rxDma->intMask = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;

    /* Start DMA channel if BDs are available */
    if (toStdBy || rxdma->pktDmaRxInfo.rxAssignedBds)
    {
        rxdma->pktDmaRxInfo.rxDma->cfg = DMA_ENABLE;
        bcmPktDma_EthRxEnable(&rxdma->pktDmaRxInfo);
    }
}
#endif /* defined(RXCHANNEL_PKT_RATE_LIMIT) */

extern int rxchannel_isr_enable[ENET_RX_CHANNELS_MAX];
extern int rxchannel_rate_limit_enable[ENET_RX_CHANNELS_MAX];
extern int rx_pkts_from_last_jiffies[ENET_RX_CHANNELS_MAX];
extern int channel_rx_rate_credit[ENET_RX_CHANNELS_MAX];
extern int rxchannel_rate_credit[ENET_RX_CHANNELS_MAX];
#define BCM_PORT_FROM_TYPE2_TAG(tag) (tag & 0x1f)

extern void enet_hex_dump(void *mem, unsigned int len);
static inline int bcmeapi_rx_pkt(BcmEnet_devctrl *pDevCtrl, unsigned char **pBuf, FkBuff_t **pFkb, 
				   int *len, int *gemid, int *phy_port_id, int *is_wifi_port, struct net_device **dev,
                                   uint32 *rxpktgood, uint32 *context_p, int *rxQueue)
{
#if !defined(CONFIG_BCM947189)
	DmaDesc dmaDesc;
#else
    DmaRxStatus  status;
#endif
	BcmEnet_RxDma *rxdma = pDevCtrl->rxdma[global_channel];
        
    ENET_RX_LOCK();

        *is_wifi_port = 0;

#ifdef BCM_ENET_DEBUG_BUILD
	ch_serviced[NEXT_INDEX(dbg_index)] = global_channel;
	BCM_ENET_RX_DEBUG("Total pkts received on this channel<%d> = %d",
					  global_channel, ch_pkts[global_channel]);
#endif
	BCM_ENET_RX_DEBUG("channels_tbd = %d; channel = %d", channels_tbd, global_channel);

#if !defined(_CONFIG_BCM_FAP)
	/* rxAssignedBds is only local for non-FAP builds */
	if (rxdma->pktDmaRxInfo.rxAssignedBds == 0) {
		ENET_RX_UNLOCK();
		BCM_ENET_RX_DEBUG("No RxAssignedBDs for this channel");
	#ifdef BCM_ENET_DEBUG_BUILD
		ch_no_bds[global_channel]++;
	#endif
		SELECT_NEXT_CHANNEL();
		return BCMEAPI_CTRL_SKIP;
	}
#endif

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
	if (!rxchannel_isr_enable[global_channel]) {
		ENET_RX_UNLOCK();
		SELECT_NEXT_CHANNEL();
		return BCMEAPI_CTRL_SKIP;
	}
#endif

	/* Read <status,length> from Rx BD at head of ring */
#if !defined(CONFIG_BCM947189)
	dmaDesc.word0 = bcmPktDma_EthRecv(&rxdma->pktDmaRxInfo, pBuf, len); 
#else
	status.word = bcmPktDma_EthRecv(&rxdma->pktDmaRxInfo, pBuf, len); 
#endif

	/* If no more rx packets, we are done for this channel */
#if !defined(CONFIG_BCM947189)
	if (dmaDesc.status & DMA_OWN) {
#else
	if (status.word == (uint32)0xFFFF) {
#endif
		BCM_ENET_RX_DEBUG("No Rx Pkts on this channel");
		ENET_RX_UNLOCK();
#ifdef BCM_ENET_DEBUG_BUILD
		ch_no_pkts[global_channel]++;
#endif
		SELECT_NEXT_CHANNEL();
		return BCMEAPI_CTRL_SKIP;
	}

#if defined(BCM_ENET_UNIMAC)
	/* If packet is marked as "FCS error" by UNIMAC, skip it,
	 * free it and stop processing for this packet */
	if (dmaDesc.status & DMA_DESC_ERROR) {
        pDevCtrl->estats.rx_packets_queue[0]++;
        pDevCtrl->estats.rx_errors_indicated_by_low_level++;
		bcmPktDma_EthFreeRecvBuf(&rxdma->pktDmaRxInfo, *pBuf);
		ENET_RX_UNLOCK();
		return BCMEAPI_CTRL_SKIP;
	}
#endif

#if defined(CC_FAP4KE_ENET_STATS) && defined(_CONFIG_BCM_FAP)
	pHostPsmGbl->stats.Q7rxTotal++;
#endif

#if defined(CONFIG_BCM_GMAC)
	if (gmac_info_pg->trans == 1) {
		/* Free all the packets received during MAC switching */
		ENET_RX_UNLOCK();
		flush_assign_rx_buffer(pDevCtrl, global_channel, *pBuf, *pBuf);
        pDevCtrl->estats.rx_packets_queue[0]++;
		pDevCtrl->stats.rx_dropped++;
        pDevCtrl->estats.rx_errors_indicated_by_low_level++;
		return BCMEAPI_CTRL_CONTINUE;
	}
#endif

#if !defined(CONFIG_BCM947189)
	if ((*len < ENET_MIN_MTU_SIZE) ||
		(dmaDesc.status & (DMA_SOP | DMA_EOP)) != (DMA_SOP | DMA_EOP)
#if defined(CONFIG_BCM_GMAC)
		|| (dmaDesc.status & DMA_GMAC_OUT_OF_RANGE)
#endif
        ) {
#else
    if (*len < ENET_MIN_MTU_SIZE || 
        status.dscr_cntr != 0 || status.rx_overflow) {
#endif
		ENET_RX_UNLOCK();
		flush_assign_rx_buffer(pDevCtrl, global_channel, *pBuf, *pBuf);
        pDevCtrl->estats.rx_packets_queue[0]++;
		pDevCtrl->stats.rx_dropped++;
		pDevCtrl->estats.rx_dropped_undersize++;
		return BCMEAPI_CTRL_CONTINUE;
	}

#if defined(RXCHANNEL_BYTE_RATE_LIMIT)
	if (channel_rx_rate_limit_enable[global_channel]) {
		if (jiffies_to_msecs(jiffies - last_byte_jiffies[global_channel])
			> 1000) {
			rx_pkts_from_last_jiffies[global_channel] = 0;
			last_byte_jiffies[channel] = jiffies;
		}
		if ((rx_bytes_from_last_jiffies[global_channel] + *len) >
			channel_rx_rate_credit[global_channel]) {
			ENET_RX_UNLOCK();
			flush_assign_rx_buffer(pDevCtrl, global_channel, *pBuf,*pBuf);
            pDevCtrl->estats.rx_packets_queue[0]++;
			pDevCtrl->stats.rx_dropped++;
			pDevCtrl->estats.rx_dropped_overrate++;
			return BCMEAPI_CTRL_CONTINUE;
		}
		rx_bytes_from_last_jiffies[global_channel] += *len;
	}
#endif /* defined(RXCHANNEL_BYTE_RATE_LIMIT) */

    *rxQueue = global_channel;

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
	if (rxchannel_rate_limit_enable[global_channel]) {
		rx_pkts_from_last_jiffies[global_channel]++;
		if ((rx_pkts_from_last_jiffies[global_channel] >=
			 rxchannel_rate_credit[global_channel]) &&
			rxchannel_isr_enable[global_channel]) {
			bcmPktDma_BcmHalInterruptDisable(global_channel, rxdma->rxIrq);
			rxchannel_isr_enable[global_channel] = 0;
			switch_rx_ring(pDevCtrl, global_channel, 1);
		}
	}
#endif /* defined(RXCHANNEL_PKT_RATE_LIMIT) */

	{
#if !defined(CONFIG_BCM947189)
#if defined(CONFIG_BCM_PORTS_ON_INT_EXT_SW)
		*phy_port_id = port_from_flag(dmaDesc.status);
		if (pDevCtrl->EnetInfo[0].sw.phy_id[*phy_port_id] & EXTSW_CONNECTED) {
			BcmEnet_hdr2* extHdr = (BcmEnet_hdr2 *)*pBuf;
	#if defined(CONFIG_BCM96362)
			if (extHdr->brcm_type == BRCM_TYPE) {
				/* Dual mTag, Skip internal SW mTag */
				extHdr = (BcmEnet_hdr2 *)(*pBuf + BRCM_TAG_LEN);
			}
	#endif /* 6362 */
			extHdr->brcm_type = BRCM_TYPE2;
			*phy_port_id = BCM_PORT_FROM_TYPE2_TAG(extHdr->brcm_tag);
			*phy_port_id = PHYSICAL_PORT_TO_LOGICAL_PORT(*phy_port_id, 1);   /* Logical port for External switch port */
		}
		else {	 /* Internal switch port */
			*phy_port_id = PHYSICAL_PORT_TO_LOGICAL_PORT(*phy_port_id, 0); /* Logical port from Internal switch port */
		}
#elif defined(CONFIG_BCM_EXT_SWITCH)  /* Only external switch ports in-use */
		{
			/* Ext Switch Only */
			((BcmEnet_hdr2*)(*pBuf))->brcm_type = BRCM_TYPE2;
			*phy_port_id = BCM_PORT_FROM_TYPE2_TAG(((BcmEnet_hdr2*)(*pBuf))->brcm_tag);
			*phy_port_id = PHYSICAL_PORT_TO_LOGICAL_PORT(*phy_port_id, 1); /* Logical port for External switch port */
		}
#else  /* Only Internal switch */
		*phy_port_id = port_from_flag(dmaDesc.status);
		*phy_port_id = PHYSICAL_PORT_TO_LOGICAL_PORT(*phy_port_id, 0); /* Logical port for Internal switch port */
#endif

#if defined(CONFIG_BCM_GMAC)
		if (IsGmacInfoActive && (global_channel == GMAC_LOG_CHAN) ) {
			*phy_port_id = enet_gmac_log_port();
		}
#endif
#else
        if (global_channel == global.pVnetDev0_g->extSwitch->connected_to_internalPort)
        {
            uint16_t brcm_tag = ((BcmEnet_hdr2*)(*pBuf))->brcm_tag;
            ((BcmEnet_hdr2*)(*pBuf))->brcm_type = htons(BRCM_TYPE2);
            *phy_port_id = BCM_PORT_FROM_TYPE2_TAG(ntohs(brcm_tag));
            *phy_port_id = PHYSICAL_PORT_TO_LOGICAL_PORT(*phy_port_id, 1); /* Logical port for External switch port */
        }
        else
        {
            *phy_port_id = global_channel;
            *phy_port_id = PHYSICAL_PORT_TO_LOGICAL_PORT(*phy_port_id, 0); /* Logical port for Internal switch port */
        }
#endif
	}

#if defined(DBL_DESC)
	/* If packet is from GPON port, get the gemid and find the gpon virtual
	   interface with which that gemid is associated */
	if ( *phy_port_id == GPON_PORT_ID) {
		*gemid = gemid_from_dmaflag(dmaDesc.status);
	}
#endif
    ENET_RX_UNLOCK();
	return BCMEAPI_CTRL_TRUE;
}

static inline void bcmeapi_prepare_rx(void)
{
#if defined(CC_FAP4KE_ENET_STATS) && defined(_CONFIG_BCM_FAP)
    pHostPsmGbl->stats.Q7budget = budget;
#endif
}

static inline void bcmeapi_napi_post(BcmEnet_devctrl *pDevCtrl)
{
    uint32 channel;
    BcmEnet_RxDma *rxdma;

    /* Enable the interrupts from all RX DMA channels */
    ENET_RX_LOCK();
    for (channel = 0; channel < cur_rxdma_channels; channel++) {
        rxdma = pDevCtrl->rxdma[channel];
#if defined(RXCHANNEL_PKT_RATE_LIMIT)
        if (rxchannel_isr_enable[channel])
#endif
        {
            bcmPktDma_BcmHalInterruptEnable(channel, rxdma->rxIrq);
#if defined(_CONFIG_BCM_FAP)
            // This is needed in the FAP version of the driver to keep it from locking up
            bcmPktDma_EthClrRxIrq(&rxdma->pktDmaRxInfo);
#endif
        }
    }
    ENET_RX_UNLOCK();
}

#if defined(_CONFIG_ENET_BCM_TM)
//#define CC_BCM_TM_DEBUG

#if defined(CC_BCM_TM_DEBUG)
static inline void bcm_tm_debug(char *text, uint32 seq, uint16 len, enet_bcm_tm_param_t *enetParam_p)
{
    printk("%s[%u,%lu]: len %u, txdma %p, pBuf %p, dmaStatus %u, destQueue %u, key %lu\n",
           text, enetParam_p->sw_port_id, seq, len, enetParam_p->txdma, enetParam_p->pBuf, enetParam_p->dmaStatus,
           enetParam_p->destQueue, enetParam_p->key);
}
#endif

static inline uint32 enet_get_dest_queue(pNBuff_t pNBuff)
{
    void *pBuf = PNBUFF_2_PBUF(pNBuff);
    uint32 destQueue;

    if(IS_SKBUFF_PTR(pNBuff))
    {
        destQueue = SKBMARK_GET_Q_PRIO(((struct sk_buff *)pBuf)->mark);
    }
    else
    {
        destQueue = SKBMARK_GET_Q_PRIO(((FkBuff_t *)pBuf)->mark);
    }

    if(destQueue >= ENET_BCM_TM_NBR_OF_QUEUES)
    {
        destQueue = 0;
    }

    return destQueue;
}

static inline int enet_pkt_enqueue(BcmPktDma_LocalEthTxDma *txdma, uint8 *pBuf,
                                   uint16 len, uint16 dmaStatus, uint32 key,
                                   uint32 sw_port_id, uint32 destQueue,
                                   int is_spdsvc_setup_packet)
{
    enet_bcm_tm_param_t enetParam;
    bcmTmDrv_enqueue_t tmEnqueue;

    enetParam.txdma = txdma;
    enetParam.pBuf = pBuf;
    enetParam.dmaStatus = dmaStatus;
    enetParam.is_spdsvc_setup_packet = is_spdsvc_setup_packet;
    enetParam.destQueue = destQueue;
    enetParam.sw_port_id = sw_port_id;
    enetParam.key = key;

    tmEnqueue.phy = BCM_TM_DRV_PHY_TYPE_ETH;
    tmEnqueue.port = sw_port_id;
    tmEnqueue.queue = destQueue;
    tmEnqueue.length = len;
    tmEnqueue.param_p = &enetParam;

#if defined(CC_BCM_TM_DEBUG)
    {
        static uint32 seq[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        bcm_tm_debug("ENQ", seq[sw_port_id]++, len, &enetParam);
    }
#endif
    return enet_bcm_tm_enqueue(&tmEnqueue);
}

static inline int enet_bcm_tm_xmit(uint16_t length, void *param_p)
{
    enet_bcm_tm_param_t *enetParam_p = param_p;
    BcmPktDma_LocalEthTxDma *txdma = enetParam_p->txdma;
    int ret = BCM_TM_TX_SUCCESS;

    ENET_TX_LOCK();

    if (txdma->txEnabled && bcmPktDma_EthXmitAvailable(txdma, 0))
    {
        bcmPktDma_EthXmit(txdma, enetParam_p->pBuf, length,
                          HOST_VIA_LINUX, enetParam_p->dmaStatus, enetParam_p->key,
                          0, 0, enetParam_p->sw_port_id, enetParam_p->destQueue, 0,
                          enetParam_p->is_spdsvc_setup_packet);

#if defined(CC_BCM_TM_DEBUG)
        {
            static uint32 seq[8] = {0, 0, 0, 0, 0, 0, 0, 0};
            bcm_tm_debug("DEQ", seq[enetParam_p->sw_port_id]++, length, enetParam_p);
        }
#endif
    }
    else
    {
        ret = BCM_TM_TX_FULL;
    }

    ENET_TX_UNLOCK();

    return ret;
}
#endif /* _CONFIG_ENET_BCM_TM */

#if defined(CONFIG_BCM_FAP_GSO)
extern fapGsoDesc_t *alloc_fapGsoDesc(void);
#endif
static inline int bcmeapi_pkt_xmt_dispatch(EnetXmitParams *pParam)
{
    int bufSource;
    uint32 key;
    int param1;
    BcmPktDma_EthTxDma *txdma = pParam->txdma;
    int param2 = -1;
#if defined(ENET_CACHE_SMARTFLUSH)
    FkBuff_t * pFkb = pParam->pFkb;
#endif
#if defined(_CONFIG_BCM_FAP)
    BcmEnet_devctrl *pDevCtrl = pParam->pDevPriv;
    uint32 destQueue = 0;
#endif /* _CONFIG_BCM_FAP */
#if defined(CONFIG_BCM_FAP_GSO)
    struct sk_buff *skb = pParam->skb;
#endif /* CONFIG_BCM_FAP_GSO */
#if defined(_CONFIG_ENET_BCM_TM)
    int ret;
#endif
    int is_spdsvc_setup_packet = 0;

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
    spdsvcHook_transmit_t spdsvc_transmit;

    spdsvc_transmit.pNBuff = pParam->pNBuff;
    spdsvc_transmit.dev = NULL;
    spdsvc_transmit.header_type = SPDSVC_HEADER_TYPE_ETH;
    spdsvc_transmit.phy_overhead = BCM_ENET_OVERHEAD;

    is_spdsvc_setup_packet = enet_spdsvc_transmit(&spdsvc_transmit);
    if(is_spdsvc_setup_packet < 0)
    {
        /* In case of error, NBuff will be free by spdsvc */
        return BCMEAPI_CTRL_CONTINUE;
    }
#else
    is_spdsvc_setup_packet = 0 ;
#endif

    key = (uint32)pParam->pNBuff;
    param1 = 0;

#if defined(DBL_DESC) && defined(ENET_GPON_CONFIG)
    if (pParam->port_id == GPON_PORT_ID)
    {
        bcmFun_t *getGemInfoFunc;
		DmaDesc16Ctrl *param2Ptr;
		BCM_GponGemPidQueueInfo gemInfo;

        getGemInfoFunc =  bcmFun_get(BCM_FUN_ID_GPON_GET_GEM_PID_QUEUE);
        gemInfo.gemPortIndex = pParam->gemid;
        if (!getGemInfoFunc || getGemInfoFunc(&gemInfo))
        {/* drop if no func or error */
            pParam->pDevPriv->estats.tx_drops_no_valid_gem_fun++;
            return BCMEAPI_CTRL_SKIP;
        }
        param2 = 0;
        param2Ptr = (DmaDesc16Ctrl*)(&param2);
        param2Ptr->gemPid = gemInfo.gemPortId;
        param2Ptr->usQueue = gemInfo.usQueueIdx;
        param2Ptr->pktColor = 0;
    }
#endif

#if defined(_CONFIG_BCM_FAP)
	bufSource = HOST_VIA_DQM;

    if(pFkb)
    {
        uint8  rxChannel = FKB_RECYCLE_CONTEXT(pFkb)->channel;
        FkBuff_t *pFkbMaster = _get_fkb_master_ptr_(pFkb);

        if(pFkbMaster->recycle_hook == (RecycleFuncP)bcm63xx_enet_recycle)
        {
                /* received from FAP */
                if(_get_master_users_(pFkbMaster) == 1)
                {
                    /* allow local recycling in FAP */
                    bufSource = FAP_ETH_RX;

#if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
                    if(txdma->txOwnership != HOST_OWNED)
#endif
                    {
                        key = (uint32)PFKBUFF_TO_PDATA(pFkb, BCM_PKT_HEADROOM);
                    }
                }
            /*
             * else, the FKB has been cloned so we need to send the nbuff back
             * to the Host for recycling:
             * bufSource = HOST_VIA_DQM;
             * key = (uint32)pNBuff;
             */

            param1 = rxChannel;
        }

#if defined (_CONFIG_BCM_XTMCFG)
        else if(pFkbMaster->recycle_hook == xtm_fkb_recycle_hook)
        {
            /* received from FAP */
            if(_get_master_users_(pFkbMaster) == 1)
            {
                bufSource = FAP_XTM_RX;

#if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
                if(txdma->txOwnership != HOST_OWNED)
#endif
                {
                    key = (uint32)PFKBUFF_TO_PDATA(pFkb, BCM_PKT_HEADROOM);
                }
            }
            /*
             * else, the FKB has been cloned so we need to send the nbuff back
             * to the Host for recycling:
             * bufSource = HOST_VIA_DQM;
             * key = (uint32)pNBuff;
             */

            param1 = rxChannel;
        }
#endif
        destQueue = SKBMARK_GET_Q_PRIO(pFkb->mark);
        bcmPktDma_tmCheckSetHighPrio(pDevCtrl->sw_port_id, destQueue,
                                     SKBMARK_GET_TC_ID(pFkb->mark), &destQueue);
    }
#if defined(CONFIG_BCM_FAP_GSO)
    else /* skb */
    {
        uint32 headerLen, totalFragLen;
        uint16 nr_frags, i;
        fapGsoDesc_t *gsoDesc;
        skb_frag_t *frag;
        uint8 *vaddr;

        if(skb->ip_summed == CHECKSUM_PARTIAL)
        {
            bufSource = HOST_VIA_DQM_CSUM;
        }

        destQueue = SKBMARK_GET_Q_PRIO(skb->mark);
        bcmPktDma_tmCheckSetHighPrio(pDevCtrl->sw_port_id, destQueue,
                                     SKBMARK_GET_TC_ID(skb->mark), &destQueue);

        if(skb_is_gso(skb) || skb_shinfo(skb)->nr_frags)
        {
            /*TODO when FRAGLIST is supported add nr_frags of all skb's in fraglist */
            nr_frags = skb_shinfo(skb)->nr_frags;

            if(nr_frags >  FAP_MAX_GSO_FRAGS)
            {
                printk("%s: nr_frags %d exceed max\n",__FUNCTION__, nr_frags);

                ENET_TX_UNLOCK();
                if(__skb_linearize(skb))
                {
                    ENET_TX_LOCK();
                    global.pVnetDev0_g->stats.tx_dropped++;
                    pParam->vstats->tx_dropped++;
                    pParam->pDevPriv->estats.tx_drops_skb_linearize_error++;
                    return BCMEAPI_CTRL_BREAK;
                }
                ENET_TX_LOCK();
            }

            /*read nr_frags again as skb might have changed in skb_linearize*/
            nr_frags = skb_shinfo(skb)->nr_frags;

            if(nr_frags)
            {
                bufSource = HOST_VIA_DQM_GSO_FRAG;

                gsoDesc = alloc_fapGsoDesc();
                if(!gsoDesc)
                {
                    printk("%s: failed to allocate gdoDesc\n",__FUNCTION__);
                    global.pVnetDev0_g->stats.tx_dropped++;
                    pParam->vstats->tx_dropped++;
                    pParam->pDevPriv->estats.tx_dropped_no_gso_dsc++;
                    return BCMEAPI_CTRL_BREAK;
                }

                totalFragLen = 0;
                for(i=0; i < nr_frags; i++ )
                {
                    frag = &skb_shinfo(skb)->frags[i];
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0))
                    vaddr = kmap_skb_frag(frag);
#else
                    vaddr = kmap_atomic(frag->page.p);
#endif
                    gsoDesc->frag_data[i]= vaddr + frag->page_offset;
                    gsoDesc->frag_len[i] = frag->size;
                    cache_flush_len(gsoDesc->frag_data[i], gsoDesc->frag_len[i]);
                    totalFragLen += frag->size;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0))
                    kunmap_skb_frag(vaddr);
#else
                    kunmap_atomic(vaddr);
#endif
                }

                headerLen = skb->len - totalFragLen;
                cache_flush_len(skb->data, headerLen);

                gsoDesc->recycle_key = (uint32)skb;
                gsoDesc->nr_frags = nr_frags;
                gsoDesc->mss = skb_shinfo(skb)->gso_size;
                param1 = skb_shinfo(skb)->gso_size;
                gsoDesc->totalLen = skb->len;
                //printk("mss is %d skb->len=%d,hdrlen=%d,fraglen=%d\n",gsoDesc->mss,skb->len, (int)headerLen, (int)totalFragLen);
                bcmPktDma_EthXmit(txdma, skb->data, headerLen,
                                  bufSource,  pParam->dmaDesc.status, (uint32)gsoDesc, param1, pParam->dqm,
                                  pDevCtrl->sw_port_id, destQueue, param2, 0);
            }
            else
            {
                bufSource = HOST_VIA_DQM_GSO;
                headerLen = skb->len;
                param1 = skb_shinfo(skb)->gso_size;/*mss*/
                cache_flush_len(skb->data, skb->len);

                bcmPktDma_EthXmit(txdma, skb->data, skb->len,
                                  bufSource,  pParam->dmaDesc.status, (uint32)skb, param1, pParam->dqm,
                                  pDevCtrl->sw_port_id, destQueue, param2, 0);

            }
            return BCMEAPI_CTRL_CONTINUE;
        }
    }
#endif /* CC_FAP4KE_PKT_GSO */

#else /* CONFIG_BCM_FAP */

    /* FAP is compiled out */
    bufSource = HOST_VIA_LINUX;
#endif /* CONFIG_BCM_FAP */

#if defined(ENET_CACHE_SMARTFLUSH)
	if(!pFkb) {	/* skb */
		struct sk_buff *skbp = pParam->skb;
        uint8 *dirty_p = skb_shinfo(skbp)->dirty_p;
        uint8 *pData = pParam->data;
        uint8 *pEnd = pData + pParam->len;
				
        // If driver returned this buffer to us with a valid dirty_p,
        // then we can shorten the flush length.
        if (dirty_p) {
            if ((dirty_p < skbp->head) || (dirty_p > pEnd))
                printk("invalid dirty_p detected: %p valid=[%p %p]\n", dirty_p, skbp->head, pEnd);
            else
                pEnd = (dirty_p < pData) ? pData : dirty_p;
        }
        nbuff_flush(pParam->pNBuff, pParam->data, pEnd - pData);
    } else {
        nbuff_flush(pParam->pNBuff, pParam->data, pParam->len);
    }
#else
    nbuff_flush(pParam->pNBuff, pParam->data, pParam->len);
#endif /* ENET_CACHE_SMARTFLUSH */

#if defined(_CONFIG_ENET_BCM_TM)
    ret = enet_pkt_enqueue(txdma, pParam->data, pParam->len,
                           pParam->dmaDesc.status, key,
                           pParam->pDevPriv->sw_port_id,
                           enet_get_dest_queue(pParam->pNBuff),
                           is_spdsvc_setup_packet);
    if(ret != BCM_TM_TX_SUCCESS)
    {
        if(ret == BCM_TM_TX_FULL)
        {
            return BCMEAPI_CTRL_SKIP;
        }
        else  // BCM_TM_TX_DISABLED
        {
            if (txdma->txEnabled && bcmPktDma_EthXmitAvailable(txdma, 0))
#endif
            {
                bcmPktDma_EthXmit(txdma,
                                  pParam->data, pParam->len, bufSource,
#if !defined(CONFIG_BCM947189)
                                  pParam->dmaDesc.status, key, param1, pParam->dqm,
#else
                                  pParam->dmaDesc.ctrl1, key, param1, pParam->dqm,
#endif
                                  pDevCtrl->sw_port_id, destQueue, param2,
                                  is_spdsvc_setup_packet
                                  );
            }
#if defined(_CONFIG_ENET_BCM_TM)
            else
            {
                /* BCM_TM is disabled and iuDMA ring is full */
                return BCMEAPI_CTRL_SKIP;
            }
        }
    }
#endif

#ifdef DEBUG_COUNTERS
    txdma->localstats.tx_pkts++;
#endif
    return BCMEAPI_CTRL_CONTINUE;
}

static inline void bcmeapi_config_tx_queue(EnetXmitParams *pParam)
{
#if !defined(CONFIG_BCM947189)
    unsigned int port_map = (1 << pParam->port_id);
    int dmaPriority;

    dmaPriority = (pParam->egress_queue > (ENET_TX_EGRESS_QUEUES_MAX-1)) ?
                    (ENET_TX_EGRESS_QUEUES_MAX-1) : pParam->egress_queue;

    if (IsExternalSwitchPort(pParam->port_id))
    {
#if defined(CONFIG_BCM_PORTS_ON_INT_EXT_SW)
        pParam->dmaDesc.status = DMA_OWN | DMA_SOP | DMA_EOP | DMA_APPEND_CRC | DMA_APPEND_BRCM_TAG | 
                                 (1 << global.pVnetDev0_g->extSwitch->connected_to_internalPort) | ((dmaPriority << 10) & DMA_PRIO);
#else
        pParam->dmaDesc.status = DMA_OWN | DMA_SOP | DMA_EOP | DMA_APPEND_CRC;
#endif
    }
    else
    {
        port_map = GET_PORTMAP_FROM_LOGICAL_PORTMAP(port_map, 0); /* Portmap for Internal switch */

        pParam->dmaDesc.status = DMA_OWN | DMA_SOP | DMA_EOP | DMA_APPEND_CRC |
                         DMA_APPEND_BRCM_TAG | (port_map) |
                         ((dmaPriority << 10) & DMA_PRIO);
    }
#else
    pParam->dmaDesc.ctrl1 = DMA_CTRL1_SOF | DMA_CTRL1_EOF | DMA_CTRL1_NOTPCIE;
#endif
}

static inline int bcmeapi_queue_select(EnetXmitParams *pParam)
{
#if !defined(_CONFIG_ENET_BCM_TM)
    BcmPktDma_EthTxDma *txdma = global.pVnetDev0_g->txdma[pParam->channel];
#if defined(_CONFIG_BCM_BPM) && defined(_CONFIG_BCM_FAP)
    uint32_t q = 0;
    uint32_t txDropThr = 0;
    uint32_t qDepth = 0;

    pParam->txdma = txdma;
    if (g_Eth_tx_iudma_ownership[pParam->channel] == HOST_OWNED)
    {
        q = (pParam->egress_queue > 3) ? 3 : pParam->egress_queue;
        txDropThr = txdma->txDropThr[q];
        qDepth = bcmPktDma_EthXmitBufCountGet_Iudma(txdma);
    }
#endif

    pParam->dqm = 0;
#if defined(_CONFIG_BCM_FAP)
    /* Send the high priority tarffic HOST2FAP HIGH priorty DQM's and other traffic
     * through LOW priority HOST2FAP DQM
     * classification is based on (skb/fkb)->mark
     */

    /* ouf of 4 switch priorities use (0,1) for low prioroty and (2,3)
       for high priorty */
    /* For 6818, there are 8 queues. However only 4 queues are exposed to the user via WEBGUI. So keeping the same logic
       Queue < 2 will be low priority and the rest would be high priority */
    if(pParam->egress_queue < 2)
    {
        pParam->dqm = DQM_HOST2FAP_ETH_XMIT_Q_LOW;
    }
    else
    {
        pParam->dqm = DQM_HOST2FAP_ETH_XMIT_Q_HI;
    }
#endif

    if (!txdma->txEnabled
            /* Check for tx slots available AFTER re-acquiring the tx lock */
            || !bcmPktDma_EthXmitAvailable(txdma, pParam->dqm)
#if defined(_CONFIG_BCM_BPM) && defined(_CONFIG_BCM_FAP)
            || ( (g_Eth_tx_iudma_ownership[pParam->channel] == HOST_OWNED) &&
                (qDepth >= txDropThr) )
#endif
       )
    {
#if defined(_CONFIG_BCM_BPM) && defined(_CONFIG_BCM_FAP)
        txdma->txDropThrPkts[q]++;
#endif

        TRACE(("%s: bcm63xx_enet_xmit low on txFreeBds\n", dev->name));
        BCM_ENET_TX_DEBUG("No more Tx Free BDs\n");
        global.pVnetDev0_g->stats.tx_dropped++;
        pParam->vstats->tx_dropped++;
        return BCMEAPI_CTRL_BREAK;
    }
#endif /* !defined(_CONFIG_ENET_BCM_TM) */

    return BCMEAPI_CTRL_CONTINUE;
}

static inline void bcmeapi_buf_reclaim(EnetXmitParams *pParam)
{
    BcmPktDma_txRecycle_t txRecycle;
    BcmPktDma_txRecycle_t *txRecycle_p;

    pParam->txdma = global.pVnetDev0_g->txdma[pParam->channel];
    ENET_TX_LOCK();
    pParam->reclaim_idx = 0;

    while((txRecycle_p = bcmPktDma_EthFreeXmitBufGet(pParam->txdma, &txRecycle)) != NULL)
    {
        pParam->delayed_reclaim_array[pParam->reclaim_idx] = txRecycle_p->key;

        pParam->reclaim_idx++;
        /*
         * only unlock and do reclaim if we have collected many free
         * buffers, otherwise, wait until end of function when we have
         * already released the tx lock to do reclaim.
         */
        if (pParam->reclaim_idx >= DELAYED_RECLAIM_ARRAY_LEN) {
            ENET_TX_UNLOCK();
            DO_DELAYED_RECLAIM(pParam->reclaim_idx, pParam->delayed_reclaim_array);
            pParam->reclaim_idx = 0;
            ENET_TX_LOCK();
        }
    }   /* end while(...) */
}

#endif /* _BCMENET_LOCAL_ */

#endif /* _BCMENET_DMA_INLINE_H_ */

