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

#include <linux/version.h>
#include "enet.h"
#include "port.h"
#include "enet_dbg.h"
#include <bcm/bcmswapitypes.h>
#include <board.h>
#include <bcmnet.h>
#ifdef RUNNER
#include "runner.h"
#endif
#ifdef SYSPVSW_DEVICE
#include "syspvsw.h"
#endif
#ifdef SF2_DEVICE
#include "sf2.h"
#endif
#if defined(CONFIG_BCM_EPON_STACK) || defined(CONFIG_BCM_EPON_STACK_MODULE)
#include "mac_drv_epon_ae.h"
#endif
#ifdef CONFIG_BCM_PTP_1588
#include "ptp_1588.h"
#endif
#if defined(DSL_DEVICES)
#include "phy_drv_dsl_phy.h"
#include "phy_drv_dsl_serdes.h"
void extlh_link_change_handler(enetx_port_t *port, int linkstatus, int speed, int duplex);
#endif
#include "opticaldet.h"
#include "bcmenet_common.h"
#include "crossbar_dev.h"
#include "phy_drv.h"
#include "phy_drv_brcm.h"
#include "phy_macsec_common.h"
#include "clk_rst.h"
#if defined(CONFIG_GT7)
#include "bcmenet_ioctl_phyext84991.h"
#endif

extern int apd_enabled;
extern int eee_enabled;
extern int phy_speed_max;

/**
 * @brief splits asymmetric input speed into up&down stream speeds
 * @param[i] asym_speed 
 * @param[o] up_speed 
 * @param[o] down_speed 
 * @return int 0 on success, -1 if asym_speed is not supported
 */
static int parse_asym_speed(const int asym_speed, int *up_speed, int *down_speed)
{
    int rc = 0;

    switch (asym_speed)
    {
    case NET_PORT_SPEED_0101:
        *up_speed = *down_speed = PHY_CAP_1000_FULL;
        break;
    case NET_PORT_SPEED_0201:
        *down_speed = PHY_CAP_2500;
        *up_speed   = PHY_CAP_1000_FULL;
        break;
    case NET_PORT_SPEED_0202:
        *up_speed = *down_speed = PHY_CAP_2500;
        break;
    case NET_PORT_SPEED_2501:
        *down_speed = PHY_CAP_2500;
        *up_speed   = PHY_CAP_1000_FULL;
        break;
    case NET_PORT_SPEED_1001:
        *down_speed = PHY_CAP_10000;
        *up_speed   = PHY_CAP_1000_FULL;
        break;
    case NET_PORT_SPEED_1025:
        *down_speed = PHY_CAP_10000;
        *up_speed   = PHY_CAP_2500;
        break;
    case NET_PORT_SPEED_1010:
        *up_speed = *down_speed = PHY_CAP_10000;
        break;
    default:
        *up_speed = *down_speed = NET_PORT_SPEED_NONE;
        rc = -1;
        break;
    }

    return rc;
}

#if defined(CONFIG_BCM_ETH_PWRSAVE)
static int pm_apd_get(void)
{
    return apd_enabled;
}

static int pm_apd_set_single(enetx_port_t *p, void *ctx)
{
    phy_dev_t *phy_dev = p->p.phy;

    if (phy_dev)
        cascade_phy_dev_apd_set(phy_dev, apd_enabled);

    return 0;
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

static int pm_eee_set_single(enetx_port_t *p, void *ctx)
{
    phy_dev_t *phy_dev = p->p.phy;

    if (phy_dev)
        cascade_phy_dev_eee_set(phy_dev, eee_enabled);

    return 0;
}

static void pm_eee_set(int enabled)
{
    if (eee_enabled != enabled) {
        eee_enabled = enabled;
        port_traverse_ports(root_sw, pm_eee_set_single, PORT_CLASS_PORT, NULL);
    }
}
#endif

static int pm_lan_pwr_set_single(enetx_port_t *p, void *ctx)
{
    int enabled = (int)(uintptr_t)ctx;

    if (PORT_ROLE_IS_WAN(p) || !p->dev)
        return 0;

    if (enabled)
        dev_change_flags(p->dev, p->dev->flags | IFF_UP);
    else
        dev_change_flags(p->dev, p->dev->flags & ~IFF_UP);

    return 0;
}

static void pm_lan_pwr_set(int enabled)
{
    port_traverse_ports(root_sw, pm_lan_pwr_set_single, PORT_CLASS_PORT, (void *)(uintptr_t)enabled);
}

static int pm_lan_pwr_get_single(enetx_port_t *p, void *ctx)
{
    if (PORT_ROLE_IS_WAN(p) || !p->dev)
        return 0;

    if (p->dev->flags & IFF_UP)
        return 1;
    
    return 0;
}

static int pm_lan_pwr_get(void)
{
    return port_traverse_ports(root_sw, pm_lan_pwr_get_single, PORT_CLASS_PORT, NULL);
}

#if defined(CONFIG_BCM_PON)
#define CONFIG_BCM_PHY_SPEED_LIMIT
#endif

#if defined(CONFIG_BCM_PHY_SPEED_LIMIT)
static int pm_physpeed_get(void)
{
    return phy_speed_max;
}

static int pm_physpeed_set_single(enetx_port_t *p, void *ctx)
{
    phy_dev_t *phy_dev = p->p.phy;

    if (phy_dev)
        phy_dev_speed_set(phy_dev, phy_speed_max, PHY_DUPLEX_FULL);

    return 0;
}

static void pm_physpeed_set(unsigned int speed)
{
    phy_speed_max = speed;
    port_traverse_ports(root_sw, pm_physpeed_set_single, PORT_CLASS_PORT, NULL);
}
#endif

enetx_port_t *unit_port_array[COMPAT_MAX_SWITCHES][COMPAT_MAX_SWITCH_PORTS] = {};
#define COMPAT_PORT(u, p) ((unit_port_array[u][p] && (unit_port_array[u][p]->port_class & (PORT_CLASS_PORT))) ? \
    unit_port_array[u][p] : NULL)

#ifdef RUNNER
#define COMPAT_RPDA(u, p) _port_rdpa_object_by_port(_compat_port_object_from_unit_port(u, p))
#endif

#ifdef ENET_GPON
int gpon_mcast_gem_set(int mcast_idx);
#else
static int gpon_mcast_gem_set(int mcast_idx)
{
    return -1;
}
#endif

static int tr_port_by_net_port(enetx_port_t *port, void *_ctx)
{
    int net_port = *(uint32_t *)_ctx;

    switch (net_port)
    {
        case NET_PORT_AE:
            if (port->port_info.is_wanconf_ae || port->port_info.is_wanconf_mux_ae)
                goto match;
            break;
        case NET_PORT_GPON:
        case NET_PORT_EPON:
            if (port->port_info.is_wanconf_mux_pon)
                goto match;
            break;
    }

    return 0;
match:
    *(enetx_port_t **)_ctx = port;
    return 1;
}

static int get_port_by_net_port(enetx_port_t *sw, port_class_t port_class, unsigned long net_port, enetx_port_t **match)
{
    void *in_out = (void *)(unsigned long)net_port;

    if (port_traverse_ports(sw, tr_port_by_net_port, port_class, &in_out) <= 0)
        return -1;

    *match = (enetx_port_t *)in_out;

    return 0;
}

static int _handle_add_port_gpon(struct net_port_t *net_port, enetx_port_t *p)
{
    mac_cfg_t mac_cfg = {};

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

    return 0;
}

static int _handle_pon_serdes_mux(struct net_port_t *net_port, enetx_port_t *p)
{
    mac_type_t mac_type;
    int mac_id = 0;
    void *priv = NULL;
    char *netdev_name = NULL;

    p->has_interface = 1;

    switch (net_port->port)
    {
#if defined(CONFIG_BCM_EPON_STACK) || defined(CONFIG_BCM_EPON_STACK_MODULE)
        case NET_PORT_AE:
        {
            static mac_priv_epon_ae_t epon_ae_priv;

            mac_type = MAC_TYPE_EPON_AE;
            p->port_info.is_epon_ae = 1;
            epon_ae_priv.phy_dev = (void *)p->p.phy;
            epon_ae_priv.rdpa_port_obj = &p->priv; /* rdpa_port_object */
            priv = (void *)&epon_ae_priv;

            unit_port_array[0][p->port_info.port] = p;
            break;
        }
        case NET_PORT_EPON:
            mac_type = MAC_TYPE_xEPON;
            p->port_info.is_epon = 1;
            netdev_name = "epon0";
            priv = &p->priv; /* rdpa_port_object */
            break;
#endif
        case NET_PORT_GPON:
            mac_type = MAC_TYPE_xGPON;
            mac_id = net_port->sub_type;
            p->port_info.is_gpon = 1;
            netdev_name = "gpondef";
            priv = &p->priv; /* rdpa_port_object */
            break;
        default:
            return 0;
    }

    if (!(p->p.mac = mac_dev_add(mac_type, mac_id, priv)))
    {
        printk("Error adding MAC device mac_type %d\n", mac_type);
        return -1;
    }

    if (netdev_name)
        strcpy(p->name, netdev_name);

    return 0;
}

static int should_handle_serdes_mux(enetx_port_t *p)
{
    return p->port_info.is_wanconf_mux_pon || p->port_info.is_wanconf_mux_ae;
}

static int _handle_add_port(struct net_port_t *net_port)
{
    int port = net_port->port;
    int rc = 0;
    char *ifname = net_port->ifname;
    enetx_port_t *p = NULL;

    if (get_port_by_net_port(root_sw, PORT_CLASS_PORT_DETECT, port, &p) < 0)
    {
        printk("Error: No port found in \"detect\" state for net_port->port %d\n", net_port->port);
        return -1;
    }

    if (port == NET_PORT_AE)
    {
#if defined(CONFIG_BCM96813)
        /* On 6813, All Ethernet ports are configured as lan in runner */
        net_port->is_wan = 0;
        
        /* add to unit_port_array */
        unit_port_array[0][p->port_info.port] = p;
#endif
    }

    if (should_handle_serdes_mux(p))
        rc = _handle_pon_serdes_mux(net_port, p);
    if (rc)
        return -1;

    p->port_info.is_wan = net_port->is_wan;
    if (*ifname)
    {
        strncpy(p->name, ifname, sizeof(p->name)-1);
        p->name[sizeof(p->name)-1]='\0';
    }

    if (port == NET_PORT_GPON)
        _handle_add_port_gpon(net_port, p);

    if (port_activate(p) || !p->p.phy)
        return -1;

    if (port == NET_PORT_AE)
    {
        phy_speed_t speed = PHY_SPEED_UNKNOWN;

        if (net_port->speed == NET_PORT_SPEED_1010)
            speed = PHY_SPEED_10000;
        else if (net_port->speed == NET_PORT_SPEED_0202)
            speed = PHY_SPEED_2500;
        else if (net_port->speed == NET_PORT_SPEED_0101)
            speed = PHY_SPEED_1000;

        if (speed != PHY_SPEED_UNKNOWN)
            phy_dev_speed_set(p->p.phy, speed, PHY_DUPLEX_FULL);
    }
    else /*E&GPON*/
    {
        int us, ds;

        if (!parse_asym_speed(net_port->speed, &us, &ds))
            phy_dev_caps_set(p->p.phy, us | ds);
    }

    /* Copy interface name back to userspace */
    strncpy(net_port->ifname, p->dev->name, sizeof(net_port->ifname)-1);
    net_port->ifname[sizeof(net_port->ifname)-1]='\0';

    return 0;
}

static void port_info_revert(enetx_port_t *p)
{
    p->port_info.is_epon_ae = 0;
    p->port_info.is_epon = 0;
    p->port_info.is_gpon = 0;
    /* On purpose not reverting p->port_info.is_wan */
}

static int _handle_del_port(int port)
{
    enetx_port_t *p = NULL;

    if (get_port_by_net_port(root_sw, PORT_CLASS_PORT, port, &p) < 0)
    {
        printk("Error: _handle_del_port: No port found\n");
        return -1;
    }

    port_deactivate(p);

    if (should_handle_serdes_mux(p))
    {
        mac_dev_del(p->p.mac);
        p->p.mac = NULL;
        port_info_revert(p);
    }

    return 0;
}

