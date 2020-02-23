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
#ifndef _BCMENET_COMMON_H_
#define _BCMENET_COMMON_H_

#if defined(_BCMENET_LOCAL_)
#include "bcm_OS_Deps.h"
#include "bcmtypes.h"
#include "bcmmii.h"
#endif

#include "bcm_mm.h"

#if (defined(CONFIG_BCM_FAP) || defined(CONFIG_BCM_FAP_MODULE))
    #define _CONFIG_BCM_FAP
#endif

#if (defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    #define _CONFIG_BCM_BPM
#endif

#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
    #define _CONFIG_BCM_INGQOS
#endif


#if (defined(CONFIG_BCM_ARL) || defined(CONFIG_BCM_ARL_MODULE))
    #define _CONFIG_BCM_ARL
#endif

#if defined(CONFIG_BCM_XTMCFG) || defined(CONFIG_BCM_XTMCFG_MODULE)
    #define _CONFIG_BCM_XTMCFG
#endif

#if defined(CONFIG_BCM_VOICE_SUPPORT)
    #define _CONFIG_BCM_VOICE_SUPPORT
#endif

#if defined(CONFIG_BCM_GPON_STACK_MODULE)
#define ENET_GPON_CONFIG /* GPON support in Ethernet driver */
#endif

#if defined(CONFIG_BCM_EPON_STACK_MODULE)
#define ENET_EPON_CONFIG /* EPON support in Ethernet driver */
#endif

#ifndef FAP_4KE
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include "boardparms.h"
#include "bcm_OS_Deps.h"
#include <linux/bcm_log.h>
#include <bcmnet.h>
#include <bcm/bcmswapitypes.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#endif

#include "bcmtypes.h"

#if defined(CONFIG_BCM_MAX_GEM_PORTS)
#define MAX_MARK_VALUES   32
#define MAX_GEM_IDS       CONFIG_BCM_MAX_GEM_PORTS
#define MAX_GPON_IFS      40
/* The bits[0:6] of status field in DmaDesc are Rx Gem ID. For now, we are
   using only 5 bits */
#define RX_GEM_ID_MASK    0x7F
#endif

#define BCM_PORT_FROM_TYPE2_TAG(tag) (tag & 0x1f) // Big Endian
#define BCM_PORT_FROM_TYPE2_TAG_LE(tag) ((tag >> 8) & 0x1f) // Little Endian

#define VLAN_TYPE           0x8100
typedef struct {
    unsigned char da[6];
    unsigned char sa[6];
    uint16 brcm_type;
    uint32 brcm_tag;
    uint16 vlan_proto;
    uint16 vlan_TCI;
    uint16 encap_proto;
} __attribute__((packed)) BcmVlan_ethhdr;

typedef struct {
    unsigned char da[6];
    unsigned char sa[6];
    uint16 brcm_type;
    uint16 brcm_tag;
    uint16 vlan_proto;
    uint16 vlan_TCI;
    uint16 encap_proto;
} __attribute__((packed)) BcmVlan_ethhdr2;

typedef struct extsw_info_s {
    unsigned int switch_id;
    int brcm_tag_type;
    int accessType;
    int page;
    int bus_num;
    int spi_ss;
    int spi_cid;
    int present;
    int connected_to_internalPort;
} extsw_info_t;

#define BCM_ENET_IFG        20 /* bytes */
#define BCM_ENET_CRC_LEN    4  /* bytes */
#define BCM_ENET_OVERHEAD   (BCM_ENET_CRC_LEN + BCM_ENET_IFG) /* bytes */

#define BRCM_TYPE2               0x888A // Big Endian
#define BRCM_TYPE2_LE            0x8A88 // Little Endian
#define BRCM_TAG_TYPE2_LEN       4
#define BRCM_TAG2_EGRESS         0x2000
#define BRCM_TAG2_EGRESS_TC_MASK 0x1c00

