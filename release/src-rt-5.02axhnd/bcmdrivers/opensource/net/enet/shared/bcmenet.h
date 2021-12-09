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
#ifndef _BCMENET_H_
#define _BCMENET_H_

#if defined(CONFIG_BCM_ENET_4908_GMAC)
#include <linux/rtnetlink.h>
#include "bcmenet_common.h"
#include "bcmPktDma_structs.h"
typedef struct BcmEnet_RxDma {

    BcmPktDma_LocalEthRxDma pktDmaRxInfo;
    int      rxIrq;   /* rx dma irq */
    struct sk_buff *freeSkbList;
    uint32   channel;

    unsigned char   **buf_pool; //[NR_RX_BDS_MAX]; /* rx buffer pool */
    unsigned char *skbs_p;
    unsigned char *end_skbs_p;
} BcmEnet_RxDma;

typedef struct BcmEnet_devctrl {

    volatile DmaRegs *dmaCtrl;          /* EMAC DMA register base address */
    /* DmaKeys, DmaSources, DmaAddresses now allocated with txBds - Apr 2010 */
    BcmPktDma_LocalEthTxDma *txdma;
    BcmEnet_RxDma *rxdma;
    
    struct net_device *dev;             /* ptr to net_device */ \
    struct rtnl_link_stats64 stats;     /* statistics used by the kernel */ \
    struct rtnl_link_stats64 cHwStats;  /* Cummulative HW stats */ \
    struct rtnl_link_stats64 lHwStats;  /* Last HW stats snapshot */ \
    int             linkState;          /* link status */ \
    int rx_work_avail;
    wait_queue_head_t rx_thread_wqh;
    struct task_struct *rx_thread;
    spinlock_t ethlock_rx;
    spinlock_t ethlock_tx;
    spinlock_t ethlock_skblist;

} BcmEnet_devctrl;

#else

#ifndef FAP_4KE
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/nbuff.h>
#include "boardparms.h"
#include "bcm_OS_Deps.h"
#include <linux/bcm_log.h>
#include <bcmnet.h>
#include <bcm/bcmswapitypes.h>
#include <linux/version.h>
#endif

#include "bcmtypes.h"
#include "bcmenet_common.h"


/* Macros need to be defined after hardware dependent headers */
#define NUM_PORTS                   1

#if defined(_CONFIG_ENET_BCM_TM)
#define ENET_BCM_TM_NBR_OF_QUEUES   8
#define ENET_BCM_TM_NBR_OF_ENTRIES  200
typedef struct {
    void *txdma;
    uint8 *pBuf;
    uint16 dmaStatus;
    struct {
        uint8 is_spdsvc_setup_packet : 1;
        uint8 destQueue              : 7;
    };
    uint8 sw_port_id;
    uint32 key;
} enet_bcm_tm_param_t;
#endif

#if defined(ENET_EPON_CONFIG)
#define MAX_EPON_IFS 8 //eight LLID
extern struct net_device* eponifid_to_dev[];
extern void *epon_data_tx_func;
#endif

#if defined(ENET_GPON_CONFIG)
extern struct net_device* gponifid_to_dev[];
#endif

#ifdef RDPA_VPORTS
extern struct net_device *rdpa_vport_to_dev[];
#endif

#if defined(_BCMENET_LOCAL_)
#define ENET_POLL_DONE        0x80000000

typedef struct {
    unsigned int extPhyMask;
    int dump_enable;
    BcmEnet_devctrl *pVnetDev0_g;
    /* The following fields are for impl6 */
    struct task_struct *rx_thread;
    wait_queue_head_t rx_thread_wqh;
    int rx_work_avail;
    spinlock_t ethlock_rx;
    spinlock_t ethlock_tx;
    spinlock_t ethlock_skblist;
}enet_global_var_t;

extern enet_global_var_t global;
extern struct net_device *vnet_dev[];
extern int vport_to_logicalport[];
extern int logicalport_to_vport[];
#define EnetGetEthernetMacInfo() (((BcmEnet_devctrl *)netdev_priv(vnet_dev[0]))->EnetInfo)

#define LOGICAL_PORT_TO_VPORT(p) logicalport_to_vport[(p)]
#define VPORT_TO_LOGICAL_PORT(p) vport_to_logicalport[(p)]
#define CBPORT_TO_VPORTIDX(p) cbport_to_vportIdx[p]
#define IsLogPortWan(log_port) ((global.pVnetDev0_g->wanPort >> log_port) & 0x1)

typedef struct
{
    uint32 port_map         :16; /* Member port logical port map */ 
    uint32 is_lan_wan_cfg   :1; /* Is the LAN-WAN configuration applied to the group */
    uint32 reserved         :15;
    uint32 blog_chnl;           /* RX Blog Channel value for this bonding group ; Required so packet from multiple bonded RX ports can hit the same flow*/
    int lan_wan_rdpa_if;        /* rdpa_if value of SF2 port configured as WAN - only valid if "is_lan_wan_cfg" set */
    int lan_wan_log_port;       /* logical port value of SF2 port configured as WAN - only valid if "is_lan_wan_cfg" set */
}bond_info_t;

/* Note : Switch only supports two trunk groups, so use separate defines if Software needs more than two group */
#define BCM_SW_MAX_TRUNK_GROUPS           (2) 

#ifndef FAP_4KE

#ifdef DYING_GASP_API


/* Flag indicates we're in Dying Gasp and powering down - don't clear once set */
extern int dg_in_context; 