static int _handle_serdes_name(struct net_port_t *net_port)
{
    int port = net_port->port;
    enetx_port_t *p = NULL;

    if (get_port_by_net_port(root_sw, PORT_CLASS_PORT, port, &p) < 0)
    {
        printk("Error: _handle_serdes_name: No port found\n");
        return -1;
    }

    /* Copy interface name back to userspace */
    strncpy(net_port->ifname, p->dev->name, sizeof(net_port->ifname));

    return 0;
}

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
    int count;
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
    {
        enet_err("Insufficient maxlen: buf_len %d, dev_len %d, maxlen %d\n",
          buf_len, dev_len, maxlen);
        return -1;
    }

    if (buf[0])
        strcat(buf, ",");

    strcat(buf, port->dev->name);

    return 0;
}

static int tr_wan_devname_concat(enetx_port_t *port, void *_ctx)
{
    list_ctx *ctx = (list_ctx *)_ctx;

    if (port->n.port_netdev_role == PORT_NETDEV_ROLE_WAN)
    {
        if (_devname_concat(port, ctx->str, ctx->maxlen) < 0)
            return -1;
        ctx->count++;
    }
    return 0;
}

static int tr_ifreq_landevname_concat(enetx_port_t *port, void *_ctx)
{
    list_ctx *ctx = (list_ctx *)_ctx;

    if ((port->dev) && (port->p.port_cap == PORT_CAP_LAN_WAN || port->p.port_cap == PORT_CAP_LAN_ONLY) && 
        (port->n.port_netdev_role != PORT_NETDEV_ROLE_WAN))
    {
        if ( _devname_concat(port, ctx->str, ctx->maxlen) < 0)
            return -1;
        ctx->count++;
    }
    return 0;
}

static int tr_ifreq_devname_concat(enetx_port_t *port, void *_ctx)
{
    list_ctx *ctx = (list_ctx *)_ctx;

    if (port->p.port_cap == ctx->op_code)
    {
        if ( _devname_concat(port, ctx->str, ctx->maxlen) < 0)
            return -1;
        ctx->count++;
    }
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

static int get_root_dev_by_dev(struct net_device *dev, struct net_device **_dev)
{
    if (!_dev)
        return -1;

    if (!dev)
        return -1;

    dev_hold(dev);
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

int get_port_by_leaf_dev(struct net_device *leaf_dev, enetx_port_t **port)
{
    struct net_device *dev;

    if (get_root_dev_by_dev(leaf_dev, &dev))
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

#if defined(DSL_DEVICES)
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

#endif // DSL_DEVICES

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
#define ESTATS_VAL_1(v1) #v1, estats->v1
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
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(tx_dropped_blog_drop, tx_dropped_no_fkb));
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT "\n", ESTATS_VAL(tx_dropped_accelerator_lan_fail, tx_dropped_accelerator_wan_fail));
#ifdef CONFIG_BCM_XDP
        buf += snprintf(buf, buf_end-buf, ESTATS_FMT1 "\n", ESTATS_VAL_1(rx_dropped_xdp));
#endif
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
#ifdef CONFIG_BCM_XDP
        estats->rx_dropped_xdp +
#endif
        estats->rx_dropped_no_srcport;

    portCnt.txDrops = estats->tx_dropped_bad_nbuff +
        estats->tx_dropped_blog_drop +
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
    if (unit < 0 || port < 0 || unit >= COMPAT_MAX_SWITCHES || port >= COMPAT_MAX_SWITCH_PORTS)
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

static phy_dev_t *enet_dev_get_phy_dev(struct net_device *dev, int sub_port)
{
    enetx_port_t *port = ((enetx_netdev *)netdev_priv(dev))->port;
    phy_dev_t *phy_dev;

    if (!phy_is_crossbar(port->p.phy))
        phy_dev = port->p.phy;
    else if (sub_port != -1)
        phy_dev = crossbar_group_phy_get(port->p.phy, PHY_PORT_TO_CROSSBAR_PORT(sub_port));
    else    // get crossbar active phy
        phy_dev = crossbar_phy_dev_active(port->p.phy);

    return phy_dev;
}

#if defined(DSL_DEVICES)

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
#endif // DSL_DEVICES

#define ethctl_field_copy_to_user(f)      \
    copy_to_user((void*)((char*)rq->ifr_data +((char*)&(ethctl->f) - (char*)&rq_data)), &(ethctl->f), sizeof(ethctl->f))

#define IsMacToMac(id)   (((id) & (MAC_CONN_VALID|MAC_CONNECTION)) == (MAC_CONN_VALID|MAC_MAC_IF))

static int cat_snprintf(char **buf, int *sz, char *fmt, ...)
{
    va_list valist;
    int new_sz;
    va_start(valist, fmt);

    new_sz = *sz + vsnprintf(0, 0, fmt, valist) + 1;
    *buf = krealloc(*sz? *buf: 0, new_sz, GFP_KERNEL);
    BUG_ON(!*buf);
    if (*sz == 0)
        **buf = 0;
    *sz = new_sz;
    vsnprintf(*buf + strlen(*buf), *sz, fmt, valist);
    va_end(valist);
    return *sz;
}

static char *print_net_dev_info(enetx_port_t *port)
{
    static char buf[256];
    struct net_device *dev = port->dev;
    mac_dev_t *mac_dev = port->p.mac;
    mac_cfg_t mac_cfg= {0};
    int netdev_link;
    mac_status_t mac_status;
    mac_status.link = -1;

    if (mac_dev)
    {
        mac_dev_cfg_get(mac_dev, &mac_cfg);
        mac_dev_read_status(mac_dev, &mac_status);
        mac_dev_stats_get(mac_dev, &port->p.mac->stats);
    }

    if (dev)
        netdev_link = netif_carrier_ok(dev);

    snprintf(buf, sizeof(buf), "%-10s: %2s Rx:%4d Tx:%4d %-4s:%-4s",
            dev? dev->name: "<IntnlDev>", 
            dev? netdev_link? "Up": "Dn": "Up",
            mac_dev? ((uint32_t)port->p.mac->stats.rx_packet)%10000:-1,
            mac_dev? ((uint32_t)port->p.mac->stats.tx_packet)%10000:-1,
            mac_dev? mac_dev_speed_to_short_str(mac_cfg.speed):"", 
            mac_dev? mac_status.link==-1? "": mac_status.link? mac_dev_speed_to_short_str(mac_status.speed): "Dn":"NA"
        );

    return buf;
}

static char *print_phy_link_status(phy_dev_t *phy_dev)
{
#if defined(DSL_DEVICES)
    phy_serdes_t *phy_serdes = phy_dev->priv;

    if (phy_is_pon_wan_ae_serdes(phy_dev->cascade_prev))
        return  phy_dev->link?"Up":"Down";

    if (phy_dev->link)
        return phy_dev_speed_to_short_str(phy_dev->speed);

    if (!PHY_IS_SERDES(phy_dev))
        return "Down";

    switch (phy_serdes->sfp_module_type)
    {
        case SFP_FIXED_PHY:
            return "Down";
        case SFP_NO_MODULE:
            return "NoSFP";
        default:
            return "SFPIn";
    }
#else
        return  phy_dev->link?"Up":"Down";
#endif
}

/* Single Type Display */
#if defined(INTER_PHY_TYPE_100BASE_FX_M) /* FIXME DT Temporary for shared/ and bcmdriver/ transition */
static char *inter_phy_type_string(phy_dev_t *phy_dev, int inter_phy_type)
{
    int configured_inter_type;
    static char buf[64];
    static char *serdes_inter_phy_string[] = { SERDES_INTER_TYPE_STRING };

    if (inter_phy_type >= ARRAY_SIZE(serdes_inter_phy_string))
        inter_phy_type = INTER_PHY_TYPE_UNKNOWN;

    configured_inter_type = phy_dev_configured_current_inter_phy_type_get(phy_dev);
    strncpy(buf, serdes_inter_phy_string[inter_phy_type],sizeof(buf)-1);
    buf[sizeof(buf)-1]='\0';
    if (phy_dev->an_enabled)
        strcat(buf, "_A");
    return buf;
}

/* Multi TYpes Display */
static char *inter_phy_types_string(int inter_phy_types)
{
    static char buf[128];
    int sz = sizeof(buf), n = 0;
    int f100, f1g, f2g, f5g, f10g;

    f100 = f1g = f2g =  f5g = f10g = 0; /* Speed flag for if we need display speed digit */
    /* Note: the order below matters for speed from low to hight */

    buf[0] = 0;
    if (inter_phy_types == INTER_PHY_TYPE_UNKNOWN_M)
        return "";

    /* Note: The order below matters, for matching the coding flag convention */
    if (inter_phy_types & INTER_PHY_TYPE_MLTI_SPEED_BASE_X_AN_M)
        n += snprintf(buf+n, sz-n, "A");
    if (inter_phy_types & INTER_PHY_TYPE_SXGMII_M)
        n += snprintf(buf+n, sz-n, "Q");
    if (inter_phy_types & INTER_PHY_TYPE_USXGMII_MP_M)
        n += snprintf(buf+n, sz-n, "M");
    if (inter_phy_types & INTER_PHY_TYPE_USXGMII_M)
        n += snprintf(buf+n, sz-n, "U");
    if (inter_phy_types & INTER_PHY_TYPE_SGMII_M)
        n += snprintf(buf+n, sz-n, "S");

    if (inter_phy_types & INTER_PHY_TYPE_100BASE_FX_M)
        n += snprintf(buf+n, sz-n, "0F");

    if (inter_phy_types & INTER_PHY_TYPE_1000BASE_X_M)
        n += snprintf(buf+n, sz-n, "%sK", f1g++? "":"1");
    if (inter_phy_types & INTER_PHY_TYPE_1GBASE_R_M)
        n += snprintf(buf+n, sz-n, "%sR", f1g++? "":"1");
    if (inter_phy_types & INTER_PHY_TYPE_1GBASE_X_M)
        n += snprintf(buf+n, sz-n, "%sX", f1g++? "":"1");

    if (inter_phy_types & INTER_PHY_TYPE_2500BASE_X_M)
        n += snprintf(buf+n, sz-n, "%sK", f2g++? "":"2");
    if (inter_phy_types & INTER_PHY_TYPE_2P5GIDLE_M)
        n += snprintf(buf+n, sz-n, "%sI", f2g++? "":"2");
    if (inter_phy_types & INTER_PHY_TYPE_2P5GBASE_R_M)
        n += snprintf(buf+n, sz-n, "%sR", f2g++? "":"2");
    if (inter_phy_types & INTER_PHY_TYPE_2P5GBASE_X_M)
        n += snprintf(buf+n, sz-n, "%sX", f2g++? "":"2");

    if (inter_phy_types & INTER_PHY_TYPE_5000BASE_X_M)
        n += snprintf(buf+n, sz-n, "%sK", f5g++? "":"5");
    if (inter_phy_types & INTER_PHY_TYPE_5GIDLE_M)
        n += snprintf(buf+n, sz-n, "%sI", f5g++? "":"5");
    if (inter_phy_types & INTER_PHY_TYPE_5GBASE_R_M)
        n += snprintf(buf+n, sz-n, "%sR", f5g++? "":"5");
    if (inter_phy_types & INTER_PHY_TYPE_5GBASE_X_M)
        n += snprintf(buf+n, sz-n, "%sX", f5g++? "":"5");

    if (inter_phy_types & INTER_PHY_TYPE_10GBASE_R_M)
        n += snprintf(buf+n, sz-n, "%sR", f10g++? "":"10");

    return buf;
}
#endif

static char *get_common_inter_types(phy_dev_t *phy_dev)
{
#if defined(INTER_PHY_TYPE_100BASE_FX_M) /* FIXME DT Temporary for shared/ and bcmdriver/ transition */
    uint32_t common_types = 0;
    static char buf[256];
    int sz = sizeof(buf), n = 0;

    common_types = cascade_phy_get_common_inter_types(phy_dev);
    n += snprintf(buf+n, sz-n, "%10s", inter_phy_types_string(common_types)); 
    return buf;
#else
    return "";
#endif
}

char *get_inter_phy_types(phy_dev_t *phy_dev)
{
    uint32_t inter_types;
    uint32_t config_types;
    uint32_t current_type;
    phy_speed_t config_speed;
    phy_duplex_t config_duplex;
    static char buf[256];
    int sz = sizeof(buf), n = 0;
        
    if cascade_phy_get_next(phy_dev)
    {
        phy_dev_inter_phy_types_get(phy_dev, INTER_PHY_TYPE_DOWN, &inter_types);
        current_type = phy_dev_current_inter_phy_type_get(phy_dev);
        phy_dev_configured_inter_phy_types_get(phy_dev, &config_types);
    }
    else
    {
        phy_dev_inter_phy_types_get(phy_dev, INTER_PHY_TYPE_UP, &inter_types);
        current_type = phy_dev_current_inter_phy_type_get(phy_dev);
        phy_dev_configured_inter_phy_types_get(phy_dev, &config_types);
    }

    if (inter_types == INTER_PHY_TYPE_UNKNOWN_M)
        return "Unknown";

    phy_dev_config_speed_get(phy_dev, &config_speed, &config_duplex);
        n += snprintf(buf+n, sz-n, "%20s:", inter_phy_types_string(inter_types));
        n += snprintf(buf+n, sz-n, "%16s", inter_phy_type_string(phy_dev, current_type));

    return buf;
}

static char *print_phy_link(phy_dev_t *phy_dev)
{
    static char buf[512];
    uint32_t caps = 0;
    phy_speed_t max_spd;
    int an, sz = sizeof(buf), n = 0;
    int configured_inter_type;
    int configured_an_enable;
    uint32_t inter_types;

    if (phy_dev->phy_drv && !phy_dev->phy_drv->caps_get)
    {
        an = 0;
        max_spd = phy_dev->speed;
    }
    else
    {
        phy_dev_caps_get(phy_dev, CAPS_TYPE_ADVERTISE, &caps);
        an = (caps & PHY_CAP_AUTONEG) > 0;
        max_spd = phy_caps_to_max_speed(caps);
    }

    phy_dev_inter_phy_types_get(phy_dev, INTER_PHY_TYPE_DOWN, &inter_types);
    configured_an_enable = phy_dev_configured_an_enabled_get(phy_dev);
    configured_inter_type = phy_dev_configured_current_inter_phy_type_get(phy_dev);

    n += snprintf(buf+n, sz-n, "Spd:%4s:%-4s ", an?"Auto":"Fix", phy_dev_speed_to_short_str(max_spd));

    if (inter_types != INTER_PHY_TYPE_UNKNOWN_M)
    {
        n += snprintf(buf+n, sz-n, "XFI:%-2s:%4s ", 
                configured_inter_type==INTER_PHY_TYPE_AUTO?"Au":"Fx",
                configured_an_enable==PHY_CFG_AN_AUTO? "AnAu": 
                    configured_an_enable==PHY_CFG_AN_ON? "AnOn": "AnOf");
        n += snprintf(buf+n, sz-n, "  %s", get_inter_phy_types(phy_dev));
    }

    n += snprintf(buf+n, sz-n, "%5s", (phy_dev->mac_link & PHY_MAC_LINK_VALID)?
        (phy_dev->mac_link & 1)? "McUp": "McDn": "");

    n += snprintf(buf+n, sz-n, "%8s", print_phy_link_status(phy_dev)); 

    return buf;
}

static char *phy_show_polarity_swap(phy_dev_t *phy_dev)
{
    static char buf[64];
    int n = 0;
    char *tr_inverse_string[2][2] = {{"  ", "Tx"}, {"Rx", "TR"}};

    n += sprintf(buf+n, "%s%3s ", 
        tr_inverse_string[phy_dev->xfi_rx_polarity_inverse][phy_dev->xfi_tx_polarity_inverse],
        !phy_dev->xfi_rx_polarity_inverse&&!phy_dev->xfi_tx_polarity_inverse? "": "Inv");
    n += sprintf(buf+n, "%3s", phy_dev->swap_pair? "Swp": "");
    return buf;
}

static char *phy_show_ref_clk(phy_dev_t *phy_dev)
{
    static char buf[64];
    int n = 0;

    buf[0] = 0;
    if (!phy_dev->shared_ref_clk_mhz)
        return buf;
    n += sprintf(buf+n, "%3dMRC", phy_dev->shared_ref_clk_mhz);
    return buf;
}

#define PHY_MAP_INDENT 12
/* *bf is a buffer pointer used for recursive call for cascaded PHYs, 
    it part of local buf and also a flag for new print */
static char *_print_phy_info(phy_dev_t *phy_dev, char *bf, int indent)
{
    static char buf[2024];
    int sz = sizeof(buf), n;
    phy_speed_t max_spd;
    int power_up;
    int apd_enable;
    static char *apd_status[] = {
        [SERDES_NO_POWER_SAVING] = "NoApd",
        [SERDES_BASIC_POWER_SAVING] = "ApdOn", 
        [SERDES_ADVANCED_POWER_SAVING] = "AvApd",
        [SERDES_FORCE_OFF] = "FrcDn",
    }; 
    int cascaded_phy = 0;
    int eee_support, eee_config, eee_resolution, autogreeen;

    if (!phy_dev || (phy_dev->flag & PHY_FLAG_NOT_PRESENTED))
        return "";

    if (bf == 0)
        bf = buf;

    n = 0;
    sz -= (bf - buf); /* Set initial printed bufer size */
    max_spd = phy_max_speed_get(phy_dev);
    if (bf != buf) /* If it is cascaded PHY, add new line and indent space */
    {
        n += snprintf(bf+n, sz-n, "\n%*c", PHY_MAP_INDENT, ' ');
        cascaded_phy = 1;
    }
    else if (phy_dev->cascade_next) /* If it first PHY and cascade, print common inter phy types */
    {
        n += snprintf(bf+n, sz-n, "%s", get_common_inter_types(phy_dev));
        n += snprintf(bf+n, sz-n, "\n%*c", PHY_MAP_INDENT, ' ');
        cascaded_phy = 1;
    }

    phy_dev_power_get(phy_dev, &power_up);
    phy_dev_apd_get(phy_dev, &apd_enable);

    n += snprintf(bf+n, sz-n, "@:%02d:%4s %4s %5s %s %s %4s Max:%-4s %*s %s",
            phy_dev->addr,
            (PhyIsExtPhyId(phy_dev)||phy_dev->phy_drv->phy_type==PHY_TYPE_I2C_PHY)? "Extn":"OnCp",
            (phy_dev->mii_type==PHY_MII_TYPE_RGMII)? "RGMI": phy_dev->cascade_prev? "Cscd": "",
            apd_status[apd_enable],

            phy_show_polarity_swap(phy_dev),
            phy_show_ref_clk(phy_dev),
            power_up? "PwUp": "PwDn",
            phy_dev_speed_to_short_str(max_spd),

            cascaded_phy? 20: 8, PhyIsPortConnectedToExternalSwitch(phy_dev)? "2_EXTSW":
                PhyIsFixedConnection(phy_dev)? "PHY_FIX": phy_dev_get_phy_name(phy_dev),
            print_phy_link(phy_dev));

    phy_dev_eee_support(phy_dev, &eee_support);
    if (eee_support)
    {
        phy_dev_eee_get(phy_dev, &eee_config);
        phy_dev_eee_resolution_get(phy_dev, &eee_resolution);
        phy_dev_eee_mode_get(phy_dev, &autogreeen);
        n += snprintf(bf+n, sz-n, " EEE:%3s", eee_config == 0? "Dis": eee_resolution == 0? "Off": autogreeen? "Gre": "Ntv");
    }
    n += snprintf(bf+n, sz-n, " %04d/%04d", phy_dev->link_flaps, phy_dev->link_flaps_per_min);

    if (phy_dev->cascade_next)
        _print_phy_info(phy_dev->cascade_next, bf+n, indent);

    return buf;
}

static char *print_phy_info(phy_dev_t *phy_dev, int indent)
{
    return _print_phy_info(phy_dev, 0, indent);
}

static char *print_cphy_info(phy_dev_t *phy_dev, int indent)
{
    static char buf[1024];
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
                CROSSBAR_PORT_TO_PHY_PORT(ext_idx),  ext_idx);
        sz = sizeof(buf) - (bf - buf);

        if (phy == crossbar_phy_dev_first(phy_dev))
            indent = bf - buf - 1;  /* -1 for return char */
        bf += snprintf(bf, sz, "%s", print_phy_info(phy, indent));
        sz = sizeof(buf) - (bf - buf);
    }
    return buf;
}

