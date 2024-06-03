/*
   <:copyright-BRCM:2023:DUAL/GPL:standard
   
      Copyright (c) 2023 Broadcom 
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
 *  Implementation of enetxapi and data structures for enet driver using Crossbow Queues
 */

#include <linux/nbuff.h>
#include <linux/kthread.h>
#include <linux/bcm_log.h>
#include <linux/bcm_skb_defines.h>

#include "enet.h"
#include "enet_types.h"

#include "crossbow_gpl.h"

#define CC_CROSSBOW_ENET_DEBUG

// FIXME: Which CPU ID?
#define CROSSBOW_ENET_RXQ_ISR_CPU_ID  0

// FIXME: Which CPU ID?
#define CROSSBOW_ENET_RCQ_ISR_CPU_ID  0

#define CROSSBOW_ENET_RECYCLE_BUDGET  256

#define CROSSBOW_ENET_NAPI_WEIGHT     32

extern int enetx_weight_budget;
extern int chan_thread_handler(void *data);

static int crossbow_enet_recycle_thread(void *arg);

static int crossbow_enet_host_bind(void);

/**********************************************************************************
 *
 * enetxapi
 *
 **********************************************************************************/

#define CROSSBOW_ENET_RXQ_PRIO_HIGH  0
#define CROSSBOW_ENET_RXQ_PRIO_LOW   1

#define CROSSBOW_ENET_TX_LOCK(_flags)                           \
    spin_lock_irqsave(&crossbow_enet_g.tx_lock, (_flags))

#define CROSSBOW_ENET_TX_UNLOCK(_flags)                         \
    spin_unlock_irqrestore(&crossbow_enet_g.tx_lock, (_flags))

#define CROSSBOW_ENET_RX_LOCK(_flags)                      \
    spin_lock_irqsave(&crossbow_enet_g.rx_lock, (_flags))

#define CROSSBOW_ENET_RX_UNLOCK(_flags)                    \
    spin_unlock_irqrestore(&crossbow_enet_g.rx_lock, (_flags))

typedef struct {
    uint32_t rxq_reads[CROSSBOW_RXQ_MAX];
    uint32_t txq_writes;
    uint32_t txq_discards;
    uint32_t rcq_reads;
} crossbow_enet_stats_t;

typedef struct {
    volatile unsigned long work_avail;
    wait_queue_head_t thread_wqh;
    struct task_struct *thread;
} crossbow_enet_recycle_t;

typedef struct {
    enetx_channel *chanp;
    spinlock_t tx_lock;
    spinlock_t rx_lock;
    crossbow_enet_recycle_t recycle;
    crossbow_enet_stats_t stats;
} crossbow_enet_t;

static crossbow_enet_t crossbow_enet_g;

void enetxapi_queue_int_enable(enetx_channel *chan, int q_id)
{
    crossbow_gpl_rxq_intr_enable(chan->rx_q[q_id]);
}

void enetxapi_queue_int_disable(enetx_channel *chan, int q_id)
{
    crossbow_gpl_rxq_intr_disable(chan->rx_q[q_id]);

    crossbow_gpl_rxq_intr_clear(chan->rx_q[q_id]);
}

int enetxapi_queues_init(enetx_channel ** _chan)
{
    enetx_channel *chan;
    int ret;

    memset(&crossbow_enet_g, 0, sizeof(crossbow_enet_t));

    ret = crossbow_enet_host_bind();
    if(ret)
    {
        return ret;
    }

    // Enet Channel
    chan = kzalloc(sizeof(enetx_channel), GFP_KERNEL);
    if(!chan)
    {
        return -1;
    }

    chan->rx_q_count = CROSSBOW_RXQ_MAX;
    chan->rx_q[0] = CROSSBOW_ENET_RXQ_PRIO_HIGH;
    chan->rx_q[1] = CROSSBOW_ENET_RXQ_PRIO_LOW;

    // FIXME: Is this a bug? (stack variable)
    *_chan = chan;

    init_waitqueue_head (&chan->rxq_wqh);
    chan->rx_thread = kthread_run(chan_thread_handler, chan, "bcmsw_rx");

    crossbow_enet_g.chanp = chan;

    // only 1 channel for now
    chan->next = NULL;

    enetx_weight_budget = CROSSBOW_ENET_NAPI_WEIGHT;

    // Tx and Recycle spinlocks
    spin_lock_init(&crossbow_enet_g.tx_lock);
    spin_lock_init(&crossbow_enet_g.rx_lock);

    // Recycle Thread
    init_waitqueue_head(&crossbow_enet_g.recycle.thread_wqh);
    crossbow_enet_g.recycle.thread = kthread_create(crossbow_enet_recycle_thread,
                                                    NULL, "bcmsw_recycle");
    wake_up_process(crossbow_enet_g.recycle.thread);

    // Enable Recycle Queue Interrupts
    crossbow_gpl_rcq_intr_enable();

    return 0;
}


