#ifndef __FC_WFD_INLINE_H_INCLUDED__
#define __FC_WFD_INLINE_H_INCLUDED__

/*
<:copyright-BRCM:2017:DUAL/GPL:standard 

   Copyright (c) 2017 Broadcom 
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
=========   Flow Cache based Wifi Forwarding Driver     =========

Flow cache base WFD add a hook function (fcache_wfd_enqueue) before
flow cache call standard HardXmit funtion to steal packets that have
wfd bit set in blog (blog_p->wfd.nic_ucast.is_wfd). And chain the
packet in WFD.

The queue in FC based WFD use NBuff, and then translate to SKB or FKB
depend on transmit dev type (DHD/NIC).It support 2 queue with
high/low priority and classify depend on blog per wifi adapter.

FC based WFD with Defer mode (FC_WFD_LAZY_DEFER) :
    System will get better performance if packet chain longer
    (better bulk packet process , less function call and better
    AMPDU aggreation). Packet recv work in NAPI mode ,system will recv
    packet from RXDMA in a shot with NET_WEIGHT (32) packets.
    But packet chain can support up to 128 packets in a chain.

    For maxmize per packet number in chain,we may need wait some period
    of time if chain doesn't contain enough packets. And have a monitor
    (WFD_NBuff_Defer_Monitor) to punch packet in queue if data rate to
    low that can't fill chain to enough length when packet stay in queue
    too long.

    Only apply on low priority queue.

*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/nbuff.h>
#include <linux/gbpm.h>
#include <linux/bcm_realtime.h>
#include <linux/blog.h>
#include "wl_pktc.h"
#include "bcm_prefetch.h"

#define FC_WFD //indicate wfd_dev.c now we working on fcache based wfd.

//#define FC_WFD_DEBUG
#undef FC_WFD_DEBUG
#define FC_WFD_LAZY_DEFER

#define FC_WFD_MAX_QUEUE_LEN (1024)
#define WFD_WLAN_QUEUE_MAX_SIZE (FC_WFD_MAX_QUEUE_LEN)

typedef struct
{
  uint16 head;
  uint16 tail;
  uint16 max_size;
  uint8 wfd_idx;
  uint8 qidx;
#if defined(FC_WFD_LAZY_DEFER)
  uint8 defer_mode; // 0: Not Allow Defer , 1 Allow Defer
  uint8 defer_cnt;
#endif /* FC_WFD_LAZY_DEFER  */
  uint32_t wlmetalist[FC_WFD_MAX_QUEUE_LEN];
  pNBuff_t buflist[FC_WFD_MAX_QUEUE_LEN];
#if defined(FC_WFD_DEBUG)
  unsigned long defer_begin_jiffies;
#endif /* FC_WFD_DEBUG */
}____cacheline_aligned WFD_NBuff_Queue,*pWFD_NBuff_Queue;

#define WFD_NBuff_Queue_Size(queue) (((queue->tail) >= (queue->head)) ? ((queue->tail)-(queue->head)) : ((queue->tail)+(queue->max_size))-(queue->head))
//#define WFD_NBuff_Queue_Ptr_Next(queue,ptr) ((ptr+1) % (queue->max_size))
#define WFD_NBuff_Queue_Ptr_Next(queue,ptr) ((ptr+1)>=(queue->max_size) ? (ptr+1-queue->max_size) : (ptr+1))
#define WFD_NBuff_Queue_Empty(queue) ((queue->head)==(queue->tail))
//#define WFD_NBuff_Queue_Full(queue) ((queue->head)==WFD_NBuff_Queue_Ptr_Next(queue->tail))
#define WFD_NBuff_Queue_Full(queue) ((WFD_NBuff_Queue_Size(queue)+1) >= queue->max_size)

#define wfd_get_qid(qidx) (qidx % WFD_NUM_QUEUES_PER_WFD_INST)
#define wfd_acc_info_get(radio_index) (NULL)
//#define wfd_queue_not_empty(radio_index,qid,qidx) (WFD_NBuff_Queue_Size((&fc_wfd_queue[qidx])) != 0)
#define wfd_queue_not_empty(radio_index,qid,qidx) (!WFD_NBuff_Queue_Empty((&fc_wfd_queue[qidx])))

#define wfd_int_enable(radio_index,qid,qidx)  //No Hardware interrupt.. do nothing
#define wfd_int_disable(radio_index,qid,qidx) //No Hardware interrupt.. do nothing
#define wfd_get_minQIdx(wfd_idx)  ((wfd_idx)*WFD_NUM_QUEUES_PER_WFD_INST)
#define wfd_get_maxQIdx(wfd_idx)  (wfd_get_minQIdx(wfd_idx) + WFD_NUM_QUEUES_PER_WFD_INST - 1 )
#define release_wfd_interfaces() 

#define Get_wfdidx_blog(blog_p) (((blog_p->wfd.nic_ucast.wfd_idx)<WFD_MAX_OBJECTS) ? (blog_p->wfd.nic_ucast.wfd_idx):(-1)) // DHD/NIC overlap on wfd_idx in blogs
//#define Get_wfd_prio(blog_p) (blog_p->wfd.nic_ucast.wfd_prio) //blog_p->wfd.dhd_ucast.wfd_prio // DHD/NIC non-overlap on wfd_prio in blogs
#define Get_wfd_prio(blog_p) (blog_p->iq_prio) //use iq_prio directly instread use wfd_prio ...