static char *print_port_info(enetx_port_t *p)
{
    static char buf[512];
    enetx_port_t *sw = p->p.parent_sw;
    int root = IS_ROOT_SW(sw);
    struct net_device *dev = p->dev;

    snprintf(buf, sizeof(buf), "%sSw:P%-2d Lgcl:%2d %3s",
            (root?"Int":"Ext"), p->port_info.port,
            PHYSICAL_PORT_TO_LOGICAL_PORT(p->port_info.port, root?0:1),
            dev? (PORT_IS_CFG_AS_WAN(p)? "Wan": (p->p.port_cap == PORT_CAP_WAN_ONLY? "Dwn": "LAN")):"");

    return buf;
}

int sw_get_mac_addresses(enetx_port_t *sw, char **buf, int *sz)
{
    int i, j;
    enetx_port_t *p;
    char *lan_mac = 0, *wan_mac = 0;

    for (j = 0; j < 2; j++)
    {
        for (i = 0; i < sw->s.port_count; i++)
        {
            if(!(p = sw->s.ports[i]) || p->port_class != PORT_CLASS_PORT || !p->dev)
                continue;

            if (PORT_ROLE_IS_WAN(p))
                wan_mac = p->dev->dev_addr;
            else
                lan_mac = p->dev->dev_addr;

            if (wan_mac && lan_mac)
                break;
        }
    }

    if (wan_mac)
        cat_snprintf(buf, sz, "   WAN MAC Address : %02X:%02X:%02X:%02X:%02X:%02X\n", 
                wan_mac[0], wan_mac[1], wan_mac[2],
                wan_mac[3], wan_mac[4], wan_mac[5]);

    if (lan_mac)
        cat_snprintf(buf, sz, "   LAN MAC Address : %02X:%02X:%02X:%02X:%02X:%02X\n", 
                lan_mac[0], lan_mac[1], lan_mac[2],
                lan_mac[3], lan_mac[4], lan_mac[5]);

    return 0;
}

