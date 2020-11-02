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

#include "g9991.h"
#include "mux_index.h"
#include "port.h"
#include <rdpa_api.h>
#include "enet_dbg.h"
#include "runner_common.h"
#include <bcmnet.h>

#define INBAND_FILTER_ETYPE 0x888A
#define INBAND_FILTER_RDPA_INDEX RDPA_FILTER_ETYPE_UDEF_INDX_MIN

#define PORT_ID_ES rdpa_if_none

static enetx_port_t *dsp_es;

bdmf_object_handle _create_rdpa_port(rdpa_if rdpaif, rdpa_emac emac, bdmf_object_handle owner, rdpa_if high_prio_sid, int create_egress_tm, int enable_set);
extern int link_pbit_tc_to_q_to_rdpa_lan_port(bdmf_object_handle port_obj);

static int tr_g9991_high_prio(enetx_port_t *port, void *_ctx)
{
    rdpa_if *rdpaif = (rdpa_if *)_ctx;

    if (port->p.port_cap == PORT_CAP_MGMT)
    {
        *rdpaif = port->p.port_id;
        return 1;
    }

    return 0;
}

rdpa_if get_first_high_priority_sid_from_sw(enetx_port_t *sw)
{
    rdpa_if rdpaif = rdpa_if_none;

    port_traverse_ports(sw, tr_g9991_high_prio, PORT_CLASS_PORT, (void *)&rdpaif);
    
    return rdpaif;
}

static int _phy_read_status(phy_dev_t *phy_dev)
{
    phy_dev_t *parent_phy_dev = (phy_dev_t *)((unsigned long)phy_dev->priv & ~0x3);
    phy_dev->link = (unsigned long)phy_dev->priv & 0x3;
    phy_dev->speed = PHY_SPEED_1000;

    if (parent_phy_dev)
        phy_dev->link &= parent_phy_dev->link;

    return 0;
}

static phy_drv_t phy_drv_g9991 =
{
    .phy_type = PHY_TYPE_G9991,
    .name = "G9991",
    .read_status = _phy_read_status,
};

static int create_rdpa_g9991_port(enetx_port_t *self, rdpa_if rdpaif, int create_egress_tm)
{
    bdmf_error_t rc = 0;
    bdmf_object_handle port_obj = NULL;
    bdmf_object_handle switch_port_obj = NULL;
    rdpa_port_dp_cfg_t port_cfg = {};
    int physical_port = self->p.parent_sw->s.parent_port->p.mac->mac_id;
    rdpa_filter_ctrl_t ctrl = { .enabled = true, .action = rdpa_forward_action_host, };
#ifdef CONFIG_BCM_XRDP
    rdpa_port_tm_cfg_t port_tm_cfg;
#endif

    if ((rc = rdpa_port_get(rdpa_if_switch, &switch_port_obj)))
    {
        enet_err("Failed to get RDPA switch port object. rc=%d\n", rc);
        goto Exit;
    }

    if (!(self->priv = port_obj = _create_rdpa_port(rdpaif, rdpa_emac0 + physical_port, switch_port_obj, rdpa_if_none, create_egress_tm, 1)))
    {
        enet_err("Failed to create RDPA port %d ret=%d\n", rdpaif, (rc = -1));
        goto Exit;
    }

    if ((rc = rdpa_port_cfg_get(port_obj, &port_cfg)))
    {
        enet_err("Failed to get configuration for RDPA port %d. rc=%d\n", rdpaif, rc);
        goto Exit;
    }

    port_cfg.physical_port = physical_port;

    if ((rc = rdpa_port_cfg_set(port_obj, &port_cfg)))
    {
        enet_err("Failed to set configuration for RDPA port %d. rc=%d\n", rdpaif, rc);
        goto Exit;
    }

    if (create_egress_tm)
    {
        rc = link_pbit_tc_to_q_to_rdpa_lan_port(self->priv);
        if (rc)
            return rc;
    }

#ifdef CONFIG_BCM_XRDP
    /* Configure control SID's with high RX ingress priority */
    if (self->p.port_cap == PORT_CAP_MGMT)
    {
        if (rdpa_port_tm_cfg_get(port_obj, &port_tm_cfg))
        {
            enet_err("rdpa_port_tm_cfg_get failed\n");
            return -EFAULT;
        }

        port_tm_cfg.discard_prty = rdpa_discard_prty_high;
        if (rdpa_port_tm_cfg_set(port_obj, &port_tm_cfg))
        {
            enet_err("rdpa_port_tm_cfg_set failed\n");
            return -EFAULT;
        }
    }
#endif

    rc = rdpa_port_ingress_filter_set(self->priv, RDPA_FILTER_ETYPE_UDEF_0, &ctrl);
    if (rc)
        return rc;

    enet_dbg("Created RDPA g9991 port %d\n", rdpaif);

Exit:
    if (rc)
        enet_err("Failed to create RDPA port object (rc=%d) for %s\n", rc, self->obj_name);

    if (switch_port_obj)
        bdmf_put(switch_port_obj);

    return rc;
}

