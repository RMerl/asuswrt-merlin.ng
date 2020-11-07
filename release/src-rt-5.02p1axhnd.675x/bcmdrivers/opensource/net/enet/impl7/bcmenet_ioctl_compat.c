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
 *  Created on: Dec/2015
 *      Author: ido@broadcom.com
 */

#include "enet.h"
#include "port.h"
#include "enet_dbg.h"
#include <bcm/bcmswapitypes.h>
#include <board.h>
#include <bcmnet.h>
#include <boardparms.h>
#ifdef RUNNER
#include "runner.h"
#endif
#ifdef SYSPVSW_DEVICE
#include "syspvsw.h"
#endif
#ifdef SF2_DEVICE
#include "sf2.h"
#endif
#ifdef CONFIG_BCM_PTP_1588
#include "ptp_1588.h"
#endif
#ifdef DSL_DEVICES
#include "phy_drv_sf2.h"    // TODO_DSL: need to move reference to C45 phy out
#endif
#include "opticaldet.h"
#include "bcmenet_common.h"
#include "crossbar_dev.h"
#include "phy_drv.h"
#include "phy_macsec_common.h"

extern int apd_enabled;
extern int eee_enabled;

#if defined(CONFIG_BCM_ETH_PWRSAVE)
static int pm_apd_set_single(enetx_port_t *p, void *ctx)
{
    phy_dev_t *phy_dev = p->p.phy;

    if (phy_dev)
        cascade_phy_dev_apd_set(phy_dev, apd_enabled);

    return 0;
}
#endif

#if defined(CONFIG_BCM_ENERGY_EFFICIENT_ETHERNET)
static int pm_eee_set_single(enetx_port_t *p, void *ctx)
{
    mac_dev_t *mac_dev = p->p.mac;
    phy_dev_t *phy_dev = p->p.phy;

    if (mac_dev)
        mac_dev_eee_set(mac_dev, 0);

    if (phy_dev)
        cascade_phy_dev_eee_set(phy_dev, eee_enabled);

    return 0;
}
#endif

#if defined(CONFIG_BCM_ETH_PWRSAVE)
static int pm_apd_get(void)
{
    return apd_enabled;
}

static void pm_apd_set(int enabled)
{
    apd_enabled = enabled;
    port_traverse_ports(root_sw, pm_apd_set_single, PORT_CLASS_PORT, NULL);
}
#endif

#if defined(CONFIG_BCM_ENERGY_EFFICIENT_ETHERNET)
static int pm_eee_get(void)
{
    return eee_enabled;
}

static void pm_eee_set(int enabled)
{

    eee_enabled = enabled;
    port_traverse_ports(root_sw, pm_eee_set_single, PORT_CLASS_PORT, NULL);
}
#endif

enetx_port_t *unit_port_array[BP_MAX_ENET_MACS][COMPAT_MAX_SWITCH_PORTS] = {};

int unit_port_oam_idx_array[BP_MAX_ENET_MACS][COMPAT_MAX_SWITCH_PORTS] = {[0 ... BP_MAX_ENET_MACS-1][0 ... COMPAT_MAX_SWITCH_PORTS-1] = -1};
#define COMPAT_PORT(u, p) ((unit_port_array[u][p] && (unit_port_array[u][p]->port_class & (PORT_CLASS_PORT))) ? \
    unit_port_array[u][p] : NULL)
#define COMPAT_OAM_IDX(u, p) (COMPAT_PORT(u, p) ? unit_port_oam_idx_array[u][p] : -1)

#ifdef RUNNER
#define COMPAT_RPDA(u, p) _port_rdpa_object_by_port(_compat_port_object_from_unit_port(u, p))
#endif

static int tr_port_by_net_port(enetx_port_t *port, void *_ctx);

static int gpon_mcast_gem_set(int mcast_idx)
{
#ifdef GPON
    int net_port = NET_PORT_GPON;
    void *in_out = (void *)(unsigned long)net_port;
    enetx_port_t *port;
    
    if (mcast_idx >= CONFIG_BCM_MAX_GEM_PORTS)
        return -1;

    if (port_traverse_ports(root_sw, tr_port_by_net_port, PORT_CLASS_PORT, &in_out) <= 0)
        return -1;

    port = (enetx_port_t *)in_out;
    return netdev_path_set_hw_subport_mcast_idx(port->dev, mcast_idx);
#else
    return -1;
#endif
}

static int _handle_epon(enetx_port_t **port)
{
    enetx_port_t *epon_port;
    port_info_t port_info =
    {
        .is_epon = 1,
        .is_detect = 1,
    };

    if (port_create(&port_info, root_sw, &epon_port))
        return -1;

    epon_port->has_interface = 1;
    if ((epon_port->p.phy = phy_dev_add(PHY_TYPE_PON, 0, NULL)) == NULL)
        return -1;

    if (phy_driver_init(PHY_TYPE_PON))
        return -1;

    if ((epon_port->p.mac = mac_dev_add(MAC_TYPE_xEPON, 0, NULL)) == NULL)
        return -1;
    
    strcpy(epon_port->name, "epon0");

    *port = epon_port;

    return 0;
}

static int tr_port_by_net_port(enetx_port_t *port, void *_ctx)
{
    int net_port = *(uint32_t *)_ctx;

    switch (net_port)
    {
        case NET_PORT_GPON:
            if (port->port_info.is_gpon)
            {
                *(enetx_port_t **)_ctx = port;
                return 1;
            }
            break;
        case NET_PORT_EPON:
        case NET_PORT_EPON_AE:
            if (port->port_info.is_epon)
            {
                *(enetx_port_t **)_ctx = port;
                return 1;
            }
            break;
        case NET_PORT_LAN_0 ... NET_PORT_LAN_7:
            if (port->p.mac && port->p.mac->mac_id == net_port)
            {
                *(enetx_port_t **)_ctx = port;
                return 1;
            }
            break;
    }

    return 0;
}

static int get_port_by_net_port(enetx_port_t *sw, port_class_t port_class, unsigned long net_port, enetx_port_t **match)
{
    void *in_out = (void *)(unsigned long)net_port;

    if (port_traverse_ports(sw, tr_port_by_net_port, port_class, &in_out) <= 0)
        return -1;

    *match = (enetx_port_t *)in_out;

    return 0;
}

static int _handle_add_port_gpon(struct net_port_t *net_port, enetx_port_t **_p)
{
    mac_cfg_t mac_cfg = {};
    enetx_port_t *p;
    port_info_t port_info =
    {
        .is_detect = 1,
        .is_gpon = 1,
    };

    if (port_create(&port_info, root_sw, &p))
        return -1;

    p->has_interface = 1;
    if ((p->p.phy = phy_dev_add(PHY_TYPE_PON, 0, NULL)) == NULL)
        return -1;

    if (phy_driver_init(PHY_TYPE_PON))
        return -1;

    if ((p->p.mac = mac_dev_add(MAC_TYPE_xGPON, net_port->sub_type, NULL)) == NULL)
        return -1;

    switch (net_port->speed)
    {
        case NET_PORT_SPEED_1010:
            mac_cfg.speed = MAC_SPEED_10000;
            break;
        default:
            mac_cfg.speed = MAC_SPEED_2500;
        /* GPON mode ignores speed in mac_dev */
    }
    
    mac_dev_cfg_set(p->p.mac, &mac_cfg);
    strcpy(p->name, "gpondef");
    *_p = p;

    return 0;
}
            
static int _handle_add_port_ae(enetx_port_t **_p)
{
    enetx_port_t *p = *_p;

    if (!p)
    {
        port_info_t port_info =
        {
            .is_epon = 1,
            .is_detect = 1,
        };

        if (port_create(&port_info, root_sw, &p))
            return -1;

        if ((p->p.phy = phy_dev_add(PHY_TYPE_PON, 0, NULL)) == NULL)
            return -1;

        if (phy_driver_init(PHY_TYPE_PON))
            return -1;
    }

    /* put port into tail of unit_port_array */
    unit_port_array[0][BP_MAX_SWITCH_PORTS-1] = p;
    p->has_interface = 1;

    /* Skip creating mac if device was already enabled */
    if (!p->p.mac && (p->p.mac = mac_dev_add(MAC_TYPE_EPON_AE, 0, p->p.phy)) == NULL)
        return -1;

    *_p = p;

    return 0;
}

static int _handle_add_port(struct net_port_t *net_port)
{
    int port = net_port->port;
    int is_wan = net_port->is_wan;
    int rc = 0;
    char *ifname = net_port->ifname;
    enetx_port_t *p = NULL;

    if (get_port_by_net_port(root_sw, PORT_CLASS_PORT_DETECT, port, &p) < 0)
    {
        if (port == NET_PORT_EPON_AE)
            rc = _handle_add_port_ae(&p);
        else if (port == NET_PORT_EPON)
            rc = _handle_epon(&p);
        else if (port == NET_PORT_GPON)
            rc = _handle_add_port_gpon(net_port, &p);
        else
            return -1;
    }
    /* NET_PORT_EPON_AE port might have been created by bp_parsing, so attach it a MAC_TYPE_EPON_AE */
    else if (port == NET_PORT_EPON_AE)
        rc = _handle_add_port_ae(&p);

    if (rc)
        return -1;

    p->port_info.is_wan = is_wan;

    if (*ifname)
        strncpy(p->name, ifname, IFNAMSIZ);

    if (port_activate(p))
        return -1;
    
    /* This is a hack since there is no post-netdev_register event in enet, and it is not initialized anywhere else */
    if (port == NET_PORT_GPON)
        p->dev->path.hw_subport_mcast_idx = NETDEV_PATH_HW_SUBPORTS_MAX;
    
    /* Copy interface name back to userspace */
    strncpy(net_port->ifname, p->dev->name, IFNAMSIZ);

    return 0;
}

#if defined(CONFIG_BCM_PON_RDP)
static int _handle_del_port(int port)
{
    return 0;
}
#else
static int _handle_del_port(int port)
{
    enetx_port_t *p = NULL;

    if (get_port_by_net_port(root_sw, PORT_CLASS_PORT, port, &p) < 0)
        return -1;

    port_deactivate(p);

    return 0;
}
#endif

static int tr_port_by_netdev(enetx_port_t *port, void *_ctx)
{
    if (port->dev == *(struct net_device **)_ctx)
    {
        *(enetx_port_t **)_ctx = port;
        return 1;
    }
    return 0;
}

int port_by_netdev(struct net_device *dev, enetx_port_t **match)
{
    void *in_out = (void *)dev;

    *match = NULL;
    if (port_traverse_ports(root_sw, tr_port_by_netdev, PORT_CLASS_PORT|PORT_CLASS_SW, &in_out) <= 0)
        return -1;
    *match =(enetx_port_t *)in_out;
    return 0;
}

typedef struct list_ctx
{
    int maxlen;
    char *str;
    int op_code;
} list_ctx;

static int _devname_concat(enetx_port_t *port, char *buf, int maxlen)
{
    int dev_len, buf_len;

    if (!port->dev)
        return 0;

    buf_len = strlen(buf);
    dev_len = strlen(port->dev->name) + (buf[0] ? 1 : 0);

    if (buf_len + dev_len >= maxlen)
        return -1;

    if (buf[0])
        strcat(buf, ",");

    strcat(buf, port->dev->name);

    return 0;
}

static int tr_wan_devname_concat(enetx_port_t *port, void *_ctx)
{
    list_ctx *ctx = (list_ctx *)_ctx;

    if (port->n.port_netdev_role == PORT_NETDEV_ROLE_WAN &&
        _devname_concat(port, ctx->str, ctx->maxlen) < 0)
    {
        return -1;
    }

    return 0;
}

static int tr_ifreq_devname_concat(enetx_port_t *port, void *_ctx)
{
    list_ctx *ctx = (list_ctx *)_ctx;

    if (port->p.port_cap == ctx->op_code && _devname_concat(port, ctx->str, ctx->maxlen) < 0)
        return -1;

    return 0;
}

static int get_root_dev_by_name(char *ifname, struct net_device **_dev)
{
    struct net_device *dev;

    if (!_dev)
        return -1;

    dev = dev_get_by_name(&init_net, ifname);
    if (!dev)
        return -1;

    /* find root device */
    while (1)
    {
        if (netdev_path_is_root(dev))
            break;

        dev_put(dev);
        dev = netdev_path_next_dev(dev);
        dev_hold(dev);
    }

    *_dev = dev;
    dev_put(dev);

    return 0;
}

int get_port_by_if_name(char *ifname, enetx_port_t **port)
{
    struct net_device *dev;

    if (get_root_dev_by_name(ifname, &dev))
    {
        return -EFAULT;
    }

    if (port_by_netdev(dev, port))
    {
        return -EFAULT;
    }
    return 0;
}

struct tr_portphy_info {
    char ifname[IFNAMSIZ];
    int  mapping;
};

static int tr_get_next_portphy_info(enetx_port_t *port, void *_ctx)
{
    struct tr_portphy_info *info = (struct tr_portphy_info *)_ctx;

    if (crossbar_group_external_endpoint_count(port->p.phy, &info->mapping) >= 0) {
        if (strlen(info->ifname) == 0) {
            // found our next interface on crossbar
            strcpy(info->ifname, port->dev->name);
            return 1;
        } else if (strcmp(info->ifname, port->dev->name)==0) {
            // found our current interface, reset ifname, to look for next
            info->ifname[0]=0;
        }
    }
    return 0;
}

struct tr_subport_info {
    int              external_endpoint;
    phy_dev_t       *phy;
    enetx_port_t    *port;
};

static int tr_get_subport_info(enetx_port_t *port, void *_ctx)
{
    struct tr_subport_info *info = (struct tr_subport_info *)_ctx;
    info->phy = crossbar_group_phy_get(port->p.phy, info->external_endpoint);
    if (info->phy) {
        info->port = port;
        return 1;
    }
    return 0;
}

static int tr_stats_clear(enetx_port_t *self, void *_ctx)
{
    enetx_port_t *target = (enetx_port_t *)_ctx;

    // if tartet is switch && self is port && self's parent switch has stats_clear then skip
    // as self's parent switch already clear mib for entire switch
    if ((target->port_class == PORT_CLASS_SW) &&
        (self->port_class != PORT_CLASS_SW) &&
        (self->p.parent_sw->s.ops->stats_clear))
        return 0;

    port_stats_clear(self);
    return 0;
}

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
    char    *buf;
    int     buf_sz;
} counterSum_t;

static int display_enet_dev_stats(enetx_port_t *port, void *ctx)
{
    // based on imp5\bcmenet.c display_enet_dev_stats()
    port_stats_t *estats = &port->n.port_stats;
    counterSum_t *cts = (counterSum_t *)ctx;
    counterSum_t portCnt = {};
    char *buf = cts->buf;
    char *buf_end = buf + cts->buf_sz;
    int i;

    if (port->p.child_sw) return 0;  // skip imp port

    if (buf)
    {
        buf += snprintf(buf, buf_end-buf, "Device %s:\n", port->dev->name);
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(tx_packets, rx_packets));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(tx_bytes, rx_bytes));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n\n", ESTATS_VAL(tx_dropped, rx_dropped));

        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(tx_mcast_packets, rx_mcast_packets));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(tx_mcast_bytes, rx_mcast_bytes));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(tx_bcast_packets, rx_bcast_packets));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n\n", ESTATS_VAL(tx_bcast_bytes, rx_bcast_bytes));
    }
