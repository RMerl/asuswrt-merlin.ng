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
 *  Created on: May/2018
 *      Author: ido@broadcom.com
 */

#include "enet.h"
#include <rdpa_api.h>
#include "port.h"
#include "mux_index.h"
#ifdef CONFIG_BCM_FTTDP_G9991
#include "g9991.h"
#endif
#include "runner_common.h"
#ifdef CONFIG_BCM_PTP_1588
#include "ptp_1588.h"
#endif

/* Forward declerations */
port_ops_t port_runner_port_wan_gbe;
port_ops_t port_runner_port;

/* Note: DSL_DEVICES in runner_standalone.c are 63146 and 4912 devices */
#if !defined(DSL_DEVICES)
static int runner_hw_switching_state = HW_SWITCHING_DISABLED;

#if defined(CONFIG_BCM_RUNNER_FLOODING) && !defined(NETDEV_HW_SWITCH)
int runner_port_flooding_set(bdmf_object_handle port_obj, int enable)
{
    int rc = 0;
    rdpa_port_dp_cfg_t cfg;

    if ((rc = rdpa_port_cfg_get(port_obj , &cfg)))
        return -EFAULT;

    if (!cfg.dal_enable)
    {
        enet_err("DA lookup disabled on port, cannot configure flooding\n");
        return -EFAULT;
    }

    if (enable)
    {
        /* Enable flooding. */
        cfg.dal_miss_action = rdpa_forward_action_flood;
    }
    else
    {
        /* Disable flooding, assume action host. XXX: Are we required to store previous action and support drop? */
        cfg.dal_miss_action = rdpa_forward_action_host;
    }
    rc = rdpa_port_cfg_set(port_obj, &cfg);
    return rc;
}

static int tr_runner_hw_switching_set_single(enetx_port_t *port, void *_ctx)
{
    unsigned long state = (unsigned long)_ctx;

    if (!port->dev)
        return 0;

    return runner_port_flooding_set(port->priv, state == HW_SWITCHING_ENABLED);
}
#endif

static int runner_hw_switching_set(enetx_port_t *sw, unsigned long state)
{
    int rc = -1;

#if defined(CONFIG_BCM_RUNNER_FLOODING) && !defined(NETDEV_HW_SWITCH)
    rc = port_traverse_ports(root_sw, tr_runner_hw_switching_set_single, PORT_CLASS_PORT, (void *)state);
    if (!rc)
        runner_hw_switching_state = (int)state;
#endif

    return rc;
}

static int runner_hw_switching_get(enetx_port_t *sw)
{
    return runner_hw_switching_state;
}
#endif //!DSL_DEVICES

sw_ops_t port_runner_sw =
{
    .init = port_runner_sw_init,
    .uninit = port_runner_sw_uninit,
    .mux_port_rx = mux_get_rx_index,
    .stats_get = port_generic_stats_get,
    .port_id_on_sw = port_runner_sw_port_id_on_sw,
#if defined(CONFIG_BCM_KERNEL_BONDING)
    .config_trunk = port_runner_sw_config_trunk,
#endif
#if !defined(DSL_DEVICES)
    .hw_sw_state_set = runner_hw_switching_set,
    .hw_sw_state_get = runner_hw_switching_get,
#endif
};

#if defined(CONFIG_BCM_RUNNER_QOS_MAPPER)
int link_pbit_tc_to_q_to_rdpa_lan_port(bdmf_object_handle port_obj)
{
    bdmf_error_t rc = 0;
    bdmf_object_handle rdpa_pbit_to_queue_obj = NULL;
#if defined(CONFIG_BCM_PLATFORM_RDP_PRV)
    bdmf_object_handle rdpa_tc_to_queue_obj = NULL;
    bdmf_number table_index = 0; 

    rc = rdpa_tc_to_queue_get(table_index, &rdpa_tc_to_queue_obj);
    if (rc != 0)
    {
        enet_err("Failed to get tc_to_q object. rc=%d\n", rc);
        goto Exit;
    }
   
    rc = bdmf_link(rdpa_tc_to_queue_obj, port_obj, NULL);
    if (rc != 0)
    {
        enet_err("Failed to link tc_to_q table to RDPA port rc=%d\n", rc);
        goto Exit;
    }
#endif    
    rc = rdpa_pbit_to_queue_get(0, &rdpa_pbit_to_queue_obj);
    if (rc != 0)
    {
        enet_err("Failed to get pbit_to_q object. rc=%d\n", rc);
        goto Exit;
    }
   
    rc = bdmf_link(rdpa_pbit_to_queue_obj, port_obj, NULL);
    if (rc != 0)
    {
        enet_err("Failed to link pbit_to_q table to RDPA port rc=%d\n", rc);
        goto Exit;
    }

Exit:
#if defined(CONFIG_BCM_PLATFORM_RDP_PRV)
    if (rdpa_tc_to_queue_obj)
        bdmf_put(rdpa_tc_to_queue_obj);
#endif
    if (rdpa_pbit_to_queue_obj)
        bdmf_put(rdpa_pbit_to_queue_obj);

    return rc;
}
#endif // !DSL_DEVICES

