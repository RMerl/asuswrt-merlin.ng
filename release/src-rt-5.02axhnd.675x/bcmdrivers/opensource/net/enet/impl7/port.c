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
#include "phy_drv.h"
#include "enet.h"
#include "enet_dbg.h"
#if defined(CONFIG_BCM_PON_RDP) || defined(CONFIG_BCM_PON_XRDP)
#include <rdpa_types.h>
#endif

/* port_types function; should only be used by port.c */
extern int _assign_port_class(enetx_port_t *port, port_type_t port_type);

enetx_port_t *root_sw;

uint32_t g_enet_flags;

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
    
    child_port->p.parent_sw = sw;

    enet_dbg("enslave %s -> %s\n", child_port->obj_name, sw->obj_name);
    return 0;
}

enetx_port_t *sw_create(port_type_t type, enetx_port_t *parent_port)
{
    enetx_port_t *sw = _allocate_port(type);
    int rc;

    if (!sw)
        return NULL;

    sema_init(&sw->s.conf_sem, 1);
    
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

static int _assert_class_port_defaults(enetx_port_t *port)
{
    if (port->port_class & (PORT_CLASS_PORT|PORT_CLASS_PORT_DETECT))
    {
        if (port->p.port_cap < 0)
            goto error;
    }
    
    if (port->n.port_netdev_role < 0)
        goto error;

    return 0;

error:
    enet_err("Asserted by missing port configuration on %s\n", port->obj_name);
    return -1;
}

static void _set_class_port_defaults(enetx_port_t *port, int port_id)
{
    port->p.port_id = port_id;

    if (port->port_class & (PORT_CLASS_PORT|PORT_CLASS_PORT_DETECT))
        port->p.handle_phy_link_change = 1;

    port->p.port_cap = -1;
    port->n.port_netdev_role = -1;

#ifdef CONFIG_BLOG
    port->n.blog_phy = BLOG_MAXPHY;
#endif
}

static enetx_port_t *_port_create(port_type_t type, int port_id, enetx_port_t *parent_sw)
{
    enetx_port_t *port = _allocate_port(type);

    if (!port)
        return NULL;

    _set_class_port_defaults(port, port_id);
    if (enslave_port(parent_sw, port))
    {
        kfree(port);
        return NULL;
    }

    return port;
}

static int port_type_resolve(enetx_port_t *port);

int port_create(port_info_t *port_info, enetx_port_t *parent_sw, enetx_port_t **_port)
{
    int ret;
    enetx_port_t *port;
    
    port = _port_create(PORT_TYPE_DETECT, 0, parent_sw);
    if (!port)
        return -1;

    port->port_info = *port_info;
    port->port_info.is_detect = 0;

    if (!port_info->is_detect)
    {
        ret = port_type_resolve(port);
        if (ret > 0)
        {
            sw_free(&port);
            return 0; /* Skip creating port */
        }

        if (ret)
        {
            enet_err("cannot resolve port_info (port %d) on %s\n", port_info->port, parent_sw->obj_name);
            return ret;
        }
    }

#if !defined(BCM_PON)
    /* Overwrite port obj_name with unit */
    if (port->port_class & PORT_CLASS_PORT)
        snprintf(port->obj_name, IFNAMSIZ, "port_%d:%d", PORT_ON_ROOT_SW(port) ? 0 : 1, port_info->port);
#endif

    *_port = port;

    return 0;
}

void _sw_free(enetx_port_t **_p, int free_port)
{
    enetx_port_t *p = *_p;
    int i;

    if (!p)
    {
        enet_err("free: missing object?\n");
        return;
    }

    if (p->port_class & (PORT_CLASS_PORT | PORT_CLASS_PORT_DETECT))
    {
        enetx_port_t *parent_sw = p->p.parent_sw;

        for (i = 0; i < parent_sw->s.port_count; i++)
        {
            if (parent_sw->s.ports[i] == p)
                parent_sw->s.ports[i] = NULL;
        }
    }

    if (p->port_class == PORT_CLASS_SW)
    {
        if (p->s.ops->mux_free)
            p->s.ops->mux_free(p);

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
    else if (p->port_class & (PORT_CLASS_PORT | PORT_CLASS_PORT_DETECT))
    {
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

        if (p->port_class == PORT_CLASS_PORT)
        {
            if (p->p.child_sw)
                sw_free(&p->p.child_sw);

            if (p->is_init && p->p.ops->uninit)
                p->p.ops->uninit(p);
        }
    }

    /* Removing NetDevice must be last */
    if (p->dev)
        enet_remove_netdevice(p);

    synchronize_rcu();

    if (free_port)
    {
        enet_dbg("freed object %s\n", p->obj_name);
        kfree(p);
        *_p = NULL;
    }
}

void sw_free(enetx_port_t **_p)
{
    _sw_free(_p, 1);
}

void phy_link_change_cb(void *ctx);

int port_init(enetx_port_t *port)
{
    int rc = -1;

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
    else
    {
        enet_dbgv("init %s\n", port->obj_name);
    }
    
    /* PORT_DETECT cannot be init from sw_init; they should be init by port_activate() */
    if (port->port_class == PORT_CLASS_PORT_DETECT)
    {
        phy_dev_power_set(port->p.phy, 0);
        return 0;
    }

    if (port->p.ops->init && (rc = port->p.ops->init(port)))
        goto exit;

    /* Port configuration must be set pre-port_init() or at p->init() callback */
    if (_assert_class_port_defaults(port))
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

        if ((rc = mac_dev_mtu_set(port->p.mac, ENET_MAX_MTU_PAYLOAD_SIZE + ENET_MAX_MTU_EXTRA_SIZE)))
        {
            enet_err("failed to set init MTU size for port: %s\n", port->obj_name);
            goto exit;
        }
        
        if (port->p.port_cap == PORT_CAP_MGMT && !port->has_interface)
            mac_dev_enable(port->p.mac);
    }

    if (port->p.phy)
    {
        if ((rc = phy_dev_init(port->p.phy)) ||
            (rc = phy_dev_power_set(port->p.phy, 0)))
        {
            phy_dev_del(port->p.phy);
            port->p.phy = NULL;

            enet_err("failed to initialize phy for port: %s. ignoring...\n", port->obj_name);
#if !defined(HALT_ON_ANY_PORT_FAILURE)
            rc = 0;
            sw_free(&port);
#endif
            goto exit;
        }

        if (port->p.handle_phy_link_change)
        {
#if defined(DSL_DEVICES)
            if (port->p.phy->phy_drv->phy_type != PHY_TYPE_PON)
                phy_register_polling_timer(port->p.phy, dslbase_phy_link_change_cb);
            else
#endif
                phy_dev_link_change_register(port->p.phy, phy_link_change_cb, port);
        }
    }

    if (port->has_interface)
    {
        if ((rc = enet_create_netdevice(port)))
            goto exit;
    }
    else
    {
        port_link_change(port, !port->p.phy);
    }

exit:
    if (rc)
    {
        enet_err("failed to initialize object (rc=%d)\n", rc);
        BUG();  /* stop initialization when error happens without continue running */
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
    enetx_port_t *port;

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
        port = sw->s.ports[i];
        if (!port)
            continue;

        if ((rc = port_init(port)) && rc != EALREADY)
            goto exit;
    }

    /* init switches connected to ports after all ports on current switch is inited */
    for (i = 0; i < sw->s.port_count; i++)
    {
        port = sw->s.ports[i];
        if (!port || !port->is_init || !port->p.child_sw)
            continue;

        rc = sw_init(port->p.child_sw);
        if (rc && rc != EALREADY)
            break;
    }

exit:
    if (rc && rc != EALREADY)
    {
        enet_err("failed to %sinitialize %s\n", was_init, sw->is_init ? sw->obj_name : "switch object");
        BUG();  /* stop initialization when error happens without continue running */
        if (0 == strlen(was_init)) /* clean up only on first initialization call */
            sw_free(&sw);
    }
    else
    {
        rc = 0;
        enet_dbg("%sinitialized %s\n", was_init, sw->obj_name);
    }

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
            if ((rc = _port_traverse_ports(port->p.child_sw, fn, classes, ctx, max_sw_depth-1)))
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

void port_generic_open(enetx_port_t *self)
{
    if ((self->p.phy) && !(g_enet_flags & ENET_FLAG_IF_PHY_PWR_SYNC_DISABLE))
    {
        phy_dev_power_set(self->p.phy, 1);
    }
    else
    {
        /* phy link change cb handles these when phy exists */
        port_link_change(self, 1);

        if (self->p.mac)
            mac_dev_enable(self->p.mac);
    }
}

void port_generic_stop(enetx_port_t *self)
{
    if ((self->p.phy) && !(g_enet_flags & ENET_FLAG_IF_PHY_PWR_SYNC_DISABLE))
    {
#if 0
        if (self->dev && strcmp(self->dev->name, "eth0"))
            phy_dev_power_set(self->p.phy, 0);
        else
#endif
            printk("%s %d skip turnning off power on %s here\n", __FUNCTION__, __LINE__, self->dev ? self->dev->name : NULL);
    }
    else
    {
        /* phy link change cb handles these when phy exists */
        if (self->p.mac)
            mac_dev_disable(self->p.mac);

        port_link_change(self, 0);
    }
}

int _port_role_set(enetx_port_t *self, port_netdev_role_t role)
{
    if (!(self->port_class & PORT_CLASS_PORT))
        return 0;

    if (role == PORT_NETDEV_ROLE_WAN)
    {
        if (self->p.port_cap == PORT_CAP_LAN_ONLY)
        {
            enet_err("%s: cannot change role to WAN, port has LAN_ONLY capability\n", self->obj_name);
            return -EFAULT;
        }
    }

    self->n.port_netdev_role = role;

    if (self->p.ops->role_set && self->p.ops->role_set(self, role))
        return -EFAULT;

    return 0;
}

int port_role_set(enetx_port_t *self, port_netdev_role_t role)
{
#if defined(CONFIG_BCM_PON)
    if (self->is_init)
        return 0;
#endif
 
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

    return _port_role_set(self, role);
}

int port_netdev_role_set(enetx_port_t *self, port_netdev_role_t role)
{
    if (port_role_set(self, role))
        return -EFAULT;
    
    enet_dev_role_update(self);

    return 0;
}

int port_generic_mtu_set(enetx_port_t *self, int mtu)
{
    if (self->p.mac)
        return mac_dev_mtu_set(self->p.mac, mtu);

    return 0;
}
    
static int port_type_resolve(enetx_port_t *port)
{
    int ret, port_id;
    enetx_port_t *parent_sw = port->p.parent_sw;
    port_type_t port_type;
    
    if (port->port_class != PORT_CLASS_PORT_DETECT)
        return -1;

    enet_dbg("initializing detect port %s..\n", port->obj_name);

    ret = parent_sw->s.ops->port_id_on_sw(&port->port_info, &port_id, &port_type);
    if (ret > 0)
        return 1; /* Skip creating port */

    if (ret)
    {
        enet_err("cannot resolve info (port %d) on %s\n", port_id, parent_sw->obj_name);
        return -1;
    }

    if (_assign_port_class(port, port_type))
        return -1;

    _set_class_port_defaults(port, port_id);

    return 0;
}

int port_activate(enetx_port_t *port)
{
    int ret;

    ret = port_type_resolve(port);
    if (ret)
        return ret;

    ret = sw_init(root_sw);
    if (ret && ret != EALREADY)
    {
        sw_free(&port);
        return -1;
    }

    return 0;
}

int port_deactivate(enetx_port_t *p)
{
    if (p->port_class != PORT_CLASS_PORT)
        return 0;

    if (p->p.mac)
    {
        mac_dev_disable(p->p.mac);
    }

    if (p->p.phy)
    {
        phy_dev_link_change_unregister(p->p.phy);
        phy_dev_power_set(p->p.phy, 0);
    }

    port_link_change(p, 0);

    if (p->is_init && p->p.ops->uninit)
        p->p.ops->uninit(p);

    p->is_init = 0;

    /* Removing NetDevice must be last */
    if (p->dev)
        enet_remove_netdevice(p);

    synchronize_rcu();

    if (_assign_port_class(p, PORT_TYPE_DETECT))
        return -1;

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
    net_stats->rx_dropped += mac_stats.rx_dropped;
    net_stats->rx_crc_errors += mac_stats.rx_fcs_error;
    net_stats->rx_errors += mac_stats.rx_alignment_error +
        mac_stats.rx_fcs_error +
        mac_stats.rx_code_error +
        mac_stats.rx_frame_length_error +
        mac_stats.rx_fifo_errors;
    net_stats->rx_length_errors += mac_stats.rx_frame_length_error;

    net_stats->tx_bytes += mac_stats.tx_byte;
    net_stats->tx_packets += mac_stats.tx_packet;
    net_stats->tx_dropped += mac_stats.tx_dropped;
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

void port_generic_sw_stats_get(enetx_port_t *self, struct rtnl_link_stats64 *net_stats)
{
    // get stats from device software counters
    net_stats->multicast += self->n.port_stats.rx_mcast_packets;
    net_stats->rx_bytes += self->n.port_stats.rx_bytes;
    net_stats->rx_packets += self->n.port_stats.rx_packets;
    net_stats->rx_dropped += self->n.port_stats.rx_dropped;

    net_stats->tx_bytes += self->n.port_stats.tx_bytes;
    net_stats->tx_packets += self->n.port_stats.tx_packets;
    net_stats->tx_dropped += self->n.port_stats.tx_dropped;

#if defined(CONFIG_BCM_KF_EXTSTATS)
    net_stats->tx_multicast_packets += self->n.port_stats.tx_mcast_packets;
    net_stats->rx_multicast_bytes += self->n.port_stats.rx_mcast_bytes;
    net_stats->tx_multicast_bytes += self->n.port_stats.tx_mcast_bytes;
    net_stats->rx_broadcast_packets += self->n.port_stats.rx_bcast_packets;
    net_stats->tx_broadcast_packets += self->n.port_stats.tx_bcast_packets;
#endif
}

int port_generic_pause_get(enetx_port_t *self, int *rx_enable, int *tx_enable)
{
    int rc;

    if (!self->p.mac)
    {
        enet_err("missing mac device in port %s\n", self->obj_name);
        return -1;
    }

    if (self->p.phy && !IsPortConnectedToExternalSwitch(self->p.phy->meta_id))
    {
        uint32_t caps;
        rc = phy_dev_caps_get(self->p.phy, CAPS_TYPE_ADVERTISE, &caps);
        if (rc) return -1;

        *rx_enable = 0; *tx_enable = 0; rc = 0;
        if (caps & PHY_CAP_PAUSE)
        {
            *rx_enable = 1; 
            *tx_enable = (caps & PHY_CAP_PAUSE_ASYM) ? 0 : 1;
        }
        else if (caps & PHY_CAP_PAUSE_ASYM)
        {
            *tx_enable = 1;
        }
        return 0;
    }
    else
    {
        return mac_dev_pause_get(self->p.mac, rx_enable, tx_enable);
    }
}

int port_generic_pause_set(enetx_port_t *self, int rx_enable, int tx_enable)
{
    int rc = 0;

    if (!self->p.mac)
    {
        enet_err("missing mac in port %s\n", self->obj_name);
        return -1;
    }

    if (self->p.phy && !IsPortConnectedToExternalSwitch(self->p.phy->meta_id))
    {
        uint32_t caps, new_caps;
        phy_dev_caps_get(self->p.phy, CAPS_TYPE_ADVERTISE, &caps);
        new_caps = caps & ~(PHY_CAP_PAUSE | PHY_CAP_PAUSE_ASYM);

        if (rx_enable && tx_enable)
            new_caps |= PHY_CAP_PAUSE;
        else if (rx_enable)
            new_caps |= PHY_CAP_PAUSE | PHY_CAP_PAUSE_ASYM;
        else if (tx_enable)
            new_caps |= PHY_CAP_PAUSE_ASYM;
            
        if (new_caps != caps)
            rc = phy_dev_caps_set(self->p.phy, new_caps);
        
        phy_dev_caps_get(self->p.phy, CAPS_TYPE_ADVERTISE, &caps);
        if (caps != new_caps)
            return -1;
    }
    else
    {
        rc = mac_dev_pause_set(self->p.mac, rx_enable, tx_enable, self->dev ? self->dev->dev_addr : NULL);
    }

    return rc;
}
