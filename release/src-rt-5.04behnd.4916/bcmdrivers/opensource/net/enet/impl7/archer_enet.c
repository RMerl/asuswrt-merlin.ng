/*
   <:copyright-BRCM:2018:DUAL/GPL:standard
   
      Copyright (c) 2018 Broadcom 
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

/***********************************
 *  Implementation of enetxapi and data structure for enet driver using CPU queues (Archer)
 */

#include <linux/nbuff.h>
#include <linux/kthread.h>
#include <linux/bcm_log.h>
#include <linux/bcm_skb_defines.h>

#include "enet.h"
#include "enet_types.h"

#include "archer_gpl.h"
#include "archer_enet.h"
#include "mpm.h"

#include "bcm_prefetch.h"
#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
#include <linux/gbpm.h>
#include "bpm.h"
#else
#error "bpm not defined"
#endif

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
#include <linux/bcm_log.h>
#include <bcmenet_common.h>
#include "bcm_archer_spdsvc.h"

_STATIC_ void archer_spdsvc_init(void);

static bcmFun_t *archer_spdsvc_transmit_hook = NULL;
static bcmFun_t *archer_spdsvc_receive_hook = NULL;
#endif /* CONFIG_BCM_SPDSVC || CONFIG_BCM_SPDSVC_MODULE */

static int archer_enet_host_bind(void);

//#define CC_ARCHER_ENET_DEBUG

#define ARCHER_ENET_RECYCLE_BUDGET 256

#define DEFAULT_NAPI_WEIGHT 32
extern int enetx_weight_budget;
extern int chan_thread_handler(void *data);

cpu_queues_t * enet_cpu_queues = NULL;

typedef struct cpu_queue_rx_info_t
{
    uint8_t *pData;
    archer_cpu_rx_info_enet_t info;
} cpu_queue_rx_info_t;

typedef struct cpu_queue_tx_info_t
{
    pNBuff_t pNBuff;
    uint16_t egress_port;
    uint16_t egress_queue;
} cpu_queue_tx_info_t;

typedef struct cpu_queue_recycle_info_t
{
    pNBuff_t pNBuff;
} cpu_queue_recycle_info_t;

#if defined(CONFIG_BCM_ARCHER_MPM)
static int archer_enet_mpm_init(void);
#endif

static int cpu_queues_init (cpu_queues_t * cpu_queues);
static int cpu_queues_free (cpu_queues_t * cpu_queues);

/*------------ Implementation of enetxapi ------------------ */
void enetxapi_queue_int_enable (enetx_channel *chan, int q_id)
{
    enet_cpu_queues->rx_notify_enable |= (1 << q_id);
}

void enetxapi_queue_int_disable (enetx_channel *chan, int q_id)
{
    if (enet_cpu_queues->rx_notify_pending_disable == 0)
    {
        enet_cpu_queues->rx_notify_pending_disable = 1;
    }
}

int enetxapi_queues_init (enetx_channel ** _chan)
{
    /* creating 1 channel, and a hi and lo priority queue for now */
    enetx_channel *chan;
    int rc;

    chan = kzalloc(sizeof(enetx_channel), GFP_KERNEL);
    if (!chan)
        return -1;

    chan->rx_q_count = NUM_RX_QUEUES;
    chan->rx_q[0] = CPU_RX_HI;
    chan->rx_q[1] = CPU_RX_LO;

    *_chan = chan;

    init_waitqueue_head (&chan->rxq_wqh);
    chan->rx_thread = kthread_run(chan_thread_handler, chan, "bcmsw_rx");

    rc = 0;

    // initialize the rx queues in each interface
    enet_cpu_queues = kzalloc (sizeof(cpu_queues_t), GFP_KERNEL);
    rc = cpu_queues_init (enet_cpu_queues);
    BUG_ON(rc != 0);

    enet_cpu_queues->chanp = chan;

    // only 1 channel for now
    chan->next = NULL;

    enetx_weight_budget = DEFAULT_NAPI_WEIGHT;

    if (!rc)
    {
        enet_dbg ("created CPU queue interface\n");
    }
    else
    {
        enet_err ("failed to create CPU queue interface\n");
        enetxapi_queues_uninit (_chan);
    }

#if defined(CONFIG_BCM_ARCHER_MPM)
    rc = archer_enet_mpm_init();
    if(rc)
    {
        return rc;
    }
#endif

    rc = archer_enet_host_bind();
    if(rc)
    {
        enetxapi_queues_uninit (_chan);

        return rc;
    }

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
    archer_spdsvc_init();
#endif
    return rc;
}


