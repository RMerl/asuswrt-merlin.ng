/*
   <:copyright-BRCM:2022:DUAL/GPL:standard

      Copyright (c) 2022 Broadcom 
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

#include <linux/kthread.h>
#include "enet.h"
#include "enet_dbg.h"
#include "enet_swqueue.h"

#if defined(ENET_SWQUEUE)
extern netdev_tx_t ___enet_xmit(pNBuff_t pNBuff, struct net_device *dev);

/**
 * =============================================================================
 * Section: ENET_SWQUEUE
 * =============================================================================
*/

/* SW queue budget */
#define ENET_SWQ_BOUND                  (128)


/** System global lock macros mutual exclusive access */
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
#define ENET_SWQUEUE_LOCK(swq)          spin_lock_bh(&((swq)->lock))
#define ENET_SWQUEUE_UNLK(swq)          spin_unlock_bh(&((swq)->lock))
#else
#define ENET_SWQUEUE_LOCK(swq)          local_irq_disable()
#define ENET_SWQUEUE_UNLK(swq)          local_irq_enable()
#endif  /* ! (CONFIG_SMP || CONFIG_PREEMPT) */


/**
 * -----------------------------------------------------------------------------
 *  enet_swqueue
 *
 *  State
 *  - Queue holding packets from ingress network devices
 *  - Queue Size
 *  - pktqueue_context registered with bcm_pktfwd
 *  - SWQ thread state
 *
 * -----------------------------------------------------------------------------
 */

struct enet_swqueue             /* enet SW queue state */
{
    spinlock_t          lock;               /* Queue lock */

    unsigned int        domain;             /* pktqueue_context domain */
    pktqueue_t          skb_pktqueue;       /* ENET SKB SW queue */
    pktqueue_t          fkb_pktqueue;       /* ENET FKB SW queue */
    uint32_t            swq_size;           /* ENET SW queue size */

    struct pktqueue_context * pktqueue_context_p;

    uint8_t             swq_schedule;       /* enet swq schedule state */
    uint32_t            schedule_cnt;       /* swq thread: swq xmit requests */
    uint32_t            complete_cnt;       /* swq thread: scheduled cnt */
    uint32_t            dispatches;         /* total xmit handler invocations */
    uint32_t            pkts_count;         /* pkts counts - xmited from swq */
    uint32_t            pkts_dropped;       /* dropped packets */
};

typedef struct enet_swqueue     enet_swqueue_t;

/** Callbacks registered with bcm_pktfwd */
static  bool    enet_swqueue_flush_pkts(void * driver, pktqueue_t * pktqueue);
static  void    enet_swqueue_flush_complete(void * driver);

/* ENET_SWQUEUE function to transmit packets from SW queue */
static inline uint32_t enet_swqueue_xmit(enet_swqueue_t * enet_swqueue,
    pktqueue_t * pktqueue, uint32_t budget, const NBuffPtrType_t NBuffPtrType);

/**
 * =============================================================================
 * Section: ENET Global System Object(s)
 * =============================================================================
 */

int enet_swq_bound = ENET_SWQ_BOUND;

/* Forward declaration */
struct enet_swqueue;
typedef struct enet_swqueue enet_swqueue_t;

#define ENET_SWQUEUE_NULL       ((enet_swqueue_t *) NULL)


/**
 * -----------------------------------------------------------------------------
 *
 * Singleton global object.
 *
 * - enet_kthread       : ENET process decriptor
 * - enet_kthread_wqh   : ENET wait queue
 * - enet_swqueue       : ENET SW queue context
 * -----------------------------------------------------------------------------
 */

struct enet_info    /* Global System State */
{

    struct task_struct    * enet_kthread;
    wait_queue_head_t       enet_kthread_wqh;
#if defined(ENET_SWQUEUE)
    enet_swqueue_t        * enet_swqueue;
#endif /* ENET_SWQUEUE */

};

typedef struct enet_info enet_info_t;

/** Static initialization of singleton system global object */
enet_info_t enet_info_g =
{
    .enet_kthread       = (struct task_struct *)NULL,
    .enet_kthread_wqh   = { },
#if defined(ENET_SWQUEUE)
    .enet_swqueue       = ENET_SWQUEUE_NULL,
#endif /* ENET_SWQUEUE */
};

#define ENET_SCHEDULE_WORK(enet_kthread_wqh)            \
    do {                                                \
        wake_up_interruptible((enet_kthread_wqh));      \
    } while (0)


