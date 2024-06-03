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
#include "enet_defs.h"
#include <net/genetlink.h>

extern rdpa_system_init_cfg_t init_cfg;
static int gpon_shutdown_done = 0;
static uint32_t gpc_pid = 0; /* gponpwrctl pid */

static const struct genl_multicast_group genl_gpc_mcgrps[] = {
	[0] = { .name = GENL_GPC_MCGRP0_NAME, }
};

int netlink_subscribe(struct sk_buff *skb, struct genl_info* info)
{
    int rc = -1;

    if (info && info->nlhdr->nlmsg_pid != NETLINK_UNSUBSCTIBE_GPC_PID)
    {
        gpc_pid = info->nlhdr->nlmsg_pid;
        rc  = 0;
    }
    else
       gpc_pid = 0;

    return rc;
}

static const struct genl_ops genl_gpc_ops[] = {
    {
        .cmd = GENL_GPC_C_MSG,
        .policy = genl_gpc_policy,
        .doit = netlink_subscribe,
        .dumpit = NULL,
    },
};

static struct genl_family genl_gpc_family = {
    .name = GENL_GPC_FAMILY_NAME,
    .version = 1,
    .maxattr = GENL_GPC_ATTR_MAX,
    .netnsok = false,
    .module = THIS_MODULE,
    .ops = genl_gpc_ops,
    .n_ops = ARRAY_SIZE(genl_gpc_ops),
    .mcgrps = genl_gpc_mcgrps,
    .n_mcgrps = ARRAY_SIZE(genl_gpc_mcgrps),
};

#if !defined(CONFIG_BCM963158) && !defined(CONFIG_BCM96813) && !defined(RDP_UFC)

static struct rtnl_link_stats64 gpon_net_stats, old_gpon_net_stats;

static void port_runner_gpon_stats_clear_sfu(enetx_port_t *self)
{
    rdpa_port_stat_t wan_port_stat = {};
    rdpa_gem_stat_t gem_stat = {};
    bdmf_object_handle gem = NULL;

    rdpa_port_stat_set(self->priv, &wan_port_stat);
    while ((gem = bdmf_get_next(rdpa_gem_drv(), gem, NULL)))
    {
        if (rdpa_gem_stat_set(gem, &gem_stat))
            break;
    }

    if (gem)
        bdmf_put(gem);

    memset(&gpon_net_stats, 0, sizeof(gpon_net_stats));
    memset(&old_gpon_net_stats, 0, sizeof(old_gpon_net_stats));
}

DEFINE_SPINLOCK(stats_lock);