int enetxapi_queues_uninit (enetx_channel **_chan)
{
    enetx_channel * chan = *_chan;

    if (chan->next)
    {
        enet_err ("only one channel is supported now\n");
        return -1;
    }
    if (chan->rx_thread)
    {
        kthread_stop (chan->rx_thread);
        chan->rx_thread = NULL;
    }

    cpu_queues_free (enet_cpu_queues);
    kfree (enet_cpu_queues);
    enet_cpu_queues = NULL;

    return 0;
}

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
_STATIC_ void archer_spdsvc_init(void)
{
    archer_spdsvc_transmit_hook = bcmFun_get(BCM_FUN_ID_SPDSVC_TRANSMIT);
    BCM_ASSERT(archer_spdsvc_transmit_hook != NULL);
    archer_spdsvc_receive_hook = bcmFun_get(BCM_FUN_ID_SPDSVC_RECEIVE);
    BCM_ASSERT(archer_spdsvc_receive_hook != NULL);
}

_STATIC_INLINE_ int archer_spdsvc_transmit(dispatch_info_t *dispatch_info)
{
    pNBuff_t pNBuff = dispatch_info->pNBuff;

    if(IS_SKBUFF_PTR(pNBuff))
    {
        struct sk_buff *skb = PNBUFF_2_SKBUFF(pNBuff);
        spdsvcHook_transmit_t spdsvc_transmit = {};
        archer_spdsvc_tag_t tag;

        spdsvc_transmit.pNBuff = pNBuff;
        spdsvc_transmit.dev = dispatch_info->port->dev;
        spdsvc_transmit.header_type = SPDSVC_HEADER_TYPE_ETH;
        spdsvc_transmit.phy_overhead = BCM_ENET_OVERHEAD;
        spdsvc_transmit.egress_type = SPDSVC_EGRESS_TYPE_ENET;
        spdsvc_transmit.transmit_helper = NULL;

        tag.enet.egress_port = dispatch_info->port->n.blog_chnl;
        tag.enet.egress_queue = dispatch_info->egress_queue;
        tag.enet.tc = SKBMARK_GET_TC_ID(skb->mark);
        spdsvc_transmit.tag = tag.u32;

        return archer_spdsvc_transmit_hook(&spdsvc_transmit);
    }

    return 0;
}

_STATIC_INLINE_ int archer_spdsvc_receive(pNBuff_t pNBuff)
{
    spdsvcHook_receive_t spdsvc_receive;

    spdsvc_receive.pNBuff = pNBuff;
    spdsvc_receive.header_type = SPDSVC_HEADER_TYPE_ETH;
    spdsvc_receive.phy_overhead = BCM_ENET_OVERHEAD;

    return archer_spdsvc_receive_hook(&spdsvc_receive);
}
#else
#define archer_spdsvc_transmit(_dispatch_info)  0
#define archer_spdsvc_receive(_pNBuff)  0
#endif /* CONFIG_BCM_SPDSVC || CONFIG_BCM_SPDSVC_MODULE */

#if defined(CONFIG_BCM_ARCHER_MPM)
#define CC_ARCHER_ENET_MPM_ERROR_CHECK

#define ARCHER_ENET_MPM_FREE_RING_SIZE_LOG2   6 // 64

typedef struct {
    mpm_ring_index_t free_ring_index;
} archer_enet_mpm_t;

static archer_enet_mpm_t archer_enet_mpm_g;

static int archer_enet_mpm_init(void)
{
    int ret;

    ret = mpm_free_ring_alloc(ARCHER_ENET_MPM_FREE_RING_SIZE_LOG2,
                              &archer_enet_mpm_g.free_ring_index);
    if(ret)
    {
        enet_err("Could not mpm_free_ring_alloc");
    }

    return ret;
}

_STATIC_INLINE_ void archer_enet_mpm_buffer_free(uint8_t *pData)
{
    int retry_count = 1000;

    do {
        if(likely(!mpm_free_pdata(archer_enet_mpm_g.free_ring_index, pData)))
        {
            return;
        }
    } while(--retry_count);

#if defined(CC_ARCHER_ENET_MPM_ERROR_CHECK)
    enet_err("Could not mpm_free_pData");
#endif
}
#endif /* CC_SYSPORT_MPM */

