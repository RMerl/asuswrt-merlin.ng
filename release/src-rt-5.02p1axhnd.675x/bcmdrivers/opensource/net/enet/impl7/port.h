/*
   <:copyright-BRCM:2015:DUAL/GPL:standard
   
      Copyright (c) 2015 Broadcom 
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
 *  Created on: Nov/2015
 *      Author: ido@broadcom.com
 */

#ifndef _PORT_H_
#define _PORT_H_

#include <linux/netdevice.h>
#include <linux/nbuff.h>
#include <phy_drv.h>
#include <mac_drv.h>
#include "enet_types.h"
#include "enet_dbg.h"
#include "port_types.h"

#if defined(CONFIG_NET_SWITCHDEV)
#include <net/switchdev.h>
#endif

typedef enum
{
    PORT_CLASS_SW = 0x1,
    PORT_CLASS_PORT = 0x2,
    /* Used as port place holders. Type can be set at later and initialized accordingly */
    PORT_CLASS_PORT_DETECT = 0x4,
} port_class_t;

typedef enum
{
    PORT_CAP_NONE,
    PORT_CAP_MGMT,
    PORT_CAP_LAN_WAN,
    PORT_CAP_LAN_ONLY,
    PORT_CAP_WAN_ONLY,
    PORT_CAP_WAN_PREFERRED,
    PORT_CAP_MAX,
} port_cap_t;

typedef enum
{
    PORT_NETDEV_ROLE_NONE,
    PORT_NETDEV_ROLE_LAN,
    PORT_NETDEV_ROLE_WAN,
} port_netdev_role_t;

#define DEBUG_STATS 

#define INC_STAT_RX_PKT_BYTES(port,len)     \
            { (port)->n.port_stats.rx_packets++; (port)->n.port_stats.rx_bytes+=(len); }
#define INC_STAT_TX_PKT_BYTES(port,len)     \
            { (port)->n.port_stats.tx_packets++; (port)->n.port_stats.tx_bytes+=(len); }

#define INC_STAT_RX_MCAST_PKT_BYTES(port,len)     \
            { (port)->n.port_stats.rx_mcast_packets++; (port)->n.port_stats.rx_mcast_bytes+=(len); }
#define INC_STAT_TX_MCAST_PKT_BYTES(port,len)     \
            { (port)->n.port_stats.tx_mcast_packets++; (port)->n.port_stats.tx_mcast_bytes+=(len); }
#define INC_STAT_RX_BCAST_PKT_BYTES(port,len)     \
            { (port)->n.port_stats.rx_bcast_packets++; (port)->n.port_stats.rx_bcast_bytes+=(len); }
#define INC_STAT_TX_BCAST_PKT_BYTES(port,len)     \
            { (port)->n.port_stats.tx_bcast_packets++; (port)->n.port_stats.tx_bcast_bytes+=(len); }

#if defined(DEBUG_STATS)
    #define MAX_NUM_OF_PRI_QS   8
    #define INC_STAT_RX_DROP(port,counter)  \
            { (port)->n.port_stats.rx_dropped++; (port)->n.port_stats.counter++; }
    #define INC_STAT_TX_DROP(port,counter)  \
            { (port)->n.port_stats.tx_dropped++; (port)->n.port_stats.counter++; }
    #define INC_STAT_DBG(port,counter)           (port)->n.port_stats.counter++
    #define INC_STAT_RX_Q(port,queue)            (port)->n.port_stats.rx_packets_queue[(queue)]++
    #define INC_STAT_TX_Q_IN(port,queue)         (port)->n.port_stats.tx_packets_queue_in[(queue)]++
    #define INC_STAT_TX_Q_OUT(port,queue)        (port)->n.port_stats.tx_packets_queue_out[(queue)]++
#else
    #define INC_STAT_RX_DROP(port,counter)       (port)->n.port_stats.rx_dropped++
    #define INC_STAT_TX_DROP(port,counter)       (port)->n.port_stats.tx_dropped++
    #define INC_STAT_DBG(port,counter)           {}
    #define INC_STAT_RX_Q(port,queue)            {}
    #define INC_STAT_TX_Q_IN(port,queue)         {}
    #define INC_STAT_TX_Q_OUT(port,queue)        {}
