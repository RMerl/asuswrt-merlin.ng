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

typedef enum
{
    PORT_TYPE_RUNNER_SW,
    PORT_TYPE_RUNNER_PORT,
    PORT_TYPE_RUNNER_GPON,
    PORT_TYPE_RUNNER_EPON,
    PORT_TYPE_RUNNER_DETECT, /* XXX: not required, can add logic here for platform detect.. ? */
    PORT_TYPE_RUNNER_WIFI,
    PORT_TYPE_G9991_SW,
    PORT_TYPE_G9991_PORT,
    PORT_TYPE_VLAN_SW,
    PORT_TYPE_VLAN_PORT,
    PORT_TYPE_DIRECT_RGMII,
    PORT_TYPE_GENERIC_DMA,
} port_type_t;

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
} port_cap_t;

typedef enum
{
    PORT_NETDEV_ROLE_NONE,
    PORT_NETDEV_ROLE_LAN,
    PORT_NETDEV_ROLE_WAN,
} port_netdev_role_t;

typedef struct
{
    unsigned long rx_packets;
    unsigned long rx_bytes;
    unsigned long rx_dropped;
    unsigned long rx_dropped_no_skb;
    unsigned long rx_dropped_blog_drop;
    unsigned long rx_packets_blog_done;
    unsigned long rx_dropped_no_rxdev;
    unsigned long tx_dropped_dispatch;
    unsigned long tx_dropped_mux_failed;
} port_stats_t;

typedef struct enetx_port_t
{
    /* Port tree */
    /* SW / port */
    port_class_t port_class;
    /* Should a Linux if be created for this port */
    int has_interface;
    /* Set after port init function has been called */
    int is_init;
    /* Force net_device name */
    char name[IFNAMSIZ];
    /* TODO: for debug, can be removed later */
    char obj_name[IFNAMSIZ];

    struct net_device *dev;
    /* Pointer to private data */
    void *priv;

    /* Port type enumeration */
    port_type_t port_type;

    /* TODO: Add locks */
    
    union
    {
        struct
        {
            /* Mac port object to perform operations such as read statistics and set speed */
            mac_dev_t *mac;
            /* Phy port object to perform operations such as read status and eee enable */
            phy_dev_t *phy;
            /* 1/100 of a second from system uptime, updated at PHY link status change */
            uint32_t phy_last_change;

            /* Physical device identifier on switch, used for muxing */
            int port_id;
            /* Role this port is allowed to act as */
            port_cap_t port_cap;
            /* port operations */
            struct port_ops_t *ops;
            /* Link to parent/child SW */
            struct enetx_port_t *parent_sw; /* set by port_create */
            struct enetx_port_t *child_sw;  /* set by init operation of switch */

            /* Cached mux/demux mappings */
            struct enetx_port_t **mux_map; /* Set by switch init */
            struct enetx_port_t **demux_map; /* Set by port add XXX */
            int port_count;
            int (*port_demux)(struct enetx_port_t *sw, enetx_rx_info_t *rx_info, FkBuff_t *fkb, struct enetx_port_t **out_port);
            int (*port_mux)(struct enetx_port_t *tx_port, pNBuff_t pNBuff, struct enetx_port_t **out_port);
        } p;
        struct
        {
            /* switch ops */
            struct sw_ops_t *ops;
            /* Link to parent port */
            struct enetx_port_t *parent_port; /* set by switch_create */
            /* Dynamically allocated list of ports on SW */
            struct enetx_port_t **ports;
            int port_count;

            /* Support for generic built-in (de)-mux capabilities */
            int mux_count;
            /* child->port_id -> enetx_port_t map, register here with mux_on_sw() */
            struct enetx_port_t **mux_map;
            int demux_count;
            /* parent->port_id -> enetx_port_t map, register here with demux_on_sw() */
            struct enetx_port_t **demux_map;
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
#endif
        /* Should set this when creating the interface when rtnl is already locked */
        int rtnl_is_locked;
        port_netdev_role_t port_netdev_role;
    } n;
} enetx_port_t;
 
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
} port_info_t;

typedef struct port_ops_t
{
    /* HW related called by port_init triggered by sw_init */
    int (*init)(enetx_port_t *self);
    int (*uninit)(enetx_port_t *self);
    int (*dispatch_pkt)(pNBuff_t pNBuff, enetx_port_t *port, int channel, int egress_queue);
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
} port_ops_t;

