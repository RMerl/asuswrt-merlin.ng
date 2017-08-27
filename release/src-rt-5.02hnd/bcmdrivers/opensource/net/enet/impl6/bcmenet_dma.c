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

//**************************************************************************
// File Name  : bcmeapi_legacy.c
//
// Description: This is Linux network driver for Broadcom Ethernet controller
//
//**************************************************************************
#define _BCMENET_LOCAL_

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
#include <spidevices.h>
#include <bcmnetlink.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>
#include "linux/bcm_assert_locks.h"
#include <linux/bcm_realtime.h>
#include "bcmenet.h"
#include "bcmmii.h"
#include "bcmenet_dma_inline_impl6.h"

#include "bcm_eth.h"
#include <linux/stddef.h>
#include <asm/atomic.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/nbuff.h>
#include "bcmPktDma.h"

#include <net/net_namespace.h>

#if defined(_CONFIG_BCM_BPM)
#include <linux/gbpm.h>
#include "bpm.h"
#endif

int cur_rxdma_channels = 1;
int cur_txdma_channels = 1;

void uninit_buffers(BcmEnet_devctrl *pDevCtrl);
static int init_buffers(BcmEnet_devctrl *pDevCtrl);
static int init_tx_channel(BcmEnet_devctrl *pDevCtrl);
static int init_rx_channel(BcmEnet_devctrl *pDevCtrl);
static FN_HANDLER_RT bcmeapi_enet_isr(int irq, void * pContext);

#if defined(_CONFIG_BCM_BPM)
static void enet_rx_set_bpm_alloc_trig( BcmEnet_devctrl *pDevCtrl);
static int enet_bpm_alloc_buf_ring(BcmEnet_devctrl *pDevCtrl, uint32 num);
static void enet_bpm_free_buf_ring(BcmEnet_devctrl *pDevCtrl);
#endif


#if defined(RXCHANNEL_PKT_RATE_LIMIT)
static void bcm63xx_timer(void * arg);
/* default pkt rate is 100 pkts/100ms */
int rxchannel_rate_credit[ENET_RX_CHANNELS_MAX] = {100};
int rxchannel_rate_limit_enable[ENET_RX_CHANNELS_MAX] = {0};
int rxchannel_isr_enable[ENET_RX_CHANNELS_MAX] = {1};
int rx_pkts_from_last_jiffies[ENET_RX_CHANNELS_MAX] = {0};
static int last_pkt_jiffies[ENET_RX_CHANNELS_MAX] = {0};
int timer_pid = -1;
atomic_t timer_lock = ATOMIC_INIT(1);
static DECLARE_COMPLETION(timer_done);
#endif /* RXCHANNEL_PKT_RATE_LIMIT */


/* Number of active interfaces. Defined in bcmenet.c */
extern int iface_cnt;
/* Bitmap for RX notification. LSB: Rx on vnet_dev[0].
 * Defined in bcmenet.c */
extern unsigned long rx_flag;

extern BcmPktDma_Bds *bcmPktDma_Bds_p;
extern struct notifier_block br_notifier;

struct kmem_cache *enetSkbCache;

/* These variables are kept to avoid the build from breaking.
 * They're used in bcmenet_dma_inline.h but aren't useful for BCM960333.
 */
/* When TX iuDMA channel is used for determining the egress queue,
   this array provides the Tx iuDMA channel to egress queue mapping
   information */
int channel_for_queue[NUM_EGRESS_QUEUES] = {0};
int use_tx_dma_channel_for_priority = 0;
/* rx scheduling control and config variables */
int scheduling = WRR_SCHEDULING;
int weight_pkts[ENET_RX_CHANNELS_MAX] = {[0 ... (ENET_RX_CHANNELS_MAX-1)] = 320};
int pending_weight_pkts[ENET_RX_CHANNELS_MAX] = {[0 ... (ENET_RX_CHANNELS_MAX-1)] = 320};
int pending_channel[ENET_RX_CHANNELS_MAX] = {0}; /* Initialization is done during module init */
int channel_ptr = 0;
int loop_index = 0;
int global_channel = 0;
int pending_ch_tbd;
int channels_tbd;
int channels_mask;
int pending_channels_mask;
int next_channel[ENET_RX_CHANNELS_MAX];

#if defined(RXCHANNEL_BYTE_RATE_LIMIT)
static int channel_rx_rate_limit_enable[ENET_RX_CHANNELS_MAX] = {0};
static int rx_bytes_from_last_jiffies[ENET_RX_CHANNELS_MAX] = {0};
/* default rate in bytes/sec */
int channel_rx_rate_credit[ENET_RX_CHANNELS_MAX] = {1000000};
static int last_byte_jiffies[ENET_RX_CHANNELS_MAX] = {0};
#endif /* defined(RXCHANNEL_BYTE_RATE_LIMIT) */

#ifdef DYING_GASP_API
extern unsigned char dg_ethOam_frame[64];
extern struct sk_buff *dg_skbp;
#endif

static inline int get_rxIrq(int netdev_id)
{
    return bcmPktDma_EthSelectRxIrq(netdev_id);
}

/* Frees the buffer ring for an Eth RX channel */
static void enet_bpm_free_buf_ring(BcmEnet_devctrl *pDevCtrl)
{
    uninit_buffers(pDevCtrl);
}

