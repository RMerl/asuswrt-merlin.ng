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

#include "runner_wifi.h"
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

int runner_port_flooding_set(bdmf_object_handle port_obj, int enable)
{
    int rc = 0;
#if defined(CONFIG_BCM_RUNNER_FLOODING) && !defined(NETDEV_HW_SWITCH)
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
#endif
    return rc;
}

static int tr_runner_hw_switching_set_single(enetx_port_t *port, void *_ctx)
{
    unsigned long state = (unsigned long)_ctx;

    if (port->n.port_netdev_role != PORT_NETDEV_ROLE_LAN || !port->dev)
        return 0;
    return runner_port_flooding_set(port->priv, state == HW_SWITCHING_ENABLED);
}

static int runner_hw_switching_state = HW_SWITCHING_DISABLED;

static int runner_hw_switching_set(enetx_port_t *sw, unsigned long state)
{
    int rc;

    rc = port_traverse_ports(root_sw, tr_runner_hw_switching_set_single, PORT_CLASS_PORT, (void *)state);
    if (!rc)
        runner_hw_switching_state = (int)state;
    return rc;
}

static int runner_hw_switching_get(enetx_port_t *sw)
{
    return runner_hw_switching_state;
}

#if defined(CONFIG_BCM_PTP_1588) && !defined(XRDP)
static int port_runner_1588_sw_demux(enetx_port_t *sw, enetx_rx_info_t *rx_info, FkBuff_t *fkb, enetx_port_t **out_port)
{
    uint32_t reason = rx_info->reason;

    if (unlikely(reason == rdpa_cpu_rx_reason_etype_ptp_1588))
        ptp_1588_rx_pkt_store_timestamp(fkb->data, fkb->len, rx_info->ptp_index);
    
    return mux_get_rx_index(sw, rx_info, fkb, out_port);
}
#endif

sw_ops_t port_runner_sw =
{
    .init = port_runner_sw_init,
    .uninit = port_runner_sw_uninit,
#if defined(CONFIG_BCM_PTP_1588) && !defined(XRDP)
    .mux_port_rx = port_runner_1588_sw_demux,
#else
    .mux_port_rx = mux_get_rx_index,
#endif
    .mux_free = mux_index_sw_free,
    .stats_get = port_generic_stats_get,
    .port_id_on_sw = port_runner_sw_port_id_on_sw,
    .hw_sw_state_set = runner_hw_switching_set,
    .hw_sw_state_get = runner_hw_switching_get,
};

int _demux_id_runner_port(enetx_port_t *self)
{
    rdpa_if demuxif = self->p.port_id;

#if defined(CONFIG_BCM_PON_RDP) || defined(CONFIG_BCM_PON_XRDP)
#ifndef CONFIG_BCM_FTTDP_G9991
    /* XXX non-G9991 FW wan gbe src port is lan0 + mac_id, not wan0 */
    if (rdpa_if_is_wan(demuxif))
    {
#ifndef XRDP
        demuxif = rdpa_if_lan0 + self->p.mac->mac_id;

        if (self->p.mac->mac_id == 5)
#endif
            demuxif = rdpa_wan_type_to_if(rdpa_wan_gbe); 
    }
#endif
#endif /* defined(CONFIG_BCM_PON_RDP) || defined(CONFIG_BCM_PON_XRDP) */
    return demuxif;
}

int link_pbit_tc_to_q_to_rdpa_lan_port(bdmf_object_handle port_obj)
{
    bdmf_error_t rc = 0;
    bdmf_object_handle rdpa_tc_to_queue_obj = NULL;
    bdmf_object_handle rdpa_pbit_to_queue_obj = NULL;
    rdpa_tc_to_queue_key_t t2q_key;

    t2q_key.dir = rdpa_dir_ds; 
    t2q_key.table = 0; 

    if ((rdpa_tc_to_queue_get(&t2q_key, &rdpa_tc_to_queue_obj)))
    {
        enet_err("Failed to get tc_to_q object. rc=%d\n", rc);
        goto Exit;
    }
   
    if ((rc = bdmf_link(rdpa_tc_to_queue_obj, port_obj, NULL)))
    {
        enet_err("Failed to link tc_to_q table to RDPA port rc=%d\n", rc);
        goto Exit;
    }
    
    if ((rdpa_pbit_to_queue_get(0, &rdpa_pbit_to_queue_obj)))
    {
        enet_err("Failed to get pbit_to_q object. rc=%d\n", rc);
        goto Exit;
    }
   
    if ((rc = bdmf_link(rdpa_pbit_to_queue_obj, port_obj, NULL)))
    {
        enet_err("Failed to link pbit_to_q table to RDPA port rc=%d\n", rc);
        goto Exit;
    }

Exit:
    if (rdpa_tc_to_queue_obj)
        bdmf_put(rdpa_tc_to_queue_obj);
    if (rdpa_pbit_to_queue_obj)
        bdmf_put(rdpa_pbit_to_queue_obj);

    return rc;
}