static int __port_runner_parent_port_init(enetx_port_t *self)
{
    self->priv = create_rdpa_port(self, self->p.port_id, NULL, 0, 0, 0);
    if (!self->priv)
    {
        enet_err("Failed to create RDPA port object for %s\n", self->obj_name);
        return -1;
    }
    return 0;
}

static int __port_runner_standalone_port_init(enetx_port_t *self)
{
    if (mux_set_rx_index(self->p.parent_sw, self->p.port_id, self))
        return -1;

    blog_chnl_unit_port_set(self);

    self->priv = create_rdpa_port(self, self->p.port_id, NULL, 1, 0, 1);
    if (!self->priv)
    {
        enet_err("Failed to create RDPA port object for %s\n", self->obj_name);
        return -1;
    }

    port_runner_wan_role_set(self, self->port_info.is_wan ? PORT_NETDEV_ROLE_WAN : PORT_NETDEV_ROLE_LAN);

#if defined(DSL_DEVICES)
    self->p.ops->print_status = port_print_status_verbose;
#else
#if defined(CONFIG_BCM_RUNNER_QOS_MAPPER) && !defined(CONFIG_BCM_FTTDP_G9991)
    if (!self->port_info.is_wan)
    {
        if (link_pbit_tc_to_q_to_rdpa_lan_port(self->priv))
            return -1;
    }
#endif
#endif //!DSL_DEVICES

    return 0;
}

int port_runner_port_init(enetx_port_t *self)
{
    bdmf_error_t rc = BDMF_ERR_OK;

    if (self->port_info.is_attached)
    {
        rc = __port_runner_parent_port_init(self);
    }
    else
    {
        rc = __port_runner_standalone_port_init(self);
    }

    enet_dbg("Initialized runner port %s\n", self->obj_name);

    return rc;
}

#if defined(CONFIG_BCM_FTTDP_G9991) && defined(XRDP)
void __dispatch_pkt_skb_check_bcast_mcast(dispatch_info_t *dispatch_info)
{
    struct sk_buff *skb = (struct sk_buff *)dispatch_info->pNBuff;
    struct ethhdr *eth = (struct ethhdr *)skb_mac_header(skb);

    if (unlikely(*eth->h_dest & 1))
    {
        if (is_broadcast_ether_addr(eth->h_dest))
            skb->pkt_type=PACKET_BROADCAST;
        else
            skb->pkt_type=PACKET_MULTICAST;
    }
}
#endif

static inline void rdpa_cpu_tx_info_init(dispatch_info_t *dispatch_info, rdpa_cpu_tx_info_t *info)
{
    info->method = rdpa_cpu_tx_port;
    info->port_obj = dispatch_info->port->priv;
    info->cpu_port = rdpa_cpu_host;
    info->drop_precedence = dispatch_info->drop_eligible;
    info->flags = 0;
    info->bits.no_lock = dispatch_info->no_lock;
}

static int port_runner_dispatch_dg_pkt(dispatch_info_t *dispatch_info)
{
    rdpa_cpu_tx_info_t info = {};

    rdpa_cpu_tx_info_init(dispatch_info, &info);
    info.x.wan.queue_id = dispatch_info->egress_queue;

    return rdpa_cpu_send_sysb((bdmf_sysb)dispatch_info->pNBuff, &info);
}

int port_runner_dispatch_pkt_lan(dispatch_info_t *dispatch_info)
{
    rdpa_cpu_tx_info_t info = {};
#ifdef CONFIG_BCM_PTP_1588
    char *ptp_offset;
#endif

    rdpa_cpu_tx_info_init(dispatch_info, &info);
    info.x.lan.queue_id = dispatch_info->egress_queue;

    enet_dbg_tx("rdpa_cpu_send: port %s queue %d\n", dispatch_info->port->name, dispatch_info->egress_queue);

#ifdef CONFIG_BCM_PTP_1588
    if (unlikely(is_pkt_ptp_1588(dispatch_info->pNBuff, &info, &ptp_offset)))
        return ptp_1588_cpu_send_sysb((bdmf_sysb)dispatch_info->pNBuff, &info, ptp_offset, dispatch_info->port);
#endif

#if defined(CONFIG_BCM_FTTDP_G9991) && defined(XRDP)
    if (IS_SKBUFF_PTR(dispatch_info->pNBuff))
        __dispatch_pkt_skb_check_bcast_mcast(dispatch_info);
#endif
    return _rdpa_cpu_send_sysb((bdmf_sysb)dispatch_info->pNBuff, &info);
}