static WFD_NBuff_Queue fc_wfd_queue[WFD_NUM_QUEUE_SUPPORTED];

/* Flow cache wifi forwarding driver Support */
typedef int (*FC_WFD_ENQUEUE_HOOK)(pNBuff_t pNBuff, const Blog_t * const blog_p);
FC_WFD_ENQUEUE_HOOK fc_wfd_enqueue_cb = NULL;
EXPORT_SYMBOL(fc_wfd_enqueue_cb);

#if defined(FC_WFD_LAZY_DEFER)
//Stop monitor defer queue when qlen over the value
#define FC_WFD_DEFER_QUEUE_TARGET_LEN (120)

//Monitor defer queue when qlen over the value, for reduce latency when traffic is low
//#define FC_WFD_DEFER_QUEUE_MIN_LEN (0) 
#define FC_WFD_DEFER_QUEUE_MIN_LEN (4)

/* Max defer time : ((FC_WFD_MAX_DEFER_CNT+1) * FC_WFD_DEFER_CHECK_INTERVAL) = 8ms */
#define FC_WFD_DEFER_CHECK_INTERVAL (HZ >> 9) // 1000/512 about 2ms ...
#define FC_WFD_MAX_DEFER_CNT (3)
//#define FC_WFD_MAX_DEFER_CNT (64)

static unsigned int FC_WFD_Defer_qidx_mask=0x0;
wait_queue_head_t fc_wfd_defer_thread_wqh;
struct task_struct *fc_wfd_defer_thread = NULL;

#define FC_WFD_DEFER_ALLOW (1)
#define FC_WFD_DEFER_DENY  (0)

#define WFD_NBuff_Defer_Allow(queue) ((queue->defer_mode) == FC_WFD_DEFER_ALLOW)
#define WFD_NBuff_Defer_Queue(qidx) (FC_WFD_Defer_qidx_mask & (1<<(qidx)))
#define WFD_NBuff_Defer_Queue_Set(qidx) (FC_WFD_Defer_qidx_mask |= (1<<(qidx)))
#define WFD_NBuff_Defer_Queue_Unset(qidx) (FC_WFD_Defer_qidx_mask &= ~(1<<(qidx)))
#define WFD_NBuff_Defer_Monitor_WakeUp() do { \
            wake_up_interruptible(&fc_wfd_defer_thread_wqh); \
          } while(0)
#endif

#if defined(BCM_PKTFWD)
#define wfd_bulk_fkb_get wfd_bulk_fkb_get_pktfwd
#define wfd_bulk_skb_get wfd_bulk_skb_get_pktfwd
#else /* PKTC */
#define wfd_bulk_fkb_get wfd_bulk_fkb_get_pktc
#define wfd_bulk_skb_get wfd_bulk_skb_get_pktc
#endif

static inline int WFD_NBuff_EnQueue_meta(pWFD_NBuff_Queue queue,pNBuff_t *pkt,uint32_t wlmeta)
{
    int tail_next = -1;
    tail_next = WFD_NBuff_Queue_Ptr_Next(queue,queue->tail);

    if((tail_next) != (queue->head) )
    {
        queue->buflist[queue->tail]=pkt;
        queue->wlmetalist[queue->tail]=wlmeta;

        queue->tail = tail_next;
        return 1 ;
    }
    return 0 ;
}

static inline pNBuff_t WFD_NBuff_DeQueue(pWFD_NBuff_Queue queue)
{
    if(!WFD_NBuff_Queue_Empty(queue))
    {
        pNBuff_t pkt;
        pkt = queue->buflist[queue->head];
#ifdef FC_WFD_DEBUG
        queue->buflist[queue->head] = (pNBuff_t)(0xDEADBEEF);
#endif
        queue->head = WFD_NBuff_Queue_Ptr_Next(queue,queue->head);
        return pkt ;
    }
    return NULL ;
}

#define WFD_NBuff_DeQueue_meta(queue,pkt,wlmeta) \
do{ \
    wlmeta = (BlogWfd_t) queue->wlmetalist[queue->head];\
    pkt = (pNBuff_t) WFD_NBuff_DeQueue(queue);\
}while(0);

static inline int WFD_NBuff_Queue_init(pWFD_NBuff_Queue queue)
{
    queue->head     = 0;
    queue->tail     = 0;
#if defined(FC_WFD_LAZY_DEFER)
    queue->defer_cnt= 0;
#endif
    queue->wfd_idx  = 0;
    queue->max_size = 0;  //Set to 0 to prevent enqueue until set max_size
    memset(queue->buflist,0x00, sizeof(pNBuff_t) * (FC_WFD_MAX_QUEUE_LEN));
    return 1;
}