static void port_runner_gpon_stats_sfu(enetx_port_t *self, struct rtnl_link_stats64 *_net_stats)
{
#define CACHE_STATS_ADD(field) \
    do { \
        if (net_stats->field < old_gpon_net_stats.field) \
            gpon_net_stats.field += U32_MAX - old_gpon_net_stats.field + new_gpon_net_stats.field; \
        else \
            gpon_net_stats.field += new_gpon_net_stats.field - old_gpon_net_stats.field; \
    } while (0)

    struct rtnl_link_stats64 new_gpon_net_stats = {}, *net_stats = &new_gpon_net_stats;
    rdpa_gem_stat_t gem_stat;
    bdmf_object_handle gem = NULL;
    rdpa_port_stat_t wan_port_stat;
    int delta;

    spin_lock(&stats_lock);
    if (rdpa_port_stat_get(self->priv, &wan_port_stat))
        goto Exit;

    net_stats->multicast = wan_port_stat.rx_multicast_pkt;
#ifdef CONFIG_BCM_KF_EXTSTATS
    net_stats->rx_broadcast_packets = wan_port_stat.rx_broadcast_pkt;
    net_stats->tx_multicast_packets = wan_port_stat.tx_multicast_pkt;
    net_stats->tx_broadcast_packets = wan_port_stat.tx_broadcast_pkt;
#endif
    net_stats->tx_packets = wan_port_stat.tx_valid_pkt;
    net_stats->rx_packets = wan_port_stat.rx_valid_pkt;

    delta = (net_stats->rx_packets - net_stats->multicast - net_stats->rx_broadcast_packets) - 
        (old_gpon_net_stats.rx_packets - old_gpon_net_stats.multicast - old_gpon_net_stats.rx_broadcast_packets);
    if (delta < 0)
        net_stats->rx_packets -= delta;

    while ((gem = bdmf_get_next(rdpa_gem_drv(), gem, NULL)))
    {
        if (rdpa_gem_stat_get(gem, &gem_stat))
            break;

        net_stats->rx_bytes += gem_stat.rx_bytes;
        net_stats->rx_dropped += gem_stat.rx_packets_discard;
        net_stats->tx_bytes += gem_stat.tx_bytes;
        net_stats->tx_dropped += gem_stat.tx_packets_discard;
    }

    if (gem)
        bdmf_put(gem);

    CACHE_STATS_ADD(multicast);
#ifdef CONFIG_BCM_KF_EXTSTATS
    CACHE_STATS_ADD(rx_broadcast_packets);
    CACHE_STATS_ADD(tx_multicast_packets);
    CACHE_STATS_ADD(tx_broadcast_packets);
#endif
    CACHE_STATS_ADD(rx_bytes);
    CACHE_STATS_ADD(rx_packets);
    CACHE_STATS_ADD(rx_dropped);
    CACHE_STATS_ADD(tx_packets);
    CACHE_STATS_ADD(tx_bytes);
    CACHE_STATS_ADD(tx_dropped);

    *_net_stats = gpon_net_stats;
    old_gpon_net_stats = new_gpon_net_stats;

Exit:
    spin_unlock(&stats_lock);
}

static void port_runner_gpon_stats_clear_hgu(enetx_port_t *self)
{
   rdpa_gem_stat_t gem_stat = {};
   bdmf_object_handle gem = NULL;
   rdpa_iptv_stat_t iptv_stat = {};
   bdmf_object_handle iptv = NULL;
   int rc;

    while ((gem = bdmf_get_next(rdpa_gem_drv(), gem, NULL)))
    {
        rc = rdpa_gem_stat_set(gem, &gem_stat);
        if (rc)
            goto gem_exit;
    }

gem_exit:
    if (gem)
        bdmf_put(gem);

    rc = rdpa_iptv_get(&iptv);
    if (rc)
        goto iptv_exit;

    rc = rdpa_iptv_iptv_stat_set(iptv, &iptv_stat);
    if (rc)
        goto iptv_exit;

iptv_exit:
    if (iptv)
        bdmf_put(iptv);
}

static void port_runner_gpon_stats_hgu(enetx_port_t *self, struct rtnl_link_stats64 *net_stats)
{
   rdpa_gem_stat_t gem_stat;
   bdmf_object_handle gem = NULL;
   rdpa_iptv_stat_t iptv_stat;
   bdmf_object_handle iptv = NULL;
   int rc;

    while ((gem = bdmf_get_next(rdpa_gem_drv(), gem, NULL)))
    {
        rc = rdpa_gem_stat_get(gem, &gem_stat);
        if (rc)
            goto gem_exit;

        net_stats->rx_bytes += gem_stat.rx_bytes;
        net_stats->rx_packets += gem_stat.rx_packets;
        net_stats->rx_dropped += gem_stat.rx_packets_discard;
        net_stats->tx_bytes += gem_stat.tx_bytes;
        net_stats->tx_packets += gem_stat.tx_packets;
        net_stats->tx_dropped += gem_stat.tx_packets_discard;
    }

gem_exit:
    if (gem)
        bdmf_put(gem);

    rc = rdpa_iptv_get(&iptv);
    if (rc)
        goto iptv_exit;

    rc = rdpa_iptv_iptv_stat_get(iptv, &iptv_stat);
    if (rc)
        goto iptv_exit;

    net_stats->multicast = iptv_stat.rx_valid_pkt;
#ifdef CONFIG_BCM_KF_EXTSTATS
    net_stats->rx_multicast_bytes = iptv_stat.rx_valid_bytes;
#endif

iptv_exit:
    if (iptv)
        bdmf_put(iptv);
}