#if defined(BCM_ENET_CB_WAN_PORT_LNX_INTF_SUPPORT)
#define MAX_NUM_OF_VPORTS   10
#else
#define MAX_NUM_OF_VPORTS   8
#endif
#define BRCM_TAG_LEN        6
#define BRCM_TYPE           0x8874
#define BRCM_TAG_UNICAST    0x00000000
#define BRCM_TAG_MULTICAST  0x20000000
#define BRCM_TAG_EGRESS     0x40000000
#define BRCM_TAG_INGRESS    0x60000000

typedef struct {
    unsigned char da[6];
    unsigned char sa[6];
    uint16 brcm_type;
    uint32 brcm_tag;
    uint16 encap_proto;
} __attribute__((packed)) BcmEnet_hdr;

typedef struct {
    unsigned char da[6];
    unsigned char sa[6];
    uint16 brcm_type;
    uint16 brcm_tag;
    uint16 encap_proto;
} __attribute__((packed)) BcmEnet_hdr2;

/* IEEE 802.3 Ethernet constant */
#define ETH_CRC_LEN             4
#define ETH_MULTICAST_BIT       0x01

#ifndef ERROR
#define ERROR(x)        printk x
#endif
#ifndef ASSERT
#define ASSERT(x)       if (x); else ERROR(("assert: "__FILE__" line %d\n", __LINE__)); 
#endif

#if defined(DUMP_TRACE)
#define TRACE(x)        printk x
#else
#define TRACE(x)
#endif

/* 
    This is general guide of return code across
    Ethernet hardware dependent API. The exact meaning
    of code set depend on each API local context.
*/
  
enum {
    BCMEAPI_CTRL_CONTINUE,  /* Return code to continue execution or loop */
    BCMEAPI_CTRL_SKIP,      /* Return code to skip current execution */
    BCMEAPI_CTRL_BREAK,     /* Return code to break current loop */
    BCMEAPI_CTRL_TRUE,      /* Return code for TRUE result */
    BCMEAPI_CTRL_FALSE,     /* Return code for FALSE result */
};

/*
    The flag which could be set in high bits
    in API return code if needed.
*/
enum {
    BCMEAPI_CTRL_FLAG_TRUE = (1<<31),       /* The TRUE bit */
    BCMEAPI_CTRL_FLAG_MASK = BCMEAPI_CTRL_FLAG_TRUE,    /* The bit mask for all return flag bits */
};

typedef struct emac_pm_addr_t {
    BOOL                valid;          /* 1 indicates the corresponding address is valid */
    unsigned int        ref;            /* reference count */
    unsigned char       dAddr[ETH_ALEN];/* perfect match register's destination address */
    char                unused[2];      /* pad */
} emac_pm_addr_t;                    
#define MAX_PMADDR          4           /* # of perfect match address */

#if defined(ENET_GPON_CONFIG)
#define BcmEnetDevGponBaseClass_s struct { \
    /* For gpon virtual interfaces */ \
    int gem_count;       /* Number of gem ids */ \
    int gponifid;   /* Unique ifindex in [0:31] for gpon virtual interface  */ \
}
#else
#define BcmEnetDevGponBaseClass_s
#endif