int sw_print_mac_phy_info(enetx_port_t *sw, char **buf, int *sz)
{
    int i, j;
    enetx_port_t *p;
    int indent, s1;

    sw_get_mac_addresses(sw, buf, sz);
    for (j = 0; j < 2; j++)
    {
        for (i = 0; i < sw->s.port_count; i++)
        {
            if(!(p = sw->s.ports[i]))
                continue;

            if (p->port_class != PORT_CLASS_PORT ||
                p->port_info.is_gpon ||
                p->port_info.is_epon ||
                p->port_info.is_epon_ae )
                continue;

            if (j == 0)    /* Parent round */
            {
                s1 = *sz;
                if (cat_snprintf(buf, sz, "%s %s ", print_net_dev_info(p),
                    print_port_info(p)) == 0)
                    return 0;
                if (!p->p.phy) {
                    cat_snprintf(buf, sz, "\n");
                    continue;
                }
                indent = *sz - s1;
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

/* To deal with limited serial port output buffer size issue */
static void console_short_printk(char *buf)
{
#define CON_BUF_SIZE 256
    char *end, *bf = buf;
    char ch;
    int slen, tlen = strlen(buf);

    for(;;) {
        slen = strlen(bf);
        if (slen < CON_BUF_SIZE-1)
            end = bf + slen;
        else
            end = bf + CON_BUF_SIZE-1;
        ch = *end;
        *end = 0;
        pr_cont("%s",bf);
        *end = ch;
        bf = end;
        if (bf == buf + tlen)
            break;
    }
}

static int sw_print_board_ver_mac_phy_info(enetx_port_t *sw, char **buf, int *sz)
{
#if !defined(CONFIG_BRCM_QEMU)
    char board_id[64];

    kerSysNvRamGetBoardId(board_id);
    cat_snprintf(buf, sz, "          Board ID : %s\n", board_id);
#endif
    sw_print_mac_phy_info(sw, buf, sz);
    return *sz;
}

void print_mac_phy_info_all(void)
{
    char *buf;
    int sz = 0;

    if (sw_print_mac_phy_info(root_sw, &buf, &sz)) {
        console_short_printk(buf);
        kfree(buf);
    }
}

void port_print_status_verbose(enetx_port_t *p)
{
    phy_dev_t *phy = get_active_phy(p->p.phy);
    if (!phy) return;
    if (phy->link)
    {
        printk((KERN_CRIT "%s (%s switch port: %d) (Logical Port: %d) (phyId: %x) Link Up at %d mbps %s duplex AN: %s\n"),
                p->dev->name, PORT_ON_ROOT_SW(p)?"Int":"Ext", p->port_info.port, 
                PHYSICAL_PORT_TO_LOGICAL_PORT(p->port_info.port, PORT_ON_ROOT_SW(p)?0:1), phy->addr,
                phy_speed_2_mbps(phy->speed), (phy->duplex==PHY_DUPLEX_FULL)?"full":"half",
                phy->an_enabled? "On": "Off");
    }
    else
    {
        printk((KERN_CRIT "%s (%s switch port: %d) (Logical Port: %d) (phyId: %x) Link DOWN.\n"),
                p->dev->name, PORT_ON_ROOT_SW(p)?"Int":"Ext", p->port_info.port, 
                    PHYSICAL_PORT_TO_LOGICAL_PORT(p->port_info.port, PORT_ON_ROOT_SW(p)?0:1), phy->addr);
    }
}

#ifdef DSL_DEVICES
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

/*
    sub_port: -1 means sub_port to be determined;
              -2 means non crossbar port.
*/
static int find_next_subport(struct net_device *dev, phy_dev_t *phy_dev, int *sub_port)
{
    if (*sub_port == -1)
    {
        phy_dev = crossbar_phy_dev_first(phy_dev);
        crossbar_info_by_phy(phy_dev, NULL, NULL, sub_port);
        *sub_port = CROSSBAR_PORT_TO_PHY_PORT(*sub_port);
        return 1;
    }
    else
    {
        phy_dev = enet_dev_get_phy_dev(dev, *sub_port);
        phy_dev = crossbar_phy_dev_next(phy_dev);
        if (phy_dev)
        {
            crossbar_info_by_phy(phy_dev, NULL, NULL, sub_port);
            *sub_port = CROSSBAR_PORT_TO_PHY_PORT(*sub_port);
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

    if (*sub_port >= 0 && find_next_subport(*dev, phy_dev, sub_port))
        return 1;

    if (port_found) {
        *sub_port = -2;     /* Return non crossbar port for the first ROOT DEVICE */
        return 1;
    }

    if (!phy_is_crossbar(phy_dev) && *sub_port == -1) {  /* If is it first non-crossbar phy, return PHY and mark sub_port as -2 */
        *sub_port = -2;
        return 1;
    }

    /* Try to find next port */
    enet_get_next_port(port, &port);
    if (port == NULL)
        return 0;

    *sub_port = -1;
    *dev = port->dev;
    phy_dev = port->p.phy;
    if (phy_is_crossbar(phy_dev))
        return find_next_subport(*dev, phy_dev, sub_port);
    else
        *sub_port = -2;

    return 1;
}
#endif

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
    tr_find_port_type_t port_type = {PHY_TYPE_158CLASS_SERDES};
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
            break;
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

#if defined(BCM_DEEP_SLEEP)
extern int bcm_wake_on_lan(mac_dev_t *mac_dev, phy_dev_t *phy_dev, int is_internal, int is_lnk);
extern int bcm_wake_on_button(void);
extern int bcm_wake_on_timer(int minutes);
extern int bcm_wake_on_wan(void);
extern int bcm_deepsleep_start(void);
#else
static int bcm_wake_on_lan(mac_dev_t *mac_dev, phy_dev_t *phy_dev, int is_internal, int is_lnk) { return 0; }
static int bcm_wake_on_button(void) { return 0; }
static int bcm_wake_on_timer(int minutes) { return 0; }
static int bcm_wake_on_wan(void) { return 0; }
static int bcm_deepsleep_start(void) { return 0; }
#endif

int enet_ioctl_compat_ethctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
    struct ethctl_data rq_data;
    struct ethctl_data *ethctl = &rq_data;
    enetx_port_t *port = NULL;
    phy_dev_t *phy_dev;
    int ret = 0;
    ethcd_t *ethcd;

    if (copy_from_user(ethctl, rq->ifr_data, sizeof(struct ethctl_data)))
        return -EFAULT;

    switch(ethctl->op)
    {
    case ETHGETSOFTWARESTATS:
        {
            char *buf;
            port = ((enetx_netdev *)netdev_priv(dev))->port;
            
            if(ethctl->buf_size <= 0)
                return -EINVAL;

            /* coverity[tainted_data] */
            /* various buf_size are possible supress warning */
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

        phy_dev = enet_dev_get_phy_dev(dev, ETHCTL_SUBPORT_GET_SUBPORT(ethctl->sub_port));
        if(!phy_dev)
            return -EFAULT;

        if (phy_dev->phy_drv->phy_type == PHY_TYPE_MAC2MAC)
            return -EINVAL;

        if (ethctl->sub_port & ETHCTL_SUBPORT_CASCADE_ENDPHY)
            phy_dev = cascade_phy_get_last(phy_dev);
        ethctl->sub_port = ETHCTL_SUBPORT_GET_SUBPORT(ethctl->sub_port);

        ethctl->phy_addr = phy_dev->addr;
        ethctl->flags = 0;

        /* Let us also return phy flags needed for accessing the phy */
        if (cascade_phy_get_prev(phy_dev))
            phy_dev = cascade_phy_get_prev(phy_dev); /* Get Serdes Pointer */
        switch (phy_dev->phy_drv->phy_type) {
            case PHY_TYPE_138CLASS_SERDES:
            case PHY_TYPE_146CLASS_SERDES:
            case PHY_TYPE_6756CLASS_SERDES:
                ethctl->flags = ETHCTL_FLAG_ACCESS_SERDES;
                break;
            case PHY_TYPE_158CLASS_SERDES:
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

            if(ethctl->buf_size <= 0)
                return -EINVAL;

            ethctl->val = 0;
            BCM_IOC_PTR_ZERO_EXT(ethctl->buf);
            if (sw_print_board_ver_mac_phy_info(root_sw, &buf, &ethctl->val) == 0)
                goto mapFail;

            if (copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)))
                goto freeMapFail;
            /* coverity[tainted_data] */
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

#if defined(DSL_DEVICES)
        case ETHCTLTXFIR:
            phy_dev = phy_dev_get(PHY_TYPE_UNKNOWN, ethctl->phy_addr);
            if(!phy_dev || !phy_dev->phy_drv)
            {
                enet_err("No PHY at address %d has been found.\n", ethctl->phy_addr);
                return -EFAULT;
            }
            if (ethctl->flags & ETHCTL_SET)
                ret = phy_priv_fun(phy_dev, PHY_OP_SET_TXFIR, &ethctl->txfir);
            else
                ret = phy_priv_fun(phy_dev, PHY_OP_GET_TXFIR, &ethctl->txfir);
            ethctl->ret_val = ret;
            if (copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)))
                return -EFAULT;
            return ret;
#endif
        case ETHCDCTL:
            ret = 0;
            ethcd = &ethctl->ethcd;
#ifdef DSL_DEVICES
            if (ethcd->flag & ETHCD_FLAG_INTERFACE_NEXT)
            {
                if(enet_get_next_phy(&dev, &ethctl->sub_port) == 0)
                {
                    ethcd->return_value = ETHCD_NO_NEXT_IF;
                    goto cd_end;
                }
            }

            phy_dev = enet_dev_get_phy_dev(dev, ethctl->sub_port);
#else
            port = ((enetx_netdev *)netdev_priv(dev))->port;
            phy_dev = port->p.phy;
#endif
            if(!phy_dev)
                return -EFAULT;
            phy_dev = cascade_phy_get_last(phy_dev);
            ethctl->phy_addr = ethcd->phy_addr = phy_dev->addr;


            phy_dev->ethcd.op = ethcd->op;
            switch(ethcd->op)
            {
                case ETHCD_GET:
                    phy_dev_cable_diag_get(phy_dev, &ethcd->return_value);
                    *ethcd = *(ethcd_t *)&phy_dev->ethcd;
                    break;

                case ETHCD_SET:
                    phy_dev_cable_diag_set(phy_dev, ethcd->value);
                    *ethcd = *(ethcd_t *)&phy_dev->ethcd;
                    break;

                case ETHCD_QUERY:
                    phy_dev_cable_diag_query(phy_dev);
                    *ethcd = *(ethcd_t *)&phy_dev->ethcd;
                    break;

                case ETHCD_RUN:
                    phy_dev_cable_diag_run(phy_dev);
                    *ethcd = *(ethcd_t *)&phy_dev->ethcd;
                    break;
            }
#ifdef DSL_DEVICES
cd_end:
#endif
            strcpy(ethctl->ifname, dev->name);
            ethctl->phy_addr = ethcd->phy_addr = phy_dev->addr;
            if (copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)))
                return -EFAULT;
            return ret;

    case ETHGETMIIREG: /* Read MII PHY register */
    case ETHSETMIIREG: /* Write MII PHY register */
#if defined(DSL_DEVICES)
        {
            int ret = 0;
            phy_dev_t *phy_dev;
            phy_type_t phy_type = PHY_TYPE_UNKNOWN;
            static char *phy_type_str[PHY_TYPE_MAX] =
            {
                [PHY_TYPE_UNKNOWN] = "PHY_TYPE_UNKNOWN",
                [PHY_TYPE_I2C_PHY] = "PHY_TYPE_I2C",
                [PHY_TYPE_158CLASS_SERDES] = "PHY_TYPE_158CLASS_SERDES",
            };

            if (ethctl->flags & (ETHCTL_FLAG_ACCESS_I2C_PHY | ETHCTL_FLAG_ACCESS_I2C_PHY_EEPROM))
                phy_type = PHY_TYPE_I2C_PHY;
            else if (ethctl->flags & (ETHCTL_FLAG_ACCESS_10GSERDES|ETHCTL_FLAG_ACCESS_10GPCS))
                phy_type = PHY_TYPE_158CLASS_SERDES;
            else 
                phy_type = PHY_TYPE_UNKNOWN;

            phy_dev = phy_dev_get(phy_type, ethctl->phy_addr);
            if(!phy_dev || !phy_dev->phy_drv)
            {
                enet_err("No PHY type %s at address %d has been found.\n", phy_type_str[phy_type], ethctl->phy_addr);
                return -EFAULT;
            }

            down(&root_sw->s.conf_sem);


            if (ethctl->flags == ETHCTL_FLAG_ACCESS_SILENT_START)
            {
                /* The single PHY is specified, so should not operate on cascade */
                if (ethctl->op == ETHSETMIIREG)
                    ret = phy_dev_silent_start_set(phy_dev, ethctl->val); 
                else
                    ret = phy_dev_silent_start_get(phy_dev, &ethctl->val);
            }
            else if (ethctl->flags == ETHCTL_FLAG_ACCESS_SERDES_POWER_MODE)
            {
                /* The single PHY is specified, so should not operate on cascade */
                if (ethctl->op == ETHSETMIIREG)
                    ret = phy_dev_apd_set(phy_dev, ethctl->val); 
                else
                    ret = phy_dev_apd_get(phy_dev, &ethctl->val);
            }
            else if (ethctl->flags & ETHCTL_FLAG_ACCESS_10GSERDES)
            {
                if (ethctl->op == ETHGETMIIREG)
                {
                    ret = phy_priv_fun(phy_dev, SERDES_OP_RD_PMD, ethctl->phy_reg, &ethctl->val);
                }
                else
                {
                    ret = phy_priv_fun(phy_dev, SERDES_OP_WR_PMD, ethctl->phy_reg, ethctl->val<<16);
                }
            }
            else if (ethctl->flags & ETHCTL_FLAG_ACCESS_10GPCS)
            {
                if (ethctl->op == ETHGETMIIREG)
                {
                    ret = phy_priv_fun(phy_dev, SERDES_OP_RD_PCS, ethctl->phy_reg, &ethctl->val);
                }
                else
                {
                    ret = phy_priv_fun(phy_dev, SERDES_OP_WR_PCS, ethctl->phy_reg, ethctl->val<<16);
                }
            } 
            else if (ethctl->flags & ETHCTL_FLAG_ACCESS_SERDES_TIMER) 
            {
                if (ethctl->op == ETHGETMIIREG)
                {
                    ret = phy_priv_fun(phy_dev, SERDES_OP_TMR_GET,
                        &ethctl->short_tmr_ms, &ethctl->short_tmr_weight,
                        &ethctl->long_tmr_ms, &ethctl->long_tmr_weight);
                }
                else
                {
                    ret = phy_priv_fun(phy_dev, SERDES_OP_TMR_SET,
                        ethctl->short_tmr_ms, ethctl->short_tmr_weight,
                        ethctl->long_tmr_ms, ethctl->long_tmr_weight, ethctl->val);
                }
            } 
            else if (ethctl->flags & ETHCTL_FLAG_ACCESS_I2C_PHY_EEPROM)
            {
                void *buf;

                phy_dev = cascade_phy_get_prev(phy_dev);
                if (!(buf = kzalloc(ethctl->buf_size, GFP_KERNEL)))
                    return -EFAULT;
                ret = phy_priv_fun(phy_dev, SERDES_OP_DUMP_EEPROM, ethctl->val/2, ethctl->phy_reg, buf, ethctl->buf_size);
                if (ret == 0)
                    copy_to_user(ethctl->buf, buf, ethctl->buf_size);
                kfree(buf);
            }
            else
            {
                uint16_t val = ethctl->val;
                if (phy_dev->phy_drv && phy_dev->phy_drv->priv_fun)
                {
                    if (ethctl->op == ETHGETMIIREG)
                        ret = phy_priv_fun(phy_dev, PHY_OP_RD_MDIO, ethctl->phy_reg, &ethctl->val);
                    else
                        ret = phy_priv_fun(phy_dev, PHY_OP_WR_MDIO, ethctl->phy_reg, ethctl->val);
                }
                else
                {
                    if (ethctl->op == ETHGETMIIREG)
                    {
                        ret = phy_bus_read(phy_dev, (uint16_t)ethctl->phy_reg, &val);
                        ret = phy_bus_read(phy_dev, MII_STATUS, &val);
                        ret = phy_bus_read(phy_dev, (uint16_t)ethctl->phy_reg, &val);
                        if (!ret)
                            ethctl->val = (int)val;
                    }
                    else
                        ret = phy_bus_write(phy_dev, (uint16_t)ethctl->phy_reg, (uint16_t)ethctl->val);
                }
            }

            up(&root_sw->s.conf_sem);

            if (ret)
            {
                ethctl->ret_val = ret;
                ret = 0;
            }

            if (copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)))
                return -EFAULT;

            return 0;
        }
#elif (defined(CONFIG_BCM_PON) && !defined(CONFIG_BRCM_QEMU))
        {
            uint16_t val = ethctl->val;
            phy_dev_t *phy_dev = phy_dev_get(PHY_TYPE_UNKNOWN, ethctl->phy_addr);

            if (!phy_dev || !phy_dev->phy_drv)
            {
                enet_err("No PHY at address %d has been found.\n", ethctl->phy_addr);
                return -EFAULT;
            }

            if (ethctl->flags == ETHCTL_FLAG_ACCESS_SILENT_START)
            {
                /* The single PHY is specified, so should not operate on cascade */
                if (ethctl->op == ETHSETMIIREG)
                    ret = phy_dev_silent_start_set(phy_dev, ethctl->val); 
                else
                    ret = phy_dev_silent_start_get(phy_dev, &ethctl->val);
            }
            else
            {
                if (ethctl->op == ETHGETMIIREG)
                    ret = ethsw_phy_exp_read(phy_dev, ethctl->phy_reg, &val);
                else
                    ret = ethsw_phy_exp_write(phy_dev, ethctl->phy_reg, val);
            }

            if (ret)
                return ret;

            ethctl->val = val;

            if (copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)))
                return -EFAULT;

            return 0;
        }