#if defined(DEBUG_STATS)
    if (buf)
    {
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(rx_packets_queue[0], rx_packets_queue[1]));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(rx_packets_queue[2], rx_packets_queue[3]));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(rx_packets_queue[4], rx_packets_queue[5]));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(rx_packets_queue[6], rx_packets_queue[7]));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(tx_packets_queue_in[0], tx_packets_queue_in[1]));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(tx_packets_queue_in[2], tx_packets_queue_in[3]));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(tx_packets_queue_in[4], tx_packets_queue_in[5]));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(tx_packets_queue_in[6], tx_packets_queue_in[7]));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(tx_packets_queue_out[0], tx_packets_queue_out[1]));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(tx_packets_queue_out[2], tx_packets_queue_out[3]));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(tx_packets_queue_out[4], tx_packets_queue_out[5]));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(tx_packets_queue_out[6], tx_packets_queue_out[7]));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(rx_dropped_no_rxdev, rx_dropped_blog_drop));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(rx_dropped_no_skb, rx_packets_blog_done));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(rx_dropped_no_srcport, rx_packets_netif_receive_skb));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(tx_dropped_dispatch, tx_dropped_mux_failed ));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(tx_dropped_no_skb, tx_dropped_bad_nbuff ));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(tx_dropped_no_lowlvl_resource, tx_dropped_no_fkb));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(tx_dropped_accelerator_lan_fail, tx_dropped_accelerator_wan_fail));
    }

    for(i = 0; i<MAX_NUM_OF_PRI_QS; i++)
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
        estats->rx_dropped_no_srcport;

    portCnt.txDrops = estats->tx_dropped_bad_nbuff +
        estats->tx_dropped_no_lowlvl_resource +
        estats->tx_dropped_mux_failed +
        estats->tx_dropped_no_fkb +
        estats->tx_dropped_no_fkb +
        estats->tx_dropped_no_skb +
        estats->tx_dropped_accelerator_lan_fail +
        estats->tx_dropped_accelerator_wan_fail;
#endif /* DEBUG_STATS */

    if (buf)
    {
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL2(portCnt.rxIn, portCnt.rx2Kernel));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL2(portCnt.rx2Blog, portCnt.rx2Kernel + portCnt.rx2Blog));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT1 "\n", ESTATS_VAL1(portCnt.rxDrops));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT1 "\n", ESTATS_VAL1((portCnt.rx2Kernel + portCnt.rx2Blog + portCnt.rxDrops)));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT1b "\n", ESTATS_VAL1b(portCnt.rxIn == (portCnt.rx2Kernel + portCnt.rx2Blog + portCnt.rxDrops)));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL2(portCnt.txIn, portCnt.txOut));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL2(portCnt.txDrops, portCnt.txOut + portCnt.txDrops));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT1b "\n", ESTATS_VAL1b(portCnt.txIn == (portCnt.txOut + portCnt.txDrops)));
        buf += snprintf(buf, buf_end-buf, "\n");
    }

    cts->rxIn += portCnt.rxIn;
    cts->rx2Kernel += portCnt.rx2Kernel;
    cts->rx2Blog += portCnt.rx2Blog;
    cts->rxDrops += portCnt.rxDrops;
    cts->txIn += portCnt.txIn;
    cts->txOut += portCnt.txOut;
    cts->txDrops += portCnt.txDrops;

    return 0;
}

static void display_software_stats(char *buf, int buf_sz, enetx_port_t *port)
{
    // based on imp5\bcmenet.c display_enet_stats()
    counterSum_t total = {};
    char *buf_end = buf + buf_sz;

    if(port->port_class == PORT_CLASS_PORT)
    {
        total.buf = buf;
        total.buf_sz = buf_sz;
        display_enet_dev_stats(port, &total);
        return;
    }

    // traverse all ports on switch
    port_traverse_ports(port, display_enet_dev_stats, PORT_CLASS_PORT, &total);

    buf += snprintf(buf, buf_end-buf, "\n");
    buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL2(total.rxIn, total.rx2Kernel));
    buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL2(total.rx2Blog, total.rx2Kernel + total.rx2Blog));
    buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL2(total.rxDrops, (total.rx2Kernel + total.rx2Blog + total.rxDrops)));
    buf += snprintf(buf, buf_end-buf, ESTATS_FMT1b "\n", ESTATS_VAL1b(total.rxIn == (total.rx2Kernel + total.rx2Blog + total.rxDrops)));
    buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL2(total.txIn, total.txOut));
    buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL2(total.txDrops, total.txOut + total.txDrops));
    buf += snprintf(buf, buf_end-buf, ESTATS_FMT1b "\n", ESTATS_VAL1b(total.txIn == (total.txOut + total.txDrops)));
}

static int _compat_validate_unit_port(int unit, int port)
{
    if (unit < 0 || port < 0 || unit >= BP_MAX_ENET_MACS || port >= COMPAT_MAX_SWITCH_PORTS)
    {
        enet_err("invalid unit %d, port %d values\n", unit, port);
        return -1;
    }

    return 0;
}


static enetx_port_t *_compat_port_object_from_unit_port(int unit, int port)
{
    enetx_port_t *p;

    if (_compat_validate_unit_port(unit, port))
        return NULL;

    if (!(p = COMPAT_PORT(unit, port)))
    {
        enet_err("cannot map unit %d, port %d to port object\n", unit, port);
        return NULL;
    }

    return p;
}

static void mac_dev_stats_to_emac_stats(struct emac_stats *emac_stats, mac_stats_t *mac_stats)
{
#define MAC_STATS_COPY(f) emac_stats->f = mac_stats->f

    MAC_STATS_COPY(rx_byte);
    MAC_STATS_COPY(rx_packet);
    MAC_STATS_COPY(rx_frame_64);
    MAC_STATS_COPY(rx_frame_65_127);
    MAC_STATS_COPY(rx_frame_128_255);
    MAC_STATS_COPY(rx_frame_256_511);
    MAC_STATS_COPY(rx_frame_512_1023);
    MAC_STATS_COPY(rx_frame_1024_1518);
    MAC_STATS_COPY(rx_frame_1519_mtu);
    MAC_STATS_COPY(rx_multicast_packet);
    MAC_STATS_COPY(rx_broadcast_packet);
    MAC_STATS_COPY(rx_unicast_packet);
    MAC_STATS_COPY(rx_alignment_error);
    MAC_STATS_COPY(rx_frame_length_error);
    MAC_STATS_COPY(rx_code_error);
    MAC_STATS_COPY(rx_carrier_sense_error);
    MAC_STATS_COPY(rx_fcs_error);
    MAC_STATS_COPY(rx_undersize_packet);
    MAC_STATS_COPY(rx_oversize_packet);
    MAC_STATS_COPY(rx_fragments);
    MAC_STATS_COPY(rx_jabber);
    MAC_STATS_COPY(rx_overflow);
    MAC_STATS_COPY(rx_control_frame);
    MAC_STATS_COPY(rx_pause_control_frame);
    MAC_STATS_COPY(rx_unknown_opcode);
    MAC_STATS_COPY(tx_byte);
    MAC_STATS_COPY(tx_packet);
    MAC_STATS_COPY(tx_frame_64);
    MAC_STATS_COPY(tx_frame_65_127);
    MAC_STATS_COPY(tx_frame_128_255);
    MAC_STATS_COPY(tx_frame_256_511);
    MAC_STATS_COPY(tx_frame_512_1023);
    MAC_STATS_COPY(tx_frame_1024_1518);
    MAC_STATS_COPY(tx_frame_1519_mtu);
    MAC_STATS_COPY(tx_fcs_error);
    MAC_STATS_COPY(tx_multicast_packet);
    MAC_STATS_COPY(tx_broadcast_packet);
    MAC_STATS_COPY(tx_unicast_packet);
    MAC_STATS_COPY(tx_total_collision);
    MAC_STATS_COPY(tx_jabber_frame);
    MAC_STATS_COPY(tx_oversize_frame);
    MAC_STATS_COPY(tx_undersize_frame);
    MAC_STATS_COPY(tx_fragments_frame);
    MAC_STATS_COPY(tx_error);
    MAC_STATS_COPY(tx_underrun);
    MAC_STATS_COPY(tx_excessive_collision);
    MAC_STATS_COPY(tx_late_collision);
    MAC_STATS_COPY(tx_single_collision);
    MAC_STATS_COPY(tx_multiple_collision);
    MAC_STATS_COPY(tx_pause_control_frame);
    MAC_STATS_COPY(tx_deferral_packet);
    MAC_STATS_COPY(tx_excessive_deferral_packet);
    MAC_STATS_COPY(tx_control_frame);
}

static int tr_port_by_port_id(enetx_port_t *port, void *_ctx)
{
    if (port->p.port_id == *(uint32_t *)_ctx)
    {
        *(enetx_port_t **)_ctx = port;
        return 1;
    }

    return 0;
}

int port_by_port_id(enetx_port_t *sw, int port_id, enetx_port_t **match)
{
    void *in_out = (void *)(unsigned long)port_id;

    if (port_traverse_ports(sw, tr_port_by_port_id, PORT_CLASS_PORT, &in_out) <= 0)
        return -1;

    *match = (enetx_port_t *)in_out;

    return 0;
}

enetx_port_t *port_by_unit_port(int unit_port)
{

#if defined(DSL_DEVICES)
    return _compat_port_object_from_unit_port(LOGICAL_PORT_TO_UNIT_NUMBER(unit_port), LOGICAL_PORT_TO_PHYSICAL_PORT(unit_port));
#else
    return _compat_port_object_from_unit_port(0, unit_port);
#endif
}

static phy_dev_t* enet_dev_get_phy_dev(struct net_device *dev, int sub_port)
{
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(dev))->port;
    phy_dev_t *phy_dev;

    if (!phy_is_crossbar(port->p.phy))
        phy_dev = port->p.phy;
    else if (sub_port != -1)
        phy_dev = crossbar_group_phy_get(port->p.phy, BP_PHY_PORT_TO_CROSSBAR_PORT(sub_port));
    else    // get crossbar active phy
        phy_dev = crossbar_phy_dev_active(port->p.phy);

    return phy_dev;
}

#if defined(DSL_DEVICES)
// ----------- SIOCETHSWCTLOPS ETHSWKERNELPOLL functions ---
extern int enet_opened;

int ioctl_ethsw_kernel_poll(struct ethswctl_data *e)
{
    // based on shared\bcmenet.c:enet_ioctl_kernel_poll()
    static int kernel_polled = 0;

    if (kernel_polled == 0)
    {
        // MAC tx/rx needs to be disabled until after 1st kernel poll
        // if enet_open() occurred first, MAC tx/rx enable need to be in 1st kernel poll instead.
        if (enet_opened)
            enet_err("error: enet_open() occurred before 1st SWMDK polling!!!\n");

        // TODO_DSL:  initPorts if necessary, do polling
    }
    kernel_polled++;

#if defined(CONFIG_BCM_ETH_DEEP_GREEN_MODE)
    if (e->mdk_kernel_poll.link_change == ETHSW_LINK_CHANGED) {
        port_sf2_deep_green_mode_handler();
    }
#endif

    e->mdk_kernel_poll.link_change = ETHSW_LINK_FORCE_CHECK;
    return BCM_E_NONE;
}

int ioctl_handle_mii(struct net_device *dev, struct ifreq *ifr, int cmd)
{
    // based on bcmenet.c:bcm63xx_enet_ioctl()
    struct mii_ioctl_data *mii = if_mii(ifr);
    enetx_port_t *port = NETDEV_PRIV(dev)->port;
    phy_dev_t *phy;
    int rc;
    u16 v16=0;

    if (port->port_class != PORT_CLASS_PORT || !port->p.phy)
        return -EINVAL;

    phy = port->p.phy;
    if (phy->sw_port != port)
        return -EINVAL;

    switch (cmd)
    {
        case SIOCGMIIREG: /* Read MII PHY register. */
            rc = phy_bus_read(phy, mii->reg_num & 0x1f, &v16);
            mii->val_out = v16;
            return rc ? -EINVAL : 0;
        case SIOCSMIIREG: /* Write MII PHY register. */
            rc = phy_bus_write(phy, mii->reg_num & 0x1f, mii->val_in);
            return rc? -EINVAL : 0;
    }

    return -EINVAL;
}

#include "phy_bp_parsing.h"
void add_unspecified_ports(enetx_port_t *sw, uint32_t port_map, uint32_t imp_map)
{
    enetx_port_t *port;
    int ndx;
    int unit = IS_ROOT_SW(sw)? 0:1;

    for (ndx = 0; ndx <= BP_MAX_SWITCH_PORTS; ndx++)
    {
        port_info_t port_info = {};
        ETHERNET_MAC_INFO emac_info = {};

        if (unit_port_array[unit][ndx]) continue;       // port object already defined
        if (!(port_map & (1<<ndx))) continue;           // skipping none defined ports

        port_info.port = ndx;
        port_info.is_undef = 1;
        if (imp_map & (1<<ndx)) port_info.is_management = 1;

        if (port_create(&port_info, sw, &port))
        {
            enet_err("Failed to create unit %d port %d\n", unit, ndx);
            return;
        }
        if (SF2_ETHSWCTL_UNIT == unit)
        {
            emac_info.ucPhyType = BP_ENET_EXTERNAL_SWITCH; /* SF2 */
        }
        else
        {
            emac_info.ucPhyType = BP_ENET_NO_PHY; /* runner */
        }
        port->p.mac = bp_parse_mac_dev(&emac_info, ndx);

        if (imp_map & (1<<ndx))
        {
            port->p.port_cap = PORT_CAP_MGMT;
        }

        unit_port_array[unit][ndx] = port;
        enet_dbgv("%s add port %px macId=%x (%s) %px\n", sw->obj_name, port, ndx, port->p.mac->mac_drv->name, port->p.mac);
    }
}
#endif // DSL_DEVICES

#define ethctl_field_copy_to_user(f)      \
    copy_to_user((void*)((char*)rq->ifr_data +((char*)&(ethctl->f) - (char*)&rq_data)), &(ethctl->f), sizeof(ethctl->f))

#define IsMacToMac(id)   (((id) & (MAC_CONN_VALID|MAC_CONNECTION)) == (MAC_CONN_VALID|MAC_MAC_IF))

static int cat_snprintf(char **buf, int *sz, char *fmt, ...)
{
    va_list valist;
    int nc;
    int len = 128;
    int crLen = 0;
    char *bf;
    va_start(valist, fmt);

    if(*sz)
        crLen = strlen(*buf);
    else
        *buf = 0;

    bf = *buf + crLen;
    len = *sz - crLen;
    nc = vsnprintf(bf, len, fmt, valist);
    if (nc > len - 1) {
        *sz = crLen + nc + 1;
        *buf = krealloc(*buf, *sz, GFP_KERNEL);
        if(*buf == NULL) {
            *sz = 0;
            goto end;
        }
        bf = *buf + crLen;
        len = *sz - crLen;
        nc = vsnprintf(bf, len, fmt, valist);
    }
end:
    va_end(valist);
    return *sz;
}
static char *print_net_dev_info(struct net_device *dev)
{
    static char buf[256];
    snprintf(buf, sizeof(buf), "%s: %4s <%02X:%02X:%02X:%02X:%02X:%02X>",
                dev->name, netif_carrier_ok(dev)?"Up":"Down",
                dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
                dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);
    return buf;
}

