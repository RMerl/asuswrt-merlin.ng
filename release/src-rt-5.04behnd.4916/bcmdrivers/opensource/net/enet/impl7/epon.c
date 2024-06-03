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

/*
 *  Created on: Nov/2022
 *      Author: ido@broadcom.com
 */

#include "port.h"
#include "mux_index.h"
#include "runner.h"
#include "runner_common.h"
#include <rdpa_api.h>
#include "enet_dbg.h"
#include <net_port.h>


static void port_runner_epon_stats(enetx_port_t *self, struct rtnl_link_stats64 *net_stats)
{
    port_generic_stats_get(self, net_stats);
}

static bdmf_object_handle create_epon_rdpa_port(char* interface_name, mac_dev_t *mac_dev, uint32_t enet_index, int is_wan)
{
    BDMF_MATTR_ALLOC(rdpa_port_attrs, rdpa_port_drv());
    bdmf_object_handle port_obj = NULL;
    bdmf_object_handle cpu_obj = NULL;
    rdpa_port_type type = rdpa_port_xepon;
    rdpa_port_dp_cfg_t port_cfg = {0};
    mac_cfg_t mac_cfg = {};
    bdmf_error_t rc = 0;

    if (!mac_dev || !mac_dev->mac_drv)
        goto Exit;

#if (defined(CONFIG_BCM_PON_XRDP) || defined(CONFIG_BCM_DSL_XRDP)) && !defined(CONFIG_ONU_TYPE_SFU)
    /* XRDP: For WAN port, SAL and DAL should be disabled by default */
    port_cfg.sal_enable = 0;
    port_cfg.dal_enable = 0;
#else
    port_cfg.sal_enable = 1;
    port_cfg.dal_enable = 1;
#endif
    port_cfg.sal_miss_action = rdpa_forward_action_host;
    port_cfg.dal_miss_action = rdpa_forward_action_host;
    if (mac_dev->mac_drv->mac_type == MAC_TYPE_EPON_AE)
        type = rdpa_port_epon_ae;

    mac_dev_cfg_get(mac_dev, &mac_cfg);
    if (((type != rdpa_port_epon_ae)) && (mac_cfg.speed != MAC_SPEED_10000))
    {
        type = rdpa_port_epon;
    }

    if ((rc = rdpa_port_type_set(rdpa_port_attrs, type)))
    {
        enet_err("Failed to set RDPA port type %d. rc=%d\n", type, rc);
        goto Exit;
    }

    if ((rc = rdpa_port_index_set(rdpa_port_attrs, 0)))
     {
        enet_err("Failed to set RDPA port index 0. rc=%d\n", rc);
        goto Exit;
    }

    if ((rc = rdpa_port_handle_set(rdpa_port_attrs, enet_index)))
     {
        enet_err("Failed to set RDPA port index 0. rc=%d\n", rc);
        goto Exit;
    }

    if ((rc = rdpa_port_is_wan_set(rdpa_port_attrs, is_wan)))
    {
        enet_err("Failed to set RDPA port is_wan. rc=%d\n", rc);
        goto Exit;
    }

    rdpa_port_cfg_set(rdpa_port_attrs, &port_cfg);
    if (type == rdpa_port_epon_ae)
    {
        if (mac_cfg.speed == MAC_SPEED_100)
            rdpa_port_speed_set(rdpa_port_attrs,rdpa_speed_100m);
        else if (mac_cfg.speed == MAC_SPEED_1000)
            rdpa_port_speed_set(rdpa_port_attrs,rdpa_speed_1g);
        else if (mac_cfg.speed == MAC_SPEED_2500)
            rdpa_port_speed_set(rdpa_port_attrs,rdpa_speed_2_5g);
        else if (mac_cfg.speed == MAC_SPEED_5000)
            rdpa_port_speed_set(rdpa_port_attrs,rdpa_speed_5g);
        else if (mac_cfg.speed == MAC_SPEED_10000)
            rdpa_port_speed_set(rdpa_port_attrs,rdpa_speed_10g);
        else
        {
            enet_err("AE speed is not supported");
            goto Exit;
        }
    }
    else
    {
        if (type == rdpa_port_epon)
            rdpa_port_speed_set(rdpa_port_attrs,rdpa_speed_1g);
        else
            rdpa_port_speed_set(rdpa_port_attrs,rdpa_speed_10g);
    }

    rc = bdmf_new_and_set(rdpa_port_drv(), NULL, rdpa_port_attrs, &port_obj);
    if (rc < 0)
    {
        enet_err("Failed to create wan port");
        goto Exit;
    }

    rc = rdpa_cpu_get(rdpa_cpu_host, &cpu_obj);
    if (rc)
    {
        enet_err("Failed to get CPU object, rc(%d)", rc);
        goto Exit;
    }
#if defined(CONFIG_BCM_PON_XRDP) || defined(CONFIG_BCM_DSL_XRDP)
    rc = rdpa_port_cpu_obj_set(port_obj, cpu_obj);
    if (rc)
    {
        enet_err("Failed to set CPU object to WAN port, rc(%d)", rc);
        goto Exit;
    }
#endif

    if ((rc = runner_default_filter_init(port_obj, RDPA_FILTERS_GROUP_WAN)))
        enet_err("Failed to set up default filter for epon RDPA port. rc=%d\n", rc);

Exit:
    if (cpu_obj)
        bdmf_put(cpu_obj);

    if (rc && port_obj)
    {
        bdmf_destroy(port_obj);
        port_obj = NULL;
    }

    BDMF_MATTR_FREE(rdpa_port_attrs);

    return port_obj;
}