#define ENET_TX_LOCK() if(dg_in_context==0) spin_lock_bh(&global.pVnetDev0_g->ethlock_tx)
#define ENET_TX_UNLOCK() if(dg_in_context==0) spin_unlock_bh(&global.pVnetDev0_g->ethlock_tx)
#define ENET_RX_LOCK() if(dg_in_context==0) spin_lock_bh(&global.pVnetDev0_g->ethlock_rx)
#define ENET_RX_UNLOCK() if(dg_in_context==0) spin_unlock_bh(&global.pVnetDev0_g->ethlock_rx)
#define ENET_SKBLIST_LOCK() if(dg_in_context==0) spin_lock_bh(&global.pVnetDev0_g->ethlock_skblist)
#define ENET_SKBLIST_UNLOCK() if(dg_in_context==0) spin_unlock_bh(&global.pVnetDev0_g->ethlock_skblist)
#define ENET_MOCA_TX_LOCK() if(dg_in_context==0) spin_lock_bh(&global.pVnetDev0_g->ethlock_moca_tx)
#define ENET_MOCA_TX_UNLOCK() if(dg_in_context==0) spin_unlock_bh(&global.pVnetDev0_g->ethlock_moca_tx)

#else


#define ENET_TX_LOCK() spin_lock_bh(&global.pVnetDev0_g->ethlock_tx)
#define ENET_TX_UNLOCK() spin_unlock_bh(&global.pVnetDev0_g->ethlock_tx)
#define ENET_RX_LOCK() spin_lock_bh(&global.pVnetDev0_g->ethlock_rx)
#define ENET_RX_UNLOCK() spin_unlock_bh(&global.pVnetDev0_g->ethlock_rx)
#define ENET_SKBLIST_LOCK() spin_lock_bh(&global.pVnetDev0_g->ethlock_skblist)
#define ENET_SKBLIST_UNLOCK() spin_unlock_bh(&global.pVnetDev0_g->ethlock_skblist)
#define ENET_MOCA_TX_LOCK() spin_lock_bh(&global.pVnetDev0_g->ethlock_moca_tx)
#define ENET_MOCA_TX_UNLOCK() spin_unlock_bh(&global.pVnetDev0_g->ethlock_moca_tx)

#endif /* ELSE #ifdef DYING_GASP_API */


#endif
#endif /* #if defined(_BCMENET_LOCAL_) */

#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 5
#define local_unreachable() __builtin_unreachable()
#else
#define local_unreachable() do { } while(0)
#endif

#define DMA_THROUGHPUT_TEST_EN  0x80000

#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM947189)
#include "bcmenet_dma.h"
#endif

#if defined(PKTC)
#include <osl.h>
#endif

#if defined(_BCMENET_LOCAL_)
int bcmeapi_create_vport(struct net_device *dev);
void bcmeapi_napi_post(struct BcmEnet_devctrl *pDevCtrl);
int ethsw_phyport_rreg2(int phy_id, int reg, uint16 *data, int flags);
int ethsw_phyport_wreg2(int phy_id, int reg, uint16 *data, int flags);
PHY_STAT enet_get_ext_phy_stat(int unit, int phy_port, int cb_port);
/*
 * IMPORTANT: The following 3 macros are only used for ISR context. The
 * recycling context is defined by enet_recycle_context_t
 */
#define CONTEXT_CHAN_MASK   0x3
#define BUILD_CONTEXT(pDevCtrl,channel) \
            (uint32)((uint32)(pDevCtrl) | ((uint32)(channel) & CONTEXT_CHAN_MASK))
#define CONTEXT_TO_PDEVCTRL(context)    (BcmEnet_devctrl*)((context) & ~CONTEXT_CHAN_MASK)
#define CONTEXT_TO_CHANNEL(context)     (int)((context) & CONTEXT_CHAN_MASK)

unsigned short bcm_type_trans(struct sk_buff *skb, struct net_device *dev);


#endif /* _BCMENET_LOCAL_ */

int bcm63xx_enet_getPortFromName(char *pIfName, int *pUnit, int *pPort);
int bcm63xx_enet_getPortmapFromName(char *pIfName, int *pUnit, unsigned int *pPortmap);

/* int bcmeapi_map_interrupt(BcmEnet_devctrl *pDevCtrl) */
#define BCMEAPI_INT_MAPPED_INTPHY (1<<0)
#define BCMEAPI_INT_MAPPED_EXTPHY (1<<1)

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
#endif /* defined(RXCHANNEL_PKT_RATE_LIMIT) */

#ifdef DYING_GASP_API
int enet_send_dying_gasp_pkt(void);
#endif /* DYING_GASP_API */

void bcmeapi_reset_mib_cnt(uint32_t sw_port);
struct net_device *enet_phyport_to_vport_dev(int port);
struct net_device *phyPortId_to_netdev(int logical_port, int gemid);

int bcm63xx_enet_isExtSwPresent(void);
int bcm63xx_enet_intSwPortToExtSw(void);
unsigned int bcm63xx_enet_extSwId(void);
void bcmeapi_enet_module_cleanup(void);
uint32 ConfigureJumboPort(uint32 regVal, int portVal, unsigned int configVal);
void bcmeapi_module_init2(void);
void link_change_handler(int port, int cb_port, int linkstatus, int speed, int duplex);
int enet_get_next_crossbar_port(int logPort, int cb_port);
#define enet_get_first_crossbar_port(logPort) enet_get_next_crossbar_port(logPort, BP_CROSSBAR_NOT_DEFINED)
#endif /* !ENET_4908_GMAC */
#endif /* _BCMENET_H_ */

