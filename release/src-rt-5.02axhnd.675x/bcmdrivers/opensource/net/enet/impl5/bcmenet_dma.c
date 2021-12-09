/*
<:copyright-BRCM:2010:DUAL/GPL:standard

   Copyright (c) 2010 Broadcom 
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
#include <linux/bcm_realtime.h>
#include "bcmenet.h"
#include "bcmmii.h"
#include "ethsw.h"
#include "ethsw_phy.h"
#include "bcmsw.h"
#include <linux/stddef.h>
#include <asm/atomic.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/nbuff.h>

#include "bcm_assert_locks.h"
#include "bcmPktDma.h"
#include "eth_pwrmngt.h"
#if defined(_CONFIG_ENET_BCM_TM)
#include "bcm_tm_defs.h"
#endif

#include <net/net_namespace.h>

#if defined(_CONFIG_BCM_INGQOS)
#include <linux/iqos.h>
#include "ingqos.h"
#endif
#if defined(_CONFIG_BCM_BPM)
#include <linux/gbpm.h>
#include "bpm.h"
#endif
#if defined(_CONFIG_BCM_ARL)
#include <linux/blog_rule.h>
#endif
#if defined(CONFIG_BCM_GMAC)
#include <bcmgmac.h>
#endif


#if !defined(CONFIG_BCM947189)
static FN_HANDLER_RT bcm63xx_ephy_isr(int irq, void *);
#endif
#if defined(CONFIG_BCM963268)
static FN_HANDLER_RT bcm63xx_gphy_isr(int irq, void *);
#endif


void uninit_buffers(BcmEnet_RxDma *rxdma);
static int init_buffers(BcmEnet_devctrl *pDevCtrl, int channel);
static inline void _assign_rx_buffer(BcmEnet_devctrl *pDevCtrl, int channel, uint8 * pData);

#if defined(_CONFIG_BCM_FAP) && (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
static thresh_t enet_rx_dqm_iq_thresh[ENET_RX_CHANNELS_MAX];

static void enet_iq_dqm_update_cong_status(BcmEnet_devctrl *pDevCtrl);
static void enet_iq_dqm_status(void);
#endif

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
static int bcm63xx_timer(void * arg);
#endif /* RXCHANNEL_PKT_RATE_LIMIT */
static void bcm63xx_uninit_rxdma_structures(int channel, BcmEnet_devctrl *pDevCtrl);
static int bcm63xx_init_rxdma_structures(int channel, BcmEnet_devctrl *pDevCtrl);

#if !defined(_CONFIG_BCM_FAP)
static FN_HANDLER_RT bcmeapi_enet_isr(int irq, void * pContext);
#endif /* !defined(_CONFIG_BCM_FAP) || defined(CONFIG_BCM_PKTDMA_TX_SPLITTING) */

/*
 * This macro can only be used inside enet_xmit2 because it uses the local
 * variables defined in that function.
 */

#if defined(_CONFIG_BCM_BPM) && defined(_CONFIG_BCM_FAP)
static void enet_bpm_init_tx_drop_thr(BcmEnet_devctrl *pDevCtrl, int chnl);
static void enet_bpm_set_tx_drop_thr( BcmEnet_devctrl *pDevCtrl, int chnl );
static void enet_bpm_dma_dump_tx_drop_thr(void);
static void enet_bpm_dump_tx_drop_thr(void);
/* Sanity checks */
#if (BPM_ENET_BULK_ALLOC_COUNT > FAP_BPM_ENET_BULK_ALLOC_MAX)
#error "ERROR - BPM_ENET_BULK_ALLOC_COUNT > FAP_BPM_ENET_BULK_ALLOC_MAX"
#endif
#endif

void uninit_rx_channel(BcmEnet_devctrl *pDevCtrl, int channel);
void uninit_tx_channel(BcmEnet_devctrl *pDevCtrl, int channel);
static int init_tx_channel(BcmEnet_devctrl *pDevCtrl, int channel);
static int init_rx_channel(BcmEnet_devctrl *pDevCtrl, int channel);

/* The number of rx and tx dma channels currently used by enet driver */
#if defined(CONFIG_BCM_GMAC)
   int cur_rxdma_channels = ENET_RX_CHANNELS_MAX;
   int cur_txdma_channels = ENET_TX_CHANNELS_MAX;
#else
   int cur_rxdma_channels = CONFIG_BCM_DEF_NR_RX_DMA_CHANNELS;
   int cur_txdma_channels = CONFIG_BCM_DEF_NR_TX_DMA_CHANNELS;
#endif
int next_channel[ENET_RX_CHANNELS_MAX];

#if defined (_CONFIG_BCM_FAP) && defined(_CONFIG_BCM_XTMCFG)
/* Add code for buffer quick free between enet and xtm - June 2010 */
RecycleFuncP xtm_fkb_recycle_hook = NULL;
#endif /* _CONFIG_BCM_XTMCFG && _CONFIG_BCM_XTMCFG*/

#if defined(_CONFIG_BCM_FAP)
#if defined(CONFIG_BCM963268)
RecycleFuncP xtm_skb_recycle_hook = NULL;
#endif
#endif

#if defined(_CONFIG_BCM_INGQOS)
extern uint32_t iqos_enable_g;
extern uint32_t iqos_debug_g;
extern uint32_t iqos_cpu_cong_g;

/* IQ status dump handler hook */
extern iqos_status_hook_t iqos_enet_status_hook_g;

static thresh_t enet_rx_dma_iq_thresh[ENET_RX_CHANNELS_MAX];

#if defined(_CONFIG_BCM_FAP)
static thresh_t enet_rx_dqm_iq_thresh[ENET_RX_CHANNELS_MAX];

/* FAP get Eth DQM queue length handler hook */
extern iqos_fap_ethRxDqmQueue_hook_t iqos_fap_ethRxDqmQueue_hook_g;

static void enet_iq_dqm_update_cong_status(BcmEnet_devctrl *pDevCtrl);
static void enet_iq_dqm_status(void);
#endif

static void enet_rx_set_iq_thresh( BcmEnet_devctrl *pDevCtrl, int chnl );
static void enet_rx_init_iq_thresh( BcmEnet_devctrl *pDevCtrl, int chnl );
static void enet_iq_dma_status(void);
static void enet_iq_status(void);
#endif

#if defined(_CONFIG_BCM_BPM)
static void enet_rx_set_bpm_alloc_trig( BcmEnet_devctrl *pDevCtrl, int chnl );
static int enet_bpm_alloc_buf_ring(BcmEnet_devctrl *pDevCtrl,int channel, uint32 num);
static void enet_bpm_free_buf_ring(BcmEnet_RxDma *rxdma, int channel);
#if defined(_CONFIG_BCM_FAP)
static uint16_t enet_bpm_dma_tx_drop_thr[ENET_TX_CHANNELS_MAX][ENET_TX_EGRESS_QUEUES_MAX];
#endif /* _CONFIG_BCM_FAP */
#endif /* _CONFIG_BCM_BPM */

extern BcmPktDma_Bds *bcmPktDma_Bds_p;
static int bcm63xx_xmit_reclaim(void);

#ifdef DYING_GASP_API
extern unsigned char dg_ethOam_frame[64];
extern struct sk_buff *dg_skbp;
#endif

static inline int get_rxIrq( int channel )
{
    int rxIrq;

#if defined(CONFIG_BCM_GMAC)
    if ( IsGmacInfoActive && (channel == GMAC_LOG_CHAN ) )
        rxIrq = INTERRUPT_ID_GMAC_DMA_0;
    else
#endif
        rxIrq = bcmPktDma_EthSelectRxIrq(channel);

    return rxIrq;
}

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
static void switch_rx_ring(BcmEnet_devctrl *pDevCtrl, int channel, int toStdBy);
/* default pkt rate is 100 pkts/100ms */
int rxchannel_rate_credit[ENET_RX_CHANNELS_MAX] = {100};
int rxchannel_rate_limit_enable[ENET_RX_CHANNELS_MAX] = {0};
int rxchannel_isr_enable[ENET_RX_CHANNELS_MAX] = {1};
int rx_pkts_from_last_jiffies[ENET_RX_CHANNELS_MAX] = {0};
static int last_pkt_jiffies[ENET_RX_CHANNELS_MAX] = {0};
int timer_pid = -1;
atomic_t timer_lock = ATOMIC_INIT(1);
static DECLARE_COMPLETION(timer_done);
#endif /* defined(RXCHANNEL_PKT_RATE_LIMIT) */

/* When TX iuDMA channel is used for determining the egress queue,
   this array provides the Tx iuDMA channel to egress queue mapping
   information */
int channel_for_queue[NUM_EGRESS_QUEUES] = {0};
int use_tx_dma_channel_for_priority = 0;
/* rx scheduling control and config variables */
int scheduling = WRR_SCHEDULING;
#define DEFAULT_WRR_WEIGHT_PKTS 8
int weights[ENET_RX_CHANNELS_MAX];
int weight_pkts[ENET_RX_CHANNELS_MAX] = {[0 ... (ENET_RX_CHANNELS_MAX-1)] = DEFAULT_WRR_WEIGHT_PKTS};
int pending_weight_pkts[ENET_RX_CHANNELS_MAX];
int pending_channel[ENET_RX_CHANNELS_MAX]; /* Initialization is done during module init */
int total_pkts_per_round;
int channel_ptr = 0;
int loop_index = 0;
int global_channel = 0;
int pending_ch_tbd;
int channels_tbd; /* Channels to be done for this WRR cycle */
int active_channels; /* Channels to be done with possible pending packets */
int channels_mask;
int pending_channels_mask;
static int bcm63xx_alloc_rxdma_bds(int channel, BcmEnet_devctrl *pDevCtrl);
static int bcm63xx_alloc_txdma_bds(int channel, BcmEnet_devctrl *pDevCtrl);

#if defined(RXCHANNEL_BYTE_RATE_LIMIT)
static int channel_rx_rate_limit_enable[ENET_RX_CHANNELS_MAX] = {0};
static int rx_bytes_from_last_jiffies[ENET_RX_CHANNELS_MAX] = {0};
/* default rate in bytes/sec */
int channel_rx_rate_credit[ENET_RX_CHANNELS_MAX] = {1000000};
static int last_byte_jiffies[ENET_RX_CHANNELS_MAX] = {0};
#endif /* defined(RXCHANNEL_BYTE_RATE_LIMIT) */

extern extsw_info_t extSwInfo;
extern BcmEnet_devctrl *pVnetDev0_g;

#ifdef BCM_ENET_RX_LOG
//Budget stats are useful when testing WLAN Tx Chaining feature.
// These stats provide an idea how many packets are processed per budget. 
// More the number of packets processed per budget, more probability of creating a longer chain quickly.
typedef struct {
        uint32 budgetStats_1;
        uint32 budgetStats_2to5;
        uint32 budgetStats_6to10;
        uint32 budgetStats_11to20;
        uint32 budgetStats_21tobelowBudget;
        uint32 budgetStats_budget;
    }budgetStats;

budgetStats  gBgtStats={0};
#endif

static inline uint32_t div_round_to_int(uint32_t dvdnd, uint32_t dvdr)
{
    dvdnd = 10 * dvdnd / dvdr;
    if (dvdnd % 10 < 5) {
        dvdnd /= 10;
    } else {
        dvdnd = (dvdnd + 9)/10;
    }
    return dvdnd;
}

static inline void wrr_compute_weights(void)
{
    int i;
    for (i = 0; i < ENET_TX_CHANNELS_MAX; i++)
    {
        weights[i] = div_round_to_int(weight_pkts[i]*MAX_WRR_WEIGHTS, total_pkts_per_round);
    }
}

#if defined(_CONFIG_BCM_INGQOS)
#if defined(_CONFIG_BCM_FAP)
/* print the IQ DQM status */
static void enet_iq_dqm_status(void)
{
    int chnl;
    int iqDepth = 0;

    for (chnl = 0; chnl < cur_rxdma_channels; chnl++)
    {
        BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)netdev_priv(vnet_dev[0]);
        BcmPktDma_EthRxDma *rxdma = &pDevCtrl->rxdma[chnl]->pktDmaRxInfo;

        if (g_Eth_rx_iudma_ownership[chnl] == HOST_OWNED)
            continue;

        if (iqos_fap_ethRxDqmQueue_hook_g == NULL)
            iqDepth = 0xFFFF;           /* Invalid value */
        else
            iqDepth = iqos_fap_ethRxDqmQueue_hook_g( chnl );

        printk("[DQM ] ENET %4d %5d %5d %5d %10u %8x\n",
                chnl,
                (int) rxdma->iqLoThreshDqm,
                (int) rxdma->iqHiThreshDqm,
                (int) iqDepth,
                (uint32_t)
#if defined(CC_IQ_STATS)
                rxdma->iqDroppedDqm,
#else
                0,
#endif
                iqos_cpu_cong_g
              );
    }
}
#endif

/* init ENET IQ thresholds */
static void enet_rx_init_iq_thresh(BcmEnet_devctrl *pDevCtrl, int chnl)
{
    BcmPktDma_EthRxDma *rxdma = &pDevCtrl->rxdma[chnl]->pktDmaRxInfo;
    int nr_rx_bds;

#if defined(_CONFIG_BCM_FAP)
    {
        nr_rx_bds = bcmPktDma_EthGetRxBds( rxdma, chnl );
        BCM_ASSERT(nr_rx_bds > 0);
        enet_rx_dma_iq_thresh[chnl].loThresh =
            (nr_rx_bds * IQ_ENET_LO_THRESH_PCT)/100;
        enet_rx_dma_iq_thresh[chnl].hiThresh =
            (nr_rx_bds * IQ_ENET_HI_THRESH_PCT)/100;
        BCM_ENET_RX_DEBUG("Enet: rxbds=%u, iqLoThresh=%u, iqHiThresh=%u\n",
                    nr_rx_bds,
                    enet_rx_dma_iq_thresh[chnl].loThresh,
                    enet_rx_dma_iq_thresh[chnl].hiThresh);
    }

    {/* DQM */
        nr_rx_bds = bcmPktDma_Bds_p->host.eth_rxdqm[chnl];

        enet_rx_dqm_iq_thresh[chnl].loThresh =
                        (nr_rx_bds * IQ_ENET_LO_THRESH_PCT)/100;
        enet_rx_dqm_iq_thresh[chnl].hiThresh =
                        (nr_rx_bds * IQ_ENET_HI_THRESH_PCT)/100;

        BCM_ENET_RX_DEBUG("Enet: dqm=%u, iqLoThresh=%u, iqHiThresh=%u\n",
                    nr_rx_bds,
                    enet_rx_dqm_iq_thresh[chnl].loThresh,
                    enet_rx_dqm_iq_thresh[chnl].hiThresh);
    }
#else
    {
        nr_rx_bds = bcmPktDma_EthGetRxBds( rxdma, chnl );

        enet_rx_dma_iq_thresh[chnl].loThresh =
                        (nr_rx_bds * IQ_ENET_LO_THRESH_PCT)/100;
        enet_rx_dma_iq_thresh[chnl].hiThresh =
                        (nr_rx_bds * IQ_ENET_HI_THRESH_PCT)/100;
    }
#endif
}


static void enet_rx_set_iq_thresh( BcmEnet_devctrl *pDevCtrl, int chnl )
{
    BcmPktDma_EthRxDma *rxdma = &pDevCtrl->rxdma[chnl]->pktDmaRxInfo;

    BCM_ENET_RX_DEBUG("Enet: chan=%d iqLoThresh=%d iqHiThresh=%d\n",
        chnl, (int) rxdma->iqLoThresh, (int) rxdma->iqHiThresh );

#if defined(_CONFIG_BCM_FAP)
    bcmPktDma_EthSetIqDqmThresh(rxdma,
                enet_rx_dqm_iq_thresh[chnl].loThresh,
                enet_rx_dqm_iq_thresh[chnl].hiThresh);
#endif

    bcmPktDma_EthSetIqThresh(rxdma,
                enet_rx_dma_iq_thresh[chnl].loThresh,
                enet_rx_dma_iq_thresh[chnl].hiThresh);
}

/* print the IQ status */
static void enet_iq_dma_status(void)
{
    int chnl;
    BcmPktDma_EthRxDma *rxdma;
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)netdev_priv(vnet_dev[0]);

    for (chnl = 0; chnl < cur_rxdma_channels; chnl++)
    {
        rxdma = &pDevCtrl->rxdma[chnl]->pktDmaRxInfo;

#if defined(_CONFIG_BCM_FAP)
        if (g_Eth_rx_iudma_ownership[chnl] != HOST_OWNED)
            continue;
#endif

        printk("[HOST] ENET %4d %5d %5d %5d %10u %8x\n",
               chnl,
               (int) rxdma->iqLoThresh,
               (int) rxdma->iqHiThresh,
               (rxdma->numRxBds - rxdma->rxAssignedBds),
               (uint32_t)
#if defined(CC_IQ_STATS)
               rxdma->iqDropped,
#else
               0,
#endif
               iqos_cpu_cong_g
        );
    }
}

