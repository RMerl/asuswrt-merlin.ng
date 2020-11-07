#ifndef __FAP_WFD_INLINE_H_INCLUDED__
#define __FAP_WFD_INLINE_H_INCLUDED__

/*
<:copyright-BRCM:2014:DUAL/GPL:standard 

   Copyright (c) 2014 Broadcom 
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/nbuff.h>
#include <linux/bcm_realtime.h>

#include <linux/gbpm.h>
#include <bpm.h>

//#include "kmap_skb.h"

#include "fap.h"
#include "fap_hw.h"
#include "fap4ke_memory.h"
#include "fap_dqm.h"
#include "fap_dqmHost.h"
#include "fap_task.h"
#include "fap4ke_packet.h"
#include "bcmPktDma.h"
#include "fap_swq_common.h"

extern SWQInfo_t wfdF2HQInfo[];
 
#define WFD_WLAN_QUEUE_MAX_SIZE 0  //SWQ_FAP2HOST_WFD_Q_DEPTH defined in fap4ke_swq.h

/* Defines related to WFD QID */
#define WFD_QID_FAPIDX_MASK  0x40000000
#define WFD_QID_FAPIDX_SHIFT 30
#define WFD_QID_DQM_MASK     0x3FFFFFFF
#define WFD_QID_DQM_SHIFT    0
#define WFD_GET_FAPIDX_FROM_QID(qid)  ((qid & WFD_QID_FAPIDX_MASK) >> WFD_QID_FAPIDX_SHIFT)
#define WFD_GET_DQM_FROM_QID(qid) (qid & WFD_QID_DQM_MASK)
#define WFD_GET_QIDX_FROM_FAPIDX_DQM(fapIdx, dqm) \
         (((dqm - DQM_FAP2HOST_WFD_BASE_Q) * NUM_FAPS) + fapIdx) 
#define WFD_GET_QIDX_FROM_QID(qid) \
         (WFD_GET_QIDX_FROM_FAPIDX_DQM(WFD_GET_FAPIDX_FROM_QID(qid),WFD_GET_DQM_FROM_QID(qid)))
#define WFD_GET_QID_FROM_FAPIDX_DQM(fapIdx, dqm) \
         (((fapIdx << WFD_QID_FAPIDX_SHIFT) & WFD_QID_FAPIDX_MASK) \
            | ((dqm << WFD_QID_DQM_SHIFT) & WFD_QID_DQM_MASK))
#define WFD_GET_WFD_IDX_FROM_DQM(dqm) (dqm - DQM_FAP2HOST_WFD_BASE_Q)
#define WFD_NUM_DQMS_PER_WFD            (1)
#define WFD_NUM_QUEUES_PER_WFD_INST     (NUM_FAPS * WFD_NUM_DQMS_PER_WFD)

/* CAUTION: Value 64 was derived based on test performed on 63268 platforms.
   Setting the cache invalidate len to a lower value can cause incorrect
   packet header information to be prefetched in cache */
#define WFD_FAP_CACHE_INVALIDATE_LEN 64

extern struct sk_buff * skb_header_alloc(void);


/*****************************************************************************
* Function: wfd_dev_rx_isr_callback                                          *
*                                                                            *
* Description: FAP WFD ISR handler                                           *
******************************************************************************/
void wfd_dev_rx_isr_callback(uint32 fapId, unsigned long dqm)
{
	uint32 wfdIdx = WFD_GET_WFD_IDX_FROM_DQM(dqm);

    /*Atomically set the queue bit on*/
    set_bit(WFD_GET_QIDX_FROM_FAPIDX_DQM(fapId, dqm),
			&wfd_objects[wfdIdx].wfd_rx_work_avail);

	{
	   SWQInfo_t *pSwQ = &wfdF2HQInfo[WFD_GET_QIDX_FROM_FAPIDX_DQM(fapId, dqm)];
	   pSwQ->swq->interrupts++;
	}
    WFD_WAKEUP_RXWORKER(wfdIdx);
}

int wfd_accelerator_init(void)
{
    return 0;
}

static inline void wfd_free_buf(enumWFDRecycleType recycleType,
                                int recycleChannel, 
                                int recycleFapIdx,
                                unsigned char *pBuf)
{
    //printk("fapIdx %d type %d channel %d\n", recycleFapIdx, recycleType, recycleChannel);
    if (recycleType == WFD_RECYCLE_TYPE_ENET)
    {
        bcmPktDma_RecycleBufToFap(recycleFapIdx, recycleChannel, pBuf,
                                  DQM_HOST2FAP_ETH_FREE_RXBUF_Q,
                                  DQM_HOST2FAP_ETH_FREE_RXBUF_Q_SIZE);
    }
    else if (recycleType == WFD_RECYCLE_TYPE_XTM)
    {
        bcmPktDma_RecycleBufToFap(recycleFapIdx, recycleChannel, pBuf,
                                  DQM_HOST2FAP_XTM_FREE_RXBUF_Q,
                                  DQM_HOST2FAP_XTM_FREE_RXBUF_Q_SIZE);
    }
    else
    {
        printk("WFD Invalid Recycle Type %d\n", (int)recycleType);
    }
}