static inline void enet_recycle_pdata(uint8_t *pData)
{
    ETH_GBPM_TRACK_BUF(fkb, GBPM_VAL_FREE, 0);

#if defined(CONFIG_BCM_ARCHER_MPM)
    archer_enet_mpm_buffer_free(pData);
#else
    gbpm_free_buf(pData);
#endif
}

_STATIC_INLINE_ void enet_recycle_handler(pNBuff_t pNBuff, unsigned long context, uint32_t flags)
{
    if ( IS_FKBUFF_PTR(pNBuff) )
    {
        /* Transmit driver is expected to perform cache invalidations */

        FkBuff_t* fkb = PNBUFF_2_FKBUFF(pNBuff);

        enet_recycle_pdata(PFKBUFF_TO_PDATA(fkb, BCM_PKT_HEADROOM));
    }
    else /* skb */
    {
        struct sk_buff *skb = PNBUFF_2_SKBUFF(pNBuff);

        if (likely(flags & SKB_DATA_RECYCLE))
        {
#if !defined(CONFIG_BCM_GLB_COHERENCY)
            void *data_startp = (void *)((uint8_t *)(skb->head) + BCM_PKT_HEADROOM);
            void *data_endp;
#if defined(CC_NBUFF_FLUSH_OPTIMIZATION)
            {
                void *dirty_p = skb_shinfo(skb)->dirty_p;
                void *shinfoBegin = (void*) skb_shinfo(skb);

                /* BPM skbs benefit from below. Linux skbs continue to relocate
                 * via skb_headerinit(). Caller would need to inform skb_headerinit
                 * of what is the true end of the data buffer. skb_headerinit
                 * may not assume bcm_pkt_lengths.h definitions.
                 */
                if ( (uintptr_t)shinfoBegin < ((uintptr_t)data_startp +
                                               (BPM_BUF_TO_END_OFFSET)) )
                {
                    void *shinfoEnd;
            
                    /* skb_shared_info is located in a DMA-able region.
                     * nr_frags could be ++/-- during sk_buff life.
                     */
                    shinfoEnd = shinfoBegin + sizeof(struct skb_shared_info);
                    cache_invalidate_region(shinfoBegin, shinfoEnd);
                }

                if (dirty_p) {
                    if ((dirty_p < (void *)skb->head) || (dirty_p > shinfoBegin)) {
                        printk("invalid dirty_p detected: %px valid=[%px %px]\n",
                               dirty_p, skb->head, shinfoBegin);
                        data_endp = shinfoBegin;
                    } else {
                        data_endp = (dirty_p < data_startp) ? data_startp : dirty_p;
                    }
                } else {
                    data_endp = shinfoBegin;
                }
            }
#else
            data_endp = (void *)(skb_shinfo(skb)) + sizeof(struct skb_shared_info);
#endif
            cache_invalidate_region(data_startp, data_endp);
#endif /* !CONFIG_BCM_GLB_COHERENCY */

            /* free the data buffer */
            enet_recycle_pdata((uint8_t *)(skb->head) + BCM_PKT_HEADROOM);
        }
#if defined(CC_ARCHER_ENET_DEBUG)
        else
        {
            printk("%s: Error only DATA recycle is supported\n", __FUNCTION__);
        }
#endif
    }
}

