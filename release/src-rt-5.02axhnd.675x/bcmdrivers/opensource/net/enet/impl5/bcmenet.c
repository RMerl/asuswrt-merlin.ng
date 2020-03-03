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
// File Name  : bcmenet.c
//
// Description: This is Linux network driver for Broadcom Ethernet controller
//
//**************************************************************************

#define VERSION     "0.1"
#define VER_STR     "v" VERSION

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
#include "br_private.h"
#include <net/arp.h>
#include <board.h>
#include <spidevices.h>
#include <bcmnetlink.h>
#include <bcm_intr.h>
#include <linux/bcm_realtime.h>
#include <linux/stddef.h>
#include <asm/atomic.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/nbuff.h>

#include <net/net_namespace.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
#include <linux/module.h>
#endif
#include <linux/version.h>

#if defined(_CONFIG_BCM_BPM)
#include <linux/gbpm.h>
#include "bpm.h"
#endif

#include "bcm_assert_locks.h"
#include "bcmmii.h"
#include "bcmenet.h"
#include "ethsw_phy.h"
#include "ethsw.h"
#include "bcmsw.h"
#include "bcmsw_cfp.h"
#include "eth_pwrmngt.h"

#include "bcmsw_api.h"
#include "bcmswaccess.h"
#include "bcmSpiRes.h"
#include "bcmswshared.h"
#include "ethswdefs.h"
#include "bcm_chip_arch.h"



#if defined(_CONFIG_BCM_INGQOS)
#include <linux/iqos.h>
#include "ingqos.h"
#endif

#if defined(CONFIG_BCM_GMAC)
#include <bcmgmac.h>
#endif

#if defined(CONFIG_BCM947189)
#include "bcmgmac_47189.h"
#endif

#ifdef CONFIG_BLOG
extern int bcm_tcp_v4_recv(pNBuff_t pNBuff, BlogFcArgs_t *fc_args);
#endif

#if defined(CONFIG_BCM_DPI_QOS_CPU)
#include "bcm_tm_defs.h"
#include "dpi_hooks.h"
#endif


#if defined(CONFIG_BCM_ETHTOOL)
#include "bcmenet_ethtool.h"
#endif

/* vnet_dev[0] is bcmsw device not attached to any physical port */
#define port_id_from_dev(dev) ((dev->base_addr == vnet_dev[0]->base_addr) ? MAX_TOTAL_SWITCH_PORTS : \
        ((BcmEnet_devctrl *)netdev_priv(dev))->sw_port_id)
extern int kerSysGetMacAddress(unsigned char *pucaMacAddr, unsigned long ulId);
static int bcm63xx_enet_open(struct net_device * dev);
static int bcm63xx_enet_close(struct net_device * dev);
static void bcm63xx_enet_timeout(struct net_device * dev);
static int bcm63xx_enet_poll_timer(void * arg);
static int bcm63xx_enet_xmit(pNBuff_t pNBuff, struct net_device * dev);
static inline int bcm63xx_enet_xmit2(struct net_device *dev, EnetXmitParams *pParam);
static struct rtnl_link_stats64 * bcm63xx_enet_query(struct net_device * dev, struct rtnl_link_stats64 *storage);
static int bcm63xx_enet_change_mtu(struct net_device *dev, int new_mtu);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 1)
static int bcm63xx_enet_rx_thread(void *arg);
#else
static int bcm63xx_enet_poll_napi(struct napi_struct *napi, int budget);
#endif
static uint32 bcm63xx_rx(void *ptr, uint32 budget);
static int bcm_set_soft_switching(int swPort, int type);
static int bcm_set_hw_stp(int swPort, int type);
static int bcm_set_mac_addr(struct net_device *dev, void *p);
static int bcm63xx_init_dev(BcmEnet_devctrl *pDevCtrl);
static int bcm63xx_uninit_dev(BcmEnet_devctrl *pDevCtrl);
static void __exit bcmenet_module_cleanup(void);
static int __init bcmenet_module_init(void);
static int bcm63xx_enet_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);
int __init bcm63xx_enet_probe(void);
static int bcm_mvtags_len(char *ethHdr);
static void bcmenet_update_pbvlan_all_bridge(void);

/* Sanity checks for user configured DMA parameters */
#if (CONFIG_BCM_DEF_NR_RX_DMA_CHANNELS > ENET_RX_CHANNELS_MAX)
#error "ERROR - Defined RX DMA Channels greater than MAX"
#endif
#if (CONFIG_BCM_DEF_NR_TX_DMA_CHANNELS > ENET_TX_CHANNELS_MAX)
#error "ERROR - Defined TX DMA Channels greater than MAX"
#endif

#if (ENET_RX_CHANNELS_MAX > 4)
#error "Overlaying channel and pDevCtrl into context param needs rework; check CONTEXT_CHAN_MASK "
#endif

struct kmem_cache *enetSkbCache;

static DECLARE_COMPLETION(poll_done);
static atomic_t poll_lock = ATOMIC_INIT(1);
static int poll_pid = -1;
struct net_device* vnet_dev[MAX_NUM_OF_VPORTS+1] = {[0 ... (MAX_NUM_OF_VPORTS)] = NULL};
int vport_to_logicalport[MAX_NUM_OF_VPORTS + 1] = {-1, 0, 1, 2, 3, 4, 5, 6, 7};
int logicalport_to_vport[MAX_TOTAL_SWITCH_PORTS] = {[0 ... (MAX_TOTAL_SWITCH_PORTS-1)] = -1};
int logicalport_to_oamIdx[MAX_TOTAL_SWITCH_PORTS] = {[0 ... (MAX_TOTAL_SWITCH_PORTS-1)] = -1};
uint32_t logicalport_to_imp_map[MAX_TOTAL_SWITCH_PORTS] = {[0 ... (MAX_TOTAL_SWITCH_PORTS-1)] = IMP_PORT_ID}; /* By default IMP port is 8 */
uint32_t imp_pbmap[BP_MAX_ENET_MACS] = {[0 ... (BP_MAX_ENET_MACS-1)] = PBMAP_MIPS}; /* By default IMP port is 8 */
uint32_t g_imp_use_lag = 0;
int g_oamIdx_map_found = 0;
PHY_STAT ext_phy_stat[BP_MAX_ENET_MACS][MAX_SWITCH_PORTS][BCMENET_CROSSBAR_MAX_EXT_PORTS] =
            { [0 ... (BP_MAX_ENET_MACS - 1)][0 ... (MAX_SWITCH_PORTS - 1)][0 ... (BCMENET_CROSSBAR_MAX_EXT_PORTS - 1)] = {0}};
int vport_cnt;  /* number of vports: bitcount of Enetinfo.sw.port_map */

#if defined(CONFIG_BCM_ETHTOOL)
extern const struct ethtool_ops enet_ethtool_ops;
#endif





static const struct net_device_ops bcm96xx_netdev_ops = {
    .ndo_open   = bcm63xx_enet_open,
    .ndo_stop   = bcm63xx_enet_close,
    .ndo_start_xmit   = (HardStartXmitFuncP)bcm63xx_enet_xmit,
#if defined(CONFIG_BCM_DPI_QOS_CPU)
    .ndo_dpi_enqueue = (HardStartXmitFuncP)(dpi_cpu_enqueue),
#endif
    .ndo_set_mac_address  = bcm_set_mac_addr,
    .ndo_do_ioctl   = bcm63xx_enet_ioctl,
    .ndo_tx_timeout   = bcm63xx_enet_timeout,
    .ndo_get_stats64  = bcm63xx_enet_query,
    .ndo_change_mtu     = bcm63xx_enet_change_mtu
};


#ifdef BCM_ENET_DEBUG_BUILD
/* Debug Variables */
/* Number of pkts received on each channel */
static int ch_pkts[ENET_RX_CHANNELS_MAX] = {0};
/* Number of times there are no rx pkts on each channel */
static int ch_no_pkts[ENET_RX_CHANNELS_MAX] = {0};
static int ch_no_bds[ENET_RX_CHANNELS_MAX] = {0};
/* Number of elements in ch_serviced debug array */
#define NUM_ELEMS 4000
/* -1 indicates beginning of an rx(). The bit31 indicates whether a pkt
   is received on that channel or not */
static unsigned int ch_serviced[NUM_ELEMS] = {0};
static int dbg_index;
#define NEXT_INDEX(index) ((++index) % NUM_ELEMS)
#define ISR_START 0xFF
#define WRR_RELOAD 0xEE
#endif
#define LLID_SINGLE 0

extsw_info_t extSwInfo = {
    .switch_id = 0,
    .brcm_tag_type = 0,
    .present = 0,
    .connected_to_internalPort = -1,
};

int bcmenet_in_init_dev = 0;

enet_global_var_t global = {
    .extPhyMask = 0,
    .dump_enable = 0,
    .pVnetDev0_g = NULL
};

/* Indicate whether the external PHY or external switch's PHY is connected */
static int bcmenet_use_ext_phy;

/* Record the return value from bcmeapi_map_interrupt(pDevCtrl) about if
    interrupt is used for internal and external PHY handling */
static int phy_int_mapped;

/* Indicate if port status polling is needed based on the phy_int_mapped */
static int force_link_check;

struct semaphore bcm_ethlock_switch_config;
struct semaphore bcm_link_handler_config;

spinlock_t bcm_ethlock_phy_access;
spinlock_t bcm_ethlock_phy_shadow;
spinlock_t bcm_extsw_access;
atomic_t phy_read_ref_cnt = ATOMIC_INIT(0);
atomic_t phy_write_ref_cnt = ATOMIC_INIT(0);
BcmEnet_devctrl *pVnetDev0_g;

#ifdef DYING_GASP_API

/* OAMPDU Ethernet dying gasp message */
unsigned char dg_ethOam_frame[64] = {
    1, 0x80, 0xc2, 0, 0, 2,
    0, 0,    0,    0, 0, 0, /* Fill Src MAC at the time of sending, from dev */
    0x88, 0x9,
    3, /* Subtype */
    5, /* code for DG frame */
    'B', 'R', 'O', 'A', 'D', 'C', 'O', 'M',
    ' ', 'B', 'C', 'G',

};

/* Socket buffer and buffer pointer for msg */
struct sk_buff *dg_skbp;

/* Flag indicates we're in Dying Gasp and powering down - don't clear once set */
int dg_in_context=0;

#endif

/* Drop precedence look up callback function. */
typedef unsigned int (*BCMENET_DP_LOOKUP_CB)(struct net_device *dev,
  uint8 *data, unsigned int len);

static BCMENET_DP_LOOKUP_CB bcmenet_dp_lookup_cb = NULL;

void bcmenet_register_dp_cb(BCMENET_DP_LOOKUP_CB dpcb) 
{
    bcmenet_dp_lookup_cb = dpcb;
}

EXPORT_SYMBOL(bcmenet_register_dp_cb);


/* Delete all the virtual eth ports */
static void delete_vport(void)
{
    int port;

    synchronize_net();

    for (port = 1; port <= vport_cnt; port++)
    {
        if (vnet_dev[port] != NULL)
        {
#ifdef SEPARATE_MAC_FOR_WAN_INTERFACES
            if(memcmp(vnet_dev[0]->dev_addr, vnet_dev[port]->dev_addr, ETH_ALEN)) {
                kerSysReleaseMacAddress(vnet_dev[port]->dev_addr);
            }
#endif
            unregister_netdev(vnet_dev[port]);
            free_netdev(vnet_dev[port]);
            vnet_dev[port] = NULL;
        }
    }
}

/* Indirection just to print the caller line number in case of error */
#define bcmenet_set_dev_mac_addr(a,b,c) _bcmenet_set_dev_mac_addr(a,b,c,__LINE__)

static int _bcmenet_set_dev_mac_addr(struct net_device *dev, struct sockaddr *pSockAddr,
                                     bool rtnlLockNeeded, int callerLine)
{
    int retVal;
    pSockAddr->sa_family = dev->type; /* set the sa_family same as dev type to avoid error */
    if (netif_running(dev)) {
        //dev_close(dev); /* ifconfig ethX down - MAC address change is not allowed on running device */
        // clear_bit === ifconfig ethX down ; close & open are unnecessary and costly
        clear_bit(__LINK_STATE_START, &dev->state);
        if (rtnlLockNeeded)
        {
            rtnl_lock();
        }
        retVal = dev_set_mac_address(dev, pSockAddr);
        if (rtnlLockNeeded)
        {
            rtnl_unlock();
        }

        //dev_open(dev); /* ifconfig ethX up */
        set_bit(__LINK_STATE_START, &dev->state);
        if (retVal) {
            printk("%d:ERROR<%d> - setting MAC address for dev %s\n",callerLine,retVal,dev->name);
        }
    }
    else {
        if (rtnlLockNeeded)
        {
            rtnl_lock();
        }
        retVal = dev_set_mac_address(dev, pSockAddr);
        if (rtnlLockNeeded)
        {
            rtnl_unlock();
        }

        if (retVal) {
            printk("%d:ERROR<%d> - setting MAC address for dev %s\n",callerLine,retVal,dev->name);
        }
    }
    return retVal;
}

static int validate_oam_idxs(uint32 port_map)
{
    int tempIdx, logical_port;
    /* OAM INDEX validation */
    for (logical_port=0; logical_port < MAX_TOTAL_SWITCH_PORTS; logical_port++)
    {
        if (logicalport_to_oamIdx[logical_port] != -1)
        {
            g_oamIdx_map_found = 1; /* OAM Index mapping is provided in board parameters */
            port_map &= ~(1<<logical_port); /* Reset logical port bit in map */
            for (tempIdx=logical_port+1; tempIdx < MAX_TOTAL_SWITCH_PORTS; tempIdx++)
            {
                if (logicalport_to_oamIdx[tempIdx] != -1 &&
                    logicalport_to_oamIdx[tempIdx] == logicalport_to_oamIdx[logical_port])
                {
                    /* Error --- OAM Index conflict */
                    printk("\n\n ERROR !! OAM INDEX <%d> Conflict <u:%d,p:%d> <u:%d,p:%d> \n\n",logicalport_to_oamIdx[tempIdx], 
                           LOGICAL_PORT_TO_UNIT_NUMBER(logical_port),LOGICAL_PORT_TO_PHYSICAL_PORT(logical_port),
                           LOGICAL_PORT_TO_UNIT_NUMBER(tempIdx),LOGICAL_PORT_TO_PHYSICAL_PORT(tempIdx));
                    return -1;
                }
            }
        }
    }

    if (port_map && g_oamIdx_map_found) /* If OAM Index is not provided for all port in board parameters -- FAIL */
    {
        /* Error --- OAM Index not available */
        printk("\n\n ERROR !! OAM INDEX not provided for all ports <0x%04x> in board parameter \n\n",(unsigned int)port_map);
        return -1;
    }
    return 0;
}

#if defined(_CONFIG_ENET_BCM_TM)
static int enet_bcm_tm_free(uint16_t length, void *param_p)
{
    enet_bcm_tm_param_t *enetParam_p = param_p;

    global.pVnetDev0_g->stats.tx_dropped++;
//    pParam->vstats->tx_dropped++;

    nbuff_flushfree((pNBuff_t)enetParam_p->key);

    return 0;
}

static int enet_bcm_tm_register(int port, struct net_device *dev)
{
    bcmFun_t *bcmFun;
    bcmTmDrv_arg_t tm;

    enet_bcm_tm_enqueue = bcmFun_get(BCM_FUN_ID_TM_ENQUEUE);
    BCM_ASSERT(enet_bcm_tm_enqueue != NULL);

    bcmFun = bcmFun_get(BCM_FUN_ID_TM_REGISTER);
    BCM_ASSERT(bcmFun != NULL);

    tm.phy = BCM_TM_DRV_PHY_TYPE_ETH;
    tm.port = port;
    tm.nbrOfQueues = ENET_BCM_TM_NBR_OF_QUEUES;
    tm.nbrOfEntries = ENET_BCM_TM_NBR_OF_ENTRIES;
    tm.paramSize = sizeof(enet_bcm_tm_param_t);
    tm.txCallbackFunc = enet_bcm_tm_xmit;
    tm.freeCallbackFunc = enet_bcm_tm_free;

    return bcmFun(&tm);
}
#endif

static char *print_phy_attribute(int phy_id)
{
    if (IsSerdes(phy_id)) return "Serdes PHY";
    if (IsExtPhyId(phy_id))
    {
        if (IsRGMII(phy_id)) return "External RGMII PHY";
        return "External PHY";
    }
    return "Internal PHY";
}

#if defined(CONFIG_BCM947189)
int getEmacCoreIndex(int logical_port)
{
    if (logical_port < MAX_SWITCH_PORTS)
        return logical_port; 
    else
        return BpGetPortConnectedToExtSwitch();
}
#endif

/* Create virtual eth ports: one for each physical switch port except
   for the GPON port */
static int create_vport(void)
{
    struct net_device *dev;
    struct sockaddr sockaddr;
    int status, vport_id, logical_port;
    int *map_p, sw_num=0, sw_pmap[BP_MAX_ENET_MACS];
    PHY_STAT phys = {};
    BcmEnet_devctrl *pDevCtrl = NULL;
    BcmEnet_devctrl *pVnetDev0 = (BcmEnet_devctrl *) netdev_priv(vnet_dev[0]);
    int phy_id, phy_idext, phy_conn;
    char *phy_devName;
    int unit = 0, physical_port = 0;
    unsigned long port_flags;
    int cb_port = BP_CROSSBAR_NOT_DEFINED, cbp; /* Initialize to invalid value */
    /* separate out the portmap for internal and external switch */
    /* For older chips - start creating Linux interfaces from the External Switch Port first - to stay backward compatible */
    sw_pmap[0] = pVnetDev0->allPortMap & SW_PORTMAP_EXT_M;
    sw_pmap[1] = pVnetDev0->allPortMap & SW_PORTMAP_M;
    if (vport_cnt > MAX_NUM_OF_VPORTS)
        return -1;

    sw_num = 0;
    map_p = &sw_pmap[sw_num]; /* Start with first switch */

    for ( vport_id = 1, logical_port = 0 ; vport_id < vport_cnt + 1; vport_id++, logical_port++, (*map_p) /= 2)
    {
        if (sw_num == 0 && sw_pmap[sw_num] == 0) { /* Done with all ports on first switch */
            sw_num++;
            map_p = &sw_pmap[sw_num]; /* ports from next switch */
            logical_port = 0; /* Reset logical port - moving to next switch port map */
        }
        /* Skip the switch ports which are not in the port_map */
        while (((*map_p) % 2) == 0)
        {
            (*map_p) /= 2;
            logical_port++;
        }

        /* Initialize the vport <--> phyport mapping tables */
        vport_to_logicalport[vport_id] = logical_port;
        logicalport_to_vport[logical_port] = vport_id;

        BCM_ASSERT(logical_port < MAX_TOTAL_SWITCH_PORTS);

        if (!bcmeapi_should_create_vport(logical_port))
        {
            (*map_p) /= 2;
            logical_port ++;
            vport_cnt--;
            continue;
        }

        physical_port = LOGICAL_PORT_TO_PHYSICAL_PORT(logical_port);
        unit = LOGICAL_PORT_TO_UNIT_NUMBER(logical_port);
        phy_id = pVnetDev0->EnetInfo[unit].sw.phy_id[physical_port];

        if(IsExtPhyId(phy_id) || IsExternalSwitchUnit(unit))
        {
            bcmenet_use_ext_phy = TRUE;
        }
        phy_conn = pVnetDev0->EnetInfo[unit].sw.phyconn[physical_port];
        phy_devName = pVnetDev0->EnetInfo[unit].sw.phy_devName[physical_port];
        port_flags = pVnetDev0->EnetInfo[unit].sw.port_flags[physical_port];

        if (pVnetDev0->EnetInfo[unit].sw.oamIndex[physical_port] >= 0) /* Any positive value */
        {
            logicalport_to_oamIdx[logical_port] = pVnetDev0->EnetInfo[unit].sw.oamIndex[physical_port];
        }

        /* TBD - Why we did not take care of crossbar ports ??*/


            dev = alloc_etherdev(sizeof(BcmEnet_devctrl));
            if (dev == NULL) {
                printk("%s: dev alloc failed \n", __FUNCTION__);
                delete_vport();
                return -ENOMEM;
            }

            pDevCtrl = netdev_priv(dev);
            memset(pDevCtrl, 0, sizeof(BcmEnet_devctrl));

            /* Set the pDevCtrl->dev to dev */
            pDevCtrl->dev = dev;

            if (phy_devName != PHY_DEVNAME_NOT_DEFINED)
            {
                dev_alloc_name(dev, phy_devName);
            }
            else
            {
                dev_alloc_name(dev, dev->name);
            }

            dev->netdev_ops             = vnet_dev[0]->netdev_ops;
#if defined(CONFIG_BCM_ETHTOOL)
            dev->ethtool_ops            = vnet_dev[0]->ethtool_ops;
#endif
            dev->priv_flags             |= vnet_dev[0]->priv_flags;
            dev->base_addr              = logical_port;

            dev->features               = vnet_dev[0]->features;
            dev->vlan_features          = vnet_dev[0]->vlan_features;
            /* Switch port id of this interface */
            pDevCtrl->sw_port_id        = logical_port; /* This is logical port number */


            status = bcmeapi_create_vport(dev);
            if (status)
            {
                /*cancel the device creation*/
                free_netdev(dev);
                return status;
            }

            netdev_path_set_hw_port(dev, logical_port, BLOG_ENETPHY);
            vnet_dev[vport_id] = dev;

            bcm63xx_enet_change_mtu(dev, BCM_ENET_DEFAULT_MTU_SIZE);

            /* The vport_id specifies the unique id of virtual eth interface */
            pDevCtrl->vport_id = vport_id;
            pDevCtrl->cb_ext_port = cb_port; /* Attach crossbar port only if virtual devices are created 
                                                for crossbar otherwise it should be BP_CROSSBAR_NOT_DEFINED */

            /* Set the default tx queue to 0 */
            pDevCtrl->default_txq = 0;
            pDevCtrl->use_default_txq = 0;
#if defined(CONFIG_BCM947189)
            pDevCtrl->emac_core = getEmacCoreIndex(logical_port);
#endif
            memmove(sockaddr.sa_data, vnet_dev[0]->dev_addr, ETH_ALEN);
            BCM_ENET_DEBUG("phy_id = %d", phy_id);

            /* Register the device after setting important stuff in device structure; 
               Kernel will call back some functions (e.g. get_stats) before we are fully done here */
            status = register_netdev(dev); 
            if (status != 0)
            {
                unregister_netdev(dev);
                free_netdev(dev);
                vnet_dev[vport_id] = NULL;
                return status;
            }

            /* rtnl lock needed since no rtnl lock taken, set 3rd arg to true */
            bcmenet_set_dev_mac_addr(dev, &sockaddr, true);

            /* Note: The parameter vport_id should be the vport_id-1. The ethsw_set_mac
               maps it to physical port id */
            if(pVnetDev0->unit == 0)
            {
                ethsw_set_mac(logical_port, phys);  
            }

            if(IsExternalSwitchPort(logical_port))
            {
                dev->priv_flags |= IFF_EXT_SWITCH;
            }
            if (IsPortSoftSwitching(port_flags))
            {
                bcm_set_soft_switching(logical_port, TYPE_ENABLE);
                if ( PHY_CONN_TYPE_PLC == phy_conn )
                {
                    bcm_set_hw_stp(logical_port, TYPE_DISABLE);
                }
            }
#if defined(CONFIG_BCM947189)
            if (PHY_CONN_TYPE_MOCA_ETH == phy_conn)
            {
                MoCA_eth_init();
            }
            /* 47189 does not have internal switch */
            if (unit == 0)
            {
                dev->priv_flags  &= ~IFF_HW_SWITCH;
            }
#endif
#if defined(_CONFIG_ENET_BCM_TM)
            enet_bcm_tm_register(logical_port, dev);
#endif
            if ((cbp = enet_get_first_crossbar_port(logical_port)) != BP_CROSSBAR_NOT_DEFINED)
            {
                printk("%s: <%s sw port: %d> <Logical : %02d> MAC : %02X:%02X:%02X:%02X:%02X:%02X\n",
                        dev->name, (unit?"Ext":"Int"), physical_port, logical_port,
                        dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2], dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);
                for (; cbp != BP_CROSSBAR_NOT_DEFINED;
                        cbp = enet_get_next_crossbar_port(logical_port, cbp))
                {
                    phy_id = enet_cb_port_to_phyid(unit, cbp);
                    printk("    Chip Physical Port %2d, Cross Bar Port %d, PHY_ID <0x%08x:0x%02x:%s >\n",
                            BP_CROSSBAR_PORT_TO_PHY_PORT(cbp), cbp,
                            phy_id, phy_id&BCM_PHY_ID_M, print_phy_attribute(phy_id));

                    if (enet_cb_port_has_combo_phy(unit, cbp))
                    {
                        enet_cb_port_get_phyids(unit, cbp, &phy_id, &phy_idext);
                        printk("                                             PHY_ID <0x%08x:0x%02x:External Cascaded PHY>\n",
                                phy_idext, phy_idext&BCM_PHY_ID_M);
                    }
                }
            }
            else
            {
                printk("%s: <%s sw port: %d> <Logical : %02d> PHY_ID <0x%08x:0x%02x:%s> MAC:%02X:%02X:%02X:%02X:%02X:%02X\n",
                        dev->name, (unit?"Ext":"Int"), physical_port, logical_port,
                        phy_id, phy_id&BCM_PHY_ID_M, print_phy_attribute(phy_id),
                        dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2], dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);
            }
    }

    /* OAM INDEX validation */
    if (validate_oam_idxs(pVnetDev0->allPortMap))
    {
        return -EINVAL;
    }
    return 0;
}