static inline void wfd_buf_recycle(uint32 *nbuff, uint32_t context, uint32_t flags)
{
    fap_wfdMsgCtxt_t recycleContext;
    enumWFDRecycleType recycleType;
    int recycleChannel;
    int recycleFapIdx;

    recycleContext.word = context;
    recycleType         = recycleContext.recycleType;
    recycleChannel      = recycleContext.recycleChannel;
    recycleFapIdx       = recycleContext.recycleFapIdx;

    if ( IS_FKBUFF_PTR(nbuff) )
    {
        uint8 *pData = PFKBUFF_TO_PDATA(PNBUFF_2_FKBUFF(nbuff),BCM_PKT_HEADROOM);

        /* No cache flush */
        wfd_free_buf(recycleType, recycleChannel, recycleFapIdx, pData);
    }
    else /* skb */
    {
        struct sk_buff *skb = PNBUFF_2_SKBUFF(nbuff);
        
        if (flags & SKB_DATA_RECYCLE)
        {
            void *data_endp;
            void *data_startp = skb->head + BCM_PKT_HEADROOM;

#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
            void *dirty_p = skb_shinfo(skb)->dirty_p;
            void *shinfoBegin = (uint8 *)skb_shinfo(skb);
            void *shinfoEnd;

            if (skb_shinfo(skb)->nr_frags == 0)
            {
                // no frags was used on this skb, so can shorten amount of data
                // flushed on the skb_shared_info structure
                shinfoEnd = shinfoBegin + offsetof(struct skb_shared_info, frags);
            } 
            else
            {
                shinfoEnd = shinfoBegin + sizeof(struct skb_shared_info);
            }
            //cache_flush_region(shinfoBegin, shinfoEnd);
            cache_invalidate_region(shinfoBegin, shinfoEnd);

            // If driver returned this buffer to us with a valid dirty_p,
            // then we can shorten the flush length.
            if (dirty_p)
            {
                if ((dirty_p < (void *)skb->head) || (dirty_p > shinfoBegin))
                {
                    printk("invalid dirty_p detected: %p valid=[%p %p]\n",
                           dirty_p, skb->head, shinfoBegin);
                    data_endp = shinfoBegin;
                } 
                else
                {
                    data_endp = (dirty_p < data_startp) ? data_startp : dirty_p;
                }
            } 
            else
            {
                data_endp = shinfoBegin;
            }
#else
            /*its ok not to invalidate skb->head to skb->data as this area is
             * not used by runner or DMA 
             */
            data_endp = (void *)(skb_shinfo(skb)) + sizeof(struct skb_shared_info);
#endif
            cache_invalidate_region(data_startp, data_endp);

            /* free the buffer to FAP */
            wfd_free_buf(recycleType, recycleChannel, recycleFapIdx, data_startp);
        }
        else
        {
            printk("%s: Error only DATA recycle is supported\n", __FUNCTION__);
        }
    }
}

static inline int wfd_recvAvailable(unsigned long qid)
{
    return (bcmPktDma_swqRecvAvailableHost(wfdF2HQInfo[WFD_GET_QIDX_FROM_QID(qid)].swq));
}

static inline void wfd_recvMsg(unsigned long qid, SWQDataMsg_t *pRxMsg)
{
    SWQInfo_t *pSwQ = &wfdF2HQInfo[WFD_GET_QIDX_FROM_QID(qid)];

    bcmPktDma_swqRecvMsgHost(pSwQ->swq, pRxMsg, pSwQ->msgSize,
                             pSwQ->qStart, pSwQ->qEnd);
	pSwQ->swq->processed++;
}