#endif

typedef struct
{
    // following counters are subset of struct net_device_stats
    unsigned long rx_packets;
    unsigned long tx_packets;
    unsigned long rx_bytes;
    unsigned long tx_bytes;
    unsigned long rx_dropped;
    unsigned long tx_dropped;

    unsigned long rx_mcast_packets;
    unsigned long tx_mcast_packets;
    unsigned long rx_mcast_bytes;
    unsigned long tx_mcast_bytes;
    unsigned long rx_bcast_packets;
    unsigned long tx_bcast_packets;
    unsigned long rx_bcast_bytes;
    unsigned long tx_bcast_bytes;

#if defined(DEBUG_STATS)
    // following are extended counters for debugging
    unsigned long rx_dropped_no_skb;
    unsigned long rx_dropped_blog_drop;
    unsigned long rx_dropped_no_rxdev;
    unsigned long rx_dropped_no_srcport;

    unsigned long rx_packets_blog_done;
    unsigned long rx_packets_netif_receive_skb;

    unsigned long rx_packets_queue[MAX_NUM_OF_PRI_QS];

    unsigned long tx_dropped_dispatch;
    unsigned long tx_dropped_mux_failed;
	unsigned long tx_dropped_bad_nbuff;
	unsigned long tx_dropped_no_lowlvl_resource;    /* not used? */
	unsigned long tx_dropped_no_fkb;
	unsigned long tx_dropped_no_skb;
    unsigned long tx_dropped_accelerator_lan_fail;
    unsigned long tx_dropped_accelerator_wan_fail;

    unsigned long tx_packets_queue_in[MAX_NUM_OF_PRI_QS];
    unsigned long tx_packets_queue_out[MAX_NUM_OF_PRI_QS];
#endif /* DEBUG_STATS */    
} port_stats_t;

#define MAX_CB_PHYS_PER_PORT       4

/* Note : Switch only supports two trunk groups, so use separate defines if Software needs more than two group */
// TODO_DSL: 158 support 4, need to move this constant to *_map_part.h ?
#define MAX_KERNEL_BONDING_GROUPS   2

typedef struct
{
    uint32_t is_lan_wan_cfg  :1;  /* Is the LAN-WAN configuration applied to the group */
    uint32_t port_count      :3;
    uint32_t grp_idx         :3;
    uint32_t reserved        :25;
    uint32_t blog_chnl_rx;        /* RX Blog Channel value for this bonding group ; Required so packet from multiple bonded RX ports can hit the same flow*/
    struct enetx_port_t *lan_wan_port;
    struct net_device *bond_dev;
}bond_info_t;

/* Port info passed on to port_create so parent switch can determine port type and index.
 * Required fields for each sw are dependant per implementation */
typedef struct port_info_t
{
    int port;
    int is_management;
    int is_attached;
    int is_detect;
    int is_gpon;
    int is_epon;
    int is_undef;
    int is_wan;
} port_info_t;

