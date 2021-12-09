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
//#include <linux/rtnetlink.h>
//#include <linux/ethtool.h>
//#include <linux/if_arp.h>
#include <linux/ppp_channel.h>
#include <linux/ppp_defs.h>
#include <linux/if_ppp.h>
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <linux/atmppp.h>
#include <linux/blog.h>
//#include <linux/proc_fs.h>
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
#include "xtmrt_archer.h"
#include <bpm.h>
#include <linux/gbpm.h>
#include <linux/bcm_log.h>
#include "bcm_prefetch.h"

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
#include "spdsvc_defs.h"
static bcmFun_t *spdsvc_receive_hook = NULL;
#endif

/**** Externs ****/

extern int bcmxtmrt_in_init_dev;


/**** Globals ****/


/**** Prototypes ****/

static void bcmxtmrt_timer(PBCMXTMRT_GLOBAL_INFO pGi);

static int archer_xtm_host_bind(void);

/**** Statics ****/
#include <bcm_async_queue.h>
#include <archer_cpu_queues.h>
#include <linux/nbuff.h>
#include <linux/kthread.h>

/* CPU queues */
#define XTM_NUM_RX_Q    2
typedef struct xtm_queues_t
{
    bcm_async_queue_t rxq[XTM_NUM_RX_Q];
    bcm_async_queue_t txq;

    spinlock_t tx_lock;
    TX_NOTIFIER tx_notifier;

    int rx_notify_enable;
    int rx_notify_pending_disable;

    /* buffer recycling thread */
    volatile unsigned long recycle_work_avail;
    wait_queue_head_t recycle_thread_wqh;
    struct task_struct *recycle_thread;
    bcm_async_queue_t recycleq;

} xtm_queues_t;

xtm_queues_t *xtm_cpu_queues = NULL;
static int xtm_recycle_thread (void *arg);

typedef struct xtm_queue_rx_info_t 
{
    uint8_t * pData;
    uint16_t length;
    uint16_t ingress_port;
    uint16_t desc_status;

} xtm_queue_rx_info_t;

typedef struct xtm_queue_tx_info_t
{
    pNBuff_t pNBuff;
    uint16_t egress_queue;
    uint16_t desc_status;

} xtm_queue_tx_info_t;

typedef struct xtm_queue_recycle_info_t
{
    pNBuff_t pNBuff;

} xtm_queue_recycle_info_t;

archer_xtm_hooks_t archer_xtm_hooks;

#define CC_XTM_CPU_QUEUE_STATS


#if defined(CC_XTM_CPU_QUEUE_STATS)
#define XTM_CPU_STATS_UPDATE(_counter) ( (_counter)++ )
#else
#define XTM_CPU_STATS_UPDATE(_counter)
#endif

#define XTM_CPU_RECYCLE_BUDGET  XTMRT_BUDGET*2 // make recycle budget larger

static int archer_xtm_bind(void *arg_p)
{
    archer_xtm_hooks_t *xtmHooks = (archer_xtm_hooks_t *)arg_p;

    archer_xtm_hooks = *xtmHooks;

    printk("archer xtm API binded\n");

    return 0;
}

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
    int entry_size, rc = 0;

    printk ("bcmxtm module init...... \n");

    memset (&archer_xtm_hooks, 0, sizeof(archer_xtm_hooks_t));

    xtm_cpu_queues = kzalloc (sizeof (xtm_queues_t), GFP_KERNEL);
    entry_size = ((sizeof(xtm_queue_rx_info_t) + 3) & ~3);
    rc = rc ? : bcm_async_queue_init (&xtm_cpu_queues->rxq[0], XTM_CPU_RX_QUEUE_SIZE, entry_size);
    rc = rc ? : bcm_async_queue_init (&xtm_cpu_queues->rxq[1], XTM_CPU_RX_QUEUE_SIZE, entry_size);

    entry_size = ((sizeof(xtm_queue_tx_info_t) + 3) & ~3);
    rc = rc ? : bcm_async_queue_init (&xtm_cpu_queues->txq, XTM_CPU_TX_QUEUE_SIZE, entry_size);
    spin_lock_init (&xtm_cpu_queues->tx_lock);

    entry_size = ((sizeof(xtm_queue_recycle_info_t) + 3) & ~3);
    rc = rc ? : bcm_async_queue_init (&xtm_cpu_queues->recycleq, XTM_CPU_RECYCLE_Q_SIZE, entry_size);

    // create recycle thread
    init_waitqueue_head (&xtm_cpu_queues->recycle_thread_wqh);
    xtm_cpu_queues->recycle_thread = kthread_create (xtm_recycle_thread, xtm_cpu_queues, "bcmxtm_recycle");
    wake_up_process (xtm_cpu_queues->recycle_thread);

    bcmFun_reg(BCM_FUN_ID_ARCHER_XTMRT_BIND, archer_xtm_bind);

    rc = rc ? : archer_xtm_host_bind();

    if (rc != 0)
    {
        printk(KERN_ERR "bcmxtm initialization failed\n");
        rc = -1;
    }

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
    spdsvc_receive_hook = bcmFun_get(BCM_FUN_ID_SPDSVC_RECEIVE);
    BCM_ASSERT(spdsvc_receive_hook != NULL);