static void port_runner_gpon_stats(enetx_port_t *self, struct rtnl_link_stats64 *_net_stats)
{
    if (init_cfg.operation_mode == rdpa_method_fc)
        port_runner_gpon_stats_hgu(self, _net_stats);
    else
        port_runner_gpon_stats_sfu(self, _net_stats);
}

static void port_runner_gpon_stats_clear(enetx_port_t *self)
{
    if (init_cfg.operation_mode == rdpa_method_fc)
        port_runner_gpon_stats_clear_hgu(self);
    else
        port_runner_gpon_stats_clear_sfu(self);
}

#endif /* !63158 */

static int port_runner_gpon_init(enetx_port_t *self)
{
    BDMF_MATTR_ALLOC(rdpa_port_attrs, rdpa_port_drv());
    rdpa_port_interface_t rdpa_port_interface;
    int rc;

    /* dev name will be configured in post_init stage */
    /*memcpy(rdpa_port_interface.name, self->name, IFNAMSIZ);*/
    rdpa_port_interface.index = 0;
    rdpa_port_interface.is_wan = 1;
    rdpa_port_interface.handle = self->p.port_id;

    self->p.port_cap = PORT_CAP_WAN_ONLY;
    self->n.port_netdev_role = PORT_NETDEV_ROLE_WAN;

    blog_chnl_with_mark_set(BLOG_GPONPHY, self); /* blog_chnl will be set to/from gem */

    if (!self->p.mac || self->p.mac->mac_drv->mac_type != MAC_TYPE_xGPON)
    {
        rc = -1;
        goto exit;
    }
    rdpa_port_interface.type = (self->p.mac->mac_id == NET_PORT_SUBTYPE_GPON) ? rdpa_port_gpon : rdpa_port_xgpon;

    if ((rc = rdpa_port_type_set(rdpa_port_attrs, rdpa_port_interface.type)))
    {
        enet_err("Failed to set RDPA port type %d. rc=%d\n", rdpa_port_interface.type, rc);
        goto exit;
    }

    if ((rc = rdpa_port_index_set(rdpa_port_attrs, rdpa_port_interface.index)))
    {
        enet_err("Failed to set RDPA port index %d. rc=%d\n", rdpa_port_interface.index, rc);
        goto exit;
    }

    if ((rc = rdpa_port_handle_set(rdpa_port_attrs, rdpa_port_interface.handle)))
    {
        enet_err("Failed to set RDPA port index %d. rc=%d\n", rdpa_port_interface.handle, rc);
        goto exit;
    }

    if ((rc = rdpa_port_is_wan_set(rdpa_port_attrs, rdpa_port_interface.is_wan)))
     {
        enet_err("Failed to set RDPA port is_wan %d. rc=%d\n", rdpa_port_interface.is_wan, rc);
        goto exit;
    }

    if (rdpa_port_interface.type == rdpa_port_gpon)
        rdpa_port_speed_set(rdpa_port_attrs,rdpa_speed_2_5g);
    else
        rdpa_port_speed_set(rdpa_port_attrs,rdpa_speed_10g);

    rc = bdmf_new_and_set(rdpa_port_drv(), NULL, rdpa_port_attrs, (bdmf_object_handle *)&self->priv);
    if (rc < 0)
    {
        enet_err("Problem creating gpon wan port object\n");
        goto exit;
    }
#if (defined(CONFIG_BCM_PON_XRDP) || defined(CONFIG_BCM_DSL_XRDP)) && !defined(CONFIG_ONU_TYPE_SFU) 
    {
        rdpa_port_dp_cfg_t port_cfg;

        rdpa_port_cfg_get(*(bdmf_object_handle *)&self->priv, &port_cfg);
        /* XRDP: For WAN port, SAL and DAL should be disabled by default */
        port_cfg.sal_enable = 0;
        port_cfg.dal_enable = 0;
        rdpa_port_cfg_set(*(bdmf_object_handle *)&self->priv, &port_cfg);
    }
#endif

    if (mux_set_rx_index(self->p.parent_sw, self->p.port_id, self))
    {
        rc = -1;
        goto exit;
    }

    if ((rc = runner_default_filter_init(*(bdmf_object_handle *)&self->priv, RDPA_FILTERS_GROUP_WAN)))
    {
        enet_err("Failed to set up default filter for gpon RDPA port. rc=%d\n", rc);
        rc = -1;
        goto exit;
    }

    rc = genl_register_family(&genl_gpc_family);
    if (rc) 
    {
        enet_err("Error creating generic netlink socket for %s family.\n", genl_gpc_family.name);
        rc = -EINVAL;
        goto exit;
    }    

    if ((self->p.mac->mac_id == NET_PORT_SUBTYPE_GPON) ||
        (self->p.mac->mac_drv->mac_type == MAC_TYPE_xGPON))
            gpon_shutdown_done = 0;

    rc = 0;
exit:
    BDMF_MATTR_FREE(rdpa_port_attrs);
    return rc;
}