typedef struct enetx_port_t
{
    /* SW / PORT object */
    port_class_t port_class;

    /* Pre-init config options: */
    /* Should a Linux if be created for this port */
    int has_interface;
    /* Force net_device name */
    char name[IFNAMSIZ];
    /* Set by port_init() function */
    int is_init;

    /* TODO: for debug, can be removed later */
    char obj_name[IFNAMSIZ];

    struct net_device *dev;

    /* Pointer to private data */
    void *priv;

    /* Port type enumeration */
    port_type_t port_type;

    /* Copy of port_info, used by detect */
    port_info_t port_info;

    union
    {
        struct
        {
            /* mac, phy must be set before calling port_init() */
            /* Mac port object to perform operations such as read statistics and set speed */
            mac_dev_t *mac;
            /* Phy port object to perform operations such as read status and eee enable */
            phy_dev_t *phy;
            /* 1/100 of a second from system uptime, updated at PHY link status change */
            uint32_t phy_last_change;
            /* Defaults to 1; when set, enet will handle phy link changes */
            int handle_phy_link_change;

            /* Physical device identifier on switch, used for muxing */
            int port_id; /* i.e: rdpaif type */

            /* Role this port is allowed to act as */
            port_cap_t port_cap;
            /* port operations */
            struct port_ops_t *ops;
            /* Link to parent/child SW */
            struct enetx_port_t *parent_sw; /* set by port_create */
            struct enetx_port_t *child_sw;  /* set by init operation of switch */

            /* mux_index: cached callback */
            int (*mux_port_tx)(struct enetx_port_t *tx_port, pNBuff_t pNBuff, struct enetx_port_t **out_port);

            bond_info_t *bond_grp;
        } p;
        struct
        {
            /* semaphore for switch configuration */
            struct semaphore conf_sem;
            /* switch ops */
            struct sw_ops_t *ops;
            /* Link to parent port */
            struct enetx_port_t *parent_port; /* set by switch_create */
            /* Dynamically allocated list of ports on SW */
            struct enetx_port_t **ports;
            int port_count;

            /* mux_index: Support for index based mapping (de/)mux capabilities */
            int mux_count;
            /* child->port_id -> enetx_port_t map, register here with mux_set_tx_index() */
            struct enetx_port_t **mux_map;
            int demux_count;
            /* parent->port_id -> enetx_port_t map, register here with mux_set_rx_index() */
            struct enetx_port_t **demux_map;

            /* TODO_DSL? need to init, only root_switch is used, need to complete STP implementation */
            int stpDisabledPortMap;
        } s;
    };
    struct
    {
        /* When active, RX/TX will set channel to/from skb mark */
        int set_channel_in_mark;
        port_stats_t port_stats; /* TODO: Should this exist only when netdev ? or also when netdev doesn't exist ? */
#ifdef CONFIG_BLOG
        /* Blog channel, phy, set on port init, used in RX/TX */
        int blog_phy;
        int blog_chnl;
        int blog_chnl_rx;   /* for port in kernel_bonding group, lowest blog_chnl is used */
#endif
        port_netdev_role_t port_netdev_role;
#if defined(CONFIG_NET_SWITCHDEV)
        uint32_t flags;
#define     PORT_FLAG_HW_FWD    (1<<0)  /* for switch indicate switchwide behavior */
#endif // CONFIG_NET_SWITCHDEV
    } n;
} enetx_port_t;
 
/* dispatch_pkt() information. */
typedef struct dispatch_info_t
{
    pNBuff_t pNBuff;
    enetx_port_t *port;
    int channel;
    int egress_queue;
    unsigned int drop_eligible;
    uint16_t lag_port;  /* for multiple connections to external switch */
    int no_lock;
} dispatch_info_t;