/* print the IQ status */
static void enet_iq_status(void)
{
#if defined(_CONFIG_BCM_FAP)
    enet_iq_dqm_status();
#endif
    enet_iq_dma_status();
}
#endif


int ephy_int_cnt = 1;   /* PHY ISR interrupt count */
int ephy_int_lock = 0;      /* PHY ISR count when start link handling */

/* Called from ISR context. No sleeping locks */
extern spinlock_t bcm_ethlock_phy_shadow;
#if !defined(CONFIG_BCM947189)
void ethsw_set_mac_link_down(void)
{
    int i = 0;
    unsigned long flags;
    uint16 v16 = 0;
    uint8 v8 = 0;
    int phyId;

    spin_lock_irqsave(&bcm_ethlock_phy_shadow, flags);

    for (i = 0; i < EPHY_PORTS; i++) {
        phyId = enet_logport_to_phyid(PHYSICAL_PORT_TO_LOGICAL_PORT(i, 0));
        if(!IsExtPhyId(phyId)) {
            ethsw_phy_rreg(phyId, MII_INTERRUPT, &v16);
            if ((pVnetDev0_g->EnetInfo[0].sw.port_map & (1U<<i)) != 0) {
                if (v16 & MII_INTR_LNK) {
                    ethsw_phy_rreg(phyId, MII_BMSR, &v16);
                    if (!(v16 & BMSR_LSTATUS)) {
                        ethsw_rreg(PAGE_CONTROL, REG_PORT_STATE + i, &v8, 1);
                        if (v8 & REG_PORT_STATE_LNK) {
                            v8 &= (~REG_PORT_STATE_LNK);
                            ethsw_wreg(PAGE_CONTROL, REG_PORT_STATE + i, &v8, 1);
                        }
                    }
                }
            }
        }
    }
    spin_unlock_irqrestore(&bcm_ethlock_phy_shadow, flags);
}

static FN_HANDLER_RT bcm63xx_ephy_isr(int irq, void * dev_id)
{
    /* PHY Interrupt is disabled here. */
    ethsw_set_mac_link_down();
    ephy_int_cnt++;

    /* re-enable PHY interrupt */
    bcmeapiPhyIntEnable(1);
    return BCM_IRQ_HANDLED;
}
#endif

#if defined(CONFIG_BCM963268)
/*
 * bcm63xx_gphy_isr: Acknowledge Gphy interrupt.
 */
FN_HANDLER_RT bcm63xx_gphy_isr(int irq, void * dev_id)
{
    /* Link Interrupt is disabled here. */
    ethsw_set_mac_link_down();
    ephy_int_cnt++;

    /* re-enable PHY interrupt */
    bcmeapiPhyIntEnable(1);
    return BCM_IRQ_HANDLED;
}
#endif

int bcmeapi_map_interrupt(BcmEnet_devctrl *pDevCtrl)
{
#if defined(CONFIG_BCM963381)
    /* 963381A0 has hardware bug on interrupt; Don't register interrupt handler */
    if(pDevCtrl->chipId == 0x3381 && pDevCtrl->chipRev == 0xa0)
    {
        return 0;
    }
#endif
 
#if defined(CONFIG_BCM947189)
    return 0;
#else
    BcmHalMapInterrupt(bcm63xx_ephy_isr, (void*)pDevCtrl, INTERRUPT_ID_EPHY);
#if defined(CONFIG_BCM963268)
    BcmHalMapInterrupt(bcm63xx_gphy_isr, (void*)pDevCtrl, INTERRUPT_ID_GPHY);
#endif
#if defined(CONFIG_BCM_GMAC)
    BcmHalMapInterrupt(bcm63xx_gmac_isr, (void*)pDevCtrl, INTERRUPT_ID_GMAC);
#endif
    return BCMEAPI_INT_MAPPED_INTPHY;
#endif
}

int bcmeapi_create_vport(struct net_device *dev)
{
    return 0;
}


#if defined(CONFIG_BCM_GMAC) && defined(_CONFIG_BCM_FAP)
volatile int fapDrv_getEnetRxEnabledStatus( int channel );
#endif /* defined(CONFIG_BCM_GMAC) && defined(_CONFIG_BCM_FAP) */

void enet_rxdma_channel_enable(int chan)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(vnet_dev[0]);
    BcmEnet_RxDma *rxdma = pDevCtrl->rxdma[chan];

    /* Enable the Rx channel */
    bcmPktDma_EthRxEnable(&rxdma->pktDmaRxInfo);

#if !defined(CONFIG_BCM947189)
    /* Enable the interrupts */
    bcmPktDma_BcmHalInterruptEnable(chan, rxdma->rxIrq);
#endif

#if defined(CONFIG_BCM_GMAC) && defined(_CONFIG_BCM_FAP)
    /* Wait for Enet RX to be enabled in FAP */
    while(!fapDrv_getEnetRxEnabledStatus( chan ));
#endif

    /* Last Step, unpark queues mapped to this channel */
    enet_park_rxdma_channel(chan, 1);
}

void enet_txdma_channel_enable(int chan)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(vnet_dev[0]);

    /* Enable the Tx channel */
    bcmPktDma_EthTxEnable(pDevCtrl->txdma[chan]);
}

void bcmeapi_ethsw_cosq_rxchannel_mapping(int *channel, int queue, int set);

#if !defined(CONFIG_BCM947189)
static int enet_flush_dma_channel(int chan)
{
#define MAX_FLUSH_LOOPS 3
    int flushed = 0;
    BcmEnet_devctrl *pDevCtrl = netdev_priv(vnet_dev[0]);
    volatile DmaDesc *dmaDesc;
	BcmEnet_RxDma *rxdma = pDevCtrl->rxdma[chan];
    BcmPktDma_LocalEthRxDma * pktDmaRx = &rxdma->pktDmaRxInfo;

    for(dmaDesc = &pktDmaRx->rxBds[pktDmaRx->rxTailIndex];
            (dmaDesc->status & DMA_OWN) == 0;
            dmaDesc = &pktDmaRx->rxBds[pktDmaRx->rxTailIndex])
    {
        dmaDesc->length = BCM_MAX_PKT_LEN;
        if (pktDmaRx->rxTailIndex == (pktDmaRx->numRxBds - 1))
        {
            dmaDesc->status = DMA_OWN | DMA_WRAP;
            pktDmaRx->rxTailIndex = 0;
        }
        else
        {
            dmaDesc->status = DMA_OWN;
            pktDmaRx->rxTailIndex++;
        }

        if(++flushed >= pktDmaRx->numRxBds * MAX_FLUSH_LOOPS)
        {
            printk (" Flush Never End - Give up. numRxBds %d, MAX_FLUSH_LOOPS: %d\n", 
                pktDmaRx->numRxBds, MAX_FLUSH_LOOPS);
            flushed = -1;
            break;
        }
    }

	return flushed;
}

void enet_wait_for_dma_clean(int chan)
{
    //volatile DmaRegs *dmaRegs = (DmaRegs *)(SWITCH_DMA_BASE);
    //volatile DmaStateRam *stram;
#define MAX_QUEUE_WAITS 1000 /* Maximum waits for queue to be real empty */
#define MIN_QUEUE_CLEAN 8   /* Times of reading queue as 0 before judging queue is empty */
    int queue, ch, dmaFlushed, queCleans, queWaits, queWaitOflow = 0;
    uint16 v16;

    /* flush DMA */
    for(queWaits = 1, dmaFlushed = 1; 
        (dmaFlushed > 0 && dmaFlushed != -1) || 
        (queWaits > 0 && queWaitOflow == 0); )  /* Channel and Queue loop */
    {
        dmaFlushed = enet_flush_dma_channel(chan);

#if defined(CONFIG_BCM_GMAC)
        /* Skip GMAC channel queue waiting */
        if ((int)get_dmaCtrl(chan)== GMAC_DMA_BASE)
        {
            queWaits = 0;
            continue;
        }
#endif
        /* Wait for all queues destined to this channel to be clean */
        for (queWaits = 0, queue=0; queue < NUM_EGRESS_QUEUES; queue++) /* Queue loop */
        {
            bcmeapi_ethsw_cosq_rxchannel_mapping(&ch, queue, 0);

            if (ch == chan)
            {
                v16 = MIPS_PORT_ID;
                ethsw_wreg(PAGE_FLOW_CTRL, REG_FC_DIAG_PORT_SEL, (void*)&v16, sizeof(v16));
                for(queCleans = 0, queWaits = 0; queCleans < MIN_QUEUE_CLEAN && queWaits < MAX_QUEUE_WAITS;)
                {
                    ethsw_rreg(PAGE_FLOW_CTRL, REG_FC_Q_MON_CNT + (queue * 2), (void*)&v16, sizeof(v16));
                    if (v16 == 0)
                    {
                        queCleans++;
                    }
                    else
                    {   
                        queWaits++;
                    }
                }
                
                if (queWaits >= MAX_QUEUE_WAITS)
                {
                    printk(" Queue Waiting Never End - Give up. Channel: %d, queue %d, reg %04x, maxWaits %d, queCleans %d\n",
                        chan, queue, v16, MAX_QUEUE_WAITS, queCleans);
                    queWaitOflow++;
                }
            }
        }
    }

    /* Wait for this channel finishing receive all data */
    #if 0
    stram = &dmaRegs->stram.s[RxChanTo0BasedPhyChan(chan)];
    for (v32 = stram->state_data; v32 != stram->state_data; v32 = stram->state_data);
    #endif
}
#endif

void enet_rxdma_channel_disable(int chan)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(vnet_dev[0]);
    BcmEnet_RxDma *rxdma = pDevCtrl->rxdma[chan];

    /* Disable the interrupts */
    bcmPktDma_BcmHalInterruptDisable(chan, rxdma->rxIrq);
#if !defined(CONFIG_BCM947189)
    enet_wait_for_dma_clean(chan);

   /* Park the queue to a safe channel before disable this one */
    enet_park_rxdma_channel(chan, 0);

    /* Stop the RXDMA channel */
    if (rxdma->pktDmaRxInfo.rxDma->cfg & DMA_ENABLE)
    {
        rxdma->pktDmaRxInfo.rxDma->cfg = DMA_PKT_HALT;
        while(rxdma->pktDmaRxInfo.rxDma->cfg & DMA_ENABLE)
        {
            rxdma->pktDmaRxInfo.rxDma->cfg = DMA_PKT_HALT;
        }
    }
#endif
    /* Disable the Rx channel */
    bcmPktDma_EthRxDisable(&rxdma->pktDmaRxInfo);

#if defined(CONFIG_BCM_GMAC) && defined(_CONFIG_BCM_FAP)
    /* Wait for Enet RX to be disabled in FAP */
    while(fapDrv_getEnetRxEnabledStatus( chan ));
#endif
}

int enet_del_rxdma_channel(int chan)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(vnet_dev[0]);

    enet_rxdma_channel_disable(chan);

    /*free the BD ring */
    uninit_rx_channel(pDevCtrl, chan);

    return 0;
}

int enet_add_txdma_channel(int chan)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(vnet_dev[0]);

    if (init_tx_channel(pDevCtrl, chan))
    {
        uninit_tx_channel(pDevCtrl, chan);
        return -1;
    }

    /* Enable the Tx channel */
    bcmPktDma_EthTxEnable(pDevCtrl->txdma[chan]);

    return 0;
}

void enet_txdma_channel_disable(int chan)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(vnet_dev[0]);

    /* Disable the Tx channel */
    bcmPktDma_EthTxDisable(pDevCtrl->txdma[chan]);
}

int enet_del_txdma_channel(int chan)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(vnet_dev[0]);

    enet_txdma_channel_disable(chan);

    /*Un-allocate the BD ring */
    uninit_tx_channel(pDevCtrl, chan);

    return 0;
}

#if !defined(CONFIG_BCM947189)
/******************************************************************************
* Function: enetDmaStatus (for debug); Not called from anywhere now.          *
* Description: Dumps information about the status of the ENET IUDMA channel   *
******************************************************************************/
void enetDmaStatus(int channel)
{
    BcmPktDma_EthRxDma *rxdma;
    BcmPktDma_EthTxDma *txdma;

    rxdma = &g_pEnetDevCtrl->rxdma[channel]->pktDmaRxInfo;
    txdma = g_pEnetDevCtrl->txdma[channel];

    printk("ENET IUDMA INFO CH %d\n", channel);
    if(channel < cur_rxdma_channels)
    {
        printk("enet dmaStatus: rxdma 0x%x, cfg at 0x%x\n",
            (unsigned int)rxdma, (unsigned int)&rxdma->rxDma->cfg);


        printk("RXDMA STATUS: HeadIndex: %d TailIndex: %d numRxBds: %d rxAssignedBds: %d\n",
                  rxdma->rxHeadIndex, rxdma->rxTailIndex,
                  rxdma->numRxBds, rxdma->rxAssignedBds);

        printk("RXDMA CFG: cfg: 0x%x intStat: 0x%x intMask: 0x%x\n",
                     rxdma->rxDma->cfg,
                     rxdma->rxDma->intStat,
                     rxdma->rxDma->intMask);
    }

    if(channel < cur_txdma_channels)
    {

        printk("TXDMA STATUS: HeadIndex: %d TailIndex: %d txFreeBds: %d\n",
                  txdma->txHeadIndex,
                  txdma->txTailIndex,
                  txdma->txFreeBds);

        printk("TXDMA CFG: cfg: 0x%x intStat: 0x%x intMask: 0x%x\n",
                     txdma->txDma->cfg,
                     txdma->txDma->intStat,
                     txdma->txDma->intMask);
    }
}
#endif

#if !defined(CONFIG_BCM947189)
static int set_cur_rxdma_channels(int num_channels)
{
    int i, j, tmp_channels;
    BcmEnet_RxDma *rxdma;
    BcmEnet_devctrl *pDevCtrl = netdev_priv(vnet_dev[0]);

    if (cur_rxdma_channels == num_channels) {
        BCM_ENET_DEBUG("Not changing current rxdma channels"
                       "as it is same as what is given \n");
        return 0;
    }
    if (num_channels > ENET_RX_CHANNELS_MAX) {
        BCM_ENET_DEBUG("Not changing current rxdma channels"
                       "as it is greater than MAX (%d) \n",ENET_RX_CHANNELS_MAX);
        return 0;
    }

    /* Increasing the number of Rx channels */
    if (num_channels > cur_rxdma_channels) {
        for (i = cur_rxdma_channels; i < num_channels; i++) {
            /* Init the Rx Channel. */
            if (init_rx_channel(pDevCtrl, i)) {
                for (j = cur_rxdma_channels; j < i; j++) {
                    uninit_rx_channel(pDevCtrl, j);
                }
                return -1;
            }
            enet_rxdma_channel_enable(i);
            channels_mask |= (1 << i);
        }

        /* Set the current Rx DMA channels to given num_channels */
        cur_rxdma_channels = num_channels;

    } else { /* Decreasing the number of Rx channels */
        /* Stop the DMA channels */
        for (i = num_channels; i < cur_rxdma_channels; i++) {
            rxdma = pDevCtrl->rxdma[i];
            rxdma->pktDmaRxInfo.rxDma->cfg = 0;
        }

        /* Disable the interrupts */
        for (i = num_channels; i < cur_rxdma_channels; i++) {
            enet_rxdma_channel_disable(i);
            channels_mask &= ~(1 << i);
        }

        /* Remember the cur_rxdma_channels as we are changing it now */
        tmp_channels = cur_rxdma_channels;

        /* Set the current Rx DMA channels to given num_channels */
        /* Set this before unint_rx_channel, so that ISR will not
           try to service a channel which is uninitialized. */
        cur_rxdma_channels = num_channels;

        /* Free the buffers and BD ring */
        for (i = num_channels; i < tmp_channels; i++) {
            uninit_rx_channel(pDevCtrl, i);
        }
    }

    wrr_compute_weights();
    wrr_rx_reset();
    return 0;
}
#endif

