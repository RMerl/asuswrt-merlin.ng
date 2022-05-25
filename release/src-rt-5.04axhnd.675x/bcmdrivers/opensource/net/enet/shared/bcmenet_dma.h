/*
 Copyright 2002-2010 Broadcom Corp. All Rights Reserved.

 <:label-BRCM:2011:DUAL/GPL:standard    
 
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
#ifndef _BCMENET_DMA_H_
#define _BCMENET_DMA_H_

#include <bcm_map_part.h>
#include "bcmPktDma_structs.h"

#ifdef _BCMENET_LOCAL_

#include <linux/bcm_tstamp.h>


#include "bcmPktDma.h"
#include <linux/version.h>
#include "bcmsw_api.h"
#include "bcmswaccess.h"
#include "bcmSpiRes.h"
#include "bcmswshared.h"

#endif /* #ifdef _BCMENET_LOCAL_ */

#include "robosw_reg_shared.h"

#define port_from_flag(flag) ((flag >> 8) & 0x000f)
#define gemid_from_dmaflag(dmaFlag) (dmaFlag & RX_GEM_ID_MASK)

#if defined(_CONFIG_BCM_VOICE_SUPPORT)
#define NETDEV_WEIGHT  16 // lower weight for less voice latency
#else
#define NETDEV_WEIGHT  32
#endif

/*---------------------------------------------------------------------*/
/* specify number of BDs and buffers to use                            */
/*---------------------------------------------------------------------*/
/* In order for ATM shaping to work correctly,
 * the number of receive BDS/buffers = # tx queues * # buffers per tx queue
 * (80 ENET buffers = 8 tx queues * 10 buffers per tx queue)
 */
#define ENET_CACHE_SMARTFLUSH

/* misc. configuration */
#define DMA_FC_THRESH_LO        5
#define DMA_FC_THRESH_HI        (NR_RX_BDS_MIN / 2)

int phy_port_to_logic_port(int port, int unit);

/*
 * device context
 */ 


/* Keep in sync with bcmPktDma_structs.h */
#define NUM_RXDMA_CHANNELS ENET_RX_CHANNELS_MAX
#define NUM_TXDMA_CHANNELS ENET_TX_CHANNELS_MAX

#define BcmPktDma_EthRxDma BcmPktDma_LocalEthRxDma
#define BcmPktDma_EthTxDma BcmPktDma_LocalEthTxDma

#define bcmeapi_add_dev_queue(dev) {}

typedef struct BcmEnet_RxDma {

    BcmPktDma_EthRxDma pktDmaRxInfo;
    int      rxIrq;   /* rx dma irq */
    struct sk_buff *freeSkbList;
    uint32   channel;


#if defined(RXCHANNEL_PKT_RATE_LIMIT)
    volatile DmaDesc *rxBdsStdBy;
    unsigned char * StdByBuf;
#endif
    unsigned char   **buf_pool; //[NR_RX_BDS_MAX]; /* rx buffer pool */
    unsigned char *skbs_p;
    unsigned char *end_skbs_p;
} BcmEnet_RxDma;

struct BcmEnet_devctrl {
    BcmEnetDevctrlBaseClass_s;

    volatile DmaRegs *dmaCtrl;          /* EMAC DMA register base address */
    /* DmaKeys, DmaSources, DmaAddresses now allocated with txBds - Apr 2010 */
    BcmPktDma_EthTxDma *txdma[NUM_TXDMA_CHANNELS];
    BcmEnet_RxDma *rxdma[NUM_RXDMA_CHANNELS];

};

#ifndef CARDNAME
#define CARDNAME    "BCM_DMA_ENET"
#endif

int bcmenet_add_proc_files(struct net_device *dev);
int bcmenet_del_proc_files(struct net_device *dev);


/* Accessor functions */

#ifdef _BCMENET_LOCAL_   /* Local definitions not to be used outside Ethernet driver */

#if defined(_CONFIG_BCM_INGQOS)
#include <linux/iqos.h>
#include "ingqos.h"
#endif

#if defined(_CONFIG_BCM_BPM)
#include <linux/gbpm.h>
#include "bpm.h"
#endif



extern int channel_for_queue[NUM_EGRESS_QUEUES];
extern int use_tx_dma_channel_for_priority;
/* rx scheduling control and config variables */
extern int scheduling;
extern int max_pkts0;
extern int weights[ENET_RX_CHANNELS_MAX];
extern int weight_pkts[ENET_RX_CHANNELS_MAX];
extern int pending_weight_pkts[ENET_RX_CHANNELS_MAX];
extern int pending_channel[ENET_RX_CHANNELS_MAX]; /* Initialization is done during module init */
extern int channel_ptr;
extern int loop_index;
extern int global_channel;
extern int pending_ch_tbd;
extern int channels_tbd;
extern int active_channels;
extern int channels_mask;
extern int pending_channels_mask;

