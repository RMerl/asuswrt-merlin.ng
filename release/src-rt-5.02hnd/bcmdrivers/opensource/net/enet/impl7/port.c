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

#include <linux/slab.h>
#include "port.h"
#include "enet.h"
#ifdef RUNNER
#include "runner.h"
#endif
#ifdef BRCM_FTTDP
#include "g9991.h"
#endif
#include "enet_dbg.h"
#include "runner_wifi.h"

enetx_port_t *root_sw;
static int dbg_port_count, dbg_sw_count, dbg_port_detect_count;

/* TODO: port types should register dynamically / at compile time */
static int _assign_port_class(enetx_port_t *port, port_type_t port_type)
{
    sw_ops_t *sw_ops;
    port_ops_t *port_ops;
    port_class_t port_class;

    switch (port_type)
    {
#ifdef RUNNER
        case PORT_TYPE_RUNNER_SW:
            sw_ops = &port_runner_sw;
            port_class = PORT_CLASS_SW;
            break;
        case PORT_TYPE_RUNNER_PORT:
            port_ops = &port_runner_port;
            port_class = PORT_CLASS_PORT;
            break;
        case PORT_TYPE_RUNNER_DETECT:
            port_class = PORT_CLASS_PORT_DETECT;
            break;
#endif
#ifdef ENET_RUNNER_WIFI
        case PORT_TYPE_RUNNER_WIFI:
            port_ops = &port_runner_wifi;
            port_class = PORT_CLASS_PORT;
            break;
#endif
#ifdef EPON
        case PORT_TYPE_RUNNER_EPON:
            port_ops = &port_runner_epon;
            port_class = PORT_CLASS_PORT;
            break;
#endif
#ifdef GPON
        case PORT_TYPE_RUNNER_GPON:
            port_ops = &port_runner_gpon;
            port_class = PORT_CLASS_PORT;
            break;
#endif
#ifdef BRCM_FTTDP
        case PORT_TYPE_G9991_SW:
            sw_ops = &port_g9991_sw;
            port_class = PORT_CLASS_SW;
            break;
        case PORT_TYPE_G9991_PORT:
            port_ops = &port_g9991_port;
            port_class = PORT_CLASS_PORT;
            break;
#endif
#ifdef VLANTAG
        case PORT_TYPE_VLAN_SW:
            sw_ops = &port_vlan_sw;
            port_class = PORT_CLASS_SW;
            break;
        case PORT_TYPE_VLAN_PORT:
            port_ops = &port_vlan_port;
            port_class = PORT_CLASS_PORT;
            break;
#endif
#ifdef ENET_DMA
        case PORT_TYPE_DIRECT_RGMII:
            sw_ops = &port_dummy_sw;
            port_class = PORT_CLASS_SW;
            break;
        case PORT_TYPE_GENERIC_DMA:
            port_ops = &port_dma_port;
            port_class = PORT_CLASS_PORT;
            break;
#endif
        default:
            enet_err("failed to create port type %d\n", port_type);
            return -1;
    }

    port->port_class = port_class;
    port->port_type = port_type;

    /* object name for easy debugging */
    if (port->port_class == PORT_CLASS_SW)
    {
        port->s.ops = sw_ops;
        snprintf(port->obj_name, IFNAMSIZ, "sw%d", dbg_sw_count++);
    }
    else if (port->port_class == PORT_CLASS_PORT)
    {
        port->p.ops = port_ops;
        snprintf(port->obj_name, IFNAMSIZ, "port%d", dbg_port_count++);
    }
    else if (port->port_class == PORT_CLASS_PORT_DETECT)
    {
        snprintf(port->obj_name, IFNAMSIZ, "portdetect%d", dbg_port_detect_count++);
    }

    enet_dbg("set %s type %d\n", port->obj_name, port_type);
            
    return 0;
}

static enetx_port_t *_allocate_port(port_type_t port_type)
{
    enetx_port_t *port;

    if (!(port = kzalloc(sizeof(enetx_port_t), GFP_KERNEL)))
    {
        enet_err("failed to allocate port object\n");
        return NULL;
    }

    if (_assign_port_class(port, port_type))
    {
        kfree(port);
        return NULL;
    }

    return port;
}

