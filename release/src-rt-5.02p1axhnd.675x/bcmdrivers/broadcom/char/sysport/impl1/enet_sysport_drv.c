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


//**************************************************************************
// File Name  : bcmenet_sysport_drv.c
//
// Description: This is Linux network driver for Broadcom System Port Ethernet controller
//
//**************************************************************************

#define VERSION     "0.1"
#define VER_STR     "v" VERSION

#if 1
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/init.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/mii.h>
#include <linux/skbuff.h>
#include <linux/kthread.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/kmod.h>
#include <linux/rtnetlink.h>
#include "linux/if_bridge.h"
#include <net/arp.h>
#include <board.h>
#include <bcm_intr.h>
#include <linux/bcm_realtime.h>
#include <linux/stddef.h>
#include <asm/atomic.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/nbuff.h>
#include <net/sch_generic.h>
#include <bcm_mm.h>

#include <net/net_namespace.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
#include <linux/module.h>
#endif
#include <linux/version.h>

#include "bcm_map_part.h"

#if defined(CONFIG_BCM_BPM)
#include <linux/gbpm.h>
#include "bpm.h"
#else
#error "bpm not defined"
#endif

#include <bcm/bcmswapitypes.h>
#include "bcmmii.h"
#include "bcmnet.h"
#include "bcmmii_xtn.h"
#include "enet_sysport_drv.h"
#endif 

#if defined(PKTC)
#include <osl.h>
#endif /* PKTC */

#define TRACE printk

#define BCM_ENET_TX_DEBUG(fmt, arg...)  printk(fmt, ##arg)
#define BCM_ENET_RX_DEBUG(fmt, arg...)  printk(fmt, ##arg)
#define BCM_ENET_DEBUG(fmt, arg...)  

#define ETH_CRC_LEN             4
#define ENET_POLL_DONE        0x80000000
#define ETH_MULTICAST_BIT       0x01

#ifndef ERROR
#define ERROR(x)        printk x
#endif
#ifndef ASSERT
#define ASSERT(x)       if (x); else ERROR(("assert: "__FILE__" line %d\n", __LINE__)); 
#endif

#ifdef CONFIG_BLOG
extern int bcm_tcp_v4_recv(pNBuff_t pNBuff, BlogFcArgs_t *fc_args);
#endif

static inline int enet_sysport_flush_free_rx_buffer(uint8 * pData, uint8 * pEnd);

#define ENET_SYSPORT_TX_LOCK()      spin_lock_bh(&g_pDevCtrl->ethlock_tx)
#define ENET_SYSPORT_TX_UNLOCK()    spin_unlock_bh(&g_pDevCtrl->ethlock_tx)
#define ENET_SYSPORT_RX_LOCK()      spin_lock_bh(&g_pDevCtrl->ethlock_rx)
#define ENET_SYSPORT_RX_UNLOCK()    spin_unlock_bh(&g_pDevCtrl->ethlock_rx)
#define ENET_SYSPORT_SW_LOCK()      spin_lock_bh(&g_pDevCtrl->bcm_extsw_access)
#define ENET_SYSPORT_SW_UNLOCK()    spin_unlock_bh(&g_pDevCtrl->bcm_extsw_access)

static BcmEnet_sysport_devctrl *g_pDevCtrl = NULL;          /* Only one device - keep global for now */
static enet_tx_recycle_ctxt  g_pTxRecycle[ENET_NUM_TX_PKT_DESC]; /* Global storage for TX recycle info */
static pkt_desc_info *g_pTxPktDesc = NULL;                  /* Global storage for TX PktDesc Info */
static pkt_desc_info *g_pRxPktDesc = NULL;                  /* Global storage for RX PktDesc Info */
static enet_sysport_skb_pool *g_pSkbPool;

static inline void dumpHexData1(uint8_t *pHead, uint32_t len)
{
    uint32_t i;
    uint8_t *c = pHead;
    for (i = 0; i < len; ++i) {
        if (i % 16 == 0)
            printk("\n");
        printk("0x%02X, ", *c++);
    }
    printk("\n");
}

static inline void dump_pkt(const char *pStr, uint8_t * pBuf, uint32_t len)
{
    if (0)
    {
        //int dump_len = ( len < 64) ? len : 64;
        int dump_len = len ;
        printk("%s: data<0x%p> len<%u>",pStr, pBuf, len);
        dumpHexData1(pBuf, dump_len);
        cache_flush_len((void*)pBuf, dump_len);
    }
}

unsigned short bcm_type_trans(struct sk_buff *skb, struct net_device *dev)
{
    struct ethhdr *eth;
    unsigned char *rawp;
    unsigned int hdrlen = sizeof(struct ethhdr);

    skb_reset_mac_header(skb);

    skb_pull(skb, hdrlen);
    eth = (struct ethhdr *)skb_mac_header(skb);

    if(*eth->h_dest&1)
    {
        if(memcmp(eth->h_dest,dev->broadcast, ETH_ALEN)==0)
            skb->pkt_type=PACKET_BROADCAST;
        else
            skb->pkt_type=PACKET_MULTICAST;
    }

    /*
     *  This ALLMULTI check should be redundant by 1.4
     *  so don't forget to remove it.
     *
     *  Seems, you forgot to remove it. All silly devices
     *  seems to set IFF_PROMISC.
     */

    else if(1 /*dev->flags&IFF_PROMISC*/)
    {
        if(memcmp(eth->h_dest,dev->dev_addr, ETH_ALEN))
            skb->pkt_type=PACKET_OTHERHOST;
    }

    if (ntohs(eth->h_proto) >= 1536)
        return eth->h_proto;

    rawp = skb->data;

    /*
     *  This is a magic hack to spot IPX packets. Older Novell breaks
     *  the protocol design and runs IPX over 802.3 without an 802.2 LLC
     *  layer. We look for FFFF which isn't a used 802.2 SSAP/DSAP. This
     *  won't work for fault tolerant netware but does for the rest.
     */
    if (*(unsigned short *)rawp == 0xFFFF)
        return htons(ETH_P_802_3);

    /*
     *  Real 802.2 LLC
     */
    return htons(ETH_P_802_2);
}

static inline uint8 *enet_sysport_alloc_data_buf(void)
{
    uint8 *pData;
    /* Get a buffer from BPM */
    pData = gbpm_alloc_buf();
    if (!pData)
    {
        printk("enet_sysport_refill_rx_pkt_desc_ring() - returned error\n");
    }
    return pData;
}

static inline void enet_sysport_free_data_buf(uint8 *pData)
{
    gbpm_free_buf(pData);
}


// Little Endian Assmptions here. We do not need the BE counterparts for now.
void extsw_rreg_mmap(int page, int reg, uint8 *data_out, int len)
{
#if defined(CONFIG_BCM94908)
    uint32 offset = ((page << 8) + reg) << 2;
#else
    uint32 offset = ((page << 8) + reg) << 3;
#endif
    uint32 val;
    uint64 data64 = 0;
    void *data = &data64;

    volatile uint32 *base = (volatile uint32 *) (SWITCH_BASE + offset);
    BCM_ENET_DEBUG("%s: Read from %8p len %d page 0x%x reg 0x%x\n", __FUNCTION__,
                   base, len, page, reg);

    ENET_SYSPORT_SW_LOCK();
    val = *base;

    BCM_ENET_DEBUG(" Read from %8p len %d page 0x%x reg 0x%x val 0x%x\n",
            base, len, page, reg, (unsigned int)val);

    switch (len) {
        case 1:
            *(uint32 *)data = (uint8)val;
            break;
        case 2:
            *(uint32 *)data = (uint16)val;
            break;
        case 4:
            *(uint32 *)data = val;
            break;
        default:
            printk("%s: len = %d NOT Handled !! \n", __FUNCTION__, len);
            break;
    }
    ENET_SYSPORT_SW_UNLOCK();
    memcpy(data_out, data, len);
    BCM_ENET_DEBUG(" Read from %8p len %d page 0x%x reg 0x%x data 0x%llx\n",
            base, len, page, reg, *(uint64 *)data);
}

void extsw_wreg_mmap(int page, int reg, uint8 *data_in, int len)
{
#if defined(CONFIG_BCM94908)
    uint32 offset = ((page << 8) + reg) << 2;
#else
    uint32 offset = ((page << 8) + reg) << 3;
#endif
    uint32 val = 0;
    uint64 data64;
    void *data = &data64;
    // Add ASSERTs if address is not aligned for the requested len.

    volatile uint32 *base = (volatile uint32 *) (SWITCH_BASE + offset);
    memcpy(data, data_in, len);
    BCM_ENET_DEBUG("%s: Write  %llx to %8p len %d \n", __FUNCTION__, data64, base, len);

    ENET_SYSPORT_SW_LOCK();
    switch (len) {
        case 1:
            val = *(uint8 *)data;
            break;
        case 2:
            val = *(uint16 *)data;
            break;
        case 4:
            val = *(uint32 *)data;
            break;
        default:
            ENET_SYSPORT_SW_UNLOCK();
            printk("%s: len = %d NOT Handled !! \n", __FUNCTION__, len);
            return;
    }
    *base = val;
    ENET_SYSPORT_SW_UNLOCK();
}

/* --------------------------------------------------------------------------
Name: enet_sysport_timeout
Purpose:
-------------------------------------------------------------------------- */
static void enet_sysport_timeout(struct net_device * dev)
{
    ASSERT(dev != NULL);
    TRACE("%s: enet_sysport_timeout\n", dev->name);

    dev->trans_start = jiffies;
    netif_wake_queue(dev);
}

/* Reads the stats from SysPort Regs */
static void syspport_hw_stats( struct net_device_stats *stats)
{
#if !defined(PKT_DESC_IUDMA)

    volatile sys_port_mib *e = SYSPORT_MIB;

    stats->rx_packets = e->RxPkts;
    stats->rx_bytes = (unsigned long) e->RxOctetsLo;
    stats->multicast = e->RxMulticastPkts;
    stats->rx_broadcast_packets = e->RxBroadcastPkts;		
    stats->rx_dropped = (unsigned long) (e->RxPkts - e->RxGoodPkts);
    stats->rx_errors = (unsigned long) 
        (e->RxFCSErrs + e->RxAlignErrs + e->RxSymbolError);
        
    stats->tx_packets = (unsigned long) e->TxPkts;
    stats->tx_bytes = (unsigned long) e->TxOctetsLo;
    stats->tx_multicast_packets = e->TxMulticastPkts;
    stats->tx_broadcast_packets = e->TxBroadcastPkts;		
    stats->tx_dropped = (unsigned long) (e->TxPkts - e->TxGoodPkts);

#else
    gmac_hw_stats(stats);
#endif
}

/* --------------------------------------------------------------------------
Name: enet_sysport_get_dev_stats
Purpose: Return the current statistics. This may be called with the card
open or closed.
-------------------------------------------------------------------------- */
static struct net_device_stats * enet_sysport_get_dev_stats(struct net_device * dev)
{
    syspport_hw_stats( &(((BcmEnet_sysport_devctrl *)netdev_priv(dev))->stats));