#undef OFFSETOF
#define OFFSETOF(STYPE, MEMBER)     ((size_t) &((STYPE *)0)->MEMBER)

static inline int bcm_mvtags_len(char *ethHdr)
{
    unsigned int end_offset = 0;
    struct vlan_ethhdr *vhd;
    BcmEnet_hdr* bhd;
    uint16 brcm_type;

    bhd = (BcmEnet_hdr*)ethHdr;
    brcm_type = ntohs(bhd->brcm_type);
    if (brcm_type == BRCM_TYPE)
    {
        end_offset += BRCM_TAG_LEN;
        bhd = (BcmEnet_hdr *)((uintptr_t)bhd + BRCM_TAG_LEN);
        brcm_type = ntohs(bhd->brcm_type);
    }

    /* FTTDP inband control traffic also uses 0x888A, but is not a switch tag. Assume they don't exist together */
    if (brcm_type == BRCM_TYPE2)
    {
        end_offset += BRCM_TAG_TYPE2_LEN;
    }

#ifdef VLAN_TAG_FFF_STRIP
    vhd = (struct vlan_ethhdr*)(ethHdr + end_offset);
    if((ntohs(vhd->h_vlan_proto) == VLAN_TYPE) &&
            (ntohs(vhd->h_vlan_TCI) & VLAN_VID_MASK) == 0xFFF)
    {
        end_offset += VLAN_HLEN;
    }
#endif

    return end_offset;
}

/*
 *  This is a modified version of eth_type_trans(), for taking care of
 *  Broadcom Tag with Ethernet type BRCM_TYPE [0x8874].
 */

unsigned short bcm_type_trans(struct sk_buff *skb, struct net_device *dev)
{
    struct ethhdr *eth;
    unsigned char *rawp;
    unsigned int end_offset = 0, from_offset = 0;
    uint16 *to, *end, *from;
    unsigned int hdrlen = sizeof(struct ethhdr);

    skb_reset_mac_header(skb);

    end_offset = bcm_mvtags_len(skb->data);
    if (end_offset)
    {
        from_offset = OFFSETOF(struct ethhdr, h_proto);

        to = (uint16*)(skb->data + from_offset + end_offset) - 1;
        end = (uint16*)(skb->data + end_offset) - 1;
        from = (uint16*)(skb->data + from_offset) - 1;

        while ( to != end )
            *to-- = *from--;
    }

    skb_set_mac_header(skb, end_offset);

    hdrlen += end_offset;

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

/* --------------------------------------------------------------------------
Name: bcm63xx_enet_open
Purpose: Open and Initialize the EMAC on the chip
-------------------------------------------------------------------------- */
static int bcm63xx_enet_open(struct net_device * dev)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);

    if (dev != vnet_dev[0])
    {
        if ((vnet_dev[0]->flags & IFF_UP) == 0)
            return -ENETDOWN;

        bcmeapi_add_dev_queue(dev);
        netif_start_queue(dev);
        return 0;
    }

    ASSERT(pDevCtrl != NULL);
    TRACE(("%s: bcm63xx_enet_open\n", dev->name));

    bcmeapi_open_dev(pDevCtrl, dev);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 1)
    /* enet does not use NAPI in 3.4 */
#else
    /* napi_enable must be called before the interrupts are enabled
       if an interrupt comes in before napi_enable is called the napi
       handler will not run and the interrupt will not be re-enabled */
    napi_enable(&pDevCtrl->napi);
#endif

    netif_start_queue(dev);

#if !defined(CONFIG_BCM_SWMDK)
    bcmsw_enable_all_macs_rxtx(1);
#endif

    return 0;
}

/* --------------------------------------------------------------------------
Name: bcm63xx_enet_close
Purpose: Stop communicating with the outside world
Note: Caused by 'ifconfig ethX down'
-------------------------------------------------------------------------- */
static int bcm63xx_enet_close(struct net_device * dev)
{
    BcmEnet_devctrl *pDevCtrl = netdev_priv(dev);

    if (dev != vnet_dev[0])
    {
        netif_stop_queue(dev);
        return 0;
    }

    ASSERT(pDevCtrl != NULL);
    TRACE(("%s: bcm63xx_enet_close\n", dev->name));

    bcmeapi_del_dev_intr(pDevCtrl);

    netif_stop_queue(dev);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 1)
    /* enet does not use NAPI in 3.4 */
#else
    napi_disable(&pDevCtrl->napi);
#endif

    return 0;
}

/* --------------------------------------------------------------------------
Name: bcm63xx_enet_timeout
Purpose:
-------------------------------------------------------------------------- */
static void bcm63xx_enet_timeout(struct net_device * dev)
{
    ASSERT(dev != NULL);
    TRACE(("%s: bcm63xx_enet_timeout\n", dev->name));

    dev->trans_start = jiffies;
    netif_wake_queue(dev);
}

#ifdef REPORT_HARDWARE_STATS
static void bcmsw_accumulate_stats(struct rtnl_link_stats64 *dstats64, 
                                   struct rtnl_link_stats64 *lstats64, 
                                   struct rtnl_link_stats64 *cstats64)
{
	int i;
    int num_flds = sizeof(*cstats64) / sizeof(u64); /* All fields are of same data-type */
	const u64 *cur = (const u64 *)cstats64;         /* Current stats from HW */
	const u64 *last = (const u64 *)lstats64;        /* Last snapshot of HW stats */
	u64 *now = (u64 *)dstats64;                     /* Device stats now */
    uint32 add_uint_max = 0;    
 
	for (i = 0; i < num_flds; i++)
    {
        /* NOTE: rx_bytes, tx_bytes are 64bit counters;
           Below wrap around may affect them if those counts wrap around (unlikely);
           rx_packets & tx_packets are 64bit as well (addition of ucast,mcast,bcast)
           so these will never wrap around at 64bit (ok)*/
        if (last[i] > cur[i]) /* wrap around */
        {
            add_uint_max = UINT_MAX; /* OK to add because HW is always returning 32bit values */
        }
        now[i] += (cur[i] + add_uint_max -last[i]); /* Add the difference from last time */ 
        add_uint_max = 0;
    }
    /* Store the current stats from HW into last stats snapshot */
    memcpy(lstats64, cstats64, sizeof(*lstats64));
}
#endif


/* --------------------------------------------------------------------------
Name: bcm63xx_enet_query
Purpose: Return the current statistics. This may be called with the card
open or closed.
-------------------------------------------------------------------------- */
static struct rtnl_link_stats64 * bcm63xx_enet_query(struct net_device * dev, 
                                                     struct rtnl_link_stats64 *stats64)
{
#ifdef REPORT_HARDWARE_STATS
    int port, log_port, extswitch;

    if (dev->base_addr == vnet_dev[0]->base_addr) /* bcmsw device */
    {
        memcpy(stats64, &(((BcmEnet_devctrl *)netdev_priv(dev))->stats), sizeof(*stats64));
        return stats64;
    }

    log_port = port_id_from_dev(dev);



    port = LOGICAL_PORT_TO_PHYSICAL_PORT(log_port);
    extswitch = IsExternalSwitchPort(log_port);

    if ((port < 0) || (port >= MAX_SWITCH_PORTS)) {
        printk("Invalid port <phy - %d : log - %d>, so stats will not be correct \n", port, log_port);
    }else {
        BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)netdev_priv(dev);
        uint32 rxDropped = 0, txDropped = 0;
        struct rtnl_link_stats64 cstats64; /* Current HW stats */
        /* reset stats before collecting - in case the called API does not provide anything */
        memset(stats64,0,sizeof(*stats64));
        memset(&cstats64,0,sizeof(cstats64));
        bcmsw_get_hw_stats(port, extswitch, &cstats64);
        bcmsw_accumulate_stats(&pDevCtrl->cHwStats, &pDevCtrl->lHwStats, &cstats64);
        memcpy(stats64, &pDevCtrl->cHwStats, sizeof(*stats64));
        /* Add the dropped packets in software */
        stats64->rx_dropped += pDevCtrl->stats.rx_dropped;
        stats64->tx_dropped += pDevCtrl->stats.tx_dropped;
        rxDropped = 0, txDropped = 0;
        bcmeapi_EthGetStats(log_port, &rxDropped, &txDropped);

        stats64->rx_dropped += rxDropped;
        stats64->tx_dropped += txDropped;
    }
    return stats64;
#else
    memcpy(stats64, &(((BcmEnet_devctrl *)netdev_priv(dev))->stats), sizeof(*stats64));
    return stats64;
#endif
}

static int bcm63xx_enet_change_mtu(struct net_device *dev, int new_mtu)
{

    int max_mtu = ENET_MAX_MTU_PAYLOAD_SIZE;

    if (new_mtu < ETH_ZLEN || new_mtu > max_mtu)
    {
        printk("MTU bcmenet max %d, new %d\n", max_mtu, new_mtu);
        return -EINVAL;
    }
    dev->mtu = new_mtu;

    return 0;
}

/*
 * handle_link_status_change
 */
void link_change_handler(int port, int cb_port, int linkstatus, int speed, int duplex)
{
    IOCTL_MIB_INFO *mib;
    int mask, vport;
    struct net_device *dev = vnet_dev[0];
    struct net_device *pNetDev;
    BcmEnet_devctrl *priv = (BcmEnet_devctrl *)netdev_priv(dev);
    int linkMask;
    int sw_port;
#if defined(CONFIG_BCM_ETH_PWRSAVE)
    int phyId = 0; // 0 is a valid phy_id but not an external phy_id. So we are OK initializing it to 0.

    if (!IsExternalSwitchPort(port))
    {
        phyId = priv->EnetInfo[0].sw.phy_id[LOGICAL_PORT_TO_PHYSICAL_PORT(port)];
    }

    ethsw_phy_pll_up(0);
#endif
    if (port >= MAX_TOTAL_SWITCH_PORTS)
    {
        return;
    }

    down(&bcm_link_handler_config);

    sw_port  = port;
    vport    = LOGICAL_PORT_TO_VPORT(port);
    mask     = 1 << port;
    /* Boundary condition check */
    /* This boundary check condition will fail for the internal switch port that is connected to
       the external switch as the vport would be -1. This is normal and expected */
    if ( vport >= 0 || vport < (sizeof(vnet_dev)/sizeof(vnet_dev[0])))
        pNetDev  = vnet_dev[vport];
    else
        pNetDev = NULL;
    linkMask = linkstatus << port;
    vport   -= 1; /* -1 for ethsw_set_mac */

    if ( NULL == pNetDev )
    {
        up(&bcm_link_handler_config);
        return;
    }

    if ((priv->linkState & mask) != linkMask) {
        BCM_ENET_LINK_DEBUG("port=%x; vport=%x", port, vport);

        mib = &((BcmEnet_devctrl *)netdev_priv(pNetDev))->MibInfo;
        if (linkstatus) {
            bcmeapi_link_check(port, linkstatus, speed);
#if defined(CONFIG_BCM_ETH_PWRSAVE)
            /* Link is up, so de-isolate the Phy  */
            if (IsExtPhyId(phyId))
            {
                ethsw_isolate_phy(phyId, 0);
            }
#endif


            /* Just set a flag for EEE because a 1 second delay is required */
            priv->eee_enable_request_flag[0] |= (1<<sw_port);

            if (speed == 2500)
            {
                mib->ulIfSpeed = SPEED_2500MBIT;
            }
            else if (speed == 1000)
            {
                mib->ulIfSpeed = SPEED_1000MBIT;
            }
            else if (speed == 100)
            {
                mib->ulIfSpeed = SPEED_100MBIT;
            }
            else if (speed == 200)
            {
                mib->ulIfSpeed = SPEED_200MBIT;
            }
            else
            {
                mib->ulIfSpeed = SPEED_10MBIT;
            }

            bcmeapi_EthSetPhyRate(port, 1, mib->ulIfSpeed, pNetDev->priv_flags & IFF_WANDEV);

#if defined(CONFIG_BCM_JUMBO_FRAME) && !defined(CONFIG_BCM_MAX_MTU_SIZE)
    {
        int we_locked_rtnl = FALSE;
        /* When jumbo frame support is enabled - the jumbo MTU is applicable only for 1000M interfaces */
        int max_mtu = (speed == 1000 ? ENET_MAX_MTU_PAYLOAD_SIZE : ENET_NON_JUMBO_MAX_MTU_PAYLOAD_SIZE);
            if (!rtnl_is_locked())
            {
                rtnl_lock();
                we_locked_rtnl = TRUE;
            }

            if (pNetDev->mtu > max_mtu)
                dev_set_mtu(pNetDev, max_mtu);
            if (we_locked_rtnl == TRUE) {
                rtnl_unlock();
            }
    }
#endif
            mib->ulIfLastChange  = (jiffies * 100) / HZ;
            mib->ulIfDuplex = (unsigned long)duplex;
            priv->linkState |= mask;
            if(IsLogPortWan(port))
            {
                priv->wanUpPorts++;
            }
            else
            {
                priv->lanUpPorts++;
            }
#if defined(CONFIG_BCM_EXT_SWITCH)
            bcmeapi_conf_que_thred();
            if (IsExternalSwitchPort(port) && mib->ulIfSpeed == SPEED_2500MBIT)
            {
                extsw_set_port_imp_map_2_5g();
            }
#endif
            bcmeapi_set_mac_speed(port, speed);
            printk((KERN_CRIT "%s (%s switch port: %d) (Logical Port: %d) Link UP %d mbps %s duplex\n"),
                    pNetDev->name, (LOGICAL_PORT_TO_UNIT_NUMBER(port)?"Ext":"Int"),LOGICAL_PORT_TO_PHYSICAL_PORT(port), port, speed, duplex?"full":"half");

            /* notify linux after we have finished setting our internal state */
            if (netif_carrier_ok(pNetDev) == 0)
                netif_carrier_on(pNetDev);
        } else {
#if defined(CONFIG_BCM_ETH_PWRSAVE)
            /* Link is down, so isolate the Phy. To prevent switch rx lockup
               because of packets entering switch with DLL/Clock disabled */
            if (IsExtPhyId(phyId) && !IsSerdes(phyId))
            {
                ethsw_isolate_phy(phyId, 1);
            }
#endif


            if (IsExternalSwitchPort(sw_port))
            {
                extsw_fast_age_port(LOGICAL_PORT_TO_PHYSICAL_PORT(sw_port), 0);
            }
            else
            {
                fast_age_port(LOGICAL_PORT_TO_PHYSICAL_PORT(sw_port), 0);
            }

#if defined(CONFIG_BCM_EXT_SWITCH)
            if (IsExternalSwitchPort(sw_port) && mib->ulIfSpeed == SPEED_2500MBIT)
            {
                extsw_set_port_imp_map_non_2_5g();
            }
#endif /* CONFIG_BCM_EXT_SWITCH */

            mib->ulIfLastChange  = 0;
            mib->ulIfSpeed       = 0;
            mib->ulIfDuplex      = 0;
            priv->linkState &= ~mask;

            if (IsLogPortWan(port))
            {
                priv->wanUpPorts--;
            }
            else
            {
                priv->lanUpPorts--;
            }
#if defined(CONFIG_BCM_EXT_SWITCH)
            bcmeapi_conf_que_thred();
#endif
            bcmeapi_EthSetPhyRate(port, 0, mib->ulIfSpeed, pNetDev->priv_flags & IFF_WANDEV);

            /* Clear any pending request to enable eee and disable it */
            priv->eee_enable_request_flag[0] &= ~(1<<sw_port);
            priv->eee_enable_request_flag[1] &= ~(1<<sw_port);
            BCM_ENET_DEBUG("%s: port %d  disabling EEE\n", __FUNCTION__, sw_port);
#if defined(CONFIG_BCM_GMAC)
            if (IsGmacPort( sw_port ) )
            {
                volatile GmacEEE_t *gmacEEEp = GMAC_EEE;
                gmacEEEp->eeeCtrl.linkUp = 0;
                gmacEEEp->eeeCtrl.enable = 0;
            }
#endif
            ethsw_eee_port_enable(sw_port, 0, 0);

            printk((KERN_CRIT "%s (%s switch port: %d) (Logical Port: %d) Link DOWN.\n"),
                    pNetDev->name, (LOGICAL_PORT_TO_UNIT_NUMBER(port)?"Ext":"Int"),LOGICAL_PORT_TO_PHYSICAL_PORT(port), port);

            /* notify linux after we have finished setting our internal state */
            if (netif_carrier_ok(pNetDev) != 0)
                netif_carrier_off(pNetDev);
        }

#if defined(CONFIG_BCM_ETH_DEEP_GREEN_MODE)
        ethsw_deep_green_mode_handler(priv->linkState);
#endif
    }

    up(&bcm_link_handler_config);
}

/*
 * Wrapper function for other Kernel modules to check
 * if a given logical port is WAN or NOT.
 */
static int bcmenet_is_wan_port(void *ctxt)
{
    int logical_port = *((int*)ctxt);
    return IsLogPortWan(logical_port);
}

static int bcmenet_is_bonded_lan_wan_port(void *ctxt)
{
    int ret_val = 0;
    return ret_val;
}

static int bcmenet_link_might_changed(void)
{
    return force_link_check | bcmeapi_link_might_changed();
}


#if defined(CONFIG_BCM_SWMDK)
static int link_change_handler_wrapper(void *ctxt)
{
    LinkChangeArgs *args = ctxt;

    BCM_ASSERT(args);
    if (args->activeELink) {
        bcmeapi_aelink_handler(args->linkstatus);
    }
    link_change_handler(args->port,
                        BP_CROSSBAR_NOT_DEFINED,
            args->linkstatus,
            args->speed,
            args->duplex);

#if defined(CONFIG_BCM_GMAC)
    if ( IsGmacPort(args->port) ) {
        gmac_link_status_changed(args->linkstatus, args->speed, args->duplex);
    }
#endif
    return 0;
}

/* SWMDK Support Case */
static int bcm63xx_enet_poll_timer(void * arg)
{
    struct net_device *dev = vnet_dev[0];
    BcmEnet_devctrl *priv = (BcmEnet_devctrl *)netdev_priv(dev);
    int i;
    long myJiffies;

    /* */
#if !defined(CONFIG_BCM947189)
    bcmeapiPhyIntEnable(1);
#endif

    /* Enable the Phy interrupts of internal Phys */
    for (i = 0; i < TOTAL_SWITCH_PORTS - 1; i++) {
        if ((priv->EnetInfo[0].sw.port_map) & (1<<i)) {
            if (!IsExtPhyId(priv->EnetInfo[0].sw.phy_id[i])) {
                ethsw_phy_intr_ctrl(i, 1);
            } else {
                global.extPhyMask |= (1 << i);
            }
        }
    }
#if defined(CONFIG_BCM_ENET)
    set_current_state(TASK_INTERRUPTIBLE);
    /* Sleep for 1 tick) */
    schedule_timeout(HZ/100);
#endif
    /* Start with virtual interfaces as down */
    for (i = 1; i <= vport_cnt; i++) {
        if ( vnet_dev[i] != NULL )
        {
            if (netif_carrier_ok(vnet_dev[i]) != 0)
                netif_carrier_off(vnet_dev[i]);
        }
    }

    /* */
    while (atomic_read(&poll_lock) > 0)
    {
        /* Adjust timer for consumed lose to make 1S timer more accurate */
        myJiffies = jiffies;

        /* reclaim tx descriptors and buffers */
        bcmeapi_enet_poll_timer();

        /* swmkd can't manage external phy connects to 47189 MAC directly */
#if defined(CONFIG_BCM947189)
        if (global.extPhyMask)
        {
            for (i = 0; i < BP_MAX_ENET_MACS; i++) {
                if (global.extPhyMask & (1 << i)) {
                    PHY_STAT phys;
                    int phy_port, log_port;
                    int unit, mask, tmp;

                    unit = 0;
                    phy_port = i;
                    log_port = PHYSICAL_PORT_TO_LOGICAL_PORT(phy_port, unit);
        
                    /* Get the status of Phy connected to switch port i */
                    phys = ethsw_phy_stat(unit, phy_port, BP_CROSSBAR_NOT_DEFINED);

                    /* Mask for this port */
                    mask = (1 << log_port);

                    /* If link is up, set tmp with the mask of this port */
                    tmp = (phys.lnk != 0) ? mask : 0;
                    if ((priv->linkState & mask) != tmp)
                    {
                        link_change_handler(log_port, BP_CROSSBAR_NOT_DEFINED, phys.lnk, 
                            phys.spd1000 ? 1000 : (phys.spd100 ? 100:10), phys.fdx);

                        /* if the phy connects to 47189 mac directly, need to reconfgiure mac by link speed */
                        if (phys.lnk)
                            gmac_link_status_changed(phy_port, phys.lnk, 
                                phys.spd1000 ? 1000 : (phys.spd100 ? 100:10), phys.fdx);
                    }
                }
            }
        }
#endif
        /*   */
        bcmeapi_update_link_status();
        set_current_state(TASK_INTERRUPTIBLE);

        /* Sleep for HZ jiffies (1sec) */
        myJiffies = HZ - (jiffies - myJiffies);
        if (myJiffies < 0) myJiffies = HZ;
        schedule_timeout(myJiffies);
    }

    complete_and_exit(&poll_done, 0);
    printk("bcm63xx_enet_poll_timer: thread exits!\n");

    return 0;
}