static int _mux_demux_on_sw(char *sw_name, enetx_port_t ***map, int *count, int index, enetx_port_t *port)
{
    enetx_port_t **ports;
    int old_count = *count;

    if (!port)
    {
        if (index > old_count)
            return -1;
            
        (*map)[index] = NULL;

        return 0;
    }
    
    if (index >= old_count)
    {
        ports = krealloc(*map, sizeof(void *)*(index + 1), GFP_KERNEL);
        if (!ports)
        {
            enet_err("cannot de/mux %s: %s:%d not enough memory\n", port->obj_name, sw_name, index);
            return -ENOMEM;
        }

        memset(&(ports[old_count]), 0, sizeof(void *)*(index - old_count));
        *count = index + 1;
        *map = ports;
    }
    else if ((*map)[index])
    {
        enet_err("cannot de/mux %s: %s:%d already taken by %s\n", port->obj_name, sw_name, index, ((*map)[index])->obj_name);
        return -EBUSY;
    }

    (*map)[index] = port;
    
    return 0;
}

/* demux port on switch */
int demux_on_sw(enetx_port_t *sw, int index, enetx_port_t *port)
{
    int rc;

    if (!(rc = _mux_demux_on_sw(sw->obj_name, &sw->s.demux_map, &sw->s.demux_count, index, port)))
    {
        if (!port)
        {
            enet_dbg("demux clear: %s:%d\n", sw->obj_name, index);
            return 0;
        }

        /* Cache demux OP in port object */
        port->p.port_demux = sw->s.ops->port_demux;
        enet_dbg("demux %s: %s:%d\n", port->obj_name, sw->obj_name, index);
    }

    return rc;
}

/* mux port on switch */
/* Usually for non-root_sw, should call mux_on_sw with port->p.port_id for muxing */
int mux_on_sw(enetx_port_t *from, enetx_port_t *sw, int index, enetx_port_t *to)
{
    int rc;

    if (!(rc = _mux_demux_on_sw(sw->obj_name, &sw->s.mux_map, &sw->s.mux_count, index, to)))
    {
        if (!to)
        {
            enet_dbg("mux clear %s -> NULL: %s:%d\n", from->obj_name, sw->obj_name, index);
            return 0;
        }

        /* Cache mux OP in port object */
        from->p.port_mux = sw->s.ops->port_mux;
        enet_dbg("mux %s -> %s: %s:%d\n", from->obj_name, to->obj_name, sw->obj_name, index);
    }

    return rc;
}

/* Enslave switch to port */
static int enslave_sw(enetx_port_t *port, enetx_port_t *child_sw)
{
    if (port->p.child_sw)
    {
        enet_err("switch %s already ensalved to %s\n", port->p.child_sw->obj_name, port->obj_name);
        return -1;
    }

    port->p.child_sw = child_sw;

    return 0;
}

/* Enslave port to switch */
static int enslave_port(enetx_port_t *sw, enetx_port_t *child_port)
{
    enetx_port_t **ports;

    ports = krealloc(sw->s.ports, sizeof(enetx_port_t*) * (sw->s.port_count + 1), GFP_KERNEL);
    if (!ports)
        return -ENOMEM;

    sw->s.ports = ports;
    sw->s.ports[sw->s.port_count] = child_port;
    sw->s.port_count++;

    return 0;
}

enetx_port_t *sw_create(port_type_t type, enetx_port_t *parent_port)
{
    enetx_port_t *sw = _allocate_port(type);
    int rc;

    if (!sw)
        return NULL;

    if (parent_port)
    {
        sw->s.parent_port = parent_port;
        rc = enslave_sw(parent_port, sw);
        if (rc)
        {
            kfree(sw);
            return NULL;
        }
    }
    
    enet_dbg("created %s(name: %s)/parent:%s\n", sw->obj_name, sw->name, parent_port ? parent_port->obj_name: "none");

    return sw;
}

static enetx_port_t *_port_create(port_type_t type, int port_id, enetx_port_t *parent_sw)
{
    int rc;
    enetx_port_t *port = _allocate_port(type);

    if (!port)
        return NULL;

    port->p.port_id = port_id;
    port->p.parent_sw = parent_sw;

    rc = enslave_port(parent_sw, port);
    if (rc)
    {
        kfree(port);
        return NULL;
    }

    return port;
}

enetx_port_t *port_create(port_info_t *port_info, enetx_port_t *parent_sw)
{
    int port_id;
    port_type_t port_type;

    if (parent_sw->s.ops->port_id_on_sw(port_info, &port_id, &port_type))
    {
        enet_err("cannot resolve port_info (port %d) on %s\n", port_info->port, parent_sw->obj_name);
        return NULL;
    }

    return _port_create(port_type, port_id, parent_sw);
}