typedef struct port_ops_t
{
    /* HW related called by port_init triggered by sw_init */
    int (*init)(enetx_port_t *self);
    int (*uninit)(enetx_port_t *self);
    int (*dispatch_pkt)(dispatch_info_t *dispatch_info);
    /* Will be called by .ndo_get_stats, should collect statistics (eg: from MAC/other HW) */
    void (*stats_get)(enetx_port_t *self, struct rtnl_link_stats64 *net_stats);
    void (*stats_clear)(enetx_port_t *self);
    int (*pause_get)(enetx_port_t *port, int *rx_enable, int *tx_enable);
    int (*pause_set)(enetx_port_t *port, int rx_enable, int tx_enable);
    /* Called from .ndo_open() */
    void (*open)(enetx_port_t *self);
    /* Called from .ndo_stop() */
    void (*stop)(enetx_port_t *self);
    /* Called when port role is changed, first time initialization according to role should be done at init() */
    int (*role_set)(enetx_port_t *self, port_netdev_role_t role);
    int (*mtu_set)(enetx_port_t *self, int mtu);
    /* configure port spanning tree mode and state */
    int (*stp_set)(enetx_port_t *self, int mode, int state);
        #define STP_MODE_DISABLE    0
        #define STP_MODE_ENABLE     1
        #define STP_MODE_UNCHANGED  -1
        #define STP_STATE_UNCHANGED -1
    /* tx adjust priority q based on external switch queue remapping */ 
    uint32_t (*tx_q_remap)(enetx_port_t *port, uint32_t txq);
    /* tx load balancing select which IMP port to use when CONFIG_BCM_ENET_MULTI_IMP_SUPPORT is supported */
    uint16_t (*tx_lb_imp)(enetx_port_t *port, uint16_t port_id, void* pHdr);
    /* for SF2 based external switch adding BRCM tag when tx */
    int (*tx_pkt_mod)(enetx_port_t *port, pNBuff_t *pNBuff, uint8_t **data, uint32_t *len, unsigned int port_map);
    /* for SF2 based external switch removing BRCM tag when rx */
    int (*rx_pkt_mod)(enetx_port_t *port, struct sk_buff *skb);
    int (*mib_dump)(enetx_port_t *self, int all);   /* ETHSWDUMPMIB */
    void (*print_status)(enetx_port_t *self);
    /* fast age ARL for port */
    void (*fast_age)(enetx_port_t *port);
    /* Return private data string for proc interface */
    char *(*print_priv)(enetx_port_t *port);
    /* Will be called when link status changes */
    int (*link_change)(enetx_port_t *self, int up);
    int (*pfc_set)(enetx_port_t *port, int pfc_rx_enable, int pfc_tx_enable);
    int (*pfc_get)(enetx_port_t *port, int *pfc_rx_enable, int *pfc_tx_enable);
    int (*pfc_tx_timer_set)(enetx_port_t *port, int priority, int pfc_tx_timer);
    int (*pfc_tx_timer_get)(enetx_port_t *port, int priority, int *pfc_tx_timer);

#if defined(CONFIG_NET_SWITCHDEV)
    struct switchdev_ops switchdev_ops;
#endif
    int (*mib_dump_us)(enetx_port_t *self, void *e);   /* ETHSWMIBDUMP */  // add by Andrew
} port_ops_t;

typedef struct sw_ops_t
{
    /* Initialize switch HW - called every time SW is enslaved to port */
    int (*init)(enetx_port_t *self);
    int (*uninit)(enetx_port_t *self);
    /* Will be called by .ndo_get_stats, should collect sw statistics (eg: total underlying ports) */
    void (*stats_get)(enetx_port_t *self, struct rtnl_link_stats64 *net_stats);
    void (*stats_clear)(enetx_port_t *self);
    /* Used by port_create(), should return the sw designated port_id and port type according to port_info
     * retval positive to skip port creation */
    int (*port_id_on_sw)(port_info_t *port_info, int *port_id, port_type_t *port_type);

    /* mux_index: Support for index based mapping (de/)mux capabilities */
    /* Will be cached and called from port object for performance */
    /* Demux rx_port and fkb on sw to out_port */
    /* called with rcu_read_lock() */
    int (*mux_port_rx)(enetx_port_t *sw, enetx_rx_info_t *rx_info, FkBuff_t *fkb, enetx_port_t **out_port);
    /* Mux tx_port->port_id and fkb on tx_port->parent_sw to out_port */
    int (*mux_port_tx)(enetx_port_t *tx_port, pNBuff_t pNBuff, enetx_port_t **out_port);
    void (*mux_free)(struct enetx_port_t *sw);

    /* hardware switching configuration ETHSWSWITCHING */
    int (*hw_sw_state_set)(enetx_port_t *sw, unsigned long state);
        #define HW_SWITCHING_ENABLED    1
        #define HW_SWITCHING_DISABLED   0
    int (*hw_sw_state_get)(enetx_port_t *sw);

    /* configure trunk group */
    int (*config_trunk)(enetx_port_t *sw, enetx_port_t *port, int grp_no, int add);
    /* configure pbvlan in hw */
    int (*update_pbvlan)(enetx_port_t *sw, unsigned int pmap);

    /* access switch registers */
    void (*rreg)(int page, int reg, void *data_out, int len);
    void (*wreg)(int page, int reg, void *data_in,  int len);
    /* fast ager ARL for entire switch */
    void (*fast_age)(enetx_port_t *sw);
    /* Return private data string for proc interface */
    char *(*print_priv)(enetx_port_t *port);
} sw_ops_t;

