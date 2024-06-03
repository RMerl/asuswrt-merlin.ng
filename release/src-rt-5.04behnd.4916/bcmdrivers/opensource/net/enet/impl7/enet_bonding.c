/*
   <:copyright-BRCM:2022:DUAL/GPL:standard

      Copyright (c) 2022 Broadcom 
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

#if defined(CONFIG_BCM_KERNEL_BONDING) && !defined(CONFIG_BCM_ETHTOOL)
#error "CONFIG_BCM_ETHTOOL/BUILD_ETHTOOL must be defined with CONFIG_BCM_KERNEL_BONDING"
#endif

#include <linux/types.h>
#include <linux/bcm_log.h>
#include "enet.h"
#include "port.h"

extern int port_by_netdev(struct net_device *dev, enetx_port_t **match);
extern void update_pbvlan_all_bridge(void);
extern enetx_port_t *port_by_unit_port(int unit_port);


bond_info_t bond_grps[MAX_KERNEL_BONDING_GROUPS];

struct tr_sw_bond_map_data
{
    bond_info_t *grp;
    uint32_t pmap;
};

static int tr_sw_bond_map(enetx_port_t *port, void *_ctx)
{
    struct tr_sw_bond_map_data *data = (struct tr_sw_bond_map_data *)_ctx;

    if (port->p.bond_grp == data->grp)
        data->pmap |= 1 << port->port_info.port;
    return 0;
}

static uint32_t enet_get_sw_bonding_map(enetx_port_t *sw, uint16_t grp_no)
{
    struct tr_sw_bond_map_data data = {};

    data.grp = &bond_grps[grp_no];
    // traverse current switch only
    _port_traverse_ports(sw, tr_sw_bond_map, PORT_CLASS_PORT, &data, 1);
    return data.pmap;
}

#if defined(CONFIG_BCM_LOG)

struct tr_bond_chk_data
{
    bond_info_t *grp;
    enetx_port_t *port;                 // port to be added/removed
    uint32_t pmap[COMPAT_MAX_SWITCHES];
    int lan_cnt, wan_cnt;
};

static int tr_bond_err_chk(enetx_port_t *port, void *_ctx)
{
    struct tr_bond_chk_data *data = (struct tr_bond_chk_data *)_ctx;

    if ((port->p.bond_grp == data->grp) || (port == data->port))
    {
        data->pmap[PORT_ON_ROOT_SW(port)?0:1] |= 1<< port->port_info.port;
        if (PORT_ROLE_IS_WAN(port) && data->grp->lan_wan_port != port)
            data->wan_cnt++;
        else
            data->lan_cnt++;
    }
    return 0;
}
/* Function to do error check before making changes to the bonding group */
static int enet_bonding_error_check(bond_info_t *grp, enetx_port_t *port)
{
    struct tr_bond_chk_data data = {};
    int cnt, ndx;

    data.grp = grp;
    data.port = port;
    port_traverse_ports(root_sw, tr_bond_err_chk, PORT_CLASS_PORT, &data);

#if defined(CONFIG_BCM_DSL_RDP)
    /* WAN-LAN bonding is supported on 3GEN DSL platforms only */
    if (data.wan_cnt > 1)
    {
        enet_err("Two WAN ports can't be bonded <0x%04x-0x%04x>\n", data.pmap[0], data.pmap[1]);
        return -1;
    }
    if (data.wan_cnt && data.lan_cnt > 1)
    {
        enet_err("More than one LAN port can't be bonded with WAN <0x%04x-0x%04x>\n", data.pmap[0], data.pmap[1]);
        return -1;
    }
#else
    if (data.wan_cnt && data.lan_cnt)
    {
        enet_err("LAN port can't be bonded with WAN <0x%04x-0x%04x>\n", data.pmap[0], data.pmap[1]);
        return -1;
    }
#endif

    for (ndx=0,cnt=0; ndx < COMPAT_MAX_SWITCHES; ndx++)
        if (data.pmap[ndx]) cnt++;
    if (cnt > 1 && !data.wan_cnt)
    {
        enet_err("Can't bond across switch units <0x%04x-0x%04x>\n", data.pmap[0], data.pmap[1]);
        return -1;
    }
    return 0;
}