/**
 * =============================================================================
 * Section: ENET SW QUEUE Functional Interface
 * =============================================================================
 */

/**
 * -----------------------------------------------------------------------------
 *
 * Function   : enet_swqueue_init
 * Description: Construct all ENET SW queue subsystems.
 *
 *              Initialize SW queue and register "flush" and "flush complete"
 *              handlers with bcm_pktfwd.
 *              These handlers are used by ingress network devices to enqueue
 *              packtes to SW queue and inform egress network device (ENET) for
 *              arrival of new packets in SW queue.
 *
 * Impl Caveat:
 *      ENET SW queue is serviced by DHD/WLAN-NIC device drivers and both
 *      drivers use only SKB buffers for upstream traffic so marked ENET
 *      pktqueue as SKBUFF_PTR.
 *
 * -----------------------------------------------------------------------------
 */

int
enet_swqueue_init(uint32_t swq_size)
{
    int                 mem_bytes;
    enet_swqueue_t    * enet_swqueue;
    enet_info_t       * enet_info = &enet_info_g;

    mem_bytes = sizeof(enet_swqueue_t);

    enet_swqueue = (enet_swqueue_t *) kmalloc(mem_bytes, GFP_ATOMIC);
    if (enet_swqueue == ENET_SWQUEUE_NULL)
    {
        enet_err("enet_swqueue kmalloc %d failure", mem_bytes);
        return ENET_FAILURE;
    }

    memset(enet_swqueue, 0, mem_bytes);

    spin_lock_init(&enet_swqueue->lock); /* Initialize swq lock */

    enet_swqueue->domain         = PKTQUEUE_XDOMAIN_IDX;
    enet_swqueue->swq_schedule   = 0; /* Schedule ENET SW queue dispatch */

    /* Validate and set queue size */
    if (swq_size < ENET_SWQUEUE_MIN_SIZE)
        swq_size = ENET_SWQUEUE_MIN_SIZE;
    else if (swq_size > ENET_SWQUEUE_MAX_SIZE)
        swq_size = ENET_SWQUEUE_MAX_SIZE;

    enet_swqueue->swq_size = swq_size;

    /* Initialize SKB pktqueue */
    enet_swqueue->skb_pktqueue.NBuffPtrType = SKBUFF_PTR;
    PKTQUEUE_RESET(&enet_swqueue->skb_pktqueue);  /* head,tail, not reset */

    /* Initialize FKB pktqueue */
    enet_swqueue->fkb_pktqueue.NBuffPtrType = FKBUFF_PTR;
    PKTQUEUE_RESET(&enet_swqueue->fkb_pktqueue);  /* head,tail, not reset */

    /* Register "flush" & "flush complete" handlers with bcm_pktfwd */
    enet_swqueue->pktqueue_context_p =
        pktqueue_context_register(enet_swqueue_flush_pkts,
                                  enet_swqueue_flush_complete,
                                  enet_info, enet_swqueue->domain);

    enet_info->enet_swqueue = enet_swqueue;

    if (enet_kthread_init())
        return -1;

    PKTQUEUE_TRACE("ENET swqueue initialized swq_size[%d]", swq_size);

    return ENET_SUCCESS;
}   /* enet_swqueue_init() */


/**
 * -----------------------------------------------------------------------------
 *
 * Function   : enet_swqueue_fini
 * Description: Destruct ENET SW queue subsystem.
 *
 * -----------------------------------------------------------------------------
 */

void
enet_swqueue_fini(void)
{
    enet_swqueue_t        * enet_swqueue;
    pktqueue_context_t    * pktqueue_context_p;
    enet_info_t           * enet_info = &enet_info_g;

    enet_swqueue = enet_info->enet_swqueue;

    enet_info->enet_swqueue = ENET_SWQUEUE_NULL;

    ENET_ASSERT(enet_swqueue != ENET_SWQUEUE_NULL);

    pktqueue_context_p = enet_swqueue->pktqueue_context_p;
    enet_swqueue->pktqueue_context_p = PKTQUEUE_CONTEXT_NULL;

    if (pktqueue_context_p != PKTQUEUE_CONTEXT_NULL)
    {
        /* Debug dump using pktqueue_context_dump() */
        pktqueue_context_dump(pktqueue_context_p);

        /* Free pktqueue_context resources */
        pktqueue_context_unregister(pktqueue_context_p);
    }

    memset(enet_swqueue, 0xff, sizeof(enet_swqueue_t));  /* scribble */
    kfree(enet_swqueue);

    PKTQUEUE_TRACE("ENET TX queue Destructed");

}   /* enet_swqueue_fini() */


