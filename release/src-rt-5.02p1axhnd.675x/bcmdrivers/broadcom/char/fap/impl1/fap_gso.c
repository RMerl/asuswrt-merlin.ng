/*
<:copyright-BRCM:2009:proprietary:standard

   Copyright (c) 2009 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
*/

#if defined(CONFIG_BCM_FAP_GSO_LOOPBACK)

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

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0))
#include "kmap_skb.h"
#else
#include "linux/highmem.h"
#endif

#include "fap.h"
#include "fap_hw.h"
#include "fap_local.h"
#include "fap4ke_memory.h"
#include "fap_dqm.h"
#include "fap_dqmHost.h"
#include "fap_task.h"
#include "fap4ke_packet.h"
#include "bcmPktDma.h"
#include "fap_swq.h"



/* use FAP 0 */
#define GSO_LOOPBACK_FAPID   0

extern fapGsoDesc_t *alloc_fapGsoDesc(void);
extern void free_fapGsoDesc(fapGsoDesc_t *gsoDesc_p);

static struct task_struct *fapGso_LoopBkThread = NULL;
static wait_queue_head_t   fapGso_wqh; 
static int rx_work_avail = 0;

static DEFINE_SPINLOCK(fapGsoloopBk_H2Flock);
#define GSO_LOOPBACK_H2F_LOCK() spin_lock_bh(&fapGsoloopBk_H2Flock) 
#define GSO_LOOPBACK_H2F_UNLOCK() spin_unlock_bh(&fapGsoloopBk_H2Flock) 

static DEFINE_SPINLOCK(fapGsoloopBk_F2Hlock);
#define GSO_LOOPBACK_F2H_LOCK() spin_lock_bh(&fapGsoloopBk_F2Hlock) 
#define GSO_LOOPBACK_F2H_UNLOCK() spin_unlock_bh(&fapGsoloopBk_F2Hlock) 


static SWQInfo_t gsoLoopBkH2FQInfo;
static SWQInfo_t gsoLoopBkF2HQInfo;


static inline int fapGsoLoopBk_send2Fap(host2Fap_GsoloopBkMsg_t *msg)
{
    GSO_LOOPBACK_H2F_LOCK();
    if(swqXmitAvailableHost(gsoLoopBkH2FQInfo.swq, gsoLoopBkH2FQInfo.msgSize,
                 gsoLoopBkH2FQInfo.qStart, gsoLoopBkH2FQInfo.qEnd))
    {
        swqXmitMsgHost(gsoLoopBkH2FQInfo.swq, (SWQDataMsg_t *)msg, gsoLoopBkH2FQInfo.msgSize,
                gsoLoopBkH2FQInfo.fapId, gsoLoopBkH2FQInfo.dqm,  
                gsoLoopBkH2FQInfo.qStart, gsoLoopBkH2FQInfo.qEnd);

        GSO_LOOPBACK_H2F_UNLOCK();
        return 0;
    }
    else
    {
        GSO_LOOPBACK_H2F_UNLOCK();
        /* return -1 so  caller can drop the skb & gsoDesc  */
        return -1;
    }
}