/* Allocates the buffer ring for an Eth RX channel */
static int enet_bpm_alloc_buf_ring(BcmEnet_devctrl *pDevCtrl, uint32 num)
{
    unsigned char *pFkBuf, *pData;
    uint32 context = BUILD_CONTEXT(pDevCtrl, 0);
    uint32 buf_ix;

    for (buf_ix=0; buf_ix < num; buf_ix++)
    {
        if ((pFkBuf = (uint8_t *) gbpm_alloc_buf()) == NULL)
            return GBPM_ERROR;

        pData = PFKBUFF_TO_PDATA(pFkBuf,BCM_PKT_HEADROOM);

        /* Place a FkBuff_t object at the head of pFkBuf */
        fkb_preinit(pFkBuf, (RecycleFuncP)bcm63xx_enet_recycle_impl6, context);

        cache_flush_region(pData, (uint8_t*)pFkBuf + BCM_PKTBUF_SIZE);
        bcmPktDma_EthFreeRecvBuf(&pDevCtrl->rxdma[0]->pktDmaRxInfo, pData);
    }
    return GBPM_SUCCESS;
}

int  bcmeapi_open_dev(BcmEnet_devctrl *pDevCtrl, struct net_device *dev)
{
    BcmEnet_RxDma *rxdma;
    BcmPktDma_EthTxDma *txdma;

    ENET_RX_LOCK();
    pDevCtrl->dmaCtrl->controller_cfg |= DMA_MASTER_EN;

    /*  Enable the Rx DMA channels and their interrupts  */
    rxdma = pDevCtrl->rxdma[0];
#if defined(RXCHANNEL_PKT_RATE_LIMIT)
    rxchannel_isr_enable[pDevCtrl->vport_id] = 1;
#endif
    /*
     * Disabling and reenabling a DMA channel leaves one BD in the ring
     * in an inconsistent state, causing DMA stalls. Leave channel enabled
     * bcmPktDma_EthRxEnable(&rxdma->pktDmaRxInfo);
     */
    bcmPktDma_BcmHalInterruptEnable(0, rxdma->rxIrq);
    ENET_RX_UNLOCK();

    ENET_TX_LOCK();
    /*  Enable the Tx DMA channels  */
    txdma = pDevCtrl->txdma[0];
    bcmPktDma_EthTxEnable(txdma);
    txdma->txEnabled = 1;
    ENET_TX_UNLOCK();
    return 0; /* success */
}

void bcmeapi_del_dev_intr(BcmEnet_devctrl *pDevCtrl)
{
    BcmEnet_RxDma *rxdma;
    BcmPktDma_EthTxDma *txdma;

    ENET_RX_LOCK();
    rxdma = pDevCtrl->rxdma[0];
    bcmPktDma_BcmHalInterruptDisable(0, rxdma->rxIrq);
    /*
     * Disabling and reenabling a DMA channel leaves one BD in the ring
     * in an inconsistent state, causing DMA stalls. Leave channel enabled
     * bcmPktDma_EthRxDisable(&rxdma->pktDmaRxInfo);
     */
    ENET_RX_UNLOCK();

    ENET_TX_LOCK();
    txdma = pDevCtrl->txdma[0];
    txdma->txEnabled = 0;
    bcmPktDma_EthTxDisable(txdma);
    ENET_TX_UNLOCK();
}

int bcmeapi_init_dev(struct net_device *dev)
{
#if defined(RXCHANNEL_PKT_RATE_LIMIT)
    struct task_struct * bcmsw_timer_task_struct;
#endif    

#ifdef DYING_GASP_API
	/* Set up dying gasp buffer from packet transmit when we power down */
    dg_skbp = alloc_skb(64, GFP_ATOMIC);
    if (dg_skbp)
    {
        memset(dg_skbp->data, 0, 64);
   		dg_skbp->len = 64;
        memcpy(dg_skbp->data, dg_ethOam_frame, sizeof(dg_ethOam_frame));
    }
#endif

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
    bcmsw_timer_task_struct = kthread_run(bcm63xx_timer, NULL, "bcmsw_timer");
    timer_pid = bcmsw_timer_task_struct->pid;
   
    if (timer_pid < 0)
        return -ENOMEM;
#endif
    return 0;
}

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
/*
 * bcm63xx_timer: 100ms timer for updating rx rate control credits
 */
static int bcm63xx_timer(void * arg)
{
    BcmEnet_devctrl *priv;
    BcmEnet_RxDma *rxdma;
    unsigned int elapsed_msecs;
    int i;
    struct sched_param param;

    daemonize("bcmsw_timer");
    param.sched_priority = BCM_RTPRIO_DATA;
    sched_setscheduler(current, SCHED_RR, &param);
    set_user_nice(current, 0);

    while (atomic_read(&timer_lock) > 0)
    {
        for (i = 0; i < iface_cnt; i++) {
            ENET_RX_LOCK();
            priv = (BcmEnet_devctrl *)netdev_priv(vnet_dev[i]);
            if (rxchannel_rate_limit_enable[i]) {
                elapsed_msecs = jiffies_to_msecs(jiffies -
                        last_pkt_jiffies[i]);
                if (elapsed_msecs >= 99) {
                    rxdma = priv->rxdma[0];
                    BCM_ENET_DEBUG("pkts_from_last_jiffies = %d \n",
                            rx_pkts_from_last_jiffies[i]);
                    rx_pkts_from_last_jiffies[i] = 0;
                    last_pkt_jiffies[i] = jiffies;
                    if (rxchannel_isr_enable[i] == 0) {
                        BCM_ENET_DEBUG("Enabling DMA Channel & Interrupt \n");
                        /* Disable this. Should be implemented differently in
                         * impl6
                        switch_rx_ring(priv, 0, 0);
                        */
                        bcmPktDma_BcmHalInterruptEnable(i, rxdma->rxIrq);
                        rxchannel_isr_enable[i] = 1;
                    }
                }
            }
            ENET_RX_UNLOCK();
        }

        /*  Sleep for HZ/10 jiffies (100ms)  */
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(HZ/10);
    }

    complete_and_exit(&timer_done, 0);
    printk("bcm63xx_timer: thread exits!\n");

    return 0;
}
#endif /* defined(RXCHANNEL_PKT_RATE_LIMIT) */