void sw_free(enetx_port_t **_p)
{
    enetx_port_t *p = *_p;
    int i;

    if (!p)
    {
        enet_err("free: missing object?\n");
        return;
    }
    
    if (p->dev)
        enet_remove_netdevice(p);
    
    if (p->port_class == PORT_CLASS_SW)
    {
        kfree(p->s.demux_map);
        p->s.demux_map = NULL;
        kfree(p->s.mux_map);
        p->s.mux_map = NULL;

        for (i = 0; i < p->s.port_count; i++)
        {
            if (!p->s.ports[i])
                continue;

            sw_free(&p->s.ports[i]);
        }
        
        p->s.port_count = 0;
        kfree(p->s.ports);
        p->s.ports = NULL;

        if (p->is_init)
            p->s.ops->uninit(p);
    }
    else if (p->port_class == PORT_CLASS_PORT)
    {
        enetx_port_t *parent_sw = p->p.parent_sw;

        if (p->p.mac)
        {
            mac_dev_disable(p->p.mac);
            mac_dev_del(p->p.mac);
            p->p.mac = NULL;
            enet_dbg("freed MAC object\n");
        }
    
        if (p->p.phy)
        {
            phy_dev_link_change_unregister(p->p.phy);
            phy_dev_power_set(p->p.phy, 0);
            phy_dev_del(p->p.phy);
            p->p.phy = NULL;
            enet_dbg("freed PHY object\n");
        }

        if (p->p.child_sw)
            sw_free(&p->p.child_sw);
    
        if (p->is_init)
            p->p.ops->uninit(p);

        for (i = 0; i < parent_sw->s.port_count; i++)
        {
            if (parent_sw->s.ports[i] == p)
                parent_sw->s.ports[i] = NULL;
        }
    }
    else if (p->port_class == PORT_CLASS_PORT_DETECT)
    {
        if (p->p.mac)
        {
            mac_dev_del(p->p.mac);
            p->p.mac = NULL;
            enet_dbg("freed MAC object\n");
        }
    
        if (p->p.phy)
        {
            phy_dev_del(p->p.phy);
            p->p.phy = NULL;
            enet_dbg("freed PHY object\n");
        }
    }
    
    kfree(p);
    *_p = NULL;
    enet_dbg("freed object %s\n", p->obj_name);
}

void phy_link_change_cb(void *ctx);

int port_init(enetx_port_t *port)
{
    enetx_port_t *sw = port->p.child_sw;
    int rc;

    if (!port)
    {
        enet_err("no port object to initialize\n");
        return -1;
    }

    if (port->is_init)
    {
        enet_err("port object already initialized: %s\n", port->obj_name);
        return EALREADY;
    }
    
#ifdef CONFIG_BLOG
    port->n.blog_phy = BLOG_MAXPHY;
#endif

    if ((rc = port->p.ops->init(port)))
        goto exit;

    port->is_init = 1;

    if (port->p.mac)
    {
        if ((rc = mac_dev_init(port->p.mac)) ||
            (rc = mac_dev_eee_set(port->p.mac, 0)))
        {
            enet_err("failed to initialize mac for port: %s\n", port->obj_name);
            goto exit;
        }
    }

    if (port->p.phy)
    {
        if ((rc = phy_dev_init(port->p.phy)) ||
            (rc = phy_dev_power_set(port->p.phy, 0)))
        {
            enet_err("failed to initialize phy for port: %s\n", port->obj_name);
            goto exit;
        }

        phy_dev_link_change_register(port->p.phy, phy_link_change_cb, port);
    }

    if (port->has_interface)
    {
        if ((rc = enet_create_netdevice(port)))
            goto exit;
    }

    if (sw)
        rc = sw_init(sw);

exit:
    if (rc)
    {
        enet_err("failed to initialize object (rc=%d)\n", rc);
        sw_free(&port);
        return rc;
    }
    
    enet_dbg("initialized object %s\n", port->obj_name);

    return 0;
}

