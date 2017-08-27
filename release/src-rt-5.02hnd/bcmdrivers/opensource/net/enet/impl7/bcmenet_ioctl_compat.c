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
#include <bcmnet.h>
#include <boardparms.h>
#ifdef RUNNER
#include "runner.h"
#endif

extern int apd_enabled;
extern int eee_enabled;

static int pm_apd_set_single(enetx_port_t *p, void *ctx)
{
    phy_dev_t *phy_dev = p->p.phy;

    if (phy_dev)
        phy_dev_apd_set(phy_dev, apd_enabled);

    return 0;
}

static int pm_eee_set_single(enetx_port_t *p, void *ctx)
{
    mac_dev_t *mac_dev = p->p.mac;
    phy_dev_t *phy_dev = p->p.phy;

    if (mac_dev)
        mac_dev_eee_set(mac_dev, 0);

    if (phy_dev)
        phy_dev_eee_set(phy_dev, eee_enabled);

    return 0;
}

static int pm_apd_get(void)
{
    return apd_enabled;
}

static void pm_apd_set(int enabled)
{
    if (apd_enabled == enabled)
        return;

    apd_enabled = enabled;
    port_traverse_ports(root_sw, pm_apd_set_single, PORT_CLASS_PORT, NULL);
}

static int pm_eee_get(void)
{
    return eee_enabled;
}

static void pm_eee_set(int enabled)
{
    if (eee_enabled == enabled)
        return;

    eee_enabled = enabled;
    port_traverse_ports(root_sw, pm_eee_set_single, PORT_CLASS_PORT, NULL);
}

#define COMPAT_MAX_SWITCH_PORTS 24
enetx_port_t *unit_port_array[BP_MAX_ENET_MACS][COMPAT_MAX_SWITCH_PORTS] = {}; 
int unit_port_oam_idx_array[BP_MAX_ENET_MACS][COMPAT_MAX_SWITCH_PORTS] = {[0 ... BP_MAX_ENET_MACS-1][0 ... COMPAT_MAX_SWITCH_PORTS-1] = -1}; 
#define COMPAT_PORT(u, p) ((unit_port_array[u][p] && unit_port_array[u][p]->port_class == PORT_CLASS_PORT) ? \
    unit_port_array[u][p] : NULL)
#define COMPAT_OAM_IDX(u, p) (COMPAT_PORT(u, p) ? unit_port_oam_idx_array[u][p] : -1)

#ifdef RUNNER
#define COMPAT_RPDA(u, p) _port_rdpa_object_by_port(_compat_port_object_from_unit_port(u, p))
#endif

#ifdef GPON
static int _handle_gpon(struct ifreq *rq)
{
    struct gponif_data gpn;
    static enetx_port_t *gpon_port;
    port_info_t port_info =
    {
        .is_gpon = 1,
    };

    if (copy_from_user(&gpn, rq->ifr_data, sizeof(gpn)))
        return -EFAULT;

    switch (gpn.op)
    {
        case CREATEGPONVPORT:
            if (gpon_port)
                return -EALREADY;

            if (!(gpon_port = port_create(&port_info, root_sw)))
                return -EFAULT;

            gpon_port->n.rtnl_is_locked = 1;
            gpon_port->has_interface = 1;
            strcpy(gpon_port->name, "gpondef");
            /* TODO: p->phy = DUMMY_GPON_PHY for link status */
            if (sw_init(root_sw))
                return -EFAULT;
            
            gpon_port->dev->path.hw_subport_mcast_idx = NETDEV_PATH_HW_SUBPORTS_MAX;

            return 0;

        case DELETEALLGPONVPORTS:
        case DELETEGPONVPORT:
            if (!gpon_port)
                return -ENOENT;

            gpon_port->n.rtnl_is_locked = 1;
            sw_free(&gpon_port);
            return 0;

        case GETFREEGEMIDMAP:
        case SETGEMIDMAP:
        case GETGEMIDMAP:
            return 0;
        case SETMCASTGEMID:
            {
                int i;
            
                if (!gpon_port)
                    return -ENOENT;

                for (i = 0; i < CONFIG_BCM_MAX_GEM_PORTS; i++)
                {
                    if (!gpn.gem_map_arr[i])
                        continue;

                    netdev_path_set_hw_subport_mcast_idx(gpon_port->dev, i);
                    break;
                }

                return 0;
            }
    }

    return -EOPNOTSUPP;
}
#endif