static inline int WFD_NBuff_Queue_Deinit(pWFD_NBuff_Queue queue)
{
    pNBuff_t pNBuff = NULL;

    while((pNBuff = WFD_NBuff_DeQueue(queue)) != NULL)
    {
        switch((unsigned long) pNBuff)
        {
            case (unsigned long)NULL:
#ifdef FC_WFD_DEBUG
            case (unsigned long)0xDEADBEFF:
                //Shouldn't be happened
                if(net_ratelimit()) 
                {
                    printk("#### %s:%d #### qidx:%d Abnormal packet pNBuff:%p\n"
                        ,__FUNCTION__,__LINE__,queue->qidx,pNBuff);
                }
#endif
                break;
            default:
                nbuff_free((pNBuff_t) pNBuff); //Drop packet ...
                break;
        }
    }

    WFD_NBuff_Queue_init(queue);

    return 1;
}


#if defined(FC_WFD_LAZY_DEFER)
static int WFD_NBuff_Defer_Monitor(void *context)
{
    int qidx = 0;
    wfd_object_t * wfd_p;
    uint32_t qmask = 0,last_qmask=0;
    pWFD_NBuff_Queue queue  = NULL ;
    long timeout = 0;
    long ret;
    while (1)
    {
        if(FC_WFD_Defer_qidx_mask) //wait event with timeout..
        {
           if(timeout <= 0)  
               timeout = FC_WFD_DEFER_CHECK_INTERVAL;

           ret = wait_event_interruptible_timeout( fc_wfd_defer_thread_wqh
                                                 ,(FC_WFD_Defer_qidx_mask^last_qmask) || kthread_should_stop()
                                                 , timeout);

           switch(ret)
           {
               case 0:
               case 1:
               case (-ERESTARTSYS):
                   //Really timeout ..
                   timeout = 0;
               break;
               default:
                   timeout = ret; //remain timeout
               break;
           }
        }
        else //nothing to monitor ...just wait event ..
        {
            timeout = -ERESTARTSYS;
            wait_event_interruptible(fc_wfd_defer_thread_wqh
                                    ,FC_WFD_Defer_qidx_mask ||  kthread_should_stop()
                                    );
        }

        if (kthread_should_stop())
        {
            printk(KERN_INFO "kthread_should_stop detected in wfd\n");
            break;
        }

//#ifdef FC_WFD_DEBUG
#if 0
        if(timeout == 0 )
        {
            static unsigned long last_jiffies=0,current_jiffies=0;
            current_jiffies = jiffies;

            if(net_ratelimit()) 
            {
                printk("#### %s:%d #### interval:%dms\n",__FUNCTION__,__LINE__,(jiffies_to_msecs(current_jiffies-last_jiffies)));
            }
            last_jiffies=current_jiffies;
        }
#endif

        qmask = FC_WFD_Defer_qidx_mask;
        last_qmask = qmask; 
        qidx  = 0;

        while(qmask !=0)
        {
            if(qmask & (1<<qidx))
            {
               qmask &= ~(1<<qidx) ; //umask queue we checked
               queue=(&fc_wfd_queue[qidx]);
               wfd_p = &wfd_objects[queue->wfd_idx];
               if((queue->defer_cnt >= FC_WFD_MAX_DEFER_CNT) || (WFD_NBuff_Queue_Size(queue) >= FC_WFD_DEFER_QUEUE_TARGET_LEN))
               {
#ifdef FC_WFD_DEBUG
                   if(queue->defer_cnt >= FC_WFD_MAX_DEFER_CNT)
                   if(net_ratelimit()) 
                   {
                       int diff_ms = jiffies_to_msecs(jiffies-queue->defer_begin_jiffies);
                       printk("#### %s:%d #### timeout. qidx:%d defer_cnt:%d ql:%d defer:%dms\n",__FUNCTION__,__LINE__
                        ,qidx
                        ,queue->defer_cnt
                        ,WFD_NBuff_Queue_Size(queue)
                        ,diff_ms
                        );
                   }
#endif /* FC_WFD_DEBUG */

                    queue->defer_cnt = 0;
                    WFD_NBuff_Defer_Queue_Unset(qidx);
                    wfd_p->wfd_rx_work_avail |= (1<<qidx);
                    WFD_WAKEUP_RXWORKER(wfd_p->wfd_idx);
               }else
               {
                   wfd_p->wfd_rx_work_avail &= ~(1<<qidx);//Lie for wfd to avoid check the queue ... 
                   if(timeout == 0)
                        queue->defer_cnt++;
#if 0
                  if(net_ratelimit()) 
                  {
                      printk("#### %s:%d #### defer_cnt:%d qlen:%d timeout:0x%016lX\n",__FUNCTION__,__LINE__,queue->defer_cnt,WFD_NBuff_Queue_Size(queue),timeout);
                  }
#endif
               }
            }
            qidx++;
        }
    }

    return 0;
}
#endif