int enetxapi_queues_uninit(enetx_channel **_chan)
{
    enetx_channel * chan = *_chan;

    if(chan->next)
    {
        enet_err("only one channel is supported now\n");

        return -1;
    }

    if(chan->rx_thread)
    {
        kthread_stop (chan->rx_thread);

        chan->rx_thread = NULL;
    }

    return 0;
}

static inline void crossbow_enet_fkb_free(FkBuff_t *fkb)
{
    unsigned long flags;

    CROSSBOW_ENET_RX_LOCK(flags);

    crossbow_gpl_rcq_fkb_free(fkb);

    CROSSBOW_ENET_RX_UNLOCK(flags);
}

static inline void crossbow_enet_recycle_handler(pNBuff_t pNBuff, unsigned long context,
                                                 uint32_t flags)
{
    if(IS_FKBUFF_PTR(pNBuff))
    {
        /* Transmit driver is expected to perform cache invalidations */

        FkBuff_t *fkb = PNBUFF_2_FKBUFF(pNBuff);

        crossbow_enet_fkb_free(fkb);
    }
    else /* skb */
    {
        struct sk_buff *skb = PNBUFF_2_SKBUFF(pNBuff);

        if(likely(flags & SKB_DATA_RECYCLE))
        {
            FkBuff_t *fkb = PHEAD_TO_PFKBUFF(skb->head); 

            crossbow_enet_fkb_free(fkb);
        }
#if defined(CC_CROSSBOW_ENET_DEBUG)
        else
        {
            bcm_print("%s: Error only DATA recycle is supported\n", __FUNCTION__);
        }
#endif
    }
}

int enetxapi_rx_pkt(int q_id, FkBuff_t **fkb, enetx_rx_info_t *rx_info)
{
    crossbow_gpl_rxq_entry_t entry;
    int ret;

    ret = crossbow_gpl_rxq_read(q_id, &entry);
    if(likely(!ret))
    {
        *fkb = fkb_init(entry.pData, BCM_PKT_HEADROOM, entry.pData, entry.data_len);
        (*fkb)->recycle_hook = (RecycleFuncP)crossbow_enet_recycle_handler;
        (*fkb)->recycle_context = 0;
#if defined(CONFIG_BCM_CSO)
        (*fkb)->rx_csum_verified = entry.rx_csum_verified;
#endif
        rx_info->src_port = entry.ingress_port;
        rx_info->extra_skb_flags = 0;
        rx_info->data_offset = 0;
        rx_info->is_group_fwd_exception = entry.group_fwd_exception;

        crossbow_enet_g.stats.rxq_reads[q_id]++;
    }

    return ret;
}

int enetxapi_queue_need_reschedule (enetx_channel *chan, int q_id)
{
    return crossbow_gpl_rxq_not_empty(q_id);
}

static irqreturn_t crossbow_enet_rxq_isr(int irq, void *param)
{
    /* wake up enet rx thread */
    enetx_rx_isr(crossbow_enet_g.chanp);

    return IRQ_HANDLED;
}

void enetxapi_fkb_databuf_recycle(FkBuff_t *fkb)
{
    crossbow_enet_fkb_free(fkb);
}

void enetxapi_buf_recycle(struct sk_buff *skb, uint32_t context, uint32_t flags)
{
    // SKB Recycle Function

#if defined(CC_CROSSBOW_ENET_DEBUG)
    if(flags & SKB_RECYCLE)
    {
        bcm_print("\n\tERROR: enetxapi_buf_recycle: SKB_RECYCLE flag is set\n\n");
    }
#endif

    crossbow_enet_recycle_handler(skb, context, flags);
}