int enetxapi_rx_pkt (int q_id, FkBuff_t **fkb, enetx_rx_info_t *rx_info)
{
    bcm_async_queue_t *queue_p = &enet_cpu_queues->rxq[q_id];

    if (enet_cpu_queues->rx_notify_pending_disable)
    {
        enet_cpu_queues->rx_notify_enable = 0;
    }

    if(likely(bcm_async_queue_not_empty(queue_p)))
    {
        cpu_queue_rx_info_t *entry_p;
        cpu_queue_rx_info_t entry;

        ENET_CPU_STATS_UPDATE(queue_p->stats.reads);

        entry_p = (cpu_queue_rx_info_t *)bcm_async_queue_entry_read (queue_p);

        entry.pData = READ_ONCE(entry_p->pData);
        entry.info.u32 = READ_ONCE(entry_p->info.u32);

        bcm_async_queue_entry_dequeue (queue_p);

        if(likely(bcm_async_queue_not_empty(queue_p)))
        {
            cpu_queue_rx_info_t *next_entry_p =
                (cpu_queue_rx_info_t *)bcm_async_queue_entry_read (queue_p);
            uint8_t *next_pData = READ_ONCE(next_entry_p->pData);

            bcm_prefetch(next_pData);
        }

        *fkb = fkb_init (entry.pData, BCM_PKT_HEADROOM, entry.pData, entry.info.data_len);
        (*fkb)->recycle_hook = (RecycleFuncP)enet_recycle_handler;
        (*fkb)->recycle_context = 0;
#if defined(CONFIG_BCM_CSO)
        (*fkb)->rx_csum_verified = entry.info.rx_csum_verified;
#endif
        rx_info->src_port = entry.info.ingress_port;
        rx_info->extra_skb_flags = 0;
        rx_info->data_offset = 0;
        rx_info->is_group_fwd_exception = entry.info.group_fwd_exception;

        if(likely(!archer_spdsvc_receive(FKBUFF_2_PNBUFF((*fkb)))))
        {
            return 0;
        }
    }

    return -1;
}

int enetxapi_queue_need_reschedule (enetx_channel *chan, int q_id)
{
    return (int)(bcm_async_queue_not_empty(&enet_cpu_queues->rxq[q_id]));
}

void enetxapi_fkb_databuf_recycle(FkBuff_t *fkb)
{
    ETH_GBPM_TRACK_FKB(fkb, GBPM_VAL_RECYCLE, 0);

    enet_recycle_pdata(PFKBUFF_TO_PDATA(fkb, BCM_PKT_HEADROOM));
}

void enetxapi_buf_recycle(struct sk_buff *skb, uint32_t context, uint32_t flags)
{
    // SKB Recycle Function

#if defined(CC_ARCHER_ENET_DEBUG)
    if(flags & SKB_RECYCLE)
    {
        printk("\n\tERROR: enetxapi_buf_recycle: SKB_RECYCLE flag is set\n\n");
    }
#endif

    ETH_GBPM_TRACK_SKB(skb, GBPM_VAL_RECYCLE, 0);

    enet_recycle_handler(skb, context, flags);
}

int cpu_queues_tx_send (int tx_type, dispatch_info_t *dispatch_info)
{
    int rc = -1;

    if(likely(!archer_spdsvc_transmit(dispatch_info)))
    {
        unsigned long flags;

        ENET_CPU_TX_LOCK(tx_type, flags);

        if (bcm_async_queue_not_full (&enet_cpu_queues->txq[tx_type]))
        {
            cpu_queue_tx_info_t *tx_info_p = (cpu_queue_tx_info_t *)
                bcm_async_queue_entry_write (&enet_cpu_queues->txq[tx_type]);

            WRITE_ONCE(tx_info_p->pNBuff, dispatch_info->pNBuff);
            WRITE_ONCE(tx_info_p->egress_port, dispatch_info->port->n.blog_chnl);
            WRITE_ONCE(tx_info_p->egress_queue, dispatch_info->egress_queue);

            bcm_async_queue_entry_enqueue (&enet_cpu_queues->txq[tx_type]);

            ENET_CPU_STATS_UPDATE(enet_cpu_queues->txq[tx_type].stats.writes);

            rc = 0;
        }
        else
        {
            // queue is full, free the packet
#if defined(CONFIG_BCM_GLB_COHERENCY)
            nbuff_free(dispatch_info->pNBuff);
#else
            nbuff_flushfree(dispatch_info->pNBuff);
#endif
            ENET_CPU_STATS_UPDATE(enet_cpu_queues->txq[tx_type].stats.discards);
        }

        ENET_CPU_TX_UNLOCK(tx_type, flags);

        /* notify Archer regardless if the queue is full or not to make sure send can proceed */
        enet_cpu_queues->tx_notifier[tx_type]();
    }

    return rc;
}