/**
 * -----------------------------------------------------------------------------
 * Function : Dump ENET SW queue.
 * -----------------------------------------------------------------------------
 */

static void
enet_swqueue_dump(void)
{
    enet_swqueue_t        * enet_swqueue;
    enet_info_t           * enet_info = &enet_info_g;

    enet_swqueue = enet_info->enet_swqueue;

    if (enet_swqueue == ENET_SWQUEUE_NULL)
        return;

    /* Dump enet_swqueue stats */
    printk("\nENET SW queue stats:\n");
    printk("domain<%u> dispatch<%u>  pkts<%u> dropped<%u>\n",
            enet_swqueue->domain, enet_swqueue->dispatches,
            enet_swqueue->pkts_count, enet_swqueue->pkts_dropped);
    printk("schedule<%u>  complete<%u>\n\n", enet_swqueue->schedule_cnt,
            enet_swqueue->complete_cnt);

    if (enet_swqueue->pktqueue_context_p != PKTQUEUE_CONTEXT_NULL)
    {
        pktqueue_context_dump(enet_swqueue->pktqueue_context_p);
    }

}   /* enet_swqueue_dump() */


/**
 * =============================================================================
 * Section: ENET SW queue Packet Processing
 * =============================================================================
 */

/**
 * -----------------------------------------------------------------------------
 *
 * Callback function registered with bcm_pktfwd.
 *
 * Operation: Ingress device driver (WLAN) will bin packets into its domain
 * specific local pktqueue. Once all packets are binned, Ingress device driver
 * will invoke enet_swqueue_flush_pkts() to "flush" packets from WLAN pktqueue
 * to corresponding peer (ENET) pktqueue and wakeup egress network device (ENET)
 * thread using "flush complete" handle enet_swqueue_flush_complete().
 * enet_swqueue_flush_complete() will schedule ENET thread to transmit packets
 * from SW queue on to appropriate egress port (net_device) using  pktfwd_key
 * tagged to packet.
 *
 * enet_swqueue_flush_pkts() and enet_swqueue_flush_complete() are invoked in
 * ingress device driver "WLAN" thread context.
 *
 * Helper   : Helper functions used by ENET driver to flush packets are
 *    enet_swqueue_xfer_pkts() : Translate network buffers in ingress
 *                               pktqueue to SKB buffers and append it
 *                               to ENET SW queue.*
 *
 * -----------------------------------------------------------------------------
 */

/* Translate and transfer network buffers to enet SW queue */
static inline void
enet_swqueue_xfer_pkts(
    enet_swqueue_t  * enet_swqueue,
    pktqueue_t      * src_pktqueue,     /* producer's pktqueue */
    pktqueue_t      * dst_pktqueue,     /* consumer's pktqueue */
    const NBuffPtrType_t NBuffPtrType)
{

    ENET_ASSERT(src_pktqueue->len != 0U);

    /* Check for queue avail */
    /* TODO: Append avail len and drop remaining packets */
    if ((src_pktqueue->len + dst_pktqueue->len) > enet_swqueue->swq_size)
    {
        enet_swqueue->pkts_dropped += src_pktqueue->len;
        __pktqueue_free_pkts(src_pktqueue, NBuffPtrType);
    }
    else
    {
        __pktqueue_xfer_pkts(src_pktqueue, dst_pktqueue, NBuffPtrType);
    }

    PKTQUEUE_RESET(src_pktqueue); /* head,tail, not reset */

}   /* enet_swqueue_xfer_pkts() */