#else
/*
 * Non SWMDK Support Case
 * bcm63xx_enet_poll_timer: reclaim transmit frames which have been sent out
*/
static int bcm63xx_enet_poll_timer(void * arg)
{
    IOCTL_MIB_INFO *mib;
    PHY_STAT phys;
    int linkChanged = 0, oldstat, tmp, mask, i, log_port;
    int phyId, speed, unit, phy_port;
    struct net_device *dev = vnet_dev[0];
    BcmEnet_devctrl *priv = (BcmEnet_devctrl *)netdev_priv(dev);
    int ephy_sleep_delay = 0;

    /* */
#if !defined(CONFIG_BCM947189)
    bcmeapiPhyIntEnable(1);
#endif

    /* Enable the Phy interrupts of internal Phys */
    for (i = 0; i < EPHY_PORTS; i++) {
        if ((priv->EnetInfo[0].sw.port_map) & (1<<i)) {
            if (!IsExtPhyId(priv->EnetInfo[0].sw.phy_id[i])) {
                ethsw_phy_intr_ctrl(i, 1);
            }
        }
    }

#if defined(CONFIG_BCM_ENET)
    set_current_state(TASK_INTERRUPTIBLE);
    /* Sleep for 1 tick) */
    schedule_timeout(HZ/100);
#endif
    /* Start with virtual interfaces as down */
    for (i = 1; i <= vport_cnt; i++) {
        if ( vnet_dev[i] != NULL )
        {
            if (netif_carrier_ok(vnet_dev[i]) != 0) {
                netif_carrier_off(vnet_dev[i]);
            }
        }
    }

    /* */
    while (atomic_read(&poll_lock) > 0)
    {
        bcmeapi_enet_poll_timer();

        /* Start with New link status of all vports as 0*/
        oldstat = priv->linkState;

#if defined(CONFIG_BCM_ETH_PWRSAVE)
        ephy_sleep_delay = ethsw_ephy_auto_power_down_wakeup();
#endif

        if((linkChanged = bcmenet_link_might_changed()))
        {
            for (i = 1; i <= vport_cnt; i++)
            {
                if ( vnet_dev[i] != NULL )
                {
                    log_port = VPORT_TO_LOGICAL_PORT(i);
                    unit = LOGICAL_PORT_TO_UNIT_NUMBER(log_port);
                    phy_port = LOGICAL_PORT_TO_PHYSICAL_PORT(log_port);
                    phyId = priv->EnetInfo[unit].sw.phy_id[LOGICAL_PORT_TO_PHYSICAL_PORT(log_port)];


                    if (!linkChanged)
                        continue;

                    /* Get the MIB of this vport */
                    mib = &((BcmEnet_devctrl *)netdev_priv(vnet_dev[i]))->MibInfo;

                    /* Get the status of Phy connected to switch port i */
                    phys = ethsw_phy_stat(LOGICAL_PORT_TO_UNIT_NUMBER(log_port), LOGICAL_PORT_TO_PHYSICAL_PORT(log_port), BP_CROSSBAR_NOT_DEFINED);

                    /* Mask for this port */
                    mask = (1 << log_port);

                    /* If link is up, set tmp with the mask of this port */
                    tmp = (phys.lnk != 0) ? mask : 0;

                    /* Update the new link status */

                    /* If link status has changed for this switch port i, update
                       the interface status */
                    if ((linkChanged & ETHSW_LINK_MIGHT_CHANGED) ||
                            (priv->linkState & mask) != tmp)
                    {

                        priv->linkState &= ~mask;
                        priv->linkState |= tmp;
                        /* Set the MAC with link/speed/duplex status from Phy */
                        /* Note: The parameter i should be the vport id. The
                           ethsw_set_mac maps it to physical port id */
                        ethsw_set_mac(log_port, phys);

                        /* If Link has changed from down to up, indicate upper layers
                           and print the link status */
                        if (phys.lnk)
                        {
                            bcmeapi_link_check(log_port, phys.lnk, phys.spd1000? 1000:(phys.spd100? 100:10));
#if defined(CONFIG_BCM_ETH_PWRSAVE)
                            /* Link is up, so de-isolate the Phy  */
                            if (IsExtPhyId(phyId)) {
                                ethsw_isolate_phy(phyId, 0);
                            }
#endif

                            /* Just set a flag for EEE because a 1 second delay is required */
                            priv->eee_enable_request_flag[0] |= mask;

                            if (phys.spd100)
                                mib->ulIfSpeed = SPEED_100MBIT;
                            else if (phys.spd1000)
                                mib->ulIfSpeed = SPEED_1000MBIT;
                            else
                                mib->ulIfSpeed = SPEED_10MBIT;

                            mib->ulIfLastChange  = (jiffies * 100) / HZ;

                            if (phys.spd2500)
                                speed=2500;
                            else if (phys.spd1000)
                                speed=1000;
                            else if (phys.spd100)
                                speed=100;
                            else
                                speed=10;

                            bcmeapi_EthSetPhyRate(log_port, 1, mib->ulIfSpeed, vnet_dev[i]->priv_flags & IFF_WANDEV);

#if defined(CONFIG_BCM_GMAC)
                            if (IsGmacPort( log_port ) )
                            {
                                gmac_link_status_changed(1, speed, phys.fdx);
                            }
#endif

#if defined(CONFIG_BCM947189)
                            /* if the phy connects to 47189 mac directly, 
                             * need to reconfgiure mac by link speed 
                             */
                            if (unit == 0)
                            {
                                gmac_link_status_changed(phy_port, 1, speed, phys.fdx);
                            }
#endif

                            if (netif_carrier_ok(vnet_dev[i]) == 0)
                            {
                                printk((KERN_CRIT "%s Link UP %d mbps %s duplex\n"),
                                        vnet_dev[i]->name, speed, phys.fdx?"full":"half");
                                netif_carrier_on(vnet_dev[i]);
                            }
                        }
                        else
                        {
#if defined(CONFIG_BCM_ETH_PWRSAVE)
                            /* Link is down, so isolate the Phy. To prevent switch rx lockup
                               because of packets entering switch with DLL/Clock disabled */
                            if (IsExtPhyId(phyId)) {
                                ethsw_isolate_phy(phyId, 1);
                            }
#endif
                            /* Clear any pending request to enable eee and disable it */
                            priv->eee_enable_request_flag[0] &= ~mask;
                            priv->eee_enable_request_flag[1] &= ~mask;
                            ethsw_eee_port_enable(VPORT_TO_LOGICAL_PORT(i), 0, 0);

                            mib->ulIfLastChange  = 0;
                            mib->ulIfSpeed       = 0;

                            bcmeapi_EthSetPhyRate(log_port, 0, 0, vnet_dev[i]->priv_flags & IFF_WANDEV);

#if defined(CONFIG_BCM_GMAC)
                            if (IsGmacPort( log_port ) )
                            {
                                gmac_link_status_changed(0, 0, 0);
                            }
#endif

                            /* If link has changed from up to down, indicate upper
                               layers and print the 'Link Down' message */
                            if (netif_carrier_ok(vnet_dev[i]) != 0)
                            {
                                printk((KERN_CRIT "%s Link DOWN.\n"), vnet_dev[i]->name);
                                netif_carrier_off(vnet_dev[i]);
                            }

                        }
                    }
                }
            }
        }

#if defined(CONFIG_BCM_ETH_PWRSAVE)
        ephy_sleep_delay += ethsw_ephy_auto_power_down_sleep();
#endif

        /* Check for delayed request to enable EEE */
        ethsw_eee_process_delayed_enable_requests();

#if (CONFIG_BCM_EXT_SWITCH_TYPE == 53115)
        extsw_apd_set_compatibility_mode();
#endif

        bcmeapi_update_link_status();

        /*   */
        set_current_state(TASK_INTERRUPTIBLE);

        /* Sleep for HZ jiffies (1sec), minus the time that was already */
        /* spent waiting for EPHY PLL  */
        schedule_timeout(HZ - ephy_sleep_delay);
    }

    complete_and_exit(&poll_done, 0);
    printk("bcm63xx_enet_poll_timer: thread exits!\n");

    return 0;
}
#endif
#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)

/* --------------------------------------------------------------------------
Name: bcmenet_get_hash
Purpose: Simple Random hash based on MAC DA and SA
-------------------------------------------------------------------------- */
static inline unsigned int bcmenet_get_hash(unsigned char *pDa, unsigned char *pSa)
{
#if 0
    unsigned int hash_val = (unsigned int)(pDa[0]) + (unsigned short)(pDa[4]) +
                            (unsigned int)(pSa[0]) + (unsigned short)(pSa[4]);
    hash_val ^= ( hash_val >> 16 );
    hash_val ^= ( hash_val >>  8 );
    hash_val ^= ( hash_val >>  3 );

    return ( hash_val );
#else
    return pDa[5]+pSa[5];
#endif
}
#endif /* CONFIG_BCM_ENET_MULTI_IMP_SUPPORT*/

#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
/* P8/EMAC0=2.5G, P5/EMAC1=2.5G and P4/EMAC2=1.4G = 6.4G
 * Divide the hash into 64 Buckets and distribute :
 * 0-24 = EMAC0/P8          
 * 25-49 = EMAC1/P5         
 * 50-63 = EMAC2/P4         */
#define ENET_INVALID_PORT_ID (0xFFFF)
static uint16_t imp_to_emac_map[] = {
                                /* P0 - non-IMP */    ENET_INVALID_PORT_ID,
                                /* P1 - non-IMP */    ENET_INVALID_PORT_ID,
                                /* P2 - non-IMP */    ENET_INVALID_PORT_ID,
                                /* P3 - non-IMP */    ENET_INVALID_PORT_ID,
                                /* P4 -     IMP */     2,
                                /* P5 -     IMP */     1,
                                /* P6 - non-IMP */    ENET_INVALID_PORT_ID,
                                /* P7 - non-IMP */    ENET_INVALID_PORT_ID,
                                /* P8 -     IMP */     0
                                };

static inline uint16_t bcmenet_load_balance_imp(uint16 port_id, void* pHdr)
{
    uint16 emac_to_use = 0;
    if (g_imp_use_lag)
    {
        unsigned char *pDa = ((BcmEnet_hdr *)pHdr)->da;
        unsigned char *pSa = ((BcmEnet_hdr *)pHdr)->sa;
        unsigned int hash_value=0;
        hash_value = bcmenet_get_hash(pDa, pSa) % 64;
        emac_to_use = hash_value/25;
    }
    else /* Port Grouping is used */
    {
        emac_to_use = imp_to_emac_map[logicalport_to_imp_map[port_id]];
        if ( unlikely(emac_to_use == ENET_INVALID_PORT_ID) ) {
            printk("bcmenet_load_balance_imp() : Invalid IMP %d for Port %d\n",logicalport_to_imp_map[port_id],port_id);
            emac_to_use = 0;
        }
    }
    return emac_to_use;
}
#else
#define bcmenet_load_balance_imp(port_id, pHdr) 0
#endif /* CONFIG_BCM_ENET_MULTI_IMP_SUPPORT */
static struct sk_buff *bcm63xx_skb_put_tag(struct sk_buff *skb,
        struct net_device *dev, unsigned int port_map)
{
    BcmEnet_hdr *pHdr = (BcmEnet_hdr *)skb->data;
    int i, headroom;
    int tailroom;

    if (ntohs(pHdr->brcm_type) == BRCM_TYPE2)
    {
        headroom = 0;
        tailroom = ETH_ZLEN + BRCM_TAG_TYPE2_LEN - skb->len;
    }
    else
    {
        headroom = BRCM_TAG_TYPE2_LEN;
        tailroom = ETH_ZLEN - skb->len;
    }

    if (tailroom < 0)
    {
        tailroom = 0;
    }

#if defined(CONFIG_BCM_USBNET_ACCELERATION)
    if ((skb_writable_headroom(skb) < headroom) || (skb_tailroom(skb) < tailroom))
#else
    if ((skb_headroom(skb) < headroom) || (skb_tailroom(skb) < tailroom))
#endif
    {
        struct sk_buff *oskb = skb;
        skb = skb_copy_expand(oskb, headroom, tailroom, GFP_ATOMIC);
        kfree_skb(oskb);
        if (!skb)
        {
            return NULL;
        }
    }
#if defined(CONFIG_BCM_USBNET_ACCELERATION)
    else if ((headroom != 0) && (skb->clone_wr_head == NULL))
#else
    else if ((headroom != 0) && !(skb_clone_writable(skb, headroom)))
#endif
    {
        skb = skb_unshare(skb, GFP_ATOMIC);
        if (!skb)
        {
            return NULL;
        }
    }

    if (tailroom > 0)
    {
        if (skb_is_nonlinear(skb))
        {
            /* Non linear skb whose skb->len is < minimum Ethernet Packet Length
               (ETHZLEN or ETH_ZLEN + BroadcomMgmtTag Length) */
            if (skb_linearize(skb))
            {
                return NULL;
            }
        }
        memset(skb->data + skb->len, 0, tailroom);  /* padding to 0 */
        skb_put(skb, tailroom);
    }

    if (headroom != 0)
    {
        uint16 *to, *from;
        BcmEnet_hdr2 *pHdr2 = (BcmEnet_hdr2 *)skb_push(skb, headroom);
        to = (uint16*)pHdr2;
        from = (uint16*)(skb->data + headroom);
        for ( i=0; i<ETH_ALEN; *to++ = *from++, i++ ); /* memmove 2 * ETH_ALEN */
        /* set ingress brcm tag and TC bit */
        pHdr2->brcm_type = htons(BRCM_TAG2_EGRESS | (SKBMARK_GET_Q_PRIO(skb->mark) << 10));
        pHdr2->brcm_tag  = htons(port_map);
    }
    return skb;
}

static inline void bcm63xx_fkb_put_tag(FkBuff_t * fkb_p,
        struct net_device * dev, unsigned int port_map)
{
    int i;
    int tailroom;
    uint16 *from = (uint16*)fkb_p->data;
    BcmEnet_hdr2 *pHdr = (BcmEnet_hdr2 *)from;

    if (ntohs(pHdr->brcm_type) != BRCM_TYPE2)
    {
        uint16 * to = (uint16*)fkb_push(fkb_p, BRCM_TAG_TYPE2_LEN);
        pHdr = (BcmEnet_hdr2 *)to;
        for ( i=0; i<ETH_ALEN; *to++ = *from++, i++ ); /* memmove 2 * ETH_ALEN */
        /* set port of ingress brcm tag */
        pHdr->brcm_tag = htons(port_map);

    }
    /* set ingress brcm tag and TC bit */
    pHdr->brcm_type = htons(BRCM_TAG2_EGRESS | (SKBMARK_GET_Q_PRIO(fkb_p->mark) << 10));
    tailroom = ETH_ZLEN + BRCM_TAG_TYPE2_LEN - fkb_p->len;
    if (tailroom > 0)
    {
        fkb_pad(fkb_p, tailroom);
        fkb_p->dirty_p = _to_dptr_from_kptr_(fkb_p->data + fkb_p->len);
    }
}

#if defined(CONFIG_BCM_6802_MoCA)
/* Will be removed once bmoca-6802.c remove the calling */
void bcmenet_register_moca_fc_bits_cb(void (*cb)(void *, unsigned long *), int isWan, void * arg)
{
    return;
}
EXPORT_SYMBOL(bcmenet_register_moca_fc_bits_cb);
#endif




void inline get_mark_pNbuff(pNBuff_t *pNBuff, uint32_t **mark)
{
    void * pBuf = PNBUFF_2_PBUF(pNBuff);

     if ( IS_SKBUFF_PTR(pNBuff) )
    {
        *mark = (uint32_t *)&(((struct sk_buff *)pBuf)->mark);
    }
    else
    {
        *mark = (uint32_t *)&(((FkBuff_t *)pBuf)->mark); 
    }
}

/* --------------------------------------------------------------------------
Name: bcm63xx_enet_xmit
Purpose: Send ethernet traffic
-------------------------------------------------------------------------- */
static int bcm63xx_enet_xmit(pNBuff_t pNBuff, struct net_device *dev)
{
    bool is_chained = FALSE;
#if defined(PKTC)
    pNBuff_t pNBuff_next = NULL;
#endif
    EnetXmitParams param = { {0}, 0};
    int ret;
    uint32_t *mark;

#if defined(PKTC)
    /* for PKTC, pNBuff is chained skb */
    if (IS_SKBUFF_PTR(pNBuff))
    {
        is_chained = PKTISCHAINED(pNBuff);
    }
#endif

    param.pDevPriv = netdev_priv(dev);
    param.port_id  = port_id_from_dev(dev);
    param.is_chained = is_chained;
    param.vstats   = &param.pDevPriv->stats;
    do {
        param.pNBuff = pNBuff;
        BCM_ENET_TX_DEBUG("The physical port_id is %d\n", param.port_id);

        get_mark_pNbuff(pNBuff, &mark);
        param.pDevPriv->estats.tx_packets_queue_in[SKBMARK_GET_Q_PRIO(*mark)]++;

#if defined(CONFIG_BCM963268) || defined(CONFIG_BCM63381) || defined(CONFIG_BCM947189)
        /* Packet is NOT chained && is_SKB && Not_Classified_by_Kernel */
        if (!is_chained && IS_SKBUFF_PTR(pNBuff) && !SKBMARK_GET_FLOW_ID(*mark))
        {
            struct sk_buff *skb = PNBUFF_2_SKBUFF(pNBuff);
            int hiPrioFlag=0;
            check_arp_lcp_pkt(skb->data, hiPrioFlag);
            if (hiPrioFlag)
            {
                uint32_t hiPrioQ = MAX_PRIORITY_VALUE;
                /* Give the highest possible priority to ARP/LCP packets */
                *mark = SKBMARK_SET_Q_PRIO(*mark, hiPrioQ);
            }
        }   
#endif

        /* Do driver level queue remapping */
        bcmeapi_enet_prepare_xmit(dev, mark);

        if (nbuff_get_params_ext(pNBuff, &param.data, &param.len,
                    &param.mark, &param.priority, &param.r_flags) == NULL)
        {
            global.pVnetDev0_g->stats.tx_dropped++;
            param.vstats->tx_dropped++;
            param.pDevPriv->estats.tx_dropped_bad_nbuff++;
            return 0;
        }

        if (global.dump_enable)
            DUMP_PKT(param.data, param.len);

#ifdef USE_DEFAULT_EGRESS_QUEUE
        if (param.pDevPriv->use_default_txq)
        {
            BCM_ENET_TX_DEBUG("Using default egress queue %d \n", param.egress_queue);
            param.egress_queue = SKBMARK_GET_Q_PRIO(param.mark);
            bcmeapi_select_tx_def_queue(&param);
        }
        else
#endif
        {
            BCM_ENET_TX_DEBUG("Using mark for channel and queue \n");
            param.egress_queue = SKBMARK_GET_Q_PRIO(param.mark);
            bcmeapi_select_tx_nodef_queue(&param);
        }

        BCM_ENET_TX_DEBUG("The egress queue is %d \n", param.egress_queue);

#if defined(PKTC)
        if (is_chained)
            pNBuff_next = PKTCLINK(pNBuff);
#endif

        ret = bcm63xx_enet_xmit2(dev, &param);

#if defined(PKTC)
        if (is_chained)
        {
            pNBuff = pNBuff_next;
        }
#endif

    } while (is_chained && pNBuff && IS_SKBUFF_PTR(pNBuff));

    return ret;
}

static inline int bcm63xx_enet_xmit2(struct net_device *dev, EnetXmitParams *pParam)
{
    unsigned int port_map ; 

    pParam->lag_port =  bcmenet_load_balance_imp(pParam->port_id, pParam->data);
    port_map = (1 << pParam->port_id);
    pParam->gemid = -1;
    /* tx request should never be on the bcmsw interface */
    BCM_ASSERT_R((dev != vnet_dev[0]), 0);

    if(IS_FKBUFF_PTR(pParam->pNBuff))
    {
        pParam->pFkb = PNBUFF_2_FKBUFF(pParam->pNBuff);
    }
    else
    {
        pParam->skb = PNBUFF_2_SKBUFF(pParam->pNBuff);
    }
    bcmeapi_get_tx_queue(pParam);

        {
           pParam->blog_chnl = pParam->port_id;
           pParam->blog_phy  = BLOG_ENETPHY;
        }

#ifdef CONFIG_BLOG
    /*
     * Pass to blog->fcache, so it can construct the customized
     * fcache based execution stack.
     */
    if (pParam->is_chained == FALSE)
    {
#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
        if (IS_SKBUFF_PTR(pParam->pNBuff))
        {
            if (PNBUFF_2_SKBUFF(pParam->pNBuff)->blog_p)
            {
                PNBUFF_2_SKBUFF(pParam->pNBuff)->blog_p->lag_port = pParam->lag_port;
            }
        }
#endif /* CONFIG_BCM_ENET_MULTI_IMP_SUPPORT */
        blog_emit( pParam->pNBuff, dev, TYPE_ETH, pParam->blog_chnl, pParam->blog_phy ); /* CONFIG_BLOG */
    }
#endif

    bcmeapi_buf_reclaim(pParam);

    if(bcmeapi_queue_select(pParam) == BCMEAPI_CTRL_BREAK)
    {
        pParam->pDevPriv->estats.tx_dropped_no_lowlvl_resource++;
        goto unlock_drop_exit;
    }

    bcmeapi_config_tx_queue(pParam);

    if (IsExternalSwitchPort(pParam->port_id))
    {
        if ( pParam->pFkb ) {
            FkBuff_t * pFkbOrig = pParam->pFkb;
            pParam->pFkb = fkb_unshare(pFkbOrig);

            if (pParam->pFkb == FKB_NULL)
            {
                fkb_free(pFkbOrig);
                global.pVnetDev0_g->stats.tx_dropped++;
                pParam->vstats->tx_dropped++;
                pParam->pDevPriv->estats.tx_dropped_no_fkb++;
                goto unlock_exit;
            }
            bcm63xx_fkb_put_tag(pParam->pFkb, dev, GET_PORTMAP_FROM_LOGICAL_PORTMAP(port_map, 1)); /* Portmap for external switch */
            pParam->data = pParam->pFkb->data;
            pParam->len  = pParam->pFkb->len;
            pParam->pNBuff = PBUF_2_PNBUFF((void*)pParam->pFkb,FKBUFF_PTR);
        } else {
            ENET_TX_UNLOCK();
            pParam->skb = bcm63xx_skb_put_tag(pParam->skb, dev, GET_PORTMAP_FROM_LOGICAL_PORTMAP(port_map,1));    /* Portmap for external switch and also pads to 0 */
            ENET_TX_LOCK();
            if (pParam->skb == NULL) {
                global.pVnetDev0_g->stats.tx_dropped++;
                pParam->vstats->tx_dropped++;
                pParam->pDevPriv->estats.tx_dropped_no_skb++;
                goto unlock_exit;
            }
            pParam->data = pParam->skb->data;   /* Re-encode pNBuff for adjusted data and len */
            pParam->len  = pParam->skb->len;
            pParam->pNBuff = PBUF_2_PNBUFF((void*)pParam->skb,SKBUFF_PTR);
        }

    }

    if ( pParam->len < ETH_ZLEN )
    {
        nbuff_pad(pParam->pNBuff, ETH_ZLEN - pParam->len);
        nbuff_set_len (pParam->pNBuff, ETH_ZLEN) ;
        nbuff_get_context(pParam->pNBuff, &pParam->data, (uint32_t *)&pParam->len);
    }

    if (bcmenet_dp_lookup_cb != NULL)
    {
        pParam->drop_eligible = bcmenet_dp_lookup_cb(dev, pParam->data, pParam->len);
    }

    switch(bcmeapi_pkt_xmt_dispatch(pParam))
    {
        case BCMEAPI_CTRL_SKIP:
            goto unlock_drop_exit;
        default:
            break;
    }

#ifdef DYING_GASP_API
    /* If in dying gasp, abort housekeeping since we're about to power down */
    if(dg_in_context)
        return 0;
#endif

    /* update stats */
    pParam->vstats->tx_bytes += pParam->len + ETH_CRC_LEN;
    pParam->vstats->tx_packets++;
    pParam->pDevPriv->estats.tx_packets_queue_out[pParam->egress_queue]++;

    global.pVnetDev0_g->stats.tx_bytes += pParam->len + ETH_CRC_LEN;
    global.pVnetDev0_g->stats.tx_packets++;
    global.pVnetDev0_g->dev->trans_start = jiffies;

unlock_exit:
    bcmeapi_xmit_unlock_exit_post(pParam);
    return 0;

unlock_drop_exit:
    global.pVnetDev0_g->stats.tx_dropped++;
    pParam->vstats->tx_dropped++;
    bcmeapi_xmit_unlock_drop_exit_post(pParam);
    return 0;

}