static int port_g9991_port_init(enetx_port_t *self)
{
    bdmf_error_t rc;
    rdpa_if rdpaif = self->p.port_id;
    void *priv;

    priv = (void *)(unsigned long)self->p.parent_sw->s.parent_port->p.phy;
    if (self->p.port_cap == PORT_CAP_MGMT)
        priv = (void *)((unsigned long)priv | 0x1);

    if (!(self->p.phy = phy_dev_add(PHY_TYPE_G9991, rdpaif - rdpa_if_lan0, priv)))
        return -1;

    self->n.port_netdev_role = PORT_NETDEV_ROLE_LAN;

#ifdef CONFIG_BLOG
    self->n.blog_phy = BLOG_SIDPHY;
    self->n.blog_chnl = self->n.blog_chnl_rx = rdpaif - rdpa_if_lan0;
#endif
    
    if (mux_set_rx_index(root_sw, rdpaif, self))
        return -1;

    if ((rc = create_rdpa_g9991_port(self, rdpaif, 1)))
        return -1;

    return 0;
}

static int _rdpa_destroy_object(bdmf_object_handle *rdpa_obj)
{
    if (*rdpa_obj)
    {
        enet_dbg("destroyed RDPA port %s\n", bdmf_object_name(*rdpa_obj));
        bdmf_destroy(*rdpa_obj);
        *rdpa_obj = NULL;
    }

    return 0;
}

static int port_g9991_port_uninit(enetx_port_t *self)
{
    mux_set_rx_index(root_sw, self->p.port_id, NULL);

    return _rdpa_destroy_object((bdmf_object_handle *)&self->priv);
}

static enetx_port_t *create_port(int port_id, char *devname, char *errorname, enetx_port_t *sw)
{
    enetx_port_t *p;
    port_info_t port_info =
    {
        .port = port_id,
    };

    if (port_create(&port_info, sw, &p))
    {
        enet_err("Failed to create g9991 %s port\n", errorname);
        return NULL;
    }

    p->has_interface = 1;
    strncpy(p->name, devname, IFNAMSIZ);

    return p;
}

static int port_g9991_sw_init(enetx_port_t *self)
{
    bdmf_object_handle filter_obj;
    int rc = 0;

    /* A single error sample port created for the system, shared among DPU's. SID mapping will be set by mux_set_rx_index() in port_init() */
    if (!dsp_es)
    {
        if (!(dsp_es = create_port(PORT_ID_ES, "dsp_es", "error sample", self)))
            return -1;

        if (rdpa_filter_get(&filter_obj))
            return -1;

        if ((rc = rdpa_filter_etype_udef_set(filter_obj, INBAND_FILTER_RDPA_INDEX, INBAND_FILTER_ETYPE)))
        {
            enet_err("Failed to set udef filter %d. rc=%d\n", INBAND_FILTER_RDPA_INDEX, rc);
            goto Exit;
        }

        /* Mapping rdpa_cpu_rx_reason_etype_udef_0 to rdpa_cpu_tc_high will be done from shared
         * _rdpa_reason_set_tc_and_queue() */

        bdmf_put(filter_obj);

        /* Since this block runs only once, reuse it to register phy driver */
        phy_driver_set(&phy_drv_g9991);
    }

    /* RX Pause should be disabled; MAC should not parse G.999.1 frames which might look like pause packets */
#ifdef CONFIG_BCM_XRDP
    /* W/A on LPORT, which does not update pause capability flag; later copied to MAC configuration, to enable TX pause */
    self->s.parent_port->p.phy->pause_tx = 1;
#endif

    port_open(self->s.parent_port);

Exit:
    return rc;
}

static int port_g9991_sw_uninit(enetx_port_t *self)
{
    port_stop(self->s.parent_port);
    return _rdpa_destroy_object((bdmf_object_handle *)&self->priv);
}

#ifdef CONFIG_BCM_XRDP
extern int port_runner_dispatch_pkt_lan(dispatch_info_t *dispatch_info);
static int dispatch_pkt_lan_bridge(dispatch_info_t *dispatch_info)
{
    if (dispatch_info->port->p.port_cap == PORT_CAP_MGMT)
        dispatch_info->drop_eligible = rdpa_discard_prty_high;

    return port_runner_dispatch_pkt_lan(dispatch_info);
}
#else
static int dispatch_pkt_lan_bridge(dispatch_info_t *dispatch_info)
{
    int rc;
    rdpa_cpu_tx_info_t info = {};

    info.method = rdpa_cpu_tx_bridge;
    info.port = rdpa_if_wan0; /* FIXME: MULTI-WAN XPON */
    /* XXX: send to reserved gemflow */
    info.x.wan.flow = RDPA_MAX_GEM_FLOW + dispatch_info->port->p.port_id - rdpa_if_lan0;
    info.x.wan.queue_id = 0; /* XXX: Temp. limitation default flow to queue 0 */
    info.drop_precedence = dispatch_info->drop_eligible;
    enet_dbg_tx("rdpa_cpu_send_sysb: flow %d\n", info.x.wan.flow);

    /* FTTDP FW does not support sending from sysb, so we need to copy to bpm */
    rc = rdpa_cpu_send_raw(bdmf_sysb_data(dispatch_info->pNBuff),
      bdmf_sysb_length(dispatch_info->pNBuff), &info);
    /* rdpa_cpu_send_raw copies to bpm but does not free buffer */
    nbuff_flushfree(dispatch_info->pNBuff);

    return rc;
}
#endif