static void setup_rxdma_channel(int channel)
{
    BcmEnet_RxDma *rxdma = global.pVnetDev0_g->rxdma[channel];
#if !defined(CONFIG_BCM947189)
    volatile DmaRegs *dmaCtrl = get_dmaCtrl( channel );
    int phy_chan = get_phy_chan( channel );
    DmaStateRam *StateRam = (DmaStateRam *)&dmaCtrl->stram.s[phy_chan*2];

    memset(StateRam, 0, sizeof(DmaStateRam));

    BCM_ENET_DEBUG("Setup rxdma channel %d, baseDesc 0x%x\n", (int)channel,
        (unsigned int)VIRT_TO_PHYS((uint32 *)rxdma->pktDmaRxInfo.rxBds));

        rxdma->pktDmaRxInfo.rxDma->cfg = 0;
        rxdma->pktDmaRxInfo.rxDma->maxBurst = DMA_MAX_BURST_LENGTH;
        rxdma->pktDmaRxInfo.rxDma->intMask = 0;
        rxdma->pktDmaRxInfo.rxDma->intStat = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;
        rxdma->pktDmaRxInfo.rxDma->intMask = DMA_DONE | DMA_NO_DESC | DMA_BUFF_DONE;

    dmaCtrl->stram.s[phy_chan * 2].baseDescPtr =
            (uint32)VIRT_TO_PHYS((uint32 *)rxdma->pktDmaRxInfo.rxBds);
#else
    rxdma->pktDmaRxInfo.rxDma->addrhigh = 0;
    rxdma->pktDmaRxInfo.rxDma->addrlow = (unsigned int)(rxdma->pktDmaRxInfo.rxBdsPhysBase);
    rxdma->pktDmaRxInfo.rxDma->control &= (~DMA_OFFSET_MASK);
    rxdma->pktDmaRxInfo.rxDma->control |= (DMA_DATA_OFFSET << DMA_OFFSET_SHIFT);
    rxdma->pktDmaRxInfo.rxDma->control |= (DMA_OVERFLOW_CONTINUE
                                           | DMA_PTY_CHK_DISABLE
                                           | (2 << DMA_PREFETCH_CTL_SHIFT)
                                           | (3 << DMA_BURST_LEN_SHIFT));
    rxdma->pktDmaRxInfo.rxDma->control |= DMA_EN;
    rxdma->pktDmaRxInfo.rxDma->ptr = (unsigned int)(rxdma->pktDmaRxInfo.rxBdsPhysBase) + (sizeof(DmaDesc) * (rxdma->pktDmaRxInfo.numRxBds - 1));
    rxdma->pktDmaRxInfo.rxTailIndex = rxdma->pktDmaRxInfo.numRxBds - 1;
    rxdma->pktDmaRxInfo.channel_init = 1;

    // enable rx interrupt
    rxdma->pktDmaRxInfo.miscReg->intmask = I_RI;

#endif
}

static void setup_txdma_channel(int channel)
{
#if !defined(CONFIG_BCM947189)
    DmaStateRam *StateRam;
    BcmPktDma_EthTxDma *txdma;
    volatile DmaRegs *dmaCtrl = get_dmaCtrl( channel );
    int phy_chan = get_phy_chan( channel );
    txdma = global.pVnetDev0_g->txdma[channel];

    StateRam = (DmaStateRam *)&dmaCtrl->stram.s[(phy_chan*2) + 1];
    memset(StateRam, 0, sizeof(DmaStateRam));

    BCM_ENET_DEBUG("setup_txdma_channel: %d, baseDesc 0x%x\n",
        (int)channel, (unsigned int)VIRT_TO_PHYS((uint32 *)txdma->txBds));

    txdma->txDma->cfg = 0;
#if defined(DBL_DESC)
    txdma->txDma->maxBurst = DMA_MAX_BURST_LENGTH | DMA_DESCSIZE_SEL;
#else
    txdma->txDma->maxBurst = DMA_MAX_BURST_LENGTH;
#endif
    txdma->txDma->intMask = 0;

    dmaCtrl->stram.s[(phy_chan * 2) + 1].baseDescPtr =
        (uint32)VIRT_TO_PHYS((uint32 *)txdma->txBds);
#else
    BcmPktDma_EthTxDma *txdma;
    txdma = global.pVnetDev0_g->txdma[channel];


    BCM_ENET_DEBUG("setup_txdma_channel: %d, baseDesc 0x%x\n",
        (int)channel, (unsigned int)VIRT_TO_PHYS((uint32 *)txdma->txBds));

    /*
     * 1 - Keep the default values for burstlen, prefetch_ctrl,
     *     prefetch_trhreshold, and multiple_outstanding_reads.
     * 2 - Configure the TX BD ring base address.
     * 2 - Enable TX channel.
     */
    txdma->txDma->addrhigh = 0;
    txdma->txDma->addrlow = (unsigned int)(txdma->txBdsPhysBase);

    /* Configure DMA controller options */
    txdma->txDma->control |= (DMA_PTY_CHK_DISABLE
                              | (1 << DMA_MULTIPLE_OUTSTANDING_READS_SHIFT)
                              | (2 << DMA_BURST_LEN_SHIFT)
                              | (3 << DMA_PREFETCH_CTL_SHIFT)
                              | (3 << DMA_PREFETCH_THRESH_SHIFT));

    txdma->txDma->control |= DMA_EN;
    /* Start with an empty descriptor table (ptr = addrlow) */
    txdma->txDma->ptr = txdma->txDma->addrlow;

#endif
}
/*
 * init_rx_channel: Initialize Rx DMA channel
 */
static int init_rx_channel(BcmEnet_devctrl *pDevCtrl, int channel)
{
    BcmEnet_RxDma *rxdma;
#if !defined(CONFIG_BCM947189)
    volatile DmaRegs *dmaCtrl = get_dmaCtrl( channel );
    int phy_chan = get_phy_chan( channel );
#else
    volatile DmaRegs *dmaCtrl;
    char *isrName;
#endif

    TRACE(("bcm63xxenet: init_rx_channel\n"));
    BCM_ENET_DEBUG("Initializing Rx channel %d \n", channel);

    /* setup the RX DMA channel */
    rxdma = pDevCtrl->rxdma[channel];

    /* init rxdma structures */
#if !defined(CONFIG_BCM947189)
    rxdma->pktDmaRxInfo.rxDma = &dmaCtrl->chcfg[phy_chan * 2];
    rxdma->rxIrq = get_rxIrq( channel );

    /* disable the interrupts from device */
    bcmPktDma_BcmHalInterruptDisable(channel, rxdma->rxIrq);

    /* Reset the DMA channel */
    dmaCtrl->ctrl_channel_reset = 1 << (phy_chan * 2);
    dmaCtrl->ctrl_channel_reset = 0;
#else
    dmaCtrl = get_dmaCtrl( rxdma->coreIndex );
    rxdma->pktDmaRxInfo.rxDma = &dmaCtrl->dmarcv;
    /* Reset the DMA channel */
    rxdma->pktDmaRxInfo.rxDma->control &= ~DMA_EN;
#endif

    /* allocate RX BDs */
#if defined(ENET_RX_BDS_IN_PSM)
    if (!rxdma->bdsAllocated)
#endif
    {
        if (bcm63xx_alloc_rxdma_bds(channel,pDevCtrl) < 0)
            return -1;
    }

   printk("ETH Init: Ch:%d - %d rx BDs at 0x%x\n",
          channel, rxdma->pktDmaRxInfo.numRxBds, (unsigned int)rxdma->pktDmaRxInfo.rxBds);

#if !defined(CONFIG_BCM947189)
    setup_rxdma_channel( channel );

    bcmPktDma_EthInitRxChan(rxdma->pktDmaRxInfo.numRxBds, &rxdma->pktDmaRxInfo);
#endif

#if defined(_CONFIG_BCM_INGQOS)
    enet_rx_set_iq_thresh( pDevCtrl, channel );
#endif
#if defined(_CONFIG_BCM_BPM)
    enet_rx_set_bpm_alloc_trig( pDevCtrl, channel );
#endif

    /* initialize the receive buffers */
    if (init_buffers(pDevCtrl, channel)) {
        printk(KERN_NOTICE CARDNAME": Low memory.\n");
        uninit_buffers(pDevCtrl->rxdma[channel]);
        return -ENOMEM;
    }
#if defined(_CONFIG_BCM_BPM)
    gbpm_resv_rx_buf( GBPM_PORT_ETH, channel, rxdma->pktDmaRxInfo.numRxBds,
        (rxdma->pktDmaRxInfo.numRxBds * BPM_ENET_ALLOC_TRIG_PCT/100) );
#endif

#if defined(CONFIG_BCM947189)
    rxdma->rxIrq = get_rxIrq( rxdma->coreIndex );
	if ((isrName = kmalloc(16, GFP_ATOMIC)) == NULL) {
		printk(KERN_ERR "kmalloc(16, GFP_ATOMIC) failed for intr name\n");
		return -1;
	}
    sprintf(isrName, "bcmenet%d", rxdma->coreIndex);
    request_irq(rxdma->rxIrq , (void *)bcmeapi_enet_isr, 0, isrName, 
                (void*)(BUILD_CONTEXT(pDevCtrl,channel)));

    setup_rxdma_channel( channel );
#endif

//    bcm63xx_dump_rxdma(channel, rxdma);
    return 0;
}

/*
 * uninit_rx_channel: un-initialize Rx DMA channel
 */
void uninit_rx_channel(BcmEnet_devctrl *pDevCtrl, int channel)
{
    BcmEnet_RxDma *rxdma;
#if !defined(CONFIG_BCM947189)
    volatile DmaRegs *dmaCtrl = get_dmaCtrl( channel );
    int phy_chan = get_phy_chan( channel );
#endif

    TRACE(("bcm63xxenet: init_rx_channel\n"));
    BCM_ENET_DEBUG("un-initializing Rx channel %d \n", channel);

    /* setup the RX DMA channel */
    rxdma = pDevCtrl->rxdma[channel];

#if defined(_CONFIG_BCM_FAP)
#if defined(CONFIG_BCM_GMAC)
    bcmPktDma_EthUnInitRxChan(&rxdma->pktDmaRxInfo);
#endif
#else
    uninit_buffers(rxdma);
#endif

#if !defined(CONFIG_BCM947189)
    /* Reset the DMA channel */
    dmaCtrl->ctrl_channel_reset = 1 << (phy_chan * 2);
    dmaCtrl->ctrl_channel_reset = 0;
#else
    /* Reset the DMA channel */
    rxdma->pktDmaRxInfo.rxDma->control &= ~DMA_EN;
#endif

#if !defined(ENET_RX_BDS_IN_PSM)
    /* remove the rx bd ring & rxBdsStdBy */
    if (rxdma->pktDmaRxInfo.rxBdsBase) {
        kfree((void *)rxdma->pktDmaRxInfo.rxBdsBase);
    }
#endif

//    bcm63xx_dump_rxdma(channel, rxdma);
}

#if defined(_CONFIG_BCM_BPM) && defined(_CONFIG_BCM_FAP)
static void enet_bpm_set_tx_drop_thr( BcmEnet_devctrl *pDevCtrl, int chnl )
{
    BcmPktDma_EthTxDma *txdma = pDevCtrl->txdma[chnl];
    int q;
    BCM_ENET_DEBUG("Enet: BPM Set Tx Chan=%d Owner=%d\n", chnl,
        g_Eth_tx_iudma_ownership[chnl]);
    if (g_Eth_tx_iudma_ownership[chnl] == HOST_OWNED)
    {
        for (q=0; q < ENET_TX_EGRESS_QUEUES_MAX; q++)
            txdma->txDropThr[q] = enet_bpm_dma_tx_drop_thr[chnl][q];
    }

    bcmPktDma_EthSetTxChanBpmThresh(txdma,
        (uint16 *) &enet_bpm_dma_tx_drop_thr[chnl]);
}
#endif

/*
 * init_tx_channel: Initialize Tx DMA channel
 */
static int init_tx_channel(BcmEnet_devctrl *pDevCtrl, int channel)
{
    BcmPktDma_EthTxDma *txdma;
#if !defined(CONFIG_BCM947189)
    volatile DmaRegs *dmaCtrl = get_dmaCtrl( channel );
    int phy_chan = get_phy_chan( channel );
#else
    volatile DmaRegs *dmaCtrl;
#endif

    TRACE(("bcm63xxenet: init_txdma\n"));
    BCM_ENET_DEBUG("Initializing Tx channel %d \n", channel);

#if !defined(CONFIG_BCM947189)
    /* Reset the DMA channel */
    dmaCtrl->ctrl_channel_reset = 1 << ((phy_chan * 2) + 1);
    dmaCtrl->ctrl_channel_reset = 0;

    txdma = pDevCtrl->txdma[channel];
    txdma->txDma = &dmaCtrl->chcfg[(phy_chan * 2) + 1];
#else
    txdma = pDevCtrl->txdma[channel];
    dmaCtrl = get_dmaCtrl( txdma->coreIndex );
    txdma->txDma = &dmaCtrl->dmaxmt;
#endif

    /* allocate and assign tx buffer descriptors */
#if defined(ENET_TX_BDS_IN_PSM)
    if (!txdma->bdsAllocated)
#endif
    {
        /* allocate TX BDs */
        if (bcm63xx_alloc_txdma_bds(channel,pDevCtrl) < 0)
        {
            printk("Allocate Tx BDs Failed ! ch %d \n", channel);
            return -1;
        }
    }

    setup_txdma_channel( channel );

    printk("ETH Init: Ch:%d - %d tx BDs at 0x%x\n", channel, txdma->numTxBds, (unsigned int)txdma->txBds);

    bcmPktDma_EthInitTxChan(txdma->numTxBds, txdma);
#if defined(_CONFIG_BCM_BPM) && defined(_CONFIG_BCM_FAP)
    enet_bpm_set_tx_drop_thr( pDevCtrl, channel );
#endif

//    bcm63xx_dump_txdma(channel, txdma);
    return 0;
}

/*
 * uninit_tx_channel: un-initialize Tx DMA channel
 */
void uninit_tx_channel(BcmEnet_devctrl *pDevCtrl, int channel)
{
    BcmPktDma_EthTxDma *txdma;
#if !defined(CONFIG_BCM947189)
    volatile DmaRegs *dmaCtrl = get_dmaCtrl( channel );
    int phy_chan = get_phy_chan( channel );
#endif

    TRACE(("bcm63xxenet: uninit_tx_channel\n"));
    BCM_ENET_DEBUG("un-initializing Tx channel %d \n", channel);

    txdma = pDevCtrl->txdma[channel];

#if defined(CONFIG_BCM_GMAC)
    bcmPktDma_EthUnInitTxChan(txdma);
#endif

#if !defined(CONFIG_BCM947189)
    /* Reset the DMA channel */
    dmaCtrl->ctrl_channel_reset = 1 << ((phy_chan * 2) + 1);
    dmaCtrl->ctrl_channel_reset = 0;
#else
    /* Reset the DMA channel */
    txdma->txDma->control &= ~DMA_EN;
#endif

#if !defined(ENET_TX_BDS_IN_PSM)
    /* remove the tx bd ring */
    if (txdma->txBdsBase) {
        kfree((void *)txdma->txBdsBase);
    }
#endif
//    bcm63xx_dump_txdma(channel, txdma);
}

#if !defined(CONFIG_BCM947189)
int enet_add_rxdma_channel(int chan)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(vnet_dev[0]);
    BcmEnet_RxDma *rxdma = pDevCtrl->rxdma[chan];

    /* Stop the RXDMA (just a precaution) */
    if (rxdma->pktDmaRxInfo.rxDma->cfg & DMA_ENABLE)
    {
        rxdma->pktDmaRxInfo.rxDma->cfg = DMA_PKT_HALT;
        while(rxdma->pktDmaRxInfo.rxDma->cfg & DMA_ENABLE)
        {
            rxdma->pktDmaRxInfo.rxDma->cfg = DMA_PKT_HALT;
        }
    }

    /* Allocate the BD ring and buffers */
    if (init_rx_channel(pDevCtrl, chan))
    {
        uninit_rx_channel(pDevCtrl, chan);
        return -1;
    }

    /* Enable the interrupts */
    bcmPktDma_BcmHalInterruptEnable(chan, rxdma->rxIrq);

    return 0;
}