static int port_runner_epon_init(enetx_port_t *self)
{
    int is_ae = self->p.mac->mac_drv->mac_type == MAC_TYPE_EPON_AE;
    int rc;

    self->p.port_cap = is_ae ? PORT_CAP_LAN_WAN : PORT_CAP_WAN_ONLY;
    self->n.port_netdev_role = self->port_info.is_wan ? PORT_NETDEV_ROLE_WAN : PORT_NETDEV_ROLE_LAN;
    if (mux_set_rx_index(self->p.parent_sw, self->p.port_id, self))
        return -1;

    if (!(self->priv = create_epon_rdpa_port(self->name, self->p.mac, self->p.port_id, self->n.port_netdev_role == PORT_NETDEV_ROLE_WAN)))
    {
        enet_err("Failed to create EPON rdpa port object for %s\n", self->obj_name);
        return -1;
    }

    if (is_ae && (rc = create_rdpa_egress_tm(self->priv)))
        goto Exit;

    if (!is_ae)
        blog_chnl_with_mark_set(BLOG_EPONPHY, self); /*Epon mode: channel will be set to/from llid */
    else
        blog_chnl_unit_port_set(self);

Exit:
    return rc;
}

static int port_runner_epon_uninit(enetx_port_t *self)
{
    bdmf_object_handle port_obj = self->priv;
    rdpa_port_tm_cfg_t port_tm_cfg;

    if (!port_obj)
        return 0;

    if (!rdpa_port_tm_cfg_get(port_obj, &port_tm_cfg))
        if (port_tm_cfg.sched)
            bdmf_destroy(port_tm_cfg.sched);

    rdpa_port_uninit_set(port_obj, 1);
    bdmf_destroy(port_obj);
    self->priv = 0;

    if ((self->p.mac->mac_drv->mac_type == MAC_TYPE_EPON_AE))
        blog_chnl_unit_port_unset(self);

    mux_set_rx_index(self->p.parent_sw, self->p.port_id, NULL);
    return 0;
}

static int dispatch_pkt_epon(dispatch_info_t *dispatch_info)
{
    int rc;
    rdpa_cpu_tx_info_t info = {};

    /* TODO: queue/channel mapping from EponFrame.c:EponTrafficSend() */
    info.method = rdpa_cpu_tx_port;
    info.port_obj = dispatch_info->port->priv;      
    info.cpu_port = rdpa_cpu_host;
    info.x.wan.queue_id = dispatch_info->egress_queue;
    if (dispatch_info->port->n.set_channel_in_mark)
        info.x.wan.flow = dispatch_info->channel;

    info.drop_precedence = dispatch_info->drop_eligible;

    rc = _rdpa_cpu_send_sysb((bdmf_sysb)dispatch_info->pNBuff, &info);

    return rc;
}

static int port_runner_epon_role_set(enetx_port_t *self, port_netdev_role_t role)
{
    int rc;

    if ((rc = rdpa_port_is_wan_set(self->priv, role == PORT_NETDEV_ROLE_WAN)))
        enet_err("Failed to set RDPA port is_wan for port %s. rc=%d\n", self->obj_name, rc);

    return rc;
}

port_ops_t port_runner_epon =
{
    .init = port_runner_epon_init,
    .post_init = port_runner_port_post_init,
    .uninit = port_runner_epon_uninit,
    .dispatch_pkt = dispatch_pkt_epon,
    .stats_get = port_runner_epon_stats,
    .mtu_set = port_runner_mtu_set,
    /* TODO: stats_clear */
    .mib_dump = port_runner_mib_dump,
    .print_status = port_runner_print_status,
    .print_priv = port_runner_print_priv,
    .link_change = port_runner_link_change,
    .role_set = port_runner_epon_role_set,
#if (defined(CONFIG_BCM_PON_XRDP) && defined(CONFIG_NET_SWITCHDEV) && (LINUX_VERSION_CODE < KERNEL_VERSION(5,1,0)))
    .switchdev_ops =
    {
        .switchdev_port_attr_get = runner_port_attr_get,
        .switchdev_port_attr_set = runner_port_attr_set,
    },
#endif
    .mib_dump_us = port_runner_mib_dump_us, // add by Andrew
};