FN_HANDLER_RT bcm63xx_gphy_isr(int irq, void * dev_id)
{
    bcmeapiPhyIntEnable(1);
    return BCM_IRQ_HANDLED;
}

int bcmeapi_map_interrupt(BcmEnet_devctrl *pDevCtrl)
{
    BcmHalMapInterrupt(bcm63xx_gphy_isr, (void*)pDevCtrl, INTERRUPT_ID_EPHY);
    return BCMEAPI_INT_MAPPED_INTPHY;
}

void bcmeapi_free_irq(BcmEnet_devctrl *pDevCtrl)
{
    free_irq(INTERRUPT_ID_EPHY, pDevCtrl);
}

static void bcm63xx_uninit_txdma_structures(BcmEnet_devctrl *pDevCtrl)
{
    BcmPktDma_EthTxDma *txdma;
    int nr_tx_bds = bcmPktDma_Bds_p->host.eth_txbds[pDevCtrl->vport_id];

    txdma = pDevCtrl->txdma[0];

    /* disable DMA */
    txdma->txEnabled = 0;
    txdma->txDma->cfg = 0;
    (void) bcmPktDma_EthTxDisable(txdma);

    /* if any, free the tx skbs */
    while (txdma->txFreeBds < nr_tx_bds) {
        txdma->txFreeBds++;
        nbuff_free((void *)txdma->txRecycle[txdma->txHeadIndex++].key);
        if (txdma->txHeadIndex == nr_tx_bds)
            txdma->txHeadIndex = 0;
    }

    /* free the transmit buffer descriptor ring */
    txdma = pDevCtrl->txdma[0];
    /* remove the tx bd ring */
    if (txdma->txBdsBase) {
        kfree((void *)txdma->txBdsBase);
    }
    /* free the txdma channel structures */
    if (pDevCtrl->txdma[0]) {
        kfree((void *)(pDevCtrl->txdma[0]));
    }
}

static void bcm63xx_uninit_rxdma_structures(BcmEnet_devctrl *pDevCtrl)
{
    BcmEnet_RxDma *rxdma;
    int rxIrq = bcmPktDma_EthSelectRxIrq(pDevCtrl->sw_port_id);

    rxdma = pDevCtrl->rxdma[0];
    rxdma->pktDmaRxInfo.rxDma->cfg = 0;
    (void) bcmPktDma_EthRxDisable(&rxdma->pktDmaRxInfo);

    /* disable the interrupts from device */
    /* Channel parameter is not used for Duna */
    bcmPktDma_BcmHalInterruptDisable(0, rxIrq);
    /* free the IRQ */
    free_irq(rxIrq, (BcmEnet_devctrl *)BUILD_CONTEXT(pDevCtrl,0));

    /* release allocated receive buffer memory */
    uninit_buffers(pDevCtrl);

    /* free the receive buffer descriptor ring */
    if (rxdma->pktDmaRxInfo.rxBdsBase) {
        kfree((void *)rxdma->pktDmaRxInfo.rxBdsBase);
    }

    /* free the rxdma channel structures */
    if (pDevCtrl->rxdma[0]) {
        kfree((void *)(pDevCtrl->rxdma[0]));
    }
}

void bcmeapi_free_queue(BcmEnet_devctrl *pDevCtrl)
{
    bcm63xx_uninit_txdma_structures(pDevCtrl);
    bcm63xx_uninit_rxdma_structures(pDevCtrl);
#if defined(_CONFIG_BCM_BPM)
    gbpm_unresv_rx_buf(GBPM_PORT_ETH, pDevCtrl->vport_id);
#endif
    bcmenet_del_proc_files(pDevCtrl->dev);
}

static int bcm63xx_init_txdma_structures(BcmEnet_devctrl *pDevCtrl)
{
    BcmPktDma_EthTxDma *txdma;

    pDevCtrl->txdma[0] = (BcmPktDma_EthTxDma *) (kzalloc(
                           sizeof(BcmPktDma_EthTxDma), GFP_KERNEL));
    if (pDevCtrl->txdma[0] == NULL) {
        printk("Unable to allocate memory for tx dma rings \n");
        return -ENXIO;
    }
    BCM_ENET_DEBUG("The txdma is 0x%p \n", pDevCtrl->txdma[0]);
    txdma = pDevCtrl->txdma[0];

    /* init number of Tx BDs in each tx ring */
    txdma->numTxBds = bcmPktDma_EthGetTxBds(txdma, pDevCtrl->vport_id);
    BCM_ENET_DEBUG("Enet: txbds=%u \n", txdma->numTxBds);
    return 0;
}