static int set_cur_txdma_channels(int num_channels)
{
    int i, j, tmp_channels;
    BcmEnet_devctrl *pDevCtrl = netdev_priv(vnet_dev[0]);

#if !defined(CONFIG_BCM_GMAC) && defined(_CONFIG_BCM_FAP)
#if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING) || defined(CONFIG_BCM963268)
    if (num_channels != ENET_TX_CHANNELS_MAX)
#else
    if (num_channels != 1)
#endif
    {
        BCM_LOG_ERROR(BCM_LOG_ID_ENET, "Invalid number of Tx channels : %u\n",
                      num_channels);
        return -EINVAL;
    }
#endif

    if (cur_txdma_channels == num_channels) {
        BCM_ENET_DEBUG("Not changing current txdma channels"
                       "as it is same as what is given \n");
        return 0;
    }
    if (num_channels > ENET_TX_CHANNELS_MAX) {
        BCM_ENET_DEBUG("Not changing current txdma channels"
                       "as it is greater than max (%d) \n",ENET_TX_CHANNELS_MAX);
        return 0;
    }

    /* Increasing the number of Tx channels */
    if (num_channels > cur_txdma_channels) {
        /* Initialize the new channels */
        for (i = cur_txdma_channels; i < num_channels; i++) {
            if (init_tx_channel(pDevCtrl, i)) {
                for (j = cur_txdma_channels; j < i; j++) {
                    uninit_tx_channel(pDevCtrl, j);
                }
                return -1;
            }
        }

        for (i = cur_txdma_channels; i < num_channels; i++) {
            bcmPktDma_EthTxEnable(pDevCtrl->txdma[i]);
        }

        /* Set the current Tx DMA channels to given num_channels */
        cur_txdma_channels = num_channels;

    } else { /* Decreasing the number of Tx channels */
        for (i = num_channels; i < cur_txdma_channels && i < sizeof(pDevCtrl->txdma)/sizeof(pDevCtrl->txdma[0]); i++) {
            bcmPktDma_EthTxDisable(pDevCtrl->txdma[i]);
        }

        /* Remember the cur_txdma_channels as we are changing it now */
        tmp_channels = cur_txdma_channels;

        /* Set the current Tx DMA channels to given num_channels */
        cur_txdma_channels = num_channels;

        /*Un-allocate the BD ring */
        for (i = num_channels; i < tmp_channels; i++) {
            uninit_tx_channel(pDevCtrl, i);
        }
    }

    return 0;
}

int bcmeapi_set_num_rxques(struct ethctl_data *ethctl)
{
    int val = 0;
    if (ethctl->num_channels <= ENET_RX_CHANNELS_MAX) {
        if (ethctl->num_channels < ENET_RX_CHANNELS_MAX) {
            printk("Warning: The switch buffers will fill up "
                    "if the switch configuration is not modified "
                    "to not to send packets on disabled rx dma "
                    "channels!!! \n");
            printk("Continuing with set_rxdma_channels... \n");
        }
        if (set_cur_rxdma_channels(ethctl->num_channels)) {
            printk("Error in setting cur_rxdma_channels \n");
            return -EFAULT;
        }
    } else {
        printk("Max: %d \n", ENET_RX_CHANNELS_MAX);
        val = -EINVAL;
    }
    return val;
}

int bcmeapi_get_num_rxques(struct ethctl_data *ethctl)
{
    ethctl->ret_val = cur_rxdma_channels;
    return 0;
}

int bcmeapi_set_num_txques(struct ethctl_data *ethctl)
{
    int val = 0;
    if (ethctl->num_channels <= ENET_TX_CHANNELS_MAX) {
        if (ethctl->num_channels > 1) {
            printk("Warning: If the DUT does not support "
                    "un-aligned Tx buffers, you should not be "
                    "doing this!!! \n");
            printk("Continuing with set_txdma_channels... \n");
        }
        if (set_cur_txdma_channels(ethctl->num_channels)) {
            printk("Error in setting cur_txdma_channels \n");
            return -EFAULT;
        }
    } else {
        printk("Max: %d \n", ENET_TX_CHANNELS_MAX);
        val = -EINVAL;
    }
    return val;
}

int bcmeapi_get_num_txques(struct ethctl_data *ethctl)
{
    ethctl->ret_val = cur_txdma_channels;
    return 0;
}

int bcmeapi_config_queue(struct ethswctl_data *e)
{
    {
        struct ethswctl_data e2;
        int iudma_ch = e->val;
        int retval = 0;
        int j;

        if(e->port < BP_MAX_SWITCH_PORTS)
        {
            if(TYPE_GET == e->type)
            {
                e2.type = TYPE_GET;
                e2.port = e->port;
                e2.priority = 0;
                retval = bcmeapi_ioctl_ethsw_cosq_port_mapping(&e2);
                if(retval >= 0)
                {
                    printk("eth%d mapped to iuDMA%d\n", e2.port, retval);
                    return(0);
                }
            }
            else if(iudma_ch < ENET_RX_CHANNELS_MAX)
            {   /* TYPE_SET */
                /* The equivalent of "ethswctl -c cosq -p port -q {j} -v {iudma_ch}" */
                /* This routes packets of all priorities on eth 'port' to egress queue 'iudma_ch' */
                e2.port = e->port;
                for(j = 0; j <= MAX_PRIORITY_VALUE; j++)
                {
                    e2.type = TYPE_SET;
                    e2.priority = j;
                    e2.queue = iudma_ch;

                    retval = bcmeapi_ioctl_ethsw_cosq_port_mapping(&e2);
                }
                if(retval == 0)
                {
                    printk("eth%d mapped to iuDMA%d\n", e->port, iudma_ch);
                    return(0);
                }
            }
            else
                printk("Invalid iuDMA channel number %d\n", iudma_ch);
        }
        else
            printk("Invalid Ethernet port number %d\n", e->port);
    }
    return(BCM_E_ERROR);
}

void dump_dma_desc(DmaDesc *desc)
{
    printk("BufLen: 0x%03x, O:%d,E:%d,S:%d,W:%d, Pri: %d, ApBcmTg: %d, bufPtr: 0x%08x:%08x",
            desc->length & 0xfff,
            (desc->status & DMA_OWN)>0, (desc->status & DMA_EOP)>0,
            (desc->status & DMA_SOP)>0, (desc->status & DMA_WRAP)>0,
            (desc->status & DMA_PRIO) >> 10, (desc->status & DMA_APPEND_BRCM_TAG)>0,
            (int)desc->address, (int)phys_to_virt(desc->address));
}

static DmaDesc *get_next_desc( DmaDesc *descBase, DmaDesc *desc, int rev)
{
    if (rev)
    {
        if ( desc == descBase)
        {
            for (;desc; desc++) if (desc->status & DMA_WRAP) return desc;
        }
        else return --desc;
    }

    if (desc->status & DMA_WRAP) return descBase;
    return ++desc;
}

static void dump_dma_buf(DmaDesc *desc, int bytes)
{
    if (desc->address == 0 || (desc->address & 0xf))
    {
        printk( "Invalid buffer address, skip buffer dumping\n");
    }
    else
    {
        enet_hex_dump(phys_to_virt(desc->address), bytes==-1? desc->length&DMA_DESC_BUFLENGTH: bytes);
    }
}

static void dump_dma_desc_and_buf( DmaDesc *descBase, DmaDesc *curDesc, int cnt, int bytes)
{
    int i;
    DmaDesc *desc;

    if (cnt == -1)
    {
        printk ("Dump Full DMA Buffer Descriptor Ring:\n");
        printk( "============================================\n" );
        for( i=0, desc = descBase; ; desc++, i++)
        {
            printk("   %04d:%04x: ", i, (int)desc);
            dump_dma_desc(desc);
            printk ("%s\n", desc == curDesc? " <== Current Prossing Pointer" : "");
            printk("\n");
            if (bytes) dump_dma_buf(desc, bytes);
            if (desc->status & DMA_WRAP) break;
        }
    }
    else
    {
        printk ("Dump %d DMA Buffer Descriptor Entries BACKWARD from Current One:\n", cnt);
        printk( "============================================\n" );
        desc = curDesc;
        for (i=0; i < cnt; i++)
        {
            printk("   %04d:%04x: ", i, (int)desc);
            dump_dma_desc(desc);
            printk("\n");
            if (bytes) dump_dma_buf(desc, bytes);
            desc = get_next_desc(descBase, desc, 1);
        }
    }
}

#define DMA_FMT2 "    %-40s = "
void dump_dma_chan(int phyChan, volatile DmaRegs *dmaRegs, struct ethswctl_data *e)
{
    int v32, ringOffset;
    DmaDesc *desc, *descBase;
    volatile DmaStateRam *stram;

    printk ("DMA per channel hardware information:\n");
    printk( "============================================\n" );
    if ((phyChan & 1) == 0)
    {
        printk(DMA_FMT2 "%d\n", "Rx flow_control_en", (dmaRegs->controller_cfg & (DMA_FLOWC_CH1_EN << (phyChan/2)))>0);
        printk(DMA_FMT2 "0x%08x\n", "flow_control_low_threshold", (int)dmaRegs->flowctl_ch1_thresh_lo*(phyChan/2*3));
        printk(DMA_FMT2 "0x%08x\n", "flow_control_hi_threshold", (int)dmaRegs->flowctl_ch1_thresh_hi*(phyChan/2*3));
        printk(DMA_FMT2 "0x%08x\n", "flow_control_buffer_allocation", (int)dmaRegs->flowctl_ch1_alloc*(phyChan/2*3)); 
    }

    v32 = dmaRegs->chcfg[phyChan].cfg;
    printk(DMA_FMT2 "0x%08x\n", "DMAChannelControlConfig", v32);
    printk("         .BufHal: %d, .PktHal: %d, .EnDMA: %d\n",
            (v32&DMA_BURST_HALT)>0, (v32&DMA_PKT_HALT)>0, (v32&DMA_ENABLE)>0); 
    printk(DMA_FMT2 "0x%08x\n",  "intStat", (int)dmaRegs->chcfg[phyChan].intStat);
    printk(DMA_FMT2 "0x%08x\n", "intMask", (int)dmaRegs->chcfg[phyChan].intMask);
    printk(DMA_FMT2 "0x%08x\n",  "maxBurst", (int)dmaRegs->chcfg[phyChan].maxBurst);
    printk("\n");

    printk ("DMA State RAM\n");
    printk( "============================================\n" );
    stram = &dmaRegs->stram.s[phyChan];
    v32 = stram->state_data;
    ringOffset = v32&0x1fff;
    printk(DMA_FMT2 "0x%08x:(virtual)0x%08x\n", "RingStartAddress", 
        (int)stram->baseDescPtr, (int)phys_to_virt(stram->baseDescPtr));
    printk(DMA_FMT2 "0x%08x\n",  "StateData", v32);
    printk("        .State[b31:30]:0x%1x .ByteDone:0x%03x[b27:16] .RingOffset[b12:0]: %d:0x%04x\n",
            (v32>>30)&3, (v32>>16)&0xfff, v32&0x1fff, v32&0x1ffff);
    printk(DMA_FMT2 "0x%08x\n", "DescriptorLenStatus", (int)stram->desc_len_status);
    printk(DMA_FMT2 "0x%08x:(virtual)0x%08x\n", "CurrentBufferPointer", 
        (int)stram->desc_base_bufptr, (int)phys_to_virt(stram->desc_base_bufptr));
    printk("\n");

    descBase = (DmaDesc *) phys_to_virt(stram->baseDescPtr);
    desc = descBase + ringOffset;
    if (descBase == 0)
    {
        printk("Buffer Descriptor Ring is Null, skip dump\n");
        return;
    }

    if (descBase == 0 || ((int)descBase & 0xf))
    {
        printk("Invalid Base address, probably channel not initialized. Skip further dump.\n");
        return;
    }

    if (e->val == 0)    /* No descriptor dump request, dump default last two descriptor */
    {
        dump_dma_desc_and_buf(descBase, desc, 2, e->length==0? 48: e->length);
    }
    else
    {
        dump_dma_desc_and_buf(descBase, e->val==-1?descBase+ringOffset:desc, e->val, e->length);
    }
}

void bcmeapi_dump_queue(struct ethswctl_data *e, BcmEnet_devctrl *pDevCtrl)
{
    BcmEnet_RxDma   *rxdma;
    BcmPktDma_LocalEthRxDma * pktDmaRx;
    BcmPktDma_LocalEthTxDma * pktDmaTx;
    int phyChan, startRxChan, endRxChan, startTxChan, endTxChan, channel;
    volatile DmaRegs *dmaRegs;

    if (e->channel == ETHSW_DMA_GMAC_CHAN)
    {
#if defined(CONFIG_BCM_GMAC)
        dmaRegs = GMAC_DMA;
#else
        printk(" GMAC is not supported on this platform.\n");
        return;
#endif
    }
    else
    {
        dmaRegs = SW_DMA;
    }

    if (e->sub_type == TYPE_GET)
    {
        printk("%s DMA Global Configuration:\n",
                (e->channel == ETHSW_DMA_GMAC_CHAN)? "GMAC iuDMA": "Robo Switch iuDMAC");
        printk( "============================================\n" );
        printk("    Config: 0x%08x\n", (int)dmaRegs->controller_cfg);
        printk("        .idma_en: %d, .flow_ch1: %d, .flow_ch3: %d, .flow_ch5, %d, flow_ch7: %d\n", 
                (dmaRegs->controller_cfg & DMA_MASTER_EN)>0,
                (dmaRegs->controller_cfg & DMA_FLOWC_CH1_EN)>0,
                (dmaRegs->controller_cfg & (DMA_FLOWC_CH1_EN<<1))>0,
                (dmaRegs->controller_cfg & (DMA_FLOWC_CH1_EN<<2))>0,
                (dmaRegs->controller_cfg & (DMA_FLOWC_CH1_EN<<3))>0);
        printk("        .rxdma_protect_en: %d, state_ram_protect_en: %d\n",
                (dmaRegs->controller_cfg & (1<<5))>0,
                (dmaRegs->controller_cfg & (1<<29))>0);
        printk("    DamGlobalInterruptStatus: 0x%08x, DmaGlobalInterruptMask: 0x%08x\n",
                    (int)dmaRegs->ctrl_global_interrupt_status, (int)dmaRegs->ctrl_global_interrupt_mask);
        printk("    ChannelReset: 0x%08x\n", (int)dmaRegs->ctrl_channel_reset);
        printk("\n");

        if (e->channel == -1)
        {
            startRxChan = startTxChan = 0;
            endRxChan = ENET_RX_CHANNELS_MAX;
            endTxChan = ENET_TX_CHANNELS_MAX;
        }
        else
        {
            if (e->channel != ETHSW_DMA_GMAC_CHAN)
            {
                startRxChan = startTxChan = e->channel;
                endRxChan = endTxChan = e->channel + 1;
            }
            else
            {
                startRxChan = startTxChan = 0;
                endRxChan = endTxChan = 1;
            }
        }

        if (e->op_map & ETHSW_DMA_RX)
        {
            for(channel = startRxChan; channel < endRxChan; channel++)
            {
                rxdma = pDevCtrl->rxdma[channel];
                pktDmaRx = &rxdma->pktDmaRxInfo;


                if (pktDmaRx == NULL)
                {
                    printk("Rx Channel %d is not intialized, skip.\n", channel);
                    continue;
                }

#if defined(CONFIG_BCM_GMAC)
                if (gmac_info_pg->active == 1 && channel == GMAC_LOG_CHAN)
                {
                    printk("GMAC channel is active, skip not avaialbe RoboSwitch Channel %d Software information\n", channel);
                }
                else if (gmac_info_pg->active == 0 && e->channel == ETHSW_DMA_GMAC_CHAN)
                {
                    printk("GMAC channel is inactive, skip GMAC Channel Software Information\n");
                }
                else
#endif
                {
                    printk( "Receiving Channel %d Software Info:\n", channel);
                    printk( "=============================================\n" );
#define DMA_FMT(var, val) "    %-40s = " #val "\n", #var, (var)
#define DMA_FMT3(var, val, val2) "    %-40s = " #val":"#val2 "\n", #var, (int)(var), (int)(var)
                    printk( DMA_FMT(pktDmaRx->rxEnabled, %d));
                    printk( DMA_FMT(rxdma->rxIrq, %d));
                    printk( DMA_FMT(pktDmaRx->rxBds, 0x%p));
                    printk( DMA_FMT3(pktDmaRx->numRxBds, %d, 0x%x));
                    printk( DMA_FMT3(pktDmaRx->rxAssignedBds, %d, 0x%x));
                    printk( DMA_FMT3(pktDmaRx->rxHeadIndex, %d, 0x%04x));
                    printk( DMA_FMT3(pktDmaRx->rxTailIndex, %d, 0x%04x));
                }

#if defined(_CONFIG_BCM_FAP)
                printk(" Note: FAP is using DMA\n");
                printk( DMA_FMT3(pktDmaRx->fapIdx, %d, 0x%x));
                printk( DMA_FMT3(rxdma->bdsAllocated, %d, 0x%x));
#endif

                printk("\n");
                phyChan = RxChanTo0BasedPhyChan(channel);
                dump_dma_chan(phyChan, dmaRegs, e);
                printk("\n");
            }
        }

        if (e->op_map & ETHSW_DMA_TX)
        {
            for(channel = startTxChan; channel < endTxChan; channel++)
            {
                pktDmaTx = pDevCtrl->txdma[channel];

                if (pktDmaTx == NULL)
                {
                    printk("Tx Channel %d is not intialized, skip.\n", channel);
                    continue;
                }

#if defined(CONFIG_BCM_GMAC)
                if (gmac_info_pg->active == 1 && channel == GMAC_LOG_CHAN)
                {
                    printk("GMAC channel is active, skip not avaialbe RoboSwitch Channel %d Software information\n", channel);
                }
                else if (gmac_info_pg->active == 0 && e->channel == ETHSW_DMA_GMAC_CHAN)
                {
                    printk("GMAC channel is inactive, skip GMAC Channel Software Information\n");
                }
                else
#endif
                {
                    printk( "Transmisstion Channel %d Software Info:\n", channel);
                    printk( "===============================================\n" );
                    printk( DMA_FMT(pktDmaTx->txEnabled, %d));
                    printk( DMA_FMT(pktDmaTx->txBdsBase, 0x%p));
                    printk( DMA_FMT(pktDmaTx->txBds/*txBdsBase*/, 0x%p));
                    printk( DMA_FMT(pktDmaTx->numTxBds, %d));
                    printk( DMA_FMT(pktDmaTx->txFreeBds, %d));
                    printk( DMA_FMT(pktDmaTx->txHeadIndex, %d));
                    printk( DMA_FMT(pktDmaTx->txTailIndex, %d));
                    printk( DMA_FMT(pktDmaTx->txRecycle, 0x%p));
                }

#if defined(_CONFIG_BCM_FAP)
                printk( DMA_FMT((int)pktDmaTx->fapIdx, %d));
                printk( DMA_FMT((int)pktDmaTx->bdsAllocated, %d));
#endif

                printk("\n");
                phyChan = TxChanTo0BasedPhyChan(channel);
                dump_dma_chan(phyChan, dmaRegs, e);
                printk("\n");
            }
        }

#if defined(CONFIG_BCM_GMAC)
        if (e->channel == -1)
        {
            e->channel = ETHSW_DMA_GMAC_CHAN;    
            bcmeapi_dump_queue(e, pDevCtrl);
            e->channel = -1;    
        }
#endif

        return;
    }

    if (e->channel==ETHSW_DMA_GMAC_CHAN)
    {
#if defined(CONFIG_BCM_GMAC)
        channel = GMAC_LOG_CHAN;
#else
        printk(" Error: GMAC does not exist in this platform\n");
        return;
#endif
    }
    else
    {
        channel = e->channel;
    }

    if (e->sub_type == TYPE_ENABLE)
    {
        if (e->op_map & ETHSW_DMA_RX) enet_rxdma_channel_enable(channel);
        if (e->op_map & ETHSW_DMA_TX) enet_txdma_channel_enable(channel);
        return;
    }

    if (e->sub_type == TYPE_DISABLE)
    {
        if (e->op_map & ETHSW_DMA_RX) enet_rxdma_channel_disable(channel);
        if (e->op_map & ETHSW_DMA_TX) enet_txdma_channel_disable(channel);
        return;
    }
}