#endif

    return 0;
}  /* bcmxapi_module_init() */

static void xtm_recycle_pdata (uint8_t *pData)
{
    gbpm_free_buf(pData);
}

static int cpu_queues_free (void)
{
    int i;
    xtm_queue_rx_info_t * rx_info;
    xtm_queue_tx_info_t * tx_info;
    xtm_queue_recycle_info_t *recycle_info;

    for (i=0; i < 2; i++)
    {
        /* make sure the queues are empty before freeing the queue */
        while (bcm_async_queue_not_empty (&xtm_cpu_queues->rxq[i]))
        {
            uint8_t *pData;

            rx_info = (xtm_queue_rx_info_t *)bcm_async_queue_entry_read (&xtm_cpu_queues->rxq[i]);

            pData = READ_ONCE(rx_info->pData);

            xtm_recycle_pdata (pData);

            bcm_async_queue_entry_dequeue (&xtm_cpu_queues->rxq[i]);

            XTM_CPU_STATS_UPDATE(xtm_cpu_queues->rxq[i].stats.reads);
        }
        kfree ((void *)xtm_cpu_queues->rxq[i].alloc_p);
    }
    while (bcm_async_queue_not_empty (&xtm_cpu_queues->txq))
    {
        pNBuff_t pNBuff;

        tx_info = (xtm_queue_tx_info_t *)bcm_async_queue_entry_read (&xtm_cpu_queues->txq);

        pNBuff = READ_ONCE(tx_info->pNBuff);

#if defined(CONFIG_BCM_GLB_COHERENCY)
        nbuff_free(pNBuff);
#else
        nbuff_flushfree(pNBuff);
#endif
        bcm_async_queue_entry_dequeue (&xtm_cpu_queues->txq);

        XTM_CPU_STATS_UPDATE(xtm_cpu_queues->txq.stats.reads);
    }
    kfree ((void *)xtm_cpu_queues->txq.alloc_p);

    while(bcm_async_queue_not_empty(&xtm_cpu_queues->recycleq))
    {
        pNBuff_t pNBuff;

        recycle_info = (xtm_queue_recycle_info_t *)bcm_async_queue_entry_read (&xtm_cpu_queues->recycleq);

        pNBuff = READ_ONCE(recycle_info->pNBuff);

#if defined(CONFIG_BCM_GLB_COHERENCY)
        nbuff_free(pNBuff);
#else
        nbuff_flushfree(pNBuff);
#endif
        bcm_async_queue_entry_dequeue (&xtm_cpu_queues->recycleq);

        XTM_CPU_STATS_UPDATE(xtm_cpu_queues->recycleq.stats.reads);
    }
    kfree ((void *)xtm_cpu_queues->recycleq.alloc_p);

    return 0;
}

/*---------------------------------------------------------------------------
 * void bcmxapi_module_cleanup(void)
 * Description:
 *    Called when the driver is unloaded.
 * Returns: void
 *---------------------------------------------------------------------------
 */
void bcmxapi_module_cleanup(void)
{
    printk("bcmxtm module cleanup\n");

    bcmxapi_disable_rx_interrupt();
    cpu_queues_free();
    kfree ((void *)xtm_cpu_queues);
    xtm_cpu_queues = NULL;

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
    xtm_cpu_queues->rx_notify_enable = 1;
    xtm_cpu_queues->rx_notify_pending_disable = 0;

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
    if (xtm_cpu_queues->rx_notify_pending_disable == 0)
        xtm_cpu_queues->rx_notify_pending_disable = 1;

    return 0;
   
}  /* bcmxapi_disable_rx_interrupt() */

int bcmxapi_XtmCreateDevice (uint32_t devId, uint32_t encapType, uint32_t headerLen, uint32_t trailerLen)
{
    /* device details is handled in LinkUp message instead */
    return 1;
}

int bcmxapi_XtmLinkUp (uint32_t devId, uint32_t matchId)
{
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;
    PBCMXTMRT_DEV_CONTEXT pDevCtx = pGi->pDevCtxs[devId];
    uint32_t headerLen = 0, trailerLen = 0, encap = 0, trafficType = 0;
    UINT16 bufStatus = 0;

    /* determine the desc_status required for txdma for accelerated traffic */
    bcmxtmrt_get_bufStatus(pDevCtx, 0, &bufStatus);

    encap = pDevCtx->ulEncapType;
    trafficType = pDevCtx->ulTrafficType;

    if (pDevCtx->ulHdrType == HT_PTM &&
        (pDevCtx->ulFlags & CNI_HW_REMOVE_TRAILER) == 0)
    {
        trailerLen = (ETH_FCS_LEN + XTMRT_PTM_CRC_SIZE);
    }

    if ((pDevCtx->ulFlags & CNI_HW_REMOVE_HEADER) == 0)
        headerLen = HT_LEN(pDevCtx->ulHdrType);
    
    if (archer_xtm_hooks.deviceDetails)
        archer_xtm_hooks.deviceDetails(devId, encap, trafficType, (uint32_t)bufStatus, headerLen, trailerLen);


    if (archer_xtm_hooks.reInitDma)
    {
        archer_xtm_hooks.reInitDma();
    }

    if (archer_xtm_hooks.xtmLinkUp)
    {
       archer_xtm_hooks.xtmLinkUp(devId, matchId, pDevCtx->ucTxVcid);
    }

    return 1;
}