int fapGsoLoopBk_offload(struct sk_buff *skb, unsigned int txDevId)
{

    fapGsoDesc_t *gsoDesc = NULL;
    struct net_device *dev = skb->dev;
    host2Fap_GsoloopBkMsg_t txMsg;
    uint16 nr_frags, i;

    if(dev == NULL)
    {
        printk("skb<%p> skb->len=%d, nr_frags=%d skb->dev<%p>\n",
                skb, skb->len ,skb_shinfo(skb)->nr_frags, skb->dev);
        goto drop_exit;
    }


    if(skb_is_gso(skb) || skb_shinfo(skb)->nr_frags)
    {
        /*note: FRAGLIST is not supported for GSOLOOPBACK*/ 
        nr_frags = skb_shinfo(skb)->nr_frags;

        if(nr_frags >  FAP_MAX_GSO_FRAGS)
        {
            printk("%s: nr_frags %d exceed max\n",__FUNCTION__, nr_frags);

            if(__skb_linearize(skb))
            {
                printk("%s:skb linearize failed dropping the skb \n",__FUNCTION__);
                goto drop_exit;
            }
        }

        /*read nr_frags again as skb might have changed in skb_linearize*/
        nr_frags = skb_shinfo(skb)->nr_frags;

        if(nr_frags)
        {
            uint32 headerLen, totalFragLen;
            skb_frag_t *frag;
            uint8 *vaddr;


            gsoDesc = alloc_fapGsoDesc();
            if(!gsoDesc)
            {
                printk("%s: failed to allocate gdoDesc\n",__FUNCTION__);
                goto drop_exit;
            }

            totalFragLen = 0;
            for(i=0; i < nr_frags; i++ )
            {
                frag = &skb_shinfo(skb)->frags[i];

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0)
                vaddr = kmap_skb_frag(frag);
#else
                vaddr = kmap_atomic(frag->page.p);
#endif
                gsoDesc->frag_data[i]= vaddr + frag->page_offset;
                gsoDesc->frag_len[i] = frag->size;
                cache_flush_len(gsoDesc->frag_data[i], gsoDesc->frag_len[i]);
                totalFragLen += frag->size;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0)
                kunmap_skb_frag(vaddr);
#else
                kunmap_atomic(vaddr);
#endif
            }

            gsoDesc->recycle_key = (uint32)skb;
            gsoDesc->nr_frags = nr_frags;
            gsoDesc->mss = skb_shinfo(skb)->gso_size;
            gsoDesc->totalLen = skb->len;
            headerLen = skb->len - totalFragLen;
            txMsg.hdr = skb->data;
            txMsg.hdrLen  = headerLen;
            txMsg.gsoDesc  = gsoDesc;
            txMsg.mss  = skb_shinfo(skb)->gso_size;

            txMsg.msgId = HOST2FAP_MSG_GSO_LOOPBACK_FRAG;

        }
        else
        {
            txMsg.hdr = skb->data;
            txMsg.len  = skb->len;
            txMsg.recycle_key  = (uint32)skb;
            txMsg.mss  = skb_shinfo(skb)->gso_size;

            txMsg.msgId  = HOST2FAP_MSG_GSO_LOOPBACK_GSO;
        }
    }
    else if(skb->ip_summed == CHECKSUM_PARTIAL)
    {
        if(skb_header_cloned(skb) && pskb_expand_head(skb, 0, 0, GFP_ATOMIC))
        {
            printk("%s: failed to expand skb clone head\n",__FUNCTION__);
            goto drop_exit;
        }

        txMsg.hdr = skb->data;
        txMsg.len  = skb->len;
        txMsg.recycle_key  = (uint32)skb;
        txMsg.mss  = 0;

        txMsg.msgId = HOST2FAP_MSG_GSO_LOOPBACK_CSUM;
    }
    else
    {
        txMsg.hdr = skb->data;
        txMsg.len  = skb->len;
        txMsg.recycle_key  = (uint32)skb;
        txMsg.mss  = 0;

        txMsg.msgId = HOST2FAP_MSG_GSO_LOOPBACK_PASSTHRU;
    }
    
    txMsg.devId = txDevId;

    cache_flush_len(txMsg.hdr, txMsg.len);
    
#if 1
    BCM_LOG_INFO(BCM_LOG_ID_FAP, "txMsg.msgId=%d, txMsg.devId=%d, txMsg.priority=%d, txMsg.hdr=%p,\n"
            "txMsg.recycle_key=%p, txMsg.len=%d txMsg.mss=%d \n",
             txMsg.msgId, txMsg.devId, txMsg.priority, txMsg.hdr,
            (void *)txMsg.recycle_key, txMsg.len, txMsg.mss);
#endif

    if(fapGsoLoopBk_send2Fap(&txMsg) == 0)
    {
        /* update stats */
        return 0;
    }
    else
    {
        /* update stats */
        dev_kfree_skb_any(skb);
        free_fapGsoDesc(gsoDesc);
        return 0;
    }