static char *print_phy_link(phy_dev_t *phy_dev)
{
    static char buf[256];
    uint32_t caps = 0;
    phy_speed_t max_spd;
    int an;

    phy_dev_caps_get(phy_dev, CAPS_TYPE_ADVERTISE, &caps);
    an = (caps & PHY_CAP_AUTONEG) > 0;
    max_spd = phy_caps_to_max_speed(caps);
    snprintf(buf, sizeof(buf), "Cfg: %s%s; %s %s", an?"ANG.":"FIX.", phy_get_speed_string(max_spd),
        phy_dev->link?"Up":"Down", phy_dev->link?phy_get_speed_string(phy_dev->speed):"");
    return buf;
}

/* *bf is a buffer pointer used for recursive call for cascaded PHYs */
static char *_print_phy_info(phy_dev_t *phy_dev, char *bf, int indent)
{
    static char buf[512];
    int sz;
    phy_speed_t max_spd;

    if (phy_dev->flag & PHY_FLAG_NOT_PRESENTED)
        return "";

    if (bf == 0)
        bf = buf;

    sz = sizeof(buf) - (bf - buf);
    max_spd = phy_max_speed_get(phy_dev);
    if (bf != buf)
        bf += snprintf(bf, sz, "\n%*s", indent, "");
    sz = sizeof(buf) - (bf - buf);

    bf += snprintf(bf, sz, "PHY_ID <0x%08x:0x%02x:%8s %s%s %s> %s",
            phy_dev->meta_id, phy_dev->addr,
            (IsExtPhyId(phy_dev->meta_id)||phy_dev->phy_drv->phy_type==PHY_TYPE_I2C_PHY)? "External":"On-chip",
            IsRGMII(phy_dev->meta_id)? "RGMII ": phy_dev->cascade_prev? "Cascaded ": "",
            phy_get_speed_string(max_spd),
            phy_dev->phy_drv->name,
            print_phy_link(phy_dev));
    if (phy_dev->cascade_next)
        _print_phy_info(phy_dev->cascade_next, bf, indent);

    return buf;
}

static char *print_phy_info(phy_dev_t *phy_dev, int indent)
{
    return _print_phy_info(phy_dev, 0, indent);
}

static char *print_cphy_info(phy_dev_t *phy_dev, int indent)
{
    static char buf[512];
    char *bf = buf;
    phy_dev_t *phy;
    int ext_idx;
    int sz = sizeof(buf);

    if (!phy_is_crossbar(phy_dev))
        return print_phy_info(phy_dev, indent);

    bf[0]=0;
    for (phy = crossbar_phy_dev_first(phy_dev); phy; phy = crossbar_phy_dev_next(phy))
    {
        ext_idx = crossbar_external_endpoint(phy);
        bf += snprintf(bf, sz, "\n      Chip Port %2d, CrossBar Port %d, ",
                BP_CROSSBAR_PORT_TO_PHY_PORT(ext_idx),  ext_idx);
        sz = sizeof(buf) - (bf - buf);

        if (phy == crossbar_phy_dev_first(phy_dev))
            indent = bf - buf - 1;
        bf += snprintf(bf, sz, "%s", print_phy_info(phy, indent));
        sz = sizeof(buf) - (bf - buf);
    }
    return buf;
}

static char *print_port_info(enetx_port_t *p)
{
    static char buf[256];
    enetx_port_t *sw = p->p.parent_sw;
    mac_dev_t *mac = p->p.mac;
    int root = IS_ROOT_SW(sw);

    snprintf(buf, sizeof(buf), "<%sSw P%d, Logical %02d>",
            (root?"Int":"Ext"), mac->mac_id,
            PHYSICAL_PORT_TO_LOGICAL_PORT(mac->mac_id, root?0:1));
    return buf;
}


int sw_print_mac_phy_info(enetx_port_t *sw, char **buf, int *sz)
{
    int i, j;
    enetx_port_t *p;
    struct net_device *dev;
    int indent, s1;

    for (j = 0; j < 2; j++)
    {
        for (i = 0; i < sw->s.port_count; i++)
        {
            if(!(p = sw->s.ports[i]))
                continue;

            if (p->port_class != PORT_CLASS_PORT)
                continue;

            if (j == 0)    /* Parent round */
            {
                if (p->p.child_sw || !(dev = p->dev) || !p->p.phy)
                    continue;

                s1 = *sz;
                if (cat_snprintf(buf, sz, "%s %s ", print_net_dev_info(dev),
                    print_port_info(p)) == 0)
                    return 0;
                indent = *sz - s1 - 1;
                if (cat_snprintf(buf, sz, "%s\n", print_cphy_info(p->p.phy, indent)) == 0)
                    return 0;
            }
            else    /* Child switch round */
            {
                if (!p->p.child_sw) continue;

                if (sw_print_mac_phy_info(p->p.child_sw, buf, sz) == 0)
                    return 0;
                continue;
            }
        }
    }
    return *sz;
}

static int tr_get_next_port(enetx_port_t *port, void *ctx)
{
    enetx_port_t **pt = (enetx_port_t **)ctx;
    struct net_device *dev = port->dev;
    phy_dev_t *phy = port->p.phy;

    if (!dev || !phy)
        return 0;

    if (*pt == NULL)
    {
        *pt = port;
        return 1;
    }

    if (port == *pt)
        *pt = NULL;

    return 0;
}

int enet_get_next_port(enetx_port_t *port, enetx_port_t **next_port)
{
    int rc;

    enetx_port_t *_port = port;
    rc = port_traverse_ports(root_sw, tr_get_next_port, PORT_CLASS_PORT, &_port);
    *next_port = _port;
    return rc;
}

static int find_next_subport(struct net_device *dev, phy_dev_t *phy_dev, int *sub_port)
{
    if (*sub_port == -1)
    {
        phy_dev = crossbar_phy_dev_first(phy_dev);
        crossbar_info_by_phy(phy_dev, NULL, NULL, sub_port);
        *sub_port = BP_CROSSBAR_PORT_TO_PHY_PORT(*sub_port);
        return 1;
    }
    else
    {
        phy_dev = enet_dev_get_phy_dev(dev, *sub_port);
        phy_dev = crossbar_phy_dev_next(phy_dev);
        if (phy_dev)
        {
            crossbar_info_by_phy(phy_dev, NULL, NULL, sub_port);
            *sub_port = BP_CROSSBAR_PORT_TO_PHY_PORT(*sub_port);
            return 1;
        }
        *sub_port = -1;
    }
    return 0;
}

static int enet_get_next_phy(struct net_device **dev, int *sub_port)
{
    enetx_port_t *port;
    phy_dev_t *phy_dev;
    int port_found = 0;

    if (!strcmp((*dev)->name,  ETHERNET_ROOT_DEVICE_NAME))
    {
        enet_get_next_port(NULL, &port);
        *dev = port->dev;
        *sub_port = -1;
        port_found = 1;
    }
    else
        port = ((enetx_netdev *)netdev_priv(*dev))->port;

    phy_dev = port->p.phy;
    if (phy_is_crossbar(phy_dev) && find_next_subport(*dev, phy_dev, sub_port))
        return 1;

    if (*sub_port != -1 && find_next_subport(*dev, phy_dev, sub_port))
        return 1;

    if (port_found)
        return 1;

    /* Try to find next port */
    enet_get_next_port(port, &port);
    if (port == NULL)
        return 0;

    *sub_port = -1;
    *dev = port->dev;
    phy_dev = port->p.phy;
    if (phy_is_crossbar(phy_dev))
        return find_next_subport(*dev, phy_dev, sub_port);

    return 1;
}

typedef struct {
    phy_type_t phy_type;
    phy_dev_t *phy_dev;
} tr_find_port_type_t;

static int tr_find_port_type(enetx_port_t *port, void *_ctx)
{
    tr_find_port_type_t *port_type = _ctx;
    phy_dev_t *phy_dev = port->p.phy;;

    
    if (phy_dev == NULL || phy_dev->phy_drv == NULL)
        return 0;

    if (phy_dev->phy_drv->phy_type == port_type->phy_type) {
        port_type->phy_dev = phy_dev;
        return 1;
    }

    return 0;
}

int eth_get_gpon_sfp_info(sfp_type_t *sfp_type)
{
    phy_dev_t *phy_dev;
    TRX_TYPE trx_type = SFP_TYPE_UNKNOWN;
    tr_find_port_type_t port_type = {PHY_TYPE_XGAE};
    int rc;
    enetx_port_t *port;

    /* if the board does not defined 10GAE interface, give control to GPON */
    rc = port_traverse_ports(root_sw, tr_find_port_type, PORT_CLASS_PORT, &port_type);
    if (rc <= 0) {
        *sfp_type = SFP_TYPE_NOT_ETHERNET;
        return 0;
    }

    phy_dev = port_type.phy_dev;
    port = phy_dev->sw_port;
    /* if 10GAE is not configured as WAN, give control to GPON */
    if(port->n.port_netdev_role != PORT_NETDEV_ROLE_WAN) {
        *sfp_type = SFP_TYPE_NOT_ETHERNET;
        return 0;
    }

#if defined(CONFIG_I2C) && !defined(CONFIG_I2C_GPIO) && defined(CONFIG_BCM_OPTICALDET)
    rc = trx_get_type(phy_dev->cascade_next->addr, &trx_type);
    switch(rc) {
        case OPTICALDET_NOSFP:
            *sfp_type = SFP_TYPE_NO_MODULE;
            break;
        case OPTICALDET_INVPARM:
            printk("********** BUG bus %d\n", phy_dev->cascade_next->addr);
            BUG();
        default:
            switch (trx_type) {
                case TRX_TYPE_XPON:
                    *sfp_type = SFP_TYPE_XPON;
                    break;
                case TRX_TYPE_ETHERNET:
                    *sfp_type = SFP_TYPE_ETHERNET;
                    break;
                case TRX_TYPE_UNKNOWN:
                    *sfp_type = SFP_TYPE_UNKNOWN;
                    break;
            }
            return 0;
    }
#endif

    *sfp_type = trx_type;

    return 0;
}

int enet_ioctl_compat_ethctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
    struct ethctl_data rq_data;
    struct ethctl_data *ethctl = &rq_data;
    enetx_port_t *port = NULL;
    phy_dev_t *phy_dev;
    int ret;

    if (copy_from_user(ethctl, rq->ifr_data, sizeof(struct ethctl_data)))
        return -EFAULT;

    switch(ethctl->op)
    {
    case ETHGETSOFTWARESTATS:
        {
            char *buf;
            port = ((enetx_netdev *)netdev_priv(dev))->port;
            
            buf = kmalloc(ethctl->buf_size, GFP_KERNEL);
            display_software_stats(buf, ethctl->buf_size, port);
            buf[ethctl->buf_size-1] = 0;
            BCM_IOC_PTR_ZERO_EXT(ethctl->buf);
            if (copy_to_user(ethctl->buf, buf, ethctl->buf_size))
            {
                kfree(buf);
                return -EFAULT;
            }
            kfree(buf);
            return 0;
        }
    /* MII read/write functions for EGPHY compatible PHYs, will not work correctly on other PHY types */
    case ETHGETPHYID: /* Get address of MII PHY in use by dev */
        port = NETDEV_PRIV(dev)->port;

        if (port->port_class != PORT_CLASS_PORT || !port->p.phy)
            return -EINVAL;

        phy_dev = enet_dev_get_phy_dev(dev, ethctl->sub_port);
        if(!phy_dev)
            return -EFAULT;

        if (phy_dev->phy_drv->phy_type == PHY_TYPE_MAC2MAC)
            return -EINVAL;

        ethctl->phy_addr = phy_dev->addr;
        ethctl->flags = 0;

        /* Let us also return phy flags needed for accessing the phy */
        switch (phy_dev->phy_drv->phy_type) {
            case PHY_TYPE_SF2_SERDES:
                ethctl->flags = ETHCTL_FLAG_ACCESS_SERDES;
                break;
            case PHY_TYPE_XGAE:
                ethctl->flags = ETHCTL_FLAG_ACCESS_10GSERDES;
                break;
            default:
                break;
        }

        if (copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)))
            return -EFAULT;

        return 0;
    case ETHPHYMAP:
        {
            char *buf;

            ethctl->val = 0;
            BCM_IOC_PTR_ZERO_EXT(ethctl->buf);
            if (sw_print_mac_phy_info(root_sw, &buf, &ethctl->val) == 0)
                goto mapFail;

            if (copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)))
                goto freeMapFail;
            buf[ethctl->buf_size-1] = 0;
            if (copy_to_user(ethctl->buf, buf, ethctl->buf_size))
                goto freeMapFail;

            kfree(buf);
            return 0;
freeMapFail:
            kfree(buf);
mapFail:
            return -EFAULT;
        }

        case ETHCDGET:
        case ETHCDSET:
        case ETHCDRUN:
            ret = 0;
            if (ethctl->flags & INTERFACE_NEXT)
            {
                if(enet_get_next_phy(&dev, &ethctl->sub_port) == 0)
                {
                    ethctl->flags &= ~INTERFACE_NEXT;
                    goto cd_end;
                }
            }

            phy_dev = enet_dev_get_phy_dev(dev, ethctl->sub_port);
            if(!phy_dev)
                return -EFAULT;
            phy_dev = cascade_phy_get_last(phy_dev);
            ethctl->phy_addr = phy_dev->addr;


            if (!phy_dev_cable_diag_is_supported(phy_dev)) {
                ethctl->ret_val = CD_NOT_SUPPORTED;
                goto cd_end;
            }

            if(ethctl->op == ETHCDGET) {
                ethctl->ret_val = phy_dev_cable_diag_is_enabled(phy_dev)? CD_ENABLED: CD_DISABLED;
                goto cd_end;
            }

            if(ethctl->op == ETHCDSET) {
                phy_dev_cable_diag_set(phy_dev, ethctl->val);
                ethctl->ret_val = phy_dev_cable_diag_is_enabled(phy_dev)? CD_ENABLED: CD_DISABLED;
                goto cd_end;
            }

            if (phy_dev->link)
                ethctl->flags |= CD_LINK_UP;
            else
                ethctl->flags &= ~CD_LINK_UP;
            ret = phy_dev_cable_diag_run(phy_dev, &ethctl->ret_val, ethctl->pair_len);
cd_end:
            strcpy(ethctl->ifname, dev->name);
            if (copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)))
                return -EFAULT;
            return ret;

    case ETHGETMIIREG: /* Read MII PHY register */
    case ETHSETMIIREG: /* Write MII PHY register */