    return &(((BcmEnet_sysport_devctrl *)netdev_priv(dev))->stats);
}

static int enet_sysport_change_mtu(struct net_device *dev, int new_mtu)
{
    int max_mtu = ENET_MAX_MTU_PAYLOAD_SIZE;

    if (new_mtu < ETH_ZLEN || new_mtu > max_mtu)
        return -EINVAL;
    dev->mtu = new_mtu;

    return 0;
}

#if defined(PKT_DESC_IUDMA)
static inline int enet_sysport_pkt_avail_to_reclaim(void)
{
    volatile PktDesc    *p_pktDesc;
    /* Check the status of first/head PktDesc */
    p_pktDesc = (volatile PktDesc *)((uintptr_t)g_pTxPktDesc->desc_ptr.p_desc_host_addr + (g_pTxPktDesc->head_idx * sizeof(PktDesc)));
    return (!(p_pktDesc->status & PD_STS_OWN)); /* SW sets it and HW resets it */
}
static inline void enet_sysport_pkt_reclaimed(void)
{
    /* Not needed */
}
#else

/* TBD - We should do bulk reclaim by reading the register once */
static inline int enet_sysport_pkt_avail_to_reclaim(void)
{
    /* Read sysport consumed index */
    uint32_t hw_c_index = SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX;
    hw_c_index = (hw_c_index & SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX_CONSUMER_INDEX_M) >> SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX_CONSUMER_INDEX_S;
    return (hw_c_index != g_pTxPktDesc->tx_c_index);
}
static inline void enet_sysport_pkt_reclaimed(void)
{
    g_pTxPktDesc->tx_c_index++; /* 16bit - wrap around expected */
}
#endif 

static inline void enet_sysport_reclaim_tx_pkt_desc(void)
{
    ENET_SYSPORT_TX_LOCK();

    while (g_pTxPktDesc->assigned_pkt_desc) /* Any pending assigned packet in PktDesc Ring ? */
    {
        if (enet_sysport_pkt_avail_to_reclaim())
        {
            nbuff_free((pNBuff_t)g_pTxRecycle[g_pTxPktDesc->head_idx]);
            /* Move the head index forward */
            g_pTxPktDesc->head_idx++;
            g_pTxPktDesc->head_idx &= (g_pTxPktDesc->total_pkt_desc - 1); /* Take care of wrap-around */
            /* decrease the assigned PktDesc */
            g_pTxPktDesc->assigned_pkt_desc--;

            enet_sysport_pkt_reclaimed();
        }
        else
        {
            break; /* No more PktDesc to free ; This condition should happen only if host is bursting more than sysport can transmit */
        }
    }

    ENET_SYSPORT_TX_UNLOCK();
}

#if defined(PKT_DESC_IUDMA)
static inline uint16_t enet_sysport_get_tx_pkt_desc_status(volatile PktDesc *p_pktDesc, uint32_t tail_idx)
{
    if (tail_idx == g_pTxPktDesc->total_pkt_desc - 1)
    {
       return (PD_STS_WRAP | PD_STS_OWN | PD_STS_SOP | PD_STS_EOP | PD_STS_APPEND_CRC);
    }
    else
    {
        return (PD_STS_OWN | PD_STS_SOP | PD_STS_EOP | PD_STS_APPEND_CRC);
    }

}
static inline void enet_sysport_enable_hw_tx(void)
{
    g_pTxPktDesc->pDmaChCfg->cfg |= DMA_ENABLE;
}
#else

#define enet_sysport_get_tx_pkt_desc_status(dummy1, dummy2) (PD_STS_SOP | PD_STS_EOP | PD_STS_APPEND_CRC)

static inline void enet_sysport_enable_hw_tx(void)
{
    /* TBD - avoid multiple register accesses */
    uint32_t hw_p_index = SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX;
    hw_p_index &= SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX_PRODUCER_INDEX_M;
    hw_p_index++;
    hw_p_index &= SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX_PRODUCER_INDEX_M;

    SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX = hw_p_index;
}
#endif

static inline int enet_sysport_xmit_pkt(uint8 *pData, uint16 data_len, void* recycle_key)
{
    volatile PktDesc    *p_pktDesc;
    uint32_t            tail_idx;
    uint16_t            status;

    ENET_SYSPORT_TX_LOCK();

    if(unlikely(g_pTxPktDesc->assigned_pkt_desc == g_pTxPktDesc->total_pkt_desc)) /* No room to transmit */
    {
        ENET_SYSPORT_TX_UNLOCK();
        return 1;
    }

    tail_idx = g_pTxPktDesc->tail_idx; /* Put the buffer in the end of queue based on SW view */
    p_pktDesc = (volatile PktDesc *)((uintptr_t)g_pTxPktDesc->desc_ptr.p_desc_host_addr + (tail_idx * sizeof(PktDesc)));

    /* Store recycle Pointer */
    g_pTxRecycle[tail_idx] = recycle_key;

    /* Move the tail index forward */
	g_pTxPktDesc->tail_idx++;
	g_pTxPktDesc->tail_idx &= (g_pTxPktDesc->total_pkt_desc - 1); /* Take care of wrap-around */
    g_pTxPktDesc->assigned_pkt_desc++;

    TX_PKT_DESC_ASSIGN_BUF_ADDR(p_pktDesc, pData);
    TX_PKT_DESC_ASSIGN_BUF_LEN(p_pktDesc, data_len);
    status = enet_sysport_get_tx_pkt_desc_status(p_pktDesc,tail_idx);
    TX_PKT_DESC_ASSIGN_STATUS(p_pktDesc, status);

    wmb();

    enet_sysport_enable_hw_tx();

    ENET_SYSPORT_TX_UNLOCK();

    return 0;
}

/* --------------------------------------------------------------------------
Name: enet_sysport_xmit
Purpose: Send ethernet traffic
-------------------------------------------------------------------------- */
static int enet_sysport_xmit(pNBuff_t pNBuff, struct net_device *dev)
{
    uint8_t *p_data; 
    uint32_t data_len;
    uint32_t skb_mark; 
    uint32_t priority; 
    uint32_t rflags;

#if defined(PKTC)
    bool is_chained = FALSE;
    pNBuff_t pNBuff_next;

    if (IS_SKBUFF_PTR(pNBuff))	{
        is_chained = PKTISCHAINED(pNBuff);
    }

    do {

        if (is_chained) {
            pNBuff_next = PKTCLINK(pNBuff);
        }
#endif /* PKTC */

    if (nbuff_get_params_ext(pNBuff, &p_data, &data_len,
                             &skb_mark, &priority, &rflags) == NULL)
    {
        printk("%s() : nbuff_get_params_ext() reutned error... Tx pkt drop\n",__FUNCTION__);
        goto drop_exit;
    }

#ifdef CONFIG_BLOG
    /*
     * Pass to blog->fcache, so it can construct the customized
     * fcache based execution stack.
     */
#if defined(PKTC)
    if (!is_chained)
#endif /* PKTC */
    blog_emit( pNBuff, dev, TYPE_ETH, 0, BLOG_ENETPHY ); /* CONFIG_BLOG */
#endif

    if ( data_len < ETH_ZLEN )
    {
        nbuff_pad(pNBuff, ETH_ZLEN - data_len);
        data_len = ETH_ZLEN;
    }

    /* First reclaim already transmitted buffers */
    enet_sysport_reclaim_tx_pkt_desc();

    dump_pkt("TX PKT: ", p_data, data_len);

    nbuff_flush(pNBuff, p_data, data_len);

    if(enet_sysport_xmit_pkt(p_data, data_len, pNBuff))
    {
        printk("%s() : bcmeapi_pkt_xmt_dispatch() reutned error... Tx pkt drop\n",__FUNCTION__);
        goto drop_exit;
    }

    g_pDevCtrl->stats.tx_bytes += data_len + ETH_CRC_LEN;
    g_pDevCtrl->stats.tx_packets++;
    g_pDevCtrl->dev->trans_start = jiffies;

#if defined(PKTC)
        if (is_chained) {
            pNBuff = pNBuff_next;
        }
    }while (pNBuff && is_chained && IS_SKBUFF_PTR(pNBuff));
#endif /* PKTC */

    return 0;

drop_exit:
    g_pDevCtrl->stats.tx_dropped++;
    nbuff_flushfree(pNBuff);
    return 0;

}

#if defined(PKT_DESC_IUDMA)
static inline void enet_sysport_enable_rx_intr(void)
{
    /* Enable the interrupts from RX DMA channels */
    ENET_SYSPORT_RX_LOCK();
    g_pRxPktDesc->pDmaChCfg->intStat = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
    g_pRxPktDesc->pDmaChCfg->intMask = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
    ENET_SYSPORT_RX_UNLOCK();
}
#else
static inline void enet_sysport_enable_rx_intr(void)
{
    /* Enable the sysport CPU L2-Interrupt controller 0 */
    ENET_SYSPORT_RX_LOCK();
    SYSPORT_INTR0->SYSTEMPORT_INTR_CPU_MASK_CLEAR = SYSPORT_INTR_RDMA_MULTI_BUFFER_DONE_M;
    ENET_SYSPORT_RX_UNLOCK();
}
#endif


static inline int enet_sysport_free_skb(struct sk_buff *skb, int free_flag)
{
    if( !(free_flag & SKB_RECYCLE )) /* No SKB recycling needed */
    {
        return 1;
    }

    preempt_disable();
    ENET_SYSPORT_RX_LOCK();
    /* Check if from dynamic skb allocation */
    if ((unsigned char *)skb < g_pSkbPool->start_skbs_p || (unsigned char *)skb >= g_pSkbPool->end_skbs_p)
    {
        kmem_cache_free(g_pSkbPool->enetSkbCache, skb);
    }
    else
    {
        skb->next_free = g_pSkbPool->freeSkbList;
        g_pSkbPool->freeSkbList = skb;      
    }

    ENET_SYSPORT_RX_UNLOCK();
    preempt_enable();
    return 0;
}


static inline void enet_sysport_recycle_skb_or_data(struct sk_buff *skb,
                                                    uintptr_t context, uint32 free_flag)
{
    if (enet_sysport_free_skb(skb, free_flag))
    { // free data
        uint8 *pData = skb->head + BCM_PKT_HEADROOM;
        uint8 *pEnd  = pData + BCM_MAX_PKT_LEN;
        enet_sysport_flush_free_rx_buffer(pData, pEnd);
    }
}

static inline int enet_sysport_skb_headerinit(int len, struct sk_buff *skb, 
                                         FkBuff_t * pFkb, unsigned char *pBuf)
{
    uintptr_t recycle_context = 0;

    skb_headerinit(BCM_PKT_HEADROOM,
                   BCM_MAX_PKT_LEN,
                   skb, pBuf, (RecycleFuncP)enet_sysport_recycle_skb_or_data,
                   recycle_context,(void*)pFkb->blog_p);

    skb_trim(skb, len - ETH_CRC_LEN);

    return 0;
}