#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 1)
/* Driver for kernel 3.4 uses a dedicated thread for rx processing */
static int bcm63xx_enet_rx_thread(void *arg)
{
    struct BcmEnet_devctrl *pDevCtrl=(struct BcmEnet_devctrl *) arg;
    uint32 work_done;
    uint32 ret_done;
    int budget = NETDEV_WEIGHT;


    while (1)
    {
        wait_event_interruptible(pDevCtrl->rx_thread_wqh,
                pDevCtrl->rx_work_avail);

        if (kthread_should_stop())
        {
            printk(KERN_INFO "kthread_should_stop detected on bcmsw-rx\n");
            break;
        }

        /* got some work to do */
        bcmeapi_update_rx_queue(pDevCtrl);

        work_done = bcm63xx_rx(pDevCtrl, budget);
        ret_done = work_done & ENET_POLL_DONE;
        work_done &= ~ENET_POLL_DONE;

        BCM_ENET_RX_DEBUG("Work Done: %d \n", (int)work_done);

        if (ret_done == ENET_POLL_DONE)
        {
            /*
             * No more packets.  Indicate we are done (rx_work_avail=0) and
             * re-enable interrupts (bcmeapi_napi_post) and go to top of
             * loop to wait for more work.
             */
            pDevCtrl->rx_work_avail = 0;
            bcmeapi_napi_post(pDevCtrl);
        }
        else
        {
            bcmeapi_napi_leave(pDevCtrl);

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
#else
/* Driver for kernel 2.6.30 uses NAPI. */
static int bcm63xx_enet_poll_napi(struct napi_struct *napi, int budget)
{
    struct BcmEnet_devctrl *pDevCtrl = container_of(napi, struct BcmEnet_devctrl, napi);

    uint32 work_done;
    uint32 ret_done;

    bcmeapi_update_rx_queue(pDevCtrl);

    work_done = bcm63xx_rx(pDevCtrl, budget);
    ret_done = work_done & ENET_POLL_DONE;
    work_done &= ~ENET_POLL_DONE;

    BCM_ENET_RX_DEBUG("Work Done: %d \n", (int)work_done);

    if (work_done == budget || ret_done != ENET_POLL_DONE)
    {
        bcmeapi_napi_leave(pDevCtrl);
        /* We have either exhausted our budget or there are
           more packets on the DMA (or both).  Simply
           return, and the framework will reschedule
           this function automatically */
        return work_done;
    }

    /* we are done processing packets */

    napi_complete(napi);
    bcmeapi_napi_post(pDevCtrl);
    return work_done;
}
#endif  /* LINUX_VERSION_CODE */

struct net_device *phyPortId_to_netdev(int logical_port, int gemid)
{
    struct net_device *dev = NULL;
        {
            int vport;
            /* LAN or ETH-WAN ports */
            vport = LOGICAL_PORT_TO_VPORT(logical_port);
            BCM_ENET_RX_DEBUG("logical_port=%d vport=%d\n", (int)logical_port, vport);

            if ((vport > 0) && (vport <= vport_cnt))
            {
                dev = vnet_dev[vport];
            }

        }
        return dev;
}

/*
 *  bcm63xx_rx: Process all received packets.
 */
static uint32 bcm63xx_rx(void *ptr, uint32 budget)
{
    BcmEnet_devctrl *pDevCtrl = ptr;
    struct net_device *dev = NULL;
    BlogFcArgs_t fc_args;
    unsigned char *pBuf = NULL;
    struct sk_buff *skb = NULL;
    int len=0, phy_port_id = -1, no_stat = 0, ret;
    uint32 rxpktgood = 0, rxpktprocessed = 0;
    uint32 rxpktmax = budget + (budget / 2);
    struct rtnl_link_stats64 *vstats;
    struct enet_device_stats *estats;
    FkBuff_t * pFkb = NULL;
    uint32_t blog_chnl, blog_phy; /* used if CONFIG_BLOG enabled */
    uint32 rxContext1 = 0; /* Dummy variable used to hold the value returned from called function -
                              MUST not be changed from caller */
    int is_wifi_port = 0;
    int rxQueue;
    int gemid = 0;
#if defined(CONFIG_BLOG)
    BlogAction_t blogAction;
#endif

    // TBD -- this can be looked into but is not being done for now
    /* When the Kernel is upgraded to 2.6.24 or above, the napi call will
       tell you the received queue to be serviced. So, loop across queues
       can be removed. */
    /* RR loop across channels until either no more packets in any channel or
       we have serviced budget number of packets. The logic is to keep the
       channels to be serviced in next_channel array with channels_tbd
       tracking the number of channels that still need to be serviced. */
    for(bcmeapi_prepare_rx(); --budget > 0 && (rxpktgood & ENET_POLL_DONE) == 0; dev = NULL, pBuf = NULL, skb = NULL)
    {
        ret = bcmeapi_rx_pkt(pDevCtrl, &pBuf, &pFkb, &len, &gemid, &phy_port_id, &is_wifi_port, &dev, &rxpktgood, &rxContext1, &rxQueue);

        if(ret & BCMEAPI_CTRL_FLAG_TRUE)
        {
            no_stat = 1;
            ret &= ~BCMEAPI_CTRL_FLAG_MASK;
        }
        else
        {
            no_stat = 0;
        }

        if(ret != BCMEAPI_CTRL_TRUE)
        {
            switch(ret)
            {
                case BCMEAPI_CTRL_BREAK:
                    goto after_rx;
                case BCMEAPI_CTRL_CONTINUE:
                    goto next_rx;
                case BCMEAPI_CTRL_SKIP:
                    continue;
                default:
                    break;
            }
        }

        BCM_ENET_RX_DEBUG("Processing Rx packet");
        rxpktprocessed++;


        if (!is_wifi_port)/*TODO remove is_wifi_port from enet driver*/
        {
            dev = phyPortId_to_netdev(phy_port_id, gemid);
        }

        if(dev == NULL)
        {
            /* possibility of corrupted source port in dmaFlag */
            if (!no_stat)
                pDevCtrl->stats.rx_dropped++;

            pDevCtrl->estats.rx_dropped_no_rxdev++;
            bcmeapi_kfree_buf_irq(pDevCtrl, pFkb, pBuf);
            BCM_ENET_INFO("ETH Rcv: Pkt with invalid phy_port_id/vport(0x%x/0x%x) or gemid = 0x%x\n",
                    phy_port_id, LOGICAL_PORT_TO_VPORT(phy_port_id), gemid);
            goto next_rx;
        }

        vstats = &(((BcmEnet_devctrl *) netdev_priv(dev))->stats);
        estats = &(((BcmEnet_devctrl *) netdev_priv(dev))->estats);
        if (!no_stat)
        {
            /* Store packet & byte count in our portion of the device structure */
            vstats->rx_packets ++;
            vstats->rx_bytes += len;
            estats->rx_packets_queue[rxQueue]++;

            /* Store packet & byte count in switch structure */
            pDevCtrl->stats.rx_packets++;
            pDevCtrl->stats.rx_bytes += len;
        }


            {
#if 1 /*TODO remove is_wifi_port */
                if (is_wifi_port)
                {
                    blog_chnl = 0;
                    blog_phy = BLOG_WLANPHY;/* blog rx phy type is WLAN */
                }
                else
#endif
                {
                    blog_chnl = phy_port_id;/* blog rx channel is switch port */
                    blog_phy = BLOG_ENETPHY;/* blog rx phy type is ethernet */

                }
            }

        /* FkBuff_t<data,len> in-placed leaving headroom */
        pFkb = fkb_init(pBuf, BCM_PKT_HEADROOM,
                pBuf, len - ETH_CRC_LEN );
        bcmeapi_set_fkb_recycle_hook(pFkb);


#ifdef CONFIG_BLOG
        blogAction = blog_finit_args( pFkb, dev, TYPE_ETH, blog_chnl, blog_phy, &fc_args);

        if ( blogAction == PKT_DROP )
        {
            bcmeapi_blog_drop(pDevCtrl, pFkb, pBuf);

            /* Store dropped packet count in our portion of the device structure */
            vstats->rx_dropped++;
            estats->rx_dropped_blog_drop++;

			/* Store dropped packet count in switch structure */
            pDevCtrl->stats.rx_dropped++;
            goto next_rx;
        }
        else
        {
            bcmeapi_buf_alloc(pDevCtrl);
        }

        /* packet consumed, proceed to next packet*/
        if ( blogAction == PKT_DONE )
        {
            estats->rx_packets_blog_done++;
            goto next_rx;
        }
        else if ( blogAction == PKT_TCP4_LOCAL)
        {
            estats->rx_packets_blog_done++;
            bcm_tcp_v4_recv((void*)CAST_REAL_TO_VIRT_PNBUFF(pFkb,FKBUFF_PTR) ,&fc_args);
            goto next_rx;
        }

        if (blogAction != PKT_NORM && is_wifi_port)
            pFkb->blog_p->wl_hw_support.is_rx_hw_acc_en = 1;

#endif /* CONFIG_BLOG */

        /*allocate skb & initialize it using fkb */

        if (bcmeapi_alloc_skb(pDevCtrl, &skb) == BCMEAPI_CTRL_FALSE) {
            fkb_release(pFkb);
            pDevCtrl->stats.rx_dropped++;
            estats->rx_dropped_no_skb++;
            bcmeapi_kfree_buf_irq(pDevCtrl, pFkb, pBuf);
            if ( rxpktprocessed < rxpktmax )
                continue;
            break;
        }

        if(bcmeapi_skb_headerinit(len, pDevCtrl, skb, pFkb, pBuf) == BCMEAPI_CTRL_CONTINUE)
        {
            estats->rx_dropped_skb_headinit++;
            bcmeapi_kfree_buf_irq(pDevCtrl, pFkb, pBuf);
            goto next_rx;
        }

        skb->protocol = bcm_type_trans(skb, dev);
        skb->dev = dev;
        if (global.dump_enable) {
            DUMP_PKT(skb->data, skb->len);
        }
        
#if defined(CONFIG_BCM_CSO)
        if(pFkb->rx_csum_verified)
        {
            skb->ip_summed = CHECKSUM_UNNECESSARY;
        }
#endif

#if defined(CONFIG_BCM_DPI_QOS_CPU)
        if(dev->priv_flags & IFF_WANDEV)
        {
            ret = dpi_cpu_enqueue(SKBUFF_2_PNBUFF(skb), dev);
            if(ret != BCM_TM_TX_FULL)
            {
                estats->rx_packets_netif_receive_skb++;
            }
        }
        else
#endif
        {
            estats->rx_packets_netif_receive_skb++;
            local_bh_disable();
            netif_receive_skb(skb);
            local_bh_enable();
        }

next_rx:
        if (bcmeapi_prepare_next_rx(&rxpktgood) != BCMEAPI_CTRL_CONTINUE)
        {
            break;
        }
    } /* end while (budget > 0) */

after_rx:
    pDevCtrl->dev->last_rx = jiffies;

    bcmeapi_rx_post(&rxpktgood);

    BCM_ASSERT_NOT_HAS_SPINLOCK_C(&global.pVnetDev0_g->ethlock_rx);

    return rxpktgood;
}


static int bcm_set_soft_switching(int swPort, int type)
{
    int vport;

    BcmEnet_devctrl *pDevCtrl = netdev_priv(vnet_dev[0]);
    ASSERT(pDevCtrl != NULL);

    vport = LOGICAL_PORT_TO_VPORT(swPort);
    if (type == TYPE_ENABLE) {
        pDevCtrl->softSwitchingMap  |= (1 << swPort);
        vnet_dev[vport]->priv_flags &= ~IFF_HW_SWITCH;
    }
    else if (type == TYPE_DISABLE) {
        pDevCtrl->softSwitchingMap &= ~(1 << swPort);
        vnet_dev[vport]->priv_flags |= IFF_HW_SWITCH;
    }
    else {
        return -EINVAL;
    }

    if (IsExternalSwitchPort(swPort))
    {
        extsw_set_wanoe_portmap(GET_PORTMAP_FROM_LOGICAL_PORTMAP(pDevCtrl->wanPort,1));
    }
    else
    {
        ethsw_set_wanoe_portmap(GET_PORTMAP_FROM_LOGICAL_PORTMAP(pDevCtrl->wanPort,0));
    }

    /* Update the PBVLAN map in the switch if required */
    bcmenet_update_pbvlan_all_bridge();

    return 0;
}

static int bcm_set_hw_stp(int swPort, int type)
{
    int vport;
    int unit;
    int phyport;
    BcmEnet_devctrl *pDevCtrl = netdev_priv(vnet_dev[0]);
    ASSERT(pDevCtrl != NULL);
                        
    vport   = LOGICAL_PORT_TO_VPORT(swPort);
    unit    = LOGICAL_PORT_TO_UNIT_NUMBER(swPort);
    phyport = LOGICAL_PORT_TO_PHYSICAL_PORT(swPort);
    if (type == TYPE_ENABLE) {
        pDevCtrl->stpDisabledPortMap &= ~(1 << swPort);
        if ( vnet_dev[vport]->flags & IFF_UP) {
            /* set stp state to forward in case bridge STP is off 
               if bridge STP is on, STP state machine will be forced to restart */
            bcmeapi_ethsw_set_stp_mode(unit, phyport, REG_PORT_STP_STATE_FORWARDING);
            dev_change_flags(vnet_dev[vport], (vnet_dev[vport]->flags & ~IFF_UP));
            dev_change_flags(vnet_dev[vport], (vnet_dev[vport]->flags | IFF_UP));
        }
        else {
            bcmeapi_ethsw_set_stp_mode(unit, phyport, REG_PORT_STP_STATE_DISABLED);
        }
    }
    else if (type == TYPE_DISABLE) {
        pDevCtrl->stpDisabledPortMap |= (1 << swPort);
        bcmeapi_ethsw_set_stp_mode(unit, phyport, REG_PORT_NO_SPANNING_TREE);
    }
    else {
        return -EINVAL;
    }
    return 0;
}

/*
 * Set the hardware MAC address.
 */
static int bcm_set_mac_addr(struct net_device *dev, void *p)
{
    struct sockaddr *addr = p;

    if(netif_running(dev))
        return -EBUSY;

    memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);
    return 0;
}

/*
 * bcm63xx_init_dev: initialize Ethernet MACs,
 * allocate Tx/Rx buffer descriptors pool, Tx header pool.
 * Note that freeing memory upon failure is handled by calling
 * bcm63xx_uninit_dev, so no need of explicit freeing.
 */
static int bcm63xx_init_dev(BcmEnet_devctrl *pDevCtrl)
{
    int rc = 0;

    TRACE(("bcm63xxenet: bcm63xx_init_dev\n"));

    bcmenet_in_init_dev = 1;

    /* Handle pkt rate limiting independently in the FAP. No need for global array */

    if ((rc = bcmeapi_init_queue(pDevCtrl)) < 0)
    {
        return rc;
    }

    pDevCtrl->use_default_txq = 0;

#if !defined(CONFIG_BCM947189)
    bcmeapiPhyIntEnable(0);
#endif

    bcmenet_in_init_dev = 0;
    /* if we reach this point, we've init'ed successfully */
    return 0;
}

/* Uninitialize tx/rx buffer descriptor pools */
static int bcm63xx_uninit_dev(BcmEnet_devctrl *pDevCtrl)
{
    if (pDevCtrl) {

#if !defined(CONFIG_BCM947189)
        bcmeapiPhyIntEnable(0);
#endif
        bcmeapi_free_queue(pDevCtrl);
        bcmeapi_free_irq(pDevCtrl);

        /* Deleate the proc files */
        ethsw_del_proc_files();

        /* unregister and free the net device */
        if (pDevCtrl->dev) {
            if (pDevCtrl->dev->reg_state != NETREG_UNINITIALIZED) {
                kerSysReleaseMacAddress(pDevCtrl->dev->dev_addr);
                unregister_netdev(pDevCtrl->dev);
            }
            free_netdev(pDevCtrl->dev);
        }
    }

    return 0;
}

static int str_append(char *buf, char *name, int *cum_len, int bufLen)
{
    int len = strlen(name);

    if ((*cum_len + (*cum_len>0) + len + 1) > bufLen)
        return 0;

    if(*cum_len) {
        strcat(buf, ",");
        (*cum_len)++;
    }

    strcat(buf, name);

    *cum_len += len;

    return len;
}

int enetLogPort2DevName(int portMap, char *buf, int bufLen)
{
    int i, strLen = 0;
    struct net_device *dev;

    buf[0] = 0;
    for (i = 0; i < MAX_TOTAL_SWITCH_PORTS; i++)
    {
        int vport_idx = LOGICAL_PORT_TO_VPORT(i);

        if ((portMap & (1<<i)) == 0 || vport_idx <= 0)
            continue;

        dev = vnet_dev[vport_idx];

        if (dev == NULL)
            continue;

        if (str_append(buf, dev->name, &strLen, bufLen) == 0)
            break;

    }

    return strLen;
}

void bcm_init_port_attr(void)
{
    int unit, port;
    BcmEnet_devctrl *pDevCtrl = netdev_priv(vnet_dev[0]);
    u32 port_flags, 
        port_maps,
        wan_only_def_map,
        lan_only_def_map,
        wan_pref_def_map;
        char nameBuf[256];

    for(unit = 0; unit <= pDevCtrl->unit; unit++)
    {
        port_maps = enet_get_portmap(unit);
        wan_only_def_map = 0;
        lan_only_def_map = 0;
        wan_pref_def_map = 0;

        for(port = 0; port < MAX_SWITCH_PORTS; port++)
        {
            if((port_maps & (1<<port)) == 0 || IsInterSwitchPort(unit, port))
            {
                continue;
            }

            port_flags = enet_get_port_flags(unit, port) & PORT_FLAG_LANWAN_M;

            switch(port_flags)
            {
                case PORT_FLAG_WAN_ONLY:
                    wan_only_def_map |= (1<<port);
                    break;
                case PORT_FLAG_LAN_ONLY:
                    lan_only_def_map |= (1 << port);
                    break;
                case PORT_FLAG_WAN_PREFERRED:
                    wan_pref_def_map |= (1<<port);
                    break;
            }

#if defined(CONFIG_BCM_GMAC)
            /* GMAC port is only on the internal switch  */
            if ( gmac_is_gmac_supported() && !IsExternalSwitchUnit(unit) && gmac_is_gmac_port(port))
            {
                chip_arch_wan_pref_portmap[unit] |= (1<<port);
            }
#endif
        }

        if(chip_arch_wan_only_portmap[unit] & lan_only_def_map)
        {
            printk("ERROR: Conflict LAN Only Port Attributes on Switch %d:\n", unit);
            printk("     Chip WAN Only: %04x, Defined LAN Only Port: 0x%04x\n",
                chip_arch_wan_only_portmap[unit], lan_only_def_map);
            lan_only_def_map &= ~chip_arch_wan_only_portmap[unit];
            printk("     Forced Defined LAN Only Port Attribute to: 0x%04x\n", lan_only_def_map);
        }

        if(chip_arch_lan_only_portmap[unit] & wan_only_def_map)
        {
            printk("ERROR: Conflict WAN Only Port Attribute on Switch %d:\n", unit);
            printk("     Chip LAN Only: %04x, Defined WAN Only Port: 0x%04x\n",
                chip_arch_lan_only_portmap[unit], wan_only_def_map);
            wan_only_def_map &= ~chip_arch_lan_only_portmap[unit];
            printk("     Forced Defined WAN Only Port Attribute to: 0x%04x\n", wan_only_def_map);
        }

        pDevCtrl->wanOnlyPortChip |= PHYSICAL_PORTMAP_TO_LOGICAL_PORTMAP(chip_arch_wan_only_portmap[unit] & port_maps, unit);
        pDevCtrl->wanPrefPortChip |= PHYSICAL_PORTMAP_TO_LOGICAL_PORTMAP(chip_arch_wan_pref_portmap[unit] & port_maps, unit);
        pDevCtrl->lanOnlyPortChip |= PHYSICAL_PORTMAP_TO_LOGICAL_PORTMAP(chip_arch_lan_only_portmap[unit] & port_maps, unit);

        pDevCtrl->wanOnlyPortDef |= PHYSICAL_PORTMAP_TO_LOGICAL_PORTMAP(wan_only_def_map, unit);
        pDevCtrl->wanPrefPortDef |= PHYSICAL_PORTMAP_TO_LOGICAL_PORTMAP(wan_pref_def_map, unit);
        pDevCtrl->lanOnlyPortDef |= PHYSICAL_PORTMAP_TO_LOGICAL_PORTMAP(lan_only_def_map, unit);
    }

    pDevCtrl->wanOnlyPorts = pDevCtrl->wanOnlyPortChip | pDevCtrl->wanOnlyPortDef;
    pDevCtrl->wanPrefPorts = pDevCtrl->wanPrefPortChip | pDevCtrl->wanPrefPortDef;
    pDevCtrl->lanOnlyPorts = pDevCtrl->lanOnlyPortChip | pDevCtrl->lanOnlyPortDef;

    /* Final result conflict checking and correction */
    if(pDevCtrl->wanOnlyPorts & pDevCtrl->lanOnlyPorts)
    {
        printk("ERROR: Conflict WAN Only vs. LAN Only Port list 0x%04x vs 0x%04x\n",
            pDevCtrl->wanOnlyPorts, pDevCtrl->lanOnlyPorts);
        pDevCtrl->lanOnlyPorts &= ~pDevCtrl->wanOnlyPorts;
        printk("       Removed WAN Only ports from LAN Only Port list: 0x%04x\n",
            pDevCtrl->lanOnlyPorts);
    }

    if(pDevCtrl->wanOnlyPorts & pDevCtrl->wanPrefPorts)
    {
        printk("ERROR: Conflict WAN Only vs. WAN Preferred Port list 0x%04x vs 0x%04x\n",
            pDevCtrl->wanOnlyPorts, pDevCtrl->wanPrefPorts);
        pDevCtrl->wanPrefPorts &= ~pDevCtrl->wanOnlyPorts;
        printk("       Removed WAN Only ports from WAN Preferred Port list: 0x%04x\n",
            pDevCtrl->wanPrefPorts);
    }

    if(pDevCtrl->lanOnlyPorts & pDevCtrl->wanPrefPorts)
    {
        printk("ERROR: Conflict WAN Preferred vs. LAN Only Port list 0x%04x vs 0x%04x\n",
            pDevCtrl->wanPrefPorts, pDevCtrl->lanOnlyPorts);
        pDevCtrl->wanPrefPorts &= ~pDevCtrl->lanOnlyPorts;
        printk("       Removed LAN Only ports from WAN Preferred Port list: 0x%04x\n",
            pDevCtrl->wanPrefPorts);
    }

    pDevCtrl->wanLanCapPorts = pDevCtrl->allPortMap & 
        ~(pDevCtrl->wanOnlyPorts | pDevCtrl->lanOnlyPorts | pDevCtrl->wanPrefPorts);

    enetLogPort2DevName(pDevCtrl->allPortMap, nameBuf, sizeof(nameBuf));
    printk("All Port Bit Map: 0x%04x: %s\n", pDevCtrl->allPortMap, nameBuf);
    
    enetLogPort2DevName(pDevCtrl->wanOnlyPorts, nameBuf, sizeof(nameBuf));
    printk("   Chip WAN Only Ports %04x, Defined WAN Only Ports %04x, WAN Only Port Result: 0x%04x:%s\n",
        pDevCtrl->wanOnlyPortChip, pDevCtrl->wanOnlyPortDef, pDevCtrl->wanOnlyPorts, nameBuf);

    enetLogPort2DevName(pDevCtrl->wanPrefPorts, nameBuf, sizeof(nameBuf));
    printk("   Chip WAN Preffered Ports %04x, Defined WAN Preffered Ports %04x, WAN Preffered Port Result: 0x%04x:%s\n",
        pDevCtrl->wanPrefPortChip, pDevCtrl->wanPrefPortDef, pDevCtrl->wanPrefPorts, nameBuf);

    enetLogPort2DevName(pDevCtrl->lanOnlyPorts, nameBuf, sizeof(nameBuf));
    printk("   Chip LAN Only Ports %04x, Defined LAN Only Ports %04x, LAN Only Port Result: 0x%04x:%s\n",
        pDevCtrl->lanOnlyPortChip, pDevCtrl->lanOnlyPortDef, pDevCtrl->lanOnlyPorts, nameBuf);

    enetLogPort2DevName(pDevCtrl->wanLanCapPorts, nameBuf, sizeof(nameBuf));
    printk("   WAN/LAN Both Capable Ports 0x%04x:%s\n", pDevCtrl->wanLanCapPorts, nameBuf);
}
static void bcmenet_check_addl_wan_config(void)
{
}

/*
 *      bcm63xx_enet_probe: - Probe Ethernet switch and allocate device
 */
int __init bcm63xx_enet_probe(void)
{
    static int probed = 0, j;
    struct net_device *dev = NULL;
    BcmEnet_devctrl *pDevCtrl = NULL;
    struct task_struct * bcmsw_task_struct;
    unsigned int chipid;
    unsigned int chiprev;
    unsigned char macAddr[ETH_ALEN];
    ETHERNET_MAC_INFO *EnetInfo;
    int status = 0, unit = 0, ret;
    BcmEnet_devctrl *pVnetDev0;
#if defined(CONFIG_BCM_IEEE1905)
    uint8_t ieee1905_multicast_mac[6] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x13};
#endif

    TRACE(("bcm63xxenet: bcm63xx_enet_probe\n"));

    if (probed)
    {
        /* device has already been initialized */
        return -ENXIO;
    }
    probed++;

    /* zero out global variable */

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
    enet_spdsvc_transmit = bcmFun_get(BCM_FUN_ID_SPDSVC_TRANSMIT);
    BCM_ASSERT(enet_spdsvc_transmit != NULL);
#endif

    bcmeapi_get_chip_idrev(&chipid, &chiprev);
    dev = alloc_etherdev(sizeof(*pDevCtrl));
    if (dev == NULL)
    {
        printk(KERN_ERR CARDNAME": Unable to allocate net_device!\n");
        return -ENOMEM;
    }

    vnet_dev[0] = dev;
    pDevCtrl = netdev_priv(dev);
    memset(pDevCtrl, 0, sizeof(BcmEnet_devctrl));
    EnetInfo = pDevCtrl->EnetInfo;

    spin_lock_init(&pDevCtrl->ethlock_tx);
    spin_lock_init(&pDevCtrl->ethlock_rx);
    spin_lock_init(&bcm_ethlock_phy_access);
    spin_lock_init(&bcm_ethlock_phy_shadow);
    spin_lock_init(&bcm_extsw_access);

    if(BpGetEthernetMacInfo(EnetInfo, BP_MAX_ENET_MACS) != BP_SUCCESS)
    {
        printk(KERN_DEBUG CARDNAME" board id not set\n");
        return -ENODEV;
    }

    if ((EnetInfo[1].ucPhyType == BP_ENET_EXTERNAL_SWITCH) ||
            (EnetInfo[1].ucPhyType == BP_ENET_SWITCH_VIA_INTERNAL_PHY)) {
        unit = 1;
    }

    pDevCtrl->dev = dev;
    pDevCtrl->unit = unit;
    pDevCtrl->chipId  = chipid;
    pDevCtrl->chipRev = chiprev;

    if ((ret = bcmeapi_ethsw_init())) return ret;
    bcmeapi_ethsw_init_ports();

#if !defined(CONFIG_BCM_SWMDK)
    pDevCtrl->enetLinkHandlePmap = enet_get_consolidated_portmap();
#endif
    // Create a port map with only end ports. A port connected to external switch is ignored.
    pDevCtrl->allPortMap = PHYSICAL_PORTMAP_TO_LOGICAL_PORTMAP(EnetInfo[0].sw.port_map, 0); /* Internal Switch portmap */
    if (unit == 1)
    {
        pDevCtrl->allPortMap |= PHYSICAL_PORTMAP_TO_LOGICAL_PORTMAP(EnetInfo[1].sw.port_map, 1); /* External Switch portmap */
        extSwInfo.connected_to_internalPort = BpGetPortConnectedToExtSwitch();
        pDevCtrl->allPortMap &= ~SET_PORT_IN_LOGICAL_PORTMAP(extSwInfo.connected_to_internalPort, 0);
    }

    /* Remove ports connected to FTTdp DSP from unit 0 of pDevCtrl->allPortMap */
    for (j = 0; j < BP_MAX_SWITCH_PORTS; j++)
    {
        if (EnetInfo[0].sw.port_flags[j] & PORT_FLAG_ATTACHED)
        {
        	pDevCtrl->allPortMap &= ~SET_PORT_IN_LOGICAL_PORTMAP(j, 0);
        }
    }

    pVnetDev0_g = global.pVnetDev0_g = pDevCtrl;
    global.pVnetDev0_g->extSwitch = &extSwInfo;

#if defined(CONFIG_BCM_PKTDMA_TX_SPLITTING)
    global.pVnetDev0_g->enetTxChannel = PKTDMA_ETH_TX_HOST_IUDMA;   /* default for enet tx on HOST */
#endif

    if (unit == 1) {
        int bus_type, spi_id;
        /* get external switch access details */
        get_ext_switch_access_info(EnetInfo[1].usConfigType, &bus_type, &spi_id);
        extSwInfo.accessType = bus_type;
        extSwInfo.bus_num = (bus_type == MBUS_SPI)?LEG_SPI_BUS_NUM:HS_SPI_BUS_NUM;
        extSwInfo.spi_ss = spi_id;
        extSwInfo.spi_cid = 0;
        extSwInfo.brcm_tag_type = BRCM_TYPE2;
        extSwInfo.present = 1;
        status = bcmeapi_init_ext_sw_if(&extSwInfo);
#if defined(CONFIG_BCM_PORTS_ON_INT_EXT_SW)
        pDevCtrl->wanPort |= 1 << (PHYSICAL_PORT_TO_LOGICAL_PORT(extSwInfo.connected_to_internalPort, 0));
#endif
    }

#if defined(CONFIG_BCM947189)
    gmac_init();
    dma_set_coherent_mask(&dev->dev, DMA_BIT_MASK(32));
#endif

    /* Disable all port MAC for safe init, in case BSP did not do it */
    bcmsw_enable_all_macs_rxtx(0);

    {
        char buf[BRCM_MAX_CHIP_NAME_LEN];
        printk("Broadcom BCM%s Ethernet Network Device ", kerSysGetChipName(buf, BRCM_MAX_CHIP_NAME_LEN));
        printk(VER_STR);
        printk("\n");
    }

#if defined(CONFIG_BCM963268)
    // Now select ROBO at Phy3
    BCM_ENET_DEBUG( "Select ROBO at Mux (bit18=0x40000)" );
    GPIO->RoboswGphyCtrl |= GPHY_MUX_SEL_GMAC;
    GPIO->RoboswGphyCtrl &= ~GPHY_MUX_SEL_GMAC;

    BCM_ENET_DEBUG( "\tGPIORoboswGphyCtrl<0x%p>=0x%x",
            &GPIO->RoboswGphyCtrl, (uint32_t) GPIO->RoboswGphyCtrl );
#endif

#if defined(CONFIG_BCM_GMAC)
    gmac_init();
#endif

    if ((status = bcm63xx_init_dev(pDevCtrl)))
    {
        printk((KERN_ERR CARDNAME ": device initialization error!\n"));
        bcm63xx_uninit_dev(pDevCtrl);
        return -ENXIO;
    }

    dev_alloc_name(dev, dev->name);
    sprintf(dev->name, ETHERNET_ROOT_DEVICE_NAME);

    dev->base_addr = -1;    /* Set the default invalid address to identify bcmsw device */
    bcmeapi_add_proc_files(dev, pDevCtrl);

    ethsw_add_proc_files(dev);

    dev->netdev_ops = &bcm96xx_netdev_ops;
#if defined(CONFIG_BCM_ETHTOOL)
    dev->ethtool_ops = &bcm63xx_enet_ethtool_ops;
    bcmenet_private_ethtool_ops = &enet_ethtool_ops;
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 1)
    /*
     * In Linux 3.4, we do not use softirq or NAPI.  We create a thread to
     * do the rx processing work.
     */
    pDevCtrl->rx_work_avail = 0;
    init_waitqueue_head(&pDevCtrl->rx_thread_wqh);
    pDevCtrl->rx_thread = kthread_create(bcm63xx_enet_rx_thread, pDevCtrl, "bcmsw_rx");
    wake_up_process(pDevCtrl->rx_thread);
#else
    netif_napi_add(dev, &pDevCtrl->napi, bcm63xx_enet_poll_napi, NETDEV_WEIGHT);
#endif

    netdev_path_set_hw_port(dev, 0, BLOG_ENETPHY);

    dev->watchdog_timeo     = 2 * HZ;
    /* setting this flag will cause the Linux bridge code to not forward
       broadcast packets back to other hardware ports */
    dev->priv_flags         = IFF_HW_SWITCH;
    dev->mtu = BCM_ENET_DEFAULT_MTU_SIZE; /* bcmsw dev : Explicitly assign the MTU size based on buffer size allocated */

#if defined(_CONFIG_BCM_FAP) && defined(CONFIG_BCM_FAP_GSO)
    dev->features           = NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_TSO | NETIF_F_TSO6 | NETIF_F_UFO;
    dev->vlan_features      = NETIF_F_SG | NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_TSO | NETIF_F_TSO6 | NETIF_F_UFO;
#endif

#if defined(CONFIG_BCM_KF_EXTSTATS)
    /* Indicate we're supporting extended statistics */
    dev->features           |= NETIF_F_EXTSTATS;
#endif    

    bitcount(vport_cnt, pDevCtrl->allPortMap);
    ethsw_init_table(pDevCtrl);
    ethsw_reset_ports(dev);

    status = register_netdev(dev);

    if (status != 0)
    {
        bcm63xx_uninit_dev(pDevCtrl);
        printk(KERN_ERR CARDNAME "bcm63xx_enet_probe failed, returns %d\n", status);
        return status;
    }

#ifdef DYING_GASP_API
    /* Set up dying gasp handler */
    kerSysRegisterDyingGaspHandler(pDevCtrl->dev->name, &ethsw_switch_power_off, dev);
#endif

    macAddr[0] = 0xff;
    kerSysGetMacAddress(macAddr, dev->ifindex);

    if((macAddr[0] & ETH_MULTICAST_BIT) == ETH_MULTICAST_BIT)
    {
        memcpy(macAddr, "\x00\x10\x18\x63\x00\x00", ETH_ALEN);
        printk((KERN_CRIT "%s: MAC address has not been initialized in NVRAM.\n"), dev->name);
    }

    memmove(dev->dev_addr, macAddr, ETH_ALEN);

    /* Check additional WAN - MUST be done before we start creating Linux interfaces */
    bcmenet_check_addl_wan_config();

    status = create_vport();

    if (status != 0)
        return status;


    pVnetDev0 = (BcmEnet_devctrl *) netdev_priv(vnet_dev[0]);

    ethsw_init_config(pDevCtrl->unit, EnetInfo[0].sw.port_map,  pDevCtrl->wanPort);
    ethsw_phy_config();

    /* Ethernet Switch init is complete - flush the dynamic ARL entries to start clean */
    fast_age_all(0);
    if (bcm63xx_enet_isExtSwPresent()) {
        fast_age_all_ext(0);
    }

    enet_arl_write((uint8_t*)dev->dev_addr, 0, 
        ARL_DATA_ENTRY_VALID|ARL_DATA_ENTRY_STATIC|IMP_PORT_ID);

#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
    /* Static MAC works for all scenarios, so just add multiport
     * MAC only when multiple IMP ports are in use. */
    bcmsw_set_multiport_address_ext((uint8_t*)dev->dev_addr);
#else
    if (extSwInfo.present == 1)
    {
        enet_arl_write_ext((uint8_t*)dev->dev_addr, 0, 
                ARL_DATA_ENTRY_VALID|ARL_DATA_ENTRY_STATIC|IMP_PORT_ID);
    }
#endif

#if defined(CONFIG_BCM_IEEE1905)
    bcmeapi_set_multiport_address(ieee1905_multicast_mac);
    bcmsw_set_multiport_address_ext(ieee1905_multicast_mac);
#endif

    if( (ret = bcmeapi_init_dev(dev)) < 0)
        return ret;

    bcm_init_port_attr();

#if !defined(CONFIG_BCM_SWMDK)
    ethsw_eee_init();
#endif
    phy_int_mapped = bcmeapi_map_interrupt(pDevCtrl);
    if((bcmenet_use_ext_phy && !(phy_int_mapped & BCMEAPI_INT_MAPPED_EXTPHY)) ||
        !(phy_int_mapped & BCMEAPI_INT_MAPPED_INTPHY))
    {
        force_link_check = ETHSW_LINK_FORCE_CHECK;
    }

    bcmsw_task_struct = kthread_run(bcm63xx_enet_poll_timer, NULL, "bcmsw");
    poll_pid = bcmsw_task_struct->pid;
#if defined(CONFIG_BCM947189)
    gmac_set_active();
#endif

    set_bit(__LINK_STATE_START, &dev->state);
    dev->netdev_ops->ndo_open(dev);
    dev->flags |= IFF_UP;

#if defined(CONFIG_BCM_ETH_DEEP_GREEN_MODE)
    ethsw_deep_green_mode_handler(pVnetDev0->linkState);
#endif


    return ((poll_pid < 0)? -ENOMEM: 0);
}