int bcmeapi_ioctl_test_config(struct ethswctl_data *e)
{
    if (e->type == TYPE_GET)
    {
        if (e->sub_type == SUBTYPE_ISRCFG)
        {
#if defined(RXCHANNEL_PKT_RATE_LIMIT)
            BCM_ENET_DEBUG("Given channel: 0x%02x \n ", e->channel);
            e->ret_val = rxchannel_isr_enable[e->channel];
#endif
        }
        else if (e->sub_type == SUBTYPE_RXDUMP)
        {
            e->ret_val = global.dump_enable;
        }

        BCM_ENET_DEBUG("e->ret_val: 0x%02x \n ", e->ret_val);
    }
    else
    {
        if (e->sub_type == SUBTYPE_ISRCFG)
        {
#if defined(RXCHANNEL_PKT_RATE_LIMIT)
            BcmEnet_RxDma *rxdma;
            BcmEnet_devctrl *pDevCtrl = netdev_priv(vnet_dev[0]);
            BcmPktDma_LocalEthRxDma * local_rxdma;

            BCM_ENET_DEBUG("Given channel: 0x%02x \n ", e->channel);
            BCM_ENET_DEBUG("Given val: %d \n ", e->val);
            rxchannel_isr_enable[e->channel] = e->val;

            /* Enable/Disable the interrupts for given RX DMA channel */
            rxdma = pDevCtrl->rxdma[e->channel];
            local_rxdma = &rxdma->pktDmaRxInfo;
            if (e->val)
            {
                enet_rxdma_channel_enable(e->channel);
            }
            else
            {
                enet_rxdma_channel_disable(e->channel);
            }
#endif
        }
        else if (e->sub_type == SUBTYPE_RXDUMP)
        {
            global.dump_enable = e->val;
#ifdef BCM_ENET_RX_LOG
    #define PERCENT(a) (((gBgtStats.a)*100)/totalpkts)
            printk("Dumping BudgetStats\n");
            printk("budgetStats_1 %u budgetStats_2to5 %u budgetStats_6to10 %u budgetStats_11to20 %u budgetStats_21tobelowBudget %u budgetStats_budget %u\n", 
                   gBgtStats.budgetStats_1, gBgtStats.budgetStats_2to5, gBgtStats.budgetStats_6to10, 
                   gBgtStats.budgetStats_11to20, gBgtStats.budgetStats_21tobelowBudget, gBgtStats.budgetStats_budget);
            {
                uint32 totalpkts=0;
                totalpkts= gBgtStats.budgetStats_1 + gBgtStats.budgetStats_2to5 + gBgtStats.budgetStats_6to10 + 
                           gBgtStats.budgetStats_11to20 + gBgtStats.budgetStats_21tobelowBudget + gBgtStats.budgetStats_budget;
                if (totalpkts != 0)
                {
                    printk("budgetStatsPer_1 %u budgetStatsPer_2to5 %u budgetStatsPer_6to10 %u budgetStatsPer_11to20 %u budgetStatsPer_21tobelowBudget %u budgetStatsPer_budget %u\n",
                           PERCENT(budgetStats_1), PERCENT(budgetStats_2to5), PERCENT(budgetStats_6to10), PERCENT(budgetStats_11to20), PERCENT(budgetStats_21tobelowBudget),
                           PERCENT(budgetStats_budget));
                }
            }
#endif			
        }
    }

    /* Note : handling of RESETMIB is moved outside of this function into the caller */
    if (e->sub_type == SUBTYPE_RESETSWITCH)
    {
    }
    return 0;
}

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
int bcmeapi_ioctl_rx_pkt_rate_limit_config(struct ethswctl_data *e)
{
    BcmEnet_RxDma *rxdma;
    BcmEnet_devctrl *pDevCtrl = netdev_priv(vnet_dev[0]);

    BCM_ENET_DEBUG("Given channel: %d \n ", e->channel);
    if (e->channel >= ENET_RX_CHANNELS_MAX || e->channel < 0) {
        return -EINVAL;
    }
    if (e->type == TYPE_GET) {
        e->ret_val = rxchannel_rate_limit_enable[e->channel];
        BCM_ENET_DEBUG("e->ret_val: 0x%02x \n ", e->ret_val);
    } else {
        BCM_ENET_DEBUG("Given rate_enable_cfg: %d \n ", e->val);
        rxdma = pDevCtrl->rxdma[e->channel];
        ENET_RX_LOCK();
        rxchannel_rate_limit_enable[e->channel] = e->val;
        if ((e->val == 0) && (rxchannel_isr_enable[e->channel] == 0)) {
            switch_rx_ring(pDevCtrl, e->channel, 0);
            bcmPktDma_BcmHalInterruptEnable(e->channel, rxdma->rxIrq);
            rxchannel_isr_enable[e->channel] = 1;
        }
        ENET_RX_UNLOCK();
    }

    return 0;
}

int bcmeapi_ioctl_rx_pkt_rate_config(struct ethswctl_data *e)
{
    BCM_ENET_DEBUG("Given channel: 0x%02x \n ", e->channel);
    if (e->type == TYPE_GET) {
        e->ret_val = rxchannel_rate_credit[e->channel] * 10;
        BCM_ENET_DEBUG("e->ret_val: 0x%02x \n ", e->ret_val);
    } else {
        BCM_ENET_DEBUG("Given rate: %d \n ", e->val);
        rxchannel_rate_credit[e->channel] = (e->val/10 > 1)?(e->val/10):1;
    }

    return 0;
}
#endif /* defined(RXCHANNEL_PKT_RATE_LIMIT) */

#if defined(RXCHANNEL_BYTE_RATE_LIMIT)
int bcmeapi_ioctl_rx_rate_config(struct ethswctl_data *e)
{
    BCM_ENET_DEBUG("Given channel: 0x%02x \n ", e->channel);
    if (e->type == TYPE_GET) {
        e->ret_val = channel_rx_rate_credit[e->channel];
        BCM_ENET_DEBUG("e->ret_val: 0x%02x \n ", e->ret_val);
    } else {
        BCM_ENET_DEBUG("Given rate: %d \n ", e->val);
        channel_rx_rate_credit[e->channel] = e->val;
    }

    return 0;
}

static int bcmeapi_ioctl_rx_rate_limit_config(struct ethswctl_data *e)
{
    BCM_ENET_DEBUG("Given channel: %d \n ", e->channel);
    if (e->type == TYPE_GET) {
        e->ret_val = channel_rx_rate_limit_enable[e->channel];
        BCM_ENET_DEBUG("e->ret_val: 0x%02x \n ", e->ret_val);
    } else {
        BCM_ENET_DEBUG("Given rate_enable_cfg: %d \n ", e->val);
        channel_rx_rate_limit_enable[e->channel % ENET_RX_CHANNELS_MAX] = e->val;
    }

    return 0;
}
#endif

int bcmeapi_ioctl_ethsw_wrrparam(struct ethswctl_data *e)
{
    int i;
    int total_of_weights = 0;

    if (e->type == TYPE_GET) {
        e->max_pkts_per_iter = total_pkts_per_round;
        e->rx_queues = cur_rxdma_channels;
        memset(e->weights, 0, sizeof(e->weights));
        memset(e->weight_pkts, 0, sizeof(e->weights));
        memcpy(e->weights, weights, sizeof(e->weights[0])*cur_rxdma_channels);
        memcpy(e->weight_pkts, weight_pkts, sizeof(e->weight_pkts[0])*cur_rxdma_channels);
    } else {
        total_pkts_per_round = e->max_pkts_per_iter;
        for(i=0; i<cur_rxdma_channels; i++) {
            weights[i] = e->weights[i];
        }

        total_of_weights = 0;
        for(i=0; i<cur_rxdma_channels; i++) {
            total_of_weights += weights[i];
        }

        for(i=0; i<cur_rxdma_channels; i++) {
            /* Round to closest integer so that total pkts per round will be met */
            weight_pkts[i] = div_round_to_int(weights[i]*total_pkts_per_round, total_of_weights);
        }

        wrr_rx_reset();
    }
    return 0;
}

int bcmeapi_ioctl_ethsw_rxscheduling(struct ethswctl_data *e)
{
    int i;

    if (e->type == TYPE_GET) {
        e->scheduling = scheduling;
    } else {
        if (e->scheduling == WRR_SCHEDULING) {
            scheduling = WRR_SCHEDULING;
            for(i=0; i < ENET_RX_CHANNELS_MAX; i++) {
                pending_weight_pkts[i] = weight_pkts[i];
                pending_channel[i] = i;
            }
            /* reset the other scheduling variables */
            global_channel = channel_ptr = loop_index = 0;
            pending_ch_tbd = cur_rxdma_channels;
        } else if (e->scheduling == SP_SCHEDULING) {
            global_channel = cur_rxdma_channels - 1;
            scheduling = SP_SCHEDULING;
        } else {
            return -EFAULT;
        }
    }
    return 0;
}
#else
int enet_add_rxdma_channel(int chan)
{
    return 0;
}

int bcmeapi_set_num_rxques(struct ethctl_data *ethctl)
{
    int val = 0;
    return val;
}

int bcmeapi_get_num_rxques(struct ethctl_data *ethctl)
{
    return 0;
}

int bcmeapi_set_num_txques(struct ethctl_data *ethctl)
{
    int val = 0;
    return val;
}

int bcmeapi_get_num_txques(struct ethctl_data *ethctl)
{
    return 0;
}

int bcmeapi_config_queue(struct ethswctl_data *e)
{
    return(0);
}

void dump_dma_desc(DmaDesc *desc)
{
}


void dump_dma_chan(int phyChan, volatile DmaRegs *dmaRegs, struct ethswctl_data *e)
{
}

void bcmeapi_dump_queue(struct ethswctl_data *e, BcmEnet_devctrl *pDevCtrl)
{
}

int bcmeapi_ioctl_test_config(struct ethswctl_data *e)
{
    return 0;
}

int bcmeapi_ioctl_ethsw_wrrparam(struct ethswctl_data *e)
{
    return 0;
}

int bcmeapi_ioctl_ethsw_rxscheduling(struct ethswctl_data *e)
{
    return 0;
}
#endif

/*
 * Wrapper function for other Kernel modules to check
 * if a given logical port is configured for software switching.
 */
static int bcmeapi_is_swswitch_port(void *ctxt)
{
    char *ifname = (char *)ctxt;
    BcmEnet_devctrl *pDevCtrl;
    int ret;
    int unit;
    int port;
    int logPort;

    ret    = bcm63xx_enet_getPortFromName(ifname, &unit, &port);
    if(ret < 0)
    {
        return 0;
    }

    pDevCtrl = netdev_priv(vnet_dev[0]);
    logPort = PHYSICAL_PORT_TO_LOGICAL_PORT(port, unit);
    return ((pDevCtrl->softSwitchingMap >> logPort) & 0x1);
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
#endif

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
    if (timer_pid < 0)
        return -ENOMEM;
#endif

    return 0;
}


void bcmeapi_module_init2(void)
{

    /* Register ARL Entry clear routine */
    bcmFun_reg(BCM_FUN_IN_ENET_CLEAR_ARL_ENTRY, remove_arl_entry_wrapper);
#if defined(CONFIG_BCM_GMAC)
    bcmFun_reg(BCM_FUN_ID_ENET_GMAC_ACTIVE, ChkGmacActive);
    bcmFun_reg(BCM_FUN_ID_ENET_GMAC_PORT, ChkGmacPort);
#endif
    bcmFun_reg(BCM_FUN_ID_ENET_IS_SWSWITCH_PORT, bcmeapi_is_swswitch_port);

#if defined(_CONFIG_BCM_FAP)
    /* Add code for buffer quick free between enet and xtm - June 2010 */
    bcmPktDma_set_enet_recycle((RecycleFuncP)bcm63xx_enet_recycle);
#endif /* defined(_CONFIG_BCM_FAP) */

}

void bcmeapi_enet_module_cleanup(void)
{
#if defined(_CONFIG_BCM_ARL)
    bcm_arl_process_hook_g = NULL;
#endif

#if defined(_CONFIG_BCM_INGQOS)
    iqos_enet_status_hook_g = NULL;
#endif

#if defined(_CONFIG_BCM_BPM)
    gbpm_g.enet_status = (gbpm_enet_status_hook_t)NULL;
#if defined(_CONFIG_BCM_FAP)
    gbpm_g.enet_thresh = (gbpm_enet_thresh_hook_t)NULL;
#endif
#endif



#if defined(RXCHANNEL_PKT_RATE_LIMIT)
    if (timer_pid >= 0) {
      atomic_dec(&timer_lock);
      wait_for_completion(&timer_done);
    }
#endif
}

void bcmeapi_free_irq(BcmEnet_devctrl *pDevCtrl)
{
#if !defined(CONFIG_BCM947189)
    free_irq(INTERRUPT_ID_EPHY, pDevCtrl);
#endif
#if defined(CONFIG_BCM963268)
    free_irq(INTERRUPT_ID_GPHY, pDevCtrl);
#endif
#if defined(CONFIG_BCM_GMAC)
    free_irq(INTERRUPT_ID_GMAC, pDevCtrl);
#endif
}

void bcmeapi_add_proc_files(struct net_device *dev, BcmEnet_devctrl *pDevCtrl)
{
    bcmenet_add_proc_files(dev);
    dev->base_addr  = (unsigned int)pDevCtrl->rxdma[0]->pktDmaRxInfo.rxDma;
}

void bcmeapi_get_chip_idrev(unsigned int *chipid, unsigned int *chiprev)
{
#if defined(CONFIG_BCM947189)
    *chipid  = (MISC->chipid & CID_ID_MASK);
    *chiprev = (MISC->chipid & CID_REV_MASK) >> CID_REV_SHIFT;
#else
    *chipid  = (PERF->RevID & CHIP_ID_MASK) >> CHIP_ID_SHIFT;
    *chiprev = (PERF->RevID & REV_ID_MASK);
#endif
}