drop_exit:
    /*in drop cases just do cache invalidate */
    cache_invalidate_len(skb->data, skb->len);
    /* update stats */
    dev_kfree_skb_any(skb);
    return (0);
}

/* recycle callback for skb->data */
static void fapGsoLoopBk_recycle(struct sk_buff *skb, uint32 context, uint32 flags)
{

    if (flags & SKB_DATA_RECYCLE) {
        uint8 *dataStart_p, *dataEnd_p;

        //dataStart_p = skb->head + RX_ENET_SKB_HEADROOM;
        dataStart_p = skb->data;
        dataEnd_p = (UINT8*)(skb_shinfo(skb)) + sizeof(struct skb_shared_info);

        cache_flush_region(dataStart_p, dataEnd_p);
    
        /* free the buffer to global BPM pool */
        {
            /* BPM pool needs data buffer pointer. Use skb->head */
            uint8 *bpm_buf_p = skb->head;
            bpm_buf_p += BCM_PKT_HEADROOM;
            gbpm_free_buf((void *)bpm_buf_p);
        }
    }
    else
    {
       BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Error only DATA recycle is supported\n");

    }
}

static int fapGsoLoopBk_threadFunc(void *thread_data)
{
    uint32 dqm = DQM_FAP2HOST_GSO_LOOPBACK_Q;
    uint32     qbit = 1 << dqm;

    uint32 budget;
    void *pBuf =NULL;
    uint16 len=0;
    struct net_device *dev;
    struct sk_buff *skb=NULL;
    FkBuff_t * pFkb = NULL;
    uint32 rxAvail;
    fap2Host_GsoloopBkMsg_t rxMsg;

    uint32 fapId = GSO_LOOPBACK_FAPID;


    BCM_LOG_INFO(BCM_LOG_ID_FAP,"fapGso_LoopBkThread running\n");
    while(1)
    {
        wait_event_interruptible(fapGso_wqh,rx_work_avail);

        if (kthread_should_stop())
        {
            printk(KERN_INFO "kthread_should_stop detected on fapGso_LoopBkThread\n");
            break;
        }

        GSO_LOOPBACK_F2H_LOCK();
        budget = 64;
        while(1)
        {
            rxAvail = swqRecvAvailableHost(gsoLoopBkF2HQInfo.swq);
            if(!rxAvail)
                break;

            if(--budget == 0)
                break;

            swqRecvMsgHost(gsoLoopBkF2HQInfo.swq, (SWQDataMsg_t *)&rxMsg, gsoLoopBkF2HQInfo.msgSize,
                            gsoLoopBkF2HQInfo.qStart, gsoLoopBkF2HQInfo.qEnd);

            pBuf= rxMsg.pBuf;
            len= rxMsg.len;

#if 1
            BCM_LOG_INFO(BCM_LOG_ID_FAP, "rxMsgId= %d rxMsg.devId=%d rxMsg.priority =%d rxMsg.pBuf =%p rxMsg.len =%d\n",
                    rxMsg.msgId, rxMsg.devId, rxMsg.priority,rxMsg.pBuf, rxMsg.len);
#endif

            if(rxMsg.msgId == FAP2HOST_MSG_GSO_LOOPBACK_XMIT_BUF)
            {

                dev = bcm_gso_loopback_devid2devptr(rxMsg.devId);
                if(!dev)
                {
                    printk( KERN_ERR "%s: failed to find the gso device for devId=%d msgId=%d\n",
                            __FUNCTION__, rxMsg.devId, rxMsg.msgId);
                    goto drop_bpm_buffer;
                }

                /*TODO do we need to increment any reference to this device here */
                pFkb = fkb_init(pBuf, BCM_PKT_HEADROOM,
                        pBuf, len);
                /*set the recyle hook */
                pFkb->recycle_hook = (RecycleFuncP)fapGsoLoopBk_recycle;
                pFkb->recycle_context = 0;

                skb = skb_xlate(pFkb);
                if(!skb)
                {
                    printk(" %s: failed to allocate skb\n", __FUNCTION__);
                    goto drop_bpm_buffer; 
                }

                skb->dev= dev;
                skb->priority = rxMsg.priority;

                dev->netdev_ops->ndo_start_xmit(skb, dev);
            }
            else if(rxMsg.msgId == FAP2HOST_MSG_GSO_LOOPBACK_XMIT_HOSTBUF)
            {

                dev = bcm_gso_loopback_devid2devptr(rxMsg.devId);
                if(!dev)
                {
                    printk( KERN_ERR "%s: failed to find the gso device for devId=%d msgId=%d\n",
                            __FUNCTION__, rxMsg.devId, rxMsg.msgId);

                    dev_kfree_skb_any(( struct sk_buff *)pBuf);
                }

                skb =  (struct sk_buff *)pBuf;  
                /*update dev pointer */
                skb->dev = dev;
                skb->ip_summed = CHECKSUM_COMPLETE;

                dev->netdev_ops->ndo_start_xmit(skb, dev);
            }
            else if(rxMsg.msgId == FAP2HOST_MSG_GSO_LOOPBACK_PASSTHRU) 
            {
                dev = bcm_gso_loopback_devid2devptr(rxMsg.devId);
                if(!dev)
                {
                    printk( KERN_ERR "%s: failed to find the gso device for devId=%d msgId=%d\n",
                            __FUNCTION__, rxMsg.devId, rxMsg.msgId);
                    dev_kfree_skb_any(( struct sk_buff *)pBuf);
                }

                skb =  (struct sk_buff *)pBuf;  
                /*update dev pointer */
                skb->dev = dev;
                dev->netdev_ops->ndo_start_xmit(skb, dev);
            }
            else if(rxMsg.msgId == FAP2HOST_MSG_GSO_LOOPBACK_FREE_HOSTBUF) 
            {
                dev_kfree_skb_any(( struct sk_buff *)pBuf);
            }
            else
            {
                BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Unexpected message rxMsgId= %d rxMsg.devId=%d rxMsg.priority =%d rxMsg.pBuf =%p rxMsg.len =%d\n",
                        rxMsg.msgId, rxMsg.devId, rxMsg.priority,rxMsg.pBuf, rxMsg.len);
                goto drop_bpm_buffer;
            }

            continue;


drop_bpm_buffer:
            gbpm_free_buf((void *)pBuf);
        }

        /* if more packets present in SW queue, reschedule thread */
        if(!rxAvail)
        {
            DQMQueueDataReg_S dqmMsg;
            /* read the dummy message in the associated dqm */
            if(dqmRecvAvailableHost(fapId,DQM_FAP2HOST_GSO_LOOPBACK_Q))
            {
                dqmRecvMsgHost(fapId,DQM_FAP2HOST_GSO_LOOPBACK_Q, 1, &dqmMsg);
                /* clear the interrupt */
                dqmClearNotEmptyIrqStsHost(fapId, qbit);
            }
            /* re-check if packets are available */
            rxAvail = swqRecvAvailableHost(gsoLoopBkF2HQInfo.swq);
        }

        if(rxAvail)
        {
            GSO_LOOPBACK_F2H_UNLOCK();
            /* there are more packets available, lets re-schedule */
            yield();
        }
        else
        {
            rx_work_avail =0;
            GSO_LOOPBACK_F2H_UNLOCK();

            /* enable the interrupt */
            dqmEnableNotEmptyIrqMskHost(fapId, qbit);
        }
    }
    return 0;
}
 