static int bcm63xx_init_rxdma_structures(BcmEnet_devctrl *pDevCtrl)
{
    BcmEnet_RxDma *rxdma;

    /* init rx dma channel structures */
    pDevCtrl->rxdma[0] = (BcmEnet_RxDma *) (kzalloc(
                           sizeof(BcmEnet_RxDma), GFP_KERNEL));
    if (pDevCtrl->rxdma[0] == NULL) {
        printk("Unable to allocate memory for rx dma rings \n");
        return -ENXIO;
    }
    BCM_ENET_DEBUG("The rxdma is 0x%p \n", pDevCtrl->rxdma[0]);

    rxdma = pDevCtrl->rxdma[0];

    /* init number of Rx BDs in each rx ring. */
    rxdma->pktDmaRxInfo.numRxBds =
                    bcmPktDma_EthGetRxBds(&rxdma->pktDmaRxInfo, pDevCtrl->vport_id);

    /* request IRQs only once at module init */
    {
      /* Substitute channel parameter for enetcore number */
      int rxIrq = bcmPktDma_EthSelectRxIrq(pDevCtrl->sw_port_id);

      /* disable the interrupts from device */
      bcmPktDma_BcmHalInterruptDisable(0, rxIrq);

      /* a Host owned channel */
      BcmHalMapInterrupt(bcmeapi_enet_isr, (void*)(BUILD_CONTEXT(pDevCtrl, 0)), rxIrq);
    }
    return 0;
}

/*
 * bcmeapi_enet_isr: Acknowledge interrupt and check if any packets have
 *                  arrived on Rx DMA channel 0..3
 */
static FN_HANDLER_RT bcmeapi_enet_isr(int irq, void * pContext)
{
    /* this code should not run in DQM operation !!! */

    BcmEnet_devctrl *pDevCtrl;
    enet_global_var_t *global_p = &global;

    pDevCtrl = CONTEXT_TO_PDEVCTRL((uint32)pContext);

    /* Only rx channels owned by the Host come through this ISR */
    bcmPktDma_EthClrRxIrq_Iudma(&pDevCtrl->rxdma[0]->pktDmaRxInfo);

    set_bit(pDevCtrl->vport_id, (volatile unsigned long *)&rx_flag);
    BCMENET_WAKEUP_RXWORKER(global_p);

    return BCM_IRQ_HANDLED;
}

static int bcm63xx_alloc_txdma_bds(BcmEnet_devctrl *pDevCtrl)
{
    BcmPktDma_EthTxDma *txdma;
    int nr_tx_bds;

    txdma = pDevCtrl->txdma[0];
    nr_tx_bds = txdma->numTxBds;

    /* BDs allocated in bcmPktDma lib in PSM or in DDR */
    /* First parameter is not used inside the function for Duna */
    txdma->txBdsBase = bcmPktDma_EthAllocTxBds(pDevCtrl->vport_id, nr_tx_bds);
    if (txdma->txBdsBase == NULL)
    {
        printk("Unable to allocate memory for Tx Descriptors \n");
        return -ENOMEM;
    }

    BCM_ENET_DEBUG("bcm63xx_alloc_txdma_bds txdma->txBdsBase 0x%x",
        (unsigned int)txdma->txBdsBase);

    txdma->txBds = txdma->txBdsBase;
    txdma->txRecycle = (BcmPktDma_txRecycle_t *)((uint32)txdma->txBds + (nr_tx_bds * sizeof(DmaDesc)));

    /* Align BDs to a 16/32 byte boundary - Apr 2010 */
    txdma->txBds = (volatile void *)(((int)txdma->txBds + 0xF) & ~0xF);
    txdma->txBds = (volatile void *)CACHE_TO_NONCACHE(txdma->txBds);
    txdma->txRecycle = (BcmPktDma_txRecycle_t *)((uint32)txdma->txBds + (nr_tx_bds * sizeof(DmaDesc)));
    txdma->txRecycle = (BcmPktDma_txRecycle_t *)NONCACHE_TO_CACHE(txdma->txRecycle);

    txdma->txFreeBds = nr_tx_bds;
    txdma->txHeadIndex = txdma->txTailIndex = 0;
    nr_tx_bds = txdma->numTxBds;

    /* BDs allocated in bcmPktDma lib in PSM or in DDR */
    memset((char *) txdma->txBds, 0, sizeof(DmaDesc) * nr_tx_bds );

    return 0;
}

/* Note: this may be called from an atomic context */
static int bcm63xx_alloc_rxdma_bds(BcmEnet_devctrl *pDevCtrl)
{
   BcmEnet_RxDma *rxdma;
   rxdma = pDevCtrl->rxdma[0];

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
   /* Allocate 1 extra BD for rxBdsStdBy */
   /* The channel parameter is not used inside the function for Duna */
   rxdma->pktDmaRxInfo.rxBdsBase = bcmPktDma_EthAllocRxBds(pDevCtrl->vport_id, rxdma->pktDmaRxInfo.numRxBds + 1);
#else
   rxdma->pktDmaRxInfo.rxBdsBase = bcmPktDma_EthAllocRxBds(pDevCtrl->vport_id, rxdma->pktDmaRxInfo.numRxBds);
#endif
   if ( rxdma->pktDmaRxInfo.rxBdsBase == NULL )
   {
      printk("Unable to allocate memory for Rx Descriptors \n");
      return -ENOMEM;
   }

   /* Align BDs to a 16-byte boundary - Apr 2010 */
   rxdma->pktDmaRxInfo.rxBds = (volatile DmaDesc *)(((int)rxdma->pktDmaRxInfo.rxBdsBase + 0xF) & ~0xF);
   rxdma->pktDmaRxInfo.rxBds = (volatile DmaDesc *)CACHE_TO_NONCACHE(rxdma->pktDmaRxInfo.rxBds);

   /* Local copy of these vars also initialized to zero in bcmPktDma channel init */
   rxdma->pktDmaRxInfo.rxAssignedBds = 0;
   rxdma->pktDmaRxInfo.rxHeadIndex = rxdma->pktDmaRxInfo.rxTailIndex = 0;

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
   /* stand by bd ring with only one BD */
   rxdma->rxBdsStdBy = &rxdma->pktDmaRxInfo.rxBds[rxdma->pktDmaRxInfo.numRxBds];
#endif

   return 0;
}