int sw_init(enetx_port_t *sw)
{
    int i, rc = 0;
    char *was_init;

    if (!sw)
    {
        enet_err("no switch object to initialize\n");
        return -1;
    }

    /* Used for error handling, since when port is added, it's switch init will be called to init the port (skips
     * already initialized ports) */
    was_init = sw->is_init ? "re" : "";
    if (sw->is_init)
        goto init_ports;
    
    if ((rc = sw->s.ops->init(sw)))
        goto exit;
    
    sw->is_init = 1;
    
    if (sw->has_interface)
    {
        if ((rc = enet_create_netdevice(sw)))
            goto exit;
    }

init_ports:
    for (i = 0; i < sw->s.port_count; i++)
    {
        if (!sw->s.ports[i])
            continue;

        if (sw->s.ports[i]->port_class == PORT_CLASS_PORT_DETECT)
            continue;

        if ((rc = port_init(sw->s.ports[i])) && rc != EALREADY)
            goto exit;
    }

exit:
    if (rc && rc != EALREADY)
    {
        enet_err("failed to %sinitialize %s\n", was_init, sw->is_init ? sw->obj_name : "switch object");
        if (0 == strlen(was_init)) /* clean up only on first initialization call */
            sw_free(&sw);
    }
    else
        enet_dbg("%sinitialized %s\n", was_init, sw->obj_name);

    return rc;
}

int _port_traverse_ports(enetx_port_t *port, port_traverse_cb fn, port_class_t classes, void *ctx, int max_sw_depth)
{
    int i, rc;

    if (!port)
        return -1;

    if (port->port_class == PORT_CLASS_SW)
    {
        if (max_sw_depth <= 0)
        {
            if (classes & PORT_CLASS_SW && (rc = fn(port, ctx)))
                return rc;
        }
        
        if (max_sw_depth == 0)
            return 0;

        for (i = 0; i < port->s.port_count; i++)
        {
            if (!port->s.ports[i])
                continue;

            if ((rc = _port_traverse_ports(port->s.ports[i], fn, classes, ctx, max_sw_depth)))
                return rc;
        }
    }
    else if (port->port_class == PORT_CLASS_PORT)
    {
        if (classes & PORT_CLASS_PORT && (rc = fn(port, ctx)))
            return rc;

        if (port->p.child_sw)
        {
            if ((rc = _port_traverse_ports(port->p.child_sw, fn, classes, ctx, max_sw_depth--)))
                return rc;
        }
    }
    else if (port->port_class == PORT_CLASS_PORT_DETECT)
    {
        if (classes & PORT_CLASS_PORT_DETECT)
            return fn(port, ctx);
    }
    else
        return -1;

    return 0;
}

int port_generic_sw_demux(enetx_port_t *sw, enetx_rx_info_t *rx_info, FkBuff_t *fkb, enetx_port_t **out_port)
{
    *out_port = sw->s.demux_map[rx_info->src_port];
    enet_dbg_rx("demux rx_port %d tx_port %d sw: %s port: %p\n", rx_info->src_port, *out_port ? (*out_port)->p.port_id : -1, sw->obj_name, sw->s.demux_map[rx_info->src_port]);
    
    if (unlikely(!*out_port))
        return -1;

#ifdef NEXT_LEVEL_DEMUX_REQUIRED
    sw = (*out_port)->p.child_sw;
    if (sw && sw->s.ops->port_demux)
        return sw->s.ops->port_demux(sw, (*out_port)->p.port_id, fkb, out_port);
#endif

    return 0;
}

int port_generic_sw_mux(enetx_port_t *tx_port, pNBuff_t pNBuff, enetx_port_t **out_port)
{
    enetx_port_t *sw = tx_port->p.parent_sw;
    *out_port = sw->s.mux_map[tx_port->p.port_id];

    enet_dbg_tx("muxed %s on %s:%d to %s\n", tx_port->obj_name, sw->obj_name, tx_port->p.port_id, (*out_port) ? (*out_port)->obj_name : "error");
    
    if (unlikely(!(*out_port)))
        return -1;

    return 0;
}

void port_generic_open(enetx_port_t *self)
{
    if (self->p.phy)
        phy_dev_power_set(self->p.phy, 1);

    if (self->p.mac)
        mac_dev_enable(self->p.mac);
}

void port_generic_stop(enetx_port_t *self)
{
    if (self->p.phy)
        phy_dev_power_set(self->p.phy, 0);

    if (self->p.mac)
        mac_dev_disable(self->p.mac);
}