int bcm63xx_enet_isExtSwPresent(void)
{
    return extSwInfo.present;
}

int bcm63xx_enet_intSwPortToExtSw(void)
{
    return extSwInfo.connected_to_internalPort;
}

unsigned int bcm63xx_enet_extSwId(void)
{
    return extSwInfo.switch_id;
}

static int bridge_notifier(struct notifier_block *nb, unsigned long event, void *info);
static void bridge_update_pbvlan(char *brName);
struct notifier_block br_notifier = {
    .notifier_call = bridge_notifier,
};

static int bridge_stp_handler(struct notifier_block *nb, unsigned long event, void *portInfo);
static struct notifier_block br_stp_handler = {
    .notifier_call = bridge_stp_handler,
};

static void __exit bcmenet_module_cleanup(void)
{
    BcmEnet_devctrl *pDevCtrl;
    TRACE(("bcm63xxenet: bcmenet_module_cleanup\n"));

    /*first disable the poll timer*/
    if (poll_pid >= 0)
    {
      atomic_dec(&poll_lock);
      wait_for_completion(&poll_done);
    }

    bcmeapi_enet_module_cleanup();

    delete_vport();


    pDevCtrl = (BcmEnet_devctrl *)netdev_priv(vnet_dev[0]);

    if (pDevCtrl)
    {
#ifdef DYING_GASP_API
        if(pDevCtrl->EnetInfo[0].ucPhyType == BP_ENET_EXTERNAL_SWITCH)
            kerSysDeregisterDyingGaspHandler(pDevCtrl->dev->name);
#endif
        bcm63xx_uninit_dev(pDevCtrl);
    }

    bcmFun_dereg(BCM_FUN_ID_ENET_LINK_CHG);
    bcmFun_dereg(BCM_FUN_ID_RESET_SWITCH);
    bcmFun_dereg(BCM_FUN_ID_ENET_CHECK_SWITCH_LOCKUP);
    bcmFun_dereg(BCM_FUN_ID_ENET_GET_PORT_BUF_USAGE);
    bcmFun_dereg(BCM_FUN_IN_ENET_CLEAR_ARL_ENTRY);
    bcmFun_dereg(BCM_FUN_ID_ENET_IS_SWSWITCH_PORT);

#if defined(CONFIG_BCM_KF_NETFILTER)
    /* Unregister bridge notifier hooks */
    unregister_bridge_notifier(&br_notifier);
    unregister_bridge_stp_notifier(&br_stp_handler);
#endif

}

static int bcmeapi_ioctl_use_default_txq_config(BcmEnet_devctrl *pDevCtrl,
        struct ethswctl_data *e)
{
    if (e->type == TYPE_GET) {
        e->ret_val= pDevCtrl->use_default_txq;
        BCM_ENET_DEBUG("e->ret_val: 0x%02x \n ", e->ret_val);
    } else {
        BCM_ENET_DEBUG("Given use_default_txq: 0x%02x \n ", e->val);
        pDevCtrl->use_default_txq = e->val;
    }

    return 0;
}

static int bcmeapi_ioctl_default_txq_config(BcmEnet_devctrl *pDevCtrl,
        struct ethswctl_data *e)
{
    if (e->type == TYPE_GET) {
        e->queue = pDevCtrl->default_txq;
        BCM_ENET_DEBUG("e->queue: 0x%02x \n ", e->queue);
    } else {
        BCM_ENET_DEBUG("Given queue: 0x%02x \n ", e->queue);
        if ((e->queue >= NUM_EGRESS_QUEUES) || (e->queue < 0)) {
            printk("Invalid queue \n");
            return BCM_E_ERROR;
        }
        pDevCtrl->default_txq = e->queue;
    }

    return 0;
}



#ifdef BCM_ENET_DEBUG_BUILD
static int bcmeapi_ioctl_getrxcounters(void)
{
    int a = 0, b = 0, c = 0, d = 0, f = 0, cnt = 0;

    for (cnt = 0; cnt < ENET_RX_CHANNELS_MAX; cnt++)
    {
        printk("Rx counters: %d No Rx Pkts counters: %d No Rx BDs counters: %d\n", ch_pkts[cnt],
                ch_no_pkts[cnt], ch_no_bds[cnt]);
    }

    printk("Channels: ");
    for (cnt = 0; cnt < NUM_ELEMS; cnt++) {
        if (ch_serviced[cnt] == WRR_RELOAD) {
            printk("\nCh0 = %d, Ch1 = %d, Ch2 = %d, Ch3 = %d \n", a,b,c,d);
            a = b = c =d = 0;
            printk("\nReloaded WRR weights \n");
        } else if (ch_serviced[cnt] == ISR_START) {
            printk("ISR START (Weights followed by channels serviced) \n");
            printk("x- indicates pkt received \n");
        } else {
            if (ch_serviced[cnt] & (1<<31)) {
                printk("x-");
                f = ch_serviced[cnt] & 0xF;
                if (f == 0) {
                    a++;
                } else if (f == 1) {
                    b++;
                } else if (f == 2) {
                    c++;
                } else if (f == 3) {
                    d++;
                }
            }
            printk("%d ", ch_serviced[cnt] & (~(1<<31)));
        }
    }
    printk("\n");
    return 0;
}

static int bcmeapi_ioctl_setrxcounters(void)
{
    int cnt = 0;

    for (cnt = 0; cnt < ENET_RX_CHANNELS_MAX; cnt++)
    {
        ch_pkts[cnt] = ch_no_pkts[cnt] = ch_no_bds[cnt] = 0;
    }
    for (cnt=0; cnt<4000; cnt++) {
        ch_serviced[cnt] = 0;
    }
    dbg_index = 0;

    return 0;
}
#endif


#define ESTATS_FMT "  %-40s: %10lu;   %-40s: %10lu"
#define ESTATS_VAL(v1, v2) #v1, estats->v1, #v2, estats->v2
#define ESTATS_FMT1 "  %-40s: %10lu"
#define ESTATS_FMT1b "  %-40s: %10s"
#define ESTATS_VAL1b(v1) #v1, v1? "True": "False"
#define ESTATS_VAL1(v1) #v1, v1
#define ESTATS_VAL2(v1, v2) #v1, v1, #v2, v2
typedef struct counterSum
{
    unsigned long rxIn, rx2Kernel, rx2Blog, rxDrops,
                  txIn, txOut, txDrops, txExtraChain;
} counterSum_t;

static void display_enet_dev_stats(BcmEnet_devctrl *pDevCtl, counterSum_t *cts)
{
    struct net_device *dev = pDevCtl->dev;
    enet_device_stats *estats = &pDevCtl->estats;
    counterSum_t portCnt;
    int i;

    memset(&portCnt, 0, sizeof(portCnt));

    printk("Device %s:\n", dev->name);
    printk(ESTATS_FMT "\n", ESTATS_VAL(rx_packets_queue[0], rx_packets_queue[1]));
    printk(ESTATS_FMT "\n", ESTATS_VAL(rx_packets_queue[2], rx_packets_queue[3]));
    printk(ESTATS_FMT "\n", ESTATS_VAL(rx_packets_queue[4], rx_packets_queue[5]));
    printk(ESTATS_FMT "\n", ESTATS_VAL(rx_packets_queue[6], rx_packets_queue[7]));
    printk(ESTATS_FMT "\n", ESTATS_VAL(tx_packets_queue_in[0], tx_packets_queue_in[1]));
    printk(ESTATS_FMT "\n", ESTATS_VAL(tx_packets_queue_in[2], tx_packets_queue_in[3]));
    printk(ESTATS_FMT "\n", ESTATS_VAL(tx_packets_queue_in[4], tx_packets_queue_in[5]));
    printk(ESTATS_FMT "\n", ESTATS_VAL(tx_packets_queue_in[6], tx_packets_queue_in[7]));
    printk(ESTATS_FMT "\n", ESTATS_VAL(tx_packets_queue_out[0], tx_packets_queue_out[1]));
    printk(ESTATS_FMT "\n", ESTATS_VAL(tx_packets_queue_out[2], tx_packets_queue_out[3]));
    printk(ESTATS_FMT "\n", ESTATS_VAL(tx_packets_queue_out[4], tx_packets_queue_out[5]));
    printk(ESTATS_FMT "\n", ESTATS_VAL(tx_packets_queue_out[6], tx_packets_queue_out[7]));
    printk(ESTATS_FMT "\n", ESTATS_VAL(rx_dropped_no_rxdev, rx_dropped_blog_drop));
    printk(ESTATS_FMT "\n", ESTATS_VAL(rx_dropped_no_skb, rx_packets_blog_done));
    printk(ESTATS_FMT "\n", ESTATS_VAL(rx_dropped_skb_headinit, rx_packets_netif_receive_skb));
    printk(ESTATS_FMT "\n", ESTATS_VAL(rx_errors_indicated_by_low_level, rx_dropped_undersize));
    printk(ESTATS_FMT "\n", ESTATS_VAL(rx_dropped_overrate, tx_dropped_bad_nbuff ));
    printk(ESTATS_FMT "\n", ESTATS_VAL(tx_dropped_no_lowlvl_resource, tx_dropped_no_fkb));
    printk(ESTATS_FMT "\n", ESTATS_VAL(tx_dropped_no_skb, tx_dropped_no_gem_ids));
    printk(ESTATS_FMT "\n", ESTATS_VAL(tx_dropped_bad_gem_id, tx_dropped_misaligned_nbuff));
    printk(ESTATS_FMT "\n", ESTATS_VAL(tx_drops_no_valid_gem_fun, tx_drops_skb_linearize_error));
    printk(ESTATS_FMT "\n", ESTATS_VAL(tx_dropped_runner_lan_fail, tx_dropped_runner_wan_fail));
    printk(ESTATS_FMT "\n", ESTATS_VAL(tx_dropped_no_gso_dsc, tx_dropped_sid_tx_fail));
    printk(ESTATS_FMT "\n", ESTATS_VAL(tx_dropped_no_rdpa_port_mapped, tx_dropped_no_gem_tcount));
    printk(ESTATS_FMT "\n", ESTATS_VAL(tx_dropped_no_epon_tx_fun, tx_dropped_no_epon_oam_fun));
    printk(ESTATS_FMT "\n", ESTATS_VAL(tx_dropped_gpon_tx_fail, tx_dropped_epon_tx_fail));
    printk(ESTATS_FMT "\n", ESTATS_VAL(tx_dropped_epon_oam_fail, tx_dropped_xpon_lan_fail));

    for(i = 0; i<8; i++)
    {
        portCnt.rxIn += estats->rx_packets_queue[i]; 
        portCnt.txIn += estats->tx_packets_queue_in[i]; 
        portCnt.txOut += estats->tx_packets_queue_out[i]; 
    }

    portCnt.rx2Kernel = estats->rx_packets_netif_receive_skb;
    portCnt.rx2Blog = estats->rx_packets_blog_done;
    portCnt.rxDrops =   estats->rx_dropped_no_rxdev +
        estats->rx_dropped_blog_drop +
        estats->rx_dropped_no_skb +
        estats->rx_dropped_skb_headinit +
        estats->rx_errors_indicated_by_low_level +
        estats->rx_dropped_undersize +
        estats->rx_dropped_overrate;

    portCnt.txDrops = estats->tx_dropped_bad_nbuff +
        estats->tx_dropped_no_lowlvl_resource +
        estats->tx_dropped_no_fkb +
        estats->tx_dropped_no_skb +
        estats->tx_dropped_no_gem_ids +
        estats->tx_dropped_bad_gem_id +
        estats->tx_dropped_misaligned_nbuff +
        estats->tx_drops_no_valid_gem_fun +
        estats->tx_drops_skb_linearize_error +
        estats->tx_dropped_runner_lan_fail +
        estats->tx_dropped_runner_wan_fail +
        estats->tx_dropped_no_gso_dsc +
        estats->tx_dropped_sid_tx_fail +
        estats->tx_dropped_no_rdpa_port_mapped +
        estats->tx_dropped_no_gem_tcount +
        estats->tx_dropped_no_epon_tx_fun +
        estats->tx_dropped_no_epon_oam_fun +
        estats->tx_dropped_gpon_tx_fail +
        estats->tx_dropped_epon_tx_fail +
        estats->tx_dropped_epon_oam_fail +
        estats->tx_dropped_xpon_lan_fail;

    printk(ESTATS_FMT "\n", ESTATS_VAL2(portCnt.rxIn, portCnt.rx2Kernel));
    printk(ESTATS_FMT "\n", ESTATS_VAL2(portCnt.rx2Blog, portCnt.rx2Kernel + portCnt.rx2Blog));
    printk(ESTATS_FMT1 "\n", ESTATS_VAL1(portCnt.rxDrops));
    printk(ESTATS_FMT1 "\n", ESTATS_VAL1((portCnt.rx2Kernel + portCnt.rx2Blog + portCnt.rxDrops)));
    printk(ESTATS_FMT1b "\n", ESTATS_VAL1b(portCnt.rxIn == (portCnt.rx2Kernel + portCnt.rx2Blog + portCnt.rxDrops)));
    printk(ESTATS_FMT "\n", ESTATS_VAL2(portCnt.txIn, portCnt.txOut));
    printk(ESTATS_FMT "\n", ESTATS_VAL2(portCnt.txDrops, portCnt.txOut + portCnt.txDrops));
    printk(ESTATS_FMT1b "\n", ESTATS_VAL1b(portCnt.txIn == (portCnt.txOut + portCnt.txDrops)));
    printk("\n");

    if(cts)
    {
        cts->rxIn += portCnt.rxIn;
        cts->rx2Kernel += portCnt.rx2Kernel;
        cts->rx2Blog += portCnt.rx2Blog;
        cts->rxDrops += portCnt.rxDrops;
        cts->txIn += portCnt.txIn;
        cts->txOut += portCnt.txOut;
        cts->txDrops += portCnt.txDrops;
    }
    return;
}

static void display_enet_stats(BcmEnet_devctrl *_pDevCtl)
{
    struct net_device *dev = _pDevCtl->dev;
    BcmEnet_devctrl * pDevCtl;
    counterSum_t total;
    int vport;

    memset(&total, 0, sizeof(total));

    if(strcmp(dev->name, ETHERNET_ROOT_DEVICE_NAME) != 0)
        return display_enet_dev_stats(_pDevCtl, NULL);

    for (vport = 0; vport <= vport_cnt; vport++) 
    {
        dev = vnet_dev[vport];
        if (dev == NULL)
            continue;

        pDevCtl = (BcmEnet_devctrl *)netdev_priv(dev);
        display_enet_dev_stats(pDevCtl, &total);
    }

    printk("\n");
    printk(ESTATS_FMT "\n", ESTATS_VAL2(total.rxIn, total.rx2Kernel));
    printk(ESTATS_FMT "\n", ESTATS_VAL2(total.rx2Blog, total.rx2Kernel + total.rx2Blog));
    printk(ESTATS_FMT "\n", ESTATS_VAL2(total.rxDrops, (total.rx2Kernel + total.rx2Blog + total.rxDrops)));
    printk(ESTATS_FMT1b "\n", ESTATS_VAL1b(total.rxIn == (total.rx2Kernel + total.rx2Blog + total.rxDrops)));
    printk(ESTATS_FMT "\n", ESTATS_VAL2(total.txIn, total.txOut));
    printk(ESTATS_FMT "\n", ESTATS_VAL2(total.txDrops, total.txOut + total.txDrops));
    printk(ESTATS_FMT1b "\n", ESTATS_VAL1b(total.txIn == (total.txOut + total.txDrops)));
}

static void display_software_stats(BcmEnet_devctrl * pDevCtrl)
{

    printk("\n");
    printk("TxPkts:       %10llu \n", pDevCtrl->stats.tx_packets);
    printk("TxOctets:     %10llu \n", pDevCtrl->stats.tx_bytes);
    printk("TxDropPkts:   %10llu \n", pDevCtrl->stats.tx_dropped);
    printk("\n");
    printk("RxPkts:       %10llu \n", pDevCtrl->stats.rx_packets);
    printk("RxOctets:     %10llu \n", pDevCtrl->stats.rx_bytes);
    printk("RxDropPkts:   %10llu \n", pDevCtrl->stats.rx_dropped);
    printk("\n");

    display_enet_stats(pDevCtrl);
}

#define BIT_15 0x8000
#define MAX_NUM_WAN_IFACES 8
#define MAX_WAN_IFNAMES_LEN ((MAX_NUM_WAN_IFACES * (IFNAMSIZ + 1)) + 2)