struct task_struct* fapGsoLoopBk_createThread(void)
{
    struct task_struct *tsk;

    tsk = kthread_create(fapGsoLoopBk_threadFunc, NULL,"fapGsoLoopBk");

    if (IS_ERR(tsk)) {
        printk("fapGso_LoopBkThread creation failed\n");
        return NULL;
    }

    kthread_bind(tsk, 0);/*pin the thread to cpu0 */

    printk("fapGso_LoopBkThread created successfully\n");
    return tsk;
}

/*****************************************************************************
* Function: fapGso_recv_dqmhandler                                           *
*                                                                            *
* Description: Handles GSO loopback pkts from FAP                            *
******************************************************************************/
void fapGsoLoopBk_recv_dqmhandler(uint32 fapId, unsigned long unused)
{
    if( fapId != GSO_LOOPBACK_FAPID)
    {
        printk(KERN_ERR "Unexpected gso loopback interrupt from fapId=%d\n",(int)fapId);
        return;
    }

    if (rx_work_avail == 0) 
    {
        rx_work_avail = 1; 
        wake_up_interruptible(&fapGso_wqh);
    }
    else
    {
        printk(KERN_ERR "%s:Error interupt should not occur when rx_work_avail != 0\n", __FUNCTION__);
    }
}

void fapGsoLoopBk_dumpSWQs(uint32 fapId)
{
    printk("SWQ: HOST2FAP_GSO_LOOPBACK \n");

    swqDumpHost(&pHostPsmGbl(fapId)->gsoLoopBackH2FSwq);

    printk("SWQ: FAP2HOST_GSO_LOOPBACK \n");
    swqDumpHost(&pHostPsmGbl(fapId)->gsoLoopBackF2HSwq);

    printk("GsoLoopBackThread->state =%ld\n", fapGso_LoopBkThread->state);
}