/** Flush pkts from Ingress pktqueue to ENET SW queue */
static bool
enet_swqueue_flush_pkts(void * driver, pktqueue_t * pktqueue)
{
    pktqueue_t        * enet_pktqueue;
    enet_swqueue_t    * enet_swqueue;
    enet_info_t       * enet_info = (enet_info_t *)driver;

    ENET_ASSERT(enet_info == &enet_info_g);

    enet_swqueue = enet_info->enet_swqueue;

    ENET_ASSERT(enet_swqueue != ENET_SWQUEUE_NULL);

    ENET_SWQUEUE_LOCK(enet_swqueue); // +++++++++++++++++++++++++++++++++++++++

    if (pktqueue->NBuffPtrType == SKBUFF_PTR)
    {
        enet_pktqueue = &enet_swqueue->skb_pktqueue;
        enet_swqueue_xfer_pkts(enet_swqueue, pktqueue,
            enet_pktqueue, SKBUFF_PTR);
    }
    else /* pktqueue->NBuffPtrType == FKBUFF_PTR */
    {
        enet_pktqueue = &enet_swqueue->fkb_pktqueue;
        enet_swqueue_xfer_pkts(enet_swqueue, pktqueue,
            enet_pktqueue, FKBUFF_PTR);
    }

    ENET_SWQUEUE_UNLK(enet_swqueue); // ---------------------------------------

    return true;
}   /* enet_swqueue_flush_pkts() */


/* Flush complete invoked by ingress driver (WLAN) */
static void
enet_swqueue_flush_complete(void * driver)
{
    enet_swqueue_t    * enet_swqueue;
    enet_info_t       * enet_info = (enet_info_t *)driver;

    ENET_ASSERT(enet_info == &enet_info_g);

    enet_swqueue = enet_info->enet_swqueue;

    ENET_ASSERT(enet_swqueue != ENET_SWQUEUE_NULL);

    if (likely(enet_swqueue->swq_schedule == 0))
    {
        enet_swqueue->schedule_cnt++;
        enet_swqueue->swq_schedule = ~0;

        /* Wake up ENET thread to xmit packets from SW queue */
        ENET_SCHEDULE_WORK(&enet_info->enet_kthread_wqh);
    }
}   /* enet_swqueue_flush_complete() */


/**
 * -----------------------------------------------------------------------------
 *
 * Function   : enet_swqueue_xmit
 * Description: Extract packets from SW queue and transmit on to appropriate
 *              egress port (net_device) using pktfwd_key tagged to packet.
 * -----------------------------------------------------------------------------
 */

static inline uint32_t
enet_swqueue_xmit(enet_swqueue_t  * enet_swqueue,
                  pktqueue_t      * pktqueue,
                  uint32_t          budget,
                  const NBuffPtrType_t NBuffPtrType)
{
    uint32_t            rx_pktcnt;
    d3lut_key_t         d3lut_key;
    pktqueue_t          temp_pktqueue;  /* Declared on stack */
    d3lut_elem_t      * d3lut_elem;
    pktqueue_pkt_t    * pkt;

    ENET_SWQUEUE_LOCK(enet_swqueue); // +++++++++++++++++++++++++++++++++++++++

    /* Transfer packets to a local pktqueue */
    temp_pktqueue.head   = pktqueue->head;
    temp_pktqueue.tail   = pktqueue->tail;
    temp_pktqueue.len    = pktqueue->len;

    PKTQUEUE_RESET(pktqueue); /* head,tail, not reset */

    ENET_SWQUEUE_UNLK(enet_swqueue); // ---------------------------------------

    /* Now lock-less; transmit packets from local pktqueue */

    d3lut_key.v16 = 0; /* 2b-radio, 2b-incarn, 12b-dest */
    rx_pktcnt = 0;

    while (budget)
    {
        if (temp_pktqueue.len != 0U)
        {
            pkt             = temp_pktqueue.head;
            temp_pktqueue.head  = PKTQUEUE_PKT_SLL(pkt, NBuffPtrType);
            PKTQUEUE_PKT_SET_SLL(pkt, PKTQUEUE_PKT_NULL, NBuffPtrType);
            temp_pktqueue.len--;

            d3lut_key.v16 = PKTQUEUE_PKT_KEY(pkt, NBuffPtrType);

            d3lut_elem = d3lut_k2e(d3lut_gp, d3lut_key);
            if (likely(d3lut_elem != D3LUT_ELEM_NULL))
            {
                 ___enet_xmit((pNBuff_t) pkt, d3lut_elem->ext.net_device);
                 ++rx_pktcnt;
            }
            else
            {
                PKTQUEUE_PKT_FREE(pkt);
                enet_swqueue->pkts_dropped++;
            }
        }
        else /* temp_pktqueue.len == 0 : No more packets to read */
        {
            break;
        }

        --budget;
    } /* while (budget) */

    if (temp_pktqueue.len != 0U) {
        /* Out of budget, prepend left-over packets to ENET SWq */

        ENET_SWQUEUE_LOCK(enet_swqueue); // +++++++++++++++++++++++++++++++++++

        if (pktqueue->len == 0) {
            pktqueue->tail = temp_pktqueue.tail;
        } else {
            PKTQUEUE_PKT_SET_SLL(temp_pktqueue.tail, pktqueue->head,
                                    NBuffPtrType);
        }

        pktqueue->head = temp_pktqueue.head;
        pktqueue->len += temp_pktqueue.len;

        ENET_SWQUEUE_UNLK(enet_swqueue); // -----------------------------------

        PKTQUEUE_RESET(&temp_pktqueue); /* head,tail, not reset */

    }

    return rx_pktcnt;

}   /* enet_swqueue_xmit() */