static void port_g9991_port_stats_clear(enetx_port_t *self)
{
    bdmf_object_handle port_obj = self->priv;
    rdpa_port_stat_t port_stat = {};

    rdpa_port_stat_set(port_obj, &port_stat);
}

static void port_g9991_port_stats_get(enetx_port_t *self, struct rtnl_link_stats64 *net_stats)
{
    bdmf_object_handle port_obj = self->priv;
    rdpa_port_stat_t port_stat;

    rdpa_port_stat_get(port_obj, &port_stat);

    net_stats->multicast = port_stat.rx_multicast_pkt;
    net_stats->rx_bytes = port_stat.rx_valid_bytes;
    net_stats->rx_packets = port_stat.rx_valid_pkt;
    net_stats->tx_bytes = port_stat.tx_valid_bytes;
    net_stats->tx_packets = port_stat.tx_valid_pkt;
#if defined(CONFIG_BCM_KF_EXTSTATS)
    net_stats->tx_multicast_packets = port_stat.tx_multicast_pkt;
    net_stats->rx_broadcast_packets = port_stat.rx_broadcast_pkt;
    net_stats->tx_broadcast_packets = port_stat.tx_broadcast_pkt;
#endif
}

static int port_g9991_sw_port_id_on_sw(port_info_t *port_info, int *port_id, port_type_t *port_type)
{
    if (port_info->port == PORT_ID_ES)
        *port_id = PORT_ID_ES;
    else
        *port_id = rdpa_if_lan0 + port_info->port;

    if (port_info->port == PORT_ID_ES || port_info->is_attached)
    {
        *port_type = PORT_TYPE_G9991_ES_PORT;
    }
    else
    {
        *port_type = PORT_TYPE_G9991_PORT;
    }

    return 0;
}

int port_g9991_mtu_set(enetx_port_t *self, int mtu)
{
    return 0;
}

sw_ops_t port_g9991_sw =
{
    .init = port_g9991_sw_init,
    .uninit = port_g9991_sw_uninit,
    .port_id_on_sw = port_g9991_sw_port_id_on_sw,
};

port_ops_t port_g9991_port =
{
    .init = port_g9991_port_init,
    .uninit = port_g9991_port_uninit,

    .dispatch_pkt = dispatch_pkt_lan_bridge,
    .stats_get = port_g9991_port_stats_get,
    .stats_clear = port_g9991_port_stats_clear,
    .mtu_set = port_g9991_mtu_set,
    .link_change = port_runner_link_change,
};

static int dispatch_pkt_es(dispatch_info_t *dispatch_info)
{
    /* This interface is only for RX */
    nbuff_flushfree(dispatch_info->pNBuff);

    return 0;
}

static int port_g9991_es_port_init(enetx_port_t *self)
{
    rdpa_if rdpaif = self->p.port_id;
    rdpa_ic_result_t def_flow = { .action = rdpa_forward_action_host, .disable_stat = 1, };
    int rc;

    /* skip demux of dummy port */
    if (rdpaif == PORT_ID_ES)
        return 0;
    
    if (create_rdpa_g9991_port(self, rdpaif, 0))
        return -1;

    if ((rc = rdpa_port_def_flow_set(self->priv, &def_flow)))
    {
        enet_err("Failed to set RDPA port default flow for ES port %d. rc=%d\n", rdpaif, rc);
        return -1;
    }

    if (mux_set_rx_index(root_sw, rdpaif, dsp_es))
        return -1;
    
    return 0;
}

static int port_g9991_es_port_uninit(enetx_port_t *self)
{
    rdpa_if rdpaif = self->p.port_id;
    
    if (rdpaif == PORT_ID_ES)
        return 0;

    if (mux_set_rx_index(root_sw, rdpaif, NULL))
        return -1;
    
    return _rdpa_destroy_object((bdmf_object_handle *)&self->priv);
}

port_ops_t port_g9991_es_port =
{
    .init = port_g9991_es_port_init,
    .uninit = port_g9991_es_port_uninit,
    .dispatch_pkt = dispatch_pkt_es,
};

