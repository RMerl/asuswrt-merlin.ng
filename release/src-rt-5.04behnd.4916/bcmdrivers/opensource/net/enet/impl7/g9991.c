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
#include "enet.h"
#include "enet_dbg.h"
#include "runner_common.h"
#include <bcmnet.h>

#define INBAND_FILTER_ETYPE 0x888A
#define INBAND_FILTER_RDPA_INDEX RDPA_FILTER_ETYPE_UDEF_INDX_MIN

#define PORT_ID_ES 0xFF

static enetx_port_t *dsp_es;

static int is_phy_driver_set = 0;

int _rdpa_port_name_set(enetx_port_t *self, char *ovr_name);
int _rdpa_port_def_flow_trap_set(bdmf_object_handle port_obj, int port_id, rdpa_port_type port_type);

extern int link_pbit_tc_to_q_to_rdpa_lan_port(bdmf_object_handle port_obj);

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

static int _set_ingress_priority_high(enetx_port_t *self)
{
    rdpa_port_tm_cfg_t port_tm_cfg;

    if (rdpa_port_tm_cfg_get(self->priv, &port_tm_cfg))
    {
        enet_err("rdpa_port_tm_cfg_get failed\n");
        return -1;
    }

    port_tm_cfg.discard_prty = rdpa_discard_prty_high;
    if (rdpa_port_tm_cfg_set(self->priv, &port_tm_cfg))
    {
        enet_err("rdpa_port_tm_cfg_set failed\n");
        return -1;
    }

    return 0;
}

#define G9991_HANDLE_OFFSET 17

static int create_rdpa_g9991_port(enetx_port_t *self, int create_egress_tm, int create_ingress_filters)
{
    bdmf_error_t rc = 0;
    bdmf_object_handle port_obj = NULL;
    rdpa_filter_ctrl_t ctrl = { .enabled = true, .action = rdpa_forward_action_host, };

    /* Create G9991 port owned by emac */
    port_obj = create_rdpa_port(self, self->p.port_id + G9991_HANDLE_OFFSET, self->p.parent_sw->s.parent_port->priv,
        create_egress_tm, 1, create_ingress_filters);
    if (!port_obj)
    {
        enet_err("Failed to create SID RDPA port %d ret=%d\n", self->p.port_id, (rc = -1));
        goto Exit;
    }
    self->priv = port_obj;

#if defined(CONFIG_BCM_RUNNER_QOS_MAPPER)
    if (create_egress_tm)
    {
        rc = link_pbit_tc_to_q_to_rdpa_lan_port(self->priv);
        if (rc)
            goto Exit;
    }
#endif

    /* Configure control SID's with high RX ingress priority */
    if (self->p.port_cap == PORT_CAP_MGMT)
    {
        if (_set_ingress_priority_high(self))
            goto Exit;
    }

    rc = rdpa_port_ingress_filter_set(self->priv, RDPA_FILTER_ETYPE_UDEF_0, &ctrl);
    if (rc)
        goto Exit;

    rc = mux_set_rx_index(root_sw, self->p.port_id + G9991_HANDLE_OFFSET, self);
    if (rc)
        goto Exit;

    enet_dbg("Created RDPA g9991 port %d\n", self->p.port_id);

Exit:
    if (rc)
        enet_err("Failed to create RDPA port object (rc=%d) for %s\n", rc, self->obj_name);

    return rc;
}

static int port_g9991_port_init(enetx_port_t *self)
{
    bdmf_error_t rc;
    void *priv;

    priv = (void *)(unsigned long)self->p.parent_sw->s.parent_port->p.phy;
    if (self->p.port_cap == PORT_CAP_MGMT)
        priv = (void *)((unsigned long)priv | 0x1);
    else
        self->p.port_cap = PORT_CAP_LAN_ONLY;

    if (!(self->p.phy = phy_dev_add(PHY_TYPE_G9991, self->p.port_id, priv)))
        return -1;

    self->n.port_netdev_role = PORT_NETDEV_ROLE_LAN;

    blog_chnl_unit_port_set(self);

    if ((rc = create_rdpa_g9991_port(self, 1, 1)))
        return -1;

    if (!strlen(self->name))
        snprintf(self->name, IFNAMSIZ, "sid%d", self->p.port_id);

    return 0;
}

static int port_g9991_port_post_init(enetx_port_t *self)
{
    return _rdpa_port_name_set(self, NULL);
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

    blog_chnl_unit_port_unset(self);
    return _rdpa_destroy_object((bdmf_object_handle *)&self->priv);
}

static enetx_port_t *__create_port(int port_id, char *devname, char *errorname, enetx_port_t *sw)
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
    if (devname)
        strncpy(p->name, devname, IFNAMSIZ);

    return p;
}