#endif /* defined(CONFIG_BCM_PON) */
#ifdef DSL_DEVICES
    case ETHMOVESUBPORT:
        {
            port = ((enetx_netdev *)netdev_priv(dev))->port;
            ethctl->ret_val = 0;

            if (port == root_sw) {  // get next interface-phy mapping
                struct tr_portphy_info info = {};
                strncpy(info.ifname, ethctl->ifname, sizeof(info.ifname)-1);
                info.ifname[sizeof(info.ifname)-1]='\0';
                if (port_traverse_ports(root_sw, tr_get_next_portphy_info, PORT_CLASS_PORT, &info) <= 0)
                    return -ENODATA;

                ethctl->ret_val = info.mapping << CROSSBAR_PORT_BASE;
                strcpy(ethctl->ifname, info.ifname);
            } else { // move operation
                struct tr_subport_info info = {};
                info.external_endpoint = PHY_PORT_TO_CROSSBAR_PORT(ethctl->sub_port);

                if (dev->flags & IFF_UP) {
                    ethctl->ret_val = ETHMOVESUBPORT_RET_DSTDEV_UP;     // err: dest dev is not ifconfig down
                } else if (port_traverse_ports(root_sw, tr_get_subport_info, PORT_CLASS_PORT, &info) <= 0) {
                    ethctl->ret_val = ETHMOVESUBPORT_RET_INVALID_EP;    // err: subport not valid or not currently assigned
                } else if (info.port->dev->flags & IFF_UP) {
                    strcpy(ethctl->ifname, info.port->dev->name);
                    ethctl->ret_val = ETHMOVESUBPORT_RET_SRCDEV_UP;     // err: src dev is not ifconfig down
                } else if (phy_is_mac_to_mac(info.phy)) {
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
#endif
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

    case ETHPHYWAKEENABLE:
        {
            wol_params_t wol_params = {};
            mac_dev_t *mac_dev;
            int is_internal;

            if (!ethctl->flags)
                return bcm_deepsleep_start();

            if (ethctl->flags & ETHCTL_FLAG_WAKE_BTN_SET)
                return bcm_wake_on_button();

            if (ethctl->flags & ETHCTL_FLAG_WAKE_TME_SET)
                return bcm_wake_on_timer(ethctl->val);

            if (ethctl->flags & ETHCTL_FLAG_WAKE_WAN_SET)
                return bcm_wake_on_wan();

            mac_dev = ((enetx_netdev *)netdev_priv(dev))->port->p.mac;
            if (!mac_dev)
                return -EFAULT;

            phy_dev = enet_dev_get_phy_dev(dev, ethctl->sub_port);
            if (!phy_dev)
                return -EFAULT;

            phy_dev = cascade_phy_get_last(phy_dev);

            /* Reduce PHY speed in order to save power */
            if (ethctl->flags & ETHCTL_FLAG_WAKE_SPD_SET)
                phy_dev_speed_set(phy_dev, ethctl->spd, PHY_DUPLEX_FULL);

            /* Set magic packet if provided or from network device */
            if (ethctl->flags & ETHCTL_FLAG_WAKE_MAC_SET)
                memcpy(wol_params.mac_addr, ethctl->mac, ETH_ALEN);
            else
                memcpy(wol_params.mac_addr, dev->dev_addr, ETH_ALEN);

            /* Set password if provided */
            if (ethctl->flags & ETHCTL_FLAG_WAKE_PSW_SET)
            {
                wol_params.en_psw = 1;
                memcpy(wol_params.password, ethctl->psw, ETH_ALEN);
            }

            /* Set number of repetitions if provided */
            if (ethctl->flags & ETHCTL_FLAG_WAKE_REP_SET)
                wol_params.repetitions = ethctl->rep;

           /* Set the detect method (magic packet, ARP or link up) */
            wol_params.en_mpd = (ethctl->flags & ETHCTL_FLAG_WAKE_MPD_SET) ? 1 : 0;
            wol_params.en_ard = (ethctl->flags & ETHCTL_FLAG_WAKE_ARD_SET) ? 1 : 0;
            wol_params.en_lnk = (ethctl->flags & ETHCTL_FLAG_WAKE_LNK_SET) ? 1 : 0;
            is_internal = (ethctl->flags & ETHCTL_FLAG_WAKE_INT_SET) ? 1 : 0;

            /* Set the detect location (MAC or PHY) */
            if (is_internal)
                ret = mac_dev_wol_enable(mac_dev, &wol_params);
            else
                ret = phy_dev_wol_enable(phy_dev, &wol_params);

            bcm_wake_on_lan(mac_dev, phy_dev, is_internal, wol_params.en_lnk);

            return ret ? EFAULT : 0;
        }

#if defined(CONFIG_BCM_ENERGY_EFFICIENT_ETHERNET)
    case ETHGETPHYEEE:
        phy_dev = enet_dev_get_phy_dev(dev, ethctl->sub_port);
        phy_dev = cascade_phy_get_last_active(phy_dev);
        if (!phy_dev)
            return -EFAULT;
    
        if (ethctl->val2 & ETHCTL_VAL_SET_FLAG) {
            if ((ret = cascade_phy_dev_eee_mode_set(phy_dev, ethctl->val2 & ~(ETHCTL_VAL_SET_FLAG)))) {
                ethctl->ret_val2 = -1;
                if(copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data))) return -EFAULT; 
                return -EFAULT;
            }
            return 0;
        }

        cascade_phy_dev_eee_get(phy_dev, &ethctl->ret_val);
        ethctl->ret_val2 = phy_dev->autogreeen;
        return copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)) ? -EFAULT : 0;
    case ETHGETPHYEEERESOLUTION:
        phy_dev = enet_dev_get_phy_dev(dev, ethctl->sub_port);
        phy_dev = cascade_phy_get_last_active(phy_dev);
        if (!phy_dev)
            return -EFAULT;
        cascade_phy_dev_eee_resolution_get(phy_dev, &ethctl->ret_val);
        ethctl->ret_val2 = phy_dev->autogreeen;
        return copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)) ? -EFAULT : 0;
    case ETHSETPHYEEEON:
    case ETHSETPHYEEEOFF:
        phy_dev = enet_dev_get_phy_dev(dev, ethctl->sub_port);
        if (!phy_dev)
            return -EFAULT;
        if (ethctl->val2 & ETHCTL_VAL_SET_FLAG) {
            if ((ret = cascade_phy_dev_eee_mode_set(phy_dev, ethctl->val2 & ~(ETHCTL_VAL_SET_FLAG)))) {
                ethctl->ret_val2 = -1;
                if(copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data))) return -EFAULT;
                return -EFAULT;
            }
        }
        return cascade_phy_dev_eee_set(phy_dev, ethctl->op == ETHSETPHYEEEON ? 1 : 0);
#endif // #if defined(CONFIG_BCM_ENERGY_EFFICIENT_ETHERNET)
#if defined(CONFIG_BCM_ETH_PWRSAVE)
    case ETHGETPHYAPD:
        phy_dev = enet_dev_get_phy_dev(dev, ethctl->sub_port);
        if (!phy_dev)
            return -EFAULT;
        cascade_phy_dev_apd_get(phy_dev, &ethctl->ret_val);
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
        phy_dev_t *phy;

        port = ((enetx_netdev *)netdev_priv(dev))->port;

#if defined(DSL_DEVICES)
        phy = get_active_phy(port->p.phy);
#else  
        phy = port->p.phy;      
#endif 
        if(copy_from_user(&data, rq->ifr_data + sizeof(struct ethctl_data), sizeof(macsec_api_data))) return -EFAULT;
        
        phy_dev_macsec_oper(phy, &data);
        return copy_to_user(rq->ifr_data + sizeof(struct ethctl_data), &data, sizeof(macsec_api_data)) ? -EFAULT : 0;
    }

    case ETHWIRESPEEDGET:
    case ETHWIRESPEEDSET:
    {
        ret = 0;

        phy_dev = enet_dev_get_phy_dev(dev, ethctl->sub_port);
        if (!phy_dev)
            return -EFAULT;
        phy_dev = cascade_phy_get_last(phy_dev);

        if (ethctl->op == ETHWIRESPEEDSET)
            ret |= phy_dev_wirespeed_set(phy_dev, ethctl->val);

        if (ethctl->op == ETHWIRESPEEDGET || ethctl->op == ETHWIRESPEEDSET)
            ret |= phy_dev_wirespeed_get(phy_dev, &ethctl->ret_val);

        if (copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)))
            return -EFAULT;

        return ret;
    }