static inline struct sk_buff *enet_sysport_alloc_skb(void)
{
    struct sk_buff *skb = NULL;

    ENET_SYSPORT_RX_LOCK();

    /* First allocate from local cache/pool */
	if (g_pSkbPool->freeSkbList) {
		skb = g_pSkbPool->freeSkbList;
		g_pSkbPool->freeSkbList = skb->next_free;
        skb->next_free = NULL;
	}
	else { /* local cache is empty - get from dynamic pool */
		skb = kmem_cache_alloc(g_pSkbPool->enetSkbCache, GFP_ATOMIC);
	}

    ENET_SYSPORT_RX_UNLOCK();

	return skb;
}

#if defined(PKT_DESC_IUDMA)
static inline void enet_sysport_assign_pkt_desc_to_hw(volatile PktDesc *p_pktDesc, uint32_t tail_idx)
{
    if (tail_idx == g_pRxPktDesc->total_pkt_desc - 1)
    {
        p_pktDesc->status = PD_STS_OWN | PD_STS_WRAP;
    }
    else
    {
        p_pktDesc->status = PD_STS_OWN;
    }
}
#else
static inline void enet_sysport_assign_pkt_desc_to_hw(volatile PktDesc *p_pktDesc, uint32_t tail_idx)
{
    uint32_t hw_c_index = (SYSPORT_RDMA->SYSTEMPORT_RDMA_CONSUMER_INDEX & SYSPORT_RDMA_CONSUMER_INDEX_CONS_IDX_M);
    hw_c_index++; /* C-Index is 16-bit value but the Hi-16bit of register are read-only, so it is ok to increment, it will wrap-around in HW */
    SYSPORT_RDMA->SYSTEMPORT_RDMA_CONSUMER_INDEX = hw_c_index; 
}
#endif
static inline int enet_sysport_assign_buffer_to_pkt_desc(uint8 *pData)
{
    volatile PktDesc    *p_pktDesc;
    uint32_t            tail_idx;

    tail_idx = g_pRxPktDesc->tail_idx; /* Put the buffer in the end of queue based on SW view */
    p_pktDesc = (volatile PktDesc *)((uintptr_t)g_pRxPktDesc->desc_ptr.p_desc_host_addr + (tail_idx * sizeof(PktDesc)));

    RX_PKT_DESC_ASSIGN_BUF_ADDR(p_pktDesc, pData);
    RX_PKT_DESC_ASSIGN_BUF_LEN(p_pktDesc, BCM_MAX_PKT_LEN);

    /* Move the tail index forward */
	g_pRxPktDesc->tail_idx++;
	g_pRxPktDesc->tail_idx &= (g_pRxPktDesc->total_pkt_desc - 1); /* Take care of wrap-around */

    g_pRxPktDesc->assigned_pkt_desc++;

    /* During ring initialization - ok to increment consumer index; It will get initialized during RDMA init */
    enet_sysport_assign_pkt_desc_to_hw(p_pktDesc, tail_idx);

    return 0;
}

static inline int enet_sysport_refill_rx_pkt_desc_ring(void)
{
    uint8 *pData;

    pData = enet_sysport_alloc_data_buf();

    if( !pData )  return -ENOMEM;

    if( enet_sysport_assign_buffer_to_pkt_desc(pData) )
    {
        enet_sysport_free_data_buf(pData);
        printk("enet_sysport_assign_buffer_to_rx_pkt_desc_ring() - returned error\n");
        return -ENOMEM;
    }
    return 0;
}
static inline int enet_sysport_free_rx_buffer(uint8 * pData)
{
    int err = 0; 
    preempt_disable();

    ENET_SYSPORT_RX_LOCK();

    if (g_pRxPktDesc->total_pkt_desc == g_pRxPktDesc->assigned_pkt_desc) /* Ring is full; Release the buffer to BPM */
    {
        enet_sysport_free_data_buf(pData);
    }
    else /* Free the buffer to BD ring */
    {
        err = enet_sysport_assign_buffer_to_pkt_desc(pData);
    }

    ENET_SYSPORT_RX_UNLOCK();
    preempt_enable();

    return err;
}

static inline int enet_sysport_flush_free_rx_buffer(uint8 * pData, uint8 * pEnd)
{
    cache_flush_region(pData, pEnd);
    return enet_sysport_free_rx_buffer(pData);
}

/* Callback: fkb and data recycling */
static inline void enet_sysport_recycle_fkb(struct fkbuff * pFkb, uint32 context)
{
    uint8 *pData = PFKBUFF_TO_PDATA(pFkb,BCM_PKT_HEADROOM);

    enet_sysport_free_rx_buffer(pData); /* No cache flush */
}

/* Common recycle callback for fkb, skb or data */
static inline void enet_sysport_recycle_nbuff(pNBuff_t pNBuff, uint32 context, uint32 flags)
{
    if ( IS_FKBUFF_PTR(pNBuff) ) {
        enet_sysport_recycle_fkb(PNBUFF_2_FKBUFF(pNBuff), context);
    } else { /* IS_SKBUFF_PTR(pNBuff) */
        enet_sysport_recycle_skb_or_data(PNBUFF_2_SKBUFF(pNBuff),context,flags);
    }
}

#if defined(PKT_DESC_IUDMA)
static inline int enet_sysport_rx_pkt_ready(volatile PktDesc **pp_pktDesc)
{
    *pp_pktDesc = (volatile PktDesc *)((uintptr_t)g_pRxPktDesc->desc_ptr.p_desc_host_addr + (g_pRxPktDesc->head_idx * sizeof(PktDesc)));
    return (!((*pp_pktDesc)->status & PD_STS_OWN)); /* SW sets it and HW resets it; If not set, pkt is available */
}
static inline void enet_sysport_rx_pkt_rcvd(void)
{
    /* Nothing to do */
}
#else
static inline int enet_sysport_rx_pkt_ready(volatile PktDesc **pp_pktDesc)
{
    uint32_t hw_p_index;

    /* Read the register that contains both Consumer & Producer Index - to avoid reading two registers */
    hw_p_index = SYSPORT_RDMA->SYSTEMPORT_RDMA_PRODUCER_INDEX & SYSPORT_RDMA_PRODUCER_INDEX_PROD_IDX_M;

    return (g_pRxPktDesc->rx_c_index != hw_p_index);
}
static inline void enet_sysport_rx_pkt_rcvd(void)
{
    g_pRxPktDesc->rx_c_index++;
}
#endif

static inline int enet_sysport_rx_pkt(uint8_t **p_pkt, int *p_len, uint32_t *p_status)
{
    volatile PktDesc *p_pktDesc;

    ENET_SYSPORT_RX_LOCK();

    if (unlikely(!g_pRxPktDesc->assigned_pkt_desc))
    {
        ENET_SYSPORT_RX_UNLOCK();
        return 0;
    }
    if (enet_sysport_rx_pkt_ready(&p_pktDesc)) 
    {
        p_pktDesc = (volatile PktDesc *)((uintptr_t)g_pRxPktDesc->desc_ptr.p_desc_host_addr + (g_pRxPktDesc->head_idx * sizeof(PktDesc)));
        *p_pkt = (uint8_t *)(phys_to_virt(p_pktDesc->address));
        *p_len = p_pktDesc->length; 
        *p_status = p_pktDesc->status;
        /* Move the head index forward */
        g_pRxPktDesc->head_idx++;
        g_pRxPktDesc->head_idx &= (g_pRxPktDesc->total_pkt_desc - 1); /* Take care of wrap-around */
        /* decrease the assigned BDs */
        g_pRxPktDesc->assigned_pkt_desc--;

        enet_sysport_rx_pkt_rcvd();

        enet_sysport_refill_rx_pkt_desc_ring(); /* Nothing to do with returned error */

        ENET_SYSPORT_RX_UNLOCK();
        return 1;
    }
    ENET_SYSPORT_RX_UNLOCK();
    return 0;
}

/*
 *  enet_sysport_rx_pkts: Process all received packets.
 */
static uint32 enet_sysport_rx_pkts(uint32 budget)
{
    uint32_t desc_status = 0;

    struct net_device *dev = NULL;
    unsigned char *pBuf = NULL;
    struct sk_buff *skb = NULL;
    int len=0;
    FkBuff_t * pFkb = NULL;
#if defined(CONFIG_BLOG)
    BlogAction_t blogAction;
    BlogFcArgs_t fc_args;
#endif
    for(; --budget > 0; )
    {
        if (!enet_sysport_rx_pkt(&pBuf, &len, &desc_status)) 
        {
            return ENET_POLL_DONE;
        }

        /* No scatter-gather and any pkt error */
        if ( !(desc_status & PD_STS_SOP) || !(desc_status & PD_STS_EOP) ||
             (desc_status & PD_STS_RX_PKT_ERR) ||
             (len < ENET_MIN_MTU_SIZE)) 
        {
            /* We haven't brought the packet in cache, release */
            enet_sysport_free_rx_buffer(pBuf);
            g_pDevCtrl->stats.rx_dropped++;
            continue;
        }

        cache_invalidate_len(pBuf, BCM_MAX_PKT_LEN);

        dump_pkt("RX PKT: ", pBuf, len );

        dev = g_pDevCtrl->dev;

        /* Store packet & byte count in switch structure */
        g_pDevCtrl->stats.rx_packets++;
        g_pDevCtrl->stats.rx_bytes += len;


        /* FkBuff_t<data,len> in-placed leaving headroom */
        pFkb = fkb_init(pBuf, BCM_PKT_HEADROOM,
                        pBuf, len - ETH_CRC_LEN );
        {
            pFkb->recycle_hook = (RecycleFuncP)enet_sysport_recycle_nbuff;
            pFkb->recycle_context = 0;
        }


#ifdef CONFIG_BLOG
        blogAction = blog_finit_args( pFkb, dev, TYPE_ETH, 0, BLOG_ENETPHY, &fc_args);
        if ( blogAction == PKT_DROP )
        {
            enet_sysport_flush_free_rx_buffer((uint8*)pFkb, pBuf);

            /* Store dropped packet count in our portion of the device structure */
            g_pDevCtrl->stats.rx_dropped++;
            continue;
        }

        /* packet consumed, proceed to next packet*/
        if ( blogAction == PKT_DONE )
        {
            continue;
        }
        else if ( blogAction == PKT_TCP4_LOCAL)
        {
            bcm_tcp_v4_recv((void*)CAST_REAL_TO_VIRT_PNBUFF(pFkb,FKBUFF_PTR) , &fc_args);
            continue;
        }

#endif /* CONFIG_BLOG */

        /*allocate skb & initialize it using fkb */
        skb = enet_sysport_alloc_skb();
        if (!skb) {
            fkb_release(pFkb);
            g_pDevCtrl->stats.rx_dropped++;
            enet_sysport_flush_free_rx_buffer((uint8*)pFkb, pBuf);
            continue;
        }

        if(enet_sysport_skb_headerinit(len, skb, pFkb, pBuf))
        {
            enet_sysport_flush_free_rx_buffer((uint8*)pFkb, pBuf);
            continue;
        }
        /* TBD : If there is no BRCM TAG - better to use eth_type_trans() */
        skb->protocol = bcm_type_trans(skb, dev); 
        skb->dev = dev;
        
        local_bh_disable();
        netif_receive_skb(skb);
        local_bh_enable();

    } /* end while (budget > 0) */

    g_pDevCtrl->dev->last_rx = jiffies;

    return 0;
}