void fapGsoLoopBk_uninit(void)
{
    __attribute__((unused)) const uint32 fapId = GSO_LOOPBACK_FAPID;

    if(pHostFapSdram(fapId)->initParams.gsoLoopBackH2FSwqMem_p != NULL)
    {
        kfree((void *)KSEG0ADDR(pHostFapSdram(fapId)->initParams.gsoLoopBackH2FSwqMem_p));
        pHostFapSdram(fapId)->initParams.gsoLoopBackH2FSwqMem_p= NULL;
    }

    if(pHostFapSdram(fapId)->initParams.gsoLoopBackF2HSwqMem_p != NULL)
    {
        kfree((void *)KSEG0ADDR(pHostFapSdram(fapId)->initParams.gsoLoopBackF2HSwqMem_p));
        pHostFapSdram(fapId)->initParams.gsoLoopBackF2HSwqMem_p= NULL;
    }
}


int fapGsoLoopBk_init(void)
{
    uint32 fapId = GSO_LOOPBACK_FAPID;

    init_waitqueue_head(&fapGso_wqh);
    fapGso_LoopBkThread = fapGsoLoopBk_createThread();
    if(!fapGso_LoopBkThread)
    {
        return FAP_ERROR;
    }
    wake_up_process(fapGso_LoopBkThread);

    /*allocate and set the DDR memory for SWQueues */
    {
        int bufSize = SWQ_HOST2FAP_GSO_LOOPBACK_Q_MEM_SIZE *4;
        uint8 *swqMem_p = kmalloc(bufSize, GFP_KERNEL);
        

        if(swqMem_p == NULL)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Could not allocate(%d bytes) for SWQ_HOST2FAP_GSO_LOOPBACK_Q ",bufSize);
            return FAP_ERROR;
        }

        /* Invalidate memory from Host D$ */
        fap_cacheInvFlush((void *)(swqMem_p),
                          (void *)(swqMem_p + bufSize - 1),
                          0);

        pHostFapSdram(fapId)->initParams.gsoLoopBackH2FSwqMem_p =
            (uint32 *)KSEG1ADDR(swqMem_p);

        printk("Allocated FAP%d SWQ_HOST2FAP_GSO_LOOPBACK_Q mem=%p : %d bytes\n",
                fapId, pHostFapSdram(fapId)->initParams.gsoLoopBackH2FSwqMem_p, bufSize);

        bufSize = SWQ_FAP2HOST_GSO_LOOPBACK_Q_MEM_SIZE *4;
        swqMem_p = kmalloc(bufSize, GFP_KERNEL);

        if(swqMem_p == NULL)
        {
            BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Could not allocate(%d bytes) for SWQ_FAP2HOST_GSO_LOOPBACK_Q ",bufSize);
            kfree((void *)KSEG0ADDR(pHostFapSdram(fapId)->initParams.gsoLoopBackH2FSwqMem_p));
            pHostFapSdram(fapId)->initParams.gsoLoopBackH2FSwqMem_p= NULL;
            return FAP_ERROR;
        }

        /* Invalidate memory from Host D$ */
        fap_cacheInvFlush((void *)(swqMem_p),
                          (void *)(swqMem_p + bufSize - 1),
                          0);

        pHostFapSdram(fapId)->initParams.gsoLoopBackF2HSwqMem_p =
            (uint32 *)KSEG1ADDR(swqMem_p);

        printk("Allocated FAP%d SWQ_FAP2HOST_GSO_LOOPBACK_Q mem=%p : %d bytes\n",
                fapId, pHostFapSdram(fapId)->initParams.gsoLoopBackF2HSwqMem_p, bufSize);
    }
    
    /* store the static information about the swq's in cached memory */ 
     gsoLoopBkH2FQInfo.swq     = &pHostPsmGbl(fapId)->gsoLoopBackH2FSwq;
     gsoLoopBkH2FQInfo.qStart  = pHostFapSdram(fapId)->initParams.gsoLoopBackH2FSwqMem_p;
     gsoLoopBkH2FQInfo.qEnd    = gsoLoopBkH2FQInfo.qStart + SWQ_HOST2FAP_GSO_LOOPBACK_Q_MEM_SIZE;
     gsoLoopBkH2FQInfo.dqm     = DQM_HOST2FAP_GSO_LOOPBACK_Q;
     gsoLoopBkH2FQInfo.fapId   = fapId;
     gsoLoopBkH2FQInfo.msgSize = SWQ_HOST2FAP_GSO_LOOPBACK_Q_MSG_SIZE;

     gsoLoopBkF2HQInfo.swq     = &pHostPsmGbl(fapId)->gsoLoopBackF2HSwq;
     gsoLoopBkF2HQInfo.qStart  = pHostFapSdram(fapId)->initParams.gsoLoopBackF2HSwqMem_p;
     gsoLoopBkF2HQInfo.qEnd    = gsoLoopBkF2HQInfo.qStart + SWQ_FAP2HOST_GSO_LOOPBACK_Q_MEM_SIZE;
     gsoLoopBkF2HQInfo.dqm     = DQM_FAP2HOST_GSO_LOOPBACK_Q;
     gsoLoopBkF2HQInfo.fapId   = fapId;
     gsoLoopBkF2HQInfo.msgSize = SWQ_FAP2HOST_GSO_LOOPBACK_Q_MSG_SIZE;

     printk("GSO LOOPBACK Cached HOST2FAP Q INFO:\n"
             " Swq =%p qStart=%p qEnd=%p msgSize=%d dqm=%d fapId=%d\n",
             gsoLoopBkH2FQInfo.swq, gsoLoopBkH2FQInfo.qStart, gsoLoopBkH2FQInfo.qEnd,
             gsoLoopBkH2FQInfo.msgSize, gsoLoopBkH2FQInfo.dqm, gsoLoopBkH2FQInfo.fapId);    

     printk("GSO LOOPBACK Cached FAP2HOST Q INFO:\n"
             " Swq =%p qStart=%p qEnd=%p msgSize=%d dqm=%d fapId=%d\n",
             gsoLoopBkF2HQInfo.swq, gsoLoopBkF2HQInfo.qStart, gsoLoopBkF2HQInfo.qEnd,
             gsoLoopBkF2HQInfo.msgSize, gsoLoopBkF2HQInfo.dqm, gsoLoopBkF2HQInfo.fapId);    

     /* initialize the hook */
     bcm_gso_loopback_hw_offload = fapGsoLoopBk_offload;  

    return FAP_SUCCESS;
}

#endif /* CONFIG_BCM_FAP_GSO_LOOPBACK */