//static int fcache_wfd_enqueue(pNBuff_t pNBuff, struct net_device *dev)
static int fcache_wfd_enqueue(pNBuff_t pNBuff, const Blog_t * const blog_p)
{
    int wfdIdx;
    int qid ;
    wfd_object_t * wfd_p;
    pWFD_NBuff_Queue queue;

    /* is_wfd offset is overlap on dhd_ucast/nic_ucast/mcast 
       not support multicast yet ...
    */
    if(blog_p && (!(blog_p->wfd.nic_ucast.is_wfd) || (blog_p->rx.multicast)))
    {
        return 0; // not take pkt, keep fc goning on 
    }

    wfdIdx = Get_wfdidx_blog(blog_p);

    if(wfdIdx<0 || !(wfd_objects[wfdIdx].isValid)) //Unknow wfdIdx
    {
        return 0; // not take pkt, keep fc goning on 
    }

    /* Map wfd_prio (iq_prio) to queue id */
    qid = (Get_wfd_prio(blog_p) == BLOG_IQ_PRIO_HIGH) ? (WFD_NUM_QUEUES_PER_WFD_INST-1) : 0 ;

    wfd_p = &wfd_objects[wfdIdx];
    queue=(&fc_wfd_queue[wfd_get_minQIdx(wfd_p->wfd_idx)+ qid ]) ;

    /* enqueue packet to fc_wfd_queue */
    /* backup wl metadata from blog to queue */
    if(WFD_NBuff_EnQueue_meta(queue,pNBuff,(uint32_t)blog_p->wl))
    {
        wfd_p->count_rx_queue_packets++;
    }else
    {
        nbuff_free((pNBuff_t) pNBuff); //Drop packet ...
        gs_count_no_buffers[queue->qidx]++;
#ifdef FC_WFD_DEBUG
        if(net_ratelimit()) 
            printk("#### %s:%d #### FC WFD enqueue Drop packet after check Full !!... qidx:%d len:%d\n",__FUNCTION__,__LINE__,queue->qidx,WFD_NBuff_Queue_Size(queue));
#endif
        return 1; // PKT_DONE ...

    }


#if defined(FC_WFD_LAZY_DEFER)
    if(!WFD_NBuff_Defer_Queue((queue->qidx)))
    {//Non Defer Queue
        if(    (WFD_NBuff_Defer_Allow(queue))
            && (WFD_NBuff_Queue_Size(queue) < FC_WFD_DEFER_QUEUE_TARGET_LEN)
            && (WFD_NBuff_Queue_Size(queue) > FC_WFD_DEFER_QUEUE_MIN_LEN))
        { //Enable Defer ...
            WFD_NBuff_Defer_Queue_Set((queue->qidx));
#if defined(FC_WFD_DEBUG)  
            queue->defer_begin_jiffies = jiffies;
#endif /* FC_WFD_DEBUG */
            //wfd_p->wfd_rx_work_avail |= 1<<(queue->qidx);
            wfd_p->wfd_rx_work_avail &= ~(1<<(queue->qidx)) ;
            WFD_NBuff_Defer_Monitor_WakeUp();
        }else
        {
            if(!(wfd_p->wfd_rx_work_avail & 1<<(queue->qidx)))
            {
                wfd_p->wfd_rx_work_avail |= 1<<(queue->qidx) ;
                WFD_WAKEUP_RXWORKER(wfd_p->wfd_idx);
            }
        }
    }else
    {//Defer Queue
        if(WFD_NBuff_Queue_Size(queue) >= FC_WFD_DEFER_QUEUE_TARGET_LEN)
        { //Disable Defer ...
            queue->defer_cnt = 0;
            WFD_NBuff_Defer_Queue_Unset(queue->qidx);
            wfd_p->wfd_rx_work_avail |= (1<<(queue->qidx));
            WFD_WAKEUP_RXWORKER(wfd_p->wfd_idx);
        }
    }
#else
    {
        if(!(wfd_p->wfd_rx_work_avail & 1<<(queue->qidx)))
        {
            wfd_p->wfd_rx_work_avail |= 1<<(queue->qidx) ;
            WFD_WAKEUP_RXWORKER(wfd_p->wfd_idx);
        }
    }
#endif

    return 1; // PKT_DONE ...
}


int wfd_accelerator_init(void)
{
    int idx = 0;
    for(idx=0;idx<WFD_NUM_QUEUE_SUPPORTED;idx++)
    {
        WFD_NBuff_Queue_init(&fc_wfd_queue[idx]);
    }

#if defined(FC_WFD_LAZY_DEFER)
    /* Create Thread */
    FC_WFD_Defer_qidx_mask = 0x0;

    init_waitqueue_head(&fc_wfd_defer_thread_wqh);
    fc_wfd_defer_thread = kthread_create(WFD_NBuff_Defer_Monitor, NULL, "fc_wfd_defer");

     if(fc_wfd_defer_thread)
    {
        struct sched_param param;
        param.sched_priority = 5;
        sched_setscheduler(fc_wfd_defer_thread, SCHED_RR, &param);
        wake_up_process(fc_wfd_defer_thread);
    }
#endif

    fc_wfd_enqueue_cb = fcache_wfd_enqueue;


    printk("====  FC base WFD initialed  =====\n");

    return 0;
}

int fc_wfd_accelerator_deinit(void)
{
    int idx = 0;

    fc_wfd_enqueue_cb = NULL; //block fcache enqueue packet to WFD

#if defined(FC_WFD_LAZY_DEFER)
    /* Destory Monitor Thread */
    FC_WFD_Defer_qidx_mask = 0x0;

    kthread_stop(fc_wfd_defer_thread);
    fc_wfd_defer_thread = NULL;

    memset(&fc_wfd_defer_thread_wqh,0x00,sizeof(wait_queue_head_t));
#endif

    for(idx=0;idx<WFD_NUM_QUEUE_SUPPORTED;idx++)
    {
        WFD_NBuff_Queue_Deinit(&fc_wfd_queue[idx]);
    }

    printk("====  FC base WFD Deinited  =====\n");

    return 0;
}