extern extsw_info_t extSwInfo;
extern BcmEnet_devctrl *pVnetDev0_g;

extern int cur_rxdma_channels;
extern int cur_txdma_channels;
extern int next_channel[ENET_RX_CHANNELS_MAX];



static void bcm63xx_enet_recycle_skb_or_data(struct sk_buff *skb,
                                             uint32 context, uint32 free_flag);
int bcmeapi_ioctl_kernel_poll(struct ethswctl_data *e);
int bcmeapi_ioctl_link_change(BcmEnet_devctrl *priv, struct ethswctl_data *e);
void bcmeapi_dump_queue(struct ethswctl_data *e, BcmEnet_devctrl *pDevCtrl);
int bcmeapi_init_queue(BcmEnet_devctrl *pDevCtrl);
void bcmeapi_del_dev_intr(BcmEnet_devctrl *pDevCtrl);
void bcmeapi_get_chip_idrev(unsigned int *chipid, unsigned int *chiprev);
int bcmeapi_get_num_txques(struct ethctl_data *ethctl);
int bcmeapi_set_num_txques(struct ethctl_data *ethctl);
int bcmeapi_get_num_rxques(struct ethctl_data *ethctl);
int bcmeapi_set_num_rxques(struct ethctl_data *ethctl);
int bcmeapi_config_queue(struct ethswctl_data *e);
void bcmeapi_module_init(void);

int bcmeapi_link_might_changed(void);
int bcmeapi_map_interrupt(BcmEnet_devctrl *pDevCtrl);
void bcmeapi_free_irq(BcmEnet_devctrl *pDevCtrl);
void bcmeapi_EthGetStats(int log_port, uint32 *rxDropped, uint32 *txDropped);
#if defined(_CONFIG_ENET_BCM_TM)
void bcmeapi_EthSetPhyRate(int port, int enable, uint32_t bps, int isWanPort);
#else
#define bcmeapi_EthSetPhyRate(port, enable, bps, isWanPort)
#endif
void bcmeapi_set_mac_speed(int port, int speed);
void bcmeapi_aelink_handler(int linkstatus);
void bcmeapi_enet_poll_timer(void);
void bcmeapi_update_link_status(void);

/* Defined only if more than one channel and FAP supported */
#define bcmeapi_prepare_next_rx(rxpktgood) BCMEAPI_CTRL_CONTINUE
int bcmbal_select_next_channel(uint32 *rxpktgood);
void bcmeapi_free_queue(BcmEnet_devctrl *pDevCtrl);
int bcmeapi_init_dev(struct net_device *dev);
#if !(defined(_CONFIG_BCM_BPM))
#define   bcmeapi_set_fkb_recycle_hook(pFkb)
#else
#endif
int  bcmeapi_open_dev(BcmEnet_devctrl *pDevCtrl, struct net_device *dev);
void bcmeapi_add_proc_files(struct net_device *dev, BcmEnet_devctrl *pDevCtrl);
int bcmeapi_ioctl_rx_pkt_rate_config(struct ethswctl_data *e);
int bcmeapi_ioctl_rx_pkt_rate_limit_config(struct ethswctl_data *e);
int bcmeapi_ioctl_rx_rate_config(struct ethswctl_data *e);
int bcmeapi_ioctl_ethsw_rxscheduling(struct ethswctl_data *e);
int bcmeapi_ioctl_ethsw_wrrparam(struct ethswctl_data *e);
int bcmeapi_ioctl_test_config(struct ethswctl_data *e);

#define bcmenet_logical_port_to_rdpa_if(log_port) (-1);
#define enet_get_current_cb_port(logPort) BP_CROSSBAR_NOT_DEFINED
#define bcmeapi_should_create_vport(logical_port) (1) /* Always crate vport for all port */
/*
 * Recycling context definition
 */
#define DELAYED_RECLAIM_ARRAY_LEN 8

struct enet_xmit_params {
    enet_xmit_params_base;
    uint32 dqm;
    DmaDesc dmaDesc;
    BcmPktDma_EthTxDma *txdma;
    int channel;
    FkBuff_t *pFkb;
    struct sk_buff *skb;
    uint32 reclaim_idx;
    uint32 delayed_reclaim_array[DELAYED_RECLAIM_ARRAY_LEN];
};

#include "bcmenet_dma_inline.h"

#endif /* _BCMENET_LOCAL_ */

#endif /* _BCMENET_DMA_H_ */