void enet_swqueue_kthread_handler(void *context)
{
    enet_info_t       * enet_info = (enet_info_t *)context;
    enet_swqueue_t    * enet_swqueue = enet_info->enet_swqueue;

    /* Dispatch packets from SW queues bounded by ENET_SWQ_BOUND */
    if (enet_swqueue->swq_schedule)
    {
        /* Transmit packets from SKB based SW queue */
        if (enet_swqueue->skb_pktqueue.len != 0U)
        {
            enet_swqueue->pkts_count += enet_swqueue_xmit(enet_swqueue,
                    &enet_swqueue->skb_pktqueue, enet_swq_bound, SKBUFF_PTR);
        }

        /* Transmit packets from FKB based SW queue */
        if (enet_swqueue->fkb_pktqueue.len != 0U)
        {
            enet_swqueue->pkts_count += enet_swqueue_xmit(enet_swqueue,
                    &enet_swqueue->fkb_pktqueue, enet_swq_bound, FKBUFF_PTR);
        }

        enet_swqueue->dispatches++;
        enet_swqueue->complete_cnt++;
        if ((enet_swqueue->skb_pktqueue.len == 0U) &&
                (enet_swqueue->fkb_pktqueue.len == 0U))
        {
            /* SW queue is empty, clear swq_schedule state */
            enet_swqueue->swq_schedule = 0U;
        }
    }
}

/**
 * -----------------------------------------------------------------------------
 * Function : Dump all ENET global systems.
 * Usage: "echo dump > /proc/driver/enet/cmd"
 * -----------------------------------------------------------------------------
 */

void
enet_sys_dump(void)
{
    enet_swqueue_dump();
}   /* enet_sys_dump() */


/**
 * -----------------------------------------------------------------------------
 * Function : enet_kthread_handler
 * -----------------------------------------------------------------------------
 */

static uint8_t _should_schedule_swqueue(void *context)
{
#if defined(ENET_SWQUEUE)
    enet_info_t       * enet_info = (enet_info_t *)context;
    enet_swqueue_t    * enet_swqueue = enet_info->enet_swqueue;


    return enet_swqueue->swq_schedule;
#endif
    return 0;
}

static int
enet_kthread_handler(void *context)
{
    enet_info_t       * enet_info = (enet_info_t *)context;

    ENET_ASSERT(enet_info == &enet_info_g);
    enet_err("Instantiating ENET thread\n");

    while (1)
    {
        wait_event_interruptible(enet_info->enet_kthread_wqh,
                                 _should_schedule_swqueue(context) ||
                                 kthread_should_stop());

        if (kthread_should_stop())
        {
            enet_err(KERN_INFO "kthread_should_stop detected in enet\n");
            break;
        }

        enet_swqueue_kthread_handler(context);
    }

    return ENET_SUCCESS;
}   /* enet_kthread_handler() */

int enet_kthread_init(void)
{
    char                    threadname[32] = {0};
    struct task_struct    * enet_kthread;
    enet_info_t           * enet_info = &enet_info_g;

    /* Create ENET Kernel thread and wait queue. */
    init_waitqueue_head(&enet_info->enet_kthread_wqh);

    sprintf(threadname, ENET_THREAD_NAME);
    enet_kthread = kthread_create(enet_kthread_handler, enet_info, threadname);

    if (IS_ERR(enet_kthread)) {
        enet_err("Failed to create %s kthread\n", threadname);
        return (int)PTR_ERR(enet_kthread);
    }

    enet_info->enet_kthread = enet_kthread;
    /* TODO: Should bind enet thread to a CPU core.
     * kthread_bind()*/
    wake_up_process(enet_kthread);

    enet_err("ENET system contructed and configured %s thread\n", threadname);
    return 0;
}

#endif