#if defined(CONFIG_GT7)
    case ETHGETEXT84991MIIREG: /* Read MII PHY indirectly connected BCM84991 register */
    case ETHSETEXT84991MIIREG: /* Write MII PHY indirectly connected BCM84991 register */
	{
		int ret = 0;
		uint32_t phy_addr, phy_devad;
		uint16_t phy_reg;

		phy_addr = ethctl->phy_addr;
		phy_devad = ethctl->phy_reg >> 16;
		phy_reg = (uint16_t)ethctl->phy_reg;

		down(&root_sw->s.conf_sem);

		if (ethctl->op == ETHGETEXT84991MIIREG)
			ret = mdio_read_c45_register(phy_addr, phy_devad, phy_reg, (uint16_t*)&ethctl->val);
		else
			ret = mdio_write_c45_register(phy_addr, phy_devad, phy_reg, ethctl->val);

		up(&root_sw->s.conf_sem);

		if (ret)
			return -EFAULT;

		if (copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)))
			return -EFAULT;

		return 0;
	}

    case ETHGETEXT84991PHYPWR:
		if (phyext84991_power_get(ethctl->phy_addr, &ethctl->val))
			return -EFAULT;

		if (copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)))
			return -EFAULT;

        return 0;

    case ETHSETEXT84991PHYPWRON:
    case ETHSETEXT84991PHYPWROFF:
        return phyext84991_power_set(ethctl->phy_addr, ethctl->op == ETHSETEXT84991PHYPWRON) ? -EFAULT : 0;

    case ETHSETEXT84991PHYRESET:
        return phyext84991_phy_reset(ethctl->phy_addr) ? -EFAULT : 0;

    case ETHGETEXT84991MEDIATYPE:
    case ETHSETEXT84991MEDIATYPE:
		if (phyext84991_mediatype(ethctl))
			return -EFAULT;

		if (copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)))
			return -EFAULT;

        return 0;

    case ETHSETEXT84991EEEON:
    case ETHSETEXT84991EEEOFF:
        if (phyext84991_phy_eee_autogreeen_read(ethctl->phy_addr, &ethctl->ret_val2))
			return -EFAULT;

		if (phyext84991_phy_eee_set(ethctl->phy_addr, ethctl->op == ETHSETEXT84991EEEON ? 1 : 0))
			return -EFAULT;
        ethctl->ret_val = (ethctl->op == ETHSETEXT84991EEEON ? 1 : 0);

        if (ethctl->val2 & ETHCTL_VAL_SET_FLAG) {
            if (phyext84991_phy_eee_mode_set(ethctl->phy_addr, ethctl->val2 & ~(ETHCTL_VAL_SET_FLAG))) {
                ethctl->ret_val2 = -1;
                return -EFAULT;
            }
            ethctl->ret_val2 = ethctl->val2 & ~(ETHCTL_VAL_SET_FLAG);
        }

        if (copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)))
            return -EFAULT;

        return 0;

    case ETHGETEXT84991EEE:
		if (phyext84991_phy_eee_get(ethctl->phy_addr, &ethctl->ret_val))
			return -EFAULT;

        if (phyext84991_phy_eee_autogreeen_read(ethctl->phy_addr, &ethctl->ret_val2))
			return -EFAULT;

		if (copy_to_user(rq->ifr_data, ethctl, sizeof(struct ethctl_data)))
			return -EFAULT;

        return 0;
#endif
    default:
        return -EOPNOTSUPP;
    }

    enet_dbgv(" dev=%s, cmd=%x(SIOCETHCTLOPS), unknown ops=%d\n", NETDEV_PRIV(dev)->port->obj_name, cmd, ethctl->op);
    return -EOPNOTSUPP;
}

static int tr_get_soft_sw_map(enetx_port_t *port, void *_ctx)
{
    int *map = (int *)_ctx;

    if (port->dev && port->p.port_cap != PORT_CAP_MGMT &&  !is_netdev_hw_switch(port->dev)) {
        *map |= 1<< PHYSICAL_PORT_TO_LOGICAL_PORT(port->port_info.port, PORT_ON_ROOT_SW(port)?0:1);
    }

    return 0;
}

static int tr_net_dev_hw_switching_set(enetx_port_t *port, void *_ctx)
{
    struct ethswctl_data *ethswctl = (struct ethswctl_data *)_ctx;

#if defined(EPON_SFU) || defined (GPON_SFU)
    /* in SFU WAN port can be configured as HW switch port in order to allow HW flooding from it */
    if (!port->dev)
#else /* ! (EPON_SFU || GPON_SFU) */
    if (port->n.port_netdev_role == PORT_NETDEV_ROLE_WAN || !port->dev)
#endif /* EPON_SFU || GPON_SFU */
        return 0;

    if (ethswctl->type == TYPE_ENABLE)
        netdev_hw_switch_set(port->dev);
    else
        netdev_hw_switch_unset(port->dev);

    if (port->p.ops->stp_set && (port->p.port_cap != PORT_CAP_MGMT)) {
        port->p.ops->stp_set(port, (ethswctl->type == TYPE_ENABLE)? STP_MODE_ENABLE:STP_MODE_DISABLE, STP_STATE_UNCHANGED);
        port->p.ops->role_set(port,(ethswctl->type == TYPE_ENABLE)? PORT_NETDEV_ROLE_LAN:PORT_NETDEV_ROLE_NONE);
    }
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
        ethswctl->ret_val = EOPNOTSUPP;
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
        ethswctl->ret_val = EFAULT;
        return -EFAULT;
    }
    ethswctl->ret_val = 0;

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

static int tr_port_mib_dump(enetx_port_t *port, void *_ctx)
{
    struct ethswctl_data *ethswctl = (struct ethswctl_data *) _ctx;
    switch (ethswctl->unit) {
    case 0:  if (!PORT_ON_ROOT_SW(port)) return 0; break;
    case 1:  if (PORT_ON_ROOT_SW(port)) return 0; break;
    }
    port_mib_dump(port, ethswctl->type, ethswctl->sub_type);
    return 0;
}

struct obj_name_ctx 
{
    char *name;
    enetx_port_t *port;
} typedef obj_name_ctx_t;

static int tr_find_port_obj_name(enetx_port_t *port, void *_ctx)
{
    obj_name_ctx_t *c = (obj_name_ctx_t *)_ctx;
    if (strncmp(port->obj_name, c->name, OBJIFNAMSIZ))
        return 0;
    c->port = port;
    return 1;
}

static enetx_port_t *get_port_by_obj_name(char *name)
{
    obj_name_ctx_t ctx;
    ctx.name = name;
    ctx.port = NULL;
    port_traverse_ports(root_sw, tr_find_port_obj_name, PORT_CLASS_PORT, &ctx);
    return ctx.port;
}

static enetx_port_t *get_ioctl_port(struct net_device *dev, struct ethswctl_data *ethswctl)
{
    enetx_port_t *port = NULL;
    if (ethswctl->addressing_flag & ETHSW_ADDRESSING_DEV)
        port = ((enetx_netdev *)netdev_priv(dev))->port;
    else 
        port = get_port_by_obj_name(ethswctl->ifname);
    return port;
}