static inline void sched_by_queue_status(wfd_object_t *wfd_p,pWFD_NBuff_Queue queue)
{
    if( (!WFD_NBuff_Queue_Empty(queue)))
    {
#if defined(FC_WFD_LAZY_DEFER)
        if(    (WFD_NBuff_Defer_Allow(queue))
            && (WFD_NBuff_Queue_Size(queue) < FC_WFD_DEFER_QUEUE_TARGET_LEN)
            && (WFD_NBuff_Queue_Size(queue) > FC_WFD_DEFER_QUEUE_MIN_LEN))
        { //Enable Defer ...
#if defined(FC_WFD_DEBUG)
            queue->defer_begin_jiffies = jiffies;
#endif /* FC_WFD_DEBUG */
            WFD_NBuff_Defer_Queue_Set((queue->qidx));
            WFD_NBuff_Defer_Monitor_WakeUp();
        }else
#endif
        {
            if(!(wfd_p->wfd_rx_work_avail & 1<<(queue->qidx)))
            {
                wfd_p->wfd_rx_work_avail |= 1<<(queue->qidx) ;
                WFD_WAKEUP_RXWORKER(wfd_p->wfd_idx);
            }
        }
    }
}


static inline FkBuff_t *nbuff_2_wfd_fkb(pNBuff_t pNBuff)
{
    FkBuff_t *fkb_p = NULL;

    if(!pNBuff)
    {
        return NULL;
    }

    if( IS_SKBUFF_PTR(pNBuff) )
    {  /* Convert SKB to FKB */
        struct sk_buff *skb_p = NULL;  
        skb_p = PNBUFF_2_SKBUFF(pNBuff);
#if defined(BCA_CPEROUTER)
        /* For wl/impl5x */
        /* Convert to fkb */
        fkb_p = fkb_init(BPM_PHEAD_TO_BUF(skb_p->head), BCM_PKT_HEADROOM,
                       skb_p->data, skb_p->len);

        fkb_p->recycle_hook       = gbpm_recycle_pNBuff;
        fkb_p->recycle_context    = 0;

        /* Free SKB header */
        skb_header_free(skb_p);
#else 
       /* For wl/impl2x non-bpm buf */
        /* CAUTION: Tag that the fkbuff is from sk_buff */
        fkb_p = (FkBuff_t *) &skb_p->fkbInSkb;

        fkb_p->flags = _set_in_skb_tag_(0); /* clear and set in_skb tag */
        FKB_CLEAR_LEN_WORD_FLAGS(fkb_p->len_word); /*clears bits 31-24 of skb->len */            
        fkb_set_ref(fkb_p, 1);
#endif

        /* No vaild dirty_p (dirty_p field overlap with fkb_p->flags )in fkb any more ,flush whole pkt .. */
        cache_flush_len(fkb_p->data, fkb_p->len);
    }else
    {/* FKB */
        fkb_p = PNBUFF_2_FKBUFF(pNBuff);
    }

    return fkb_p;
}

static inline struct sk_buff *nbuff_2_wfd_skb(pNBuff_t pNBuff)
{
    struct sk_buff *skb_p = NULL ;  

    if(!pNBuff)
    {
        return NULL;
    }

    if( IS_FKBUFF_PTR(pNBuff) )
    {/* Convert FKB to SKB */
        skb_p = nbuff_xlate((pNBuff_t )pNBuff);

        if (likely(skb_p))
        {
            FkBuff_t *fkb_p = NULL;

            fkb_p = PNBUFF_2_FKBUFF(pNBuff);
            SKB_BPM_TAINTED(skb_p);
        }else
        {
           // No skb buf 
            nbuff_free((pNBuff_t) pNBuff);
            return NULL;
        }
    }else
    {/* SKB */
        skb_p = PNBUFF_2_SKBUFF(pNBuff);
    }
    
#if !defined(PKTFWD) //PKTFWD will perform later ...
    //Clear cb for make sure PKTTAG is empty
   skb_cb_zero(skb_p);  
#endif

    return skb_p;

}