struct tr_bond_wan_cfg_data
{
    bond_info_t *grp;
    enetx_port_t *port;         // port to be added/removed
    enetx_port_t *lan_port;
    enetx_port_t *wan_port;
};

static int tr_bond_wan_cfg(enetx_port_t *port, void *_ctx)
{
    struct tr_bond_wan_cfg_data *data = (struct tr_bond_wan_cfg_data *)_ctx;

    if ((port->p.bond_grp == data->grp) || (port == data->port))
    {
        if (PORT_ROLE_IS_WAN(port) && data->grp->lan_wan_port != port)
            data->wan_port = port;
        else
            data->lan_port = port;
    }

    return (data->wan_port && data->lan_port)? 1 : 0;
}

/* Function to configure the switch port as WAN port based on grouping */
static void bcmenet_do_wan_bonding_cfg_for_grp(uint16_t grp_no, uint16_t add_member, enetx_port_t *port)
{
    /* For the provided group and port, configuration is changed. Take care of any WAN port related configuration */
    struct tr_bond_wan_cfg_data data = {};
    bond_info_t *bond_grp = &bond_grps[grp_no];

    data.grp = bond_grp;
    data.port = port;
    port_traverse_ports(root_sw, tr_bond_wan_cfg, PORT_CLASS_PORT, &data);

    if (data.wan_port && data.lan_port) /* Both LAN & WAN are/were part of the group */
    {
        if (add_member ^ bond_grp->is_lan_wan_cfg)
        {
            /* modify lan port port_cap, so role_set won't fail */
#if defined(CONFIG_BCM_DSL_RDP)
            data.lan_port->p.port_cap = add_member? PORT_CAP_LAN_WAN : PORT_CAP_LAN_ONLY;
#endif
            port_netdev_role_set(data.lan_port, add_member? PORT_NETDEV_ROLE_WAN: PORT_NETDEV_ROLE_LAN);
            /* also clear lan port ARL entries */
            if (add_member && data.lan_port->p.ops->fast_age)
                data.lan_port->p.ops->fast_age(data.lan_port);
            bond_grp->is_lan_wan_cfg = add_member;
            bond_grp->lan_wan_port = add_member ? data.lan_port : NULL;
            if (add_member)
                netdev_wan_set(bond_grp->bond_dev);
            else
                netdev_wan_unset(bond_grp->bond_dev);
        }
    }
}

#ifdef CONFIG_BLOG
static int tr_set_grp_blog_chnl(enetx_port_t *port, void *_ctx)
{
    bond_info_t *bond_grp = (bond_info_t *)_ctx;

    if ((port->p.bond_grp == bond_grp) && (bond_grp->blog_chnl_rx > port->n.blog_chnl))
        bond_grp->blog_chnl_rx = port->n.blog_chnl;
    return 0;
}
static int tr_set_port_blog_chnl_rx(enetx_port_t *port, void *_ctx)
{
    bond_info_t *bond_grp = (bond_info_t *)_ctx;

    if (port->p.bond_grp == bond_grp)
    {
        if (bond_grp->blog_chnl_rx == port->n.blog_chnl)
            netdev_path_set_hw_port(bond_grp->bond_dev, port->n.blog_chnl, port->n.blog_phy);
        port->n.blog_chnl_rx = bond_grp->blog_chnl_rx;
    }
    return 0;
}
static void update_bond_grp_blog_chnl_rx(bond_info_t *bond_grp)
{
    if (bond_grp->port_count == 0)
        return;
    /* find lowest blog_chnl_rx */
    bond_grp->blog_chnl_rx = 0xffffffff;
    port_traverse_ports(root_sw, tr_set_grp_blog_chnl, PORT_CLASS_PORT, bond_grp);
    enet_dbgv("bond_grp %d blog_chnl=%x\n", bond_grp->grp_idx, bond_grp->blog_chnl_rx);
    /* set all member ports with this this blog_chnl_rx value */
    port_traverse_ports(root_sw, tr_set_port_blog_chnl_rx, PORT_CLASS_PORT, bond_grp);
}
#endif /* CONFIG_BLOG */