#if defined(_CONFIG_BCM_FAP) && \
    (defined(CONFIG_BCM963268))
static void enet_conf_cos_for_wan(int swPort, int *data)
{
    struct ethswctl_data e2;
    int i, j;

    if(data)
    {
        if ( !IsExternalSwitchPort(swPort))
        {
            /* The equivalent of "ethswctl -c cosq -p {i} -q {j} -v 1" */
            /* where i = all eth ports (0..5) except the WAN port (swPort) */
            /* This routes packets of all priorities on the WAN eth port to egress queue 0 */
            /* This routes packets of all priorities on all other eth ports to egress queue 1 */
            for(i = 0; i < BP_MAX_SWITCH_PORTS; i++)
            {
                for(j = 0; j <= MAX_PRIORITY_VALUE; j++)
                {
                    e2.type = TYPE_SET;
                    e2.port = i;
                    e2.priority = j;

                    if ((LOGICAL_PORT_TO_PHYSICAL_PORT(swPort) == i)
                       )
                    {
                        e2.queue = PKTDMA_ETH_DS_IUDMA;  /* WAN port mapped to DS FAP */
                    }
                    else
                    {
                        e2.queue = PKTDMA_ETH_US_IUDMA;  /* other ports to US FAP */
                    }

                    bcmeapi_ioctl_ethsw_cosq_port_mapping(&e2);
                }
            }
        }
    }
    else
    {
        /* Return all ethernet ports to be processed on the FAP - Nov 2010 (Jira 7811) */
        /* The equivalent of "ethswctl -c cosq -p {i} -q {j} -v 0" */
        /* where i = all eth ports (0..5) including the WAN port (swPort) */
        for(i = 0; i < BP_MAX_SWITCH_PORTS ; i++)
        {
            for(j = 0; j <= MAX_PRIORITY_VALUE; j++)
            {
                e2.type = TYPE_SET;
                e2.port = i;
                e2.priority = j;
                /* All ports mapped to default iuDMA - Mar 2011 */
                e2.queue = PKTDMA_DEFAULT_IUDMA;
                bcmeapi_ioctl_ethsw_cosq_port_mapping(&e2);
            }
        }
    }
}
#else
static void enet_conf_cos_for_wan(int swPort, int *data)
{
}
#endif

int enet_ioctl_kernel_poll(struct ethswctl_data *e)
{
    static int initPorts = 0;

    if (!initPorts)
    {
        initPorts = 1;
        bcmsw_enable_all_macs_rxtx(1);
    }
    bcmeapi_ioctl_kernel_poll(e);
    e->mdk_kernel_poll.link_change = bcmenet_link_might_changed();
    
    return 0;
}

static int bcm63xx_enet_getUnitPortFromOamIdx(int oamIdx, int *pUnit, int *pPort)
{
    int logical_port;
    /* TBD : This is unnecessary loop; Better if we can restrict the OAM index to
     * 0..MAX_TOTAL_SWITCH_PORTS-1 */
    if (g_oamIdx_map_found)
    {
        for (logical_port=0; logical_port < MAX_TOTAL_SWITCH_PORTS; logical_port++)
        {
            if (logicalport_to_oamIdx[logical_port] == oamIdx)
            {
                *pUnit = LOGICAL_PORT_TO_UNIT_NUMBER(logical_port);
                *pPort = LOGICAL_PORT_TO_PHYSICAL_PORT(logical_port);
                return 0;
            }
        }
        return -1;
    }
    else
    {
        /* No mapping available - Return the same with unit=0 */
        *pUnit = 0;
        *pPort = oamIdx;
    }
    return 0;
}

static int bcm63xx_enet_getOamIdxFromUnitPort(int unit, int port, int *oamIdx)
{
    int logical_port;

    if (g_oamIdx_map_found)
    {
        logical_port = PHYSICAL_PORT_TO_LOGICAL_PORT(port, unit);
        if(logical_port < MAX_TOTAL_SWITCH_PORTS)
            *oamIdx = logicalport_to_oamIdx[logical_port];
        else
            return -1;
    }
    else
    {
        /* No mapping available - Return the same with unit=0 */
        *oamIdx = port;
    }
    return 0;
}

#define SET_IFR_RET(e, m) {ifrd_off = &e->m, ifrd_size = sizeof(e->m);}
#define SET_IFR_RET_GET(e, m) {if (e->type == TYPE_GET) SET_IFR_RET(e, m);}
#define SET_IFR_RET_LEN(e, d, l) {ifrd_off = &e->d, ifrd_size = e->l;}
#define SET_2IFR_RET(e,m,n) {ifrd_off = &e->m; ifrd_size = sizeof(e->m); ifrd_off2 = &e->n; ifrd_size2 = sizeof(e->n);}
#define SET_2IFR_RET_GET(e,m,n) {if(e->type == TYPE_GET) SET_2IFR_RET(e,m,n);}

#define SET_ESW_RET2(m) SET_IFR_RET(e, m)
#define SET_ESW_RET(m) SET_IFR_RET_GET(e, m)
#define SET_ESW_RET3(d, l) SET_IFR_RET_LEN(e, d, l)
#define SET_2ESW_RET(m,n) SET_2IFR_RET_GET(e,m,n)
#define SET_2ESW_RET2(m,n) SET_2IFR_RET(e,m,n)

#define SET_ECT_RET2(m) SET_IFR_RET(ethctl,m)
static int enet_dev_get_phy_dev(struct net_device *dev, struct ethctl_data *ethctl)
{
    int phy_id = 0;
    BcmEnet_devctrl *pDevCtrl, *pEthCtrl;
    int unit, port;
    int sub_port;

    pDevCtrl = netdev_priv(vnet_dev[0]);
    pEthCtrl = (BcmEnet_devctrl *)netdev_priv(dev);

    unit = LOGICAL_PORT_TO_UNIT_NUMBER(pEthCtrl->sw_port_id);
    port = LOGICAL_PORT_TO_PHYSICAL_PORT(pEthCtrl->sw_port_id);

    if (ethctl->sub_port == -1) 
        phy_id = pDevCtrl->EnetInfo[unit].sw.phy_id[port];
    else
    {
        sub_port = BP_PHY_PORT_TO_CROSSBAR_PORT(ethctl->sub_port);
        phy_id = pDevCtrl->EnetInfo[unit].sw.crossbar[sub_port].phy_id;
    }
    return phy_id;
}

static int bcm63xx_enet_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
    BcmEnet_devctrl *pDevCtrl, *pEthCtrl;
    char *buf, *ubuf;
    int *data, cb_port;
    ifreq_ext_t *ifx;
    struct ethswctl_data *e;
    struct ethctl_data *ethctl;
    struct interface_data *enetif_data;
    struct mii_ioctl_data *mii;
    struct rtnl_link_stats64 *vstats;
    struct sockaddr sockaddr;
    int val = 0, mask = 0, bufLen = 0, cum_len = 0, portMap = 0;
    int vport, phy_id, phy_id_ext, swPort, phyId, unit, port, logical_port, ifrd_size = 0, ifrd_size2 = 0;
    union {
        struct ethswctl_data ethswctl_data;
        struct ethctl_data ethctl_data;
        struct interface_data interface_data;
        ifreq_ext_t ifre;
    } rq_data;
    void *ifrd_off = &rq_data, *ifrd_off2 = &rq_data;


    pDevCtrl = netdev_priv(vnet_dev[0]);
    ASSERT(pDevCtrl != NULL);

    /* pointers pointing to if_req storage */
    mii = (struct mii_ioctl_data *)&rq->ifr_data;

    /* pointers pointing to ifr_data */
    data = (int*)&rq_data;
    e = (struct ethswctl_data*)&rq_data;
    ethctl = (struct ethctl_data*)&rq_data;
    enetif_data = (struct interface_data*)&rq_data;
    ifx = (ifreq_ext_t *)&rq_data;

#if defined(CONFIG_BCM_ETH_PWRSAVE)
    ethsw_phy_pll_up(0);