#if defined(CONFIG_NET_SWITCHDEV)
#define PORT_SET_HW_FWD(p)      (p)->n.flags |= PORT_FLAG_HW_FWD
#define PORT_CLR_HW_FWD(p)      (p)->n.flags &= ~PORT_FLAG_HW_FWD
#define PORT_IS_HW_FWD(p)       ((p)->n.flags & PORT_FLAG_HW_FWD)
#define PORT_SET_EXT_SW(p)
#else
#define PORT_SET_HW_FWD(p)      (p)->dev->priv_flags |= IFF_HW_SWITCH
#define PORT_CLR_HW_FWD(p)      (p)->dev->priv_flags &= ~IFF_HW_SWITCH
#define PORT_IS_HW_FWD(p)       ((p)->dev->priv_flags & IFF_HW_SWITCH)
#define PORT_SET_EXT_SW(p)      if ((p)->dev) (p)->dev->priv_flags |= IFF_EXT_SWITCH
#endif

/* Functions used to create port tree */
/* Create switch object */
enetx_port_t *sw_create(port_type_t type, enetx_port_t *parent_port);
/* Creates port object: calls port_id_on_sw() of parent_sw to select port_type based on port_info;
 * returns positive number if port_id_on_sw() decides not to create a port */
int port_create(port_info_t *port_info, enetx_port_t *parent_sw, enetx_port_t **port);
/* Start initialization sequence for switch sw and underlying port tree */
int sw_init(enetx_port_t *sw);
int port_init(enetx_port_t *port);
/* Uninitialize and free switch (or port) and its underlying objects */
void sw_free(enetx_port_t **sw);
/* Initializes port from class detect */
int port_activate(enetx_port_t *port);
/* Uninitializes port to class detect */
int port_deactivate(enetx_port_t *port);

/* generic PORT_CLASS_PORT ops */
void port_generic_open(enetx_port_t *self);
void port_generic_stop(enetx_port_t *self);
int port_generic_mtu_set(enetx_port_t *self, int mtu);
void port_generic_stats_get(enetx_port_t *self, struct rtnl_link_stats64 *net_stats);
void port_generic_sw_stats_get(enetx_port_t *self, struct rtnl_link_stats64 *net_stats);
int port_generic_pause_set(enetx_port_t *self, int rx_enable, int tx_enable);
int port_generic_pause_get(enetx_port_t *self, int *rx_enable, int *tx_enable);

/* root switch */
extern enetx_port_t *root_sw;
#define PORT_ON_ROOT_SW(port)   ((port)->p.parent_sw == root_sw)
#define IS_ROOT_SW(sw)          ((sw) == root_sw)

#include "boardparms.h"     // TODO_DSL: remove unit_port_array to remove BP_MAX_ENET_MACS
#define COMPAT_MAX_SWITCH_PORTS 24
extern enetx_port_t *unit_port_array[BP_MAX_ENET_MACS][COMPAT_MAX_SWITCH_PORTS]; 

extern uint32_t g_enet_flags;
#define ENET_FLAG_IF_PHY_PWR_SYNC_DISABLE    (1UL << 0)  // don't synchronize PHY power setting based on interface state

typedef int (*port_traverse_cb)(enetx_port_t *port, void *ctx);
/* Helper function which recursively traverses 'port', calls fn_port(p, ctx) on each PORT_CLASS_PORT,
   and fn_sw(p, ctx) on each PORT_CLASS_SW; Callback should return 0 to continue to next port,
   < 0 to halt on error, > 0 to stop without returning error.
   max_sw_depth is the number of sw recursians it will make, -1 for all, 1 for current sw only */