static int port_runner_gpon_uninit(enetx_port_t *self)
{
    int rc;
    bdmf_object_handle port_obj = self->priv;

    if ((self->p.mac->mac_id == NET_PORT_SUBTYPE_GPON) ||
        (self->p.mac->mac_drv->mac_type == MAC_TYPE_xGPON))
            gpon_shutdown_done = 1;

    mux_set_rx_index(self->p.parent_sw, self->p.port_id, NULL);
    
    /* Unregister the family */
    rc = genl_unregister_family(&genl_gpc_family);
    if(rc)
        enet_err("Failed to unregister %s GENL family: %d\n",genl_gpc_family.name, rc);
    gpc_pid = 0;

    bdmf_destroy(port_obj);
    self->priv = 0;

    return 0;
}

static int netlink_notify_subscriber(void)
{
    void *hdr;
    int res, flags = GFP_ATOMIC;
    char msg[GENL_GPC_ATTR_MSG_MAX] = {0};
    struct sk_buff *skb = genlmsg_new(NLMSG_DEFAULT_SIZE, flags);

    if (!skb)
    {
        enet_err("Failed to alloc. SKB.\n");
        return -1;
    }
    hdr = genlmsg_put(skb, 0, 0, &genl_gpc_family, flags, GENL_GPC_C_MSG);
    if (!hdr)
    {
        enet_err("Unknown error!\n");
        goto nlmsg_fail;
    }

    res = nla_put_string(skb, GENL_GPC_ATTR_MSG, msg);
    if (res)
    {
        enet_err("err %d\n", res);
        goto nlmsg_fail;
    }
    genlmsg_end(skb, hdr);
    if (genlmsg_multicast(&genl_gpc_family, skb, 0, 0, flags) < 0)
    {
        enet_err("Failed to genlmsg_multicast\n");
    }
    return 0;

nlmsg_fail:
    genlmsg_cancel(skb, hdr);
    nlmsg_free(skb);
    return -1;
}