#endif

    switch (cmd)
    {
        case SIOCGMIIPHY:       /* Get address of MII PHY in use. */
            swPort = port_id_from_dev(dev);
            if (swPort >= MAX_TOTAL_SWITCH_PORTS )
            {
                printk("SIOCGMIIPHY : Invalid swPort: %d \n", swPort);
                val = -EINVAL;
                break;
            }

            phy_id = enet_logport_to_phyid(swPort);
            mii->phy_id =  (u16)phy_id;
            if(!IsPhyConnected(phy_id)) return -EINVAL;
            BCM_ENET_DEBUG("%s: swPort/logport %d phy_id: 0x%x flag 0x%x \n", __FUNCTION__,
                    swPort, mii->phy_id, mii->val_out);
            break;

        case SIOCGMIIREG:       /* Read MII PHY register. */
            down(&bcm_ethlock_switch_config);
            ethsw_phyport_rreg2(mii->phy_id, mii->reg_num & 0x1f,
                    (uint16 *)&mii->val_out, mii->val_out);
            up(&bcm_ethlock_switch_config);
            break;

        case SIOCGSWITCHPORT:       /* Get Switch Port. */
            if (copy_from_user(enetif_data, rq->ifr_data, sizeof(*enetif_data))) goto ErrorEfault;
            if (bcm63xx_enet_getPortFromName(enetif_data->ifname, &unit, &port)) goto ErrorEfault;
            enetif_data->switch_port_id = PHYSICAL_PORT_TO_LOGICAL_PORT(port, unit);
            ifrd_off = &enetif_data->switch_port_id;
            ifrd_size = sizeof(enetif_data->switch_port_id);
            break;

        case SIOCSMIIREG:       /* Write MII PHY register. */
            down(&bcm_ethlock_switch_config);
            BCM_ENET_DEBUG("phy_id: %d; reg_num = %d; val = 0x%x \n", mii->phy_id,
                    mii->reg_num, mii->val_in);
            ethsw_phyport_wreg2(mii->phy_id, mii->reg_num & 0x1f, (uint16 *)&mii->val_in, mii->val_out);
            up(&bcm_ethlock_switch_config);
            break;

        case SIOCGLINKSTATE:
            if (dev == vnet_dev[0])
            {
                mask = 0xffffffff;
            }
            else
            {
                swPort = port_id_from_dev(dev);
                if (swPort >= MAX_TOTAL_SWITCH_PORTS )
                {
                    printk("SIOCGLINKSTATE : Invalid swPort: %d \n", swPort);
                    return -EINVAL;
                }
                mask = 0x00000001 << swPort;
            }
            *data = (pDevCtrl->linkState & mask)? 1: 0;
            ifrd_size = sizeof(*data);
            break;

        case SIOCSCLEARMIBCNTR:
            ASSERT(pDevCtrl != NULL);

            /* NOTE : pDevCtrl is pointing to bcmsw device */
            memset(&pDevCtrl->stats, 0, sizeof(pDevCtrl->stats));
            memset(&pDevCtrl->cHwStats, 0, sizeof(pDevCtrl->cHwStats));
            memset(&pDevCtrl->lHwStats, 0, sizeof(pDevCtrl->lHwStats));
            /* port 0 is bcmsw */
            for (vport = 1; vport <= vport_cnt; vport++)
            {
                BcmEnet_devctrl *pDevCtrlTemp;
                if (vnet_dev[vport])
                {
                    bcmeapi_reset_mib_cnt(port_id_from_dev(vnet_dev[vport]));
                    pDevCtrlTemp = ((BcmEnet_devctrl *)netdev_priv(vnet_dev[vport]));
                    vstats = &(pDevCtrlTemp->stats);
                    memset(vstats, 0, sizeof(*vstats));
                    atomic_long_set(&vnet_dev[vport]->rx_dropped, 0);
                    memset(&pDevCtrlTemp->cHwStats, 0, sizeof(pDevCtrlTemp->cHwStats));
                    memset(&pDevCtrlTemp->lHwStats, 0, sizeof(pDevCtrlTemp->lHwStats));
                }
            }
            bcmeapi_reset_mib();
            bcmeapi_reset_mib_ext();
            val = 0;
            break;

        case SIOCMIBINFO:
            // Setup correct port indexes.
            swPort = port_id_from_dev(dev);
            if (swPort >= MAX_TOTAL_SWITCH_PORTS )
            {
                printk("SIOCMIBINFO : Invalid swPort: %d \n", swPort);
                return -EINVAL;
            }
            vport = LOGICAL_PORT_TO_VPORT(swPort);

            if (vnet_dev[vport])
            {
                IOCTL_MIB_INFO *mib;

                // Create MIB address.
                mib = &((BcmEnet_devctrl *)netdev_priv(vnet_dev[vport]))->MibInfo;

                // Copy MIB to caller.
                if (copy_to_user((void*)rq->ifr_data, (void*)mib, sizeof(IOCTL_MIB_INFO))) goto ErrorEfault;
            }
            else
            {
                goto ErrorEfault;
            }

            val = 0;
            break;

        case SIOCGQUERYNUMPORTS:
            swPort = port_id_from_dev(dev);

            for (cb_port = enet_get_first_crossbar_port(swPort);
                    cb_port != BP_CROSSBAR_NOT_DEFINED;
                    cb_port = enet_get_next_crossbar_port(swPort, cb_port))
            {
                val |= (1<<cb_port);
            }
            val <<= BP_CROSSBAR_PORT_BASE;


            *data = val;
            ifrd_size = sizeof(*data);
            val = 0;
            break;

        case SIOCSWANPORT:
            if (dev == vnet_dev[0]) goto ErrorEfault;

            swPort = port_id_from_dev(dev);
            if (swPort >= MAX_TOTAL_SWITCH_PORTS)
            {
                printk("SIOCSWANPORT : Invalid swPort: %d \n", swPort);
                return -EINVAL;
            }

            phyId  = enet_logport_to_phyid(swPort);

            if ((pDevCtrl->lanOnlyPorts & swPort))
            {
                printk("SIOCSWANPORT : %s not allowed as WAN <%s switch port: %d> <Logical Port: %d>\n",
                        dev->name,
                        (LOGICAL_PORT_TO_UNIT_NUMBER(swPort)?"Ext":"Runner"),LOGICAL_PORT_TO_PHYSICAL_PORT(swPort), swPort);
                val = -EINVAL;
                break;
            }

            if ( rq->ifr_data ) {
                pDevCtrl->wanPort |= (1 << swPort);
                dev->priv_flags |= IFF_WANDEV;
                dev->priv_flags &= ~IFF_HW_SWITCH;
#if defined(CONFIG_BCM_GMAC)
                if ( IsGmacPort(swPort) )
                {
                    ethsw_eee_port_enable(swPort, 0, 0);
                    gmac_set_wan_port( 1 );
                    ethsw_eee_port_enable(swPort, 1, pDevCtrl->linkState & (1<<swPort));
                }
#endif
                if (pDevCtrl->linkState & (1<<swPort))
                {
                    bcmeapi_EthSetPhyRate(swPort, 1, ((BcmEnet_devctrl *)netdev_priv(dev))->MibInfo.ulIfSpeed, 1);
                }
#ifdef SEPARATE_MAC_FOR_WAN_INTERFACES
                val = kerSysGetMacAddress(dev->dev_addr, dev->ifindex);
                if (val == 0) {
                    memmove(sockaddr.sa_data, dev->dev_addr, ETH_ALEN);

                    /* rtnl lock not needed since dev_ioctl would have taken the lock, set 3rd arg to false */
                    bcmenet_set_dev_mac_addr(dev, &sockaddr, false);
                }
#endif
            } else {
#if defined(CONFIG_BCM_GMAC)
                if (IsGmacPort( swPort ))
                {
                    gmac_set_wan_port( 0 );
                }
#endif
                if (pDevCtrl->linkState & (1<<swPort))
                {
                    bcmeapi_EthSetPhyRate(swPort, 1, ((BcmEnet_devctrl *)netdev_priv(dev))->MibInfo.ulIfSpeed, 0);
                }


#ifdef SEPARATE_MAC_FOR_WAN_INTERFACES
                /* only release MAC if the port was configured as a WAN port  previously */
                if ( pDevCtrl->wanPort & (1 << swPort)) {
                    kerSysReleaseMacAddress(dev->dev_addr);
                    memmove(sockaddr.sa_data, vnet_dev[0]->dev_addr, ETH_ALEN);

                    /* rtnl lock not needed since dev_ioctl would have taken the lock, set 3rd arg to false */
                    bcmenet_set_dev_mac_addr(dev, &sockaddr, false);
                }
#endif

                pDevCtrl->wanPort &= ~(1 << swPort);
                dev->priv_flags &= (~IFF_WANDEV);
#if defined(CONFIG_BCM947189)
                if (LOGICAL_PORT_TO_UNIT_NUMBER(swPort) != 0)
                    dev->priv_flags |= IFF_HW_SWITCH;
#else
                dev->priv_flags |= IFF_HW_SWITCH;
#endif
            }

            enet_conf_cos_for_wan(swPort, rq->ifr_data);

            if (IsExternalSwitchPort(swPort))
            {
                extsw_set_wanoe_portmap(GET_PORTMAP_FROM_LOGICAL_PORTMAP(pDevCtrl->wanPort,1));
            }
            else
            {
                ethsw_set_wanoe_portmap(GET_PORTMAP_FROM_LOGICAL_PORTMAP(pDevCtrl->wanPort,0));
            }

            TRACE(("Set %s wan port %d", dev->name, (int)rq->ifr_data));
            BCM_ENET_DEBUG("bcmenet:SIOCSWANPORT Set %s wan port %s, wan map 0x%x dev flags 0x%x \n",
                    dev->name, rq->ifr_data?"ENB": "DISB",
                    (unsigned int)pDevCtrl->wanPort, (unsigned int)dev->priv_flags);

            /* Update the PBVLAN map in the switch if required */
            bcmenet_update_pbvlan_all_bridge();

            if ( rq->ifr_data )
            {
                bcmsw_enable_mac_rxtx_log(swPort, 1);
            }
            else
            {   /* If this is a WAN-Only port, disable the MAC rxtx */
                if (pDevCtrl->wanOnlyPorts & (1<<swPort))
                {
                    bcmsw_enable_mac_rxtx_log(swPort, 0);
                }
            }

            val = 0;
            break;

        case SIOCGWANPORT:
            if (copy_from_user(e, rq->ifr_data, sizeof(*e))) goto ErrorEfault;
            portMap = pDevCtrl->wanPort;
            ubuf = e->up_len.uptr;
            bufLen = e->up_len.len;
            goto PORTMAPIOCTL;

        case SIOCIFREQ_EXT:
            if (copy_from_user(ifx, rq->ifr_data, sizeof(*ifx))) goto ErrorEfault;

            BCM_IOC_PTR_ZERO_EXT(ifx->stringBuf);
            ubuf = ifx->stringBuf;
            bufLen = ifx->bufLen;

            switch (ifx->opcode)
            {
                case SIOCGPORTWANONLY:
                    portMap = pDevCtrl->wanOnlyPorts;
                    break;
                case SIOCGPORTWANPREFERRED:
                    portMap = pDevCtrl->wanPrefPorts;
                    break;
                case SIOCGPORTLANONLY:
                    portMap = pDevCtrl->lanOnlyPorts;
                    break;
            }

            PORTMAPIOCTL:   /* Common fall through code to return inteface name string based on port bit map */
            val = 0;
            if (ubuf == NULL) goto ErrorEfault;

            buf = kmalloc(bufLen, GFP_KERNEL);
            if( buf == NULL )
            {
                printk(KERN_ERR "bcmenet:SIOCGWANPORT: kmalloc of %d bytes failed\n", bufLen);
                return -ENOMEM;
            }
            buf[0] = 0;

            cum_len = enetLogPort2DevName(portMap, buf, bufLen);

            if (copy_to_user((void*)ubuf, (void*)buf, cum_len + 1)) val = -EFAULT;
            kfree(buf);
            break;


        case SIOCETHSWCTLOPS:
            if (copy_from_user(e, rq->ifr_data, sizeof(*e))) goto ErrorEfault;
            switch(e->op) {
                case ETHSWDUMPPAGE:
                    BCM_ENET_DEBUG("ethswctl ETHSWDUMPPAGE ioctl");
                    if (e->unit) {
                        bcmsw_dump_page_ext(e->page);
                    } else {
                        bcmeapi_ethsw_dump_page(e->page);
                    }
                    val = 0;
                    break;

                    /* Print out enet iuDMA info - Aug 2010 */
                case ETHSWDUMPIUDMA:
                    bcmeapi_dump_queue(e, pDevCtrl);
                    break;

                    /* Get/Set the iuDMA rx channel for a specific eth port - Jan 2011 */
                case ETHSWIUDMASPLIT:
                    bcmeapi_config_queue(e);
                    break;

                case ETHSWDUMPMIB:
                    BCM_ENET_DEBUG("ethswctl ETHSWDUMPMIB ioctl");
                    if (e->unit)
                    {
                        val = bcmsw_dump_mib_ext(e->port, e->type);
                    }
                    else
                    {
                        val = bcmeapi_ethsw_dump_mib(e->port, e->type, e->queue);
                    }
                    break;

                case ETHSWSWITCHING:
                    BCM_ENET_DEBUG("ethswctl ETHSWSWITCHING ioctl");
                    if (e->type == TYPE_ENABLE) {
                        val = ethsw_set_hw_switching(HW_SWITCHING_ENABLED);
                    } else if (e->type == TYPE_DISABLE) {
                        val = ethsw_set_hw_switching(HW_SWITCHING_DISABLED);
                    } else {
                        e->status = ethsw_get_hw_switching_state();
                        SET_ESW_RET2(status);
                        val = 0;
                    }
                    break;

                case ETHSWRXSCHEDULING:
                    BCM_ENET_DEBUG("ethswctl ETHSWRXSCHEDULING ioctl");
                    val = bcmeapi_ioctl_ethsw_rxscheduling(e);
                    SET_ESW_RET(scheduling);
                    break;

                case ETHSWWRRPARAM:
                    BCM_ENET_DEBUG("ethswctl ETHSWWRRPARAM ioctl");
                    val = bcmeapi_ioctl_ethsw_wrrparam(e);
                    ifrd_size = sizeof(*e);

                    break;

                case ETHSWUSEDEFTXQ:
                    BCM_ENET_DEBUG("ethswctl ETHSWUSEDEFTXQ ioctl");
                    pEthCtrl = (BcmEnet_devctrl *)netdev_priv(dev);
                    val = bcmeapi_ioctl_use_default_txq_config(pEthCtrl,e);
                    SET_ESW_RET(ret_val);
                    break;

                case ETHSWDEFTXQ:
                    BCM_ENET_DEBUG("ethswctl ETHSWDEFTXQ ioctl");
                    pEthCtrl = (BcmEnet_devctrl *)netdev_priv(dev);
                    val = bcmeapi_ioctl_default_txq_config(pEthCtrl, e);
                    SET_ESW_RET(queue);
                    break;

                case ETHSWQUEMAP:
                    val = bcmeapi_ioctl_que_map(pDevCtrl, e);
                    ifrd_size = sizeof(*e);
                    break;

                case ETHSWQUEMON:
                    val = bcmeapi_ioctl_que_mon(pDevCtrl, e);
                    ifrd_size = sizeof(*e);
                    break;

#if defined(RXCHANNEL_BYTE_RATE_LIMIT)
                case ETHSWRXRATECFG:
                    BCM_ENET_DEBUG("ethswctl ETHSWRXRATECFG ioctl");
                    val =  bcmeapi_ioctl_rx_rate_config(e);
                    SET_ESW_RET(ret_val);
                    break;

                case ETHSWRXRATELIMITCFG:
                    BCM_ENET_DEBUG("ethswctl ETHSWRXRATELIMITCFG ioctl");
                    val =  bcmeapi_ioctl_rx_rate_limit_config(e);
                    SET_ESW_RET(ret_val);
                    break;
#endif /* defined(RXCHANNEL_BYTE_RATE_LIMIT) */

#if defined(RXCHANNEL_PKT_RATE_LIMIT)
                case ETHSWRXPKTRATECFG:
                    BCM_ENET_DEBUG("ethswctl ETHSWRXRATECFG ioctl");
                    val = bcmeapi_ioctl_rx_pkt_rate_config(e);
                    SET_ESW_RET(ret_val);
                    break;

                case ETHSWRXPKTRATELIMITCFG:
                    BCM_ENET_DEBUG("ethswctl ETHSWRXRATELIMITCFG ioctl");
                    val = bcmeapi_ioctl_rx_pkt_rate_limit_config(e);
                    SET_ESW_RET(ret_val);
                    break;
#endif

                case ETHSWTEST1:
                    BCM_ENET_DEBUG("ethswctl ETHSWTEST1 ioctl");
                    val = bcmeapi_ioctl_test_config(e);
                    SET_ESW_RET(ret_val);
                    break;


                case ETHSWPORTPAUSECAPABILITY:
                    BCM_ENET_DEBUG("ethswctl ETHSWPORTPAUSECAPABILITY ioctl");
                    val = bcmeapi_ioctl_ethsw_port_pause_capability(e);
                    SET_ESW_RET(ret_val);
                    break;

                case ETHSWACBCONTROL:
                    BCM_IOC_PTR_ZERO_EXT(e->vptr);
                    val = bcmeapi_ioctl_extsw_config_acb(e);
                    break;

                case ETHSWCONTROL:
                    BCM_ENET_DEBUG("ethswctl ETHSWCONTROL ioctl");
                    if(bcm63xx_enet_isExtSwPresent() && e->unit == 1) {
                        val = bcmeapi_ioctl_extsw_control(e);
                    } else {
                        val = bcmeapi_ioctl_ethsw_control(e);
                    }
                    ifrd_size = sizeof(*e);
                    break;

                case ETHSWPRIOCONTROL:
                    BCM_ENET_DEBUG("ethswctl ETHSWPRIOCONTROL ioctl");
                    if(bcm63xx_enet_isExtSwPresent() && e->unit == 1) {
                        val = bcmeapi_ioctl_extsw_prio_control(e);
                    }
                    else
                    {
                        val = bcmeapi_ioctl_ethsw_prio_control(e);
                    }

                    ifrd_size = sizeof(*e);
                    break;

                case ETHSWVLAN:
                    BCM_ENET_DEBUG("ethswctl ETHSWVLAN ioctl");
                    if (e->unit == 0) {
                        val = bcmeapi_ioctl_ethsw_vlan(e);
                    } else {
                        val = bcmeapi_ioctl_extsw_vlan(e);
                    }
                    if (e->type == TYPE_GET) ifrd_size = sizeof(*e);
                    
                    break;
#ifdef BCM_ENET_DEBUG_BUILD
                case ETHSWGETRXCOUNTERS:
                    BCM_ENET_DEBUG("ethswctl ETHSWGETRXCOUNTERS ioctl");
                    val = bcmeapi_ioctl_getrxcounters();
                    break;

                case ETHSWRESETRXCOUNTERS:
                    BCM_ENET_DEBUG("ethswctl ETHSWRESETRXCOUNTERS ioctl");
                    val = bcmeapi_ioctl_setrxcounters();
                    break;
#endif

                case ETHSWPBVLAN:
                    BCM_ENET_DEBUG("ethswctl ETHSWPBVLAN ioctl");
                    if(bcm63xx_enet_isExtSwPresent() && e->unit == 1)
                    {
                        val = bcmeapi_ioctl_extsw_pbvlan(e);
                    }
                    else if (e->unit == 0)
                    {
                        val = bcmeapi_ioctl_ethsw_pbvlan(e);
                    }
                    else
                    {
                        val = BCM_E_ERROR; /* Invalid switch unit */
                    }
                    SET_ESW_RET(fwd_map);
                    break;

                case ETHSWCOSCONF:
                    BCM_ENET_DEBUG("ethswctl ETHSWCOSCONF ioctl");
                    val = bcmeapi_ioctl_ethsw_cosq_config(e);
                    SET_ESW_RET(numq);
                    break;

                case ETHSWCOSSCHED:
                    BCM_ENET_DEBUG("ethswctl ETHSWCOSSCHED ioctl");
                    if(bcm63xx_enet_isExtSwPresent() && e->unit == 1)
                    {
                        val = bcmeapi_ioctl_extsw_cosq_sched(e);
                    }
                    else
                    {
                        val =  bcmeapi_ioctl_ethsw_cosq_sched(e);
                    }
                    ifrd_size = sizeof(*e);
                    break;

                case ETHSWCOSPORTMAP:
                    BCM_ENET_DEBUG("ethswctl ETHSWCOSMAP ioctl");
                    if(bcm63xx_enet_isExtSwPresent() && e->unit == 1)
                    {
                        val = bcmeapi_ioctl_extsw_cosq_port_mapping(e);
                    }
                    else
                    {
                        val = bcmeapi_ioctl_ethsw_cosq_port_mapping(e);
                    }

                    if(val < 0)
                    {
                        if(-BCM_E_ERROR == val) val = BCM_E_ERROR;
                        break;
                    }

                    if(e->type == TYPE_GET)
                    {
                        /* queue returned from function. Return value to user */
                        e->queue = val;
                        SET_ESW_RET2(queue);
                        val = 0;
                    }
                    break;

                case ETHSWCOSRXCHMAP:
                    BCM_ENET_DEBUG("ethswctl ETHSWRXCOSCHMAP ioctl");
                    val = bcmeapi_ioctl_ethsw_cosq_rxchannel_mapping(e);
                    SET_ESW_RET(channel);
                    break;

                case ETHSWCOSTXCHMAP:
                    BCM_ENET_DEBUG("ethswctl ETHSWCOSTXCHMAP ioctl");
                    val = bcmeapi_ioctl_ethsw_cosq_txchannel_mapping(e);
                    SET_ESW_RET(queue);
                    break;

                case ETHSWCOSTXQSEL:
                    BCM_ENET_DEBUG("ethswctl ETHSWCOSTXQSEL ioctl");
                    val = bcmeapi_ioctl_ethsw_cosq_txq_sel(e);
                    SET_ESW_RET(ret_val);
                    break;

                case ETHSWSTATCLR:
                    BCM_ENET_DEBUG("ethswctl ETHSWSTATINIT ioctl");
                    val = bcmeapi_ioctl_ethsw_clear_stats
                        ((uint32_t)pDevCtrl->EnetInfo[0].sw.port_map);
                    break;

                case ETHSWSTATPORTCLR:
                    BCM_ENET_DEBUG("ethswctl ETHSWSTATCLEAR ioctl");
                    val = bcmeapi_ioctl_ethsw_clear_port_stats(e);
                    break;

                case ETHSWSTATSYNC:
                    BCM_ENET_DEBUG("ethswctl ETHSWSTATSYNC ioctl");
                    val = ethsw_counter_collect
                        ((uint32_t)pDevCtrl->EnetInfo[0].sw.port_map, 0);
                    break;

                case ETHSWSTATGET:
                    BCM_ENET_DEBUG("ethswctl ETHSWSTATGET ioctl");
                    val = bcmeapi_ioctl_ethsw_counter_get(e);
                    SET_ESW_RET2(counter_val);
                    break;

                case ETHSWEMACGET:
                    BCM_ENET_DEBUG("ethswctl ETHSWEMACGET ioctl");
                    val = bcmeapi_ioctl_ethsw_get_port_emac(e);
                    SET_ESW_RET2(emac_stats_s);
                    break;

                case ETHSWRDPAPORTGET:
                    BCM_ENET_DEBUG("ethswctl ETHSWRDPAPORTGET ioctl");
                    e->val = bcmenet_logical_port_to_rdpa_if(PHYSICAL_PORT_TO_LOGICAL_PORT(e->port, e->unit));
                    val = 0;
                    SET_ESW_RET2(val);
                    break;

                case ETHSWEMACCLEAR:
                    BCM_ENET_DEBUG("ethswctl ETHSWEMACCLEAR ioctl");
                    val = bcmeapi_ioctl_ethsw_clear_port_emac(e);
                    break;

                case ETHSWPORTRXRATE:
                    BCM_ENET_DEBUG("ethswctl ETHSWPORTRXRATE ioctl");
                    if (e->type == TYPE_GET) {
                        val = bcmeapi_ioctl_ethsw_port_irc_get(e);
                        SET_2ESW_RET2(limit, burst_size);
                    } else {
                        val = bcmeapi_ioctl_ethsw_port_irc_set(e);
                    }
                    break;

                case ETHSWPORTTXRATE:
                    BCM_ENET_DEBUG("ethswctl ETHSWPORTTXRATE ioctl");
                    if(bcm63xx_enet_isExtSwPresent() && e->unit == 1) {
                        BCM_IOC_PTR_ZERO_EXT(e->vptr);
                        val = bcmeapi_ioctl_extsw_port_erc_config(e);
                    } else {
                        if (e->type == TYPE_GET) {
                            val = bcmeapi_ioctl_ethsw_port_erc_get(e);
                        } else {
                            val = bcmeapi_ioctl_ethsw_port_erc_set(e);
                        }
                    }
                    ifrd_size = sizeof(*e);
                    break;
                case ETHSWPORTSHAPERCFG:
                    BCM_ENET_DEBUG("ethswctl ETHSWPORTSHAPERCFG ioctl");
                    if(bcm63xx_enet_isExtSwPresent() && e->unit == 1) {
                        val = bcmeapi_ioctl_extsw_port_shaper_config(e);
                    }
                    break;
                case ETHSWJUMBO:
                    BCM_ENET_DEBUG("ethswctl ETHSWJUMBO ioctl");
                    if (e->unit == 0) {
                        val = bcmeapi_ioctl_ethsw_port_jumbo_control(e);
                    } else {
                        val = bcmeapi_ioctl_extsw_port_jumbo_control(e);
                    }
                    SET_ESW_RET2(ret_val);
                    break;

                case ETHSWPORTTRAFFICCTRL:
                    BCM_ENET_DEBUG("ethswctl ETHSWPORTTRAFFICCTRL ioctl");
                    val = bcmeapi_ioctl_ethsw_port_traffic_control(e);
                    SET_ESW_RET(ret_val);
                    break;

                case ETHSWPORTLOOPBACK:
                    BCM_ENET_DEBUG("ethswctl ETHSWPORTLOOPBACK ioctl");
                    phy_id = pDevCtrl->EnetInfo[0].sw.phy_id[e->port];
                    val = bcmeapi_ioctl_ethsw_port_loopback(e, phy_id);
                    SET_ESW_RET(ret_val);
                    break;

                case ETHSWARLACCESS:
                    BCM_ENET_DEBUG("ethswctl ETHSWARLACCESS ioctl");
                    val = bcmeapi_ioctl_ethsw_arl_access(e);
                    ifrd_size = sizeof(*e);
                    break;

                case ETHSWPORTDEFTAG:
                    BCM_ENET_DEBUG("ethswctl ETHSWPORTDEFTAG ioctl");
                    val = bcmeapi_ioctl_ethsw_port_default_tag_config(e);
                    SET_ESW_RET(priority);
                    break;

                case ETHSWCOSPRIORITYMETHOD:
                    BCM_ENET_DEBUG("ethswctl ETHSWCOSPRIORITYMETHOD ioctl");
                    if (bcm63xx_enet_isExtSwPresent() && e->unit == 1 && 
                                 enet_is_mmapped_external_switch(e->unit)) {
                        // call for StarFighter2
                        val = bcmeapi_ioctl_extsw_cos_priority_method_config(e);
                    } else {
                        // call for Legcy external & Robo switch
                        val = bcmeapi_ioctl_ethsw_cos_priority_method_config(e);
                    }
                    SET_ESW_RET(ret_val);
                    break;

                case ETHSWCOSDSCPPRIOMAP:
                    BCM_ENET_DEBUG("ethswctl ETHSWCOSDSCPPRIOMAP ioctl");
                    if(bcm63xx_enet_isExtSwPresent() && e->unit == 1) {
                        val = bcmeapi_ioctl_extsw_dscp_to_priority_mapping(e);
                    } else {
                        val = bcmeapi_ioctl_ethsw_dscp_to_priority_mapping(e);
                    }
                    SET_ESW_RET(priority);
                    break;

                case ETHSWCOSPCPPRIOMAP:
                    BCM_ENET_DEBUG("ethswctl ETHSWCOSPCPPRIOMAP ioctl");
                    if(bcm63xx_enet_isExtSwPresent() && e->unit == 1) {
                        val = bcmeapi_ioctl_extsw_pcp_to_priority_mapping(e);
                    } else {
                        val = bcmeapi_ioctl_ethsw_pcp_to_priority_mapping(e);
                    }
                    SET_ESW_RET(priority);
                    break;

                case ETHSWCOSPIDPRIOMAP:
                    BCM_ENET_DEBUG("ethswctl ETHSWCOSPCPPRIOMAP ioctl");
                    if((bcm63xx_enet_isExtSwPresent() && e->unit == 1) &&
                                 enet_is_mmapped_external_switch(e->unit)) {
                        // call for StarFighter2
                        val = bcmeapi_ioctl_extsw_pid_to_priority_mapping(e); //
                    } else {
                        val = bcmeapi_ioctl_ethsw_pid_to_priority_mapping(e);
                    }
                    SET_ESW_RET(priority);
                    break;

                case ETHSWREGACCESS:
                    val = enet_ioctl_ethsw_regaccess(e);
                    SET_ESW_RET3(data[0], length);
                    break;

                case ETHSWSPIACCESS:
                    //BCM_ENET_DEBUG("ethswctl ETHSWSPIACCESS ioctl");
                    val = bcmeapi_ioctl_ethsw_spiaccess(global.pVnetDev0_g->extSwitch->bus_num,
                            global.pVnetDev0_g->extSwitch->spi_ss, global.pVnetDev0_g->extSwitch->spi_cid, e);
                    SET_ESW_RET3(data[0], length);
                    break;

                case ETHSWPSEUDOMDIOACCESS:
                    //BCM_ENET_DEBUG("ethswctl ETHSWPSEUDOMDIOACCESS ioctl");
                    val = enet_ioctl_ethsw_pmdioaccess(dev, e);
                    SET_ESW_RET3(data[0], length);
                    break;

                case ETHSWINFO:
                    BCM_ENET_DEBUG("ethswctl ETHSWINFO ioctl");
                    val = enet_ioctl_ethsw_info(dev, e, pDevCtrl);
                    ifrd_size = sizeof(*e);
                    break;

                case ETHSWLINKSTATUS:
                    BCM_ENET_DEBUG("ethswctl ETHSWLINKSTATUS ioctl");
                        swPort = PHYSICAL_PORT_TO_LOGICAL_PORT(e->port, e->unit);

#if defined(CONFIG_BCM_GMAC)
                        if (IsGmacPort( swPort ) )
                        {
                            gmac_link_status_changed(e->status, e->speed, e->duplex);
                        }
#endif
                        link_change_handler(swPort, BP_CROSSBAR_NOT_DEFINED, e->status, e->speed, e->duplex);
                        val = 0;
                    break;

                case ETHSWEXTPHYLINKSTATUS:
                    BCM_ENET_DEBUG("ethswctl ETHSWEXTPHYLINKSTATUS ioctl %d, %d, val %d, status %d", e->unit, e->port, e->val, e->status);
                    if (e->type == TYPE_SET)
                    {
                        int cbPort;
                        int cbIndex = 0;
                        PHY_STAT phys;

                        for(cbPort = 0; cbPort < BCMENET_CROSSBAR_MAX_EXT_PORTS; cbPort++)
                        {
                            if ( (pDevCtrl->EnetInfo[e->unit].sw.crossbar[cbPort].switch_port == e->port) &&
                                 (pDevCtrl->EnetInfo[e->unit].sw.crossbar[cbPort].phyconn == e->val) )
                            {
                                break;
                            }
                        }

                        if ( cbPort == BCMENET_CROSSBAR_MAX_EXT_PORTS ) {
                           cbPort = BP_CROSSBAR_NOT_DEFINED;
                           cbIndex = 0;
                        }
                        else
                        {
                           cbIndex = cbPort;
                        }

                        memset(&phys, 0, sizeof(PHY_STAT));
                        /* use speed and duplex from ioctl if provided */
                        if ( e->speed != 0 )
                        {
                            if ( e->speed == 2500 )
                            {
                                phys.spd2500 = 1;
                            }
                            else if ( e->speed == 1000 )
                            {
                                phys.spd1000 = 1;
                            }
                            else if ( e->speed == 100 )
                            {
                                phys.spd100 = 1;
                            }
                            else if (e->speed == 10)
                            {
                                phys.spd10 = 1;
                            }
                            phys.fdx = e->duplex ? 1 : 0;
                        }
                        else
                        {
                            phys = ethsw_phy_stat(e->unit, e->port, cbPort);
                        }
                        phys.lnk = e->status ? 1 : 0;
                        ext_phy_stat[e->unit][e->port][cbIndex] = phys;
                    }
                    else
                    {
                        BCM_ENET_DEBUG("ethswctl ETHSWEXTPHYLINKSTATUS operation not supported (%d)", e->type);
                    }
                    break;

#if defined(CONFIG_BCM_SWMDK)
                case ETHSWKERNELPOLL:
                    val = enet_ioctl_kernel_poll(e);
                    SET_ESW_RET2(mdk_kernel_poll.link_change);
                    break;
#endif
                case ETHSWPHYCFG:
                    BCM_ENET_DEBUG("ethswctl ETHSWPHYCFG ioctl");
                    val = bcmeapi_ioctl_phy_cfg_get(dev, e);
                    SET_ESW_RET2(phycfg);
                    break;

                case ETHSWPHYMODE:
                    BCM_ENET_DEBUG("ethswctl ETHSWPHYMODE ioctl");

                    pEthCtrl = (BcmEnet_devctrl *)netdev_priv(dev);
                    e->sub_port = BP_PHY_PORT_TO_CROSSBAR_PORT(e->sub_port);
                    if (e->addressing_flag & ETHSW_ADDRESSING_DEV)
                    {
                        e->unit = LOGICAL_PORT_TO_UNIT_NUMBER(pEthCtrl->sw_port_id);
                        e->port = LOGICAL_PORT_TO_PHYSICAL_PORT(pEthCtrl->sw_port_id);
                    }

                    if (!(e->addressing_flag & ETHSW_ADDRESSING_SUBPORT))
                    {
                        phy_id = pDevCtrl->EnetInfo[e->unit].sw.phy_id[e->port];
                    }
                    else
                    {
                        /* If SUBPORT addressing is used but sub_unit is not set, set it to unit */
                        if (e->sub_unit == -1)
                        {
                            e->sub_unit = e->unit;
                        }
                        phy_id = pDevCtrl->EnetInfo[e->sub_unit].sw.crossbar[e->sub_port].phy_id;
                    }

                    if (((e->addressing_flag & ETHSW_ADDRESSING_SUBPORT) ||
                        enet_phyid_get_cbport(e->unit, e->port, phy_id, &e->sub_port)) &&
                        enet_cb_port_has_combo_phy(e->unit, e->sub_port))
                    {
                        enet_cb_port_get_phyids(e->unit, e->sub_port, &phy_id, &phy_id_ext);
                        val = bcmeapi_ioctl_ethsw_combophy_mode(e, phy_id, phy_id_ext);
                    }
                    else
                    {
                        val = bcmeapi_ioctl_ethsw_phy_mode(e, phy_id);
                    }
                    if (e->type == TYPE_GET) ifrd_size = sizeof(*e);
                    break;

                case ETHSWGETIFNAME:
                    logical_port = PHYSICAL_PORT_TO_LOGICAL_PORT(e->port, e->unit);
                    BCM_ENET_DEBUG("ethswctl ETHSWGETIFNAME ioctl");
                    if ((LOGICAL_PORT_TO_VPORT(logical_port) != -1) &&
                            (vnet_dev[LOGICAL_PORT_TO_VPORT(logical_port)] != NULL)) {
                        memcpy(e->ifname, vnet_dev[LOGICAL_PORT_TO_VPORT(logical_port)]->name, sizeof(e->ifname));
                        SET_ESW_RET(ifname);
                    } else {
                        /* Return error as there is no interface for the given port */
                        val = -EFAULT;
                    }
                    break;

                case ETHSWMULTIPORT:
                    BCM_ENET_DEBUG("ethswctl ETHSWMULTIPORT ioctl");
                    val = bcmeapi_ioctl_set_multiport_address(e);
                    break;

                case ETHSWDOSCTRL:
                    BCM_ENET_DEBUG("ethswctl ETHSWDOSCTRL ioctl");
                    val = enet_ioctl_ethsw_dos_ctrl(e);
                    SET_ESW_RET(dosCtrl);
                    break;

                case ETHSWDEBUG:
                    bcmeapi_ioctl_debug_conf(e);
                    SET_ESW_RET(ret_val);
                    break;

                case ETHSWUNITPORT:
                    if (bcm63xx_enet_getPortmapFromName(e->ifname, &e->unit, &e->port_map)){
                        printk("No device name %s found in Ethernet driver\n", e->ifname);
                        val = -EFAULT;
                    }
                    SET_2ESW_RET2(unit, port_map);
                    break;

                case ETHSWOAMIDXMAPPING:
                    if(e->oam_idx_str.map_sub_type == OAM_MAP_SUB_TYPE_FROM_UNIT_PORT){
                        if (bcm63xx_enet_getOamIdxFromUnitPort(e->unit, e->port, &e->oam_idx_str.oam_idx)){
                            val = -EFAULT;
                        }
                    }
                    else{
                        if (bcm63xx_enet_getUnitPortFromOamIdx(e->oam_idx_str.oam_idx, &e->unit, &e->port)){
                            val = -EFAULT;
                        }
                        if (e->oam_idx_str.map_sub_type == OAM_MAP_SUB_TYPE_TO_RDPA_IF){
                            e->oam_idx_str.rdpa_if = bcmenet_logical_port_to_rdpa_if(PHYSICAL_PORT_TO_LOGICAL_PORT(e->port, e->unit));
                        }
                    }
                    ifrd_size = sizeof(*e);
                    val = 0;
                    break;

                case ETHSWSOFTSWITCHING:
                    if ( e->type == TYPE_GET ) {
                        e->status = pDevCtrl->softSwitchingMap;
                        SET_ESW_RET2(status);
                    }
                    else {
                        swPort = PHYSICAL_PORT_TO_LOGICAL_PORT(e->port, e->unit);
                        if (swPort >= MAX_TOTAL_SWITCH_PORTS) {
                            val = -EINVAL;
                            break;
                        }
                        val = bcm_set_soft_switching(swPort, e->type);
                    }
                    break;

                case ETHSWHWSTP:
                    if ( e->type == TYPE_GET )
                    {
                        e->status = pDevCtrl->stpDisabledPortMap;
                        SET_ESW_RET2(status);
                        }
                    else
                    {
                        swPort = PHYSICAL_PORT_TO_LOGICAL_PORT(e->port, e->unit);
                        if (swPort >= MAX_TOTAL_SWITCH_PORTS) {
                            val = -EINVAL;
                            break;
                        }
                        val = bcm_set_hw_stp(swPort, e->type);
                    }
                    break;

                case ETHSWMIRROR:
                    if ( e->type == TYPE_GET ) {
                        bcmeapi_ioctl_ethsw_port_mirror_get(e);
                        SET_ESW_RET2(port_mirror_cfg);
                    }
                    else {
                        bcmeapi_ioctl_ethsw_port_mirror_set(e);
                    }
                    break;


#if defined(CONFIG_BCM_ETH_PWRSAVE)
                case ETHSWPHYAPD:
                    if ( e->type == TYPE_GET ) {
                        e->val = BcmPwrMngtGetEthAutoPwrDwn();
                        SET_ESW_RET2(val);
                    } else {
                        BcmPwrMngtSetEthAutoPwrDwn(e->val, pDevCtrl->linkState);
                    }
                    val = 0;
                    break;
#endif

#if defined(CONFIG_BCM_ENERGY_EFFICIENT_ETHERNET)
                case ETHSWPHYEEE:
                    if ( e->type == TYPE_GET )
                    {
                        e->val = BcmPwrMngtGetEnergyEfficientEthernetEn();
                        SET_ESW_RET2(val);
                    }
                    else
                        BcmPwrMngtSetEnergyEfficientEthernetEn(e->val);
                    val = 0;
                    break;
#endif

#if defined(CONFIG_BCM_ETH_DEEP_GREEN_MODE)
                case ETHSWDEEPGREENMODE:
                    if ( e->type == TYPE_GET )
                    {
                        e->val = BcmPwrMngtGetDeepGreenMode(e->val);
                        SET_ESW_RET2(val);
                    }
                    else
                        BcmPwrMngtSetDeepGreenMode(e->val, pDevCtrl->linkState);
                    val = 0;
                    break;
#endif

                case ETHSWPORTSTORMCTRL:
                    BCM_ENET_DEBUG("ethswctl ETHSWPORTSTORMCTRL ioctl");
                    if(bcm63xx_enet_isExtSwPresent() && e->unit == 1) {
                        val = bcmeapi_ioctl_extsw_port_storm_ctrl(e);
                    } 
                    ifrd_size = sizeof(*e);
                    break;

                case ETHSWCFP:
                    bcmeapi_ioctl_cfp(e);
                    ifrd_off = &e->cfpArgs;
                    ifrd_size = sizeof(e->cfpArgs);
                    break;
                default:
                    BCM_ENET_DEBUG("ethswctl unsupported ioctl");
                    val = -EOPNOTSUPP;
                    break;
            }
            break;

        case SIOCETHCTLOPS:
            if (copy_from_user(ethctl, rq->ifr_data, sizeof(*ethctl))) goto ErrorEfault;
            switch(ethctl->op) {


                case ETHGETNUMTXDMACHANNELS:
                    val = bcmeapi_get_num_txques(ethctl);
                    SET_ECT_RET2(ret_val);
                    break;

                case ETHSETNUMTXDMACHANNELS:
                    val = bcmeapi_set_num_txques(ethctl);
                    break;

                case ETHGETNUMRXDMACHANNELS:
                    val = bcmeapi_get_num_rxques(ethctl);
                    SET_ECT_RET2(ret_val);
                    break;

                case ETHSETNUMRXDMACHANNELS:
                    val = bcmeapi_set_num_rxques(ethctl);
                    break;

                case ETHGETSOFTWARESTATS:
                    pEthCtrl = (BcmEnet_devctrl *)netdev_priv(dev);
                    display_software_stats(pEthCtrl);
                    val = 0;
                    break;

                case ETHGETPHYID: /* Get address of MII PHY in use by dev */
                    ethctl->phy_addr = phy_id = enet_dev_get_phy_dev(dev, ethctl);
                    if (!IsPhyConnected(phy_id))
                        return -EINVAL;

                    ethctl->flags = 0;
                    /* Let us also return phy flags needed for accessing the phy */
                    if (IsSerdes(phy_id))
                    {
                            ethctl->flags = ETHCTL_FLAG_ACCESS_SERDES;
                    }

                    if (copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)))
                        return -EFAULT;

                    return 0;

                case ETHGETPHYPWR:
                case ETHSETPHYPWRON:
                case ETHSETPHYPWROFF:
                    ethctl->phy_addr = enet_dev_get_phy_dev(dev, ethctl);

                    if(ethctl->phy_addr != 0) {
                        uint16  reg_data;
                        ethsw_phy_rreg(ethctl->phy_addr, MII_BMCR, &reg_data);

                        if (ethctl->op == ETHGETPHYPWR) {
                            ethctl->ret_val = !(reg_data & BMCR_PDOWN);
                            ifrd_size = sizeof(*ethctl);
                            val = 0;
                            break;
                        }

                        reg_data |= BMCR_ANRESTART;
                        if (ethctl->op == ETHSETPHYPWRON)
                            reg_data &= ~BMCR_PDOWN;
                        else
                            reg_data |= BMCR_PDOWN;
                        ethsw_phy_wreg(ethctl->phy_addr, MII_BMCR, &reg_data);
                    }

                    swPort = port_id_from_dev(dev);
                    if (swPort >= MAX_TOTAL_SWITCH_PORTS)
                    {
                        printk("Invalid swPort: %d, op=%d\n", swPort, ethctl->op);
                        val = -EINVAL;
                        break;
                    }

                    if (ethctl->op == ETHGETPHYPWR) {
                        ethctl->ret_val = !ethsw_switch_get_port_power_mode(swPort);
                        ifrd_size = sizeof(*ethctl);
                    } else {
                        ethsw_switch_manage_port_power_mode(swPort, ethctl->op==ETHSETPHYPWRON);
                    }
                    val = 0;
                    break;

                case ETHGETMIIREG:       /* Read MII PHY register. */
                    BCM_ENET_DEBUG("phy_id: %d; reg_num = %d \n", ethctl->phy_addr, ethctl->phy_reg);
                    {
                        down(&bcm_ethlock_switch_config);
                        ethctl->ret_val = ethsw_phy_rreg32(ethctl->phy_addr,
                                       ethctl->phy_reg, &ethctl->val, ethctl->flags);
                        BCM_ENET_DEBUG("phy_id: %d;   reg_num = %d  val = 0x%x\n",
                                ethctl->phy_addr, ethctl->phy_reg, val);
                        up(&bcm_ethlock_switch_config);
                        ifrd_size = sizeof(*ethctl);
                    }
                    break;

                case ETHSETMIIREG:       /* Write MII PHY register. */
                    BCM_ENET_DEBUG("phy_id: %d; reg_num = %d; val = 0x%x \n", ethctl->phy_addr,
                            ethctl->phy_reg, ethctl->val);
                    {
                        down(&bcm_ethlock_switch_config);
                        ethctl->ret_val = ethsw_phy_wreg32(ethctl->phy_addr,
                               ethctl->phy_reg, &ethctl->val, ethctl->flags);
                        BCM_ENET_DEBUG("phy_id: %d; reg_num = %d  val = 0x%x\n",
                                ethctl->phy_addr, ethctl->phy_reg, val);
                        up(&bcm_ethlock_switch_config);
                    }
                    break;