static void setup_rxdma_channel(BcmEnet_devctrl *pDevCtrl)
{
    BcmEnet_RxDma *rxdma = pDevCtrl->rxdma[0];
    volatile DmaRegs *dmaCtrl = pDevCtrl->dmaCtrl;
    int phy_chan = 0;
    DmaStateRam *StateRam = (DmaStateRam *)&dmaCtrl->stram.s[phy_chan * 2];

    memset(StateRam, 0, sizeof(DmaStateRam));

    BCM_ENET_DEBUG("Setup rxdma channel %d, baseDesc 0x%x\n", 0,
        (unsigned int)VIRT_TO_PHY((uint32 *)rxdma->pktDmaRxInfo.rxBds));

    rxdma->pktDmaRxInfo.rxDma->cfg = 0;
    rxdma->pktDmaRxInfo.rxDma->maxBurst = DMA_MAX_BURST_LENGTH;
    rxdma->pktDmaRxInfo.rxDma->intMask = 0;
    rxdma->pktDmaRxInfo.rxDma->intStat = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
    rxdma->pktDmaRxInfo.rxDma->intMask = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;

    dmaCtrl->stram.s[phy_chan * 2].baseDescPtr =
            (uint32)VIRT_TO_PHY((uint32 *)rxdma->pktDmaRxInfo.rxBds);
}

static void setup_txdma_channel(BcmEnet_devctrl *pDevCtrl)
{
    DmaStateRam *StateRam;
    BcmPktDma_EthTxDma *txdma;
    volatile DmaRegs *dmaCtrl = pDevCtrl->dmaCtrl;
    int phy_chan = 0;
    /* txdma[0] because there's only one TX channel */
    txdma = pDevCtrl->txdma[0];

    StateRam = (DmaStateRam *)&dmaCtrl->stram.s[(phy_chan * 2) + 1];
    memset(StateRam, 0, sizeof(DmaStateRam));

    BCM_ENET_DEBUG("setup_txdma_channel: baseDesc 0x%x\n",
        (unsigned int)VIRT_TO_PHY((uint32 *)txdma->txBds));

    txdma->txDma->cfg = 0;
    txdma->txDma->maxBurst = DMA_MAX_BURST_LENGTH;
    txdma->txDma->intMask = 0;

    dmaCtrl->stram.s[(phy_chan * 2) + 1].baseDescPtr =
        (uint32)VIRT_TO_PHY((uint32 *)txdma->txBds);
}

/*
 * init_tx_channel: Initialize Tx DMA channel
 */
static int init_tx_channel(BcmEnet_devctrl *pDevCtrl)
{
    BcmPktDma_EthTxDma *txdma;
    volatile DmaRegs *dmaCtrl = pDevCtrl->dmaCtrl;
    int phy_chan = 0;

    TRACE(("bcm63xxenet: init_txdma\n"));

    /* Reset the DMA channel */
    dmaCtrl->ctrl_channel_reset = 1 << ((phy_chan * 2) + 1);
    dmaCtrl->ctrl_channel_reset = 0;

    txdma = pDevCtrl->txdma[0];
    /* DMA TX channel is always channel 2 (chcfg[1]) */
    txdma->txDma = &dmaCtrl->chcfg[(phy_chan * 2) + 1];

    /* allocate and assign tx buffer descriptors */
    if (bcm63xx_alloc_txdma_bds(pDevCtrl) < 0)
    {
        printk("Allocate Tx BDs Failed ! ch %d \n", 1);
        return -1;
    }
    setup_txdma_channel(pDevCtrl);
    bcmPktDma_EthInitTxChan(txdma->numTxBds, txdma);
    return 0;
}

static void enet_rx_set_bpm_alloc_trig(BcmEnet_devctrl *pDevCtrl)
{
    BcmPktDma_EthRxDma *rxdma = &pDevCtrl->rxdma[0]->pktDmaRxInfo;
    uint32  allocTrig = rxdma->numRxBds * BPM_ENET_ALLOC_TRIG_PCT/100;

    bcmPktDma_EthSetRxChanBpmThresh(rxdma,
        allocTrig, BPM_ENET_BULK_ALLOC_COUNT);
}

/*
 * init_rx_channel: Initialize Rx DMA channel
 */