int enet_ioctl_compat_ethswctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
    struct ethswctl_data rq_data;
    struct ethswctl_data *ethswctl = &rq_data;
    phy_dev_t *phy_dev;
    enetx_port_t *port = NULL;
    uint32_t supported_caps = 0;
    int i;
    int ret;

    ret = 0;

    if (copy_from_user(ethswctl, rq->ifr_data, sizeof(struct ethswctl_data)))
        return -EFAULT;

    switch(ethswctl->op)
    {
        case ETHSWPORTMCASTGEMSET:
            if (gpon_mcast_gem_set(ethswctl->val))
                return -EFAULT;

            return 0;
        case ETHSWPORTCREATE:
            if (_handle_add_port(&ethswctl->net_port))
                return -EFAULT;

            if (copy_to_user((void *)rq->ifr_data, ethswctl, sizeof(struct ethswctl_data)))
                return -EFAULT;

            return 0;
        case ETHSWPORTDELETE:
            if (_handle_del_port(ethswctl->net_port.port))
                return -EFAULT;

            return 0;
        case ETHSWPORTSERDESNAME:
            if (_handle_serdes_name(&ethswctl->net_port))
                return -EFAULT;

            if (copy_to_user((void *)rq->ifr_data, ethswctl, sizeof(struct ethswctl_data)))
                return -EFAULT;

            return 0;
        case ETHSWPORTTXQSTATE:
            {
                uint32_t bitmap = 1 << ethswctl->queue;

                if (get_port_by_if_name(ethswctl->ifname, &port))
                    return -EFAULT;

                switch (ethswctl->type) {
                case TYPE_ENABLE:
                    if (!(port->p.enabled_txq_map & bitmap)) {  // enable txq
                        port->p.enabled_txq_map |= bitmap;
                    }
                    break;
                case TYPE_DISABLE:
                    if (port->p.enabled_txq_map & bitmap) {     // disable txq
                        // flush flow?
                        port->p.enabled_txq_map &= ~bitmap;
                    }
                    break;
                }
                return 0;
            }
            
        case ETHSWPORTTRAFFICCTRL:
            {
               port = get_ioctl_port(dev, ethswctl);

               if (!port)
                    return -EFAULT;

                if (ethswctl->type == TYPE_GET)
                {
                    enet_err("%s ETHSWPORTTRAFFICCTRL: Unsupported request\n", __FUNCTION__);
                    return -EFAULT;
                }

                if (ethswctl->type == TYPE_SET)
                {
                    if (ethswctl->val)
                        port_generic_open(port);
                    else
                        port_generic_stop(port);
                }

                return 0;
            }

        case ETHSWDUMPMIB:
            {
                if (ethswctl->port == -1)
                {
                    port_traverse_ports(root_sw, tr_port_mib_dump, PORT_CLASS_PORT, ethswctl);
                    return 0;
                }
                if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
                    return -EFAULT;
                return port_mib_dump(port, ethswctl->type, ethswctl->sub_type);
            }

        case ETHSWBONDCHK:
            {
                enetx_port_t *port1, *port2;
                char *errStr = NULL;
                
                if (get_port_by_if_name(ethswctl->bond_chk.ifname1, &port1) ||
                    get_port_by_if_name(ethswctl->bond_chk.ifname2, &port2))
                    errStr = "Device not found";
                else {
#if defined(SF2_DUAL) || defined(SF2_EXTERNAL)
                    if (port1->p.parent_sw != port2->p.parent_sw)
                        errStr = "Devices on different switch units";
#endif                    
                }
                if (errStr) {
                    BCM_IOC_PTR_ZERO_EXT(ethswctl->bond_chk.uptr);
                    ethswctl->bond_chk.len = strlen(errStr);
                    if(copy_to_user(ethswctl->bond_chk.uptr, errStr, ethswctl->bond_chk.len))
                        return -EFAULT;
                } else
                    ethswctl->bond_chk.len=0;

                if (copy_to_user((void *)rq->ifr_data, ethswctl, sizeof(struct ethswctl_data)))
                    return -EFAULT;
                return errStr ? -1 : 0;
            }
            
#if defined(SF2_DEVICE)
#if !defined(SF2_EXTERNAL)
        case ETHSWACBCONTROL:
            BCM_IOC_PTR_ZERO_EXT(ethswctl->vptr);
            return ioctl_extsw_cfg_acb(ethswctl);
#endif //!SF2_EXTERNAL
        case ETHSWCONTROL:
            if (!IS_UNIT_SF2(ethswctl->unit)) return -(EOPNOTSUPP);

            ret = ioctl_extsw_control(ethswctl);
            if (!ret && copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                return -EFAULT;

            return ret;
        case ETHSWPRIOCONTROL:
            if (!IS_UNIT_SF2(ethswctl->unit)) return -(EOPNOTSUPP);

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
            if (!IS_UNIT_SF2(ethswctl->unit)) return -(EOPNOTSUPP);

            ret = ioctl_extsw_que_mon(ethswctl);
            if (!ret && copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                return -EFAULT;

            return ret;
        case ETHSWMACLMT:
            if (!IS_UNIT_SF2(ethswctl->unit)) return -(EOPNOTSUPP);

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
        case ETHSWPDEFVLAN:
            if (!IS_UNIT_SF2(ethswctl->unit)) return -(EOPNOTSUPP);

            ret = ioctl_extsw_defvlan(ethswctl);
            if (!ret && (ethswctl->type == TYPE_GET))
                if (ethswctl_field_copy_to_user(vid))
                    return -EFAULT;

            return ret;
        case ETHSWPBVLAN:
            if (!IS_UNIT_SF2(ethswctl->unit)) return -(EOPNOTSUPP);

            ret = ioctl_extsw_pbvlan(ethswctl);
            if (!ret && (ethswctl->type == TYPE_GET))
                if (ethswctl_field_copy_to_user(fwd_map))
                    return -EFAULT;

            return ret;
        case ETHSWMIRROR:
            if (!IS_UNIT_SF2(ethswctl->unit)) {
                return -(EOPNOTSUPP);
            }

            ret = ioctl_extsw_port_mirror_ops(ethswctl);
            if (!ret && (ethswctl->type == TYPE_GET))
                if (ethswctl_field_copy_to_user(port_mirror_cfg))
                    return -EFAULT;
            return ret;
        case ETHSWPORTTRUNK:
            if (!IS_UNIT_SF2(ethswctl->unit)) return -(EOPNOTSUPP);

            ret = ioctl_extsw_port_trunk_ops(ethswctl);
            if (!ret && (ethswctl->type == TYPE_GET))
                if (ethswctl_field_copy_to_user(port_trunk_cfg))
                    return -EFAULT;
            return ret;
        case ETHSWREGACCESS:
            if (!IS_UNIT_SF2(ethswctl->unit)) return -(EOPNOTSUPP);
            if (ethswctl->offset & IS_PHY_ADDR_FLAG) {
                port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->offset & PORT_ID_M);
                if (!port) return BCM_E_ERROR;
                ret = ioctl_extsw_regaccess(ethswctl, port);
            } 
            else
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
            if (!IS_UNIT_SF2(ethswctl->unit)) return -(EOPNOTSUPP);

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
            if (!IS_UNIT_SF2(ethswctl->unit)) return -(EOPNOTSUPP);

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
            if (!IS_UNIT_SF2(ethswctl->unit)) {
                enet_err("runner COS priority method config not supported yet.\n");    //TODO_DSL?
                return -(EOPNOTSUPP);
            }

            ret = ioctl_extsw_cos_priority_method_cfg(ethswctl);
            if (!ret && (ethswctl->type == TYPE_GET))
                if (ethswctl_field_copy_to_user(ret_val))
                    return -EFAULT;
            return ret;
        case ETHSWJUMBO:
            if (!IS_UNIT_SF2(ethswctl->unit)) return -(EOPNOTSUPP);

            ret = ioctl_extsw_port_jumbo_control(ethswctl);
            if (!ret && ethswctl_field_copy_to_user(ret_val))
                return -EFAULT;
            return ret;

        case ETHSWCOSPCPPRIOMAP:
            if (!IS_UNIT_SF2(ethswctl->unit)) return 0;

            ret = ioctl_extsw_pcp_to_priority_mapping(ethswctl);
            if (!ret && (ethswctl->type == TYPE_GET))
                if (ethswctl_field_copy_to_user(priority))
                    return -EFAULT;
            return ret;
        case ETHSWCOSPIDPRIOMAP:
            if (!IS_UNIT_SF2(ethswctl->unit)) {
                enet_err("runner COS PID priority mapping not supported yet.\n");    //TODO_DSL?
                return -(EOPNOTSUPP);
            }

            ret = ioctl_extsw_pid_to_priority_mapping(ethswctl);
            if (!ret && (ethswctl->type == TYPE_GET))
                if (ethswctl_field_copy_to_user(priority))
                    return -EFAULT;
            return ret;
        case ETHSWCOSDSCPPRIOMAP:
            if (!IS_UNIT_SF2(ethswctl->unit)) return 0;

            ret = ioctl_extsw_dscp_to_priority_mapping(ethswctl);
            if (!ret && (ethswctl->type == TYPE_GET))
                if (ethswctl_field_copy_to_user(priority))
                    return -EFAULT;
            return ret;

        case ETHSWPORTSHAPERCFG:
            if (!IS_UNIT_SF2(ethswctl->unit)) return -(EOPNOTSUPP);

            return ioctl_extsw_port_shaper_config(ethswctl);

        case ETHSWDOSCTRL:
            if (!IS_UNIT_SF2(ethswctl->unit)) return -(EOPNOTSUPP);

            ret = ioctl_extsw_dos_ctrl(ethswctl);
            if (!ret && (ethswctl->type == TYPE_GET))
                if (ethswctl_field_copy_to_user(dosCtrl))
                    return -EFAULT;
            return ret;

        case ETHSWSNOOPCTRL:
            {
                char *buf = NULL;                
                if (!IS_UNIT_SF2(ethswctl->unit)) return -(EOPNOTSUPP);

                if (ethswctl->type == TYPE_GET || ethswctl->type == TYPE_HELP)
                    if (!(buf = kzalloc(ethswctl->snoopCtrl.usz, GFP_KERNEL)))
                        return -EFAULT;
                if (buf) buf[0]='\0';
                ret = ioctl_extsw_snoop_ctrl(ethswctl, buf, ethswctl->snoopCtrl.usz);
                if (!ret && buf) {
                    BCM_IOC_PTR_ZERO_EXT(ethswctl->snoopCtrl.uptr);
                    if (copy_to_user(ethswctl->snoopCtrl.uptr, buf, strlen(buf)+1) || ethswctl_field_copy_to_user(snoopCtrl.val))
                        ret = -EFAULT;
                }
                if (buf) kfree(buf);
                return ret;
            }
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
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0))
                        dev_change_flags(port->dev, (port->dev->flags & ~IFF_UP));
                        dev_change_flags(port->dev, (port->dev->flags | IFF_UP));
#else
                        dev_change_flags(port->dev, (port->dev->flags & ~IFF_UP), NULL);
                        dev_change_flags(port->dev, (port->dev->flags | IFF_UP), NULL);