#if defined(CONFIG_BCM_ENERGY_EFFICIENT_ETHERNET)
                case ETHGETPHYEEE:
                case ETHSETPHYEEEON:
                case ETHSETPHYEEEOFF:
                case ETHGETPHYEEERESOLUTION:
                    BCM_ENET_DEBUG("op=%d phy_id=0x%x sub_port=%d\n",
                                    ethctl->op, ethctl->phy_addr, ethctl->sub_port);
                    val = -EINVAL;
                    swPort = port_id_from_dev(dev);
                    if (swPort >= MAX_TOTAL_SWITCH_PORTS)
                    {
                        printk("Invalid swPort: %d, op=%d\n", swPort, ethctl->op);
                        break;
                    }

                    phy_id = enet_dev_get_phy_dev(dev, ethctl);
                    if (!phy_id)
                    {
                        printk("Invalid phy_id: %d, op=%d sub_port=%d\n",
                                    ethctl->phy_addr, ethctl->op, ethctl->sub_port);
                        break;
                    }

                    val = 0;
                    ifrd_size = sizeof(*ethctl);
                    if (ethctl->op == ETHGETPHYEEE) {
                        if (ethsw_eee_get_by_log_port(swPort, &ethctl->ret_val))
                                goto ErrorEfault;
                        break;
                    }
                    if (ethctl->op == ETHGETPHYEEERESOLUTION) {
                        if (ethsw_eee_resolution_get_by_log_port(swPort, &ethctl->ret_val))
                                goto ErrorEfault;
                        break;
                    }
                    ethsw_eee_set_by_log_port(swPort, ethctl->op == ETHSETPHYEEEON);
                    ifrd_size = 0;
                    break;
#endif // #if defined(CONFIG_BCM_ENERGY_EFFICIENT_ETHERNET)

                default:
                    val = -EOPNOTSUPP;
                    break;
            }
            break;

        default:
            val = -EOPNOTSUPP;
            break;
    }

    if (ifrd_size)
    {
        if (copy_to_user((void *)((char *)rq->ifr_data + ((char *)ifrd_off - (char *)&rq_data)), 
            ifrd_off, ifrd_size)) goto ErrorEfault;
    }

    if (ifrd_size2)
    {
        if (copy_to_user((void *)((char *)rq->ifr_data + ((char *)ifrd_off2 - (char *)&rq_data)), 
            ifrd_off2, ifrd_size2)) goto ErrorEfault;
    }
    return val;

ErrorEfault:
    return -EFAULT;
}




static int __init bcmenet_module_init(void)
{
    int status;

    TRACE(("bcm63xxenet: bcmenet_module_init\n"));

    bcmeapi_module_init();

    sema_init(&bcm_ethlock_switch_config, 1);
    sema_init(&bcm_link_handler_config, 1);

    if ( BCM_SKB_ALIGNED_SIZE != skb_aligned_size() )
    {
        printk("skb_aligned_size mismatch. Need to recompile enet module\n");
        return -ENOMEM;
    }
#if defined(_CONFIG_BCM_ARL)
    bcm_arl_process_hook_g = enet_hook_for_arl_access;
#endif


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

    status = bcm63xx_enet_probe();

#if defined(CONFIG_BCM_SWMDK)
    bcmFun_reg(BCM_FUN_ID_ENET_LINK_CHG, link_change_handler_wrapper);
#endif /*CONFIG_BCM_SWMDK*/
    bcmFun_reg(BCM_FUN_ID_ENET_IS_WAN_PORT, bcmenet_is_wan_port);
    bcmFun_reg(BCM_FUN_ID_ENET_IS_BONDED_LAN_WAN_PORT, bcmenet_is_bonded_lan_wan_port);

    bcmeapi_module_init2();

#if defined(CONFIG_BCM_KF_NETFILTER)
    /* Register bridge notifier hooks */
    register_bridge_stp_notifier(&br_stp_handler);
    register_bridge_notifier(&br_notifier);
#endif

    return status;
}

int bcm63xx_enet_getPortmapFromDev(struct net_device *dev, int *pUnit, unsigned int *pPortmap)
{
    int port;
    int i;

    for (i = 1; i <= vport_cnt; i++) {
        if (dev == vnet_dev[i]) {
            break;
        }
    }

    if ( i > vport_cnt ) {
        return -1;
    }

    port = port_id_from_dev(dev);
    if ( port >= MAX_TOTAL_SWITCH_PORTS )
    {
        return -1;
    }
    *pUnit = LOGICAL_PORT_TO_UNIT_NUMBER(port);
    *pPortmap = ( 1 << LOGICAL_PORT_TO_PHYSICAL_PORT(port) );

    return 0;

}

int bcm63xx_enet_getPortFromDev(struct net_device *dev, int *pUnit, int *pPort)
{
    unsigned int portmap;
    int err;
    err = bcm63xx_enet_getPortmapFromDev(dev,pUnit,&portmap);
    if (err) return err;

    *pPort = 0;
    while(portmap)
    {
        if (portmap & (1<<(*pPort)))
        {
            break;
        }
        *pPort += 1;
    }
    return 0;

}
int bcm63xx_enet_getPortmapFromName(char *pIfName, int *pUnit, unsigned int *pPortmap)
{
    struct net_device *dev;

    dev = dev_get_by_name(&init_net, pIfName);
    if (NULL == dev)
    {
        return -1;
    }

    /* find root device */
    while( 1 )
    {
        if(netdev_path_is_root(dev))
        {
            break;
        }
        dev_put(dev);
        dev = netdev_path_next_dev(dev);
        dev_hold(dev);
    }

    if ( bcm63xx_enet_getPortmapFromDev(dev, pUnit, pPortmap) < 0 )
    {
        dev_put(dev);
        return -1;
    }
    dev_put(dev);
    return 0;
}

int bcm63xx_enet_getPortFromName(char *pIfName, int *pUnit, int *pPort)
{
    int err;
    unsigned int portmap;

    err = bcm63xx_enet_getPortmapFromName(pIfName,pUnit,&portmap);
    if (err) return err;

    *pPort=0;
    while(portmap)
    {
        if (portmap & (1<<(*pPort)))
        {
            break;
        }
        *pPort +=1;
    }
    return 0;
}

/* return logical portmap for all the Ethernet Driver managed ports in this bridge */
static uint32_t bcmenet_bridge_get_logical_pmap(char *brName)
{
    unsigned int brPort = 0xFFFFFFFF;
    struct net_device *dev;
    uint32_t portMap = 0, physclPortMap = 0;
    int unit;

    for(;;)
    {
#if defined(CONFIG_BCM_KF_NETFILTER)
        dev = bridge_get_next_port(brName, &brPort);
#else
        dev = NULL;
#endif
        if (dev == NULL)
            break;
        /* find root device */
        while( !netdev_path_is_root(dev) )
        {
            dev = netdev_path_next_dev(dev);
        }
        {
            /* One linux device may have multiple physical ports in case of port-trunking/LAG */
            if ( bcm63xx_enet_getPortmapFromDev(dev, &unit, &physclPortMap) < 0 )
            {
                continue;
            }

            portMap |= SET_PORTMAP_IN_LOGICAL_PORTMAP(physclPortMap, unit);
        }
    }

    return portMap;
}
void bcmenet_update_pbvlan_all_bridge(void)
{
    char br_list[64];
    char *brName, *tracker;
    rcu_read_lock();
    bridge_get_br_list(br_list, sizeof(br_list));
    tracker = br_list;
    while (tracker)
    {
        brName = tracker;
        tracker = strchr(tracker,',');
        if(tracker)
        {
            *tracker = '\0';
            tracker++;
        }
        bridge_update_pbvlan(brName);
    }
    rcu_read_unlock();
}


static int bridge_notifier(struct notifier_block *nb, unsigned long event, void *info)
{
    struct bridge_notifier_info *info_p = info; 
    switch (event)
    {
        case BREVT_IF_CHANGED:
            bridge_update_pbvlan(info_p->br_dev->name);
            break;
    }
    return NOTIFY_DONE;
}

static int bridge_stp_handler(struct notifier_block *nb, unsigned long event, void *portInfo)
{
    struct stpPortInfo *pInfo = (struct stpPortInfo *)portInfo;
    BcmEnet_devctrl *pDevCtrl = netdev_priv(vnet_dev[0]);

    switch (event)
    {
        case BREVT_STP_STATE_CHANGED:
            {
                unsigned char stpVal;
                int port;
                int unit;

                if ( bcm63xx_enet_getPortFromName(&pInfo->portName[0], &unit, &port ) < 0 )
                {
                    break;
                }

                switch ( pInfo->stpState )
                {
                    case BR_STATE_BLOCKING:
                        stpVal = REG_PORT_STP_STATE_BLOCKING;
                        break;

                    case BR_STATE_FORWARDING:
                        stpVal = REG_PORT_STP_STATE_FORWARDING;
                        break;

                    case BR_STATE_LEARNING:
                        stpVal = REG_PORT_STP_STATE_LEARNING;
                        break;

                    case BR_STATE_LISTENING:
                        stpVal = REG_PORT_STP_STATE_LISTENING;
                        break;

                    case BR_STATE_DISABLED:
                        stpVal = REG_PORT_STP_STATE_DISABLED;
                        break;

                    default:
                        stpVal = REG_PORT_NO_SPANNING_TREE;
                        break;
                }

                if ( 0 == (pDevCtrl->stpDisabledPortMap & (1 << PHYSICAL_PORT_TO_LOGICAL_PORT(port, unit))) ) {
                    bcmeapi_ethsw_set_stp_mode(unit, port, stpVal);
                }
                break;
            }
    }
    return NOTIFY_DONE;
}


static void bridge_update_pbvlan(char *brName)
{
    uint32_t portMapLoop, logicalPortMap, fwdPortMap, port, unit, impPortMap;

    /* Get the logical portmap for all the Ethernet Driver managed ports connected to this bridge */
    logicalPortMap = bcmenet_bridge_get_logical_pmap(brName); 
    /* Remove the bits from portmap for wanPort and softSwitchingMap --- These ports are always isolated */
    logicalPortMap &= ~(global.pVnetDev0_g->wanPort | global.pVnetDev0_g->softSwitchingMap);
    if (logicalPortMap == 0) /* No ports in this bridge are from Ethernet Driver - bail out */
    {
        return;
    }

    for(unit=0; unit < MAX_SWITCH_UNITS; unit++)
    {
        fwdPortMap = portMapLoop = GET_PORTMAP_FROM_LOGICAL_PORTMAP(logicalPortMap,unit);
        port = 0; /* Start from first switch port */
        while(portMapLoop)
        {
            if (portMapLoop & (1<<port))
            {
                /* Set IMP port as well in the forwarding portmap */
                impPortMap = (1<<logicalport_to_imp_map[PHYSICAL_PORT_TO_LOGICAL_PORT(port, unit)]);

                /* Set PBVLANMAP for this port */
                if (unit == 0) /* Internal switch */
                {
                    ethsw_set_pbvlan(port, fwdPortMap|impPortMap);
                }
                else
                {
                    extsw_set_pbvlan(port, fwdPortMap|impPortMap);
                }
                portMapLoop &= ~(1<<port); /* reset port bit in portMap */
            }
            port++;
        }
    }
}

/*
 * We need this function in non-gmac build as well.
 * It searches both the internal and external switch ports.
 */
int enet_logport_to_phyid(int log_port)
{

    ASSERT(log_port < (MAX_TOTAL_SWITCH_PORTS));
    return enet_sw_port_to_phyid(LOGICAL_PORT_TO_UNIT_NUMBER(log_port), LOGICAL_PORT_TO_PHYSICAL_PORT(log_port));
}

int enet_sw_port_to_phyid(int unit, int phy_port)
{
    ETHERNET_MAC_INFO *info = EnetGetEthernetMacInfo();
    if (unit >= BP_MAX_ENET_MACS || phy_port < 0 || phy_port >= BP_MAX_SWITCH_PORTS) {
        /* Error */
        return 0;
    }
    return info[unit].sw.phy_id[phy_port];
}

int enet_cb_port_to_phyid(int unit, int cb_port)
{
    ETHERNET_MAC_INFO *info = EnetGetEthernetMacInfo();
    if ( (cb_port < 0) || (cb_port >= BCMENET_CROSSBAR_MAX_EXT_PORTS))
    {
        return 0;
    }
    return info[unit].sw.crossbar[cb_port].phy_id;
}

int enet_cb_port_has_combo_phy(int unit, int cb_port)
{
    ETHERNET_MAC_INFO *info = EnetGetEthernetMacInfo();

    if ( (cb_port < 0) || (cb_port >= BCMENET_CROSSBAR_MAX_EXT_PORTS))
    {
        return 0;
    }
    return info[unit].sw.crossbar[cb_port].phy_id_ext > 0;
}

void enet_cb_port_get_phyids(int unit, int cb_port, int *phyId, int *phyIdExt)
{
    ETHERNET_MAC_INFO *info = EnetGetEthernetMacInfo();
    if ( (cb_port < 0) || (cb_port >= BCMENET_CROSSBAR_MAX_EXT_PORTS))
    {
        *phyId = *phyIdExt = 0;
        return;
    }
    *phyId = info[unit].sw.crossbar[cb_port].phy_id;
    *phyIdExt = info[unit].sw.crossbar[cb_port].phy_id_ext;
}

int enet_phyid_get_cbport(int unit, int port, int phyId, int *cbport)
{
    ETHERNET_MAC_INFO *info = EnetGetEthernetMacInfo();
    int cb_port, logPort;

    logPort = PHYSICAL_PORT_TO_LOGICAL_PORT(port, unit);
    for (cb_port = enet_get_first_crossbar_port(logPort);
            cb_port != BP_CROSSBAR_NOT_DEFINED;
            cb_port = enet_get_next_crossbar_port(logPort, cb_port))
    {
        if (phyId == info[unit].sw.crossbar[cb_port].phy_id);
        {
            *cbport = cb_port;
            return 1;
        }
    }
    return 0;
}

unsigned long enet_get_portmap(unsigned char unit)
{
    struct net_device *dev = vnet_dev[0];
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)netdev_priv(dev);
    if (unit >= BP_MAX_ENET_MACS) {
        /* Error */
        return 0;
    }
    return pDevCtrl->EnetInfo[unit].sw.port_map;
}

unsigned long enet_get_port_flags(unsigned char unit, int port)
{
    struct net_device *dev = vnet_dev[0];
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)netdev_priv(dev);
    if (unit >= BP_MAX_ENET_MACS || port < 0 || port >= BP_MAX_SWITCH_PORTS) {
        /* Error */
        return 0;
    }
    return pDevCtrl->EnetInfo[unit].sw.port_flags[port];
}

unsigned int enet_get_consolidated_portmap(void)
{
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)netdev_priv(vnet_dev[0]);
    return pDevCtrl->allPortMap;
}

#ifdef DYING_GASP_API
int enet_send_dying_gasp_pkt(void)
{
    struct net_device *dev = NULL;
    int i;

    /* Indicate we are in a dying gasp context and can skip
       housekeeping since we're about to power down */
    dg_in_context = 1;

    if (dg_skbp == NULL) {
        BCM_ENET_DEBUG("%s No DG skb to send \n", __FUNCTION__);
        return -1;
    }
    for (i = 0; i < MAX_TOTAL_SWITCH_PORTS - 1; i++)
    {
        int iVport = LOGICAL_PORT_TO_VPORT(i);

        /* Is this a port for a valid ethernet device? */
        if(iVport <= 0)
        {
            /* Nope - skip to next port */
            continue;
        }

        /* Get dev pointer for this port */
        dev = vnet_dev[iVport];

        /* Is this a WAN port? */
        if (dev && dev->priv_flags & IFF_WANDEV) {
            int iRetVal;

            /* Copy src MAC from dev into dying gasp packet */
            memcpy(dg_skbp->data + ETH_ALEN, dev->dev_addr, ETH_ALEN);

            /* Transmit dying gasp packet */
            iRetVal = bcm63xx_enet_xmit(SKBUFF_2_PNBUFF(dg_skbp), dev);
            printk("\n%s DG sent out on wan port %s (ret=%d)\n", __FUNCTION__, dev->name, iRetVal);
            break;
        }
    } // for
    return 0;
}
#endif

/* Physical port to logical port mapping */
struct net_device *enet_phyport_to_vport_dev(int port)
{
    ASSERT(port < TOTAL_SWITCH_PORTS);
    return (vnet_dev[LOGICAL_PORT_TO_VPORT(PHYSICAL_PORT_TO_LOGICAL_PORT(port, extSwInfo.present))]);
}

uint32 ConfigureJumboPort(uint32 regVal, int portVal, unsigned int configVal) // bill
{
    UINT32 controlBit;

    // Test for valid port specifier.
    if ((portVal >= ETHSWCTL_JUMBO_PORT_GPHY_0) && (portVal <= ETHSWCTL_JUMBO_PORT_ALL))
    {
        // Switch on port ID.
        switch (portVal)
        {
            case ETHSWCTL_JUMBO_PORT_MIPS:
                controlBit = ETHSWCTL_JUMBO_PORT_MIPS_MASK;
                break;
            case ETHSWCTL_JUMBO_PORT_GPON:
                controlBit = ETHSWCTL_JUMBO_PORT_GPON_MASK;
                break;
            case ETHSWCTL_JUMBO_PORT_USB:
                controlBit = ETHSWCTL_JUMBO_PORT_USB_MASK;
                break;
            case ETHSWCTL_JUMBO_PORT_MOCA:
                controlBit = ETHSWCTL_JUMBO_PORT_MOCA_MASK;
                break;
            case ETHSWCTL_JUMBO_PORT_GPON_SERDES:
                controlBit = ETHSWCTL_JUMBO_PORT_GPON_SERDES_MASK;
                break;
            case ETHSWCTL_JUMBO_PORT_GMII_2:
                controlBit = ETHSWCTL_JUMBO_PORT_GMII_2_MASK;
                break;
            case ETHSWCTL_JUMBO_PORT_GMII_1:
                controlBit = ETHSWCTL_JUMBO_PORT_GMII_1_MASK;
                break;
            case ETHSWCTL_JUMBO_PORT_GPHY_1:
                controlBit = ETHSWCTL_JUMBO_PORT_GPHY_1_MASK;
                break;
            case ETHSWCTL_JUMBO_PORT_GPHY_0:
                controlBit = ETHSWCTL_JUMBO_PORT_GPHY_0_MASK;
                break;
            default: // ETHSWCTL_JUMBO_PORT_ALL:
                controlBit = ETHSWCTL_JUMBO_PORT_MASK_VAL;  // ALL bits
                break;
        }

        // Test for accept JUMBO frames.
        if (configVal != 0)
        {
            // Setup register value to accept JUMBO frames.
            regVal |= controlBit;
        }
        else
        {
            // Setup register value to reject JUMBO frames.
            regVal &= ~controlBit;
        }
    }

    // Return new JUMBO configuration control register value.
    return regVal;
}

int enet_get_next_crossbar_port(int logPort, int cb_port)
{
    int phsclPort = LOGICAL_PORT_TO_PHYSICAL_PORT(logPort);
    int sw_unit = LOGICAL_PORT_TO_UNIT_NUMBER(logPort);
    BcmEnet_devctrl *pDevCtrl = (BcmEnet_devctrl *)netdev_priv(vnet_dev[0]);
    ETHERNET_MAC_INFO *info = &pDevCtrl->EnetInfo[sw_unit];
    int muxExtPort = (cb_port == BP_CROSSBAR_NOT_DEFINED ? 0: cb_port+1);

    for(/* muxExtPort init above*/; muxExtPort < BCMENET_CROSSBAR_MAX_EXT_PORTS; muxExtPort++)
    {
        /* Skip no member crossbar ports */
        if (info->sw.crossbar[muxExtPort].switch_port != phsclPort) continue;
        return muxExtPort;
    }
    return BP_CROSSBAR_NOT_DEFINED;
}

PHY_STAT enet_get_ext_phy_stat(int unit, int phy_port, int cb_port)
{
    PHY_STAT phys;

    memset(&phys, 0, sizeof(PHY_STAT));
    if ((unit >= BP_MAX_ENET_MACS) || (phy_port < 0) || (phy_port >= BP_MAX_SWITCH_PORTS)) {
        return phys;
    }
    else
    {
        if ( cb_port == BP_CROSSBAR_NOT_DEFINED )
        {
            cb_port = 0;
        }
        if ( (cb_port < 0) || (cb_port >= BCMENET_CROSSBAR_MAX_EXT_PORTS) )
        {
            return phys;
        }
        return ext_phy_stat[unit][phy_port][cb_port];
    }
}

#define ENET_HEX_DUMP_COLS 16
void enet_hex_dump(void *mem, unsigned int len)
{
    unsigned int i;

    for(i = 0; i < len; i++)
    {
        /* print offset */
        if(i % ENET_HEX_DUMP_COLS == 0)
        {
            printk("0x%08lx: ", (long)mem + i);
        }
        else if (i % 8 == 0)
        {
            printk(" ");
        }

        /* print hex data */
        printk("%02x ", 0xFF & ((char*)mem)[i]);

        if((i+1) % ENET_HEX_DUMP_COLS == 0)
        {
            printk("\n");
        }
    }

    if (i % ENET_HEX_DUMP_COLS) printk("\n");
}

int enet_get_total_ports_num(void)
{
    return vport_cnt;
}


module_init(bcmenet_module_init);
module_exit(bcmenet_module_cleanup);
MODULE_LICENSE("GPL");