typedef struct enet_device_stats
{
    unsigned long   rx_packets_queue[8];
	unsigned long	rx_dropped_no_rxdev;
	unsigned long	rx_dropped_blog_drop;
	unsigned long	rx_dropped_no_skb;
	unsigned long	rx_packets_blog_done;
	unsigned long	rx_dropped_skb_headinit;
	unsigned long	rx_packets_netif_receive_skb;
	unsigned long	rx_errors_indicated_by_low_level;
	unsigned long	rx_dropped_undersize;
	unsigned long	rx_dropped_overrate;
	unsigned long	tx_dropped_bad_nbuff;
	unsigned long	tx_dropped_no_lowlvl_resource;
	unsigned long	tx_dropped_no_fkb;
	unsigned long	tx_dropped_no_skb;
	unsigned long	tx_dropped_no_gem_ids;
	unsigned long	tx_dropped_bad_gem_id;
	unsigned long	tx_dropped_misaligned_nbuff;
    unsigned long   tx_packets_queue_in[8];
    unsigned long   tx_packets_queue_out[8];
    unsigned long   tx_drops_no_valid_gem_fun;
    unsigned long   tx_drops_skb_linearize_error;
    unsigned long   tx_dropped_runner_lan_fail;
    unsigned long   tx_dropped_runner_wan_fail;
    unsigned long   tx_dropped_no_gso_dsc;
    unsigned long   tx_dropped_sid_tx_fail;
    unsigned long   tx_dropped_no_rdpa_port_mapped;
    unsigned long   tx_dropped_no_gem_tcount;
    unsigned long   tx_dropped_no_epon_tx_fun;
    unsigned long   tx_dropped_no_epon_oam_fun;
    unsigned long   tx_dropped_gpon_tx_fail;
    unsigned long   tx_dropped_epon_tx_fail;
    unsigned long   tx_dropped_epon_oam_fail;
    unsigned long   tx_dropped_xpon_lan_fail;
} enet_device_stats;

#define BcmEnetDevctrlBaseClass_s \
struct { \
    struct net_device *dev;             /* ptr to net_device */ \
    struct net_device *next_dev;        /* next device */ \
    struct rtnl_link_stats64 stats;     /* statistics used by the kernel */ \
    struct rtnl_link_stats64 cHwStats;  /* Cummulative HW stats */ \
    struct rtnl_link_stats64 lHwStats;  /* Last HW stats snapshot */ \
    struct enet_device_stats estats;    /* statistics used by Ethernet driver */ \
    struct tasklet_struct task;         /* tasklet */ \
    int             linkState;          /* link status */ \
    int             lanUpPorts;         /* Link Up LAN Port number */ \
    int             wanUpPorts;         /* Link Up WAN Port number */ \
    /* Port attribute reported to upper layer */ \
    int             wanPort;            /* wan port selection */ \
    int             wanOnlyPorts;       /* WAN Only port bit map */ \
    int             wanPrefPorts;       /* WAN Preferred port bit map */ \
    int             lanOnlyPorts;       /* LAN Only port bit map */ \
    int             allPortMap;         /* All port bit map */ \
    int             wanLanCapPorts;     /* WAN/LAN both capable port bit map */ \
    /* Port attribute restricted by Chip architecture */ \
    int             wanOnlyPortChip;   /* WAN Only port bit map */ \
    int             wanPrefPortChip;   /* WAN Preferred port bit map */ \
    int             lanOnlyPortChip;   /* LAN Only port bit map */ \
    /* Port attribute defined by boardparameters */ \
    int             wanOnlyPortDef;    /* WAN Only port bit map */ \
    int             wanPrefPortDef;    /* WAN Preferred port bit map */ \
    int             lanOnlyPortDef;    /* LAN Only port bit map */ \
    int             unit;               /* device control index */ \
    unsigned int    vid; \
    uint16          chipId;             /* chip's id */ \
    uint16          chipRev;            /* step */ \
 \
    spinlock_t ethlock_tx; \
    spinlock_t ethlock_moca_tx; \
    spinlock_t ethlock_rx; \
    spinlock_t ethlock_skblist; \
 \
    emac_pm_addr_t  pmAddr[MAX_PMADDR]; /* perfect match address */ \
    extsw_info_t  *extSwitch;          /* external switch */ \
    ETHERNET_MAC_INFO EnetInfo[BP_MAX_ENET_MACS]; \
    IOCTL_MIB_INFO MibInfo; \
 \
    int sw_port_id; /* Logical Port Number */ \
    int vport_id;   /* Unique id of virtual eth interface */ \
    int cb_ext_port; /* Crossbar external port; BP_CROSSBAR_NOT_DEFINED if not applicable */ \
 \
    int default_txq; \
    int use_default_txq; \
 \
    struct task_struct *rx_thread; \
    wait_queue_head_t   rx_thread_wqh; \
    int                 rx_work_avail; \
    struct napi_struct napi; /* obsolete: used in 2.6.30 kernel */ \
 \
    int eee_enable_request_flag[2]; \
    BcmEnetDevGponBaseClass_s; \
\
    int softSwitchingMap;        /* port map of LAN ports using MIPS for RX and TX */ \
    int stpDisabledPortMap;      /* port map of ports with HW stp disabled */ \
    int learningDisabledPortMap; /* port map of ports with learning disabled */ \
    int enetLinkHandlePmap;      /* port map excluded from SWMDK PHY management */ \
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 1)
#define BCMENET_WAKEUP_RXWORKER(x) do { \
           if ((x)->rx_work_avail == 0) { \
               (x)->rx_work_avail = 1; \
               wake_up_interruptible(&((x)->rx_thread_wqh)); }} while (0)