static uint32_t
wfd_bulk_fkb_get(unsigned long qid, unsigned long budget, void *priv)
{
    unsigned int rx_pktcnt = 0;
    fap2Host_wfdMsg_t rxMsg;
    void *pBuf = NULL;
    uint16 len = 0;
    int32 size_adjust = 0;
    FkBuff_t *fkb_p;
    uint32 wfd_cache_inv_len = WFD_FAP_CACHE_INVALIDATE_LEN;
    wfd_object_t *wfd_p = (wfd_object_t *)priv;
    void *rx_pkts[NUM_PACKETS_TO_READ_MAX];

    while (budget)
    {
        if (!wfd_recvAvailable(qid))
        {
            break;
        }

        wfd_recvMsg(qid, (SWQDataMsg_t *)&rxMsg);

        pBuf = rxMsg.pBuf;
        len = rxMsg.length;

        size_adjust = rxMsg.ctxt.pBufOffset;

        /* If offSetSign is 1. Hdrs worth size pBufOffset were removed - packet is short */
        /* FAP has moved the head pointer by pBufOffset (removed hdrs) */
        /* If offSetSign is 0. Hdrs worth size pBufOffset were added - packet is long */
        size_adjust = size_adjust + (rxMsg.ctxt.offSetSign * -2 * size_adjust);

        /* Init FkBuff */
        fkb_p = fkb_init(pBuf - size_adjust,
                         BCM_PKT_HEADROOM - size_adjust,
                         pBuf - size_adjust,
                         len + size_adjust);
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
        if (wfd_cache_inv_len > fkb_p->len)
        {
            wfd_cache_inv_len = fkb_p->len;
        }
        fkb_p->dirty_p = _to_dptr_from_kptr_(fkb_p->data + wfd_cache_inv_len);
#endif
        fkb_p->recycle_hook = (RecycleFuncP)wfd_buf_recycle;
        fkb_p->recycle_context = rxMsg.ctxt.word;
        fkb_p->wl.ucast.dhd.flowring_idx = rxMsg.ctxt.dhd.flow_ring_idx;
        fkb_p->wl.ucast.dhd.wl_prio = rxMsg.ctxt.dhd.dhd_prio;
        fkb_p->wl.ucast.dhd.ssid = rxMsg.ctxt.dhd.ssid;

        rx_pkts[rx_pktcnt] = (void *)fkb_p;

        budget--;
        rx_pktcnt++;
    }

    if (rx_pktcnt)
    {
        (void)wfd_p->wfd_fwdHook(rx_pktcnt, (uint32_t)rx_pkts, wfd_p->wl_radio_idx, 0);
    }

    return rx_pktcnt;
}

static uint32_t
wfd_bulk_skb_get(unsigned long qid, unsigned long budget, void *priv)
{
    unsigned int rx_pktcnt;
    void *rx_pkts[NUM_PACKETS_TO_READ_MAX];
    struct sk_buff *skb_p;
    fap2Host_wfdMsg_t rxMsg;
    void *pBuf = NULL;
    uint16 len = 0;
    int32 size_adjust = 0;
    uint32 recycle_context = 0;
    uint32 wfd_cache_inv_len = WFD_FAP_CACHE_INVALIDATE_LEN;
    wfd_object_t *wfd_p = (wfd_object_t *)priv;

    rx_pktcnt = 0;

    while (budget)
    {
        if (!wfd_recvAvailable(qid))
        {
            break;
        }

        wfd_recvMsg(qid, (SWQDataMsg_t *)&rxMsg);

        pBuf = rxMsg.pBuf;
        len = rxMsg.length;

        size_adjust = rxMsg.ctxt.pBufOffset;

        /* If offSetSign is 1. Hdrs worth size pBufOffset were removed - packet is short */
        /* FAP has moved the head pointer by pBufOffset (removed hdrs) */
        /* If offSetSign is 0. Hdrs worth size pBufOffset were added - packet is long */
        size_adjust = size_adjust + (rxMsg.ctxt.offSetSign * -2 * size_adjust);

        /* allocate skb structure*/
        skb_p = skb_header_alloc();

        if (!skb_p)
        {
            printk("%s : SKB Allocation failure\n", __FUNCTION__);
            wfd_free_buf(rxMsg.ctxt.recycleFapIdx,
                         rxMsg.ctxt.recycleType,
                         rxMsg.ctxt.recycleChannel,
                         pBuf);
            break;
        }

        recycle_context = rxMsg.ctxt.word;

        /* initialize the skb */
        skb_headerinit(BCM_PKT_HEADROOM - size_adjust,
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
                       SKB_DATA_ALIGN(len + BCM_SKB_TAILROOM + size_adjust),
#else
                       BCM_MAX_PKT_LEN + size_adjust,
#endif
                       skb_p, pBuf - size_adjust, (RecycleFuncP)wfd_buf_recycle, recycle_context, NULL);

        skb_trim(skb_p, len + size_adjust);
        skb_p->recycle_flags &= SKB_NO_RECYCLE; /* no skb recycle,just do data recyle */

#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
        if (wfd_cache_inv_len > skb_p->len)
        {
            wfd_cache_inv_len = skb_p->len;
        }

        /* Set dirty pointer to optimize cache invalidate */
        skb_shinfo(skb_p)->dirty_p = skb_p->data + wfd_cache_inv_len;
#endif
        /* wlTxChainIdx = 16bit; wl_chainidx = 8bit */
        skb_p->wl.ucast.nic.wl_chainidx = rxMsg.ctxt.nic.wlTxChainIdx;

        DECODE_WLAN_PRIORITY_MARK(rxMsg.ctxt.nic.priority, skb_p->mark);

        rx_pkts[rx_pktcnt] = (void *)skb_p;

        budget--;
        rx_pktcnt++;
    }

    if (rx_pktcnt)
    {
        (void)wfd_p->wfd_fwdHook(rx_pktcnt, (uint32_t)rx_pkts, wfd_p->wl_radio_idx, 0);
    }

    return rx_pktcnt;
}