#if defined(BCM_PKTFWD)
/* PKTFWD */
/* Dispatch all pending pktlists to peer's pktlist_context, and wake peer */
static inline void wfd_pktfwd_xfer(pktlist_context_t *wfd_pktlist_context,
                                   const NBuffPtrType_t NBuffPtrType)
{
    int prio;
    pktlist_context_t *wl_pktlist_context = wfd_pktlist_context->peer;

    /* Grab the peer's pktlist_context, maybe a different thread. */
    PKTLIST_LOCK(wl_pktlist_context);

    /* Dispatch active mcast pktlists from wfd to wl - not by priority */
    __pktlist_xfer_work(wfd_pktlist_context, wl_pktlist_context,
                        &wfd_pktlist_context->mcast,
                        &wl_pktlist_context->mcast, "MCAST", NBuffPtrType);

    /* Dispatch active ucast pktlists from wfd to wl - by priority */
    for (prio = 0; prio < PKTLIST_PRIO_MAX; ++prio)
    {
        /* Process non empty ucast[] worklists in wfd pktlist context */
        __pktlist_xfer_work(wfd_pktlist_context, wl_pktlist_context,
                            &wfd_pktlist_context->ucast[prio],
                            &wl_pktlist_context->ucast[prio], "UCAST",
                            NBuffPtrType);
    }

            
#if 0 
    if(net_ratelimit())
    {
        printk("#### %s:%d #### %pS --> %p\n",__FUNCTION__,__LINE__
            ,wfd_pktlist_context->xfer_fn
            ,wl_pktlist_context
            );
    }
#endif


    /* Release peer's pktlist context */
    PKTLIST_UNLK(wl_pktlist_context);

    /* Wake peer wl thread: invoke handoff handler to wake peer driver.
     * handoff handler is the HOOK32 wfd_completeHook in wfd_bind.
     */
    (wfd_pktlist_context->xfer_fn)(wfd_pktlist_context->peer);

    wfd_pktlist_context->dispatches++;
}   /* wfd_pktfwd_xfer() */


static uint32_t
wfd_bulk_fkb_get_pktfwd(unsigned long qid, unsigned long budget, void *priv)
{
    unsigned int rx_pktcnt = 0;
    wfd_object_t *wfd_p = (wfd_object_t *)priv;
    pWFD_NBuff_Queue queue;
    register BlogWfd_t wfd_meta;

    queue=(&fc_wfd_queue[wfd_get_minQIdx(wfd_p->wfd_idx)+ qid ]) ;

    {
        pNBuff_t pNBuff;
        uint16_t pktlist_prio, pktlist_dest;
        pktlist_context_t *wfd_pktlist_context = wfd_p->pktlist_context_p;
        pktlist_context_t *dhd_pktlist_context = wfd_pktlist_context->peer;
        uint16_t pktfwd_key = (uint16_t) ~0;
        uint16_t flowring_idx;
        FkBuff_t *fkb_p = NULL;        
        wfd_swqueue_t     * wfd_swqueue = wfd_p->wfd_swqueue;

        bcm_prefetch(&(queue->buflist[queue->head]));
        bcm_prefetch(&(queue->wlmetalist[queue->head]));

        WFD_SWQUEUE_LOCK(wfd_swqueue); // +++++++++++++++++++++++++++++++++++++++++

        while (budget--)
        {
            WFD_NBuff_DeQueue_meta(queue,pNBuff,wfd_meta);

            if(!(fkb_p = nbuff_2_wfd_fkb(pNBuff)))
            {
                if(pNBuff)
                    gs_count_no_skbs[(queue->qidx)]++;
                
                break; //nothing in queue ...
            }

            if( ((queue->head) & 0x03) == 0 )
            {
                bcm_prefetch(&(queue->buflist[queue->head]));
                bcm_prefetch(&(queue->wlmetalist[queue->head]));
            }
            
            fkb_p->wl.ucast.dhd.is_ucast     = 1; /* Fix me : always unicast for now */
            fkb_p->wl.ucast.dhd.wl_prio      = wfd_meta.dhd_ucast.priority;
            fkb_p->wl.ucast.dhd.flowring_idx = wfd_meta.dhd_ucast.flowring_idx;
            fkb_p->wl.ucast.dhd.ssid         = wfd_meta.dhd_ucast.ssid;

            /* Fetch the 3b WLAN prio, by skipping the 1b IQPRIO */
            pktlist_prio =  wfd_meta.dhd_ucast.priority;
            pktfwd_key = (uint16_t) ~0;
    
            if(fkb_p->wl.pktfwd.is_ucast)
            {
                ASSERT(dhd_pktlist_context->keymap_fn);
                flowring_idx = wfd_meta.dhd_ucast.flowring_idx;
    
                (dhd_pktlist_context->keymap_fn)(wfd_p->wl_radio_idx,
                    &pktfwd_key, &flowring_idx, pktlist_prio, PKTFWD_KEYMAP_F2K);
    
                if (pktfwd_key == ((uint16_t)~0)) {
                    /* Stale packets */
                    //wfd_databuf_free_to_pool(PFKBUFF_TO_PDATA(fkb_p, BCM_PKT_HEADROOM));
                    nbuff_free((pNBuff_t) pNBuff); //Drop packet ...
                    gs_count_rx_error[(queue->qidx)]++;
                    continue;
                }
    
                pktlist_dest = PKTLIST_DEST(pktfwd_key); 
            }
            else
            {
                pktlist_dest = PKTLIST_MCAST_ELEM; /* last col in 2d pktlist */ 
            }
    
            PKTFWD_ASSERT(pktlist_dest <= PKTLIST_MCAST_ELEM);
            PKTFWD_ASSERT(pktlist_prio <  PKTLIST_PRIO_MAX);
    
            __pktlist_add_pkt(wfd_pktlist_context, /* add to local pktlist */
                              pktlist_prio, pktlist_dest, pktfwd_key,
                              FKBUFF_2_PNBUFF(fkb_p), FKBUFF_PTR);

            rx_pktcnt++;            
        }

        WFD_SWQUEUE_UNLK(wfd_swqueue); // -----------------------------------------       

        if (rx_pktcnt)
        {
            wfd_pktfwd_xfer(wfd_pktlist_context, FKBUFF_PTR);
            wfd_p->wl_chained_packets += rx_pktcnt;
        }
    }

    sched_by_queue_status(wfd_p,queue);

    return rx_pktcnt;
}