void bcmxtmrt_recycle_handler (pNBuff_t pNBuff, unsigned long context, uint32_t flag)
{
    /* copy enet_recycle_handler() */
    if (IS_FKBUFF_PTR(pNBuff))
    {
        /* Transmit driver is expected to perform cache invalidations */

        FkBuff_t* fkb = PNBUFF_2_FKBUFF(pNBuff);

        xtm_recycle_pdata(PFKBUFF_TO_PDATA(fkb, BCM_PKT_HEADROOM));
    }
    else /* skb */
    {
        struct sk_buff *skb = PNBUFF_2_SKBUFF(pNBuff);

        if (likely(flag & SKB_DATA_RECYCLE))
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
                        printk("invalid dirty_p detected: %p valid=[%p %p]\n",
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
            xtm_recycle_pdata((uint8_t *)(skb->head) + BCM_PKT_HEADROOM);
        }
    }

    return;
}

void bcmxtmrt_recycle_skb_or_data (struct sk_buff *skb, UINT32 context, UINT32 nFlag)
{
    if(nFlag & SKB_RECYCLE)
    {
        printk("\n\tERROR: bcmxtmrt_recycle_skb_or_data: SKB_RECYCLE flag is set\n\n");
    }

    bcmxtmrt_recycle_handler(skb, context, nFlag);
}

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
static inline int xtmrt_spdsvc_receive (PBCMXTMRT_DEV_CONTEXT pDevCtx, pNBuff_t pNBuff, UINT32 len)
{
    spdsvcHook_receive_t spdsvc_receive;
    int result = 0;
    UINT32 isAtmCell = 0;
    UINT32 hdrType = pDevCtx->ulHdrType;
    UINT32 rfc2684Type = RFC2684_NONE;

    if ((pDevCtx->Addr.ulTrafficType & TRAFFIC_TYPE_ATM_MASK) == TRAFFIC_TYPE_ATM)
    {
        if (IS_SKBUFF_PTR(pNBuff))
        {
            struct sk_buff *skb = PNBUFF_2_SKBUFF(pNBuff);
            if ((skb->protocol & htons(~FSTAT_CT_MASK)) == htons(SKB_PROTO_ATM_CELL))
            {
                isAtmCell = 1;
            }
        }
    }
    if (!isAtmCell)
    {
        if (!(pDevCtx->ulFlags & CNI_HW_ADD_HEADER) && HT_LEN(hdrType) && !isAtmCell)
        {
            rfc2684Type = HT_TYPE(hdrType);
        }

        spdsvc_receive.pNBuff = pNBuff;
        spdsvc_receive.header_type = SPDSVC_HEADER_TYPE_ETH;
        spdsvc_receive.phy_overhead = bcmxtmrt_xtmOverhead(hdrType, len, rfc2684Type, pDevCtx);

        result = spdsvc_receive_hook (&spdsvc_receive);
    }
    return result;
}