static int dispatch_pkt_gbe_wan(dispatch_info_t *dispatch_info)
{
    rdpa_cpu_tx_info_t info = {};

    rdpa_cpu_tx_info_init(dispatch_info, &info);
    info.x.wan.queue_id = dispatch_info->egress_queue;

    enet_dbg_tx("rdpa_cpu_send: port %d queue %d\n", info.port, dispatch_info->egress_queue);

#if defined(CONFIG_BCM_FTTDP_G9991) && defined(XRDP)
    if (IS_SKBUFF_PTR(dispatch_info->pNBuff))
        __dispatch_pkt_skb_check_bcast_mcast(dispatch_info);
#endif
    return _rdpa_cpu_send_sysb((bdmf_sysb)dispatch_info->pNBuff, &info);
}

int enetxapi_post_config(void)
{
#if defined(DSL_DEVICES)
    print_mac_phy_info_all();
#endif
    return 0;
}

#if defined(DSL_DEVICES)
extern void extlh_mac2mac_port_handle(enetx_port_t *self);

static void port_extlh_runner_open(enetx_port_t *self)
{ 
    extlh_mac2mac_port_handle(self);
    port_generic_open(self);
}
#endif

static int port_runner_role_set(enetx_port_t *port, port_netdev_role_t role)
{
    int rc;
#if !defined(DSL_DEVICES) && !defined(CONFIG_BCM_FTTDP_G9991)
    int is_wan;
#endif

#ifdef CONFIG_BCM_FTTDP_G9991
    /* nothing to change */
    if (port->n.port_netdev_role == role)
        return 0;

    rc = port_runner_port_uninit(port);
    if (rc)
    {
        enet_err("Failed to create uninit RDPA port object for %s\n", port->obj_name);
        return rc;
    }

    port->port_info.is_wan = role == PORT_NETDEV_ROLE_WAN;

    rc = port_runner_port_init(port);
    if (rc)
    {
        enet_err("Failed to create init RDPA port object for %s\n", port->obj_name);
        return rc;
    }

    rc = port_runner_port_post_init(port);
    if (rc)
    {
        enet_err("port_runner_port_post_init failed, object %s\n", port->obj_name);
        return rc;
    }

    return 0;
#else

    rc = port_runner_wan_role_set(port, role);
    if (rc)
         return rc;

#if defined(DSL_DEVICES)
    rc = port_set_wan_role_link(port, role);
#else
    is_wan = port->n.port_netdev_role == PORT_NETDEV_ROLE_WAN;
    /* override ops with correct dispatch_pkt */
    if (is_wan)
        port->p.ops = &port_runner_port_wan_gbe;
    else
        port->p.ops = &port_runner_port;
#endif
#endif

    return rc;
}

port_ops_t port_runner_port =
{
    .init = port_runner_port_init,
    .post_init = port_runner_port_post_init,
    .uninit = port_runner_port_uninit,
    .dispatch_pkt = port_runner_dispatch_pkt_lan,
    .dispatch_dg_pkt = port_runner_dispatch_dg_pkt,
    .stats_get = port_runner_port_stats_get,
    .stats_clear = port_runner_port_stats_clear,
    .pause_get = port_generic_pause_get,
    .pause_set = port_generic_pause_set,
    .mtu_set = port_runner_mtu_set,
    .mib_dump = port_runner_mib_dump,
    .print_status = port_runner_print_status,
    .role_set = port_runner_role_set,
    .print_priv = port_runner_print_priv,
    .link_change = port_runner_link_change,
#if defined(DSL_DEVICES)
    .open = port_extlh_runner_open,
#endif
#if defined(CONFIG_NET_SWITCHDEV) && (LINUX_VERSION_CODE < KERNEL_VERSION(5,1,0))
    .switchdev_ops =
    {
        .switchdev_port_attr_get = runner_port_attr_get,
        .switchdev_port_attr_set = runner_port_attr_set,
    },
#endif
    .mib_dump_us = port_runner_mib_dump_us, // add by Andrew
};

port_ops_t port_runner_port_wan_gbe =
{
    .init = port_runner_port_init,
    .post_init = port_runner_port_post_init,
    .uninit = port_runner_port_uninit,
    .dispatch_pkt = dispatch_pkt_gbe_wan,
    .dispatch_dg_pkt = port_runner_dispatch_dg_pkt,
    .stats_get = port_runner_port_stats_get,
    .stats_clear = port_runner_port_stats_clear,
    .pause_get = port_generic_pause_get,
    .pause_set = port_generic_pause_set,
    .mtu_set = port_runner_mtu_set,
    .mib_dump = port_runner_mib_dump,
    .print_status = port_runner_print_status,
    .print_priv = port_runner_print_priv,
    .role_set = port_runner_role_set,
    .link_change = port_runner_link_change,
#if defined(DSL_DEVICES)
    .open = port_extlh_runner_open,
#endif
#if defined(CONFIG_NET_SWITCHDEV) && (LINUX_VERSION_CODE < KERNEL_VERSION(5,1,0))
    .switchdev_ops =
    {
        .switchdev_port_attr_get = runner_port_attr_get,
        .switchdev_port_attr_set = runner_port_attr_set,
    },
#endif
    .mib_dump_us = port_runner_mib_dump_us, // add by Andrew
};