_STATIC_INLINE_ void enet_cpu_queue_recycle_buffers(bcm_async_queue_t *recycleqp)
{
    int budget = ARCHER_ENET_RECYCLE_BUDGET;

    while(likely(budget-- && bcm_async_queue_not_empty(recycleqp)))
    {
        cpu_queue_recycle_info_t *recycle_p = (cpu_queue_recycle_info_t *)
            bcm_async_queue_entry_read (recycleqp);
        pNBuff_t pNBuff;

        pNBuff = READ_ONCE(recycle_p->pNBuff);

#if defined(CONFIG_BCM_GLB_COHERENCY)
        nbuff_free_ex(pNBuff, 0);
#else
        nbuff_flushfree_ex(pNBuff, 0);
#endif
        bcm_async_queue_entry_dequeue (recycleqp);

        ENET_CPU_STATS_UPDATE(recycleqp->stats.reads);
    }
}

static int enet_recycle_thread (void *arg)
{
    cpu_queues_t * cpu_queues = (cpu_queues_t *)arg;

    while (1)
    {
        wait_event_interruptible (cpu_queues->recycle_thread_wqh,
                                  cpu_queues->recycle_work_avail);
        if (kthread_should_stop())
        {
            printk(KERN_INFO "killing bcmsw_recycle\n");
            break;
        }

        enet_cpu_queue_recycle_buffers (&cpu_queues->recycleq);

        if (bcm_async_queue_not_empty (&cpu_queues->recycleq))
        {
            schedule();
        }
        else
        {
            clear_bit(0, &cpu_queues->recycle_work_avail);
        }
    }
    return 0;
}

static int cpu_queues_init (cpu_queues_t * cpu_queues)
{
    int i, entry_size, rc = 0;

    /* initialize the CPU RX queues */
    for (i=0; i < NUM_RX_QUEUES; i++)
    {
        // Make entrie size 32 bit aligned
        entry_size = ((sizeof(cpu_queue_rx_info_t) + 3) & ~3);

        rc = rc ? : bcm_async_queue_init (&cpu_queues->rxq[i], ENET_CPU_RX_QUEUE_SIZE, entry_size);
        if (rc)
        {
            printk("failed to initialize rx queue %d!!!!\n", i);
            return -1;
        }
    }

    // initialize the CPU TX queues
    for (i=0; i < NUM_TX_QUEUES; i++)
    {
        // Make entrie size 32 bit aligned
        entry_size = ((sizeof(cpu_queue_tx_info_t) + 3) & ~3);

        rc = rc ? : bcm_async_queue_init (&cpu_queues->txq[i], ENET_CPU_TX_QUEUE_SIZE, entry_size);
        if (rc)
        {
            printk("failed to initialize tx queue %d!!!!\n", i);
            return -1;
        }

        spin_lock_init (&cpu_queues->tx_lock[i]);
    }

    // recycle queue
    entry_size = ((sizeof(cpu_queue_recycle_info_t) + 3) & ~3);
    rc = rc ? : bcm_async_queue_init (&cpu_queues->recycleq, ENET_CPU_RECYCLE_Q_SIZE, entry_size);
    if (rc)
    {
        printk("failed to initialize recycle queue!\n");
        return -1;
    }

    init_waitqueue_head(&cpu_queues->recycle_thread_wqh);
    cpu_queues->recycle_thread = kthread_create(enet_recycle_thread, cpu_queues, "bcmsw_recycle");
    wake_up_process(cpu_queues->recycle_thread);

    return rc;
}

static int cpu_queues_free (cpu_queues_t * cpu_queues)
{
    int i;
    cpu_queue_rx_info_t * cpu_rxinfo;
    cpu_queue_tx_info_t * cpu_txinfo;
    cpu_queue_recycle_info_t *cpu_recycle;

    for (i=0; i < NUM_RX_QUEUES; i++)
    {
        /* make sure the queues are empty before freeing the queue */
        while (bcm_async_queue_not_empty (&cpu_queues->rxq[i]))
        {
            uint8_t *pData;

            cpu_rxinfo = (cpu_queue_rx_info_t *)bcm_async_queue_entry_read (&cpu_queues->rxq[i]);

            pData = READ_ONCE(cpu_rxinfo->pData);

            enet_recycle_pdata (pData);

            bcm_async_queue_entry_dequeue (&cpu_queues->rxq[i]);
        }
        kfree ((void *)cpu_queues->rxq[i].alloc_p);
    }
    for (i=0; i < NUM_TX_QUEUES; i++)
    {
        while (bcm_async_queue_not_empty (&cpu_queues->txq[i]))
        {
            pNBuff_t pNBuff;

            cpu_txinfo = (cpu_queue_tx_info_t *)bcm_async_queue_entry_read (&cpu_queues->txq[i]);

            pNBuff = READ_ONCE(cpu_txinfo->pNBuff);

#if defined(CONFIG_BCM_GLB_COHERENCY)
            nbuff_free(pNBuff);
#else
            nbuff_flushfree(pNBuff);
#endif
            bcm_async_queue_entry_dequeue (&cpu_queues->txq[i]);
        }
        kfree ((void *)cpu_queues->txq[i].alloc_p);
    }

    while(bcm_async_queue_not_empty(&cpu_queues->recycleq))
    {
        pNBuff_t pNBuff;

        cpu_recycle = (cpu_queue_recycle_info_t *)bcm_async_queue_entry_read (&cpu_queues->recycleq);

        pNBuff = READ_ONCE(cpu_recycle->pNBuff);

#if defined(CONFIG_BCM_GLB_COHERENCY)
        nbuff_free(pNBuff);
#else
        nbuff_flushfree(pNBuff);
#endif
        bcm_async_queue_entry_dequeue (&cpu_queues->recycleq);
    }
    kfree ((void *)cpu_queues->recycleq.alloc_p);

    return 0;
}