#if defined(CONFIG_BCM_FTTDP_G9991) && defined(XRDP)
extern void __dispatch_pkt_skb_check_bcast_mcast(dispatch_info_t *dispatch_info);
#endif
static int dispatch_pkt_gpon(dispatch_info_t *dispatch_info)
{
    int rc;
    rdpa_cpu_tx_info_t info = {};

    info.method = rdpa_cpu_tx_port;
    info.port_obj = dispatch_info->port->priv;      
    info.cpu_port = rdpa_cpu_host;
    info.x.wan.queue_id = dispatch_info->egress_queue;
    info.x.wan.flow = dispatch_info->channel;
    info.drop_precedence = dispatch_info->drop_eligible;
    info.flags = 0;

    if (dispatch_info->channel == 0)
    {
        nbuff_flushfree(dispatch_info->pNBuff);
        return 0;
    }

#if defined(CONFIG_BCM_FTTDP_G9991) && defined(XRDP)
    if (IS_SKBUFF_PTR(dispatch_info->pNBuff))
        __dispatch_pkt_skb_check_bcast_mcast(dispatch_info);
#endif

    if (unlikely(gpc_pid))
    {
        netlink_notify_subscriber();
    }
    rc = _rdpa_cpu_send_sysb((bdmf_sysb)dispatch_info->pNBuff, &info);
    if (unlikely(rc != 0))
    {
        rdpa_gem_flow_us_cfg_t us_cfg = {};
        bdmf_object_handle gem = NULL;

        if (!rdpa_gem_get(dispatch_info->channel, &gem) && gem)
        {
            rdpa_gem_us_cfg_get(gem, &us_cfg);
            bdmf_put(gem);

            if (!us_cfg.tcont)
            {
                enet_err("can't send sysb - no tcont for gem (%d) \n", dispatch_info->channel);
                return rc;
            }
        }

        enet_err("_rdpa_cpu_send_sysb() rc %d (wan_flow: %d queue_id: %u)\n",
          rc, dispatch_info->channel, dispatch_info->egress_queue);
    }

    return rc;
}

static int port_runner_gpon_post_init(enetx_port_t *self)
{
    self->dev->bcm_nd_ext.path.hw_subport_mcast_idx = NETDEV_PATH_HW_SUBPORTS_MAX;

    return port_runner_port_post_init(self);
}

port_ops_t port_runner_gpon =
{
    .init = port_runner_gpon_init,
    .post_init = port_runner_gpon_post_init,
    .uninit = port_runner_gpon_uninit,
    .dispatch_pkt = dispatch_pkt_gpon,
#if defined(CONFIG_BCM_DSL_XRDP) || defined(RDP_UFC)
    .stats_get = port_generic_sw_stats_get,
#else
    .stats_get = port_runner_gpon_stats,
    .stats_clear = port_runner_gpon_stats_clear,
#endif
    .mtu_set = port_runner_mtu_set,
    /* TODO: stats_clear */
    .mib_dump = port_runner_mib_dump,
    .print_status = port_runner_print_status,
    .print_priv = port_runner_print_priv,
#if (defined(CONFIG_BCM_PON_XRDP) && defined(CONFIG_NET_SWITCHDEV) && (LINUX_VERSION_CODE < KERNEL_VERSION(5,1,0)))
    .switchdev_ops =
    {
        .switchdev_port_attr_get = runner_port_attr_get,
        .switchdev_port_attr_set = runner_port_attr_set,
    },
#endif
    .mib_dump_us = port_runner_mib_dump_us, // add by Andrew
};

int gpon_shutdown_done_by_enet(void)
{
    return gpon_shutdown_done;
}
#ifndef TEST_INGRESS
EXPORT_SYMBOL(gpon_shutdown_done_by_enet);
#endif

static int tr_port_by_gpon(enetx_port_t *port, void *_ctx)
{
    if (!port->port_info.is_gpon)
        return 0;

    *(enetx_port_t **)_ctx = port;
    return 1;
}

int gpon_mcast_gem_set(int mcast_idx)
{
    int net_port = NET_PORT_GPON;
    void *in_out = (void *)(unsigned long)net_port;
    enetx_port_t *port;

    if (mcast_idx >= CONFIG_BCM_MAX_GEM_PORTS)
        return -1;

    if (port_traverse_ports(root_sw, tr_port_by_gpon, PORT_CLASS_PORT, &in_out) <= 0)
        return -1;

    port = (enetx_port_t *)in_out;
    return netdev_path_set_hw_subport_mcast_idx(port->dev, mcast_idx);
}