static int bcm63xx_enet_rx_thread(void *arg)
{
    uint32 work_done;
    int budget = 32;


    while (1)
    {
        wait_event_interruptible(g_pDevCtrl->rx_thread_wqh,
                                 g_pDevCtrl->rx_work_avail);

        if (kthread_should_stop())
        {
            printk(KERN_INFO "kthread_should_stop detected on bcmsw-rx\n");
            break;
        }

        work_done = enet_sysport_rx_pkts(budget);

        //BCM_ENET_RX_DEBUG("Work Done: %d \n", (int)work_done);

        if (work_done & ENET_POLL_DONE)
        {
            /*
             * No more packets.  Indicate we are done (rx_work_avail=0) and
             * re-enable interrupts (bcmeapi_napi_post) and go to top of
             * loop to wait for more work.
             */
            g_pDevCtrl->rx_work_avail = 0;
            enet_sysport_enable_rx_intr();
        }
        else
        {
            /* We have either exhausted our budget or there are
               more packets on the DMA (or both).  Yield CPU to allow
               others to have a chance, then continue to top of loop for more
               work.  */
            if (current->policy == SCHED_FIFO || current->policy == SCHED_RR)
                yield();
        }
    }

    return 0;
}

/*
 * Set the hardware MAC address.
 */
static int enet_sysport_set_mac_addr(struct net_device *dev, void *p)
{
    struct sockaddr *addr = p;

    if(netif_running(dev))
        return -EBUSY;

    memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);
    return 0;
}

#if defined(PKT_DESC_IUDMA)
static inline void enet_sysport_disable_rx_intr(void)
{
    g_pRxPktDesc->pDmaChCfg->intMask = 0;

    g_pRxPktDesc->pDmaChCfg->intStat = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;  // clr interrupts
}
#else
static inline void enet_sysport_disable_rx_intr(void)
{
    /* disable RX interrupts i.e. mask the interrupt */
    SYSPORT_INTR0->SYSTEMPORT_INTR_CPU_CLEAR = SYSPORT_INTR_RDMA_MULTI_BUFFER_DONE_M;
    SYSPORT_INTR0->SYSTEMPORT_INTR_CPU_MASK_SET = SYSPORT_INTR_RDMA_MULTI_BUFFER_DONE_M;
}
#endif

#define BCMENET_WAKEUP_RXWORKER(x) do { \
           if ((x)->rx_work_avail == 0) { \
               (x)->rx_work_avail = 1; \
               wake_up_interruptible(&((x)->rx_thread_wqh)); }} while (0)

static FN_HANDLER_RT bcmeapi_enet_isr(int irq, void * pContext)
{
    /* TBD - Disable the interrupts */
    enet_sysport_disable_rx_intr();

    BCMENET_WAKEUP_RXWORKER(g_pDevCtrl);

    return BCM_IRQ_HANDLED;
}


#define ETHERNET_DEVICE_NAME     "eth0"
static atomic_t poll_lock = ATOMIC_INIT(1);
static DECLARE_COMPLETION(poll_done);

static int bcm63xx_enet_poll_timer(void * arg)
{
    set_current_state(TASK_INTERRUPTIBLE);
    /* Sleep for 1 tick) */
    schedule_timeout(HZ/100);
    /* */
    while (atomic_read(&poll_lock) > 0)
    {
        enet_sysport_reclaim_tx_pkt_desc();

        /*   */
        set_current_state(TASK_INTERRUPTIBLE);

        /* Sleep for HZ jiffies (1sec), minus the time that was already */
        /* spent waiting for EPHY PLL  */
        schedule_timeout(HZ);
    }

    complete_and_exit(&poll_done, 0);
    printk("bcm63xx_enet_poll_timer: thread exits!\n");

    return 0;
}

static int poll_pid = -1;

static int __init enet_sysport_set_rx_interrupt(void)
{
#if defined(PKT_DESC_IUDMA)
    int rxIrq = INTERRUPT_ID_GMAC_DMA_0;
#else
    int rxIrq = INTERRUPT_ID_SYSTEMPORT_INTR0;
#endif

    BcmHalMapInterrupt(bcmeapi_enet_isr,(void*)g_pDevCtrl, rxIrq);

    printk("ENET map Rx Interrupt ID = %d\n",rxIrq);

    return 0;
}

void display_software_stats(void)
{

    printk("\n\nSoftware Stats\n\n");
    printk("TxPkts:       %10lu \n", g_pDevCtrl->stats.tx_packets);
    printk("TxOctets:     %10lu \n", g_pDevCtrl->stats.tx_bytes);
    printk("TxDropPkts:   %10lu \n", g_pDevCtrl->stats.tx_dropped);
    printk("\n");
    printk("RxPkts:       %10lu \n", g_pDevCtrl->stats.rx_packets);
    printk("RxOctets:     %10lu \n", g_pDevCtrl->stats.rx_bytes);
    printk("RxDropPkts:   %10lu \n", g_pDevCtrl->stats.rx_dropped);
    printk("\n");

    //display_enet_stats(g_pDevCtrl);
}