#ifdef EPON
static int _handle_epon(int ethctl_op)
{
    static enetx_port_t *epon_port;
    port_info_t port_info =
    {
        .is_epon = 1,
    };

    switch (ethctl_op)
    {
        case ETHCREATEEPONVPORT:
            if (epon_port)
                return -EALREADY;

            if (!(epon_port = port_create(&port_info, root_sw)))
                return -EFAULT;

            epon_port->n.rtnl_is_locked = 1;
            epon_port->has_interface = 1;
            strcpy(epon_port->name, "epon0");
            /* TODO: p->phy = DUMMY_EPON_PHY for link status */
            if (sw_init(root_sw))
                return -EFAULT;

            return 0;
    }
    
    return -EOPNOTSUPP;
}
#endif
                        
#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848)
static int tr_init_detect(enetx_port_t *port, void *_ctx)
{
    struct ethctl_data *ethctl = (struct ethctl_data *)_ctx;
    int port_id = ethctl->val;

    if (port->p.port_id != port_id)
        return 0;

    port->n.rtnl_is_locked = 1;
    if (port_init_detect(port, PORT_TYPE_RUNNER_PORT))
        return -1;
    
    strncpy(ethctl->ifname, port->dev->name, strlen(port->dev->name));

    return 1;
}

static int _handle_init_wan(struct ifreq *rq, struct ethctl_data *ethctl)
{
    int rc;

    if ((rc = port_traverse_ports(root_sw, tr_init_detect, PORT_CLASS_PORT_DETECT, ethctl)) <= 0)
    {
        if (rc == 0)
            enet_err("did not find any auto detect ports with index %d\n", ethctl->val);

        return -EFAULT;
    }

    if (copy_to_user((void *)rq->ifr_data, ethctl, sizeof(*ethctl)))
        return -EFAULT;

    return 0;
}
#endif

static int tr_is_enet_dev(enetx_port_t *port, void *ctx)
{
    return port->dev == (struct net_device *)ctx;
}

static int is_enet_dev(struct net_device *dev)
{
    return port_traverse_ports(root_sw, tr_is_enet_dev, PORT_CLASS_PORT | PORT_CLASS_SW, (void *)dev) > 0;
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

/* returns if a port with a phy is in down state */
static int tr_is_phy_down(enetx_port_t *port, void *_ctx)
{
    if (port->p.phy)
        return port->p.phy->link ? 0 : 1;

    return 0;
}
    
static int tr_stats_clear(enetx_port_t *port, void *_ctx)
{
    port_stats_clear(port);
    return 0;
}
        
static void display_software_stats(enetx_port_t *port)
{
/* TODO:
    printk("\n");
    printk("TxPkts:       %10lu \n", pDevCtrl->stats.tx_packets);
    printk("TxOctets:     %10lu \n", pDevCtrl->stats.tx_bytes);
    printk("TxDropPkts:   %10lu \n", pDevCtrl->stats.tx_dropped);
    printk("\n");
    printk("RxPkts:       %10lu \n", pDevCtrl->stats.rx_packets);
    printk("RxOctets:     %10lu \n", pDevCtrl->stats.rx_bytes);
    printk("RxDropPkts:   %10lu \n", pDevCtrl->stats.rx_dropped);
    printk("\n");

    port->n.port_stats.rx_dropped_blog_drop++;
*/
}

static int _compat_validate_unit_port(int unit, int port)
{
    if (unit < 0 || port < 0 || unit > BP_MAX_ENET_MACS || port > COMPAT_MAX_SWITCH_PORTS)
    {
        enet_err("invalid unit %d, port %d values\n", unit, port);
        return -1;
    }

    return 0;
}

static int enet_ioctl_phy_cfg_get(int unit, int port, int *phycfg)
{
    const ETHERNET_MAC_INFO *emac_info;

    if ((emac_info = BpGetEthernetMacInfoArrayPtr()) == NULL)
    {
        enet_err("Error reading Ethernet MAC info from board params\n");
        return -1;
    }

    if (_compat_validate_unit_port(unit, port))
        return -1;

    *phycfg = emac_info[unit].sw.phy_id[port];

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
    void *in_out = (void *)port_id;

    if (port_traverse_ports(sw, tr_port_by_port_id, PORT_CLASS_PORT, &in_out) <= 0)
        return -1;

    *match = (enetx_port_t *)in_out;

    return 0;
}

int enet_ioctl_compat(struct net_device *dev, struct ifreq *rq, int cmd)
{
    enetx_port_t *port = NULL;
    union {
        struct ethswctl_data ethswctl_data;
        struct ethctl_data ethctl_data;
        ifreq_ext_t ifre;
    } rq_data;

    struct ethctl_data *ethctl = (struct ethctl_data *)&rq_data;
    struct ethswctl_data *ethswctl = (struct ethswctl_data *)&rq_data;
    ifreq_ext_t *ifx = (ifreq_ext_t *)&rq_data;

    switch (cmd)
    {
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
                if (copy_to_user(ethswctl->up_len.uptr, ctx.str, ethswctl->up_len.len))
                    return -EFAULT;
            }

            kfree(ctx.str);
            return 0;
        }

    case SIOCSCLEARMIBCNTR:
        /* TODO: Should this clear all ports stats or only 'dev' */
        port_traverse_ports(root_sw, tr_stats_clear, PORT_CLASS_PORT | PORT_CLASS_SW, NULL);
        return 0;
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
                else /* TODO: add 2500, 10000 speed */
                    return -EFAULT;
            }

            if (copy_to_user(rq->ifr_data, (void *)&mib, sizeof(mib)))
                return -EFAULT;

            return 0;
        }
    case SIOCGLINKSTATE:
        {
            int ret, data = 0; /* XXX 64 support ?? */

            port = ((enetx_netdev *)netdev_priv(dev))->port;
            ret = port_traverse_ports(port, tr_is_phy_down, PORT_CLASS_PORT, NULL);
            data = ret == 0;

            if (copy_to_user(rq->ifr_data, &data, sizeof(data)))
                return -EFAULT;

            return 0;
        }
        