int _port_traverse_ports(enetx_port_t *port, port_traverse_cb fn, port_class_t classes, void *ctx, int max_sw_depth);
#define port_traverse_ports(port, fn, classes, ctx) _port_traverse_ports(port, fn, classes, ctx, -1)

int port_netdev_role_set(enetx_port_t *self, port_netdev_role_t role);
#define PORT_ROLE_IS_WAN(port) ((port)->n.port_netdev_role == PORT_NETDEV_ROLE_WAN)

#define PORT_SHARED_OPS_EXIST(port, opcmd) \
    (((port->port_class & (PORT_CLASS_PORT)) && port->p.ops->opcmd) || \
        (port->port_class == PORT_CLASS_SW && port->s.ops->opcmd))

#define _PORT_SHARED_OPS(ret, port, opcmd) \
    do { \
        if (port->port_class & (PORT_CLASS_PORT)) \
            ret port->p.ops->opcmd; \
        else if (port->port_class == PORT_CLASS_SW) \
            ret port->s.ops->opcmd; \
    } while (0)

#define PORT_SHARED_OPS(port, opcmd) _PORT_SHARED_OPS(, port, opcmd)
#define PORT_SHARED_OPS_RET(rc, port, opcmd) _PORT_SHARED_OPS(rc =, port, opcmd)
            
static inline int port_stats_get(enetx_port_t *port, struct rtnl_link_stats64 *net_stats)
{
    if (!PORT_SHARED_OPS_EXIST(port, stats_get))
    {
        enet_dbg("no stats capability: %s\n", port->obj_name);
        return -1;
    }

    PORT_SHARED_OPS(port, stats_get(port, net_stats));
    return 0;
}

static inline void port_generic_stats_clear(enetx_port_t *port)
{
    if (!(port->port_class & (PORT_CLASS_PORT)))
        return;

    if (port->p.mac)
        mac_dev_stats_clear(port->p.mac);
}

static inline int port_stats_clear(enetx_port_t *port)
{
    enet_dbg("port stats clear %s\n", port->obj_name);

    memset(&port->n.port_stats, 0, sizeof(port->n.port_stats));

    if (!PORT_SHARED_OPS_EXIST(port, stats_clear))
    {
        enet_dbg("no stats capability: %s\n", port->obj_name);
        return -1;
    }

    PORT_SHARED_OPS(port, stats_clear(port));
    return 0;
}

static inline int port_pfc_capable(enetx_port_t *port)
{
    return port->p.ops->pfc_get ? 1 : 0;
}

static inline int port_pfc_get(enetx_port_t *port, int *pfc_rx_enable, int *pfc_tx_enable)
{
	if (port->port_class != PORT_CLASS_PORT || !port->p.ops->pfc_get)
    {
        enet_err("wrong port type or no PFC capability: %s\n", port->obj_name);
		return -1;
	}

	return port->p.ops->pfc_get(port, pfc_rx_enable, pfc_tx_enable);
}

static inline int port_pfc_set(enetx_port_t *port, int pfc_rx_enable, int pfc_tx_enable)
{
	if (port->port_class != PORT_CLASS_PORT || !port->p.ops->pfc_set)
    {
        enet_err("wrong port type or no PFC capability: %s\n", port->obj_name);
		return -1;
	}

	return port->p.ops->pfc_set(port, pfc_rx_enable, pfc_tx_enable);
}

static inline int port_pfc_tx_timer_get(enetx_port_t *port, int priority, int *pfc_tx_timer)
{
	if (port->port_class != PORT_CLASS_PORT || !port->p.ops->pfc_get)
    {
        enet_err("wrong port type or no PFC capability: %s\n", port->obj_name);
		return -1;
	}

	return port->p.ops->pfc_tx_timer_get(port, priority, pfc_tx_timer);
}