static int init_rx_channel(BcmEnet_devctrl *pDevCtrl)
{
    BcmEnet_RxDma *rxdma;
    volatile DmaRegs *dmaCtrl = pDevCtrl->dmaCtrl;
    int phy_chan = 0;

    TRACE(("bcm63xxenet: init_rx_channel\n"));

    /* setup the RX DMA channel */
    rxdma = pDevCtrl->rxdma[0];

    /* init rxdma structures */
    rxdma->pktDmaRxInfo.rxDma = &dmaCtrl->chcfg[phy_chan * 2];
    /* Use the enetcore number to identify the interrupt source */
    rxdma->rxIrq = get_rxIrq(pDevCtrl->sw_port_id);

    /* disable the interrupts from device */
    /* channel parameter is not used inside the function for Duna */
    bcmPktDma_BcmHalInterruptDisable(0, rxdma->rxIrq);

    /* Reset the DMA channel */
    dmaCtrl->ctrl_channel_reset = 1 << (phy_chan * 2);
    dmaCtrl->ctrl_channel_reset = 0;

    /* allocate RX BDs */
    if (bcm63xx_alloc_rxdma_bds(pDevCtrl) < 0)
        return -1;

    setup_rxdma_channel(pDevCtrl);
    bcmPktDma_EthInitRxChan(rxdma->pktDmaRxInfo.numRxBds, &rxdma->pktDmaRxInfo);

#if defined(_CONFIG_BCM_BPM)
    enet_rx_set_bpm_alloc_trig(pDevCtrl);
#endif

    /* initialize the receive buffers */
    if (init_buffers(pDevCtrl)) {
        printk(KERN_NOTICE CARDNAME": Low memory.\n");
        uninit_buffers(pDevCtrl);
        return -ENOMEM;
    }
#if defined(_CONFIG_BCM_BPM)
    /* Substitute channel parameter for enetcore number */
    gbpm_resv_rx_buf(GBPM_PORT_ETH, pDevCtrl->vport_id, rxdma->pktDmaRxInfo.numRxBds,
        (rxdma->pktDmaRxInfo.numRxBds * BPM_ENET_ALLOC_TRIG_PCT/100) );
#endif

    return 0;
}

/*
 *  init_buffers: initialize driver's pools of receive buffers
 */
static int init_buffers(BcmEnet_devctrl *pDevCtrl)
{
#if !defined(_CONFIG_BCM_BPM)
    const unsigned long BlockSize = (64 * 1024);
    const unsigned long BufsPerBlock = BlockSize / BCM_PKTBUF_SIZE;
    unsigned long AllocAmt;
    unsigned char *pFkBuf;
    int j=0;
#endif
    int i;
    unsigned char *pSkbuff;
    unsigned long BufsToAlloc;
#if defined(RXCHANNEL_PKT_RATE_LIMIT)
    unsigned char *data;
#endif
    BcmEnet_RxDma *rxdma;
    uint32 context = 0;

    RECYCLE_CONTEXT(context)->channel = 0;

    TRACE(("bcm63xxenet: init_buffers\n"));

    /* allocate recieve buffer pool */
    rxdma = pDevCtrl->rxdma[0];
    /* Local copy of these vars also initialized to zero in bcmPktDma channel init */
    rxdma->pktDmaRxInfo.rxAssignedBds = 0;
    rxdma->pktDmaRxInfo.rxHeadIndex = rxdma->pktDmaRxInfo.rxTailIndex = 0;
    BufsToAlloc = rxdma->pktDmaRxInfo.numRxBds;

#if defined(_CONFIG_BCM_BPM)
    if (enet_bpm_alloc_buf_ring(pDevCtrl, BufsToAlloc) == GBPM_ERROR)
    {
        printk(KERN_NOTICE "Eth: Low memory.\n");

        /* release all allocated receive buffers */
        enet_bpm_free_buf_ring(pDevCtrl);
        return -ENOMEM;
    }
#else
    if( (rxdma->buf_pool = kzalloc(BufsToAlloc * sizeof(uint32_t) + 0x10,
        GFP_ATOMIC)) == NULL )
    {
        printk(KERN_NOTICE "Eth: Low memory.\n");
        return -ENOMEM;
    }

    while(BufsToAlloc) {
        AllocAmt = (BufsPerBlock < BufsToAlloc) ? BufsPerBlock : BufsToAlloc;
        if( (data = kmalloc(AllocAmt * BCM_PKTBUF_SIZE + 0x10, GFP_ATOMIC)) == NULL )
        {
            /* release all allocated receive buffers */
            printk(KERN_NOTICE CARDNAME": Low memory.\n");
            for (i = 0; i < j; i++) {
                if (rxdma->buf_pool[i]) {
                    kfree(rxdma->buf_pool[i]);
                    rxdma->buf_pool[i] = NULL;
                }
            }
            return -ENOMEM;
        }

        rxdma->buf_pool[j++] = data;
        /* Align data buffers on 16-byte boundary - Apr 2010 */
        data = (unsigned char *) (((UINT32) data + 0x0f) & ~0x0f);
        for (i = 0, pFkBuf = data; i < AllocAmt; i++, pFkBuf += BCM_PKTBUF_SIZE) {
            /* Place a FkBuff_t object at the head of pFkBuf */
            fkb_preinit(pFkBuf, (RecycleFuncP)bcm63xx_enet_recycle, context);
            flush_assign_rx_buffer(pDevCtrl, 0, /* headroom not flushed */
                        PFKBUFF_TO_PDATA(pFkBuf,BCM_PKT_HEADROOM),
                        (uint8_t*)pFkBuf + BCM_PKTBUF_SIZE);
        }
        BufsToAlloc -= AllocAmt;
    }
#endif

    if (!rxdma->skbs_p)
    { /* CAUTION!!! DONOT reallocate SKB pool */
        /*
         * Dynamic allocation of skb logic assumes that all the skb-buffers
         * in 'freeSkbList' belong to the same contiguous address range. So if you do any change
         * to the allocation method below, make sure to rework the dynamic allocation of skb
         * logic. look for kmem_cache_create, kmem_cache_alloc and kmem_cache_free functions
         * in this file
        */
        if( (rxdma->skbs_p = kmalloc(
                        (rxdma->pktDmaRxInfo.numRxBds * BCM_SKB_ALIGNED_SIZE) + 0x10,
                        GFP_ATOMIC)) == NULL )
            return -ENOMEM;

        memset(rxdma->skbs_p, 0,
                    (rxdma->pktDmaRxInfo.numRxBds * BCM_SKB_ALIGNED_SIZE) + 0x10);

        rxdma->freeSkbList = NULL;

        /* Chain socket skbs */
        for(i = 0, pSkbuff = (unsigned char *)
            (((unsigned long) rxdma->skbs_p + 0x0f) & ~0x0f);
                i < rxdma->pktDmaRxInfo.numRxBds; i++, pSkbuff += BCM_SKB_ALIGNED_SIZE)
        {
            ((struct sk_buff *) pSkbuff)->next_free = rxdma->freeSkbList;
            rxdma->freeSkbList = (struct sk_buff *) pSkbuff;
        }
    }
    rxdma->end_skbs_p = rxdma->skbs_p + (rxdma->pktDmaRxInfo.numRxBds * BCM_SKB_ALIGNED_SIZE) + 0x10;


#if defined(RXCHANNEL_PKT_RATE_LIMIT)
    /* Initialize the StdBy BD Ring */
    {
    if( (data = kmalloc(BCM_PKTBUF_SIZE, GFP_ATOMIC)) == NULL ) {
        /* release all allocated receive buffers */
        printk(KERN_NOTICE CARDNAME": Low memory.\n");
        return -ENOMEM;
    }
    rxdma->StdByBuf = data;
    rxdma->rxBdsStdBy[0].address =
             (uint32)VIRT_TO_PHY(data + BCM_PKT_HEADROOM);
    rxdma->rxBdsStdBy[0].length  = BCM_MAX_PKT_LEN;
    rxdma->rxBdsStdBy[0].status = DMA_OWN | DMA_WRAP;
    }
#endif /* defined(RXCHANNEL_PKT_RATE_LIMIT) */

    return 0;
}