#else
#define BCMENET_WAKEUP_RXWORKER(x) napi_schedule(&((x)->napi))
#endif

typedef struct BcmEnet_devctrl BcmEnet_devctrl;
typedef struct enet_xmit_params EnetXmitParams;

#define enet_xmit_params_base struct { \
    unsigned int len; \
    unsigned int mark; \
    unsigned int drop_eligible; \
    unsigned int priority; \
    unsigned int r_flags; \
    int egress_queue; \
    uint16 port_id; \
    uint16 lag_port; \
    uint8 * data; \
    BcmEnet_devctrl *pDevPriv; \
    struct rtnl_link_stats64 *vstats; \
    uint32_t blog_chnl, blog_phy;       /* used if CONFIG_BLOG enabled */ \
    int gemid; \
    int is_chained; \
    pNBuff_t pNBuff; \
}

/* Below macros should control the logical switch port number mapping.
 * Currently Internal switch port numbers are 0..7
 * External switch port numbers are 8..15
 * The above numbering is fixed irrespective of compile or run time determination of EXT_SW
 * It is assumed that both internal and external switch have max 8 ports
 */

#define SW_PORT_M   (0x07)
#define SW_PORT_S   (0)
#define SW_UNIT_M   (0x08)
#define SW_UNIT_S   (3)

#define SW_PORTMAP_M (0xFF)
#define SW_PORTMAP_S (0)
#define SW_PORTMAP_EXT_M (0xFF00)
#define SW_PORTMAP_EXT_S (8)

#if defined(CONFIG_BCM_EXT_SWITCH)

#define LOGICAL_PORT_TO_UNIT_NUMBER(port)   ( ((port) & SW_UNIT_M) >> SW_UNIT_S  )
#define LOGICAL_PORT_TO_PHYSICAL_PORT(port) ( (port) & SW_PORT_M )
#define PHYSICAL_PORT_TO_LOGICAL_PORT(port, unit) ( (port) | ((unit << SW_UNIT_S) & SW_UNIT_M) )

#define LOGICAL_PORTMAP_TO_PHYSICAL_PORTMAP(pmap) ( ((pmap) & SW_PORTMAP_EXT_M) ? ( ((pmap) & SW_PORTMAP_EXT_M) >> SW_PORTMAP_EXT_S ) : ((pmap) & SW_PORTMAP_M) )
#define PHYSICAL_PORTMAP_TO_LOGICAL_PORTMAP(pmap, unit) ( ((pmap) & SW_PORTMAP_M) << (unit*SW_PORTMAP_EXT_S) )