typedef struct sw_ops_t
{
    /* Initialize switch HW - called every time SW is enslaved to port */
    int (*init)(enetx_port_t *self);
    int (*uninit)(enetx_port_t *self);
    /* Will be called by .ndo_get_stats, should collect sw statistics (eg: total underlying ports) */
    void (*stats_get)(enetx_port_t *self, struct rtnl_link_stats64 *net_stats);
    void (*stats_clear)(enetx_port_t *self);
    /* Used by port_create, should return the sw designated port_id and port type according to port_info */
    int (*port_id_on_sw)(port_info_t *port_info, int *port_id, port_type_t *port_type);
            
    /* Support for generic built-in (de)-mux capabilities */
    /* Will be cached and called from port object for performance */
    /* Demux rx_port and fkb on sw to out_port */
    int (*port_demux)(enetx_port_t *sw, enetx_rx_info_t *rx_info, FkBuff_t *fkb, enetx_port_t **out_port);
    /* Mux tx_port->port_id and fkb on tx_port->parent_sw to out_port */
    int (*port_mux)(enetx_port_t *tx_port, pNBuff_t pNBuff, enetx_port_t **out_port);
} sw_ops_t;

/* Functions used to create port tree */
/* Create switch object */
enetx_port_t *sw_create(port_type_t type, enetx_port_t *parent_port);
/* Create default port type object at index port_id on parent_sw */
enetx_port_t *port_create(port_info_t *port_info, enetx_port_t *parent_sw);
/* Start initialization sequence for switch sw and underlying port tree */
int sw_init(enetx_port_t *sw);
int port_init(enetx_port_t *port);
/* Uninitialize and free switch (or port) and its underlying objects */
void sw_free(enetx_port_t **sw);
/* Initializes port of class detect with type */
int port_init_detect(enetx_port_t *port, port_type_t type);

/* Support for generic built-in (de)-mux capabilities */
/* Assign port to demux on switch at a specific index */
int demux_on_sw(enetx_port_t *sw, int index, enetx_port_t *port);
/* Assign port to mux on switch at a specific index */
int mux_on_sw(enetx_port_t *from, enetx_port_t *sw, int index, enetx_port_t *to);
/* Generic demux function which uses sw->s.demux_map[rx_port] mapping */
int port_generic_sw_demux(enetx_port_t *sw, enetx_rx_info_t *rx_info, FkBuff_t *fkb, enetx_port_t **out_port);
/* Generic mux callback which maps to tx_port->parent_sw->parent_port */
int port_generic_sw_mux(enetx_port_t *tx_port, pNBuff_t pNBuff, enetx_port_t **out_port);

/* generic PORT_CLASS_PORT ops */
void port_generic_open(enetx_port_t *self);
void port_generic_stop(enetx_port_t *self);
int port_generic_mtu_set(enetx_port_t *self, int mtu);
void port_generic_stats_get(enetx_port_t *self, struct rtnl_link_stats64 *net_stats);

/* root switch */
extern enetx_port_t *root_sw;

typedef int (*port_traverse_cb)(enetx_port_t *port, void *ctx);
/* Helper function which recursively traverses 'port', calls fn_port(p, ctx) on each PORT_CLASS_PORT,
   and fn_sw(p, ctx) on each PORT_CLASS_SW; Callback should return 0 to continue to next port,
   < 0 to halt on error, > 0 to stop without returning error.
   max_sw_depth is the number of sw recursians it will make, -1 for all, 1 for current sw only */
int _port_traverse_ports(enetx_port_t *port, port_traverse_cb fn, port_class_t classes, void *ctx, int max_sw_depth);
#define port_traverse_ports(port, fn, classes, ctx) _port_traverse_ports(port, fn, classes, ctx, -1)

int port_netdev_role_set(enetx_port_t *self, port_netdev_role_t role);

#define PORT_SHARED_OPS_EXIST(port, opcmd) \
    ((port->port_class == PORT_CLASS_PORT && port->p.ops->opcmd) || \
        (port->port_class == PORT_CLASS_SW && port->s.ops->opcmd))

#define _PORT_SHARED_OPS(ret, port, opcmd) \
    do { \
        if (port->port_class == PORT_CLASS_PORT) \
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
    if (port->port_class != PORT_CLASS_PORT)
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

#ifdef VLANTAG
extern sw_ops_t port_vlan_sw;
extern port_ops_t port_vlan_port;
#endif

#ifdef ENET_DMA
extern sw_ops_t port_dummy_sw;
extern port_ops_t port_dma_port;
#endif
#endif