/*
 *  uninit_buffers: un-initialize driver's pools of receive buffers
 */
void uninit_buffers(BcmEnet_devctrl *pDevCtrl)
{
    int i;
    BcmEnet_RxDma *rxdma = pDevCtrl->rxdma[0];

#if defined(_CONFIG_BCM_BPM)
    uint8 *rxAddr=NULL;

    /* release all allocated receive buffers */
    for (i = 0; i < rxdma->pktDmaRxInfo.numRxBds; i++)
    {
        if (bcmPktDma_EthRecvBufGet(&rxdma->pktDmaRxInfo, &rxAddr) == TRUE)
        {
            if (rxAddr != NULL)
            {
                gbpm_free_buf((void *) PDATA_TO_PFKBUFF(rxAddr,BCM_PKT_HEADROOM));
            }
        }
    }

      gbpm_unresv_rx_buf(GBPM_PORT_ETH, pDevCtrl->vport_id);
#else
    /* release all allocated receive buffers */
    for (i = 0; i < rxdma->pktDmaRxInfo.numRxBds; i++) {
        if (rxdma->buf_pool[i]) {
            kfree(rxdma->buf_pool[i]);
            rxdma->buf_pool[i] = NULL;
        }
    }
    kfree(rxdma->buf_pool);
#endif

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
    /* Free the buffer in StdBy Ring */
    kfree(rxdma->StdByBuf);
    rxdma->StdByBuf = NULL;
    /* BDs freed elsewhere - Apr 2010 */
#endif
}

static int bcm63xx_xmit_reclaim(void)
{
    pNBuff_t pNBuff;
    BcmEnet_devctrl *pDevCtrl;
    BcmPktDma_txRecycle_t txRecycle;
    BcmPktDma_txRecycle_t *txRecycle_p;
    int i;

    for (i = 0; i < iface_cnt; i++)
    {
        pDevCtrl = (BcmEnet_devctrl *)netdev_priv(vnet_dev[i]);
        /* Obtain exclusive access to transmitter.  This is necessary because
         * we might have more than one stack transmitting at once. */
        ENET_TX_LOCK();
        while ((txRecycle_p = bcmPktDma_EthFreeXmitBufGet(pDevCtrl->txdma[0], &txRecycle)) != NULL)
        {
           pNBuff = (pNBuff_t)txRecycle_p->key;

           BCM_ENET_RX_DEBUG("bcmPktDma_EthFreeXmitBufGet TRUE! (reclaim) key 0x%x\n", (int)pNBuff);
           if (pNBuff != PNBUFF_NULL) {
               ENET_TX_UNLOCK();
               nbuff_free(pNBuff);
               ENET_TX_LOCK();
           }
        }
        ENET_TX_UNLOCK();
    }
    return 0;
}

/* Forward declarations */
void __ethsw_get_txrx_imp_port_pkts(void);

void bcmeapi_enet_poll_timer(void)
{
    /* Collect CPU/IMP Port RX/TX packets every poll period */
    __ethsw_get_txrx_imp_port_pkts();

    bcm63xx_xmit_reclaim();
}