static void bcm63xx_uninit_txdma_structures(int channel, BcmEnet_devctrl *pDevCtrl)
{
    BcmPktDma_EthTxDma *txdma;
#if !defined(_CONFIG_BCM_FAP)
    int nr_tx_bds = bcmPktDma_Bds_p->host.eth_txbds[channel];
#endif

    txdma = pDevCtrl->txdma[channel];

    /* disable DMA */
    bcmPktDma_EthTxDisable(txdma);

#if !defined(_CONFIG_BCM_FAP)
    /* if any, free the tx skbs */
    while (txdma->txFreeBds < nr_tx_bds) {
        txdma->txFreeBds++;
        nbuff_free((void *)txdma->txRecycle[txdma->txHeadIndex++].key);
        if (txdma->txHeadIndex == nr_tx_bds)
            txdma->txHeadIndex = 0;
    }
#endif

    /* free the transmit buffer descriptor ring */
    txdma = pDevCtrl->txdma[channel];
#if !defined(ENET_TX_BDS_IN_PSM)
    /* remove the tx bd ring */
    if (txdma->txBdsBase) {
        kfree((void *)txdma->txBdsBase);
    }
#endif

    /* free the txdma channel structures */
    for (channel = 0; channel < ENET_TX_CHANNELS_MAX; channel++) {
        if (pDevCtrl->txdma[channel]) {
            kfree((void *)(pDevCtrl->txdma[channel]));
        }
   }
}

void bcmeapi_free_queue(BcmEnet_devctrl *pDevCtrl)
{
    int i;

    /* Free the Tx DMA software structures */
    for (i = 0; i < ENET_TX_CHANNELS_MAX; i++) {
        bcm63xx_uninit_txdma_structures(i, pDevCtrl);
    }

    /* Free the Rx DMA software structures and packet buffers*/
    for (i = 0; i < ENET_RX_CHANNELS_MAX; i++) {
        bcm63xx_uninit_rxdma_structures(i, pDevCtrl);
#if defined(_CONFIG_BCM_BPM)
        gbpm_unresv_rx_buf( GBPM_PORT_ETH, i );
#endif
    }

#if !defined(CONFIG_BCM947189)
    bcmenet_del_proc_files(pDevCtrl->dev);
#endif
}

static void bcm63xx_uninit_rxdma_structures(int channel, BcmEnet_devctrl *pDevCtrl)
{
    BcmEnet_RxDma *rxdma;

    rxdma = pDevCtrl->rxdma[channel];
#if !defined(CONFIG_BCM947189)
    rxdma->pktDmaRxInfo.rxDma->cfg = 0;
#endif
    enet_rxdma_channel_disable(channel);

#if !defined(_CONFIG_BCM_FAP)
    {
        /* free the IRQ */
        {
            int rxIrq = bcmPktDma_EthSelectRxIrq(channel);

            /* disable the interrupts from device */
            bcmPktDma_BcmHalInterruptDisable(channel, rxIrq);
            free_irq(rxIrq, (BcmEnet_devctrl *)BUILD_CONTEXT(pDevCtrl,channel));

#if defined(CONFIG_BCM_GMAC)
            if ( gmac_info_pg->enabled && (channel == GMAC_LOG_CHAN) )
            {
                rxIrq = INTERRUPT_ID_GMAC_DMA_0;

                /* disable the interrupts from device */
                bcmPktDma_BcmHalInterruptDisable(channel, rxIrq);
                free_irq(rxIrq,
                        (BcmEnet_devctrl *)BUILD_CONTEXT(pDevCtrl,channel));
            }
#endif
        }
    }
#endif

    /* release allocated receive buffer memory */
    uninit_buffers(rxdma);

    /* free the receive buffer descriptor ring */
#if !defined(ENET_RX_BDS_IN_PSM)
    if (rxdma->pktDmaRxInfo.rxBdsBase) {
        kfree((void *)rxdma->pktDmaRxInfo.rxBdsBase);
    }
#endif

    /* free the rxdma channel structures */
    if (pDevCtrl->rxdma[channel]) {
        kfree((void *)(pDevCtrl->rxdma[channel]));
    }
}

static int bcm63xx_init_txdma_structures(int channel, BcmEnet_devctrl *pDevCtrl)
{
    BcmPktDma_EthTxDma *txdma;

    pDevCtrl->txdma[channel] = (BcmPktDma_EthTxDma *) (kzalloc(
                           sizeof(BcmPktDma_EthTxDma), GFP_KERNEL));
    if (pDevCtrl->txdma[channel] == NULL) {
        printk("Unable to allocate memory for tx dma rings \n");
        return -ENXIO;
    }

    BCM_ENET_DEBUG("The txdma is 0x%p \n", pDevCtrl->txdma[channel]);

    txdma = pDevCtrl->txdma[channel];
    txdma->channel = channel;

#if defined(_CONFIG_BCM_FAP)
    txdma->fapIdx = getFapIdxFromEthTxIudma(channel);
    txdma->bdsAllocated = 0;
#endif

#if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
    txdma->txOwnership = g_Eth_tx_iudma_ownership[channel];
#endif

#if defined(_CONFIG_BCM_BPM)
#if defined(_CONFIG_BCM_FAP)
   enet_bpm_init_tx_drop_thr( pDevCtrl, channel );
#endif
#endif

    /* init number of Tx BDs in each tx ring */
    txdma->numTxBds = bcmPktDma_EthGetTxBds( txdma, channel );

    BCM_ENET_DEBUG("Enet: txbds=%u \n", txdma->numTxBds);
    return 0;
}

int bcmeapi_init_queue(BcmEnet_devctrl *pDevCtrl)
{
#if !defined(CONFIG_BCM947189)
    BcmEnet_RxDma *rxdma;
    volatile DmaRegs *dmaCtrl;
    int phy_chan;
    int i, rc = 0;

    for(i=0; i<cur_rxdma_channels; i++) total_pkts_per_round += weight_pkts[i];
    wrr_compute_weights();
    wrr_rx_reset();

    g_pEnetDevCtrl = pDevCtrl;   /* needs to be set before assign_rx_buffers is called */
    /* Get the pointer to switch DMA registers */
    pDevCtrl->dmaCtrl = (DmaRegs *)(SWITCH_DMA_BASE);
#if defined(CONFIG_BCM_GMAC)
    pDevCtrl->gmacDmaCtrl = (DmaRegs *)(GMAC_DMA_BASE);
    BCM_ENET_DEBUG("GMAC: gmacDmaCtrl is 0x%x\n",
        (unsigned int)pDevCtrl->gmacDmaCtrl);
#endif /* defined(CONFIG_BCM_GMAC) */

    /* Initialize the Tx DMA software structures */
    for (i = 0; i < ENET_TX_CHANNELS_MAX; i++) {
        rc = bcm63xx_init_txdma_structures(i, pDevCtrl);
        if (rc < 0)
            return rc;
    }

    /* Initialize the Rx DMA software structures */
    for (i = 0; i < ENET_RX_CHANNELS_MAX; i++) {
        rc = bcm63xx_init_rxdma_structures(i, pDevCtrl);

        if (rc < 0)
            return rc;
    }

    /* allocate and assign tx buffer descriptors */
    for (i=0; i < cur_txdma_channels; ++i)
    {
        rc = init_tx_channel(pDevCtrl, i);
        if (rc < 0)
        {
            return rc;
        }

        /* Enable the Tx channel */
        bcmPktDma_EthTxEnable(pDevCtrl->txdma[i]);
    }

    pending_ch_tbd = cur_rxdma_channels;
    for (i = 0; i < cur_rxdma_channels; i++) {
        channels_mask |= (1 << i);
    }
    pending_channels_mask = channels_mask;
    pDevCtrl->default_txq = 0;

    /* alloc space for the rx buffer descriptors */
    for (i = 0; i < cur_rxdma_channels; i++)
    {
        rxdma = pDevCtrl->rxdma[i];

        rc = init_rx_channel(pDevCtrl, i);
        if (rc < 0)
        {
            return rc;
        }

        enet_rxdma_channel_enable(i);
    }

    for (i=0;i<cur_rxdma_channels;i++)
    {
        rxdma = pDevCtrl->rxdma[i];
        dmaCtrl = get_dmaCtrl( i );
        phy_chan = get_phy_chan( i );

#if defined(_CONFIG_BCM_FAP)
#if defined(CONFIG_BCM_PORTS_ON_INT_EXT_SW)
    bcmPktDma_EthInitExtSw(extSwInfo.connected_to_internalPort);
#endif
#endif
    }

    /* Workaround for 4ke */
    cache_flush_len(pDevCtrl, sizeof(BcmEnet_devctrl));
#else
    int index, rc = 0;
    int channel = 0;

    g_pEnetDevCtrl = pDevCtrl;   /* needs to be set before assign_rx_buffers is called */

    for (index = 0; index < BP_MAX_ENET_MACS; index ++) {
        if ((pDevCtrl->EnetInfo[0].sw.port_map & (1 << index)) == 0 && index)
            continue;

        /* Initialize the Tx DMA software structures */
        rc = bcm63xx_init_txdma_structures(channel, pDevCtrl);
        if (rc < 0)
            return rc;
        pDevCtrl->txdma[channel]->coreIndex = index;
   
        /* Initialize the Rx DMA software structures */
        rc = bcm63xx_init_rxdma_structures(channel, pDevCtrl);
        if (rc < 0)
            return rc;
        pDevCtrl->rxdma[channel]->coreIndex = index;
        if (index == 0)
            pDevCtrl->rxdma[channel]->pktDmaRxInfo.miscReg = ENET_CORE0_MISC;
        else
            pDevCtrl->rxdma[channel]->pktDmaRxInfo.miscReg = ENET_CORE1_MISC;


        /* allocate and assign tx buffer descriptors */
        rc = init_tx_channel(pDevCtrl, channel);
        if (rc < 0)
        {
            return rc;
        }

        /* Enable the Tx channel */
        bcmPktDma_EthTxEnable(pDevCtrl->txdma[channel]);

        /* alloc space for the rx buffer descriptors */
        rc = init_rx_channel(pDevCtrl, channel);
        if (rc < 0)
        {
            return rc;
        }

        enet_rxdma_channel_enable(channel);

        channel++;
    }

    cur_txdma_channels = channel;
    cur_rxdma_channels = channel;
    pending_ch_tbd = cur_rxdma_channels;
    for (index = 0; index < cur_rxdma_channels; index++) {
        channels_mask |= (1 << index);
    }
    pending_channels_mask = channels_mask;
    pDevCtrl->default_txq = 0;

#endif

    return rc;
}


/*
 *  init_buffers: initialize driver's pools of receive buffers
 */
static int init_buffers(BcmEnet_devctrl *pDevCtrl, int channel)
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

    RECYCLE_CONTEXT(context)->channel = channel;

    TRACE(("bcm63xxenet: init_buffers\n"));

    /* allocate recieve buffer pool */
    rxdma = pDevCtrl->rxdma[channel];
    /* Local copy of these vars also initialized to zero in bcmPktDma channel init */
    rxdma->pktDmaRxInfo.rxAssignedBds = 0;
    rxdma->pktDmaRxInfo.rxHeadIndex = rxdma->pktDmaRxInfo.rxTailIndex = 0;
    BufsToAlloc = rxdma->pktDmaRxInfo.numRxBds;

#if defined(_CONFIG_BCM_BPM)
    if (enet_bpm_alloc_buf_ring(pDevCtrl, channel, BufsToAlloc) == GBPM_ERROR)
    {
        printk(KERN_NOTICE "Eth: Low memory.\n");

        /* release all allocated receive buffers */
        enet_bpm_free_buf_ring(rxdma, channel);
        return -ENOMEM;
    }
#else
    if( (rxdma->buf_pool = kzalloc(BufsToAlloc * sizeof(uint32_t) + 0x10,
        GFP_ATOMIC)) == NULL )
    {
        printk(KERN_NOTICE "Eth: Low memory.\n");
        return -ENOMEM;
    }

    while( BufsToAlloc ) {
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
            flush_assign_rx_buffer(pDevCtrl, channel, /* headroom not flushed */
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
                    (rxdma->pktDmaRxInfo.numRxBds * BCM_SKB_ALIGNED_SIZE) + BCM_DCACHE_LINE_LEN,
                    GFP_ATOMIC)) == NULL )
        return -ENOMEM;

    memset(rxdma->skbs_p, 0,
                (rxdma->pktDmaRxInfo.numRxBds * BCM_SKB_ALIGNED_SIZE) + BCM_DCACHE_LINE_LEN);

    rxdma->freeSkbList = NULL;

    /* Chain socket skbs */
    for(i = 0, pSkbuff = (unsigned char *)
        (((unsigned long) rxdma->skbs_p + BCM_DCACHE_ALIGN_LEN) & ~BCM_DCACHE_ALIGN_LEN);
            i < rxdma->pktDmaRxInfo.numRxBds; i++, pSkbuff += BCM_SKB_ALIGNED_SIZE)
    {
        ((struct sk_buff *) pSkbuff)->next_free = rxdma->freeSkbList;
        rxdma->freeSkbList = (struct sk_buff *) pSkbuff;
    }
 }
 rxdma->end_skbs_p = rxdma->skbs_p + (rxdma->pktDmaRxInfo.numRxBds * BCM_SKB_ALIGNED_SIZE) + BCM_DCACHE_LINE_LEN;


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
             (uint32)VIRT_TO_PHYS(data + BCM_PKT_HEADROOM);
    rxdma->rxBdsStdBy[0].length  = BCM_MAX_PKT_LEN;
    rxdma->rxBdsStdBy[0].status = DMA_OWN | DMA_WRAP;
    }
#endif /* defined(RXCHANNEL_PKT_RATE_LIMIT) */

    return 0;
}

/*
 *  uninit_buffers: un-initialize driver's pools of receive buffers
 */
void uninit_buffers(BcmEnet_RxDma *rxdma)
{
    int i;

#if defined(_CONFIG_BCM_BPM)
    int channel;
    uint32 context=0;
    uint8 *rxAddr=NULL;

    channel  = RECYCLE_CONTEXT(context)->channel;

    /* release all allocated receive buffers */
    for (i = 0; i < rxdma->pktDmaRxInfo.numRxBds; i++)
    {
#if !defined(_CONFIG_BCM_FAP)
        if (bcmPktDma_EthRecvBufGet(&rxdma->pktDmaRxInfo, &rxAddr) == TRUE)
#endif
        {
            if (rxAddr != NULL)
            {
                gbpm_free_buf((void *)rxAddr);
            }
        }
    }

#if defined(_CONFIG_BCM_BPM)
      gbpm_unresv_rx_buf( GBPM_PORT_ETH, channel );
#endif
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

#if 0   /* CAUTION!!! DONOT free SKB pool */
    kfree(rxdma->skbs_p);
#endif

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
    /* Free the buffer in StdBy Ring */
    kfree(rxdma->StdByBuf);
    rxdma->StdByBuf = NULL;
    /* BDs freed elsewhere - Apr 2010 */
#endif
}

#if 0   /* For debug */
static int bcm63xx_dump_rxdma(int channel, BcmEnet_RxDma *rxdma )
{
    BcmPktDma_EthRxDma *pktDmaRxInfo_p = &rxdma->pktDmaRxInfo;

    printk( "bcm63xx_dump_rxdma channel=%d\n", (int)channel);
    printk( "=======================================\n" );
    printk( "rxdma address = 0x%p\n", rxdma);
    printk( "rxdma->rxIrq = %d\n", rxdma->rxIrq );
    printk( "pktDmaRxInfo_p = 0x%p\n", &rxdma->pktDmaRxInfo);
    printk( "pktDmaRxInfo_p->rxEnabled<0x%p>= %d\n",
        &pktDmaRxInfo_p->rxEnabled, pktDmaRxInfo_p->rxEnabled);
    printk( "pktDmaRxInfo_p->channel = %d\n", pktDmaRxInfo_p->channel );
    printk( "pktDmaRxInfo_p->rxDma = 0x%p\n", pktDmaRxInfo_p->rxDma );
    printk( "pktDmaRxInfo_p->rxBdsBase = 0x%p\n", pktDmaRxInfo_p->rxBdsBase );
    printk( "pktDmaRxInfo_p->rxBds= 0x%p\n", pktDmaRxInfo_p->rxBds);
    printk( "pktDmaRxInfo_p->numRxBds = %d\n", pktDmaRxInfo_p->numRxBds );
    printk( "pktDmaRxInfo_p->rxAssignedBds = %d\n",
        pktDmaRxInfo_p->rxAssignedBds );
    printk( "pktDmaRxInfo_p->rxHeadIndex = %d\n", pktDmaRxInfo_p->rxHeadIndex );
    printk( "pktDmaRxInfo_p->rxTailIndex = %d\n", pktDmaRxInfo_p->rxTailIndex );

#if defined(_CONFIG_BCM_FAP)
    printk( "pktDmaRxInfo_p->fapIdx = %d\n", (int) pktDmaRxInfo_p->fapIdx );
    printk( "rxdma->bdsAllocated = %d\n", rxdma->bdsAllocated );
#endif

    return 0;
}