static int enet_get_bond_grp_no(struct net_device *bond_dev)
{
    int grp_no;

    if (bond_dev) {
        // matching valid one first
        for (grp_no = 0; grp_no < MAX_KERNEL_BONDING_GROUPS; grp_no++)
            if (bond_grps[grp_no].bond_dev == bond_dev)
                return grp_no;
        // if none found, get 1st NULL
        for (grp_no = 0; grp_no < MAX_KERNEL_BONDING_GROUPS; grp_no++)
            if (!bond_grps[grp_no].bond_dev)
                return grp_no;
    }
    return -1;
}

static int enet_update_bond_config(int is_join, struct net_device *bond_dev, struct net_device *slave_dev, enetx_port_t *port)
{
    int grp_no = enet_get_bond_grp_no(bond_dev);
    int rc = 0;
    bond_info_t *bond_grp;

    if (grp_no >= MAX_KERNEL_BONDING_GROUPS || grp_no < 0)
    {
        enet_err("can't locate or exceed %d trunking groups!!!\n", MAX_KERNEL_BONDING_GROUPS);
        return -1;
    }

    bond_grp = &bond_grps[grp_no];
    rc = enet_bonding_error_check(bond_grp, port);
    if (rc)
    {
        return rc;
    }
    if (is_join)
    {
        /* Check if already a member */
        if (port->p.bond_grp)
        {
            enet_err("%s already a member of bond group = %d\n", port->obj_name, grp_no);
            return 0;
        }
        if (bond_grp->port_count == 0)
            bond_grp->bond_dev = bond_dev;
        bond_grp->port_count++;
        bond_grp->grp_idx = grp_no;
        port->p.bond_grp = bond_grp;
        if (port->p.ops->stp_set)
            port->p.ops->stp_set(port, STP_MODE_DISABLE, STP_STATE_UNCHANGED);
    }
    else
    {
        /* Check if not already a member */
        if (!port->p.bond_grp)
        {
            enet_err("%s not a member of bond group = %d\n", port->obj_name, grp_no);
            return 0;
        }
        bond_grp->port_count--;
        port->p.bond_grp = NULL;

#ifdef CONFIG_BLOG
        port->n.blog_chnl_rx = port->n.blog_chnl;
#endif
        if (bond_grp->port_count == 0)
            bond_grp->bond_dev = NULL;
    }
    /* bonding group membeship changed, update blog_chnl_rx */
#ifdef CONFIG_BLOG
    update_bond_grp_blog_chnl_rx(bond_grp);
#endif

    /* Update HW Switch - restricting to only External switch for now */
    if (port->p.parent_sw->s.ops->config_trunk)
        rc = port->p.parent_sw->s.ops->config_trunk(port->p.parent_sw, port, grp_no, is_join);

    if (!rc)
    {
        bcmenet_do_wan_bonding_cfg_for_grp(grp_no, is_join, port);
    }

    if (bond_grp->port_count == 0) /* No more members in the bond group */
    {
        memset(bond_grp, 0, sizeof(*bond_grp));
    }

    return rc;
}

static int bcmenet_is_dev_in_slave_path(void *ctxt)
{
    struct net_device *slave_dev = ((BCM_BondDevInfo*)ctxt)->slave_dev;
    struct net_device **bond_dev = ((BCM_BondDevInfo*)ctxt)->bond_dev;
    enetx_port_t *port;

    if (port_by_netdev(netdev_path_get_root(slave_dev), &port))
        return 0;

    if (port == NULL || port->p.bond_grp == NULL)
        return 0;

    if (*bond_dev == NULL)  /* check if slave_dev is part of any bond group */
    {
        *bond_dev = port->p.bond_grp->bond_dev;
        return 1;
    }
    else if (*bond_dev == port->p.bond_grp->bond_dev)
        return 1;
    return 0;
}