#endif
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
            if (!IS_UNIT_SF2(ethswctl->unit)) return -(EOPNOTSUPP);

            ret = ioctl_extsw_port_storm_ctrl(ethswctl);
            if (!ret && copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                return -EFAULT;
            return ret;

        case ETHSWMULTIPORT:
            if (!IS_UNIT_SF2(ethswctl->unit)) return BCM_E_NONE;
            if (ethswctl->type == TYPE_SET) {
                ioctl_extsw_set_multiport_address(ethswctl->unit, ethswctl->mac);
            }
            return BCM_E_NONE;
#endif // SF2_DEVICE
        case ETHSWTEST1:
            // actually no op, but required to return without error
            ethswctl->ret_val = 0;
            if (ethswctl_field_copy_to_user(ret_val))
                return -EFAULT;

            return 0;

#if ((defined(CONFIG_BCM963146) || defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813) || defined(CONFIG_BCM96855)) && (!defined(CONFIG_BRCM_QEMU)))
        case ETHSWDOSCTRL:            
            ret = _runner_rdpa_dos_ctrl(ethswctl);
            if (!ret && (ethswctl->type == TYPE_GET))
                if (ethswctl_field_copy_to_user(dosCtrl))
                    return -EFAULT;
            return ret;
#endif
            
        case ETHSWMACLOOPBACK:
            {
                enet_dbg("ethswctl ETHSWMACLOOPBACK : ioctl\n");

                if (!(port = get_ioctl_port(dev, ethswctl)))
                {
                    enet_err("failed to get port_obj from unit_port %d\n", ethswctl->port);
                    return -EFAULT;
                }

                if (!port->p.mac)
                {
                    enet_err("failed to get mac_dev\n");
                    return -EFAULT;
                }

                if (ethswctl->type == TYPE_SET)
                {
                    if (mac_dev_loopback_set(port->p.mac, ethswctl->val))
                    {
                        enet_err("%s loopback op not supported!!\n", port->p.mac->mac_drv->name);
                        return -EFAULT;
                    }
                    return 0;
                }
                else if (ethswctl->type == TYPE_GET)
                {
                    mac_loopback_t op;
                    if (mac_dev_loopback_get(port->p.mac, &op))
                    {
                        enet_err("%s loopback op not supported!!\n", port->p.mac->mac_drv->name);
                        return -EFAULT;
                    }
                    ethswctl->ret_val = op;
                    if (ethswctl_field_copy_to_user(ret_val))
                        return -EFAULT;
                    return 0;
                }
                else
                    return -EFAULT;
            }

#if defined(DSL_DEVICES)
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
            if (!(port = get_ioctl_port(dev, ethswctl)))
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
#else /* !defined(DSL_DEVICES) */
        case ETHSWPORTLOOPBACK:
            {
                mac_loopback_t loopback_op;
                int loopback_type = 0;
                int val = 0;

                enet_dbg("ethswctl ETHSWPORTLOOPBACK : ioctl\n");

                if (ethswctl->type == TYPE_SET)
                {
                    if (!(port = get_ioctl_port(dev, ethswctl)))
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
#if defined(SF2_DEVICE)
            if (!IS_UNIT_SF2(ethswctl->unit)) return -(EOPNOTSUPP);
            if (ethswctl->type == TYPE_GET)
            {
                ret = ioctl_extsw_port_irc_get(ethswctl);
                if (!ret && (ethswctl_field_copy_to_user(limit) || ethswctl_field_copy_to_user(burst_size)))
                    return -EFAULT;
            }
            else
                ret = ioctl_extsw_port_irc_set(ethswctl);
            return ret;
#else
            enet_err("Use tmctl command for rx rate control on non-sf2 port!!\n");
            return -(EOPNOTSUPP);
#endif // SF2_DEVICE
        case ETHSWPORTTXRATE:
#if defined(SF2_DEVICE)
            if (!IS_UNIT_SF2(ethswctl->unit)) return -(EOPNOTSUPP);

            BCM_IOC_PTR_ZERO_EXT(ethswctl->vptr);
            ret = ioctl_extsw_port_erc_config(ethswctl);
            if (!ret && !ethswctl->vptr && (ethswctl->type == TYPE_GET) &&
                    (ethswctl_field_copy_to_user(limit) || ethswctl_field_copy_to_user(burst_size)))
                return -EFAULT;
            return ret;
#else
            enet_err("Use tmctl command for tx rate control on non-sf2 port!!\n");
            return -(EOPNOTSUPP);
#endif // SF2_DEVICE
        case ETHSWEMACCLEAR:
            {
                if (!(port = get_ioctl_port(dev, ethswctl)))
                    return -EFAULT;

                if (!port->p.mac)
                    return -EFAULT;

                if (mac_dev_stats_clear(port->p.mac))
                    return -EFAULT;

                return 0;
            }
        case ETHSWEMACGET:
            {
                mac_stats_t *mac_stats;

                if (!(port = get_ioctl_port(dev, ethswctl)))
                    return -EFAULT;

                if (!port->p.mac)
                    return -EFAULT;
                mac_stats = &(port->p.mac->stats);

                if (mac_dev_stats_get(port->p.mac, mac_stats))
                    return -EFAULT;

                mac_dev_stats_to_emac_stats(&ethswctl->emac_stats_s, mac_stats);
                if (ethswctl_field_copy_to_user(emac_stats_s))
                    return -EFAULT;

                return 0;
            }

        case ETHSWPORTPAUSECAPABILITY:
            enet_dbg("ethswctl ETHSWPORTPAUSECAPABILITY ioctl: %s\n", ethswctl->type == TYPE_GET ? "get" : "set");
            port = get_ioctl_port(dev, ethswctl);
            if (!port)
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

                if (ethswctl->val >= PAUSE_FLOW_CTRL_OVER)
                    return -EFAULT;

                if (ethswctl->val < PAUSE_FLOW_REGULAR_OVER)
                {
                    if (port_pause_set(port, rx_enable, tx_enable))
                        return -EFAULT;
                }
                else
                {
                    if (!port_pfc_capable(port))
                        return -EFAULT;

                    if (port_pfc_set(port, pfc_rx_enable, pfc_tx_enable))
                        return -EFAULT;
                }

                return 0;
            }

            return -EFAULT;

        case ETHSWUNITPORT:
            {
                int u, p;
                struct net_device *dev;

                if (get_root_dev_by_name(ethswctl->ifname, &dev))
                    return -EFAULT;

                for (u = 0; u < COMPAT_MAX_SWITCHES; u++)
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
            if (!(port = get_ioctl_port(dev, ethswctl)))
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

                if (!(port = get_ioctl_port(dev, ethswctl)))
                    return -EFAULT;

                if (configure_bc_rate_limit_meter(port->priv, ethswctl->cpu_meter_rate_limit.rate_limit))
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

                if (!(port = get_ioctl_port(dev, ethswctl)))
                    return -EFAULT;
                
                if (!(port_obj = _port_rdpa_object_by_port(port)))
                    return -EFAULT;

                if (!(rc = rdpa_port_cfg_get(port_obj , &cfg)))
                {
                    cfg.sal_enable = cfg.dal_enable = ethswctl->sal_dal_en;
                    cfg.sal_miss_action = cfg.dal_miss_action = rdpa_forward_action_host;
                    if ((rc = rdpa_port_cfg_set(port_obj, &cfg)))
                    {
                        enet_err("failed to set rdpa sal/dal port configuration on %s\n", port->obj_name);
                    }
                }

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

            if (!(port = get_ioctl_port(dev, ethswctl)))
                return -EFAULT;

            if (!(port_obj = _port_rdpa_object_by_port(port)))
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

            if (!(port = get_ioctl_port(dev, ethswctl)))
                return -EFAULT;

            if (!(port_obj = _port_rdpa_object_by_port(port)))
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
#endif
    case ETHSWGETOBJNAME:
        if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
            return -EFAULT;

        strcpy(ethswctl->ifname, port->obj_name);
        if (ethswctl_field_copy_to_user(ifname))
            return -EFAULT;

        return 0;

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
        ethswctl->phycfg = 0;

        if (ethswctl_field_copy_to_user(phycfg))
            return -EFAULT;

        return 0;

    case ETHSWSWITCHING:
        /* coverity[user_pointer] */
        _enet_ioctl_hw_switching_set(ethswctl);
 
        if (ethswctl_field_copy_to_user(ret_val))
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
            if (!port->dev || port->n.port_netdev_role == PORT_NETDEV_ROLE_WAN || !IS_UNIT_SF2(ethswctl->unit))
#else
            if (!port->dev || port->n.port_netdev_role == PORT_NETDEV_ROLE_WAN || ethswctl->unit == 0)
#endif
                return -EOPNOTSUPP;
            // use role_set to setup switch WAN for software switching, and LAN for hw switching
            if (ethswctl->type == TYPE_ENABLE) {
                netdev_hw_switch_unset(port->dev);
                port->p.ops->role_set(port, PORT_NETDEV_ROLE_NONE);
                if (port->p.ops->fast_age)
                    port->p.ops->fast_age(port);
                if (port->p.ops->stp_set)
				    port->p.ops->stp_set(port, STP_MODE_DISABLE, STP_STATE_UNCHANGED);
            }
            else {
                netdev_hw_switch_set(port->dev);
                port->p.ops->role_set(port, PORT_NETDEV_ROLE_LAN);
                if (port->p.ops->stp_set)
                    port->p.ops->stp_set(port, STP_MODE_ENABLE, STP_STATE_UNCHANGED);
            }
            return 0;
        }
        return -EOPNOTSUPP;

    case ETHSWLINKSTATUS:
        // enet_dbg("ETHSWLINKSTATUS\n");

        if (!(port = get_ioctl_port(dev, ethswctl)) || !port->p.phy)
            return -EFAULT;

        if (ethswctl->type != TYPE_GET)
        {
#if defined(DSL_DEVICES)
            phy_dev_t *phy_dev = port->p.phy;
            phy_dev_read_status(phy_dev);
            extlh_link_change_handler(port, ethswctl->status, ethswctl->speed, ethswctl->duplex);
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

            port = get_ioctl_port(dev, ethswctl);
            if (!port)
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

            port = get_ioctl_port(dev, ethswctl);
            if (!port)
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

            port = get_ioctl_port(dev, ethswctl);
            if (!port)
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
        port = get_ioctl_port(dev, ethswctl);
        if (!port)
            return -EFAULT;

        if (!port->p.phy)
        {
            enet_err("port - %s has no PHY connected \n", port->name);
            return -EFAULT;
        }

        if (phy_is_crossbar(port->p.phy))
            if (ethswctl->addressing_flag & ETHSW_ADDRESSING_SUBPORT)
                phy_dev = crossbar_group_phy_get(port->p.phy, (ethswctl->sub_port>=CROSSBAR_PORT_BASE)?
                    PHY_PORT_TO_CROSSBAR_PORT(ethswctl->sub_port):ethswctl->sub_port);
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
            
        if (phy_is_crossbar(port->p.phy))
            if (ethswctl->addressing_flag & ETHSW_ADDRESSING_SUBPORT)
                phy_dev = crossbar_group_phy_get(port->p.phy, (ethswctl->sub_port>=CROSSBAR_PORT_BASE)?PHY_PORT_TO_CROSSBAR_PORT(ethswctl->sub_port):ethswctl->sub_port);
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

        ethswctl->speed_mode_caps = cascade_phy_get_common_inter_types(phy_dev);

        if (ethswctl->type == TYPE_SET)
        {
            phy_speed_t speed;
            phy_duplex_t duplex = ethswctl->duplex ? PHY_DUPLEX_FULL : PHY_DUPLEX_HALF;
            uint32_t curAdvCaps;

            /* Get PHY current advertisement capability */
            if (cascade_phy_dev_caps_get(phy_dev, CAPS_TYPE_ADVERTISE, &curAdvCaps))
                return -EFAULT;

            switch (ethswctl->speed)
            {
                case 10: case 100: case 1000: case 2500: case 5000: case 10000:
                    speed = phy_mbps_2_speed(ethswctl->speed); break;
                case 0:     speed = PHY_SPEED_AUTO; break;
                default:    speed = PHY_SPEED_UNKNOWN;
            }

            if (ethswctl->advPhyCaps == PHY_CAP_AUTONEG)    /* Set full support speed if Auto flag only */
                ethswctl->advPhyCaps |= supported_caps;
            else if (ethswctl->advPhyCaps == 0)         /* If not set, for the backward compatibility */
                ethswctl->advPhyCaps = phy_speed_to_cap(speed, duplex);

            /* Set non speed flags from current flag set */
            ethswctl->advPhyCaps = (ethswctl->advPhyCaps & PHY_CAP_PURE_SPEED_CAPS) | (curAdvCaps & PHY_CAP_NON_SPEED_CAPS);

            if ((ethswctl->advPhyCaps & supported_caps) == 0)
            {
                printk("Not Supported Speed %dmbps attempted\n", ethswctl->speed);
                return -EFAULT;
            }

#if defined(DSL_DEVICES)
            if (phy_is_pon_wan_ae_serdes(port->p.phy))
#endif
            {
                if (ethswctl->addressing_flag == 0)
                    return phy_dev_speed_set(phy_dev, speed, duplex) ? -EFAULT : 0;
                else
                    return phy_dev_caps_set(phy_dev, ethswctl->advPhyCaps) ? -EFAULT : 0;
            }
#if defined(DSL_DEVICES)
            else
            {
                if ((ret = cascade_phy_dev_speed_type_set(phy_dev, ethswctl->advPhyCaps, ethswctl->duplex,
                                ethswctl->config_speed_mode, ethswctl->config_an_enable)))
                    ethswctl->ret_val = RET_ERR_NOT_SUPPORTED;

                if (copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                    return -EFAULT;

                return ret? -EFAULT: 0;
            }
#endif
        }
        else if (ethswctl->type == TYPE_GET)
        {
            phy_speed_t speed;
            phy_duplex_t duplex;
            uint32_t caps = 0;

            if (phy_dev->phy_drv->config_speed_get)
            {
                if (phy_dev_config_speed_get(phy_dev, &speed, &duplex))
                    return -EFAULT;
            }

            if (phy_dev->phy_drv->caps_get)
            {
#if defined(DSL_DEVICES)
                if (!phy_is_pon_wan_ae_serdes(port->p.phy))
                {
                    if (cascade_phy_dev_caps_get(port->p.phy, CAPS_TYPE_ADVERTISE, &ethswctl->advPhyCaps))
                        return -EFAULT;

                    speed = phy_caps_to_auto_speed(ethswctl->advPhyCaps);
                    duplex = (ethswctl->advPhyCaps & (PHY_CAP_AUTONEG|PHY_CAP_10000|PHY_CAP_5000|PHY_CAP_2500|
                                PHY_CAP_1000_FULL|PHY_CAP_100_FULL|PHY_CAP_10_FULL))? PHY_DUPLEX_FULL : PHY_DUPLEX_HALF;
                }
                else
#endif
                {
                    if (phy_dev_caps_get(port->p.phy, CAPS_TYPE_ADVERTISE, &caps))
                        return -EFAULT;
                    ethswctl->advPhyCaps = caps;
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
            }
            else
                return -EFAULT;

            ethswctl->cfgSpeed = phy_speed_2_mbps(speed);

            if (duplex == PHY_DUPLEX_FULL)
                ethswctl->cfgDuplex = 1;
            else
                ethswctl->cfgDuplex = 0;


            ethswctl->phyCap = supported_caps;
            if (phy_dev->link)
            {
                ethswctl->duplex = (phy_dev->duplex == PHY_DUPLEX_FULL) ? 1 : 0;

                if (phy_dev->speed == PHY_SPEED_UNKNOWN)
                    return -EFAULT;
                else
                    ethswctl->speed = phy_speed_2_mbps(phy_dev->speed);
            }
            else
            {
                ethswctl->speed = 0;
                ethswctl->duplex = 0;
            }

            ethswctl->config_speed_mode = phy_dev_configured_current_inter_phy_type_get(phy_dev); 
            ethswctl->config_an_enable = phy_dev_configured_an_enabled_get(phy_dev);

            if (copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                return -EFAULT;

            return 0;
        }

        return -EFAULT;

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

#if defined(CONFIG_BCM_PHY_SPEED_LIMIT)
    case ETHSWPHYSPEED:
        if (ethswctl->type == TYPE_GET)
            ethswctl->val = pm_physpeed_get();
        else
            pm_physpeed_set(ethswctl->val);

        ethswctl->ret_val = 1;
        if (copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
            return -EFAULT;

        return 0;
#endif
    case ETHSWLANPWR:
        if (ethswctl->type == TYPE_GET)
            ethswctl->val = pm_lan_pwr_get();
        else
            pm_lan_pwr_set(ethswctl->val);

        ethswctl->ret_val = 1;
        if (copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
            return -EFAULT;

        return 0;

    case ETHSWXRDPDIV:
        if (ethswctl->type == TYPE_GET)
            ret = xrdp_div_get(&ethswctl->val);
        else
            ret = xrdp_div_set(ethswctl->val);

        if (ret)
            return -EFAULT;
          
        ethswctl->ret_val = 1;
        if (copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
            return -EFAULT;

        return 0;
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

            for (u = 0; u < COMPAT_MAX_SWITCHES; u++)
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

                if (phy_dev->speed == PHY_SPEED_UNKNOWN)
                    return -EFAULT;
                else
                    mib.ulIfSpeed = (uint64)1000000 * phy_speed_2_mbps(phy_dev->speed);
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
            return port_netdev_role_set(port, data ? PORT_NETDEV_ROLE_WAN : (port->p.port_cap == PORT_CAP_WAN_ONLY ? PORT_NETDEV_ROLE_NONE : PORT_NETDEV_ROLE_LAN)) ? -EFAULT : 0;
        }

    case SIOCIFREQ_EXT:
        {
            list_ctx ctx;
            port_cap_t port_cap;
            port_traverse_cb fn;
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
            case SIOCGPORTLANWAN:
                port_cap = PORT_CAP_LAN_WAN;
                break;
            case SIOCGPORTLANONLY:
                port_cap = PORT_CAP_LAN_ONLY;
                break;
            case SIOCGPORTLANALL:
                port_cap = PORT_CAP_MAX;
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
            ctx.count = 0;
                
            if (ifx->opcode == SIOCGPORTLANALL)
                fn = tr_ifreq_landevname_concat;
            else
                fn = tr_ifreq_devname_concat;

            if (!port_traverse_ports(root_sw, fn, PORT_CLASS_PORT, &ctx))
            {
                ifx->count = ctx.count;
                if (copy_to_user(rq->ifr_data , ifx, sizeof(ifreq_ext_t)))
                    return -EFAULT;

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

            map <<= CROSSBAR_PORT_BASE;

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