static int bcm63xx_dump_txdma(int channel, BcmPktDma_EthTxDma *txdma )
{
    printk( "bcm63xx_dump_txdma channel=%d\n", (int)channel);
    printk( "=======================================\n" );
    printk( "txdma address = 0x%p\n", txdma);
    printk( "txdma->txEnabled<0x%p>= %d\n", &txdma->txEnabled,
        txdma->txEnabled);
    printk( "txdma->channel = %d\n", txdma->channel );
    printk( "txdma->txDma = 0x%p\n", txdma->txDma );
    printk( "txdma->txBdsBase = 0x%p\n", txdma->txBdsBase );
    printk( "txdma->txBds= 0x%p\n", txdma->txBds);
    printk( "txdma->numTxBds = %d\n", txdma->numTxBds );
    printk( "txdma->txFreeBds = %d\n", txdma->txFreeBds );
    printk( "txdma->txHeadIndex = %d\n", txdma->txHeadIndex );
    printk( "txdma->txTailIndex = %d\n", txdma->txTailIndex );
    printk( "txdma->txRecycle = 0x%p\n", txdma->txRecycle );

#if defined(_CONFIG_BCM_FAP)
    printk( "txdma->fapIdx = %d\n", (int) txdma->fapIdx );
    printk( "txdma->bdsAllocated = %d\n", txdma->bdsAllocated );
#endif

    return 0;
}
#endif


/* Note: this may be called from an atomic context */
static int bcm63xx_alloc_rxdma_bds(int channel, BcmEnet_devctrl *pDevCtrl)
{
   BcmEnet_RxDma *rxdma;
#if defined(CONFIG_BCM947189)
   uint32 phy_addr;
#endif

   rxdma = pDevCtrl->rxdma[channel];

#if defined(CONFIG_BCM947189)
   rxdma->pktDmaRxInfo.rxBdsBase = 
                      bcmPktDma_EthAllocRxBds(&pDevCtrl->dev->dev, 
                                              channel, rxdma->pktDmaRxInfo.numRxBds, 
                                              &phy_addr);
   if ( rxdma->pktDmaRxInfo.rxBdsBase == NULL )
   {
      printk("Unable to allocate memory for Rx Descriptors \n");
      return -ENOMEM;
   }
   rxdma->pktDmaRxInfo.rxBds = (volatile DmaDesc *)
                      (((uintptr_t)rxdma->pktDmaRxInfo.rxBdsBase 
                      + BCM_DCACHE_ALIGN_LEN) & ~BCM_DCACHE_ALIGN_LEN);
   rxdma->pktDmaRxInfo.rxBdsPhysBase = (volatile DmaDesc *)(uintptr_t)phy_addr;
#else
#if defined(RXCHANNEL_PKT_RATE_LIMIT)
   /* Allocate 1 extra BD for rxBdsStdBy */
   rxdma->pktDmaRxInfo.rxBdsBase = bcmPktDma_EthAllocRxBds(channel, rxdma->pktDmaRxInfo.numRxBds + 1);
#else
   rxdma->pktDmaRxInfo.rxBdsBase = bcmPktDma_EthAllocRxBds(channel, rxdma->pktDmaRxInfo.numRxBds);
#endif
   if ( rxdma->pktDmaRxInfo.rxBdsBase == NULL )
   {
      printk("Unable to allocate memory for Rx Descriptors \n");
      return -ENOMEM;
   }
#if defined(ENET_RX_BDS_IN_PSM)
   rxdma->pktDmaRxInfo.rxBds = rxdma->pktDmaRxInfo.rxBdsBase;
#else
   /* Align BDs to a 16-byte boundary - Apr 2010 */
   rxdma->pktDmaRxInfo.rxBds = (volatile DmaDesc *)(((int)rxdma->pktDmaRxInfo.rxBdsBase + 0xF) & ~0xF);
   rxdma->pktDmaRxInfo.rxBds = (volatile DmaDesc *)CACHE_TO_NONCACHE(rxdma->pktDmaRxInfo.rxBds);
#endif
#endif

   /* Local copy of these vars also initialized to zero in bcmPktDma channel init */
   rxdma->pktDmaRxInfo.rxAssignedBds = 0;
   rxdma->pktDmaRxInfo.rxHeadIndex = rxdma->pktDmaRxInfo.rxTailIndex = 0;

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
   /* stand by bd ring with only one BD */
   rxdma->rxBdsStdBy = &rxdma->pktDmaRxInfo.rxBds[rxdma->pktDmaRxInfo.numRxBds];
#endif

#if defined(_CONFIG_BCM_FAP)
    {
        rxdma->bdsAllocated = 1;
    }
#endif

   return 0;
}

/*
 *	static int bcm63xx_alloc_txdma_bds(int channel, BcmEnet_devctrl *pDevCtrl)
 *  Original Logic Matrix:
 *	(818 && FAP && SPL) && hst: AL
 *	(818 && FAP && SPL) && !hst && PSM: NA
 *	(818 && FAP && SPL) && !hst && !PSM: AL
 *	(818 && FAP && !SPL) && PSM: NA
 *	(818 && FAP && !SPL) && !PSM: AL
 *	818 && !FAP: AL
 *
 *	(!818  && SPL) && hst: AL
 *	(!818  && SPL) && !hst && PSM: NA
 *	(!818  && SPL) && !hst && !PSM: AL
 *	!818  && !SPL && PSM: NA
 *	!818  && !SPL && !PSM: AL
*/
#if defined(DBL_DESC)
    #define DmaDescType DmaDesc16
    #if defined(_CONFIG_BCM_FAP) && defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
        #define CHECK_HOST_OWNERSHIP
    #endif
#else /* (defined(DBL_DESC)) */
    #define DmaDescType DmaDesc
    #if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
        #define CHECK_HOST_OWNERSHIP
    #endif
#endif
static int bcm63xx_alloc_txdma_bds(int channel, BcmEnet_devctrl *pDevCtrl)
{
   BcmPktDma_EthTxDma *txdma;
   int nr_tx_bds;
   bool do_align = TRUE;
#if defined(CONFIG_BCM947189)
   uint32 phy_addr;
#endif

   txdma = pDevCtrl->txdma[channel];
   nr_tx_bds = txdma->numTxBds;

   /* BDs allocated in bcmPktDma lib in PSM or in DDR */
#if defined(CONFIG_BCM947189)
   txdma->txBdsBase = bcmPktDma_EthAllocTxBds(&pDevCtrl->dev->dev, channel, nr_tx_bds, &phy_addr);
   if ( txdma->txBdsBase == NULL )
   {
      printk("Unable to allocate memory for Tx Descriptors \n");
      return -ENOMEM;
   }
   txdma->txBdsPhysBase = (volatile DmaDesc *)(uintptr_t)phy_addr;
   /* Assumption : allocated BDs are 16 Byte aligned */
   txdma->txRecycleBase = kmalloc(nr_tx_bds * sizeof(BcmPktDma_txRecycle_t) + BCM_DCACHE_LINE_LEN, GFP_ATOMIC) ;
   if (txdma->txRecycleBase !=NULL) {
       memset(txdma->txRecycleBase, 0, nr_tx_bds * sizeof(BcmPktDma_txRecycle_t) + BCM_DCACHE_LINE_LEN);
   }
   txdma->txRecycle = (BcmPktDma_txRecycle_t*)(((uintptr_t)txdma->txRecycleBase + BCM_DCACHE_ALIGN_LEN) & ~BCM_DCACHE_ALIGN_LEN); 
   do_align = FALSE; /* No further alignment needed */
   txdma->txBds = txdma->txBdsBase;
#else
   txdma->txBdsBase = bcmPktDma_EthAllocTxBds(channel, nr_tx_bds);
   if ( txdma->txBdsBase == NULL )
   {
      printk("Unable to allocate memory for Tx Descriptors \n");
      return -ENOMEM;
   }

   BCM_ENET_DEBUG("bcm63xx_alloc_txdma_bds txdma->txBdsBase 0x%x",
        (unsigned int)txdma->txBdsBase);

   txdma->txBds = txdma->txBdsBase;
   txdma->txRecycle = (BcmPktDma_txRecycle_t *)((uint32)txdma->txBds + (nr_tx_bds * sizeof(DmaDescType)));

    #if defined(CHECK_HOST_OWNERSHIP)
    if(txdma->txOwnership != HOST_OWNED)
    #endif
    {
    #if defined(ENET_TX_BDS_IN_PSM)
        do_align = FALSE;
    #endif
    }

        /* Align BDs to a 16/32 byte boundary - Apr 2010 */
    if(do_align) {
        txdma->txBds = (volatile void *)(((int)txdma->txBds + 0xF) & ~0xF);
        txdma->txBds = (volatile void *)CACHE_TO_NONCACHE(txdma->txBds);
        txdma->txRecycle = (BcmPktDma_txRecycle_t *)((uint32)txdma->txBds + (nr_tx_bds * sizeof(DmaDescType)));
        txdma->txRecycle = (BcmPktDma_txRecycle_t *)NONCACHE_TO_CACHE(txdma->txRecycle);
    }
#endif

   txdma->txFreeBds = nr_tx_bds;
   txdma->txHeadIndex = txdma->txTailIndex = 0;
   nr_tx_bds = txdma->numTxBds;

#if !defined(CONFIG_BCM947189)
   /* BDs allocated in bcmPktDma lib in PSM or in DDR */
   memset((char *) txdma->txBds, 0, sizeof(DmaDescType) * nr_tx_bds );
#else
   memset((char *) txdma->txBds, 0, sizeof(DmaDesc) * nr_tx_bds );
#endif

#if defined(_CONFIG_BCM_FAP)
#if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
    if (txdma->txOwnership != HOST_OWNED)
#endif
        txdma->bdsAllocated = 1;
#endif

   return 0;
}

#if !defined(_CONFIG_BCM_FAP)
static FN_HANDLER_RT bcmeapi_enet_isr(int irq, void * pContext);
#endif /* !defined(_CONFIG_BCM_FAP) || defined(CONFIG_BCM_PKTDMA_TX_SPLITTING) */

static int bcm63xx_init_rxdma_structures(int channel, BcmEnet_devctrl *pDevCtrl)
{
    BcmEnet_RxDma *rxdma;

    /* init rx dma channel structures */
    pDevCtrl->rxdma[channel] = (BcmEnet_RxDma *) (kzalloc(
                           sizeof(BcmEnet_RxDma), GFP_KERNEL));
    if (pDevCtrl->rxdma[channel] == NULL) {
        printk("Unable to allocate memory for rx dma rings \n");
        return -ENXIO;
    }
    BCM_ENET_DEBUG("The rxdma is 0x%p \n", pDevCtrl->rxdma[channel]);

    rxdma = pDevCtrl->rxdma[channel];
    rxdma->pktDmaRxInfo.channel = channel;
#if defined(_CONFIG_BCM_FAP)
    rxdma->pktDmaRxInfo.fapIdx = getFapIdxFromEthRxIudma(channel);
    rxdma->bdsAllocated = 0;
#endif


    /* init number of Rx BDs in each rx ring */
    rxdma->pktDmaRxInfo.numRxBds =
                    bcmPktDma_EthGetRxBds( &rxdma->pktDmaRxInfo, channel );

#if defined(_CONFIG_BCM_INGQOS)
    enet_rx_init_iq_thresh(pDevCtrl, channel);
#endif

#if !defined(CONFIG_BCM947189)
#if !defined(_CONFIG_BCM_FAP)
    {
        /* request IRQs only once at module init */
        {
            int rxIrq = bcmPktDma_EthSelectRxIrq(channel);

            /* disable the interrupts from device */
            bcmPktDma_BcmHalInterruptDisable(channel, rxIrq);

            /* a Host owned channel */
            BcmHalMapInterrupt(bcmeapi_enet_isr,
                (void*)(BUILD_CONTEXT(pDevCtrl,channel)), rxIrq);

#if defined(CONFIG_BCM_GMAC)
            if ( gmac_info_pg->enabled && (channel == GMAC_LOG_CHAN) )
            {
                rxIrq = INTERRUPT_ID_GMAC_DMA_0;

                /* disable the interrupts from device */
                bcmPktDma_BcmHalInterruptDisable(channel, rxIrq);

                BcmHalMapInterrupt(bcmeapi_enet_isr,
                    (void*)(BUILD_CONTEXT(pDevCtrl,channel)), rxIrq);
            }
#endif
        }
    }
#endif
#endif

    return 0;
}

#if !defined(_CONFIG_BCM_FAP)
/*
 * bcmeapi_enet_isr: Acknowledge interrupt and check if any packets have
 *                  arrived on Rx DMA channel 0..3
 */
static FN_HANDLER_RT bcmeapi_enet_isr(int irq, void * pContext)
{
    /* this code should not run in DQM operation !!! */

    int channel;
    BcmEnet_devctrl *pDevCtrl;

    channel = CONTEXT_TO_CHANNEL((uint32)pContext);
    pDevCtrl = CONTEXT_TO_PDEVCTRL((uint32)pContext);

    /* Only rx channels owned by the Host come through this ISR */
    bcmPktDma_EthClrRxIrq_Iudma(&pDevCtrl->rxdma[channel]->pktDmaRxInfo);

    BCMENET_WAKEUP_RXWORKER(pDevCtrl);

    return BCM_IRQ_HANDLED;
}
#endif /* !defined(_CONFIG_BCM_FAP) || defined(CONFIG_BCM_PKTDMA_TX_SPLITTING) */

#if defined(CONFIG_BCM_GMAC)
/* get GMAC's logical port id */
int enet_gmac_log_port( void )
{
    return PHYSICAL_PORT_TO_LOGICAL_PORT(GMAC_PORT_ID, 0);
}

/* Is the GMAC enabled and the port matches with GMAC's logical port? */
inline int IsGmacPort( int log_port )
{
    if ( gmac_info_pg->enabled && (log_port == enet_gmac_log_port() ) )
        return 1;
    else
        return 0;
}


/* Is the GMAC enabled and the port matches with GMAC's logical port? */
inline int ChkGmacPort( void * ctxt )
{
    return IsGmacPort( *(int *)ctxt );
}

/* Is the GMAC port active? */
inline int ChkGmacActive( void *ctxt )
{
    return IsGmacInfoActive;
}
#endif /* CONFIG_BCM_GMAC */

int bcmeapi_ioctl_kernel_poll(struct ethswctl_data *e)
{
    static int mdk_init_done = 0;

    /* MDK will calls this function for the first time after it completes initialization */
    if (!mdk_init_done) {
        mdk_init_done = 1;
        ethsw_eee_init();
    }

#if defined(CONFIG_BCM_ETH_PWRSAVE)
    ethsw_ephy_auto_power_down_wakeup();
#endif

#if defined(CONFIG_BCM_ETH_PWRSAVE)
    ethsw_ephy_auto_power_down_sleep();
#endif

    /* Check for delayed request to enable EEE */
    ethsw_eee_process_delayed_enable_requests();

#if (CONFIG_BCM_EXT_SWITCH_TYPE == 53115)
    extsw_apd_set_compatibility_mode();
#endif
    return 0;
}

static int bcm63xx_xmit_reclaim(void)
{
    int i;
    pNBuff_t pNBuff;
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)netdev_priv(vnet_dev[0]);
    BcmPktDma_txRecycle_t txRecycle;
    BcmPktDma_txRecycle_t *txRecycle_p;

#if defined(CONFIG_BCM947189)
    BcmPktDma_LocalEthTxDma *tx;
    ENET_TX_LOCK();
    for (i = 0; i < cur_txdma_channels; i++)
    {   
        /* Check DMA descriptor protocol error */
        tx = pDevCtrl->txdma[i];
        if ((tx->txDma->status0 & DMA_STATE_MASK) == DMA_STATE_STOP &&
            (tx->txDma->status1 & DMA_PROC_ERROR_MASK) == DMA_PROC_PROTO_ERROR)
        {
            /* suspend and flush */
            tx->txDma->control |= (DMA_SUSP_EN | DMA_FLUSH_GMAC);
            udelay(10);
            /* disable DMA */
            tx->txDma->control &= ~(DMA_SUSP_EN | DMA_FLUSH_GMAC | DMA_EN); 
            while (tx->txFreeBds < tx->numTxBds) 
            {
                tx->txFreeBds++;
                ENET_TX_UNLOCK();
                nbuff_free((void *)tx->txRecycle[tx->txHeadIndex++].key);
                ENET_TX_LOCK();
                if (tx->txHeadIndex == tx->numTxBds)
                    tx->txHeadIndex = 0;
            }
            tx->txHeadIndex = tx->txTailIndex = 0;
            tx->txDma->control |= DMA_EN;
            /* Start with an empty descriptor table (ptr = addrlow) */
            tx->txDma->ptr = tx->txDma->addrlow;
        } 
    }   
    ENET_TX_UNLOCK();