static int _create_error_sampling_port(enetx_port_t *self)
{
    /* A single error sample port created for the system, shared among DPU's. SID mapping will be set by
     * mux_set_rx_index() in port_init() */
    if (dsp_es)
        return 0;

    if (!(dsp_es = __create_port(PORT_ID_ES, "dsp_es", "error sample", self)))
        return -1;

    return 0;
}


static int port_g9991_sw_init(enetx_port_t *self)
{
    if (_create_error_sampling_port(self))
        return -1;

    if (!is_phy_driver_set)
    {
        phy_driver_set(&phy_drv_g9991);
        is_phy_driver_set = 1;
    }

    /* RX Pause should be disabled; MAC should not parse G.999.1 frames which might look like pause packets
     * W/A on LPORT, which does not update pause capability flag; later copied to MAC configuration,
     * to enable TX pause */
    self->s.parent_port->p.phy->pause_tx = 1;

    port_open(self->s.parent_port);

    return 0;
}

static int port_g9991_sw_uninit(enetx_port_t *self)
{
    port_stop(self->s.parent_port);
    return _rdpa_destroy_object((bdmf_object_handle *)&self->priv);
}

extern int port_runner_dispatch_pkt_lan(dispatch_info_t *dispatch_info);
static int dispatch_pkt_lan_bridge(dispatch_info_t *dispatch_info)
{
    if (dispatch_info->port->p.port_cap == PORT_CAP_MGMT)
        dispatch_info->drop_eligible = rdpa_discard_prty_high;

    return port_runner_dispatch_pkt_lan(dispatch_info);
}

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
        *port_id = port_info->port;

    if (port_info->port == PORT_ID_ES || port_info->is_attached)
    {
        *port_type = PORT_TYPE_G9991_ES_PORT;
    }
    else if (port_info->is_management)
    {
        *port_type = PORT_TYPE_G9991_CTRL_PORT; /* Control SID */
    }
    else
    {
        *port_type = PORT_TYPE_G9991_PORT; /* SID */
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
    .post_init = port_g9991_port_post_init,
    .uninit = port_g9991_port_uninit,

    .dispatch_pkt = dispatch_pkt_lan_bridge,
    .stats_get = port_g9991_port_stats_get,
    .stats_clear = port_g9991_port_stats_clear,
    .mtu_set = port_g9991_mtu_set,
    .link_change = port_runner_link_change,
#if defined(CONFIG_NET_SWITCHDEV) && (LINUX_VERSION_CODE < KERNEL_VERSION(5,1,0))
    .switchdev_ops =
    {
        .switchdev_port_attr_get = runner_port_attr_get,
        .switchdev_port_attr_set = runner_port_attr_set,
    }
#endif
};

static int dispatch_pkt_es(dispatch_info_t *dispatch_info)
{
    /* This interface is only for RX */
    nbuff_flushfree(dispatch_info->pNBuff);

    return 0;
}

static int port_g9991_es_port_init(enetx_port_t *self)
{
    bdmf_object_handle filter_obj = NULL;
    int rc = -1;

    if (self->p.port_id == PORT_ID_ES)
        return 0;

    rc = create_rdpa_g9991_port(self, 0, 0);
    if (rc)
        goto exit;

    rc = rdpa_filter_get(&filter_obj);
    if (rc)
        goto exit;

    /* Set the etype udef0 filter. Mapping rdpa_cpu_rx_reason_etype_udef_0 to rdpa_cpu_tc_high will be done from shared
     * _rdpa_reason_set_tc_and_queue() */
    rc = rdpa_filter_etype_udef_set(filter_obj, INBAND_FILTER_RDPA_INDEX, INBAND_FILTER_ETYPE);
    if (rc)
    {
        enet_err("Failed to set udef filter %d. rc=%d\n", INBAND_FILTER_RDPA_INDEX, rc);
        goto exit;
    }

    rc = _rdpa_port_def_flow_trap_set(self->priv, self->p.port_id, port_runner_port_type_mapping(self->port_type));
    if (rc)
        goto exit;

exit:
    if (filter_obj)
        bdmf_put(filter_obj);

    return rc;
}

static int port_g9991_es_port_post_init(enetx_port_t *self)
{
    char name[IFNAMSIZ];

    if (self->p.port_id == PORT_ID_ES)
        return 0;

    snprintf(name, IFNAMSIZ, "dsp_es%d", self->p.port_id);
    return _rdpa_port_name_set(self, name);
}

static int port_g9991_es_port_uninit(enetx_port_t *self)
{
    if (self->p.port_id == PORT_ID_ES)
        return 0;

    if (mux_set_rx_index(root_sw, self->p.port_id, NULL))
        return -1;

    return _rdpa_destroy_object((bdmf_object_handle *)&self->priv);
}

port_ops_t port_g9991_es_port =
{
    .init = port_g9991_es_port_init,
    .post_init = port_g9991_es_port_post_init,
    .uninit = port_g9991_es_port_uninit,
    .dispatch_pkt = dispatch_pkt_es,
};