int port_runner_port_init(enetx_port_t *self)
{
    bdmf_error_t rc;
    rdpa_if rdpaif = self->p.port_id;
    rdpa_if control_sid = rdpa_if_none;
    rdpa_emac emac = rdpa_emac_none;

    if (mux_set_rx_index(self->p.parent_sw, _demux_id_runner_port(self), self))
        return -1;

    if (self->p.mac)
        emac = rdpa_emac0 + self->p.mac->mac_id;

#ifdef CONFIG_BLOG
    self->n.blog_phy = BLOG_ENETPHY;
    self->n.blog_chnl = self->n.blog_chnl_rx = emac;
#endif

#ifdef CONFIG_BCM_FTTDP_G9991
    /* Not set later when initializing g9991 SW because of FW limitation: must be set when creating LAG port object */
    if (self->p.child_sw)
        control_sid = get_first_high_priority_sid_from_sw(self->p.child_sw);
#endif

    if (!(self->priv = create_rdpa_port(rdpaif, emac, NULL, control_sid)))
    {
        enet_err("Failed to create RDPA port object for %s\n", self->obj_name);
        return -1;
    }

    /* Override bp_parser settings, since once a rdpa port object is created, port role cannot change */
    self->p.port_cap = rdpa_if_is_wan(rdpaif) ? PORT_CAP_WAN_ONLY : PORT_CAP_LAN_ONLY;
    self->n.port_netdev_role = rdpa_if_is_wan(rdpaif) ? PORT_NETDEV_ROLE_WAN : PORT_NETDEV_ROLE_LAN;

    if (rdpa_if_is_wan(rdpaif))
        self->p.ops = &port_runner_port_wan_gbe; /* override ops with correct dispatch_pkt */
#if !defined(CONFIG_BCM_FTTDP_G9991)
    else
    {
        rc = link_pbit_tc_to_q_to_rdpa_lan_port(self->priv);
        if (rc)
            return rc;
    }
#endif

    if (rdpa_if_is_lag_and_switch(rdpaif) && (rc = link_switch_to_rdpa_port(self->priv)))
    {
        enet_err("Failed to link RDPA switch to port object %s. rc =%d\n", self->obj_name, rc);
        return rc;
    }

    enet_dbg("Initialized runner port %s\n", self->obj_name);

    return 0;
}

int port_runner_dispatch_pkt_lan(dispatch_info_t *dispatch_info)
{
    rdpa_cpu_tx_info_t info = {};
#ifdef CONFIG_BCM_PTP_1588
    char *ptp_offset;
#endif

    info.method = rdpa_cpu_tx_port;
    info.port = dispatch_info->port->p.port_id;
    info.cpu_port = rdpa_cpu_host;
    info.x.lan.queue_id = dispatch_info->egress_queue;
    info.drop_precedence = dispatch_info->drop_eligible;
    info.flags = 0;

    enet_dbg_tx("rdpa_cpu_send: port %d queue %d\n", info.port, dispatch_info->egress_queue);

#ifdef CONFIG_BCM_PTP_1588
    if (unlikely(is_pkt_ptp_1588(dispatch_info->pNBuff, &info, &ptp_offset)))
        return ptp_1588_cpu_send_sysb((bdmf_sysb)dispatch_info->pNBuff, &info, ptp_offset);
    else
#endif
    return _rdpa_cpu_send_sysb((bdmf_sysb)dispatch_info->pNBuff, &info);
}

static int dispatch_pkt_gbe_wan(dispatch_info_t *dispatch_info)
{
    rdpa_cpu_tx_info_t info = {};

    info.method = rdpa_cpu_tx_port;
    info.port = dispatch_info->port->p.port_id;
    info.cpu_port = rdpa_cpu_host;
    info.x.wan.queue_id = dispatch_info->egress_queue;
    info.drop_precedence = dispatch_info->drop_eligible;

    enet_dbg_tx("rdpa_cpu_send: port %d queue %d\n", info.port, dispatch_info->egress_queue);

    return _rdpa_cpu_send_sysb((bdmf_sysb)dispatch_info->pNBuff, &info);
}

int enetxapi_post_config(void)
{
#ifdef ENET_RUNNER_WIFI
    if (register_wifi_dev_forwarder())
        return -1;
#endif

    return 0;
}

port_ops_t port_runner_port =
{
    .init = port_runner_port_init,
    .uninit = port_runner_port_uninit,
    .dispatch_pkt = port_runner_dispatch_pkt_lan,
    .stats_get = port_runner_port_stats_get,
    .stats_clear = port_runner_port_stats_clear,
    .pause_get = port_generic_pause_get,
    .pause_set = port_generic_pause_set,
    .mtu_set = port_runner_mtu_set,
    .mib_dump = port_runner_mib_dump,
    .print_status = port_runner_print_status,
    .print_priv = port_runner_print_priv,
    .link_change = port_runner_link_change,
    .mib_dump_us = port_runner_mib_dump_us, // add by Andrew
};

port_ops_t port_runner_port_wan_gbe =
{
    .init = port_runner_port_init,
    .uninit = port_runner_port_uninit,
    .dispatch_pkt = dispatch_pkt_gbe_wan,
    .stats_get = port_runner_port_stats_get,
    .stats_clear = port_runner_port_stats_clear,
    .pause_get = port_generic_pause_get,
    .pause_set = port_generic_pause_set,
    .mtu_set = port_runner_mtu_set,
    .mib_dump = port_runner_mib_dump,
    .print_status = port_runner_print_status,
    .print_priv = port_runner_print_priv,
    .link_change = port_runner_link_change,
    .mib_dump_us = port_runner_mib_dump_us, // add by Andrew
};