#if defined(DSL_DEVICES)
        {
            int ret = 0;
            phy_dev_t *phy_dev;

            phy_dev = phy_dev_get(PHY_TYPE_UNKNOWN, ethctl->phy_addr);
            if(!phy_dev)
                return -EFAULT;

            if (ethctl->flags == ETHCTL_FLAG_ACCESS_I2C_PHY)
            {
                phy_dev = phy_dev_get_by_i2c(ethctl->phy_addr);
                if (!phy_dev)
                    return -1;
            }

            if (!(ethctl->flags & (ETHCTL_FLAG_ACCESS_10GSERDES|ETHCTL_FLAG_ACCESS_10GPCS)) &&
                    (phy_dev == NULL || !phy_dev->phy_drv))
            {
                enet_err("No PHY at address %d has been found.\n", ethctl->phy_addr);
                return -EFAULT;
            }

            down(&root_sw->s.conf_sem);


            if (ethctl->flags == ETHCTL_FLAG_ACCESS_SERDES_POWER_MODE)
            {
                if (ethctl->op == ETHSETMIIREG)
                    ret = phy_dev->phy_drv->apd_set ?
                        phy_dev->phy_drv->apd_set(phy_dev, ethctl->phy_reg) : 0;
                else
                {
                    ret = phy_dev->phy_drv->apd_get ?
                        phy_dev->phy_drv->apd_get(phy_dev, &ethctl->val) : 0;
                }
            }
#if defined(PHY_SERDES_10G_CAPABLE)
            else if (ethctl->flags & ETHCTL_FLAG_ACCESS_10GSERDES)
            {
                if (ethctl->op == ETHGETMIIREG)
                {
                    ret = readXgaeSerdesReg(ethctl->phy_reg, &ethctl->val);
                }
                else
                {
                    ret = writeXgaeSerdesReg(ethctl->phy_reg, ethctl->val<<16);
                }
            }
            else if (ethctl->flags & ETHCTL_FLAG_ACCESS_10GPCS)
            {
                if (ethctl->op == ETHGETMIIREG)
                {
                    ret = readXgaePcsReg(ethctl->phy_reg, &ethctl->val);
                }
                else
                {
                    ret = writeXgaePcsReg(ethctl->phy_reg, ethctl->val<<16);
                }
            }
#endif
            else
            {
                if (ethctl->op == ETHGETMIIREG)
                    ret = ethsw_phy_exp_read32(phy_dev, ethctl->phy_reg, &ethctl->val);
                else
                    ret = ethsw_phy_exp_write(phy_dev, ethctl->phy_reg, ethctl->val);
            }

            up(&root_sw->s.conf_sem);

            if (ret) return ret;

            if (copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)))
                return -EFAULT;

            return 0;
        }
#elif defined(CONFIG_BCM_PON)
        {
            uint16_t val = ethctl->val;
            bus_drv_t *bus_drv = NULL;

            if (ethctl->flags == ETHCTL_FLAG_ACCESS_INT_PHY)
            {
#ifdef CONFIG_BCM96838
                bus_drv = bus_drv_get(BUS_TYPE_6838_EGPHY);
#endif
#ifdef CONFIG_BCM96848
                bus_drv = bus_drv_get(BUS_TYPE_6848_INT);
#endif
#ifdef CONFIG_BCM96858
                bus_drv = bus_drv_get(BUS_TYPE_6858_LPORT);
#endif
#ifdef CONFIG_BCM96856
                bus_drv = bus_drv_get(BUS_TYPE_6846_INT);
#endif
#ifdef CONFIG_BCM96846
                bus_drv = bus_drv_get(BUS_TYPE_6846_INT);
#endif
#ifdef CONFIG_BCM96878
                bus_drv = bus_drv_get(BUS_TYPE_6846_INT);
#endif
            }
            else if (ethctl->flags == ETHCTL_FLAG_ACCESS_EXT_PHY)
            {
#ifdef CONFIG_BCM96838
                bus_drv = bus_drv_get(BUS_TYPE_6838_EXT);
#endif
#ifdef CONFIG_BCM96848
                bus_drv = bus_drv_get(BUS_TYPE_6848_EXT);
#endif
#ifdef CONFIG_BCM96858
                bus_drv = bus_drv_get(BUS_TYPE_6858_LPORT);
#endif
#ifdef CONFIG_BCM96856
                bus_drv = bus_drv_get(BUS_TYPE_6846_EXT);
#endif
#ifdef CONFIG_BCM96846
                bus_drv = bus_drv_get(BUS_TYPE_6846_EXT);
#endif
#ifdef CONFIG_BCM96878
                bus_drv = bus_drv_get(BUS_TYPE_6846_EXT);
#endif
            }

            if (!bus_drv)
            {
                enet_err("cannot resolve phy bus driver for phy_addr %d, flags %d\n", ethctl->phy_addr, ethctl->flags);
                return -EFAULT;
            }

            if (ethctl->op == ETHGETMIIREG)
            {
                if (bus_read(bus_drv, ethctl->phy_addr, ethctl->phy_reg, &val))
                    return -EFAULT;

                ethctl->val = val;
                enet_dbg("get phy_id: %d; reg_num = %d; val = 0x%x \n", ethctl->phy_addr, ethctl->phy_reg, val);

                if (copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)))
                    return -EFAULT;
            }
            else
            {
                if (bus_write(bus_drv, ethctl->phy_addr, ethctl->phy_reg, ethctl->val))
                    return -EFAULT;

                enet_dbg("set phy_id: %d; reg_num = %d; val = 0x%x \n", ethctl->phy_addr, ethctl->phy_reg, ethctl->val);
            }

            return 0;
        }
#endif /* defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96858) */
    case ETHMOVESUBPORT:
        {
            port = ((enetx_netdev *)netdev_priv(dev))->port;
            ethctl->ret_val = 0;

            if (port == root_sw) {  // get next interface-phy mapping
                struct tr_portphy_info info = {};
                strcpy(info.ifname, ethctl->ifname);
                if (port_traverse_ports(root_sw, tr_get_next_portphy_info, PORT_CLASS_PORT, &info) <= 0)
                    return -ENODATA;

                ethctl->ret_val = info.mapping << BP_CROSSBAR_PORT_BASE;
                strcpy(ethctl->ifname, info.ifname);
            } else { // move operation
                struct tr_subport_info info = {};
                info.external_endpoint = BP_PHY_PORT_TO_CROSSBAR_PORT(ethctl->sub_port);

                if (dev->flags & IFF_UP) {
                    ethctl->ret_val = ETHMOVESUBPORT_RET_DSTDEV_UP;     // err: dest dev is not ifconfig down
                } else if (port_traverse_ports(root_sw, tr_get_subport_info, PORT_CLASS_PORT, &info) <= 0) {
                    ethctl->ret_val = ETHMOVESUBPORT_RET_INVALID_EP;    // err: subport not valid or not currently assigned
                } else if (info.port->dev->flags & IFF_UP) {
                    strcpy(ethctl->ifname, info.port->dev->name);
                    ethctl->ret_val = ETHMOVESUBPORT_RET_SRCDEV_UP;     // err: src dev is not ifconfig down
                } else if (IsMacToMac(info.phy->meta_id)) {
                    ethctl->ret_val = ETHMOVESUBPORT_RET_MAC2MAC;       // err: can't move MAC2MAC connection
                } else if (!crossbar_phy_moveable(port->p.phy, info.phy, info.external_endpoint)) {
                    ethctl->ret_val = ETHMOVESUBPORT_RET_NOT_MOVEABLE;  // err: can't move non-moveable connection
                } else {
                    strcpy(ethctl->ifname, info.port->dev->name);
                    // do actual moving ....
                    crossbar_phy_del(info.port->p.phy, info.phy);

                    info.phy->sw_port = port;
                    crossbar_phy_add(port->p.phy, info.phy, info.external_endpoint);
                    crossbar_finalize();
                }
            }

            return copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)) ? -EFAULT : 0;
        }
    case ETHGETPHYPWR:
        phy_dev = enet_dev_get_phy_dev(dev, ethctl->sub_port);
        if (!phy_dev)
            return -EFAULT;
        phy_dev_power_get(phy_dev, &ethctl->ret_val);
        return copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)) ? -EFAULT : 0;
    case ETHSETPHYPWRON:
    case ETHSETPHYPWROFF:
        phy_dev = enet_dev_get_phy_dev(dev, ethctl->sub_port);
        if (!phy_dev)
            return -EFAULT;
        return phy_dev_power_set(phy_dev, ethctl->op == ETHSETPHYPWRON) ? -EFAULT : 0;

#if defined(CONFIG_BCM_ENERGY_EFFICIENT_ETHERNET)
    case ETHGETPHYEEE:
        phy_dev = enet_dev_get_phy_dev(dev, ethctl->sub_port);
        if (!phy_dev)
            return -EFAULT;
        cascade_phy_dev_eee_get(phy_dev, &ethctl->ret_val);
        return copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)) ? -EFAULT : 0;
    case ETHGETPHYEEERESOLUTION:
        phy_dev = enet_dev_get_phy_dev(dev, ethctl->sub_port);
        if (!phy_dev)
            return -EFAULT;
        cascade_phy_dev_eee_resolution_get(phy_dev, &ethctl->ret_val);
        return copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)) ? -EFAULT : 0;
    case ETHSETPHYEEEON:
    case ETHSETPHYEEEOFF:
        phy_dev = enet_dev_get_phy_dev(dev, ethctl->sub_port);
        if (!phy_dev)
            return -EFAULT;
        return cascade_phy_dev_eee_set(phy_dev, ethctl->op == ETHSETPHYEEEON ? 1 : 0);
#endif // #if defined(CONFIG_BCM_ENERGY_EFFICIENT_ETHERNET)
#if defined(CONFIG_BCM_ETH_PWRSAVE)
    case ETHGETPHYAPD:
        phy_dev = enet_dev_get_phy_dev(dev, ethctl->sub_port);
        if (!phy_dev)
            return -EFAULT;
        phy_dev_apd_get(phy_dev, &ethctl->ret_val);
        return copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)) ? -EFAULT : 0;
    case ETHSETPHYAPDON:
    case ETHSETPHYAPDOFF:
        phy_dev = enet_dev_get_phy_dev(dev, ethctl->sub_port);
        if (!phy_dev)
            return -EFAULT;
        return cascade_phy_dev_apd_set(phy_dev, ethctl->op == ETHSETPHYAPDON ? 1 : 0);
#endif // #if defined(CONFIG_BCM_ETH_PWRSAVE)
    case ETHG9991CARRIERON:
    case ETHG9991CARRIEROFF:
        {
            char ifname[IFNAMSIZ];

            sprintf(ifname, "sid%d", ethctl->val);

            if (get_port_by_if_name(ifname, &port))
                return -EFAULT;

            if (port->port_type != PORT_TYPE_G9991_PORT)
                return -EFAULT;

            port->p.phy->priv = (void *)((((unsigned long)port->p.phy->priv) & ~0x3) | (ethctl->op == ETHG9991CARRIERON)); /* Reuse and reset 2 LSB for state */
            phy_dev_link_change_notify(port->p.phy);
            return 0;
        }

    case ETHGETSFPINFO:
        eth_get_gpon_sfp_info((sfp_type_t *)&ethctl->ret_val);
        return copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)) ? -EFAULT : 0;

    case ETHPHYMACSEC:
    {
        macsec_api_data data = {};
        copy_from_user(&data, rq->ifr_data + sizeof(struct ethctl_data), sizeof(macsec_api_data));
        port = ((enetx_netdev *)netdev_priv(dev))->port;
        phy_dev_macsec_oper(port->p.phy, &data);
        return copy_to_user(rq->ifr_data + sizeof(struct ethctl_data), &data, sizeof(macsec_api_data)) ? -EFAULT : 0;
    }
    default:
        return -EOPNOTSUPP;
    }

    enet_dbgv(" dev=%s, cmd=%x(SIOCETHCTLOPS), unknown ops=%d\n", NETDEV_PRIV(dev)->port->obj_name, cmd, ethctl->op);
    return -EOPNOTSUPP;
}

static int tr_get_soft_sw_map(enetx_port_t *port, void *_ctx)
{
    int *map = (int *)_ctx;

    if (port->dev && port->p.port_cap != PORT_CAP_MGMT &&  !PORT_IS_HW_FWD(port)) {
        *map |= 1<< PHYSICAL_PORT_TO_LOGICAL_PORT(port->p.mac->mac_id, PORT_ON_ROOT_SW(port)?0:1);
    }

    return 0;
}

static int tr_net_dev_hw_switching_set(enetx_port_t *port, void *_ctx)
{
    struct ethswctl_data *ethswctl = (struct ethswctl_data *)_ctx;

    if (port->n.port_netdev_role == PORT_NETDEV_ROLE_WAN || !port->dev)
        return 0;

    if (ethswctl->type == TYPE_ENABLE)
        PORT_SET_HW_FWD(port);
    else
        PORT_CLR_HW_FWD(port);
    return 0;
}

static int _enet_ioctl_hw_switching_set(struct ethswctl_data *ethswctl)
{
#if defined(SF2_DEVICE)
    enetx_port_t *sw = sf2_sw;
#else
    enetx_port_t *sw = root_sw;
#endif
    int rc = 0;

    if (!sw || !sw->s.ops->hw_sw_state_set || !sw->s.ops->hw_sw_state_get)
    {
        enet_err("Switch does not support hw_switching op\n");
        return -1;
    }
    switch (ethswctl->type)
    {
        case TYPE_ENABLE:
        case TYPE_DISABLE:
            rc = sw->s.ops->hw_sw_state_set(sw, ethswctl->type == TYPE_ENABLE);
            if (!rc)
                port_traverse_ports(sw, tr_net_dev_hw_switching_set, PORT_CLASS_PORT, ethswctl);
            break;
        case TYPE_GET:
            ethswctl->status = sw->s.ops->hw_sw_state_get(sw);
            break;
        default:
            rc = -1;
    }
    if (rc)
    {
        enet_err("Flooding configuration failed, err %d\n", rc);
        return -EFAULT;
    }

    return 0;
}

static void _enet_ioctl_hw_switch_flag_set(struct ethswctl_data *ethswctl)
{
#if !defined(DSL_DEVICES) && defined(CONFIG_BCM_PON_XRDP)
    enetx_port_t *sw = root_sw;

    port_traverse_ports(sw, tr_net_dev_hw_switching_set, PORT_CLASS_PORT, ethswctl);

    return;
#endif
}

#define ethswctl_field_copy_to_user(f)      \
    copy_to_user((void*)((char*)rq->ifr_data +((char*)&(ethswctl->f) - (char*)&rq_data)), &(ethswctl->f), sizeof(ethswctl->f))

#define ethswctl_field_len_copy_to_user(f,l)      \
    copy_to_user((void*)((char*)rq->ifr_data +((char*)&(ethswctl->f) - (char*)&rq_data)), &(ethswctl->f), ethswctl->l)

int enet_ioctl_compat_ethswctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
    struct ethswctl_data rq_data;
    struct ethswctl_data *ethswctl = &rq_data;
    phy_dev_t *phy_dev;
    enetx_port_t *port = NULL;
    int i;
#if defined(DSL_DEVICES)
    int ret;