#if defined(CC_ENET_CPU_QUEUE_STATS)
static void enet_cpu_queue_dump(bcm_async_queue_t *queue_p, char *name, int index)
{
    printk("%s[%u]: Level %u/%u, Writes %u, Reads %u, Discards %u, Writes+Discards %u\n", name, index,
           bcm_async_queue_avail_entries(queue_p), queue_p->depth,
           queue_p->stats.writes, queue_p->stats.reads, queue_p->stats.discards,
           queue_p->stats.writes + queue_p->stats.discards);
}
#else
static void enet_cpu_queue_dump(bcm_async_queue_t *queue_p, char *name, int index)
{
    printk("%s[%u]: Level %u/%u, Write %u, Read %u\n", name, index,
           bcm_async_queue_avail_entries(queue_p), queue_p->depth,
           queue_p->write, queue_p->read);
}
#endif

void archer_enet_cpu_queue_stats(void)
{
    int i;

    for (i=0; i < NUM_RX_QUEUES; i++)
    {
        enet_cpu_queue_dump(&enet_cpu_queues->rxq[i], "ENET RXQ", i);
    }

    for (i=0; i < NUM_TX_QUEUES; i++)
    {
        enet_cpu_queue_dump(&enet_cpu_queues->txq[i], "ENET TXQ", i);
    }

    enet_cpu_queue_dump(&enet_cpu_queues->recycleq, "ENET RCQ", 0);

    printk("rx notify 0x%x (pending %u) chan->work_avail %lu\n",
           enet_cpu_queues->rx_notify_enable, enet_cpu_queues->rx_notify_pending_disable,
           enet_cpu_queues->chanp->rxq_cond);
}

/* API exposed for other modules to access the CPU queues (like archer) */
int archer_enet_tx_queue_notifier_register (int q_id, TX_NOTIFIER tx_notifier)
{
    enet_cpu_queues->tx_notifier[q_id] = tx_notifier;
    return 0;
}

int archer_enet_tx_queue_read (int q_id, pNBuff_t *ppNBuff, int *egress_port_p, int *egress_queue_p)
{
    bcm_async_queue_t *queue_p = &enet_cpu_queues->txq[q_id];

    if (bcm_async_queue_not_empty (queue_p))
    {
        cpu_queue_tx_info_t *tx_info_p = (cpu_queue_tx_info_t *)
            bcm_async_queue_entry_read (queue_p);

        *ppNBuff = READ_ONCE(tx_info_p->pNBuff);
        *egress_port_p = READ_ONCE(tx_info_p->egress_port);
        *egress_queue_p = READ_ONCE(tx_info_p->egress_queue);

        bcm_async_queue_entry_dequeue (queue_p);

        ENET_CPU_STATS_UPDATE(queue_p->stats.reads);

        return 0;
    }

    return -1;
}

int archer_enet_tx_queue_not_empty (int q_id)
{
    return bcm_async_queue_not_empty (&enet_cpu_queues->txq[q_id]);
}