#ifdef GPON
    case SIOCGPONIF:
        enet_dbg("SIOCGPONIF\n");
        return _handle_gpon(rq);
#endif
    case SIOCSWANPORT:
        {
            int data = (int)rq->ifr_data;

            port = ((enetx_netdev *)netdev_priv(dev))->port;
            return port_netdev_role_set(port, data ? PORT_NETDEV_ROLE_WAN : PORT_NETDEV_ROLE_LAN) ? -EFAULT : 0;
        }
    case SIOCETHCTLOPS:
        enet_dbg("SIOCETHCTLOPS\n");
        if (copy_from_user(ethctl, rq->ifr_data, sizeof(struct ethctl_data)))
            return -EFAULT;

        switch(ethctl->op)
        {
#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848)
        case ETHINITWAN:
            return _handle_init_wan(rq, ethctl);
#endif
#ifdef EPON
        case ETHCREATEEPONVPORT:
            return _handle_epon(ethctl->op);
#endif
        case ETHGETSOFTWARESTATS:
            port = ((enetx_netdev *)netdev_priv(dev))->port;
            display_software_stats(port);
            return 0;
#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96858)
        /* MII read/write functions for EGPHY compatible PHYs, will not work correctly on other PHY types */
        case ETHGETMIIREG: /* Read MII PHY register */
        case ETHSETMIIREG: /* Write MII PHY register */
            {
                uint16_t val = ethctl->val;
                bus_drv_t *bus_drv;
                
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
                }
                else
                {
                    enet_err("cannot resolve phy bus driver for phy_addr %d, flags %d\n", ethctl->phy_addr, ethctl->flags);
                    return -EFAULT;
                }

                if (ethswctl->op == ETHGETMIIREG)
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
#endif
        default:
            return -EOPNOTSUPP;
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

    case SIOCETHSWCTLOPS:
        if (copy_from_user(ethswctl, rq->ifr_data, sizeof(struct ethswctl_data)))
            return -EFAULT;

        switch(ethswctl->op)
        {
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
                mac_stats_t mac_stats;
                
                if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
                    return -EFAULT;

                if (!port->p.mac)
                    return -EFAULT;

                if (mac_dev_stats_get(port->p.mac, &mac_stats))
                    return -EFAULT;

                mac_dev_stats_to_emac_stats(&ethswctl->emac_stats_s, &mac_stats);
                if (copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                    return -EFAULT;

                return 0;
            }

        case ETHSWSTATPORTGET:
            return 0;
        case ETHSWPORTPAUSECAPABILITY:            
            enet_dbg("ethswctl ETHSWPORTPAUSECAPABILITY ioctl: %s\n", ethswctl->type == TYPE_GET ? "get" : "set");
            if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
                return -EFAULT;

            if (ethswctl->type == TYPE_GET)
            {
                int rx_enable, tx_enable;

                if (port_pause_get(port, &rx_enable, &tx_enable))
                    return -EFAULT;

                if (rx_enable)
                    ethswctl->ret_val = tx_enable ? PAUSE_FLOW_CTRL_BOTH : PAUSE_FLOW_CTRL_RX;
                else
                    ethswctl->ret_val = tx_enable ? PAUSE_FLOW_CTRL_TX : PAUSE_FLOW_CTRL_NONE;

                if (copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                    return -EFAULT;

                return 0;
            }
            else if (ethswctl->type == TYPE_SET)
            {
                int rx_enable = (ethswctl->val == PAUSE_FLOW_CTRL_RX) || (ethswctl->val == PAUSE_FLOW_CTRL_BOTH);
                int tx_enable = (ethswctl->val == PAUSE_FLOW_CTRL_TX) || (ethswctl->val == PAUSE_FLOW_CTRL_BOTH);

                if (port_pause_set(port, rx_enable, tx_enable))
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

                if (copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                    return -EFAULT;

                return 0;
            }

        case ETHSWSTATPORTCLR:
            if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
                return -EFAULT;

            port_stats_clear(port);
            return 0;

#ifdef RUNNER
        case ETHSWPORTSALDAL:
            {
                int rc;
                bdmf_object_handle port_obj;
                rdpa_port_dp_cfg_t cfg;

                if (ethswctl->type != TYPE_SET)
                    return -EFAULT;

                if (ethswctl->unit) /* unit == 0 for modifying lan ports, unit == 1 for rdpa_if_wan0 */
                    port_by_port_id(root_sw, rdpa_if_wan0, &port);
                else
                    port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port);

                if (!(port_obj = _port_rdpa_object_by_port(port)))
                    return -EFAULT;

                if ((rc = rdpa_port_cfg_get(port_obj , &cfg)))
                    return -EFAULT;

                cfg.sal_enable = cfg.dal_enable = ethswctl->sal_dal_en;
                if ((rc = rdpa_port_cfg_set(port_obj, &cfg)))
                {
                    enet_err("failed to set rdpa sal/dal port configuration on %s\n", port->obj_name);
                    return -EFAULT;
                }

                return 0;
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
                struct net_device *ethsw_dev = NULL;

                if (get_root_dev_by_name(ethswctl->ifname, &ethsw_dev))
                {
                    enet_err("illegal device name %s\n", ethswctl->ifname);
                    return -EFAULT;
                }

                if (!is_enet_dev(ethsw_dev))
                {
                    enet_err("is not an enet dev %s\n", ethsw_dev->name);
                    return -EFAULT;
                }

                port = ((enetx_netdev *)netdev_priv(ethsw_dev))->port;
                if (_port_rdpa_if_by_port(port, &index))
                {
                    enet_err("cannot retreive an rdpa index for %s\n", ethsw_dev->name);
                    return -EFAULT;
                }
                
                ethswctl->val = index;

                if (copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
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
            if (copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                return -EFAULT;

            return 0;

        case ETHSWPHYCFG: /* get boardparams phyid value */
            if (enet_ioctl_phy_cfg_get(ethswctl->val, ethswctl->port, &ethswctl->phycfg))
                return -EFAULT;

            if (copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                return -EFAULT;

            return 0;

        case ETHSWSWITCHING:
            /* XXX: Not implemented for runner */
            return 0;

        case ETHSWLINKSTATUS:
            enet_dbg("ETHSWLINKSTATUS\n");

            if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
                return -EFAULT;

            if (ethswctl->type != TYPE_GET)
                return -EFAULT;

            ethswctl->status = port->p.phy ? port->p.phy->link : 1;

            if (copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                return -EFAULT;

            return 0;

        case ETHSWPHYAUTONEG:
            {
                uint32_t caps;

                if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
                    return -EFAULT;

                if (!port->p.phy)
                {
                    enet_err("port - %s has no PHY conncted \n", port->name);
                    return -EFAULT;
                }

                if (ethswctl->type == TYPE_GET)
                {
                    if (phy_dev_caps_get(port->p.phy, &caps))
                        return -EFAULT;

                    ethswctl->autoneg_info = (caps & PHY_CAP_AUTONEG) ? 1 : 0;
                    ethswctl->autoneg_ad = 0;

                    if (caps & PHY_CAP_10_HALF)
                        ethswctl->autoneg_ad |= AN_10M_HALF;

                    if (caps & PHY_CAP_10_FULL)
                        ethswctl->autoneg_ad |= AN_10M_FULL;

                    if (caps & PHY_CAP_100_HALF)
                        ethswctl->autoneg_ad |= AN_100M_HALF;

                    if (caps & PHY_CAP_100_FULL)
                        ethswctl->autoneg_ad |= AN_100M_FULL;

                    if (caps & PHY_CAP_1000_HALF)
                        ethswctl->autoneg_ad |= AN_10M_HALF;

                    if (caps & PHY_CAP_1000_FULL)
                        ethswctl->autoneg_ad |= AN_1000M_FULL;

                    if (caps & PHY_CAP_PAUSE)
                        ethswctl->autoneg_ad |= AN_FLOW_CONTROL;
                    
                    /* Copied from impl5... */
                    ethswctl->autoneg_local = ethswctl->autoneg_ad;
                    ethswctl->ret_val = 0;

                    if (copy_to_user(rq->ifr_data , ethswctl, sizeof(struct ethswctl_data)))
                        return -EFAULT;

                    return 0;
                }
                else
                {
                    if (phy_dev_caps_get(port->p.phy, &caps))
                        return -EFAULT;

                    if (ethswctl->autoneg_info & AUTONEG_CTRL_MASK)
                        caps |= PHY_CAP_AUTONEG;
                    else
                        caps &= ~PHY_CAP_AUTONEG;

                    return phy_dev_caps_set(port->p.phy, caps) ? -EFAULT : 0;
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
                    enet_err("port - %s has no PHY conncted \n", port->name);
                    return -EFAULT;
                }

                if (phy_dev_caps_get(port->p.phy, &caps))
                    return -EFAULT;

                caps &= ~(PHY_CAP_10_HALF | PHY_CAP_10_FULL | PHY_CAP_100_HALF | PHY_CAP_100_FULL | PHY_CAP_1000_HALF |
                    PHY_CAP_1000_FULL | PHY_CAP_PAUSE);

                caps |= PHY_CAP_AUTONEG;

                if (ethswctl->autoneg_local & AN_FLOW_CONTROL)
                    caps |= PHY_CAP_PAUSE;

                if (ethswctl->autoneg_local & AN_10M_HALF)
                    caps |= PHY_CAP_10_HALF;

                if (ethswctl->autoneg_local & AN_10M_FULL)
                    caps |= PHY_CAP_10_FULL;

                if (ethswctl->autoneg_local & AN_100M_HALF)
                    caps |= PHY_CAP_100_HALF;

                if (ethswctl->autoneg_local & AN_100M_FULL)
                    caps |= PHY_CAP_100_FULL;

                if (ethswctl->autoneg_local & AN_1000M_HALF)
                    caps |= PHY_CAP_1000_HALF;

                if (ethswctl->autoneg_local & AN_1000M_FULL)
                    caps |= PHY_CAP_1000_FULL;

                return phy_dev_caps_set(port->p.phy, caps) ? -EFAULT : 0;
            }

        case ETHSWPHYMODE:
            {
                if (!(port = _compat_port_object_from_unit_port(ethswctl->unit, ethswctl->port)))
                    return -EFAULT;

                if (!port->p.phy)
                {
                    enet_err("port - %s has no PHY conncted \n", port->name);
                    return -EFAULT;
                }

                if (ethswctl->type == TYPE_SET)
                {
                    phy_speed_t speed;

                    if (ethswctl->speed == 10)
                        speed = PHY_SPEED_10;
                    else if (ethswctl->speed == 100)
                        speed = PHY_SPEED_100;
                    else if (ethswctl->speed == 1000)
                        speed = PHY_SPEED_1000;
                    else if (ethswctl->speed == 2500)
                        speed = PHY_SPEED_2500;
                    else if (ethswctl->speed == 10000)
                        speed = PHY_SPEED_10000;
                    else
                        speed = PHY_SPEED_UNKNOWN;

                    return phy_dev_speed_set(port->p.phy, speed,
                        ethswctl->duplex ? PHY_DUPLEX_FULL : PHY_DUPLEX_HALF) ? -EFAULT : 0;
                }
                else if (ethswctl->type == TYPE_GET)
                {
                    if (port->p.phy && port->p.phy->link)
                    {
                        phy_dev_t *phy_dev = port->p.phy;

                        ethswctl->duplex = (phy_dev->duplex == PHY_DUPLEX_FULL) ? 1 : 0;

                        if (phy_dev->speed == PHY_SPEED_10)
                            ethswctl->speed = 10;
                        else if (phy_dev->speed == PHY_SPEED_100)
                            ethswctl->speed = 100;
                        else if (phy_dev->speed == PHY_SPEED_1000)
                            ethswctl->speed = 1000;
                        else if (phy_dev->speed == PHY_SPEED_2500)
                            ethswctl->speed = 2500;
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

                for (u = 0; u < BP_MAX_ENET_MACS; u++)
                {
                    for (p = 0; p < COMPAT_MAX_SWITCH_PORTS; p++)
                    {
                        if (COMPAT_OAM_IDX(u, p) == ethswctl->oam_idx_str.oam_idx)
                            break;
                    }

                    if (p != BP_MAX_SWITCH_PORTS)
                        break;
                }
                
                if (p == BP_MAX_SWITCH_PORTS)
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
    }

    return -EOPNOTSUPP;
}