#else
#define xtmrt_spdsvc_receive(_pDevCtx, _pNBuff, _len)                        0
#endif

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
    PBCMXTMRT_DEV_CONTEXT pDevCtx;
    struct sk_buff *skb;
    UINT32 ulCell;
    UINT32 ulVcId;
    UINT32 ulMoreToReceive;
    UINT32 ulRxPktGood = 0;
    UINT32 ulRxPktProcessed = 0;
    UINT32 ulRxPktMax = ulBudget + (ulBudget / 2);
    FkBuff_t *pFkb = NULL;
    bcm_async_queue_t *queue_p;

    xtm_queue_rx_info_t *xtm_rxinfo;
    uint8_t *pData;
    int i, len, data_len;
    int desc_status;

    if (xtm_cpu_queues->rx_notify_pending_disable)
    {
        xtm_cpu_queues->rx_notify_enable = 0;
    }

    /* Assume that there is only 2 xtm cpu queues, for HI/LO priority */
    do
    {
        ulMoreToReceive = 0;

        if (ulBudget == 0)
        {
            *pulMoreToDo = 1;
            break;
        }
        xtm_rxinfo = NULL;

        spin_lock_bh(&pGi->xtmlock_rx);

        /* CPU_RX_HI = 0; CPU_RX_LO = 1 */
        for (i=CPU_RX_HI; i < XTM_NUM_RX_Q; i++)
        {
            queue_p = &xtm_cpu_queues->rxq[i];

            if(likely(bcm_async_queue_not_empty(queue_p)))
            {
                xtm_rxinfo = (xtm_queue_rx_info_t *)bcm_async_queue_entry_read (queue_p);
                pData = READ_ONCE (xtm_rxinfo->pData);
                data_len = READ_ONCE (xtm_rxinfo->length);
                desc_status = READ_ONCE(xtm_rxinfo->desc_status);

                bcm_async_queue_entry_dequeue (&xtm_cpu_queues->rxq[i]);

                if(likely(bcm_async_queue_not_empty(queue_p)))
                {
                    xtm_queue_rx_info_t *next_xtm_rxinfo =
                        (xtm_queue_rx_info_t *)bcm_async_queue_entry_read (queue_p);
                    uint8_t *next_pData = READ_ONCE (next_xtm_rxinfo->pData);

                    bcm_prefetch(next_pData);
                }

                XTM_CPU_STATS_UPDATE(xtm_cpu_queues->rxq[i].stats.reads);

                break;
            }
        }

        spin_unlock_bh(&pGi->xtmlock_rx);
        if (unlikely(xtm_rxinfo == NULL))
        {
            ulRxPktGood |= XTM_POLL_DONE;
            break;
        }
        /* there is data to process */
        pFkb = fkb_init ((uint8_t *)pData, BCM_PKT_HEADROOM, pData, data_len);
        pFkb->recycle_hook = (RecycleFuncP)bcmxtmrt_recycle_handler;
        pFkb->recycle_context = 0;

        ulRxPktProcessed++;
        ulVcId = ((desc_status>>FSTAT_MATCH_ID_SHIFT) & FSTAT_MATCH_ID_MASK);
        pDevCtx = pGi->pDevCtxsByMatchId[ulVcId];

        ulCell = (desc_status & FSTAT_PACKET_CELL_MASK) == FSTAT_CELL;

        /* error status, or packet with no pDev */
        if (((desc_status & FSTAT_ERROR) != 0) ||
            ((desc_status & (DMA_SOP|DMA_EOP)) != (DMA_SOP|DMA_EOP)) ||
            ((!ulCell) && (pDevCtx == NULL)))   /* packet */
        {
            if (ulVcId == TEQ_DATA_VCID && pGi->pTeqNetDev)
            {
                unsigned long flags;

                skb = skb_header_alloc();
                if (unlikely (!skb))
                {
                    xtm_recycle_pdata (pData);
                    printk("something went wrong, free buffer (SKB allocation failure\n");
                    goto drop_pkt;
                }
#ifdef XTM_CACHE_SMARTFLUSH
                len = data_len + SAR_DMA_MAX_BURST_LENGTH;
#else
                len = BCM_MAX_PKT_LEN;
#endif
                skb_headerinit(BCM_PKT_HEADROOM, len, skb, pData,
                  (RecycleFuncP)bcmxtmrt_recycle_skb_or_data, 0, NULL);
                __skb_trim(skb, data_len);

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
                /* free the data buffer */
                xtm_recycle_pdata (pData);
                if (pDevCtx)
                    pDevCtx->DevStats.rx_errors++;
            }
        }
        else if (!ulCell) /* process packet, pDev != NULL */
        {
            if (likely (!xtmrt_spdsvc_receive (pDevCtx, FKBUFF_2_PNBUFF((pFkb)), data_len)))
            {
                /* pBuf is just a pointer to the rx iudma buffer. */
                bcmxtmrt_processRxPkt(pDevCtx, NULL, pData,
                                      desc_status, data_len, 0);
            
                ulRxPktGood++;
                ulBudget--;
            }
            else
            {
                // if it is a speed service packet, skip budget checking
                ulRxPktProcessed = 0;
            }
        }
        else                /* process cell */
        {
            bcmxtmrt_processRxCell(pData);
            xtm_recycle_pdata (pData);
        }
drop_pkt:
        if (ulRxPktProcessed >= ulRxPktMax)
            break;
        else
            ulMoreToReceive = 1; /* more packets to receive on Rx queue? */

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
int bcmxapi_add_proc_files(void)
{

   //struct proc_dir_entry *entry;

   proc_mkdir("driver/xtm", NULL);
   /* worry about proc entries later */

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
int bcmxapi_del_proc_files(void)
{
   remove_proc_entry("driver/xtm", NULL);
   return 0;
    
}  /* bcmxapi_del_proc_files() */



/*---------------------------------------------------------------------------
 * Function Name: bcmxtmrt_timer
 * Description:
 *    Periodic timer that calls the send function to free packets
 *    that have been transmitted.
 * Returns: void
 *---------------------------------------------------------------------------
 */
static void bcmxtmrt_timer(PBCMXTMRT_GLOBAL_INFO pGi)
{
   if (pGi->pTeqNetDev && (!pGi->ulDevCtxMask))
   {
      UINT32 ulNotUsed;
      bcmxapi_rxtask(100, &ulNotUsed);

      /* Restart the timer. */
      pGi->Timer.expires = jiffies + SAR_TIMEOUT;
      add_timer(&pGi->Timer);
   }

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
   
    if (pGi->ulDrvState != XTMRT_UNINITIALIZED)
        return -EPERM;

    if (archer_xtm_hooks.reInitDma)
    {
        archer_xtm_hooks.reInitDma();
    }

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
   
    pGi->bondConfig.uConfig = pGip->bondConfig.uConfig;
    if ((pGi->bondConfig.sConfig.ptmBond == BC_PTM_BONDING_ENABLE) ||
        (pGi->bondConfig.sConfig.atmBond == BC_ATM_BONDING_ENABLE))
        //printk(CARDNAME ": PTM/ATM Bonding Mode configured in system\n");
        printk(CARDNAME ": PTM/ATM Bonding Mode configured (not supported!!!)\n");
    else
        printk(CARDNAME ": PTM/ATM Non-Bonding Mode configured in system\n");

    pGi->atmBondSidMode = ATMBOND_ASM_MESSAGE_TYPE_NOSID;

    /* Initialize a timer function for TEQ */
    init_timer(&pGi->Timer);
    pGi->Timer.data     = (unsigned long)pGi;
    pGi->Timer.function = (void *)bcmxtmrt_timer;

    printk("E-RXIntr\n");
    bcmxapi_enable_rx_interrupt();    
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
    PBCMXTMRT_GLOBAL_INFO pGi = &g_GlobalInfo;

    if (pGi->ulDrvState == XTMRT_UNINITIALIZED)
        return -EPERM;
      
    del_timer_sync(&pGi->Timer);
    printk("D-RXIntr\n");
    bcmxapi_disable_rx_interrupt();
      
    pGi->ulDrvState = XTMRT_UNINITIALIZED;
      
   return 0;

}  /* bcmxapi_DoGlobUninitReq() */

/*
 *-----------------------------------------------------------------------------
 * function   : xtm_bpm_txq_thresh
 * description: configures the queue thresholds
 * Note       : copied from xtmrt_bpm.c
 *-----------------------------------------------------------------------------
 */
static int xtm_bpm_txq_thresh( PBCMXTMRT_DEV_CONTEXT pDevCtx,
                               PXTMRT_TRANSMIT_QUEUE_ID pTxQId)
{
    UINT32 usSpeed;

    usSpeed = (pDevCtx->MibInfo.ulIfSpeed >> 20) + 1;   /* US in Mbps */

    pTxQId->ulLoThresh = XTM_BPM_TXQ_LO_THRESH(usSpeed);
    pTxQId->ulHiThresh = XTM_BPM_TXQ_HI_THRESH(usSpeed);

    pTxQId->ulDropped = 0;

    BCM_XTM_DEBUG("XTM Tx qId[%d] ulIfSpeed=%d, usSpeed=%d\n",
        pTxQId->ulQueueIndex, (int) pDevCtx->MibInfo.ulIfSpeed, (int) usSpeed);

    BCM_XTM_DEBUG("XTM Tx qId[%d] ulLoThresh=%d, ulHiThresh=%d\n",
        pTxQId->ulQueueIndex, (int) pTxQId->ulLoThresh, (int) pTxQId->ulHiThresh);

    pTxQId->usQueueSize = XTM_CPU_TX_QUEUE_SIZE;

    return GBPM_SUCCESS;
}


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

    BCM_XTM_DEBUG("DoSetTxQueue\n");

    printk("SetTxQueue for InfoSize %d QueueIndex %d\n", pDevCtx->ulTxQInfosSize,
            pTxQId->ulQueueIndex);


    local_bh_enable();  /* needed to avoid kernel error */

    xtm_bpm_txq_thresh(pDevCtx, pTxQId);

    txdma = (BcmPktDma_XtmTxDma *)kzalloc(sizeof(BcmPktDma_XtmTxDma), GFP_ATOMIC);

    local_bh_disable();

    if (txdma == NULL)
    {
        printk("Unable to allocate memory for tx dma info\n");
        return -ENOMEM;
    }
    /* Increment channels per dev context */
    pDevCtx->txdma[pDevCtx->ulTxQInfosSize++] = txdma;

    txdma->ulDmaIndex = pTxQId->ulQueueIndex;

    /* Set every transmit queue size to the number of external buffers.
     * The QueuePacket function will control how many packets are queued.
     */
    ulQueueSize = pTxQId->usQueueSize; // pGi->ulNumExtBufs;

    ulPort = PORTID_TO_PORT(pTxQId->ulPortId);

    if ((ulPort < MAX_PHY_PORTS) && (pTxQId->ucSubPriority < MAX_SUB_PRIORITIES))
    { 
        UINT32 ulPtmPrioIdx = PTM_FLOW_PRI_LOW;
        //volatile DmaRegs *pDmaCtrl = TXDMACTRL(pDevCtx);
        //volatile DmaStateRam *pStRam = pDmaCtrl->stram.s;
        UINT32 i, ulTxQs;

        txdma->ulPort        = ulPort;
        txdma->ulPtmPriority = pTxQId->ulPtmPriority;
        txdma->ulSubPriority = pTxQId->ucSubPriority;
        //txdma->ulAlg         = (UINT16)pTxQId->ucWeightAlg;
        //txdma->ulWeightValue = (UINT16)pTxQId->ulWeightValue;
        txdma->ulQueueSize   = ulQueueSize;
        txdma->txEnabled     = 1;
        txdma->ulNumTxBufsQdOne = 0;

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

    printk("TXDMA drop alg %d Lo minThresh %d maxThresh %d hi minThresh %d maxThresh %d\n", 
            pTxQId->ucDropAlg, 
            pTxQId->ucLoMinThresh, pTxQId->ucLoMaxThresh,
            pTxQId->ucHiMinThresh, pTxQId->ucHiMaxThresh );

    if (archer_xtm_hooks.setTxChanDropAlg)
    {
        archer_drop_config_t config;
        int txQSize = archer_xtm_hooks.txdmaGetQSize();

        config.algorithm = pTxQId->ucDropAlg;
        config.profile[ARCHER_DROP_PROFILE_LOW].minThres = (pTxQId->ucLoMinThresh * txQSize) / 100;
        config.profile[ARCHER_DROP_PROFILE_LOW].maxThres = (pTxQId->ucLoMaxThresh * txQSize) / 100;
        config.profile[ARCHER_DROP_PROFILE_HIGH].minThres = (pTxQId->ucHiMinThresh * txQSize) / 100;
        config.profile[ARCHER_DROP_PROFILE_HIGH].maxThres = (pTxQId->ucHiMaxThresh * txQSize) / 100;

        // the follow setting is configured through userspace application
        config.profile[ARCHER_DROP_PROFILE_LOW].dropProb = 100;
        config.profile[ARCHER_DROP_PROFILE_HIGH].dropProb = 100;
        config.priorityMask_0 = 0;
        config.priorityMask_1 = 0;

        archer_xtm_hooks.setTxChanDropAlg (txdma->ulDmaIndex, &config);
    }

    /* notify Archer TDMA should be enabled */
    if (archer_xtm_hooks.txdmaEnable)
    {
        archer_xtm_hooks.txdmaEnable(txdma->ulDmaIndex,pDevCtx->ucTxVcid);
    }
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
    txdma->txEnabled = 0;

    /* notify Archer TDMA should be disabled */
    if (archer_xtm_hooks.txdmaDisable)
    {
        archer_xtm_hooks.txdmaDisable(txdma->ulDmaIndex);
    }

    /* update total number of tx queues used by the system */
    g_GlobalInfo.ulNumTxQs--;

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
    printk("flush down TX queue, supposed to be call for bonding traffic\n");

    txdma->txEnabled = 0;

    /* notify Archer TDMA should be disabled */
    if (archer_xtm_hooks.txdmaDisable)
    {
        archer_xtm_hooks.txdmaDisable(txdma->ulDmaIndex);
    }
   
    return;
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
void bcmxapi_XtmGetStats(UINT8 vport, UINT32 *rxDropped, UINT32 *txDropped)
{
    //FIXME: This Will be fixed in the future releases as this logic needs to
    //be added to map the netdevice to corresponding queues.
    *rxDropped = 0;
    *txDropped = 0;

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
    if (vport < XTM_NUM_RX_Q)
    {
        xtm_cpu_queues->rxq[vport].stats.discards = 0;
    }
    else
    {
        printk("bcmxapi_XtmResetStats : vport (%d) exceeds number of RX queues %d\n", vport, XTM_NUM_RX_Q);
    }
    if (vport == 0)
    {
        xtm_cpu_queues->txq.stats.discards = 0;
    }   
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
int bcmxapi_queue_packet(PTXQINFO pTqi, UINT32 isAtmCell)
{
    /* should check if transmit queue is full instead? */
    return (bcm_async_queue_not_full (&xtm_cpu_queues->txq));
}


/*---------------------------------------------------------------------------
 * int bcmxapi_xmit_packet(pNBuff_t *ppNBuf, UINT8 **ppData, UINT32 *pLen,
 *                         BcmPktDma_XtmTxDma *txdma, UINT32 txdmaIdx,
 *                         UINT16 bufStatus, UINT32 skbMark)
 * Description:
 *    Enqueue the packet to the tx queue specified by txdma for transmission.
 *    Function to suit in runner based architecture.
 * Returns:
 *    0 if successful or error status
 *---------------------------------------------------------------------------
 */
int bcmxapi_xmit_packet(pNBuff_t *ppNBuf, UINT8 **ppData, UINT32 *pLen,
                        BcmPktDma_XtmTxDma *txdma, UINT32 txdmaIdx,
                        UINT16 bufStatus, UINT32 skbMark, int is_spdsvc_setup_packet,
                        void *pTcpSpdTstInfo)
{
    int rc = -1;
    UINT32 ctType;

    ctType  = ((UINT32)bufStatus & FSTAT_CT_MASK) >> FSTAT_CT_SHIFT;

    spin_lock_bh  (&xtm_cpu_queues->tx_lock);
    if (bcm_async_queue_not_full (&xtm_cpu_queues->txq))
    {
        xtm_queue_tx_info_t *tx_info = (xtm_queue_tx_info_t *)
            bcm_async_queue_entry_write (&xtm_cpu_queues->txq);

        WRITE_ONCE(tx_info->pNBuff, *ppNBuf);
        // Egress queue contains both the TC and the tx DMA queue
        WRITE_ONCE(tx_info->egress_queue, txdmaIdx);
        WRITE_ONCE(tx_info->desc_status, bufStatus);

        bcm_async_queue_entry_enqueue (&xtm_cpu_queues->txq);

        XTM_CPU_STATS_UPDATE (xtm_cpu_queues->txq.stats.writes);
        rc = 0;
    }
    else
    {
        printk("xtm : failed sending packet due to queue full\n");
        XTM_CPU_STATS_UPDATE(xtm_cpu_queues->txq.stats.discards);
    }

    spin_unlock_bh (&xtm_cpu_queues->tx_lock);

    xtm_cpu_queues->tx_notifier();

    return rc;
   
}  /* bcmxapi_xmit_packet() */


int archer_xtm_tx_queue_notifier_register (int q_id, TX_NOTIFIER tx_notifier)
{
    xtm_cpu_queues->tx_notifier = tx_notifier;
    return 0;
}

int archer_xtm_tx_queue_read (int q_id, pNBuff_t *ppNBuff, int *desc_status_p, int *egress_queue_p)
{
    bcm_async_queue_t *queue_p = &xtm_cpu_queues->txq;

    if (bcm_async_queue_not_empty (queue_p))
    {
        xtm_queue_tx_info_t *tx_info_p = (xtm_queue_tx_info_t *)
            bcm_async_queue_entry_read (queue_p);

        *ppNBuff = READ_ONCE(tx_info_p->pNBuff);
        *egress_queue_p = READ_ONCE(tx_info_p->egress_queue);
        *desc_status_p = READ_ONCE(tx_info_p->desc_status);

        bcm_async_queue_entry_dequeue (queue_p);

        XTM_CPU_STATS_UPDATE(queue_p->stats.reads);

        return 0;
    }

    return -1;
}

int archer_xtm_tx_queue_not_empty (int q_id)
{
    return bcm_async_queue_not_empty (&xtm_cpu_queues->txq);
}

int archer_xtm_rx_queue_write (int q_id, uint8_t **pData, int data_len, int ingress_port, int desc_status)
{
    int rc = 0;
    /* assume this is received from queue 0 for now */
    /* and ingress_port is descriptor status */
    if (bcm_async_queue_not_full (&xtm_cpu_queues->rxq[q_id]))
    {
        xtm_queue_rx_info_t *rx_info = (xtm_queue_rx_info_t *)
            bcm_async_queue_entry_write (&xtm_cpu_queues->rxq[q_id]);

        WRITE_ONCE(rx_info->pData, *pData);
        WRITE_ONCE(rx_info->length, data_len);
        WRITE_ONCE(rx_info->ingress_port, ingress_port);
        WRITE_ONCE(rx_info->desc_status, desc_status);

        bcm_async_queue_entry_enqueue (&xtm_cpu_queues->rxq[q_id]);

        XTM_CPU_STATS_UPDATE(xtm_cpu_queues->rxq[q_id].stats.writes);
    }
    else
    {
        XTM_CPU_STATS_UPDATE(xtm_cpu_queues->rxq[q_id].stats.discards);
        rc = -1;
    }
    if (xtm_cpu_queues->rx_notify_enable)
    {
        BCMXTMRT_WAKEUP_RXWORKER(&g_GlobalInfo);
    }
    else if (xtm_cpu_queues->rx_notify_pending_disable)
    {
        xtm_cpu_queues->rx_notify_pending_disable = 0;
    }
    return rc;
}

#define XTM_CPU_QUEUE_WAKEUP_RECYCLE_WORKER(x) do {                    \
        if ((x)->recycle_work_avail == 0) {                            \
            set_bit(0, &(x)->recycle_work_avail);                      \
            wake_up_interruptible(&((x)->recycle_thread_wqh)); }} while (0)

void archer_xtm_recycle_queue_write(pNBuff_t pNBuff)
{
    if (likely (bcm_async_queue_not_full (&xtm_cpu_queues->recycleq)))
    {
        xtm_queue_recycle_info_t *recycle_info = (xtm_queue_recycle_info_t *)
            bcm_async_queue_entry_write (&xtm_cpu_queues->recycleq);
        WRITE_ONCE(recycle_info->pNBuff, pNBuff);
        bcm_async_queue_entry_enqueue (&xtm_cpu_queues->recycleq);

        XTM_CPU_STATS_UPDATE(xtm_cpu_queues->recycleq.stats.writes);
    }
    else
    {
#if defined(CONFIG_BCM_GLB_COHERENCY)
        nbuff_free(pNBuff);
#else
        nbuff_flushfree(pNBuff);
#endif
        XTM_CPU_STATS_UPDATE(xtm_cpu_queues->recycleq.stats.discards);
    }
    XTM_CPU_QUEUE_WAKEUP_RECYCLE_WORKER(xtm_cpu_queues);
}

static inline void xtm_cpu_queue_recycle_buffers (bcm_async_queue_t *recycleqp)
{
    int budget = XTM_CPU_RECYCLE_BUDGET;

    while (likely(budget-- && bcm_async_queue_not_empty(recycleqp)))
    {
        xtm_queue_recycle_info_t *recycle_p = (xtm_queue_recycle_info_t *)
            bcm_async_queue_entry_read (recycleqp);

        pNBuff_t pNBuff = READ_ONCE (recycle_p->pNBuff);
#if defined(CONFIG_BCM_GLB_COHERENCY)
        nbuff_free_ex(pNBuff, 0);
#else
        nbuff_flushfree_ex(pNBuff, 0);
#endif
        bcm_async_queue_entry_dequeue (recycleqp);

        XTM_CPU_STATS_UPDATE(recycleqp->stats.reads);
    }
}

static int xtm_recycle_thread (void *arg)
{
    xtm_queues_t * cpu_queues = (xtm_queues_t *)arg;

    while (1)
    {
        wait_event_interruptible (cpu_queues->recycle_thread_wqh,
                                  cpu_queues->recycle_work_avail);
        if (kthread_should_stop())
        {
            printk(KERN_INFO "killing bcmxtm_recycle\n");
            break;
        }

        xtm_cpu_queue_recycle_buffers (&cpu_queues->recycleq);

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

#if defined(CC_XTM_CPU_QUEUE_STATS)
static void xtm_cpu_queue_dump(bcm_async_queue_t *queue_p, char *name, int index)
{
    printk("%s[%d]: Depth %d, Level %d, Writes %d, Reads %d, Discards %d, Writes+Discards %d\n", name, index,
           queue_p->depth, queue_p->depth - bcm_async_queue_free_entries(queue_p),
           queue_p->stats.writes, queue_p->stats.reads, queue_p->stats.discards,
           queue_p->stats.writes + queue_p->stats.discards);
}
#else
static void xtm_cpu_queue_dump(bcm_async_queue_t *queue_p, char *name, int index)
{
    printk("%s[%d]: Depth %d, Level %d, Write %d, Read %d\n", name, index,
           queue_p->depth, queue_p->depth - bcm_async_queue_free_entries(queue_p),
           queue_p->write, queue_p->read);
}
#endif

void archer_xtm_cpu_queue_stats(void)
{
    int i;

    for (i=0; i < 2; i++)
    {
        xtm_cpu_queue_dump(&xtm_cpu_queues->rxq[i], "XTM RXQ", i);
    }

    xtm_cpu_queue_dump(&xtm_cpu_queues->txq, "XTM TXQ", 0);

    xtm_cpu_queue_dump(&xtm_cpu_queues->recycleq, "XTM RCQ", 0);

    printk("Archer XTM rx notify 0x%x (pending %d)\n",
           xtm_cpu_queues->rx_notify_enable, xtm_cpu_queues->rx_notify_pending_disable);
}


static int archer_xtm_host_bind(void)
{
    bcmFun_t *archer_driver_host_bind = bcmFun_get(BCM_FUN_ID_ARCHER_HOST_BIND);
    archer_host_hooks_t hooks;
    int rc;

    if(!archer_driver_host_bind)
    {
        printk(KERN_ERR "XTMRT: Archer binding is not available\n");

        return -1;
    }

    hooks.host_type = ARCHER_HOST_TYPE_XTMRT;
    hooks.tx_queue_notifier_register = archer_xtm_tx_queue_notifier_register;
    hooks.tx_queue_read = archer_xtm_tx_queue_read;
    hooks.tx_queue_not_empty = archer_xtm_tx_queue_not_empty;
    hooks.rx_queue_write = archer_xtm_rx_queue_write;
    hooks.recycle_queue_write = archer_xtm_recycle_queue_write;
    hooks.queue_stats = archer_xtm_cpu_queue_stats;

    rc = archer_driver_host_bind(&hooks);
    if(rc)
    {
        printk(KERN_ERR "XTMRT: Could not bind to Archer\n");
    }

    return rc;
}