static inline int wfd_get_qid(int qidx)
{
    int dqm = DQM_FAP2HOST_WFD_BASE_Q + (qidx / NUM_FAPS);

    /* If number of FAPs is 2 (i.e. 63268 platform),
       "even" queue indices are for FAP 0
       and "odd" queue indices are for FAP 1 */
    return WFD_GET_QID_FROM_FAPIDX_DQM((qidx % NUM_FAPS), dqm);
}

static inline int wfd_get_fapidx(int qidx)
{
#if NUM_FAPS == 2 /* 63268 */
    /* If number of FAPs is 2 (i.e. 63268 platform),
       "even" queue indices are for FAP 0
       and "odd" queue indices are for FAP 1 */
    return (qidx % NUM_FAPS);
#else 
    return 0;
#endif
}

static inline void *wfd_acc_info_get(int radio)
{
    return NULL;
}

static inline int wfd_queue_not_empty(int radio, long qid, int qidx)
{
    uint32_t fapIdx = WFD_GET_FAPIDX_FROM_QID(qid);
    uint32_t dqm = WFD_GET_DQM_FROM_QID(qid);
    DQMQueueDataReg_S dqmMsg;

    if (bcmPktDma_swqRecvAvailableHost(wfdF2HQInfo[WFD_GET_QIDX_FROM_FAPIDX_DQM(fapIdx, dqm)].swq))
    {
        return 1;
    }

    /* read the dummy message in the associated dqm */
    if (bcmPktDma_isDqmRecvAvailableHost(fapIdx, dqm))
    {
        bcmPktDma_dqmRecvMsgHost(fapIdx, dqm, 1, &dqmMsg);

        /* clear the interrupt */
        dqmClearNotEmptyIrqStsHost(fapIdx, 1 << dqm);
    }

    return bcmPktDma_swqRecvAvailableHost(wfdF2HQInfo[WFD_GET_QIDX_FROM_FAPIDX_DQM(fapIdx, dqm)].swq);
}

static inline int wfd_config_rx_queue(int wfd_idx, int qid, uint32_t queue_size,
                                      enumWFD_WlFwdHookType eFwdHookType)
{
    uint32_t dqm = WFD_GET_DQM_FROM_QID(qid);
    uint32_t fapIdx = WFD_GET_FAPIDX_FROM_QID(qid);

    /* Both FAP0 and FAP1 queues have the same dqm id. It is
       enough to register a dqm once. So, Register when this function
       is called for FAP IDX 0 */
    if (fapIdx == 0)
    {
        /* register the host gso loopback recv handler */
        bcmPktDma_dqmHandlerRegisterHost((1 << dqm),
                                         wfd_dev_rx_isr_callback, 
                                         dqm);

        bcmPktDma_dqmHandlerEnableHost(1 << dqm);
    }

    return 0;
}

static inline void wfd_int_enable(int radio, int qid, int qidx) 
{
    bcmPktDma_dqmEnableNotEmptyIrq(WFD_GET_FAPIDX_FROM_QID(qid), (1 << WFD_GET_DQM_FROM_QID(qid)));
}

static inline int wfd_get_objidx(int qid, int qidx)
{
    return (WFD_GET_DQM_FROM_QID(qid) - DQM_FAP2HOST_WFD_BASE_Q);
}

static inline int wfd_get_minQIdx(int wfd_idx)
{
    if (wfd_idx >= WFD_MAX_OBJECTS)
    {
        printk("%s wfd_idx %d out of bounds(%d)\n", __FUNCTION__, wfd_idx, WFD_MAX_OBJECTS);
        return -1;
    }

    return (wfd_idx * WFD_NUM_QUEUES_PER_WFD_INST);
}

static inline int wfd_get_maxQIdx(int wfd_idx)
{
    if (wfd_idx >= WFD_MAX_OBJECTS)
    {
        printk("%s wfd_idx %d out of bounds(%d)\n", __FUNCTION__, wfd_idx, WFD_MAX_OBJECTS);
        return -1;
    }

    return ((wfd_idx * WFD_NUM_QUEUES_PER_WFD_INST) + (WFD_NUM_QUEUES_PER_WFD_INST - 1));
}
#define wfd_int_disable(radio, qid, qidx) 
#define release_wfd_interfaces() 

#endif
