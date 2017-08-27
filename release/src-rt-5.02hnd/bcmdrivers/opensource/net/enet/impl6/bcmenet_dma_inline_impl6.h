/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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

#define ENET_CACHE_SMARTFLUSH

#undef SELECT_NEXT_CHANNEL
#define  SELECT_NEXT_CHANNEL()    { *rxpktgood |= ENET_POLL_DONE; return BCMEAPI_CTRL_BREAK;}

static inline void bcmeapi_set_fkb_recycle_hook_impl6(FkBuff_t * pFkb,
                                        BcmEnet_devctrl *pDevCtrl);
static inline int bcmeapi_skb_headerinit_impl6(int len, BcmEnet_devctrl *pDevCtrl,
                                          struct sk_buff *skb, FkBuff_t * pFkb,
                                          unsigned char *pBuf);
static inline void __bcm63xx_enet_recycle_fkb_impl6(struct fkbuff * pFkb,
                                                  uint32 context);
static inline void bcm63xx_enet_recycle_impl6(pNBuff_t pNBuff, uint32 context,
                                                                uint32 flags);
static inline void bcm63xx_enet_recycle_skb_or_data_impl6(struct sk_buff *skb,
                                             uint32 context, uint32 free_flag);
static inline void bcmeapi_config_tx_queue_impl6(EnetXmitParams *pParam);
static inline int bcmeapi_rx_pkt_impl6(BcmEnet_devctrl *pDevCtrl,
                                  unsigned char **pBuf, int *len,
                                  uint32 *rxpktgood);



static inline void bcmeapi_set_fkb_recycle_hook_impl6(FkBuff_t * pFkb,
                                            BcmEnet_devctrl *pDevCtrl)
{
    uint32 context = BUILD_CONTEXT(pDevCtrl, 0);

    pFkb->recycle_hook = (RecycleFuncP)bcm63xx_enet_recycle_impl6;
    pFkb->recycle_context = context;
}

static inline int bcmeapi_skb_headerinit_impl6(int len, BcmEnet_devctrl *pDevCtrl,
                                          struct sk_buff *skb, FkBuff_t * pFkb,
                                          unsigned char *pBuf)
{
    uint32 recycle_context = BUILD_CONTEXT(pDevCtrl, 0);

    skb_headerinit(BCM_PKT_HEADROOM,
#if defined(ENET_CACHE_SMARTFLUSH)
            SKB_DATA_ALIGN(len+BCM_SKB_TAILROOM),
#else
            RX_BUF_LEN,
#endif
            skb, pBuf, (RecycleFuncP)bcm63xx_enet_recycle_skb_or_data_impl6,
            recycle_context,(void*)pFkb->blog_p);

    skb_trim(skb, len);

    return BCMEAPI_CTRL_TRUE;
}

/* Callback: fkb and data recycling */
static inline void __bcm63xx_enet_recycle_fkb_impl6(struct fkbuff * pFkb,
                                                  uint32 context)
{
    BcmEnet_devctrl *pDevCtrl;
    int channel = 0;
    uint8 *pData = PFKBUFF_TO_PDATA(pFkb,BCM_PKT_HEADROOM);

    pDevCtrl = CONTEXT_TO_PDEVCTRL(pFkb->recycle_context);
    _assign_rx_buffer(pDevCtrl, channel, pData); /* No cache flush */
}

/* Common recycle callback for fkb, skb or data */
static inline void bcm63xx_enet_recycle_impl6(pNBuff_t pNBuff, uint32 context,
                                                                uint32 flags)
{
    if (IS_FKBUFF_PTR(pNBuff))
    {
        __bcm63xx_enet_recycle_fkb_impl6(PNBUFF_2_FKBUFF(pNBuff), context);
    }
    else
    { /* IS_SKBUFF_PTR(pNBuff) */
        bcm63xx_enet_recycle_skb_or_data_impl6(PNBUFF_2_SKBUFF(pNBuff),
                                                context,flags);
    }
}