static inline int port_pfc_tx_timer_set(enetx_port_t *port, int priority, int pfc_tx_timer)
{
	if (port->port_class != PORT_CLASS_PORT || !port->p.ops->pfc_set)
    {
        enet_err("wrong port type or no PFC capability: %s\n", port->obj_name);
		return -1;
	}

	return port->p.ops->pfc_tx_timer_set(port, priority, pfc_tx_timer);
}

static inline int port_pause_get(enetx_port_t *port, int *rx_enable, int *tx_enable)
{
    enet_dbg("port pause get %s\n", port->obj_name);

	if (port->port_class != PORT_CLASS_PORT || !port->p.ops->pause_get)
	{
        enet_err("wrong port type or no pause capability: %s\n", port->obj_name);
        return -1;
	}

	return port->p.ops->pause_get(port, rx_enable, tx_enable);
}

static inline int port_pause_set(enetx_port_t *port, int rx_enable, int tx_enable)
{
    enet_dbg("port pause set %s: rx:%s tx:%s\n", port->obj_name,
        rx_enable ? "enable" : "disable", tx_enable ? "enable" : "disable");

	if (port->port_class != PORT_CLASS_PORT || !port->p.ops->pause_set)
    {
        enet_err("wrong port type or no pause capability: %s\n", port->obj_name);
		return -1;
	}

	return port->p.ops->pause_set(port, rx_enable, tx_enable);
}

static inline void port_open(enetx_port_t *self)
{
    if (self->port_class != PORT_CLASS_PORT)
        return;

    if (self->p.ops->open)
        self->p.ops->open(self);
    else
        port_generic_open(self);
}

static inline void port_stop(enetx_port_t *self)
{
    if (self->port_class != PORT_CLASS_PORT)
        return;

    if (self->p.ops->stop)
        self->p.ops->stop(self);
    else
        port_generic_stop(self);
}
    
static inline int port_mtu_set(enetx_port_t *self, int mtu)
{
    enet_dbg("port mtu set %s: mtu:%d\n", self->obj_name, mtu);

    if (self->port_class != PORT_CLASS_PORT)
        return -1;

    if (self->p.ops->mtu_set)
        return self->p.ops->mtu_set(self, mtu);
    
    return -1;
}

static inline int port_mib_dump(enetx_port_t *self, int all)
{
    //enet_dbg("port mib dump %s: all=%d\n", self->obj_name, all);

    if (!(self->port_class & (PORT_CLASS_PORT)))
        return -1;

    if (self->p.ops->mib_dump)
        return self->p.ops->mib_dump(self, all);
    
    return -1;
}

// add by Andrew
static inline int port_mib_dump_us(enetx_port_t *self, void *e)
{
	int ret;

    if (!(self->port_class & (PORT_CLASS_PORT)))
        return -1;

    if (self->p.ops->mib_dump_us) {
		ret = self->p.ops->mib_dump_us(self, e);
        return ret;
	}
    
    return -1;
}
// end of add

static inline void port_mib_clear(enetx_port_t *self)
{
    //enet_dbg("port mib clear %s:\n", self->obj_name);

    if (!(self->port_class & (PORT_CLASS_PORT)))
        return;

    if (self->p.ops->stats_clear)
        self->p.ops->stats_clear(self);
    
}

static inline void port_print_status(enetx_port_t *self)
{
    if ((self->port_class == PORT_CLASS_PORT) &&
        (self->p.ops->print_status))
    {
        self->p.ops->print_status(self);
    }
}

static inline int port_link_change(enetx_port_t *self, int up)
{
    if ((self->port_class == PORT_CLASS_PORT) &&
        (self->p.ops->link_change))
    {
        return self->p.ops->link_change(self, up);
    }

    return 0;
}

#if defined(DSL_DEVICES)
void dslbase_phy_link_change_cb(void *ctx);

#include <bcm/bcmswapitypes.h>

int ioctl_handle_mii(struct net_device *dev, struct ifreq *ifr, int cmd);

#endif /* defined(DSL_DEVICES) */
#endif