#endif
    /* Obtain exclusive access to transmitter.  This is necessary because
    * we might have more than one stack transmitting at once. */
    ENET_TX_LOCK();
    for (i = 0; i < cur_txdma_channels; i++)
    {
        while ((txRecycle_p = bcmPktDma_EthFreeXmitBufGet(pDevCtrl->txdma[i], &txRecycle)) != NULL)
        {
           pNBuff = (pNBuff_t)txRecycle_p->key;

           BCM_ENET_TX_DEBUG("bcmPktDma_EthFreeXmitBufGet TRUE! (reclaim) key 0x%x\n", (int)pNBuff);
           if (pNBuff != PNBUFF_NULL) {
               ENET_TX_UNLOCK();
               nbuff_free(pNBuff);
               ENET_TX_LOCK();
           }
        }   /* end while(...) */
    }   /* end for(...) */
    ENET_TX_UNLOCK();

    return 0;
}

/* Forward declarations */
void __ethsw_get_txrx_imp_port_pkts(void);

void bcmeapi_enet_poll_timer(void)
{
    /* Collect CPU/IMP Port RX/TX packets every poll period */
    __ethsw_get_txrx_imp_port_pkts();

    bcm63xx_xmit_reclaim();

#if !defined(CONFIG_BCM_SWMDK)
#if defined(_CONFIG_BCM_FAP)
#if defined (_CONFIG_BCM_XTMCFG)
    /* Add code for buffer quick free between enet and xtm - June 2010 */
    if(xtm_fkb_recycle_hook == NULL)
        xtm_fkb_recycle_hook = bcmPktDma_get_xtm_fkb_recycle();
#endif /* _CONFIG_BCM_XTMCFG */
#if defined(CONFIG_BCM963268)
    if(xtm_skb_recycle_hook == NULL)
        xtm_skb_recycle_hook = bcmPktDma_get_xtm_skb_recycle();
#endif
#endif
#endif
}

void bcmeapi_aelink_handler(int linkstatus)
{
}

void bcmeapi_set_mac_speed(int port, int speed)
{
}

#if defined(_CONFIG_BCM_FAP)
void bcmeapi_EthSetPhyRate(int port, int enable, uint32_t bps, int isWanPort)
{
    if(enable)
    {
        int unit = LOGICAL_PORT_TO_UNIT_NUMBER(port);
        unsigned long maxRate = pVnetDev0_g->EnetInfo[unit].sw.portMaxRate[LOGICAL_PORT_TO_PHYSICAL_PORT(port)];
        unsigned long phyconn = pVnetDev0_g->EnetInfo[unit].sw.phyconn[LOGICAL_PORT_TO_PHYSICAL_PORT(port)];
        uint32_t kbps;

        if ( (0 == maxRate) || (maxRate > bps) )
        {
            maxRate = bps;
        }
        kbps = ((maxRate / 1000) * 99) / 100;

        switch(maxRate)
        {
            case SPEED_1000MBIT:
                bcmPktDma_EthSetPhyRate(port, 0, kbps, isWanPort);
                break;
            default:
                bcmPktDma_EthSetPhyRate(port, 1, kbps, isWanPort);
                break;
        }
        if ( phyconn == PHY_CONN_TYPE_PLC )
        {
            bcmPktDma_EthSetPauseEn(port, 1);
        }
    }
    else
    {
        bcmPktDma_EthSetPhyRate(port, 0, 0, isWanPort);
    }
}
#elif defined(_CONFIG_ENET_BCM_TM)
void bcmeapi_EthSetPhyRate(int port, int enable, uint32_t bps, int isWanPort)
{
    int unit = LOGICAL_PORT_TO_UNIT_NUMBER(port);
    unsigned long maxRate = pVnetDev0_g->EnetInfo[unit].sw.portMaxRate[LOGICAL_PORT_TO_PHYSICAL_PORT(port)];
//    unsigned long phyconn = pVnetDev0_g->EnetInfo[unit].sw.phyconn[LOGICAL_PORT_TO_PHYSICAL_PORT(port)];
    bcmTmDrv_arg_t tm;
    bcmFun_t *bcmFun;
    uint32_t kbps;

    if ( (0 == maxRate) || (maxRate > bps) )
    {
        maxRate = bps;
    }

    kbps = ((maxRate / 1000) * 99) / 100;

    bcmFun = bcmFun_get(BCM_FUN_ID_TM_PORT_CONFIG);
    BCM_ASSERT(bcmFun != NULL);

    tm.phy = BCM_TM_DRV_PHY_TYPE_ETH;
    tm.port = port;
    tm.mode = BCM_TM_DRV_MODE_AUTO;
    tm.kbps = kbps;
    tm.mbs = 2000;
    tm.shapingType = BCM_TM_DRV_SHAPING_TYPE_DISABLED;

    if(bcmFun(&tm))
    {
        return;
    }

    bcmFun = bcmFun_get(BCM_FUN_ID_TM_PORT_ENABLE);
    BCM_ASSERT(bcmFun != NULL);

    tm.enable = enable;

    if(bcmFun(&tm))
    {
        return;
    }

    bcmFun = bcmFun_get(BCM_FUN_ID_TM_APPLY);
    BCM_ASSERT(bcmFun != NULL);

    if(bcmFun(&tm))
    {
        return;
    }

#if 0
    if ( enable && phyconn == PHY_CONN_TYPE_PLC )
    {
        bcmPktDma_EthSetPauseEn(port, 1);
    }
#endif
}
#endif

void bcmeapi_reset_mib_cnt(uint32_t sw_port)
{
	bcmPktDma_EthResetStats(sw_port);
}

int bcmeapi_link_might_changed(void)
{
    int link_changed = 0;
    
    if(ephy_int_lock != ephy_int_cnt)
    {   
        link_changed = ETHSW_LINK_MIGHT_CHANGED;
        ephy_int_lock = ephy_int_cnt;
    }

    return link_changed;
}

int  bcmeapi_open_dev(BcmEnet_devctrl *pDevCtrl, struct net_device *dev)
{
    int channel = 0;
    BcmPktDma_EthTxDma *txdma;

    ENET_RX_LOCK();
#if !defined(CONFIG_BCM947189)
    pDevCtrl->dmaCtrl->controller_cfg |= DMA_MASTER_EN;
#endif

    /*  Enable the Rx DMA channels and their interrupts  */
    for (channel = 0; channel < cur_rxdma_channels; channel++) {
#if defined(RXCHANNEL_PKT_RATE_LIMIT)
        rxchannel_isr_enable[channel] = 1;
#endif
        enet_rxdma_channel_enable(channel);
    }
    ENET_RX_UNLOCK();

    ENET_TX_LOCK();
    /*  Enable the Tx DMA channels  */
    for (channel = 0; channel < cur_txdma_channels; channel++) {
        txdma = pDevCtrl->txdma[channel];
        bcmPktDma_EthTxEnable(txdma);
        txdma->txEnabled = 1;
    }
    ENET_TX_UNLOCK();

#if defined(_CONFIG_BCM_FAP)
#if defined(CONFIG_BCM963268)
    {
        struct ethswctl_data e2;

        /* Needed to allow iuDMA split override to work properly - Feb 2011 */
        /* Set the Switch Control and QoS registers later than init for the 63268/6828 */

        /* The equivalent of "ethswctl -c cosqsched -v BCM_COSQ_COMBO -q 2 -x 1 -y 1 -z 1 -w 1" */
        /* This assigns equal weight to each of the 4 egress queues */
        /* This means the rx splitting feature cannot co-exist with h/w QOS */
        e2.type = TYPE_SET;
        e2.queue = 2;   /* mode */
        e2.scheduling = BCM_COSQ_COMBO;
        e2.weights[0] = e2.weights[1] = e2.weights[2] = e2.weights[3] = 1;
        bcmeapi_ioctl_ethsw_cosq_sched(&e2);

        /* The equivalent of "ethswctl -c cosq -q 1 -v 1" */
        /* This associates egress queue 1 on the switch to iuDMA1 */
        e2.type = TYPE_SET;
        e2.queue = 1;
        e2.channel = 1;
        bcmeapi_ioctl_ethsw_cosq_rxchannel_mapping(&e2);

    }
#endif
#endif
    return 0; /* success */
}

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
/*
 * bcm63xx_timer: 100ms timer for updating rx rate control credits
 */
static int bcm63xx_timer(void * arg)
{
    struct net_device *dev = vnet_dev[0];
    BcmEnet_devctrl *priv = (BcmEnet_devctrl *)netdev_priv(dev);
    BcmEnet_RxDma *rxdma;
    unsigned int elapsed_msecs;
    int channel;
    struct sched_param param;

    /* */
    param.sched_priority = BCM_RTPRIO_DATA;
    sched_setscheduler(current, SCHED_RR, &param);
    set_user_nice(current, 0);

    /* */
    while (atomic_read(&timer_lock) > 0)
    {
        for (channel = 0; channel < cur_rxdma_channels; channel++) {
            ENET_RX_LOCK();
            if (rxchannel_rate_limit_enable[channel]) {
                elapsed_msecs = jiffies_to_msecs(jiffies -
                        last_pkt_jiffies[channel]);
                if (elapsed_msecs >= 99) {
                    rxdma = priv->rxdma[channel];
                    BCM_ENET_DEBUG("pkts_from_last_jiffies = %d \n",
                            rx_pkts_from_last_jiffies[channel]);
                    rx_pkts_from_last_jiffies[channel] = 0;
                    last_pkt_jiffies[channel] = jiffies;
                    if (rxchannel_isr_enable[channel] == 0) {
                        BCM_ENET_DEBUG("Enabling DMA Channel & Interrupt \n");
                        switch_rx_ring(priv, channel, 0);
                        bcmPktDma_BcmHalInterruptEnable(channel, rxdma->rxIrq);
                        rxchannel_isr_enable[channel] = 1;
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

void bcmeapi_EthGetStats(int log_port, uint32 *rxDropped, uint32 *txDropped)
{
    bcmPktDma_EthGetStats(log_port, rxDropped, txDropped);
}

void bcmeapi_del_dev_intr(BcmEnet_devctrl *pDevCtrl)
{
    int channel = 0;
    BcmPktDma_EthTxDma *txdma;

    ENET_RX_LOCK();
    for (channel = 0; channel < cur_rxdma_channels; channel++) {
        enet_rxdma_channel_disable(channel);
    }
    ENET_RX_UNLOCK();

    ENET_TX_LOCK();
    for (channel = 0; channel < cur_txdma_channels; channel++) {

        txdma = pDevCtrl->txdma[channel];
        txdma->txEnabled = 0;
        bcmPktDma_EthTxDisable(txdma);
    }
    ENET_TX_UNLOCK();
}

#if defined(_CONFIG_BCM_BPM)
/*
 * Assumptions:-
 * 1. Align data buffers on 16-byte boundary - Apr 2010
 */

/* Dumps the BPM status for Eth channels */
static void enet_bpm_status(void)
{
    int chnl;
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)netdev_priv(vnet_dev[0]);

    for (chnl = 0; chnl < cur_rxdma_channels; chnl++)
    {
        BcmPktDma_EthRxDma *rxdma;
        rxdma = &pDevCtrl->rxdma[chnl]->pktDmaRxInfo;

#if defined(_CONFIG_BCM_FAP)
        if (g_Eth_rx_iudma_ownership[chnl] != HOST_OWNED)
            continue;
#endif

        printk("[HOST] ENET %4d %10u %10u %5u %4u %4u\n",
                chnl, (uint32_t) rxdma->alloc,
                (uint32_t) rxdma->free,
                (uint32_t) rxdma->rxAssignedBds,
                (uint32_t) rxdma->allocTrig,
                (uint32_t) rxdma->bulkAlloc );
    }
}


/* Allocates the buffer ring for an Eth RX channel */
static int enet_bpm_alloc_buf_ring(BcmEnet_devctrl *pDevCtrl,
        int channel, uint32 num)
{
    unsigned char  *pData;
    FkBuff_t *pFkBuf;
    uint32 context = 0;
    uint32 buf_ix;

    RECYCLE_CONTEXT(context)->channel = channel;

    for (buf_ix=0; buf_ix < num; buf_ix++)
    {
        if ( (pData = (uint8_t *) gbpm_alloc_buf()) == NULL )
            return GBPM_ERROR;

        pFkBuf = PDATA_TO_PFKBUFF(pData, BCM_PKT_HEADROOM);

        /* Place a FkBuff_t object at the head of pFkBuf */
        fkb_preinit((uint8_t *)pFkBuf, (RecycleFuncP)bcm63xx_enet_recycle, context);

        cache_flush_region(pData, (uint8_t *)pFkBuf + BCM_PKTBUF_SIZE);
        bcmPktDma_EthFreeRecvBuf(&pDevCtrl->rxdma[channel]->pktDmaRxInfo, pData);
    }

    return GBPM_SUCCESS;
}


/* Frees the buffer ring for an Eth RX channel */
static void enet_bpm_free_buf_ring(BcmEnet_RxDma *rxdma, int channel)
{
    uninit_buffers(rxdma);
}


static void enet_rx_set_bpm_alloc_trig( BcmEnet_devctrl *pDevCtrl, int chnl )
{
    BcmPktDma_EthRxDma *rxdma = &pDevCtrl->rxdma[chnl]->pktDmaRxInfo;
    uint32  allocTrig = rxdma->numRxBds * BPM_ENET_ALLOC_TRIG_PCT/100;

        BCM_ENET_DEBUG( "Enet: Chan=%d BPM Rx allocTrig=%d bulkAlloc=%d\n",
        chnl, (int) allocTrig, BPM_ENET_BULK_ALLOC_COUNT );

    bcmPktDma_EthSetRxChanBpmThresh(rxdma,
        allocTrig, BPM_ENET_BULK_ALLOC_COUNT);
}


#if defined(_CONFIG_BCM_FAP)
/* Dumps the TxDMA drop thresh for eth channels */
static void enet_bpm_dma_dump_tx_drop_thr(void)
{
    int chnl;
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)netdev_priv(vnet_dev[0]);

    for (chnl = 0; chnl < cur_txdma_channels; chnl++)
    {
        BcmPktDma_EthTxDma *txdma = pDevCtrl->txdma[chnl];
        int q;

        if (g_Eth_tx_iudma_ownership[chnl] != HOST_OWNED)
             continue;

        for ( q=0; q < ENET_TX_EGRESS_QUEUES_MAX; q++ )
            printk("[HOST] ENET %4u %4u %10u %10u\n",
               chnl, q,
               (uint32_t) txdma->txDropThr[q],
               (uint32_t) txdma->txDropThrPkts[q]);
    }
}

/* print the BPM TxQ Drop Thresh */
static void enet_bpm_dump_tx_drop_thr(void)
{
    enet_bpm_dma_dump_tx_drop_thr();
}


/* init ENET TxQ drop thresholds */
static void enet_bpm_init_tx_drop_thr(BcmEnet_devctrl *pDevCtrl, int chnl)
{
    BcmPktDma_EthTxDma *txdma = pDevCtrl->txdma[chnl];
    int nr_tx_bds;

    nr_tx_bds = bcmPktDma_EthGetTxBds( txdma, chnl );
    BCM_ASSERT(nr_tx_bds > 0);
    enet_bpm_dma_tx_drop_thr[chnl][0] =
                    (nr_tx_bds * ENET_BPM_PCT_TXQ0_DROP_THRESH)/100;
    enet_bpm_dma_tx_drop_thr[chnl][1] =
                    (nr_tx_bds * ENET_BPM_PCT_TXQ1_DROP_THRESH)/100;
    enet_bpm_dma_tx_drop_thr[chnl][2] =
                    (nr_tx_bds * ENET_BPM_PCT_TXQ2_DROP_THRESH)/100;
    enet_bpm_dma_tx_drop_thr[chnl][3] =
                    (nr_tx_bds * ENET_BPM_PCT_TXQ3_DROP_THRESH)/100;

    BCM_ENET_DEBUG("Enet: BPM DMA Init Tx Drop Thresh: chnl=%u txbds=%u thr[0]=%u thr[1]=%u thr[2]=%u thr[3]=%u\n",
                chnl, nr_tx_bds,
                enet_bpm_dma_tx_drop_thr[chnl][0],
                enet_bpm_dma_tx_drop_thr[chnl][1],
                enet_bpm_dma_tx_drop_thr[chnl][2],
                enet_bpm_dma_tx_drop_thr[chnl][3]);
}


#endif


#endif

void bcmeapi_module_init(void)
{
    int idx;

#if defined(_CONFIG_BCM_INGQOS)
    iqos_enet_status_hook_g = enet_iq_status;
#endif

#if defined(_CONFIG_BCM_BPM)
    gbpm_g.enet_status = enet_bpm_status;
#if defined(_CONFIG_BCM_FAP)
    gbpm_g.enet_thresh = enet_bpm_dump_tx_drop_thr;
#endif
#endif

    /* Initialize the static global array */
    for (idx = 0; idx < ENET_RX_CHANNELS_MAX; ++idx)
    {
        pending_channel[idx] = idx;
    }
}