int enet_sysport_switch_mib_dump(int port, int type)
{
    unsigned int v32, errcnt;

    /* Display Tx statistics */
    printk("External Switch Stats : Port# %d\n",port);
    extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXUPKTS, (void*)&v32, 4);  // Get TX unicast packet count
    printk("TxUnicastPkts:          %10u \n", v32);
    extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXMPKTS, (void*)&v32, 4);  // Get TX multicast packet count
    printk("TxMulticastPkts:        %10u \n",  v32);
    extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXBPKTS, (void*)&v32, 4);  // Get TX broadcast packet count
    printk("TxBroadcastPkts:        %10u \n", v32);
    extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXDROPS, (void*)&v32, 4);
    printk("TxDropPkts:             %10u \n", v32);

    if (type)
    {
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TX64OCTPKTS, (void*)&v32, 4);
        printk("TxPkts64Octets:         %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TX127OCTPKTS, (void*)&v32, 4);
        printk("TxPkts65to127Octets:    %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TX255OCTPKTS, (void*)&v32, 4);
        printk("TxPkts128to255Octets:   %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TX511OCTPKTS, (void*)&v32, 4);
        printk("TxPkts256to511Octets:   %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TX1023OCTPKTS, (void*)&v32, 4);
        printk("TxPkts512to1023Octets:  %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXMAXOCTPKTS, (void*)&v32, 4);
        printk("TxPkts1024OrMoreOctets: %10u \n", v32);
//
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ0PKT, (void*)&v32, 4);
        printk("TxQ0Pkts:               %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ1PKT, (void*)&v32, 4);
        printk("TxQ1Pkts:               %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ2PKT, (void*)&v32, 4);
        printk("TxQ2Pkts:               %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ3PKT, (void*)&v32, 4);
        printk("TxQ3Pkts:               %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ4PKT, (void*)&v32, 4);
        printk("TxQ4Pkts:               %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ5PKT, (void*)&v32, 4);
        printk("TxQ5Pkts:               %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ6PKT, (void*)&v32, 4);
        printk("TxQ6Pkts:               %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXQ7PKT, (void*)&v32, 4);
        printk("TxQ7Pkts:               %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXCOL, (void*)&v32, 4);
        printk("TxCol:                  %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXSINGLECOL, (void*)&v32, 4);
        printk("TxSingleCol:            %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXMULTICOL, (void*)&v32, 4);
        printk("TxMultipleCol:          %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXDEFERREDTX, (void*)&v32, 4);
        printk("TxDeferredTx:           %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXLATECOL, (void*)&v32, 4);
        printk("TxLateCol:              %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXEXCESSCOL, (void*)&v32, 4);
        printk("TxExcessiveCol:         %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXFRAMEINDISC, (void*)&v32, 4);
        printk("TxFrameInDisc:          %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXPAUSEPKTS, (void*)&v32, 4);
        printk("TxPausePkts:            %10u \n", v32);
    }
    else
    {
        errcnt=0;
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXCOL, (void*)&v32, 4);
        errcnt += v32;
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXSINGLECOL, (void*)&v32, 4);
        errcnt += v32;
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXMULTICOL, (void*)&v32, 4);
        errcnt += v32;
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXDEFERREDTX, (void*)&v32, 4);
        errcnt += v32;
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXLATECOL, (void*)&v32, 4);
        errcnt += v32;
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXEXCESSCOL, (void*)&v32, 4);
        errcnt += v32;
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_TXFRAMEINDISC, (void*)&v32, 4);
        errcnt += v32;
        printk("TxOtherErrors:          %10u \n", errcnt);
    }

    /* Display Rx statistics */
    extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXUPKTS, (void*)&v32, 4);  // Get RX unicast packet count
    printk("RxUnicastPkts:          %10u \n", v32);
    extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXMPKTS, (void*)&v32, 4);  // Get RX multicast packet count
    printk("RxMulticastPkts:        %10u \n",v32);
    extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXBPKTS, (void*)&v32, 4);  // Get RX broadcast packet count
    printk("RxBroadcastPkts:        %10u \n",v32);
    extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXDROPS, (void*)&v32, 4);
    printk("RxDropPkts:             %10u \n",v32);
    extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXDISCARD, (void*)&v32, 4);
    printk("RxDiscard:              %10u \n", v32);

    if (type)
    {
        printk("RxGoodOctetsHi:         %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXJABBERS, (void*)&v32, 4);
        printk("RxJabbers:              %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXALIGNERRORS, (void*)&v32, 4);
        printk("RxAlignErrs:            %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXFCSERRORS, (void*)&v32, 4);
        printk("RxFCSErrs:              %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXFRAGMENTS, (void*)&v32, 4);
        printk("RxFragments:            %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXOVERSIZE, (void*)&v32, 4);
        printk("RxOversizePkts:         %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXUNDERSIZEPKTS, (void*)&v32, 4);
        printk("RxUndersizePkts:        %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXPAUSEPKTS, (void*)&v32, 4);
        printk("RxPausePkts:            %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXSACHANGES, (void*)&v32, 4);
        printk("RxSAChanges:            %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXSYMBOLERRORS, (void*)&v32, 4);
        printk("RxSymbolError:          %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RX64OCTPKTS, (void*)&v32, 4);
        printk("RxPkts64Octets:         %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RX127OCTPKTS, (void*)&v32, 4);
        printk("RxPkts65to127Octets:    %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RX255OCTPKTS, (void*)&v32, 4);
        printk("RxPkts128to255Octets:   %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RX511OCTPKTS, (void*)&v32, 4);
        printk("RxPkts256to511Octets:   %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RX1023OCTPKTS, (void*)&v32, 4);
        printk("RxPkts512to1023Octets:  %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXMAXOCTPKTS, (void*)&v32, 4);
        printk("RxPkts1024OrMoreOctets: %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXJUMBOPKT , (void*)&v32, 4);
        printk("RxJumboPkts:            %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXOUTRANGEERR, (void*)&v32, 4);
        printk("RxOutOfRange:           %10u \n", v32);
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXINRANGEERR, (void*)&v32, 4);
        printk("RxInRangeErr:           %10u \n", v32);
    }
    else
    {
        errcnt=0;
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXJABBERS, (void*)&v32, 4);
        errcnt += v32;
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXALIGNERRORS, (void*)&v32, 4);
        errcnt += v32;
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXFCSERRORS, (void*)&v32, 4);
        errcnt += v32;
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXFRAGMENTS, (void*)&v32, 4);
        errcnt += v32;
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXOVERSIZE, (void*)&v32, 4);
        errcnt += v32;
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXUNDERSIZEPKTS, (void*)&v32, 4);
        errcnt += v32;
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXSYMBOLERRORS, (void*)&v32, 4);
        errcnt += v32;
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXOUTRANGEERR, (void*)&v32, 4);
        errcnt += v32;
        extsw_rreg_mmap(PAGE_MIB_P0 + (port), SF2_REG_MIB_P0_RXINRANGEERR, (void*)&v32, 4);
        errcnt += v32;
        printk("RxOtherErrors:          %10u \n", errcnt);

    }

    return 0;
}

static void sysport_dump_mib(int type)
{
#if defined(PKT_DESC_IUDMA)
    gmac_dump_mib(type);
#else
    volatile sys_port_mib *e = SYSPORT_MIB;

    /* Display Tx statistics */
    printk("GMAC Port Stats\n\n");
    printk("TxUnicastPkts:          %10u \n", e->TxUnicastPkts);
    printk("TxMulticastPkts:        %10u \n",  e->TxMulticastPkts);
    printk("TxBroadcastPkts:        %10u \n", e->TxBroadcastPkts);
    printk("TxDropPkts:             %10u \n", (e->TxPkts - e->TxGoodPkts));

    /* Display remaining tx stats only if requested */
    if (type) {
        printk("TxOctetsLo:             %10u \n", e->TxOctetsLo);
        printk("TxOctetsHi:             %10u \n", 0);
        printk("TxQoSPkts:              %10u \n", e->TxGoodPkts);
        printk("TxCol:                  %10u \n", e->TxCol);
        printk("TxSingleCol:            %10u \n", e->TxSingleCol);
        printk("TxMultipleCol:          %10u \n", e->TxMultipleCol);
        printk("TxDeferredTx:           %10u \n", e->TxDeferredTx);
        printk("TxLateCol:              %10u \n", e->TxLateCol);
        printk("TxExcessiveCol:         %10u \n", e->TxExcessiveCol);
        printk("TxFrameInDisc:          %10u \n", 0);
        printk("TxPausePkts:            %10u \n", e->TxPausePkts);
        printk("TxQoSOctetsLo:          %10u \n", e->TxOctetsLo);
        printk("TxQoSOctetsHi:          %10u \n", 0);
    }

    /* Display Rx statistics */
    printk("\n");
    printk("RxUnicastPkts:          %10u \n", e->RxUnicastPkts);
    printk("RxMulticastPkts:        %10u \n", e->RxMulticastPkts);
    printk("RxBroadcastPkts:        %10u \n", e->RxBroadcastPkts);
    printk("RxDropPkts:             %10u \n", (e->RxPkts - e->RxGoodPkts));

    /* Display remaining rx stats only if requested */
    if (type) {
        printk("RxJabbers:              %10u \n", e->RxJabbers);
        printk("RxAlignErrs:            %10u \n", e->RxAlignErrs);
        printk("RxFCSErrs:              %10u \n", e->RxFCSErrs);
        //printk("RxFragments:            %10u \n", e->RxFragments);
        printk("RxOversizePkts:         %10u \n", e->RxOversizePkts);
        printk("RxExcessSizeDisc:       %10u \n", e->RxExcessSizeDisc);
        printk("RxOctetsLo:             %10u \n", e->RxOctetsLo);
        printk("RxOctetsHi:             %10u \n", 0);
        //printk("RxUndersizePkts:        %10u \n", e->RxUndersizePkts);
        printk("RxPausePkts:            %10u \n", e->RxPausePkts);
        printk("RxGoodOctetsLo:         %10u \n", e->RxOctetsLo);
        printk("RxGoodOctetsHi:         %10u \n", 0);
        printk("RxSAChanges:            %10u \n", 0);
        printk("RxSymbolError:          %10u \n", e->RxSymbolError);
        printk("RxQoSPkts:              %10u \n", e->RxGoodPkts);
        printk("RxQoSOctetsLo:          %10u \n", e->RxOctetsLo);
        printk("RxQoSOctetsHi:          %10u \n", 0);
        printk("RxPkts64Octets:         %10u \n", e->Pkts64Octets);
        printk("RxPkts65to127Octets:    %10u \n", e->Pkts65to127Octets);
        printk("RxPkts128to255Octets:   %10u \n", e->Pkts128to255Octets);
        printk("RxPkts256to511Octets:   %10u \n", e->Pkts256to511Octets);
        printk("RxPkts512to1023Octets:  %10u \n", e->Pkts512to1023Octets);
        printk("RxPkts1024to1522Octets: %10u \n", 
            (e->Pkts1024to1518Octets + e->Pkts1519to1522));
        printk("RxPkts1523to2047:       %10u \n", e->Pkts1523to2047);
        printk("RxPkts2048to4095:       %10u \n", e->Pkts2048to4095);
        printk("RxPkts4096to8191:       %10u \n", e->Pkts4096to8191);
        printk("RxPkts8192to9728:       %10u \n", 0);
    }
    return ;

#endif
}

/* DEBUG - PktDesc display functions : Start */
static inline void enet_sysport_display_pkt_desc(volatile PktDesc *p_pktDesc)
{

    printk("0x%04x %04d 0x%08lx\n",p_pktDesc->status, p_pktDesc->length, (uintptr_t)p_pktDesc->address);
}
static int enet_sysport_dump_pkt_desc_ring(pkt_desc_info *p_pktDescInfo) /* Caller must take proper locks*/
{
    uint32_t bd_num = 0;
    volatile PktDesc *p_pktDesc;
    printk("total_pkt_desc = %d assigned_pkt_desc = %d head_idx = %d tail_idx = %d\n\n",
           p_pktDescInfo->total_pkt_desc, p_pktDescInfo->assigned_pkt_desc, p_pktDescInfo->head_idx, p_pktDescInfo->tail_idx);

    for (bd_num = 0; bd_num < p_pktDescInfo->assigned_pkt_desc; bd_num++)
    {
        p_pktDesc = (volatile PktDesc *)((uintptr_t)p_pktDescInfo->desc_ptr.p_desc_host_addr + (bd_num * sizeof(PktDesc)));
        enet_sysport_display_pkt_desc(p_pktDesc);
    }
    return 0;
}
/* DEBUG - BD PktDesc functions : End */

static void enet_sysport_dump_pkt_descs(void)
{
    printk("Dump RX BD Info...\n");
    ENET_SYSPORT_RX_LOCK();
#if defined(PKT_DESC_SYSPORT) 
    printk("SYSPORT_RDMA->SYSTEMPORT_RDMA_PRODUCER_INDEX = 0x%08x\n",SYSPORT_RDMA->SYSTEMPORT_RDMA_PRODUCER_INDEX);
    printk("SYSPORT_RDMA->SYSTEMPORT_RDMA_CONSUMER_INDEX = 0x%08x\n\n",SYSPORT_RDMA->SYSTEMPORT_RDMA_CONSUMER_INDEX);
#endif
    enet_sysport_dump_pkt_desc_ring(g_pRxPktDesc);
    ENET_SYSPORT_RX_UNLOCK();
    printk("Dump TX BD Info...\n");
    ENET_SYSPORT_TX_LOCK();
#if defined(PKT_DESC_SYSPORT) 
    printk("SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX = 0x%08x\n\n",SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX);
#endif
    enet_sysport_dump_pkt_desc_ring(g_pTxPktDesc);
    ENET_SYSPORT_TX_UNLOCK();
}

static int enet_sysport_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
    char *buf, *ubuf;
    int *data=(int*)rq->ifr_data/*, cb_port*/;
    ifreq_ext_t *ifx;
    struct ethswctl_data *e=(struct ethswctl_data*)rq->ifr_data;
    int val = 0,  bufLen = 0,  portMap = 0;

    union {
        struct ethswctl_data ethswctl_data;
        struct ethctl_data ethctl_data;
        struct interface_data interface_data;
        ifreq_ext_t ifre;
    } rq_data;

    /* pointers pointing to ifr_data */
    e = (struct ethswctl_data*)&rq_data;
    ifx = (ifreq_ext_t *)&rq_data;

    ASSERT(g_pDevCtrl != NULL);

    switch (cmd)
    {
        case SIOCGLINKSTATE:
            if (dev == g_pDevCtrl->dev)
            {
                val = 1;
            }
            else
            {
                return -EFAULT;
            }
            if (copy_to_user((void*)data, (void*)&val, sizeof(int)))
                return -EFAULT;

            val = 0;
            break;

        case SIOCETHSWCTLOPS:
            if (copy_from_user(e, rq->ifr_data, sizeof(*e))) return -EFAULT;
            switch(e->op)
            {
                case ETHSWUNITPORT:
                    {
                        val = 0;
                        if (copy_to_user((void*)&e->unit, (void*)&val, sizeof(e->unit)))
                            return -EFAULT;
                        val = 0x1;
                        if (copy_to_user((void*)&e->port_map, (void*)&val, sizeof(e->port_map)))
                            return -EFAULT;
                    }
                    break;

                case ETHSWOAMIDXMAPPING:
                    {
                        val = 0;
                        if (copy_to_user((void*)&e->unit, (void*)&val, sizeof(e->unit)))
                            return -EFAULT;
                        val = 0x0;
                        if (copy_to_user((void*)&e->port, (void*)&val, sizeof(e->port)))
                            return -EFAULT;
                    }
                    break;

                case ETHSWDUMPMIB:
                    if (e->unit && ( e->port >= 0 && e->port <= 8) )
                    {
                        enet_sysport_switch_mib_dump(e->port, e->type);
                    }
                    else
                    {
                        sysport_dump_mib(e->type);
                        if (e->type > 1)
                        {
                            enet_sysport_dump_pkt_descs();
                        }
                    }

                    display_software_stats();

                    break;

                case ETHSWPORTPAUSECAPABILITY:
                    val = 0;
                    if (copy_to_user((void*)&e->val, (void*)&val, sizeof(e->val)))
                        return -EFAULT;
                    break;

                default:
                    printk("enet_sysport_ioctl() : dev=%s SIOCETHSWCTLOPS op= < %d >\n",dev->name, e->op);
                    break;
            }
            break;
        case SIOCGWANPORT:
            if (copy_from_user(e, rq->ifr_data, sizeof(*e))) return -EFAULT;
            portMap = 0;
            ubuf = e->up_len.uptr;
            bufLen = e->up_len.len;
            goto PORTMAPIOCTL;

        case SIOCIFREQ_EXT:
            if (copy_from_user(ifx, rq->ifr_data, sizeof(*ifx))) return -EFAULT;

            BCM_IOC_PTR_ZERO_EXT(ifx->stringBuf);
            ubuf = ifx->stringBuf;
            bufLen = ifx->bufLen;

            switch (ifx->opcode)
            {
                case SIOCGPORTWANONLY:
                    portMap = 0;
                    break;
                case SIOCGPORTWANPREFERRED:
                    portMap = 0;
                    break;
                case SIOCGPORTLANONLY:
                    portMap = 0x1;
                    break;
            }

PORTMAPIOCTL:   /* Common fall through code to return inteface name string based on port bit map */
            val = 0;
            if (ubuf == NULL) {
                val = -EFAULT;
                break;
            }

            buf = kmalloc(bufLen, GFP_KERNEL);
            if( buf == NULL )
            {
                printk(KERN_ERR "bcmenet:SIOCGWANPORT: kmalloc of %d bytes failed\n", bufLen);
                return -ENOMEM;
            }
            buf[0] = 'e';
            buf[1] = 't';
            buf[2] = 'h';
            buf[3] = '0';
            buf[4] = 0;

            if (portMap && copy_to_user((void*)ubuf, (void*)buf, 5))
            {
                val = -EFAULT;
            }

            kfree(buf);
            break;

        default: 
            printk("UNHANDLED !! enet_sysport_ioctl() : dev=%s cmd < 0x%x %d >\n",dev->name, cmd, cmd - SIOCDEVPRIVATE);
            break;
    }
    return 0;
}

static const struct net_device_ops bcm96xx_netdev_ops = {
    //.ndo_open         = bcm63xx_enet_open,
    //.ndo_stop         = bcm63xx_enet_close,
    .ndo_start_xmit     = (HardStartXmitFuncP)enet_sysport_xmit,
    .ndo_set_mac_address= enet_sysport_set_mac_addr,
    .ndo_do_ioctl       = enet_sysport_ioctl,
    .ndo_tx_timeout     = enet_sysport_timeout,
    .ndo_get_stats      = enet_sysport_get_dev_stats,
    .ndo_change_mtu     = enet_sysport_change_mtu
};


/* --------------------------------------------------------------------------
Name: enet_sysport_open_dev
Purpose: Kick start the network device
-------------------------------------------------------------------------- */
static int __init enet_sysport_open_dev(void)
{
    struct net_device *dev = g_pDevCtrl->dev;
    struct task_struct * bcmsw_task_struct;

    set_bit(__LINK_STATE_START, &dev->state);
	dev->flags |= IFF_UP;	/* have to mark the flag earlier */
    netif_carrier_on(dev);
    netif_start_queue(dev);
    
	dev_activate(dev);

    /* Enable RX Interrupts */
    enet_sysport_enable_rx_intr();

    bcmsw_task_struct = kthread_run(bcm63xx_enet_poll_timer, NULL, "bcmsw");
    poll_pid = bcmsw_task_struct->pid;

    return ((poll_pid < 0)? -ENOMEM: 0);
}


/*
 *      bcmenet_sysport_init_dev: - Probe Ethernet switch and allocate device
 */
int __init bcmenet_sysport_init_dev(void)
{
    static int probed = 0;
    struct net_device *dev = NULL;
    int status = 0;
    unsigned char macAddr[ETH_ALEN];

    TRACE(("bcm63xxenet: bcmenet_sysport_init_dev\n"));

    if (probed)
    {
        /* device has already been initialized */
        return -ENXIO;
    }
    probed++;
    /* Allocate a network device with private area */
    dev = alloc_etherdev(sizeof(*g_pDevCtrl));
    if (dev == NULL)
    {
        printk("ERROR: Unable to allocate net_device!\n");
        return -ENOMEM;
    }

    g_pDevCtrl = netdev_priv(dev);
    memset(g_pDevCtrl, 0, sizeof(BcmEnet_sysport_devctrl));

    g_pDevCtrl->dev = dev;
    dma_set_coherent_mask(&dev->dev, DMA_BIT_MASK(32));

    spin_lock_init(&g_pDevCtrl->ethlock_tx);
    spin_lock_init(&g_pDevCtrl->ethlock_rx);
    spin_lock_init(&g_pDevCtrl->bcm_extsw_access);

    dev_alloc_name(dev, dev->name);
    sprintf(dev->name, ETHERNET_DEVICE_NAME);

    dev->netdev_ops = &bcm96xx_netdev_ops;

    init_waitqueue_head(&g_pDevCtrl->rx_thread_wqh);
    g_pDevCtrl->rx_thread = kthread_create(bcm63xx_enet_rx_thread, g_pDevCtrl, "bcmsw_rx");
    wake_up_process(g_pDevCtrl->rx_thread);

    netdev_path_set_hw_port(dev, 0, BLOG_ENETPHY);

    dev->watchdog_timeo     = 2 * HZ;
    dev->mtu = BCM_ENET_DEFAULT_MTU_SIZE; 


    status = register_netdev(dev);

    if (status != 0)
    {
        printk("bcmenet_sysport_probe failed, returns %d\n", status);
        return status;
    }

    macAddr[0] = 0xff;
    kerSysGetMacAddress(macAddr, dev->ifindex);

    memmove(dev->dev_addr, macAddr, ETH_ALEN);
    return (0);
}

static void __exit enet_sysport_module_cleanup(void)
{
    TRACE(("bcm63xxenet: enet_sysport_module_cleanup\n"));

}
    
static void * __init enet_sysport_alloc_coherent_mem(struct device *dev, uint32_t size32, uint32 *physical_addr)
{
    dma_addr_t phys_addr;
    /* memory allocation of NONCACHE_MALLOC for ARM is aligned to page size which is aligned to cache */
    void *p = dma_alloc_coherent(dev, size32, &phys_addr, GFP_KERNEL);

    if (p != NULL) {
       memset(p, 0, size32) ;
    }

    *physical_addr = (uint32_t) phys_addr ;
    return ( p );   /* return unaligned host address */

}
static int __init enet_sysport_alloc_pkt_desc_ring(int num_pds, pkt_desc_info *p_pktDescInfo)
{
    uint32 phy_addr;
    uint32_t size32 = (uint32_t) (num_pds * sizeof(PktDesc) + BCM_DCACHE_LINE_LEN) ;

    p_pktDescInfo->desc_ptr.p_desc_unaligned_addr = 
        (volatile PktDesc *)enet_sysport_alloc_coherent_mem(&g_pDevCtrl->dev->dev, size32, &phy_addr);
    if ( p_pktDescInfo->desc_ptr.p_desc_unaligned_addr == NULL )
    {
       printk("Unable to allocate memory for Descriptors \n");
       return -ENOMEM;
    }
    /* TBD - should phy_addr be also aligned ?? */
    p_pktDescInfo->desc_ptr.p_desc_host_addr = 
        (volatile PktDesc *)(((uintptr_t)p_pktDescInfo->desc_ptr.p_desc_unaligned_addr + BCM_DCACHE_ALIGN_LEN) & ~BCM_DCACHE_ALIGN_LEN);
    p_pktDescInfo->desc_ptr.p_desc_physical_addr = (volatile PktDesc *)(uintptr_t)phy_addr;

    p_pktDescInfo->total_pkt_desc = num_pds;

    return 0;
}
static int __init enet_sysport_alloc_rx_buffers(int num_bufs)
{
    int err = 0;
    uint8_t* pData;

    preempt_disable();
    ENET_SYSPORT_RX_LOCK();
    
    while(num_bufs)
    {
        pData = enet_sysport_alloc_data_buf();

        if (!pData)
        {
            printk("%s() : error assigning buffers to PktDesc ring\n",__FUNCTION__);
            err = -1;
            break;
        }

        /* Use common function to release the data packet pointer back to BD ring */
        if (enet_sysport_assign_buffer_to_pkt_desc(pData))
        {
            printk("error assigning buffers to PktDesc ring\n");
            err = -1;
            break;
        }
        num_bufs--;
    }

    ENET_SYSPORT_RX_UNLOCK();
    preempt_enable();

    return err;
}

static int __init enet_sysport_pkt_desc_init(void)
{   
    int err; 
#if defined(PKT_DESC_IUDMA)
    volatile DmaRegs *dmaCtrl = (DmaRegs *)(GMAC_DMA_BASE);    
#endif

    g_pTxPktDesc = kzalloc(sizeof(*g_pTxPktDesc), GFP_ATOMIC);
    g_pRxPktDesc = kzalloc(sizeof(*g_pRxPktDesc), GFP_ATOMIC);

#if defined(PKT_DESC_IUDMA)
    g_pRxPktDesc->pDmaChCfg = &dmaCtrl->chcfg[IUDMA_RX_CHAN];
    g_pTxPktDesc->pDmaChCfg = &dmaCtrl->chcfg[IUDMA_TX_CHAN];
#endif

    /* Allocate RX BDs */
    err = enet_sysport_alloc_pkt_desc_ring(ENET_NUM_RX_PKT_DESC, g_pRxPktDesc);
    if (!err)
    {
        /* Allocate RX Buffers */
        err = enet_sysport_alloc_rx_buffers(ENET_NUM_RX_PKT_DESC);
    }
    if (!err)
    {
        /* Allocate TX BDs*/
        err = enet_sysport_alloc_pkt_desc_ring(ENET_NUM_TX_PKT_DESC, g_pTxPktDesc);
    }

    return err;
}

/* SKB Management */


static int __init enet_sysport_skb_cache_init(void)
{
    uint8_t *pSkbuff;
    uint32_t num_skbs;

    uint32_t skb_pool_mem_size = (ENET_NUM_RX_PKT_DESC * BCM_SKB_ALIGNED_SIZE) + BCM_DCACHE_LINE_LEN;

    if ( BCM_SKB_ALIGNED_SIZE != skb_aligned_size() )
    {
        printk("skb_aligned_size mismatch. Need to recompile enet module\n");
        return -ENOMEM;
    }

    /* Initialize the SKB pool */
    g_pSkbPool = kzalloc(sizeof(*g_pSkbPool), GFP_ATOMIC);


    g_pSkbPool->start_skbs_p = kmalloc(skb_pool_mem_size,GFP_ATOMIC);

    if ( g_pSkbPool->start_skbs_p == NULL )
    {
        return -ENOMEM;
    }
    g_pSkbPool->end_skbs_p = g_pSkbPool->start_skbs_p + skb_pool_mem_size;

    memset(g_pSkbPool->start_skbs_p, 0,skb_pool_mem_size);

    pSkbuff = (uint8_t *)g_pSkbPool->start_skbs_p;
    /* Align the first SKB pointer to cache line */
    pSkbuff = (uint8_t *) (((uintptr_t)pSkbuff + BCM_DCACHE_ALIGN_LEN) & ~BCM_DCACHE_ALIGN_LEN);

    /* Chain skbs */
    num_skbs = 0;
    while (num_skbs < ENET_NUM_RX_PKT_DESC)
    {
        ((struct sk_buff *) pSkbuff)->next_free = g_pSkbPool->freeSkbList;
        g_pSkbPool->freeSkbList = (struct sk_buff *) pSkbuff;
        pSkbuff += BCM_SKB_ALIGNED_SIZE;
        num_skbs++;
    }

    /* Initialize the kernel cache slab for dynamic SKB allocation */

    /* create a slab cache for device descriptors */
    g_pSkbPool->enetSkbCache = kmem_cache_create("bcm_EnetSkbCache", 
                                                BCM_SKB_ALIGNED_SIZE,
                                                0, /* align */
                                                SLAB_HWCACHE_ALIGN, /* flags */
                                                NULL); /* ctor */
    if(g_pSkbPool->enetSkbCache == NULL)
    {
        printk(KERN_NOTICE "Eth: Unable to create skb cache\n");

        return -ENOMEM;
    }
    return 0;
}

static int __init enet_sysport_mem_init(void)
{
    int err;

    /* Initialize SKB cache */
    err = enet_sysport_skb_cache_init();

    if (!err)
    {
        err = enet_sysport_pkt_desc_init();
    }

    return err;
}

static void __init enet_sysport_setup_switch(void)
{    
    uint32 v32;
    uint32 port;

    /* Force IMP port link up for 4908 */
    extsw_rreg_mmap(PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(IMP_PORT_ID), (void*)&v32, sizeof(v32));
    v32 |= REG_CONTROL_MPSO_LINKPASS;
    extsw_wreg_mmap(PORT_OVERIDE_PAGE, PORT_OVERIDE_REG(IMP_PORT_ID), (void*)&v32, sizeof(v32));

    /* Enable MAC RX/TX for all switch ports and no-STP*/
    extsw_rreg_mmap(PAGE_CONTROL, REG_PORT_CTRL, (void*)&v32, sizeof(v32)); /* Read first */
    v32 &= ~REG_PORT_CTRL_DISABLE;
    v32 &= ~REG_PORT_STP_MASK;  /* No spanning tree */
    for (port = 0 ; port < 4; port++)
    {
        extsw_wreg_mmap(PAGE_CONTROL, REG_PORT_CTRL+port, (void*)&v32, sizeof(v32)); /* Port#0 is base*/
    }

    /* Set PBVLAN MAP  */
    v32 = 0x1ff;
    for (port = 0 ; port < 4; port++)
    {
        extsw_wreg_mmap(PAGE_PORT_BASED_VLAN, REG_VLAN_CTRL_P0 + (port * 2), (void*)&v32, sizeof(v32)); /* Port#0 is base*/
    }
}
#if defined(PKT_DESC_IUDMA)

static void __init reset_sysport(void)
{
    volatile DmaRegs *dmaCtrl = (DmaRegs *)(GMAC_DMA_BASE);    
    volatile DmaStateRam *StateRam;

    /* Disable the DMA controller and channel */
    dmaCtrl->chcfg[IUDMA_RX_CHAN].cfg = 0;
    dmaCtrl->chcfg[IUDMA_TX_CHAN].cfg = 0;
    dmaCtrl->controller_cfg &= ~DMA_MASTER_EN;
    /* Reset RX Channel state data */
    StateRam = &dmaCtrl->stram.s[IUDMA_RX_CHAN];
    StateRam->baseDescPtr = 0 ;
    StateRam->state_data = 0 ;
    StateRam->desc_len_status = 0 ;
    StateRam->desc_base_bufptr = 0 ;
    /* Reset TX Channel state data */
    StateRam = &dmaCtrl->stram.s[IUDMA_TX_CHAN];
    StateRam->baseDescPtr = 0 ;
    StateRam->state_data = 0 ;
    StateRam->desc_len_status = 0 ;
    StateRam->desc_base_bufptr = 0 ;
}

static void __init setup_iudma(void)
{
    volatile DmaRegs *dmaCtrl = (DmaRegs *)(GMAC_DMA_BASE);    
    volatile DmaStateRam *StateRam;
    volatile DmaChannelCfg *pDmaChCfg;

    /* setup TX */
    pDmaChCfg = &dmaCtrl->chcfg[IUDMA_TX_CHAN];
    StateRam = &dmaCtrl->stram.s[IUDMA_TX_CHAN];

    pDmaChCfg->cfg = 0;
    pDmaChCfg->maxBurst = 8; /* DMA_MAX_BURST_LENGTH; */
    pDmaChCfg->intMask = 0;

    StateRam->baseDescPtr = (uint32)(uintptr_t)g_pTxPktDesc->desc_ptr.p_desc_physical_addr;

    /* setup RX */
    pDmaChCfg = &dmaCtrl->chcfg[IUDMA_RX_CHAN];
    StateRam = &dmaCtrl->stram.s[IUDMA_RX_CHAN];

    StateRam->baseDescPtr = (uint32)(uintptr_t)g_pRxPktDesc->desc_ptr.p_desc_physical_addr;

    pDmaChCfg->cfg = 0;
    pDmaChCfg->maxBurst = 8; /* DMA_MAX_BURST_LENGTH; */
    pDmaChCfg->intMask = 0;
    pDmaChCfg->intStat = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
    pDmaChCfg->intMask = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;

    pDmaChCfg->cfg |= DMA_ENABLE;

    /* Enable Controller */
    dmaCtrl->controller_cfg |= DMA_MASTER_EN;
    dmaCtrl->controller_cfg &= ~DMA_FLOWC_CH1_EN;

}

extern int gmac_init( void );
extern int gmac_set_active( void );

static int __init setup_sys_port(void)
{
    gmac_init();
    setup_iudma();
    gmac_set_active();
    return 0;
}

#else

static void __init reset_sysport(void)
{
    volatile uint32_t v32;
    uint16_t p_idx, c_idx, timeout;

    // Note : This reset is expected to be called before driver initialization
    //        In case this reset function is not called right after CFE
    //        (where local RAM is used for DMA descriptors), 
    //        unhandled RX and TX DMA data (in DDR descriptors) will result in memory leak

    // Disable RX UMAC
    v32 = SYSPORT_UMAC->SYSTEMPORT_UMAC_CMD;
    v32 &= ~SYSPORT_UMAC_CMD_RX_ENA_M; /* Disable RX */
    SYSPORT_UMAC->SYSTEMPORT_UMAC_CMD=v32;

    // In case there is active RXDMA traffic, wait till all RXDMA is completed 
    if (SYSPORT_RDMA->SYSTEMPORT_RDMA_CONSUMER_INDEX != SYSPORT_RDMA->SYSTEMPORT_RDMA_PRODUCER_INDEX)
    {
        udelay(1000);
    }

    // Disable and Flush RX DMA
    SYSPORT_RDMA->SYSTEMPORT_RDMA_CONTROL &= ~SYSPORT_RDMA_CTRL_RDMA_EN_M;
    SYSPORT_TPC->SYSTEMPORT_TOPCTRL_RX_FLUSH_CNTL=1;

    // Disable TX DMA
    SYSPORT_TDMA->SYSTEMPORT_TDMA_CONTROL &= ~SYSPORT_TDMA_CONTROL_TDMA_EN_M;

    // Wait till all TXDMA is completed
    timeout = 1000;
    v32 = SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX;
    p_idx = (uint16_t)(v32 & SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX_PRODUCER_INDEX_M);
    c_idx = (uint16_t)(v32 >> SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX_CONSUMER_INDEX_S);

    while ((p_idx != c_idx) && timeout)
    {
        udelay (1000);  // will need to increase wait time if system port speed is slower 
                        // (e.g, it takes 1.2ms to transmit 1518 bytes packets when system port is running at 10 Mbps)
        timeout --;
        v32 = SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX;
        p_idx = (uint16_t)(v32 & SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX_PRODUCER_INDEX_M);
        c_idx = (uint16_t)(v32 >> SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX_CONSUMER_INDEX_S);
    }

    // Disable TX UNIMAC and then flush TXDMA
    v32 = SYSPORT_UMAC->SYSTEMPORT_UMAC_CMD;
    v32 &= ~SYSPORT_UMAC_CMD_TX_ENA_M; /* Disable TX */
    SYSPORT_UMAC->SYSTEMPORT_UMAC_CMD=v32;
    SYSPORT_TPC->SYSTEMPORT_TOPCTRL_TX_FLUSH_CNTL=1;

    // Reset complete, prepare for initialization
    SYSPORT_TPC->SYSTEMPORT_TOPCTRL_RX_FLUSH_CNTL=0;
    SYSPORT_TPC->SYSTEMPORT_TOPCTRL_TX_FLUSH_CNTL=0;
    SYSPORT_TDMA->SYSTEMPORT_TDMA_STATUS |= SYSPORT_TDMA_STATUS_LL_RAM_INIT_BUSY_M; // initialize the Link List RAM. 
}

/* Enable System Port RX DMA */
static int __init enet_sysport_rdma_enable(void)
{
    volatile uint32_t v32;
    int wait_cycles = 1000;

    /* Enable RX DMA */
    v32 = SYSPORT_RDMA->SYSTEMPORT_RDMA_CONTROL;/* Read current register settings */
    v32 |= SYSPORT_RDMA_CTRL_RDMA_EN_M;           /* Enable RDMA */
    SYSPORT_RDMA->SYSTEMPORT_RDMA_CONTROL=v32;

    /* Check if RDMA is ready */
    while (wait_cycles--) 
    {
       v32 = SYSPORT_RDMA->SYSTEMPORT_RDMA_STATUS;
       if ( (v32 & (SYSPORT_RDMA_STATUS_RDMA_DISABLED_M | SYSPORT_RDMA_STATUS_DESC_RAM_BUSY_M)) == 0) /* Enabled?*/
       {
               return 0;
       }
       udelay(10);
    } 

    return -1;
}

/* Enable System Port TX DMA */
static int __init enet_sysport_tdma_enable(void)
{
    volatile uint32_t v32;
    int wait_cycles = 1000;

    /* Enable TX DMA */
    v32 = SYSPORT_TDMA->SYSTEMPORT_TDMA_CONTROL;/* Read current settings */
    v32 |= SYSPORT_TDMA_CONTROL_TDMA_EN_M;        /* Disable TSB */
    SYSPORT_TDMA->SYSTEMPORT_TDMA_CONTROL=v32;

    /* Check if TDMA is ready */
    while (wait_cycles--) 
    {
       v32 = SYSPORT_TDMA->SYSTEMPORT_TDMA_STATUS;
       if ( (v32 & (SYSPORT_TDMA_STATUS_TDMA_DISABLED_M | SYSPORT_TDMA_STATUS_LL_RAM_INIT_BUSY_M)) == 0) /* Enabled?*/
       {
               return 0;
       }
       udelay(10);
    } 

    return -1;
}

static int __init enet_sysport_enable_unimac(void)
{
    volatile uint32_t v32;

    v32 = SYSPORT_UMAC->SYSTEMPORT_UMAC_CMD;
    v32 |= SYSPORT_UMAC_CMD_TX_ENA_M; /* Enable TX */
    v32 |= SYSPORT_UMAC_CMD_RX_ENA_M; /* Enable RX */
    SYSPORT_UMAC->SYSTEMPORT_UMAC_CMD=v32;
    return 0;
}

static int __init enet_sysport_enable(void)
{
    int err = 0;

    err += enet_sysport_rdma_enable();
    err += enet_sysport_tdma_enable();
    err += enet_sysport_enable_unimac();

    return err;
}

static int __init setup_sys_port(void)
{
    /* all memory allocation are done already */
    uint32_t v32 = 0;
    volatile uint32_t *pRegV32 = SYSPORT_RXCHK_BASE;

    *pRegV32 |= SYSPORT_RXCHK_CONTROL_SKIP_FCS_M;

    /* System Port RBUF configuration */

    v32 = SYSPORT_RBUF->SYSTEMPORT_RBUF_RBUF_CONTROL;     /* Read Chip Defaults */
    v32 &= ~SYSPORT_RBUF_CTRL_RSB_EN_M;                   /* Disable RSB */
    v32 &= ~SYSPORT_RBUF_CTRL_4B_ALIGN_M;                 /* Disable 4-Byte IP Alignment */
    v32 &= ~SYSPORT_RBUF_CTRL_BTAG_STRIP_M;               /* Do not strip BRCM TAG */
    v32 |= SYSPORT_RBUF_CTRL_BAD_PKT_DISCARD_M;           /* Discard Bad Packets */
    /* Read-Modify-Write */
    SYSPORT_RBUF->SYSTEMPORT_RBUF_RBUF_CONTROL=v32; 

    SYSPORT_RBUF->SYSTEMPORT_RBUF_RBUF_PACKET_READY_THRESHOLD=0x80; /* Keep chip default */

    /* System Port TBUF configuration -- No change, keep chip defaults */

    /* System Port RDMA Configuration */

    /* RDMA Control Register */
    v32 = SYSPORT_RDMA->SYSTEMPORT_RDMA_CONTROL;  /* Read Chip Defaults */
    v32 &= ~SYSPORT_RDMA_CTRL_RDMA_EN_M;          /* Disable RDMA */
    v32 &= ~SYSPORT_RDMA_CTRL_RING_CFG_M;          /* Enable Descriptor Ring Mode */
    v32 |= SYSPORT_RDMA_CTRL_DISCARD_EN_M;        /* Enable Pkt discard by RDMA when ring full */
    v32 &= ~SYSPORT_RDMA_CTRL_DATA_OFFSET_M;       /* Zero data offset - this feature could be used later to reduce host buffer size */
    v32 |= SYSPORT_RDMA_CTRL_DDR_DESC_RD_EN_M;    /* HW reads host desc from DDR in local desc memory */
    v32 |= SYSPORT_RDMA_CTRL_DDR_DESC_WR_EN_M;    /* HW writes backe local desc memory to DDR */
    v32 |= SYSPORT_RDMA_CTRL_DDR_DESC_SWAP_M;     /* Both Byte and word swap enabled for Desc - TBD - need to understand */
    /* Read-Modify-Write */
    SYSPORT_RDMA->SYSTEMPORT_RDMA_CONTROL=v32; 

    /* RDMA Buffer and Ring Size Register */
    v32 = 0;/* Reset register  */
    v32 |= ( (SYSPORT_PKT_LEN_LOG2 << SYSPORT_RDMA_BSRS_BUF_SIZE_LOG2_S) & SYSPORT_RDMA_BSRS_BUF_SIZE_LOG2_M ); /* set buf size */
    v32 |= ( (0x200 << SYSPORT_RDMA_BSRS_RING_SIZE_S) & SYSPORT_RDMA_BSRS_RING_SIZE_M ); /* force chip default of 512 */
    SYSPORT_RDMA->SYSTEMPORT_RDMA_BSRS=v32; 


    /* RDMA Consumer Index Register */
    /* Initialize RX DMA consumer index - low 16 bit; High 16-bits are read-only */
    g_pRxPktDesc->rx_c_index = 0; /* Important - this got incremented during PktDesc initialization */
    SYSPORT_RDMA->SYSTEMPORT_RDMA_CONSUMER_INDEX=0x0;
    SYSPORT_RDMA->SYSTEMPORT_RDMA_PRODUCER_INDEX=0x0;

    /* RDMA Desc Start Address Registers */
    /* In desciptor ring mode - start address is index = 0 */
    SYSPORT_RDMA->SYSTEMPORT_RDMA_START_ADDRESS_LOW=0;
    SYSPORT_RDMA->SYSTEMPORT_RDMA_START_ADDRESS_HIGH=0;

    /* RDMA DDR Desc Ring Register */
    SYSPORT_RDMA->SYSTEMPORT_RDMA_DDR_DESC_RING_START_LOW = (uint32)(uintptr_t)g_pRxPktDesc->desc_ptr.p_desc_physical_addr;
    SYSPORT_RDMA->SYSTEMPORT_RDMA_DDR_DESC_RING_START_HIGH = 0; /* Ideally we should put the Hi 8-bits here */

    /* RDMA DDR Desc Ring Size Register */
    SYSPORT_RDMA->SYSTEMPORT_RDMA_DDR_DESC_RING_SIZE = SYSPORT_NUM_RX_PKT_DESC_LOG2;

    /* RDMA Multi-Buffer-Done-Interrupt-Threshold : No timeout & interrupt every packet */
    SYSPORT_RDMA->SYSTEMPORT_RDMA_MULTIPLE_BUFFERS_DONE_INTERRUPT_THRESHOLD_PUSH_TIMER = 1;
    /* enable DDR DESC write push timer to 1 timer tick (equals 1024 RDMA sys clocks */
    SYSPORT_RDMA->SYSTEMPORT_RDMA_DDR_DESC_RING_PUSH_TIMER = (0x1 & SYSTEMPORT_RDMA_DDR_DESC_RING_PUSH_TIMER_TIMEOUT_M);
    /* TDMA Block Configuration */

    /* System port supports upto 16 Desc Rings;
       Only one TX DDR Desc ring is used; It is mapped to TX-Queue[0] */

    /* Enable TX Q#0 */
    SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_HEAD_TAIL_PTR = SYSPORT_TDMA_DESC_RING_XX_HEAD_TAIL_PTR_RING_EN_M;

    /* Initialize Producer & Consumer Index */
    SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX = 0;
    /* Q#0 DDR Desc Ring Address */
    SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DDR_DESC_RING_XX_START_LOW = (uint32)(uintptr_t)g_pTxPktDesc->desc_ptr.p_desc_physical_addr;
    SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DDR_DESC_RING_XX_START_HIGH = 0; /* Ideally this should be high 8-bit of address */

    SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_MAPPING = 0x40;
    /* enable DDR DESC read push timer to 1 timer tick (equals 1024 TDMA sys clocks */
    SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DDR_DESC_RING_XX_PUSH_TIMER = (0x1 & SYSTEMPORT_TDMA_DDR_DESC_RING_PUSH_TIMER_TIMEOUT_M);
    /* Q#0 DDR Desc Ring Size Log2 */
    SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DDR_DESC_RING_XX_SIZE = SYSPORT_NUM_TX_PKT_DESC_LOG2;
    SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_INTR_CONTROL = 0x3;
    SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_MAX_HYST_THRESHOLD = 0x00100009;
    /* enable arbitrator for Q#0 */
    SYSPORT_TDMA->SYSTEMPORT_TDMA_TIER_2_ARBITER_CTRL = 0x1; /* Round Robin */
    SYSPORT_TDMA->SYSTEMPORT_TDMA_TIER_1_ARBITER_0_CTRL = 0x1;
    SYSPORT_TDMA->SYSTEMPORT_TDMA_TIER_1_ARBITER_1_CTRL = 0x1;
    SYSPORT_TDMA->SYSTEMPORT_TDMA_TIER_1_ARBITER_2_CTRL = 0x1;
    SYSPORT_TDMA->SYSTEMPORT_TDMA_TIER_1_ARBITER_3_CTRL = 0x1;
    SYSPORT_TDMA->SYSTEMPORT_TDMA_TIER_1_ARBITER_0_QUEUE_ENABLE = 0x000000ff;
    SYSPORT_TDMA->SYSTEMPORT_TDMA_TIER_1_ARBITER_1_QUEUE_ENABLE = 0x0000ff00;
    SYSPORT_TDMA->SYSTEMPORT_TDMA_TIER_1_ARBITER_2_QUEUE_ENABLE = 0x00ff0000;
    SYSPORT_TDMA->SYSTEMPORT_TDMA_TIER_1_ARBITER_3_QUEUE_ENABLE = 0xff000000;
    /* TDMA Control Register */
    v32 = SYSPORT_TDMA->SYSTEMPORT_TDMA_CONTROL; /* Read chip defaults */
    v32 &= ~SYSPORT_TDMA_CONTROL_TSB_EN_M; /* Disable TSB */
    v32 |= SYSPORT_TDMA_CONTROL_DDR_DESC_RING_EN_M; /* Enable DDR Desc Ring fetch */
    v32 |= SYSPORT_TDMA_CONTROL_NO_ACB_M; /* No ACB */
    SYSPORT_TDMA->SYSTEMPORT_TDMA_CONTROL = v32;

    /* Enable Tier-1 arbiter for Q#0 */
    SYSPORT_TDMA->SYSTEMPORT_TDMA_TIER_1_ARBITER_0_QUEUE_ENABLE=0x1;

    /* Initialize interrupts */
    SYSPORT_INTR0->SYSTEMPORT_INTR_CPU_MASK_SET = SYSPORT_INTR_ALL_INTR_MASK; /* Disable all interrupts */
    SYSPORT_INTR0->SYSTEMPORT_INTR_CPU_CLEAR = SYSPORT_INTR_ALL_INTR_MASK;    /* Clear all interrupts */ 

    return enet_sysport_enable();
}
#endif

static int __init bcmenet_sysport_module_init(void)
{
    int err;

    TRACE(("bcm63xxenet: bcmenet_sysport_module_init\n"));

    err = bcmenet_sysport_init_dev();

    reset_sysport();

    /* Initialize all the required memory */
    if (!err)
    {
        err = enet_sysport_mem_init();
    }

    if (!err)
    {
        err = enet_sysport_set_rx_interrupt();
    }

    if (!err)
    {
        err = setup_sys_port();
    }

    if (!err)
    {
        err = enet_sysport_open_dev();
    }

    enet_sysport_setup_switch();

    return err;
}
/* ethsw_get_txrx_imp_port_pkts() - needed to satify compilation error */
void ethsw_get_txrx_imp_port_pkts(unsigned int *tx, unsigned int *rx)
{
    *tx = 0;
    *rx = 0;

    return;
}
EXPORT_SYMBOL(ethsw_get_txrx_imp_port_pkts);

late_initcall(bcmenet_sysport_module_init);
module_exit(enet_sysport_module_cleanup);
MODULE_DESCRIPTION("BCM internal ethernet system port network driver");
MODULE_LICENSE("GPL");