int cpu_queues_tx_send(int tx_type, dispatch_info_t *dispatch_info)
{
    crossbow_gpl_txq_entry_t entry;
    unsigned long flags;
    int ret;

    entry.pNBuff = dispatch_info->pNBuff;
    entry.egress_port = dispatch_info->port->n.blog_chnl;
    entry.egress_queue = dispatch_info->egress_queue;

    CROSSBOW_ENET_TX_LOCK(flags);

    ret = crossbow_gpl_txq_write(&entry);

    if(likely(!ret))
    {
        crossbow_enet_g.stats.txq_writes++;
    }
    else
    {
        nbuff_free(dispatch_info->pNBuff);

        crossbow_enet_g.stats.txq_discards++;
    }

    CROSSBOW_ENET_TX_UNLOCK(flags);

    return ret;
}

static int crossbow_enet_recycle_thread(void *arg)
{
    unsigned long flags;
    pNBuff_t pNBuff;
    int rcq_not_empty;
    int budget;

    bcm_print("Ethernet Recycle Thread Initialized\n");

    while(1)
    {
        wait_event_interruptible(crossbow_enet_g.recycle.thread_wqh,
                                 crossbow_enet_g.recycle.work_avail);
        if(kthread_should_stop())
        {
            printk(KERN_INFO "killing bcmsw_recycle\n");

            break;
        }

        CROSSBOW_ENET_TX_LOCK(flags);

        budget = CROSSBOW_ENET_RECYCLE_BUDGET;

        while(likely(budget-- && !crossbow_gpl_rcq_read(&pNBuff)))
        {
//            nbuff_free_ex(pNBuff, 0);
            nbuff_free(pNBuff);

            crossbow_enet_g.stats.rcq_reads++;
        }

        rcq_not_empty = crossbow_gpl_rcq_not_empty();

        CROSSBOW_ENET_TX_UNLOCK(flags);

        if(rcq_not_empty)
        {
            schedule();
        }
        else
        {
            clear_bit(0, &crossbow_enet_g.recycle.work_avail);

            crossbow_gpl_rcq_intr_enable();
        }
    }

    return 0;
}

static irqreturn_t crossbow_enet_rcq_isr(int irq, void *param)
{
    crossbow_gpl_rcq_intr_disable();

    crossbow_gpl_rcq_intr_clear();

    set_bit(0, &crossbow_enet_g.recycle.work_avail);

    wake_up_interruptible(&crossbow_enet_g.recycle.thread_wqh);

    return IRQ_HANDLED;
}

static void crossbow_enet_stats_dump(void)
{
    crossbow_gpl_enet_stats_t stats;
    int i;

    crossbow_gpl_enet_stats_get(&stats);

    bcm_print("ENET\n");

    for(i=0; i<CROSSBOW_RXQ_MAX; i++)
    {
        crossbow_gpl_rxq_stats_t *rxq_stats = &stats.rxq[i];

        bcm_print("\tRXQ[%d]: Level %u/%u, Packets %u, Discards %u\n",
                  i, rxq_stats->level, rxq_stats->size,
                  crossbow_enet_g.stats.rxq_reads[i], rxq_stats->discards);
    }

    bcm_print("\tTXQ: Level %u/%u, Packets %u, Discards %u, In Flight %u\n",
              stats.txq.level, stats.txq.size,
              crossbow_enet_g.stats.txq_writes,
              crossbow_enet_g.stats.txq_discards,
              crossbow_enet_g.stats.txq_writes - crossbow_enet_g.stats.rcq_reads);

    bcm_print("\tRCQ: Level %u/%u, Packets %u, Backpressure %u\n",
              stats.rcq.level, stats.rcq.size,
              crossbow_enet_g.stats.rcq_reads, stats.rcq.bp_count);
}

static int crossbow_enet_host_bind(void)
{
    crossbow_enet_hooks_t hooks;
    int rc;

    hooks.rxq_isr = crossbow_enet_rxq_isr;
    hooks.rxq_isr_cpu_id = CROSSBOW_ENET_RXQ_ISR_CPU_ID;
    hooks.rcq_isr = crossbow_enet_rcq_isr;
    hooks.rcq_isr_cpu_id = CROSSBOW_ENET_RCQ_ISR_CPU_ID;
    hooks.stats_dump = crossbow_enet_stats_dump;

    rc = crossbow_gpl_enet_bind(&hooks);
    if(rc)
    {
        enet_err("Could not bind to Archer\n");
    }

    return rc;
}

int enetxapi_rx_pkt_dump_on_demux_err(enetx_rx_info_t *rx_info)
{
    return 1;
}