int archer_enet_rx_queue_write (int q_id, uint8_t *pData, archer_cpu_rx_info_t cpu_rx_info)
{
    int rc = 0;

    if (bcm_async_queue_not_full (&enet_cpu_queues->rxq[q_id]))
    {
        cpu_queue_rx_info_t *entry_p = (cpu_queue_rx_info_t *)
            bcm_async_queue_entry_write (&enet_cpu_queues->rxq[q_id]);

        WRITE_ONCE(entry_p->pData, pData);
        WRITE_ONCE(entry_p->info.u32, cpu_rx_info.u32);

        bcm_async_queue_entry_enqueue (&enet_cpu_queues->rxq[q_id]);

        ENET_CPU_STATS_UPDATE(enet_cpu_queues->rxq[q_id].stats.writes);
    }
    else
    {
        ENET_CPU_STATS_UPDATE(enet_cpu_queues->rxq[q_id].stats.discards);

        rc = -1;
    }
    if (enet_cpu_queues->rx_notify_enable)
    {
        /* wake up enet rx thread */
        enetx_rx_isr (enet_cpu_queues->chanp);
    }
    else if (enet_cpu_queues->rx_notify_pending_disable)
    {
        enet_cpu_queues->rx_notify_pending_disable = 0;
    }
    return rc;
}

#define ENET_CPU_QUEUE_WAKEUP_RECYCLE_WORKER(x) do {                    \
        if ((x)->recycle_work_avail == 0) {                             \
            set_bit(0, &(x)->recycle_work_avail);                       \
            wake_up_interruptible(&((x)->recycle_thread_wqh)); }} while (0)

void archer_enet_recycle_queue_write(pNBuff_t pNBuff)
{
    if (likely(bcm_async_queue_not_full (&enet_cpu_queues->recycleq)))
    {
        cpu_queue_recycle_info_t *recycle_info = (cpu_queue_recycle_info_t *)
            bcm_async_queue_entry_write (&enet_cpu_queues->recycleq);

        WRITE_ONCE(recycle_info->pNBuff, pNBuff);

        bcm_async_queue_entry_enqueue (&enet_cpu_queues->recycleq);

        ENET_CPU_STATS_UPDATE(enet_cpu_queues->recycleq.stats.writes);
    }
    else
    {
#if defined(CONFIG_BCM_GLB_COHERENCY)
        nbuff_free(pNBuff);
#else
        nbuff_flushfree(pNBuff);
#endif
        ENET_CPU_STATS_UPDATE(enet_cpu_queues->recycleq.stats.discards);
    }
    ENET_CPU_QUEUE_WAKEUP_RECYCLE_WORKER(enet_cpu_queues);
}

static int archer_enet_host_bind(void)
{
    archer_host_hooks_t hooks;
    int rc;

    hooks.host_type = ARCHER_HOST_TYPE_ENET;
    hooks.tx_queue_notifier_register = archer_enet_tx_queue_notifier_register;
    hooks.tx_queue_read = archer_enet_tx_queue_read;
    hooks.tx_queue_not_empty = archer_enet_tx_queue_not_empty;
    hooks.rx_queue_write = archer_enet_rx_queue_write;
    hooks.recycle_queue_write = archer_enet_recycle_queue_write;
    hooks.queue_stats = archer_enet_cpu_queue_stats;

    rc = archer_gpl_enet_bind(&hooks);
    if(rc)
    {
        enet_err("Could not bind to Archer\n");
    }

    return rc;
}

#if defined(CC_ARCHER_ENET_DEBUG)
int cpu_queue_proc_print (int tick)
{
    int i;
    printk("=== CPU queue status====\n");
    printk(" CPU RX queues\n");
    for (i=0; i < NUM_RX_QUEUES; i++)
    {
        printk ("rxq %u read 0x%x write 0x%x\n", i, 
            enet_cpu_queues->rxq[i].read, enet_cpu_queues->rxq[i].write);
    }
    printk(" enet_rx notify enable 0x%x rx qfull 0x%x\n", enet_cpu_queues->rx_notify_enable, enet_cpu_queues->rx_full);

    printk("\n CPU TX queues\n");
    for (i=0; i < NUM_TX_QUEUES; i++)
    {
        printk ("txq %u read 0x%x write 0x%x\n", i, 
                enet_cpu_queues->txq[i].read, enet_cpu_queues->txq[i].write);
    }

    return 0;
}
#endif

int enetxapi_rx_pkt_dump_on_demux_err(enetx_rx_info_t *rx_info)
{
    return 1;
}