/*
 * This function is exact copy of bcm63xx_enet_recycle_skb_or_data_wl_tx_chain; Any bug fixes should be done in both
 */
static inline void bcm63xx_enet_recycle_skb_or_data_impl6(struct sk_buff *skb,
                                             uint32 context, uint32 free_flag)
{
    int channel = 0;
    BcmEnet_devctrl *pDevCtrl = CONTEXT_TO_PDEVCTRL(context);

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
        pEnd = pData + RX_BUF_LEN;
#endif
        flush_assign_rx_buffer(pDevCtrl, channel, pData, pEnd);
    }
}

static inline void bcmeapi_config_tx_queue_impl6(EnetXmitParams *pParam)
{
    pParam->dmaDesc.status = DMA_OWN | DMA_SOP | DMA_EOP;
}

static inline int bcmeapi_pkt_xmt_dispatch_impl6(EnetXmitParams *pParam)
{
    int bufSource;
    uint32 key;
    int param1 = 0;
    BcmPktDma_EthTxDma *txdma = pParam->txdma;

    key = (uint32)pParam->pNBuff;

    /* FAP is compiled out */
    bufSource = HOST_VIA_LINUX;

    nbuff_flush(pParam->pNBuff, pParam->data, pParam->len);

    bcmPktDma_EthXmit(txdma,
            pParam->data, pParam->len, bufSource,
            pParam->dmaDesc.status, key, param1, pParam->dqm,
            pDevCtrl->sw_port_id, destQueue, pParam->gemid, 0);

#ifdef DEBUG_COUNTERS
    txdma->localstats.tx_pkts++;
#endif
    return BCMEAPI_CTRL_CONTINUE;
}

static inline int bcmeapi_rx_pkt_impl6(BcmEnet_devctrl *pDevCtrl,
                                  unsigned char **pBuf, int *len,
                                  uint32 *rxpktgood)
{
	DmaDesc dmaDesc;
	uint32 cpuid = smp_processor_id();
	BcmEnet_RxDma *rxdma = pDevCtrl->rxdma[0];

	/* rxAssignedBds is only local for non-FAP builds */
	if (rxdma->pktDmaRxInfo.rxAssignedBds == 0) {
		RECORD_BULK_RX_UNLOCK();
		ENET_RX_UNLOCK();
		BCM_ENET_RX_DEBUG("No RxAssignedBDs for this channel");
		SELECT_NEXT_CHANNEL();
		return BCMEAPI_CTRL_SKIP;
	}

	/* Read <status,length> from Rx BD at head of ring */
	dmaDesc.word0 = bcmPktDma_EthRecv(&rxdma->pktDmaRxInfo, pBuf, len);
	*rxpktgood = 1;

	/* If no more rx packets, we are done for this channel */
	if (dmaDesc.status & DMA_OWN) {
		BCM_ENET_RX_DEBUG("No Rx Pkts on this channel");
		RECORD_BULK_RX_UNLOCK();
		ENET_RX_UNLOCK();
		SELECT_NEXT_CHANNEL();
		return BCMEAPI_CTRL_SKIP;
	}

	/* If packet is marked as "FCS error" by UNIMAC, skip it,
	 * free it and stop processing for this packet */
	if (dmaDesc.status & DMA_DESC_ERROR) {
		bcmPktDma_EthFreeRecvBuf(&rxdma->pktDmaRxInfo, *pBuf);
		RECORD_BULK_RX_UNLOCK();
		ENET_RX_UNLOCK();
		return BCMEAPI_CTRL_SKIP;
	}

	if ((*len < ENET_MIN_MTU_SIZE) ||
		(dmaDesc.status & (DMA_SOP | DMA_EOP)) != (DMA_SOP | DMA_EOP)) {
		RECORD_BULK_RX_UNLOCK();
		ENET_RX_UNLOCK();
		flush_assign_rx_buffer(pDevCtrl, global_channel, *pBuf, *pBuf);
		pDevCtrl->stats.rx_dropped++;
		return BCMEAPI_CTRL_CONTINUE;
	}

	return BCMEAPI_CTRL_TRUE;
}