static int bcmenet_handle_bonding_change(struct net_device *slave_dev)
{
    struct net_device *bond_dev = netdev_master_upper_dev_get(slave_dev);
    struct net_device *dev = slave_dev;
    int is_join = bond_dev?1:0;
    int print_once = 1;
    int err = 0;
    enetx_port_t *port;
    /* find root device */
    while( 1 )
    {
        if(netdev_path_is_root(dev))
        {
            break;
        }
        if (print_once && is_join)
        {
            print_once = 0;
            /* One of the major issue with non-root device bonding is that if leaf device gets deleted/unregistered, Ethernet driver
               won't know which physical device it was associated with and will not remove the bonding configuration */
            enet_err("\n\n WARNING : Slave device <%s> is not a root device; Bonding must be done on physical interfaces.\n\n",dev->name);
        }
        /* slave_dev is on hold in Bonding driver -- don't put it back */
        if (dev != slave_dev)
        {
            dev_put(dev);
        }
        dev = netdev_path_next_dev(dev);
        dev_hold(dev);
    }

    /* Check if this root device is managed by Ethernet Driver */
    if (port_by_netdev(dev, &port) == 0) 
    {
        if (bond_dev == NULL)
            bond_dev = port->p.bond_grp->bond_dev;
        err = enet_update_bond_config(is_join, bond_dev, slave_dev, port);
    }
    else
    {
        enet_err("Slave Device <%s> Root Dev <%s> not managed by Ethernet Driver\n",slave_dev->name,dev->name);
    }

    if (dev != slave_dev)
    {
        dev_put(dev);
    }

    /* Based on sequence of operations, like:
       - remove ethernet interface (say eth1) from bridge => all other bridge ports will isolate this ethernet interface
       - add bond interface to bridge prior to adding eth1 to bond interface 
       - now add eth1 to bond interface, this will not trigger any bridge update notification and eth1 will be left out. 
      * to avoid above condition, better to update the pbvlan mapping on every bonding update, if bond interface is in bridge. */ 
    if (!err && bond_dev && netif_is_bridge_port(bond_dev))
    {
        /* Would have been better to only update for the bridge this bond interface is part of ...
           but don't know any easy way to get the bridge from device. */
        update_pbvlan_all_bridge();
    }

    return err;
}
#endif /* CONFIG_BCM_LOG */

int bonding_is_lan_wan_port(void *ctxt)
{
    int ret_val = 0;
#ifdef CONFIG_BLOG
    int logical_port = *((int*)ctxt);
    enetx_port_t *port = blog_chnl_to_port[logical_port];
#else
    enetx_port_t *port = NULL;
#endif

    if (port && port->p.bond_grp)
    {
        if (port->p.bond_grp->is_lan_wan_cfg &&
            port->p.bond_grp->lan_wan_port == port )
        {
            ret_val = 1;
        }
    }

    return ret_val;
}

void bonding_init(void)
{
    bcmFun_reg(BCM_FUN_ID_ENET_IS_DEV_IN_SLAVE_PATH, bcmenet_is_dev_in_slave_path);
    if (!bcmFun_get(BCM_FUN_ID_ENET_IS_BONDED_LAN_WAN_PORT))
        bcmFun_reg(BCM_FUN_ID_ENET_IS_BONDED_LAN_WAN_PORT, bonding_is_lan_wan_port);
}

void bonding_uninit(void)
{
    bcmFun_dereg(BCM_FUN_ID_ENET_IS_BONDED_LAN_WAN_PORT);
    bcmFun_dereg(BCM_FUN_ID_ENET_IS_DEV_IN_SLAVE_PATH);
}

int bonding_netdev_event(unsigned long event, struct net_device *dev)
{
    switch (event) 
    {
        case NETDEV_CHANGEUPPER:
            if (netif_is_bond_slave(dev))
            {
                if (bcmenet_handle_bonding_change(dev))
                    return -1;
            }
            return 0;
        default:
            return 0;
    }
}

int bonding_update_br_pbvlan(enetx_port_t *sw, struct net_device *dev, uint32_t *portMap)
{
    int mstr_id;

    if (!netif_is_bond_master(dev))
        return 0;

    mstr_id = enet_get_bond_grp_no(dev);
    if (mstr_id >= 0 && mstr_id < MAX_KERNEL_BONDING_GROUPS)
        *portMap |= enet_get_sw_bonding_map(sw, mstr_id);

    return 1;
}