static uint32_t
wfd_bulk_skb_get_pktfwd(unsigned long qid, unsigned long budget, void *priv)
{
    unsigned int rx_pktcnt = 0;
    pWFD_NBuff_Queue queue  = NULL ;
    wfd_object_t *wfd_p = (wfd_object_t *)priv;

    queue=(&fc_wfd_queue[wfd_get_minQIdx(wfd_p->wfd_idx)+ qid ]) ;

    {
        pNBuff_t pNBuff;
        uint16_t pktlist_prio, pktlist_dest;
        pktlist_context_t *wfd_pktlist_context = wfd_p->pktlist_context_p;
        uint16_t wl_key = 0;
        struct sk_buff *skb_p = NULL ;
        register BlogWfd_t wfd_meta;

        wfd_swqueue_t     * wfd_swqueue = wfd_p->wfd_swqueue;

        bcm_prefetch(&(queue->buflist[queue->head]));
        bcm_prefetch(&(queue->wlmetalist[queue->head]));

        WFD_SWQUEUE_LOCK(wfd_swqueue); // +++++++++++++++++++++++++++++++++++++++++

        while (budget--)
        {
            WFD_NBuff_DeQueue_meta(queue,pNBuff,wfd_meta);

            if(!(skb_p = nbuff_2_wfd_skb(pNBuff)))
            {

                if(pNBuff) //cause by allocate skb fail ..
                    gs_count_no_skbs[(queue->qidx)]++;
                
                break; //no buf ? wait next time ....
            }

            if( ((queue->head) & 0x03) == 0 )
            {
                bcm_prefetch(&(queue->buflist[queue->head]));
                bcm_prefetch(&(queue->wlmetalist[queue->head]));
            }

            skb_p->wl.ucast.nic.is_ucast    = 1; /* Fix me : always unicast for now */
            skb_p->wl.ucast.nic.wl_chainidx = wfd_meta.nic_ucast.chain_idx;
            skb_p->wl.ucast.nic.wl_prio     = wfd_meta.nic_ucast.priority;

            if (skb_p->wl.ucast.nic.is_ucast)
            {
                wl_key = wfd_meta.nic_ucast.chain_idx;
                pktlist_dest = PKTLIST_DEST(wl_key);
            }
            else
            {
                wl_key = 0;
                pktlist_dest = PKTLIST_MCAST_ELEM; /* last col in 2d pktlist */
            }

            /* Fetch the 3b WLAN prio, by skipping the 1b IQPRIO */
            pktlist_prio = GET_WLAN_PRIORITY(wfd_meta.nic_ucast.priority);

            PKTFWD_ASSERT(pktlist_dest <= PKTLIST_MCAST_ELEM);
            PKTFWD_ASSERT(pktlist_prio <  PKTLIST_PRIO_MAX);

            /* add to local pktlist */
            __pktlist_add_pkt(wfd_pktlist_context, pktlist_prio, pktlist_dest,
                              wl_key, skb_p, SKBUFF_PTR);

            rx_pktcnt++;     

        }

        WFD_SWQUEUE_UNLK(wfd_swqueue); // -----------------------------------------

        if (rx_pktcnt)
        {
            wfd_pktfwd_xfer(wfd_pktlist_context, SKBUFF_PTR);
            wfd_p->wl_chained_packets += rx_pktcnt;
        }
    }

    sched_by_queue_status(wfd_p,queue);

    return rx_pktcnt;
}

#else 
/* PKTC */
static uint32_t
wfd_bulk_fkb_get_pktc(unsigned long qid, unsigned long budget, void *priv)
{
    unsigned int rx_pktcnt = 0;
    pNBuff_t pNBuff = NULL;
    FkBuff_t *fkb_p = NULL;
    pWFD_NBuff_Queue queue  = NULL ;
    wfd_object_t *wfd_p = (wfd_object_t *)priv;
    void *rx_pkts[NUM_PACKETS_TO_READ_MAX];
    register BlogWfd_t wfd_meta;

    queue=(&fc_wfd_queue[wfd_get_minQIdx(wfd_p->wfd_idx) + qid]);

    bcm_prefetch(&(queue->buflist[queue->head]));
    bcm_prefetch(&(queue->wlmetalist[queue->head]));

    while (budget--)
    {
        WFD_NBuff_DeQueue_meta(queue,pNBuff,wfd_meta);

        if(!(fkb_p = nbuff_2_wfd_fkb(pNBuff)))
        {
            if(pNBuff)
                gs_count_no_skbs[(queue->qidx)]++;
            break; //nothing in queue ...
        }
        
        if( ((queue->head) & 0x03) == 0 )
        {
            bcm_prefetch(&(queue->buflist[queue->head]));
            bcm_prefetch(&(queue->wlmetalist[queue->head]));
        }

        fkb_p->wl.ucast.dhd.is_ucast     = 1; /* Fix me : always unicast for now */
        fkb_p->wl.ucast.dhd.wl_prio      = wfd_meta.dhd_ucast.priority;
        fkb_p->wl.ucast.dhd.flowring_idx = wfd_meta.dhd_ucast.flowring_idx;
        fkb_p->wl.ucast.dhd.ssid         = wfd_meta.dhd_ucast.ssid;

        rx_pkts[rx_pktcnt] = (void *)fkb_p;
        rx_pktcnt++;
    }

    if (rx_pktcnt)
    {
#if 0
        static int max_rx_pktcnt=0;
        max_rx_pktcnt = rx_pktcnt>max_rx_pktcnt? rx_pktcnt : max_rx_pktcnt;
        if(net_ratelimit())
            printk("#### %s:%d #### qid:%d rx_pktcnt:%d \n",__FUNCTION__,__LINE__,(queue->qidx),rx_pktcnt);    
#endif
        (void)wfd_p->wfd_fwdHook(rx_pktcnt, (unsigned long)rx_pkts, wfd_p->wl_radio_idx, 0);
        wfd_p->wl_chained_packets += rx_pktcnt;
    }

    sched_by_queue_status(wfd_p,queue);

    return rx_pktcnt;
}