int port_netdev_role_set(enetx_port_t *self, port_netdev_role_t role)
{
    if (self->port_class != PORT_CLASS_PORT)
        return -EFAULT;
    
    if (!self->dev)
    {
        enet_err("cannot modify role of port without network device attached\n");
        return -EFAULT;
    }

    if (self->n.port_netdev_role == role)
    {
        enet_dbg("%s already set as role %d\n", self->obj_name, role);
        return 0;
    }

    if (role == PORT_NETDEV_ROLE_WAN)
    {
        if (self->p.port_cap == PORT_CAP_LAN_ONLY)
        {
            enet_err("%s: cannot change role to WAN, port has LAN_ONLY capability\n", self->obj_name);
            return -EFAULT;
        }
    }
    else if (role == PORT_NETDEV_ROLE_LAN)
    {
        if (self->p.port_cap == PORT_CAP_WAN_ONLY)
        {
            enet_err("%s: cannot change role to LAN, port has WAN_ONLY capability\n", self->obj_name);
            return -EFAULT;
        }
    }
    else
    {
        return -EFAULT;
    }

    if (self->p.ops->role_set && self->p.ops->role_set(self, role))
        return -EFAULT;

    self->n.port_netdev_role = role;
    
    enet_dev_flags_update(self);
    enet_dev_mac_set(self, 0); /* Release old address */
    enet_dev_mac_set(self, 1); /* Get new one based on role */

    return 0;
}

int port_generic_mtu_set(enetx_port_t *self, int mtu)
{
    if (self->p.mac)
        return mac_dev_mtu_set(self->p.mac, mtu);

    return 0;
}
    
int port_init_detect(enetx_port_t *port, port_type_t type)
{
    int rc;
    enet_dbg("initializing detect port %s..\n", port->obj_name);
    if (_assign_port_class(port, type))
        return -1;

    rc = sw_init(root_sw);
    if (rc && rc != EALREADY)
    {
        sw_free(&port);
        return -1;
    }

    return 0;
}

static int tr_port_stats_get(enetx_port_t *port, void *_ctx)
{
    struct rtnl_link_stats64 *net_stats = (struct rtnl_link_stats64 *)_ctx;

    port_stats_get(port, net_stats);

    return 0;
}

void port_generic_stats_get(enetx_port_t *self, struct rtnl_link_stats64 *net_stats)
{
    int rc;
    mac_stats_t mac_stats = { };
    
    if (self->port_class == PORT_CLASS_SW)
    {
        _port_traverse_ports(self, tr_port_stats_get, PORT_CLASS_SW | PORT_CLASS_PORT, net_stats, 1);
        return;
    }

    if (self->port_class != PORT_CLASS_PORT)
        return;

    if (!self->p.mac)
        return;

    if ((rc = mac_dev_stats_get(self->p.mac, &mac_stats)))
    {
        enet_err("error getting mac stats for %s (rc=%d)\n", self->obj_name, rc);
        return;
    }

    net_stats->collisions += mac_stats.tx_total_collision;
    net_stats->multicast += mac_stats.rx_multicast_packet;
    net_stats->rx_bytes += mac_stats.rx_byte;
    net_stats->rx_packets += mac_stats.rx_packet;
    net_stats->rx_crc_errors += mac_stats.rx_fcs_error;
    net_stats->rx_errors += mac_stats.rx_alignment_error +
        mac_stats.rx_code_error +
        mac_stats.rx_frame_length_error +
        mac_stats.rx_fifo_errors;
    net_stats->rx_length_errors += mac_stats.rx_frame_length_error;

    net_stats->tx_bytes += mac_stats.tx_byte;
    net_stats->tx_packets += mac_stats.tx_packet;
    net_stats->tx_errors += mac_stats.tx_error +
        mac_stats.tx_single_collision +
        mac_stats.tx_multiple_collision +
        mac_stats.tx_late_collision +
        mac_stats.tx_excessive_collision +
        mac_stats.tx_fcs_error +
        mac_stats.tx_oversize_frame +
        mac_stats.tx_fifo_errors;
    
    net_stats->rx_frame_errors = mac_stats.rx_alignment_error;
    net_stats->rx_fifo_errors = mac_stats.rx_fifo_errors;
    net_stats->tx_fifo_errors = mac_stats.tx_fifo_errors;

#if defined(CONFIG_BCM_KF_EXTSTATS)
    net_stats->tx_multicast_packets += mac_stats.tx_multicast_packet;
    net_stats->rx_broadcast_packets += mac_stats.rx_broadcast_packet;
    net_stats->tx_broadcast_packets += mac_stats.tx_broadcast_packet;
#endif
}