#endif

    if (copy_from_user(ethswctl, rq->ifr_data, sizeof(struct ethswctl_data)))
        return -EFAULT;

    switch(ethswctl->op)
    {
        case ETHPORTMCASTGEMSET:
            if (gpon_mcast_gem_set(ethswctl->val))
                return -EFAULT;

            return 0;
        case ETHPORTCREATE:
            if (_handle_add_port(&ethswctl->net_port))
                return -EFAULT;

            if (copy_to_user((void *)rq->ifr_data, ethswctl, sizeof(struct ethswctl_data)))
                return -EFAULT;

            return 0;
        case ETHPORTDELETE:
            if (_handle_del_port(ethswctl->net_port.port))
                return -EFAULT;

            return 0;
        case ETHSWPORTTRAFFICCTRL:
            {
                if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
                    return -EFAULT;

                if (ethswctl->type == TYPE_GET)
                {
                    enet_err("%s ETHSWPORTTRAFFICCTRL: Unsupported request\n", __FUNCTION__);
                    return -EFAULT;
                }

                if (ethswctl->type == TYPE_SET)
                {
                    if (ethswctl->val)
                        port_generic_stop(port);
                    else
                        port_generic_open(port);
                }

                return 0;
            }

        case ETHSWDUMPMIB:
            {
                if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
                    return -EFAULT;
                return port_mib_dump(port, ethswctl->type);
            }

#if defined(SF2_DEVICE)
#if !defined(SF2_EXTERNAL)
        case ETHSWDUMPPAGE:
            /* only support ext sw and page 0 */
            if ((ethswctl->unit == SF2_ETHSWCTL_UNIT) && (ethswctl->page == 0))
                ioctl_extsw_dump_page0();
            return 0;
        case ETHSWACBCONTROL:
            BCM_IOC_PTR_ZERO_EXT(ethswctl->vptr);
            return ioctl_extsw_cfg_acb(ethswctl);
#endif //!SF2_EXTERNAL
        case ETHSWCONTROL:
            if (ethswctl->unit != SF2_ETHSWCTL_UNIT || !sf2_sw) return -(EOPNOTSUPP);

            ret = ioctl_extsw_control(ethswctl);
            if (!ret && copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                return -EFAULT;

            return ret;
        case ETHSWPRIOCONTROL:
            if (ethswctl->unit != SF2_ETHSWCTL_UNIT || !sf2_sw) return -(EOPNOTSUPP);

            ret = ioctl_extsw_prio_control(ethswctl);
            if (!ret && copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                return -EFAULT;

            return ret;
        case ETHSWQUEMAP:
            if (!sf2_sw) return -(EOPNOTSUPP);
            ret = ioctl_extsw_que_map(ethswctl);
            if (ret) return ret;
            if (copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                return -EFAULT;

            return ret;
        case ETHSWQUEMON:
            if (ethswctl->unit != SF2_ETHSWCTL_UNIT || !sf2_sw) return -(EOPNOTSUPP);

            ret = ioctl_extsw_que_mon(ethswctl);
            if (!ret && copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                return -EFAULT;

            return ret;
        case ETHSWMACLMT:
            if (ethswctl->unit != SF2_ETHSWCTL_UNIT || !sf2_sw) return -(EOPNOTSUPP);

            ret = ioctl_extsw_maclmt(ethswctl);
            if (!ret && (ethswctl->type == TYPE_GET) && ethswctl_field_copy_to_user(val))
                return -EFAULT;

            return ret;
#if defined(SF2_CFP)
        case ETHSWCFP:
            ioctl_extsw_cfp(ethswctl);
            if (ethswctl_field_copy_to_user(cfpArgs))
                return -EFAULT;
            return 0;
#endif
        case ETHSWPBVLAN:
            if (ethswctl->unit != SF2_ETHSWCTL_UNIT || !sf2_sw) return -(EOPNOTSUPP);

            ret = ioctl_extsw_pbvlan(ethswctl);
            if (!ret && (ethswctl->type == TYPE_GET))
                if (ethswctl_field_copy_to_user(fwd_map))
                    return -EFAULT;

            return ret;
        case ETHSWMIRROR:
            if (ethswctl->unit != SF2_ETHSWCTL_UNIT || !sf2_sw) {
                return -(EOPNOTSUPP);
            }

            ret = ioctl_extsw_port_mirror_ops(ethswctl);
            if (!ret && (ethswctl->type == TYPE_GET))
                if (ethswctl_field_copy_to_user(port_mirror_cfg))
                    return -EFAULT;
            return ret;
        case ETHSWPORTTRUNK:
            if (ethswctl->unit != SF2_ETHSWCTL_UNIT || !sf2_sw) return -(EOPNOTSUPP);

            ret = ioctl_extsw_port_trunk_ops(ethswctl);
            if (!ret && (ethswctl->type == TYPE_GET))
                if (ethswctl_field_copy_to_user(port_trunk_cfg))
                    return -EFAULT;
            return ret;
        case ETHSWREGACCESS:
            if (ethswctl->unit != SF2_ETHSWCTL_UNIT || !sf2_sw) return -(EOPNOTSUPP);
            if ((ethswctl->offset & IS_PHY_ADDR_FLAG) &&
                    !(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->offset & PORT_ID_M)))
                return BCM_E_ERROR;
            ret = ioctl_extsw_regaccess(ethswctl, port);
            if (!ret && ethswctl->length)
                if (ethswctl_field_len_copy_to_user(data[0], length))
                    return -EFAULT;

            return ret;

        case ETHSWPSEUDOMDIOACCESS:
            ret = ioctl_extsw_pmdioaccess(ethswctl);
            if (copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                return -EFAULT;
            return ret;

        case ETHSWINFO:
            if (!sf2_sw) return -(EOPNOTSUPP);
            if ((ret = ioctl_extsw_info(ethswctl)) >= 0) {
                if (copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                    return -EFAULT;
            } else
                enet_dbg("ETHSWINFO: error ret=%d\n", ret);

            return ret;

        case ETHSWCOSSCHED:
            /* only valid for ports on external SF2 switch */
            if (ethswctl->unit != SF2_ETHSWCTL_UNIT || !sf2_sw) return -(EOPNOTSUPP);

            if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
                return -EFAULT;

            ret = ioctl_extsw_cosq_sched(port, ethswctl);
            if (ret >= 0)
            {
                if (ethswctl->type == TYPE_GET)
                {
                    ethswctl->ret_val = ethswctl->val;
                }
                if (copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                    return -EFAULT;
                return 0;
            }

            enet_dbg("ETHSWCOSSCHED: error ret=%d\n", ret);
            return ret;
        case ETHSWCOSPORTMAP:
            if (ethswctl->unit != SF2_ETHSWCTL_UNIT || !sf2_sw) return -(EOPNOTSUPP);

            ret = ioctl_extsw_cosq_port_mapping(ethswctl);
            if (ret < 0) return ret;
            if (ethswctl->type == TYPE_GET) {
                ethswctl->queue = ret;  /* queue returned from function. Return value to user */
                if (ethswctl_field_copy_to_user(queue))
                    return -EFAULT;
                ret = 0;
            }

            return ret;

        case ETHSWVLAN:
            if (!sf2_sw) return -(EOPNOTSUPP);
            ret = ioctl_extsw_vlan(ethswctl);
            if (ret < 0) return ret;
            if (ethswctl->type == TYPE_GET)
                if (ethswctl_field_copy_to_user(fwd_map) || ethswctl_field_copy_to_user(untag_map))
                    return -EFAULT;
            return ret;

        case ETHSWARLACCESS:
            if (!sf2_sw) return -(EOPNOTSUPP);
            ret = ioctl_extsw_arl_access(ethswctl);
            if (!ret && copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                return -EFAULT;
            return ret;

// add by Andrew
        case ETHSWARLDUMP:
            if (!sf2_sw) return -(EOPNOTSUPP);
            ret = ioctl_extsw_arl_dump(ethswctl);
            if (!ret && copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                return -EFAULT;
            return ret;

      case ETHSWMIBDUMP:
            if (!sf2_sw) return -(EOPNOTSUPP);
            if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
                return -EFAULT;
            ret = port_mib_dump_us(port, ethswctl);
            if (!ret && copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data))) 
                return -EFAULT;
            return ret;
// end of add

        case ETHSWCOSPRIORITYMETHOD:
            if (ethswctl->unit != SF2_ETHSWCTL_UNIT  || !sf2_sw) {
                enet_err("runner COS priority method config not supported yet.\n");    //TODO_DSL?
                return -(EOPNOTSUPP);
            }

            ret = ioctl_extsw_cos_priority_method_cfg(ethswctl);
            if (!ret && (ethswctl->type == TYPE_GET))
                if (ethswctl_field_copy_to_user(ret_val))
                    return -EFAULT;
            return ret;
        case ETHSWJUMBO:
            if (ethswctl->unit != SF2_ETHSWCTL_UNIT || !sf2_sw) return -(EOPNOTSUPP);

            ret = ioctl_extsw_port_jumbo_control(ethswctl);
            if (!ret && ethswctl_field_copy_to_user(ret_val))
                return -EFAULT;
            return ret;

        case ETHSWCOSPCPPRIOMAP:
            if (ethswctl->unit != SF2_ETHSWCTL_UNIT || !sf2_sw) return 0;

            ret = ioctl_extsw_pcp_to_priority_mapping(ethswctl);
            if (!ret && (ethswctl->type == TYPE_GET))
                if (ethswctl_field_copy_to_user(priority))
                    return -EFAULT;
            return ret;
        case ETHSWCOSPIDPRIOMAP:
            if (ethswctl->unit != SF2_ETHSWCTL_UNIT || !sf2_sw) {
                enet_err("runner COS PID priority mapping not supported yet.\n");    //TODO_DSL?
                return -(EOPNOTSUPP);
            }

            ret = ioctl_extsw_pid_to_priority_mapping(ethswctl);
            if (!ret && (ethswctl->type == TYPE_GET))
                if (ethswctl_field_copy_to_user(priority))
                    return -EFAULT;
            return ret;
        case ETHSWCOSDSCPPRIOMAP:
            if (ethswctl->unit != SF2_ETHSWCTL_UNIT || !sf2_sw) return 0;

            ret = ioctl_extsw_dscp_to_priority_mapping(ethswctl);
            if (!ret && (ethswctl->type == TYPE_GET))
                if (ethswctl_field_copy_to_user(priority))
                    return -EFAULT;
            return ret;

        case ETHSWPORTSHAPERCFG:
            if (ethswctl->unit != SF2_ETHSWCTL_UNIT || !sf2_sw) return -(EOPNOTSUPP);

            return ioctl_extsw_port_shaper_config(ethswctl);
        case ETHSWDOSCTRL:
            if (ethswctl->unit != SF2_ETHSWCTL_UNIT || !sf2_sw) return -(EOPNOTSUPP);

            ret = ioctl_extsw_dos_ctrl(ethswctl);
            if (!ret && (ethswctl->type == TYPE_GET))
                if (ethswctl_field_copy_to_user(dosCtrl))
                    return -EFAULT;
            return ret;

        case ETHSWHWSTP:
            if (!sf2_sw) return -(EOPNOTSUPP);
            if (ethswctl->type == TYPE_GET)
            {
                ethswctl->status = root_sw->s.stpDisabledPortMap;   // TODO_DSL? bridge_stp_handler ???
                if (ethswctl_field_copy_to_user(status))
                    return -EFAULT;
                return 0;
            }
            else if ((ethswctl->type == TYPE_ENABLE) || (ethswctl->type == TYPE_DISABLE))
            {
                if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
                    return -EFAULT;
                if (port->p.ops->stp_set)
                {
                    port->p.ops->stp_set(port, (ethswctl->type == TYPE_ENABLE)?STP_MODE_ENABLE:STP_MODE_DISABLE, STP_STATE_UNCHANGED);
                    if ((ethswctl->type == TYPE_ENABLE) && (port->dev->flags & IFF_UP))
                    {
                        dev_change_flags(port->dev, (port->dev->flags & ~IFF_UP));
                        dev_change_flags(port->dev, (port->dev->flags | IFF_UP));
                    }
                }
                return 0;
            }
            else
                return -EFAULT;

        case ETHSWIFSTP:
            if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
                return -EFAULT;
            if (!port->p.ops->stp_set) return -(EOPNOTSUPP);

            if (ethswctl->type == TYPE_SET)
            {
                port->p.ops->stp_set(port, STP_MODE_UNCHANGED, ethswctl->val);
                return 0;
            }
            else if (ethswctl->type == TYPE_GET)
            {
                ethswctl->val = port->p.ops->stp_set(port, STP_MODE_UNCHANGED, STP_STATE_UNCHANGED);
                if (ethswctl_field_copy_to_user(val))
                    return -EFAULT;
                return 0;
            }
            else
                return -EFAULT;

        case ETHSWPORTSTORMCTRL:
            if (ethswctl->unit != SF2_ETHSWCTL_UNIT || !sf2_sw) return -(EOPNOTSUPP);

            ret = ioctl_extsw_port_storm_ctrl(ethswctl);
            if (!ret && copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                return -EFAULT;
            return ret;

        case ETHSWMULTIPORT:
            if (ethswctl->unit == SF2_ETHSWCTL_UNIT || !sf2_sw) return BCM_E_NONE;

            if (ethswctl->type == TYPE_SET) {
                ioctl_extsw_set_multiport_address(ethswctl->mac);
            }
            return BCM_E_NONE;
#endif // SF2_DEVICE
        case ETHSWTEST1:
            // actually no op, but required to return without error
            ethswctl->ret_val = 0;
            if (ethswctl_field_copy_to_user(ret_val))
                return -EFAULT;

            return 0;
#if defined(DSL_DEVICES)


        case ETHSWEXTPHYLINKSTATUS:
            if ((ethswctl->type == TYPE_SET) &&
                    (ethswctl->speed == 0) && (ethswctl->duplex == 0))  // will not support speed, duplex change thru this ioctl
            {
                if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
                    return -EFAULT;
                return ioctl_ext_phy_link_set(port, ethswctl->status);
            }
            return -EOPNOTSUPP;

#if defined(CONFIG_BCM_ETH_DEEP_GREEN_MODE)
        case ETHSWDEEPGREENMODE:
            if ( ethswctl->type == TYPE_GET )
            {
                ethswctl->val = ioctl_pwrmngt_get_deepgreenmode(ethswctl->val);
                if (ethswctl_field_copy_to_user(val))
                    return -EFAULT;
            }
            else
                ioctl_pwrmngt_set_deepgreenmode(ethswctl->val);
            return 0;
#endif
        case ETHSWPORTLOOPBACK:
            if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
            {
                enet_err("failed to get port_obj from unit_port %d\n", ethswctl->port);
                return -EFAULT;
            }

            if (!(phy_dev = enet_dev_get_phy_dev(port->dev, ethswctl->sub_port)))
            {
                enet_err("invalid sub port specified\n");
                return -EFAULT;
            }
            phy_dev = cascade_phy_get_last(phy_dev);
            if (ethswctl->type == TYPE_SET)
            {
                if ((ret = phy_loopback_set(phy_dev, ethswctl->val,
                    phy_mbps_2_speed(ethswctl->cfgSpeed))) < 0)
                    return ret;
            }
            else
            {
                ethswctl->cfgSpeed = 0;
                ethswctl->ret_val = 0;
                if ((ret = phy_loopback_get(phy_dev, &ethswctl->ret_val, (phy_speed_t *)&ethswctl->cfgSpeed)) < 0)
                    return ret;

                ethswctl->cfgSpeed = phy_speed_2_mbps(ethswctl->cfgSpeed);
                if (ethswctl_field_copy_to_user(cfgSpeed) || ethswctl_field_copy_to_user(ret_val))
                    return -EFAULT;
            }
            return 0;
        case ETHSWKERNELPOLL:
            ret = ioctl_ethsw_kernel_poll(ethswctl);

            if (ethswctl_field_copy_to_user(mdk_kernel_poll.link_change))
                return -EFAULT;

            return ret;
#else /* !defined(DSL_DEVICES) */
        case ETHSWPORTLOOPBACK:
            {
                mac_loopback_t loopback_op;
                int loopback_type = 0;
                int val = 0;

                enet_dbg("ethswctl ETHSWPORTLOOPBACK : ioctl\n");

                if (ethswctl->type == TYPE_SET)
                {
                    if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
                    {
                        enet_err("failed to get port_obj from unit_port %d\n", ethswctl->port);
                        return -EFAULT;
                    }

                    if (!port->p.mac)
                    {
                        enet_err("failed to get mac_dev\n");
                        return -EFAULT;
                    }

                    val = ethswctl->val;

                    loopback_op = val & 0x000000FF;
                    loopback_type = (val & 0x0000FF00) >> 8;

                    if (loopback_type == 0)
                    {
                        mac_dev_loopback_set(port->p.mac, MAC_LOOPBACK_NONE);
                    }
                    else
                    {
                        mac_dev_loopback_set(port->p.mac, loopback_op);
                    }
                }

                return 0;
            }

#endif /* !defined(DSL_DEVICES) */
        case ETHSWPORTRXRATE:
#if defined(DSL_DEVICES)
            if (ethswctl->unit != SF2_ETHSWCTL_UNIT) return -(EOPNOTSUPP);

            if (ethswctl->type == TYPE_GET)
            {
                ret = ioctl_extsw_port_irc_get(ethswctl);
                if (!ret && (ethswctl_field_copy_to_user(limit) || ethswctl_field_copy_to_user(burst_size)))
                    return -EFAULT;
            }
            else
                ret = ioctl_extsw_port_irc_set(ethswctl);
            return ret;
#else /* !defined(DSL_DEVICES) */
            {
                bdmf_object_handle port_obj;
                rdpa_port_flow_ctrl_t flowctl_cfg;

                enet_dbg("ethswctl ETHSWPORTRXRATE : ioctl\n");

                /* it's about LAN ports */
                port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port);

                if (!(port_obj = _port_rdpa_object_by_port(port)))
                {
                    enet_err("failed to get port_obj for %s\n", port->obj_name);
                    return -EFAULT;
                }

                if (rdpa_port_flow_control_get(port_obj , &flowctl_cfg))
                {
                    enet_err("failed to get port_obj for %s\n", port->obj_name);
                    return -EFAULT;
                }

                if (ethswctl->type == TYPE_GET)
                {
                    enet_dbg("ethswctl->type : GET\n");

                    ethswctl->limit = flowctl_cfg.rate/1000;           /* bps -> kbps */
                    ethswctl->burst_size = (flowctl_cfg.mbs*8)/1000;   /* bytes -> kbits */

                    if (ethswctl_field_copy_to_user(limit) || ethswctl_field_copy_to_user(burst_size))
                        return -EFAULT;
                }
                else if (ethswctl->type == TYPE_SET)
                {
                    enet_dbg("ethswctl->type : SET  limit=%u burst_size=%u \n", ethswctl->limit, ethswctl->burst_size);

#ifdef CONFIG_BCM96846
#define PEAK_RATE 1000000000UL
#else
#define PEAK_RATE 10000000000UL
#endif

                    if(ethswctl->limit != 0)
                        memcpy(flowctl_cfg.src_address.b, port->dev->dev_addr, ETH_ALEN);
                    else
                        memset(flowctl_cfg.src_address.b, 0, ETH_ALEN);

                    flowctl_cfg.rate = ethswctl->limit*1000;            /* kbps -> bps per second */
                    flowctl_cfg.mbs = (ethswctl->burst_size*1000)/8;    /* kbits->bits->bytes */
                    if (flowctl_cfg.mbs < 20000)
                        flowctl_cfg.mbs = 20000;                         /* mbs/2 > jumbo packet*/
                    flowctl_cfg.threshold = flowctl_cfg.mbs/2;          /* kbits->bits->bytes, a half of mbs */

                    if (rdpa_port_flow_control_set(port_obj, &flowctl_cfg))
                    {
                        enet_err("failed to set rdpa port flow control data %s\n", port->obj_name);
                        return -EFAULT;
                    }
                }

                return 0;
            }
#endif /* !defined(DSL_DEVICES) */
        case ETHSWPORTTXRATE:
#if defined(DSL_DEVICES)
            if (ethswctl->unit != SF2_ETHSWCTL_UNIT) return -(EOPNOTSUPP);

            BCM_IOC_PTR_ZERO_EXT(ethswctl->vptr);
            ret = ioctl_extsw_port_erc_config(ethswctl);
            if (!ret && !ethswctl->vptr && (ethswctl->type == TYPE_GET) &&
                    (ethswctl_field_copy_to_user(limit) || ethswctl_field_copy_to_user(burst_size)))
                return -EFAULT;
            return ret;
#else /* !defined(DSL_DEVICES) */
            {
                bdmf_object_handle port_obj = NULL;
                rdpa_port_tm_cfg_t  port_tm_cfg;
                rdpa_tm_rl_cfg_t  tm_rl_cfg = {0};

                if (ethswctl->type == TYPE_SET)
                {
                    port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port);

                    if (!(port_obj = _port_rdpa_object_by_port(port)))
                    {
                        enet_err("failed to get port_obj for %s\n", port->obj_name);
                        return -EFAULT;
                    }

                    if (rdpa_port_tm_cfg_get(port_obj, &port_tm_cfg))
                    {
                        enet_err("rdpa_port_tm_cfg_get failed\n");
                        return -EFAULT;
                    }

                    if (!port_tm_cfg.sched)
                    {
                        enet_err("can't configure rate limiter\n");
                        return -EFAULT;
                    }

                    tm_rl_cfg.af_rate = ethswctl->limit*1000;
                    if (rdpa_egress_tm_rl_set(port_tm_cfg.sched, &tm_rl_cfg) != 0)
                    {
                        enet_err("rdpa_egress_tm_rl_set failed\n");
                        return -EFAULT;
                    }
                }
                else
                {
                    enet_err("%s ETHSWPORTTXRATE: Unsupported request\n", __FUNCTION__);
                    return -EFAULT;
                }

                return 0;
            }
#endif /* !defined(DSL_DEVICES) */
        case ETHSWEMACCLEAR:
            {
                if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
                    return -EFAULT;

                if (!port->p.mac)
                    return -EFAULT;

                if (mac_dev_stats_clear(port->p.mac))
                    return -EFAULT;

                return 0;
            }
        case ETHSWEMACGET:
            {
                mac_stats_t mac_stats = {0};

                if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
                    return -EFAULT;

                if (!port->p.mac)
                    return -EFAULT;

                if (mac_dev_stats_get(port->p.mac, &mac_stats))
                    return -EFAULT;

                mac_dev_stats_to_emac_stats(&ethswctl->emac_stats_s, &mac_stats);
                if (ethswctl_field_copy_to_user(emac_stats_s))
                    return -EFAULT;

                return 0;
            }

#ifdef RUNNER
        case ETHSWRDPAPORTGET:
            port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port);
            ethswctl->val = (port)? port->p.port_id : rdpa_if_none;

            //enet_dbgv("ETHSWRDPAPORTGET: u=%d p=%d > rdpa_if=%d\n", ethswctl->unit, ethswctl->port, ethswctl->val);
            if (ethswctl_field_copy_to_user(val))
                return -EFAULT;

            return 0;
#endif

        case ETHSWPORTPAUSECAPABILITY:
            enet_dbg("ethswctl ETHSWPORTPAUSECAPABILITY ioctl: %s\n", ethswctl->type == TYPE_GET ? "get" : "set");
            if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
                return -EFAULT;

            if (ethswctl->type == TYPE_GET)
            {
                int rx_enable = 0, tx_enable = 0, pfc_rx_enable = 0, pfc_tx_enable = 0;
                    
                if (port_pause_get(port, &rx_enable, &tx_enable))
                    return -EFAULT;

                if (port_pfc_capable(port) && port_pfc_get(port, &pfc_rx_enable, &pfc_tx_enable))
                    return -EFAULT;

                if ((rx_enable || tx_enable) && (pfc_rx_enable || pfc_tx_enable))
                    return -EFAULT;
                else if (rx_enable == 0 && tx_enable== 0 && pfc_rx_enable == 0 && pfc_tx_enable == 0)
                    ethswctl->pfc_ret = PAUSE_FLOW_CTRL_NONE;
                else if (rx_enable || tx_enable) {
                    if (rx_enable)
                        ethswctl->pfc_ret = tx_enable ? PAUSE_FLOW_CTRL_BOTH : PAUSE_FLOW_CTRL_RX;
                    else
                        ethswctl->pfc_ret = PAUSE_FLOW_CTRL_TX;
                }
                else {
                    if (pfc_rx_enable)
                        ethswctl->pfc_ret = pfc_tx_enable ? PAUSE_FLOW_PFC_BOTH : PAUSE_FLOW_PFC_RX;
                    else {
                        ethswctl->pfc_ret = PAUSE_FLOW_PFC_TX;
                        for (i=0; i<8; i++)
                            if (port_pfc_tx_timer_get(port, i, &ethswctl->pfc_timer[i]))
                                return -EFAULT;
                    }
                }
                        
                if (ethswctl_field_copy_to_user(pfc_ret))
                    return -EFAULT;

                if (ethswctl_field_copy_to_user(pfc_timer))
                    return -EFAULT;

                return 0;
            }
            else if (ethswctl->type == TYPE_SET)
            {
                int rx_enable = (ethswctl->val == PAUSE_FLOW_CTRL_RX) || (ethswctl->val == PAUSE_FLOW_CTRL_BOTH);
                int tx_enable = (ethswctl->val == PAUSE_FLOW_CTRL_TX) || (ethswctl->val == PAUSE_FLOW_CTRL_BOTH);
                int pfc_rx_enable = (ethswctl->val == PAUSE_FLOW_PFC_RX) || (ethswctl->val == PAUSE_FLOW_PFC_BOTH);
                int pfc_tx_enable = (ethswctl->val == PAUSE_FLOW_PFC_TX) || (ethswctl->val == PAUSE_FLOW_PFC_BOTH);

                if (ethswctl->val > PAUSE_FLOW_PFC_RX)
                    return -EFAULT;

                if (port_pause_set(port, rx_enable, tx_enable))
                    return -EFAULT;

                if ((pfc_rx_enable || pfc_tx_enable) && !port_pfc_capable(port))
                    return -EFAULT;

                if (port_pfc_capable(port) && port_pfc_set(port, pfc_rx_enable, pfc_tx_enable))
                    return -EFAULT;

                return 0;
            }

            return -EFAULT;

        case ETHSWUNITPORT:
            {
                int u, p;
                struct net_device *dev;

                if (get_root_dev_by_name(ethswctl->ifname, &dev))
                    return -EFAULT;

                for (u = 0; u < BP_MAX_ENET_MACS; u++)
                {
                    for (p = 0; p < COMPAT_MAX_SWITCH_PORTS; p++)
                    {
                        if (!COMPAT_PORT(u, p) || COMPAT_PORT(u, p)->dev != dev)
                            continue;

                        ethswctl->unit = u;
                        ethswctl->port_map = 1 << p;
                        break;
                    }

                    if (p != COMPAT_MAX_SWITCH_PORTS)
                        break;
                }

                if (p == COMPAT_MAX_SWITCH_PORTS)
                    return -EFAULT;

                if (ethswctl_field_copy_to_user(unit) ||
                        ethswctl_field_copy_to_user(port_map))
                    return -EFAULT;

                //enet_dbgv("ETHSWUNITPORT: if=[%s] p=%d u=%d\n", ethswctl->ifname, p, u);
                return 0;
            }

        case ETHSWSTATPORTCLR:
            if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
                return -EFAULT;

            port_stats_clear(port);
            return 0;

#ifdef RUNNER
        case ETHSWCPUMETER:
            {
                if (ethswctl->type == TYPE_GET)
                {
                    enet_err("%s ETHSWCPUMETER: Unsupported request\n", __FUNCTION__);
                    return -EFAULT;
                }

                if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
                    return -EFAULT;

                if (configure_bc_rate_limit_meter(port->p.port_id, ethswctl->cpu_meter_rate_limit.rate_limit))
                {
                    enet_err("%s ETHSWCPUMETER: Configure CPU BC rate limit failed!\n", __FUNCTION__);
                    return -EFAULT;
                }

                return 0;
            }
        case ETHSWPORTSALDAL:
            {
                int rc;
                bdmf_object_handle port_obj;
                rdpa_port_dp_cfg_t cfg;

                if (ethswctl->type != TYPE_SET)
                    return -EFAULT;

                if (ethswctl->unit) /* unit == 0 for modifying lan ports, unit == 1 for rdpa_if_wan0 */
                {
                    if ((rc = rdpa_port_get(rdpa_if_wan0, &port_obj))) /* FIXME: MULTI-WAN XPON */
                        return -EFAULT;
                }
                else
                {
                    port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port);

                    if (!(port_obj = _port_rdpa_object_by_port(port)))
                        return -EFAULT;
                }

            if (!(rc = rdpa_port_cfg_get(port_obj , &cfg)))
            {
                cfg.sal_enable = cfg.dal_enable = ethswctl->sal_dal_en;
                if ((rc = rdpa_port_cfg_set(port_obj, &cfg)))
                {
                    enet_err("failed to set rdpa sal/dal port configuration on %s\n", port->obj_name);
                }
            }

            if (ethswctl->unit)
                bdmf_put(port_obj);

            return rc ? -EFAULT : 0;
        }
    case ETHSWPORTTRANSPARENT:
        {
            bdmf_object_handle port_obj;
            rdpa_port_vlan_isolation_t vlan_isolation;
            int rc;

            enet_dbg("ETHSWPORTTRANSPARENT ioctl");

            if (ethswctl->type != TYPE_SET)
                return -EFAULT;

            if (!(port_obj = COMPAT_RPDA(ethswctl->unit, ethswctl->port)))
                return -EFAULT;

            if ((rc = rdpa_port_transparent_set(port_obj, ethswctl->transparent)))
                return -EFAULT;

            rc = rdpa_port_vlan_isolation_get(port_obj, &vlan_isolation);
            if (rc >= 0)
            {
                vlan_isolation.ds = !ethswctl->transparent;
                vlan_isolation.us = !ethswctl->transparent;
                rc = rdpa_port_vlan_isolation_set(port_obj, &vlan_isolation);
            }

            return rc ? -EFAULT : 0;
        }
    case ETHSWPORTVLANISOLATION:
        {
            bdmf_object_handle port_obj;
            rdpa_port_vlan_isolation_t vlan_isolation;
            int rc;

            enet_dbg("ETHSWPORTVLANISOLATION ioctl");

            if (ethswctl->type != TYPE_SET)
                return -EFAULT;

            if (!(port_obj = COMPAT_RPDA(ethswctl->unit, ethswctl->port)))
                return -EFAULT;

            rc = rdpa_port_vlan_isolation_get(port_obj, &vlan_isolation);
            if (rc >= 0)
            {
                vlan_isolation.ds = ethswctl->vlan_isolation.ds_enable;
                vlan_isolation.us = ethswctl->vlan_isolation.us_enable;
                rc = rdpa_port_vlan_isolation_set(port_obj, &vlan_isolation);
            }

            return rc ? -EFAULT : 0;
        }
    case ETHSWRDPAPORTGETFROMNAME:
        {
            rdpa_if index;

            if (get_port_by_if_name(ethswctl->ifname, &port))
            {
                return -ENOTTY;			/* Inappropriate ioctl for device */
            }

            if (_port_rdpa_if_by_port(port, &index))
            {
                enet_err("cannot retreive an rdpa index for %s\n", ethswctl->ifname);
                return -ENOTTY;
            }

            ethswctl->val = index;

            if (ethswctl_field_copy_to_user(val))
                return -EFAULT;

            return 0;
        }
#endif
    case ETHSWGETIFNAME:
        if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
            return -EFAULT;

        if (!port->dev)
            return -EFAULT;

        strcpy(ethswctl->ifname, port->dev->name);
        if (ethswctl_field_copy_to_user(ifname))
            return -EFAULT;

        return 0;

    case ETHSWPHYCFG: /* get boardparams phyid value */
        if (!(port = _compat_port_object_from_unit_port(ethswctl->val, ethswctl->port)) || !port->p.phy)
            return -EFAULT;
        ethswctl->phycfg = port->p.phy->meta_id;

        if (ethswctl_field_copy_to_user(phycfg))
            return -EFAULT;

        return 0;

    case ETHSWSWITCHING:
        if (_enet_ioctl_hw_switching_set(ethswctl))
            return -EFAULT;

        if (ethswctl->type == TYPE_GET && ethswctl_field_copy_to_user(status))
            return -EFAULT;
        return 0;

    case ETHSWSWITCHFLAG:
        _enet_ioctl_hw_switch_flag_set(ethswctl);

        return 0;

    case ETHSWSOFTSWITCHING:    // turn on/off hw switching for a port
        if (ethswctl->type == TYPE_GET)
        {
            ethswctl->status = 0;
            port_traverse_ports(root_sw, tr_get_soft_sw_map, PORT_CLASS_PORT, &(ethswctl->status));
            if (ethswctl_field_copy_to_user(status))
                return -EFAULT;
            return 0;
        }
        else
        {
            if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
                return -EFAULT;

            // only allow enable/disable hw switching if port has interface, and port is role LAN, and on external switch
#if defined(SF2_DEVICE)
            if (!port->dev || port->n.port_netdev_role == PORT_NETDEV_ROLE_WAN || ethswctl->unit != SF2_ETHSWCTL_UNIT)
#else
            if (!port->dev || port->n.port_netdev_role == PORT_NETDEV_ROLE_WAN || ethswctl->unit == 0)
#endif
                return -EOPNOTSUPP;
            // use role_set to setup switch WAN for software switching, and LAN for hw switching
            if (ethswctl->type == TYPE_ENABLE) {
                PORT_CLR_HW_FWD(port);
                port->p.ops->role_set(port, PORT_NETDEV_ROLE_WAN);
                if (port->p.ops->fast_age)
                    port->p.ops->fast_age(port);
            }
            else {
                PORT_SET_HW_FWD(port);
                port->p.ops->role_set(port, PORT_NETDEV_ROLE_LAN);
            }
            return 0;
        }
        return -EOPNOTSUPP;

    case ETHSWLINKSTATUS:
        // enet_dbg("ETHSWLINKSTATUS\n");

        if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)) || !port->p.phy)
            return -EFAULT;

        if (ethswctl->type != TYPE_GET)
        {
#if defined(DSL_DEVICES)
            phy_dev_t *phy_dev = port->p.phy;
            phy_dev_read_status(phy_dev);
            link_change_handler(port, ethswctl->status, ethswctl->speed, ethswctl->duplex);
#else
            return -EFAULT;
#endif
        }
        else
        {
            ethswctl->status = port->p.phy ? port->p.phy->link : 1;
            if (ethswctl_field_copy_to_user(status))
                return -EFAULT;
        }

        return 0;

    case ETHSWPHYAUTONEG:
        {
            uint32_t adCaps;
            uint32_t localCaps;

            if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
                return -EFAULT;

            if (!port->p.phy)
            {
                enet_err("port - %s has no PHY connected \n", port->name);
                return -EFAULT;
            }

            if (ethswctl->type == TYPE_GET)
            {
                if (phy_dev_caps_get(port->p.phy, CAPS_TYPE_ADVERTISE, &adCaps))
                    return -EFAULT;

                ethswctl->autoneg_info = (adCaps & PHY_CAP_AUTONEG) ? 1 : 0;
                ethswctl->autoneg_ad |= AN_AUTONEG;
                ethswctl->autoneg_local |= AN_AUTONEG;
                
                if (port->p.phy->disable_hd)
                {
                    adCaps &= ~(PHY_CAP_10_HALF | PHY_CAP_100_HALF | PHY_CAP_1000_HALF);
                }

                if (adCaps & PHY_CAP_10_HALF)
                    ethswctl->autoneg_ad |= AN_10M_HALF;

                if (adCaps & PHY_CAP_10_FULL)
                    ethswctl->autoneg_ad |= AN_10M_FULL;

                if (adCaps & PHY_CAP_100_HALF)
                    ethswctl->autoneg_ad |= AN_100M_HALF;

                if (adCaps & PHY_CAP_100_FULL)
                    ethswctl->autoneg_ad |= AN_100M_FULL;

                if (adCaps & PHY_CAP_1000_HALF)
                    ethswctl->autoneg_ad |= AN_10M_HALF;

                if (adCaps & PHY_CAP_1000_FULL)
                    ethswctl->autoneg_ad |= AN_1000M_FULL;

                if (adCaps & PHY_CAP_10000)
                    ethswctl->autoneg_ad |= AN_10000;

                if (adCaps & PHY_CAP_PAUSE)
                    ethswctl->autoneg_ad |= AN_FLOW_CONTROL;

                if (adCaps & PHY_CAP_PAUSE_ASYM)
                    ethswctl->autoneg_ad |= AN_FLOW_CONTROL_ASYM;   
                
                if (phy_dev_caps_get(port->p.phy, CAPS_TYPE_SUPPORTED, &localCaps))
                    return -EFAULT;
                
                if (port->p.phy->disable_hd)
                {
                    localCaps &= ~(PHY_CAP_10_HALF | PHY_CAP_100_HALF | PHY_CAP_1000_HALF);
                }

                if (localCaps & PHY_CAP_10_HALF)
                    ethswctl->autoneg_local |= AN_10M_HALF;

                if (localCaps & PHY_CAP_10_FULL)
                    ethswctl->autoneg_local |= AN_10M_FULL;

                if (localCaps & PHY_CAP_100_HALF)
                    ethswctl->autoneg_local |= AN_100M_HALF;

                if (localCaps & PHY_CAP_100_FULL)
                    ethswctl->autoneg_local |= AN_100M_FULL;

                if (localCaps & PHY_CAP_1000_HALF)
                    ethswctl->autoneg_local |= AN_10M_HALF;

                if (localCaps & PHY_CAP_1000_FULL)
                    ethswctl->autoneg_local |= AN_1000M_FULL;

                if (localCaps & PHY_CAP_10000)
                    ethswctl->autoneg_local |= AN_10000;

                if (port->p.ops->pause_set)
                {
                    ethswctl->autoneg_local |= AN_FLOW_CONTROL;
                }
                                
                if (copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                    return -EFAULT;

                return 0;
            }
            else
            {
                if (port->port_type != PORT_TYPE_RUNNER_EPON)
                {
                    if (ethswctl->autoneg_info & AUTONEG_CTRL_MASK)
                            return phy_dev_init(port->p.phy) ? -EFAULT : 0;

                    if (phy_dev_caps_get(port->p.phy, CAPS_TYPE_ADVERTISE, &adCaps))
                        return -EFAULT;

                    adCaps &= ~PHY_CAP_AUTONEG;
                }
                else
                {
                    if (phy_dev_caps_get(port->p.phy, CAPS_TYPE_ADVERTISE, &adCaps))
                        return -EFAULT;

                    if (ethswctl->autoneg_info & AUTONEG_CTRL_MASK)
                        adCaps |= PHY_CAP_AUTONEG;
                    else
                        adCaps &= ~PHY_CAP_AUTONEG;
                }

                return phy_dev_caps_set(port->p.phy, adCaps) ? -EFAULT : 0;
            }
        }

    case ETHSWPHYAUTONEGCAPADV:
        {
            uint32_t caps;

            if (ethswctl->type != TYPE_SET)
                return -EFAULT;

            if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
                return -EFAULT;

            if (!port->p.phy)
            {
                enet_err("port - %s has no PHY connected \n", port->name);
                return -EFAULT;
            }

            if (phy_dev_caps_get(port->p.phy, CAPS_TYPE_ADVERTISE, &caps))
                return -EFAULT;

            caps &= ~(PHY_CAP_10_HALF | PHY_CAP_10_FULL | PHY_CAP_100_HALF | PHY_CAP_100_FULL | PHY_CAP_1000_HALF |
                PHY_CAP_1000_FULL | PHY_CAP_10000 | PHY_CAP_PAUSE | PHY_CAP_PAUSE_ASYM );

            caps |= PHY_CAP_AUTONEG;

            if (ethswctl->autoneg_ad & AN_FLOW_CONTROL)
                caps |= PHY_CAP_PAUSE;

            if (ethswctl->autoneg_ad & AN_FLOW_CONTROL_ASYM)
                caps |= PHY_CAP_PAUSE_ASYM;

            if (ethswctl->autoneg_ad & AN_10M_HALF)
                caps |= PHY_CAP_10_HALF;

            if (ethswctl->autoneg_ad & AN_10M_FULL)
                caps |= PHY_CAP_10_FULL;

            if (ethswctl->autoneg_ad & AN_100M_HALF)
                caps |= PHY_CAP_100_HALF;

            if (ethswctl->autoneg_ad & AN_100M_FULL)
                caps |= PHY_CAP_100_FULL;

            if (ethswctl->autoneg_ad & AN_1000M_HALF)
                caps |= PHY_CAP_1000_HALF;

            if (ethswctl->autoneg_ad & AN_1000M_FULL)
                caps |= PHY_CAP_1000_FULL;

            if (ethswctl->autoneg_local & AN_10000)
                caps |= PHY_CAP_10000;

            return phy_dev_caps_set(port->p.phy, caps) ? -EFAULT : 0;
        }
    
    case ETHSWAUTOMDIX:
        {
            int enable;
            int ret;

            if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
                return -EFAULT;

            if (!port->p.phy)
            {
                enet_err("port - %s has no PHY connected \n", port->name);
                return -EFAULT;
            }
       
            if (ethswctl->type == TYPE_GET)
            {
                ret = phy_dev_auto_mdix_get(port->p.phy, &enable);

                if (ret == -1)
                {
                    ethswctl->ret_val = -1;
                }
                else
                {
                    ethswctl->ret_val = 0;
                    ethswctl->val = enable;
                }
                  
                if (copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                {
                    return -EFAULT;
                }
            }
            else if (ethswctl->type == TYPE_SET)
            {
                ret = phy_dev_auto_mdix_set(port->p.phy, ethswctl->val);
                if (ret == -1)
                {
                    return -EFAULT;
                }
            }

            return 0;
        }
    
    case ETHSWPHYMODE:
        {
            uint32_t supported_caps = 0;

            if (ethswctl->addressing_flag & ETHSW_ADDRESSING_DEV)
                port = ((enetx_netdev *)netdev_priv(dev))->port;
            else
                port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port);

            if (!port)
                return -EFAULT;

            if (!port->p.phy)
            {
                enet_err("port - %s has no PHY connected \n", port->name);
                return -EFAULT;
            }

            if (phy_is_crossbar(port->p.phy))
                if (ethswctl->addressing_flag & ETHSW_ADDRESSING_SUBPORT)
                    phy_dev = crossbar_group_phy_get(port->p.phy, (ethswctl->sub_port>=BP_CROSSBAR_PORT_BASE)?BP_PHY_PORT_TO_CROSSBAR_PORT(ethswctl->sub_port):ethswctl->sub_port);
                else
                    phy_dev = crossbar_phy_dev_active(port->p.phy);
            else
                phy_dev = port->p.phy;

            phy_dev = cascade_phy_get_last_active(phy_dev);

            if (!phy_dev)
            {
                if (port->port_class == PORT_CLASS_PORT)
                    enet_err("port - %s has no PHY connected \n", port->name);
                return -EFAULT;
            }

            /* Get PHY supported speed capability */
            if (cascade_phy_dev_caps_get(phy_dev, CAPS_TYPE_SUPPORTED, &supported_caps))
                return -EFAULT;

            if (ethswctl->type == TYPE_SET)
            {
                phy_speed_t speed;
                phy_duplex_t duplex = ethswctl->duplex ? PHY_DUPLEX_FULL : PHY_DUPLEX_HALF;
                uint32_t cap = 0, adCaps = 0;

                if (ethswctl->speed == 10)
                    speed = PHY_SPEED_10;
                else if (ethswctl->speed == 100)
                    speed = PHY_SPEED_100;
                else if (ethswctl->speed == 1000)
                    speed = PHY_SPEED_1000;
                else if (ethswctl->speed == 2500)
                    speed = PHY_SPEED_2500;
                else if (ethswctl->speed == 5000)
                    speed = PHY_SPEED_5000;
                else if (ethswctl->speed == 10000)
                    speed = PHY_SPEED_10000;
                else if (ethswctl->speed == 0)
                    speed = PHY_SPEED_AUTO;
                else
                    speed = PHY_SPEED_UNKNOWN;

                cap = phy_speed_to_caps(speed, duplex);
                if ((cap & supported_caps) == 0)
                {
                    printk("Not Supported Speed %dmbps attempted\n", ethswctl->speed);
                    return -EFAULT;
                }

                if (ethswctl->addressing_flag == 0)
                    return cascade_phy_dev_speed_set(phy_dev, speed, duplex) ? -EFAULT : 0;

                adCaps = supported_caps;

                if (speed != PHY_SPEED_AUTO)
                {
                    adCaps &= ~(PHY_CAP_AUTONEG |
                        PHY_CAP_10_HALF | PHY_CAP_10_FULL |
                        PHY_CAP_100_HALF | PHY_CAP_100_FULL |
                        PHY_CAP_1000_HALF | PHY_CAP_1000_FULL |
                        PHY_CAP_2500 | PHY_CAP_5000 | PHY_CAP_10000);
                    adCaps |= cap;
                }

                return phy_dev_caps_set(phy_dev, adCaps) ? -EFAULT : 0;

            }
            else if (ethswctl->type == TYPE_GET)
            {
                phy_speed_t speed;
                phy_duplex_t duplex;

                if (phy_dev->phy_drv->config_speed_get)
                {
                    if (phy_dev_config_speed_get(phy_dev, &speed, &duplex))
                        return -EFAULT;
                }
                else if (phy_dev->phy_drv->caps_get)
                {
                    uint32_t caps = 0;
                    if (phy_dev_caps_get(port->p.phy, CAPS_TYPE_ADVERTISE, &caps))
                        return -EFAULT;
                    speed = (caps&PHY_CAP_AUTONEG)? PHY_SPEED_AUTO :
                        (caps&PHY_CAP_10000)? PHY_SPEED_10000 :
                        (caps&PHY_CAP_5000)? PHY_SPEED_5000 :
                        (caps&PHY_CAP_2500)? PHY_SPEED_2500 :
                        (caps&(PHY_CAP_1000_FULL|PHY_CAP_1000_HALF))? PHY_SPEED_1000 :
                        (caps&(PHY_CAP_100_FULL|PHY_CAP_100_HALF))? PHY_SPEED_100 :
                        (caps&(PHY_CAP_10_FULL|PHY_CAP_10_HALF))? PHY_SPEED_10 : PHY_SPEED_UNKNOWN;
                    duplex = (caps&PHY_CAP_AUTONEG)? PHY_DUPLEX_FULL :
                        (caps&PHY_CAP_10000)? PHY_DUPLEX_FULL :
                        (caps&PHY_CAP_5000)? PHY_DUPLEX_FULL :
                        (caps&PHY_CAP_2500)? PHY_DUPLEX_FULL :
                        (caps&PHY_CAP_1000_FULL)? PHY_DUPLEX_FULL :
                        (caps&PHY_CAP_1000_HALF)? PHY_DUPLEX_HALF :
                        (caps&PHY_CAP_100_FULL)? PHY_DUPLEX_FULL :
                        (caps&PHY_CAP_100_HALF)? PHY_DUPLEX_HALF :
                        (caps&PHY_CAP_10_FULL)? PHY_DUPLEX_FULL :
                        (caps&PHY_CAP_10_HALF)? PHY_DUPLEX_HALF : PHY_DUPLEX_UNKNOWN;
                }
                else
                    return -EFAULT;

                switch(speed) {
                    case PHY_SPEED_AUTO:
                        ethswctl->cfgSpeed = 0;
                        break;
                    case PHY_SPEED_10000:
                        ethswctl->cfgSpeed = 10000;
                        break;
                    case PHY_SPEED_5000:
                        ethswctl->cfgSpeed = 5000;
                        break;
                    case PHY_SPEED_2500:
                        ethswctl->cfgSpeed = 2500;
                        break;
                    case PHY_SPEED_1000:
                        ethswctl->cfgSpeed = 1000;
                        break;
                    case PHY_SPEED_100:
                        ethswctl->cfgSpeed = 100;
                        break;
                    case PHY_SPEED_10:
                        ethswctl->cfgSpeed = 10;
                        break;
                    default:
                        ethswctl->cfgSpeed = 0;
                }

                if (duplex == PHY_DUPLEX_FULL)
                    ethswctl->cfgDuplex = 1;
                else
                    ethswctl->cfgDuplex = 0;


                ethswctl->phyCap = 0;
                ethswctl->phyCap |= (supported_caps & PHY_CAP_AUTONEG) ? PHY_CFG_AUTO_NEGO : 0;
                ethswctl->phyCap |= (supported_caps & PHY_CAP_10_HALF) ? PHY_CFG_10HD : 0;
                ethswctl->phyCap |= (supported_caps & PHY_CAP_10_FULL) ? PHY_CFG_10FD : 0;
                ethswctl->phyCap |= (supported_caps & PHY_CAP_100_HALF) ? PHY_CFG_100HD : 0;
                ethswctl->phyCap |= (supported_caps & PHY_CAP_100_FULL) ? PHY_CFG_100FD : 0;
                ethswctl->phyCap |= (supported_caps & PHY_CAP_1000_HALF) ? PHY_CFG_1000HD : 0;
                ethswctl->phyCap |= (supported_caps & PHY_CAP_1000_FULL) ? PHY_CFG_1000FD : 0;
                ethswctl->phyCap |= (supported_caps & PHY_CAP_2500) ? PHY_CFG_2500FD : 0;
                ethswctl->phyCap |= (supported_caps & PHY_CAP_5000) ? PHY_CFG_5000FD : 0;
                ethswctl->phyCap |= (supported_caps & PHY_CAP_10000) ? PHY_CFG_10000FD : 0;

                if (phy_dev->link)
                {
                    ethswctl->duplex = (phy_dev->duplex == PHY_DUPLEX_FULL) ? 1 : 0;

                    if (phy_dev->speed == PHY_SPEED_10)
                        ethswctl->speed = 10;
                    else if (phy_dev->speed == PHY_SPEED_100)
                        ethswctl->speed = 100;
                    else if (phy_dev->speed == PHY_SPEED_1000)
                        ethswctl->speed = 1000;
                    else if (phy_dev->speed == PHY_SPEED_2500)
                        ethswctl->speed = 2500;
                    else if (phy_dev->speed == PHY_SPEED_5000)
                        ethswctl->speed = 5000;
                    else if (phy_dev->speed == PHY_SPEED_10000)
                        ethswctl->speed = 10000;
                    else
                        return -EFAULT;
                }
                else
                {
                    ethswctl->speed = 0;
                    ethswctl->duplex = 0;
                }

                if (copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                    return -EFAULT;

                return 0;
            }

            return -EFAULT;
        }

#if defined(CONFIG_BCM_ETH_PWRSAVE)
    case ETHSWPHYAPD:
        if (ethswctl->type == TYPE_GET)
            ethswctl->val = pm_apd_get();
        else
            pm_apd_set(ethswctl->val);

        ethswctl->ret_val = 1;
        if (copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
            return -EFAULT;

        return 0;
#endif

#if defined(CONFIG_BCM_ENERGY_EFFICIENT_ETHERNET)
    case ETHSWPHYEEE:
        if (ethswctl->type == TYPE_GET)
            ethswctl->val = pm_eee_get();
        else
            pm_eee_set(ethswctl->val);

        ethswctl->ret_val = 1;
        if (copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
            return -EFAULT;

        return 0;
#endif

#ifdef RUNNER
    case ETHSWOAMIDXMAPPING:
        if (ethswctl->oam_idx_str.map_sub_type == OAM_MAP_SUB_TYPE_FROM_UNIT_PORT)
        {
            if (_compat_validate_unit_port(ethswctl->unit, ethswctl->port))
                return -EFAULT;

#if defined(DSL_DEVICES)
            /* We don't need to check Unit#0 for OAM_IDX */
            if (ethswctl->unit == 0)
                return -EFAULT;
#endif

            ethswctl->oam_idx_str.oam_idx = COMPAT_OAM_IDX(ethswctl->unit, ethswctl->port);
            if (ethswctl->oam_idx_str.oam_idx == -1)
                return -EFAULT;

            if (copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                return -EFAULT;
            return 0;
        }
        else
        {
            int u, p;

#if defined(DSL_DEVICES)
            /* We only need to check Unit#1 for OAM_IDX */
            for (u = 1; u < BP_MAX_ENET_MACS; u++)
#else
            for (u = 0; u < BP_MAX_ENET_MACS; u++)
#endif
            {
                for (p = 0; p < COMPAT_MAX_SWITCH_PORTS; p++)
                {
                    if (COMPAT_OAM_IDX(u, p) == ethswctl->oam_idx_str.oam_idx)
                        break;
                }

                if (p != COMPAT_MAX_SWITCH_PORTS)
                    break;
            }

            if (p == COMPAT_MAX_SWITCH_PORTS)
                return -EFAULT;

            ethswctl->unit = u;
            ethswctl->port = p;

            if (ethswctl->oam_idx_str.map_sub_type == OAM_MAP_SUB_TYPE_TO_RDPA_IF)
            {
                rdpa_if index;

                if (_port_rdpa_if_by_port(unit_port_array[u][p], &index))
                    return -EFAULT;

                ethswctl->oam_idx_str.rdpa_if = index;
            }

            if (copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                return -EFAULT;

            return 0;

        }
#endif

        return -EFAULT;
    }

    enet_dbgv(" dev=%s, cmd=%x(SIOCETHSWCTLOPS), unknown ops=%d u/p=%d/%d\n", NETDEV_PRIV(dev)->port->obj_name, cmd, ethswctl->op, ethswctl->unit, ethswctl->port);
    return -(EOPNOTSUPP);
}

int enet_ioctl_compat(struct net_device *dev, struct ifreq *rq, int cmd)
{
    enetx_port_t *port = NULL;
    union {
        struct ethswctl_data ethswctl_data;
        struct interface_data interface_data;
        ifreq_ext_t ifre;
    } rq_data;

    struct ethswctl_data *ethswctl = (struct ethswctl_data *)&rq_data;
    ifreq_ext_t *ifx = (ifreq_ext_t *)&rq_data;

    switch (cmd)
    {
#if defined(DSL_DEVICES)
    case SIOCGSWITCHPORT:   // Get Switch Port from name
        {
            int u, p;
            struct net_device *dev;
            struct interface_data *enetif_data = (struct interface_data*)&rq_data;

            if (copy_from_user(enetif_data, rq->ifr_data, sizeof(*enetif_data)))
                return -EFAULT;
            if (get_root_dev_by_name(enetif_data->ifname, &dev))
                return -EFAULT;

            for (u = 0; u < BP_MAX_ENET_MACS; u++)
            {
                for (p = 0; p < COMPAT_MAX_SWITCH_PORTS; p++)
                {
                    if (!COMPAT_PORT(u, p) || COMPAT_PORT(u, p)->dev != dev)
                        continue;
                    enetif_data->switch_port_id = PHYSICAL_PORT_TO_LOGICAL_PORT(p,u);
                    if (copy_to_user(rq->ifr_data, enetif_data, sizeof(*enetif_data)))
                        return -EFAULT;
                    return 0;
                }
            }

           return -EFAULT;
        }
#endif //DSL_DEVICES
    case SIOCGWANPORT:
        {
            list_ctx ctx;

            if (copy_from_user(ethswctl, rq->ifr_data, sizeof(struct ethswctl_data)))
                return -EFAULT;

            if (ethswctl->up_len.len <= 0)
            return -EFAULT;

            if (!(ctx.str = kzalloc(ethswctl->up_len.len, GFP_KERNEL)))
                return -EFAULT;

            ctx.maxlen = ethswctl->up_len.len;
            ctx.str[0] = '\0';
            if (!port_traverse_ports(root_sw, tr_wan_devname_concat, PORT_CLASS_PORT, &ctx))
            {
                BCM_IOC_PTR_ZERO_EXT(ethswctl->up_len.uptr);
                if (copy_to_user(ethswctl->up_len.uptr, ctx.str, strlen(ctx.str)+1))
                    return -EFAULT;
            }

            kfree(ctx.str);
            return 0;
        }

    case SIOCSCLEARMIBCNTR:
        {
            if (get_port_by_if_name(rq->ifr_name, &port))
            {
                enet_err("is not an enet dev %s\n", rq->ifr_name);
                return -EFAULT;
            }
            port_traverse_ports(port, tr_stats_clear, PORT_CLASS_PORT | PORT_CLASS_SW, port);
            return 0;
        }
    case SIOCMIBINFO:
        {
            IOCTL_MIB_INFO mib = { };

            port = ((enetx_netdev *)netdev_priv(dev))->port;
            if (port->port_class != PORT_CLASS_PORT)
                return -EFAULT;

            if (port->p.phy && port->p.phy->link)
            {
                phy_dev_t *phy_dev = port->p.phy;

                mib.ulIfDuplex = (phy_dev->duplex == PHY_DUPLEX_FULL) ? 1 : 0;
                mib.ulIfLastChange = port->p.phy_last_change;

                if (phy_dev->speed == PHY_SPEED_10)
                    mib.ulIfSpeed = SPEED_10MBIT;
                else if (phy_dev->speed == PHY_SPEED_100)
                    mib.ulIfSpeed = SPEED_100MBIT;
                else if (phy_dev->speed == PHY_SPEED_1000)
                    mib.ulIfSpeed = SPEED_1000MBIT;
                else if (phy_dev->speed == PHY_SPEED_2500)
                    mib.ulIfSpeed = SPEED_2500MBIT;
                else if (phy_dev->speed == PHY_SPEED_10000)
                    mib.ulIfSpeed = SPEED_10000MBIT;
                else
                    return -EFAULT;
            }

            if (copy_to_user(rq->ifr_data, (void *)&mib, sizeof(mib)))
                return -EFAULT;

            return 0;
        }
    case SIOCGLINKSTATE:
        {
            int data; /* XXX 64 support ?? */

            data = netif_carrier_ok(dev);
            //enet_dbgv("SIOCGLINKSTATE: %s, link %s ...\n", dev->name, data? "up":"down");
            if (copy_to_user(rq->ifr_data, &data, sizeof(data)))
                return -EFAULT;

            return 0;
        }
    case SIOCSWANPORT:
        {
            int data = (unsigned long)rq->ifr_data;

            port = ((enetx_netdev *)netdev_priv(dev))->port;
            return port_netdev_role_set(port, data ? PORT_NETDEV_ROLE_WAN : PORT_NETDEV_ROLE_LAN) ? -EFAULT : 0;
        }

    case SIOCIFREQ_EXT:
        {
            list_ctx ctx;
            port_cap_t port_cap;

            if (copy_from_user(ifx, rq->ifr_data, sizeof(*ifx)))
                return -EFAULT;

            BCM_IOC_PTR_ZERO_EXT(ifx->stringBuf);
            switch (ifx->opcode)
            {
            case SIOCGPORTWANONLY:
                port_cap = PORT_CAP_WAN_ONLY;
                break;
            case SIOCGPORTWANPREFERRED:
                port_cap = PORT_CAP_WAN_PREFERRED;
                break;
            case SIOCGPORTLANONLY:
                port_cap = PORT_CAP_LAN_ONLY;
                break;
            default:
                return -EFAULT;
            }

            if (ifx->bufLen <= 0)
                return -EFAULT;

            if (!(ctx.str = kzalloc(ifx->bufLen, GFP_KERNEL)))
                return -EFAULT;

            ctx.maxlen = ifx->bufLen;
            ctx.op_code = port_cap;
            ctx.str[0] = '\0';
            if (!port_traverse_ports(root_sw, tr_ifreq_devname_concat, PORT_CLASS_PORT, &ctx))
            {
                if (copy_to_user(ifx->stringBuf, ctx.str, ifx->bufLen))
                    return -EFAULT;
            }

            kfree(ctx.str);

            return 0;
        }

    case SIOCGQUERYNUMPORTS:
        {
            uint32_t map = 0;

            port = NETDEV_PRIV(dev)->port;
            if (port->port_class != PORT_CLASS_PORT)
                return -EFAULT;

            if (crossbar_group_external_endpoint_count(port->p.phy, &map) < 0)
                return ENODATA;  // port is not connected to crossbar, intentionally return positive number to be backward compatible

            map <<= BP_CROSSBAR_PORT_BASE;

            if (copy_to_user(rq->ifr_data, (void *)&map, sizeof(map)))
                return -EFAULT;

            return 0;
        }

    case SIOCETHCTLOPS:
        return enet_ioctl_compat_ethctl(dev, rq, cmd);

    case SIOCETHSWCTLOPS:
        return enet_ioctl_compat_ethswctl(dev, rq, cmd);

#ifdef CONFIG_BCM_PTP_1588
    case SIOCSHWTSTAMP:
        return ptp_ioctl_hwtstamp_set(dev, rq);

    case SIOCGHWTSTAMP:
        return ptp_ioctl_hwtstamp_get(dev, rq);
#endif

    }

    enet_dbgv(" dev=%s, unknown cmd=%x ...\n", NETDEV_PRIV(dev)->port->obj_name, cmd);
    return -EOPNOTSUPP;
}