static uint32_t
wfd_bulk_skb_get_pktc(unsigned long qid, unsigned long budget, void *priv)
{
    unsigned int rx_pktcnt = 0;
    void *rx_pkts[NUM_PACKETS_TO_READ_MAX];
    struct sk_buff *skb_p = NULL ;
    pNBuff_t pNBuff = NULL;
    pWFD_NBuff_Queue queue  = NULL ;
    wfd_object_t *wfd_p = (wfd_object_t *)priv;
    register BlogWfd_t wfd_meta;

    queue=(&fc_wfd_queue[wfd_get_minQIdx(wfd_p->wfd_idx) + qid]);

    bcm_prefetch(&(queue->buflist[queue->head]));
    bcm_prefetch(&(queue->wlmetalist[queue->head]));

    while (budget--)
    {
        WFD_NBuff_DeQueue_meta(queue,pNBuff,wfd_meta);

        if(!(skb_p = nbuff_2_wfd_skb(pNBuff)))
        {
        
            if(pNBuff) //cause by allocate skb fail ..
                gs_count_no_skbs[(queue->qidx)]++;
            
            break; //no buf ? wait next time ....
        }

        if( ((queue->head) & 0x03) == 0 )
        {
            bcm_prefetch(&(queue->buflist[queue->head]));
            bcm_prefetch(&(queue->wlmetalist[queue->head]));
        }
        
        skb_p->wl.ucast.nic.is_ucast    = 1; /* Fix me : always unicast for now */
        skb_p->wl.ucast.nic.wl_chainidx = wfd_meta.nic_ucast.chain_idx;
        skb_p->wl.ucast.nic.wl_prio     = wfd_meta.nic_ucast.priority;

        rx_pkts[rx_pktcnt] = (void *)skb_p;
        rx_pktcnt++;
    }

    if (rx_pktcnt)
    {
#if 0
        static int max_rx_pktcnt=0;
        max_rx_pktcnt = rx_pktcnt>max_rx_pktcnt? rx_pktcnt : max_rx_pktcnt;
        if(net_ratelimit())
            printk("#### %s:%d #### qid:%d rx_pktcnt:%d \n",__FUNCTION__,__LINE__,(queue->qidx),rx_pktcnt);    
#endif
        (void)wfd_p->wfd_fwdHook(rx_pktcnt, (unsigned long)rx_pkts, wfd_p->wl_radio_idx, 0);
        wfd_p->wl_chained_packets += rx_pktcnt;
    }

    sched_by_queue_status(wfd_p,queue);

    return rx_pktcnt;
}
#endif

static inline int wfd_config_rx_queue(int wfd_idx,int qid, uint32_t queue_size,
                                      enumWFD_WlFwdHookType eFwdHookType)
{
    pWFD_NBuff_Queue queue  = NULL;

    if(queue_size>FC_WFD_MAX_QUEUE_LEN)
        return -1;

    queue=(&fc_wfd_queue[wfd_get_minQIdx(wfd_idx) + qid]);

    if(queue_size)
    {
        //WFD_NBuff_Queue_init(queue);
        WFD_NBuff_Queue_Deinit(queue);
        queue->qidx = wfd_get_minQIdx(wfd_idx) + qid;
        queue->max_size = queue_size;
        queue->wfd_idx = wfd_idx;
#if defined(FC_WFD_LAZY_DEFER)
        /* Only apply defer lowest priority SKB queue (Even-Low, Odd-High for now) */
        queue->defer_mode= ( (qid != 0x00)) ? (FC_WFD_DEFER_DENY) : (FC_WFD_DEFER_ALLOW);

        printk("WFD Queue %d: qid:%d wfd_idx=%d max_size=%d defer:%d\n"
            ,queue->qidx,qid,queue->wfd_idx,queue->max_size,WFD_NBuff_Defer_Allow(queue));
#else
        printk("WFD Queue %d: qid:%d wfd_idx=%d max_size=%d\n"
            ,queue->qidx,qid,queue->wfd_idx,queue->max_size);
#endif
    }else
    { //Release all pkts in queue...
        WFD_NBuff_Queue_Deinit(queue);
    }
    return 0;
}

#endif