#define SET_PORT_IN_LOGICAL_PORTMAP(port, unit)  ( 1 << ( ((port) & SW_PORT_M) + ((unit)*SW_PORTMAP_EXT_S)) )
#define SET_PORTMAP_IN_LOGICAL_PORTMAP(pmap, unit)  ( ((pmap) & SW_PORTMAP_M) << ((unit)*SW_PORTMAP_EXT_S) )

#else  /* No external switch compiled-in */

#define LOGICAL_PORT_TO_UNIT_NUMBER(port)   (0)
#define LOGICAL_PORT_TO_PHYSICAL_PORT(port) ( (port) & SW_PORT_M )
#define PHYSICAL_PORT_TO_LOGICAL_PORT(port, unit) ( (port) & SW_PORT_M )
#define LOGICAL_PORTMAP_TO_PHYSICAL_PORTMAP(pmap) ( (pmap) & SW_PORTMAP_M )
#define PHYSICAL_PORTMAP_TO_LOGICAL_PORTMAP(pmap, unit) ( (pmap) & SW_PORTMAP_M )
#define SET_PORT_IN_LOGICAL_PORTMAP(port, unit)  ( 1 << ((port) & SW_PORT_M) )
#define SET_PORTMAP_IN_LOGICAL_PORTMAP(pmap, unit)  ( (pmap) & SW_PORTMAP_M )

#endif /* No External switch */

#define GET_PORTMAP_FROM_LOGICAL_PORTMAP(pmap, unit)  ( (((pmap) >> ((unit)*SW_PORTMAP_EXT_S)) & SW_PORTMAP_M) )

#define MAX_SWITCH_UNITS 2

#define IsExternalSwitchUnit(unit)  (unit > 0)
#define IsExternalSwitchPort(port)   IsExternalSwitchUnit(LOGICAL_PORT_TO_UNIT_NUMBER(port))

/* To find out if if the provided port connects internal switch/runner to external switch */
#define IsInterSwitchPort(unit, port) (!IsExternalSwitchUnit(unit) && \
                            (EnetGetEthernetMacInfo()[unit].sw.phy_id[port] & EXTSW_CONNECTED))

#define LOGICAL_INT_PORT_START  (0)
#define LOGICAL_INT_PORT_END    (LOGICAL_INT_PORT_START + MAX_SWITCH_PORTS - 1)

#define LOGICAL_EXT_PORT_START  (LOGICAL_INT_PORT_END+1)
#define LOGICAL_EXT_PORT_END    (LOGICAL_EXT_PORT_START + MAX_SWITCH_PORTS - 1)


#define LOGICAL_PORT_START      LOGICAL_INT_PORT_START

#if defined(CONFIG_BCM_EXT_SWITCH)
#define LOGICAL_PORT_END        LOGICAL_EXT_PORT_END
#else
#define LOGICAL_PORT_END        LOGICAL_INT_PORT_END
#endif
extern BcmEnet_devctrl *pVnetDev0_g;
int enet_logport_to_phyid(int log_port);
int enet_sw_port_to_phyid(int unit, int phy_port);
int enet_cb_port_to_phyid(int unit, int cb_port);
int enet_cb_port_has_combo_phy(int unit, int cb_port);
void enet_cb_port_get_phyids(int unit, int cb_port, int *phyId, int *phyIdxExt);
int enet_phyid_get_cbport(int unit, int port, int phyId, int *cbport);
#define IsPortRgmiiDirect(unit, port) \
    IsRgmiiDirect(enet_sw_port_to_phyid(unit, port))
#define IsPortPhyConnected(unit, port) \
    IsPhyConnected(enet_sw_port_to_phyid(unit, port))
unsigned long enet_get_portmap(unsigned char unit);
unsigned long enet_get_port_flags(unsigned char unit, int port);
unsigned int enet_get_consolidated_portmap(void);
int enet_get_total_ports_num(void);
void enet_hex_dump(void *mem, unsigned int len);

extern struct kmem_cache *enetSkbCache;
#endif /* _BCMENET_COMMON_H_ */