int bcmeapi_init_queue(BcmEnet_devctrl *pDevCtrl)
{
    BcmEnet_RxDma *rxdma;
    int rc = 0;
    static int skbcache_init;

    g_pEnetDevCtrl = pDevCtrl;   /* needs to be set before assign_rx_buffers is called */
    /* Get the pointer to switch DMA registers */
    switch (global.pVnetDev0_g->EnetInfo[0].sw.phyconn[pDevCtrl->vport_id])
    {
    case PHY_CONN_TYPE_EXT_PHY:
        pDevCtrl->dmaCtrl = (DmaRegs *)(ETH0_DMA_BASE);
        break;
    case PHY_CONN_TYPE_INT_PHY:
        pDevCtrl->dmaCtrl = (DmaRegs *)(ETH1_DMA_BASE);
        break;
    case PHY_CONN_TYPE_PLC:
        pDevCtrl->dmaCtrl = (DmaRegs *)(BRIDGE_DMA_BASE);
        break;
    default:
        return -EINVAL;
    }

    /* Initialize the Tx DMA software structures */
    rc = bcm63xx_init_txdma_structures(pDevCtrl);
    if (rc < 0)
    {
        return rc;
    }
    /* Initialize the Rx DMA software structures */
    rc = bcm63xx_init_rxdma_structures(pDevCtrl);
    if (rc < 0)
    {
        return rc;
    }
    /* allocate and assign tx buffer descriptors */
    rc = init_tx_channel(pDevCtrl);
    if (rc < 0)
    {
        return rc;
    }
    /* Enable the Tx channel */
    bcmPktDma_EthTxEnable(pDevCtrl->txdma[0]);
    pDevCtrl->default_txq = 0;

    /* alloc space for the rx buffer descriptors */
    rxdma = pDevCtrl->rxdma[0];
    rc = init_rx_channel(pDevCtrl);
    if (rc < 0)
    {
        return rc;
    }
    /* channel parameter is not used inside the function for Duna */
    bcmPktDma_BcmHalInterruptEnable(0, rxdma->rxIrq);
    bcmPktDma_EthRxEnable(&rxdma->pktDmaRxInfo);

    /* Initialize only one enetSkbCache for the driver */
    if (skbcache_init == 0)
    {
        /* create a slab cache for device descriptors */
        enetSkbCache = kmem_cache_create("bcm_EnetSkbCache",
                                         BCM_SKB_ALIGNED_SIZE,
                                         0, /* align */
                                         SLAB_HWCACHE_ALIGN, /* flags */
                                         NULL); /* ctor */

        if(enetSkbCache == NULL)
        {
            printk(KERN_NOTICE "Eth: Unable to create skb cache\n");
            return -ENOMEM;
        }
        skbcache_init = 1;
    }
    return rc;
}

void bcmeapi_add_proc_files(struct net_device *dev, BcmEnet_devctrl *pDevCtrl)
{
    bcmenet_add_proc_files(dev);
    dev->base_addr  = (unsigned int)pDevCtrl->rxdma[0]->pktDmaRxInfo.rxDma;
}

void bcmeapi_get_chip_idrev(unsigned int *chipid, unsigned int *chiprev)
{
    uint32 *rom_id = ioremap_nocache(PLC_ROM_CHECK_ADDR, 4);
    *chipid  = 0x60333;
    //*chiprev = (PERF->ChipID & CHIP_VERSION_MASK) >> CHIP_VERSION_SHIFT;
    *chiprev = (rom_id[0] == PLC_ROM_ID_A0) ? 0xA0 : 0xB0;
    iounmap(rom_id);
}

#if defined(_CONFIG_BCM_BPM)
/*
 * Assumptions:-
 * 1. Align data buffers on 16-byte boundary - Apr 2010
 */

/* Dumps the BPM status for Eth channels */
static void enet_bpm_status(void)
{
    int num_if;
    BcmEnet_devctrl *pDevCtrl;
    BcmPktDma_EthRxDma *rxdma;

    for (num_if = 0; num_if < iface_cnt; num_if++)
    {
        pDevCtrl = (BcmEnet_devctrl *)netdev_priv(vnet_dev[num_if]);
        rxdma = &pDevCtrl->rxdma[0]->pktDmaRxInfo;
        printk("[HOST] ENET %4d %10u %10u %5u %4u %4u\n",
                num_if, (uint32_t) rxdma->alloc,
                (uint32_t) rxdma->free,
                (uint32_t) rxdma->rxAssignedBds,
                (uint32_t) rxdma->allocTrig,
                (uint32_t) rxdma->bulkAlloc );
    }
}
#endif

#if defined(_CONFIG_BCM_BPM)
extern gbpm_status_hook_t gbpm_enet_status_hook_g;
#endif

void bcmeapi_module_init(void)
{
#if defined(_CONFIG_BCM_BPM)
    gbpm_enet_status_hook_g = enet_bpm_status;
#endif
}

void bcmeapi_module_init2(void)
{
    register_bridge_notifier(&br_notifier);
}

void bcmeapi_enet_module_cleanup(void)
{
#if defined(_CONFIG_BCM_BPM)
    gbpm_enet_status_hook_g = NULL;
#endif

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
    if (timer_pid >= 0) {
      atomic_dec(&timer_lock);
      wait_for_completion(&timer_done);
    }
#endif
}
