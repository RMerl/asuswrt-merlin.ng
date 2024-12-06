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
#include "rdpa_api.h"
#include "port.h"
#include "mac_drv.h"
#include "mux_index.h"
#include <crossbar_dev.h>
#include "runner.h"
#include "runner_common.h"
#include <spdsvc_defs.h>
#include <linux/kthread.h>

#ifdef SF2_DEVICE
#include "sf2.h"
#include "sf2_common.h"
#endif

#include <bcmnet.h>
#include "bcmenet_common.h"

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
#include "linux/bcm_log.h"
static bcmFun_t *enet_spdsvc_transmit;
#endif

static struct task_struct *house_keeping_thread;

// =========== DSL runner port ops =============================
static int _port_sf2_port_init(enetx_port_t *self)
{
    int create_ingress_filters = 1;
    net_device_handle_t handle = PHYSICAL_PORT_TO_LOGICAL_PORT(self->port_info.port, PORT_ON_ROOT_SW(self)?0:1);

    if (mux_set_rx_index(root_sw, handle, self))
        return -1;

    blog_chnl_unit_port_set(self);

#ifndef XRDP
    create_ingress_filters = 0;
#endif

    if (!(self->priv = create_rdpa_port(self, handle, NULL, 1, 0, create_ingress_filters)))
    {
        enet_err("Failed to create RDPA port object for %s\n", self->obj_name);
        return -1;
    }

#ifdef XRDP
    port_runner_wan_role_set(self, self->port_info.is_wan ? PORT_NETDEV_ROLE_WAN : PORT_NETDEV_ROLE_LAN);
#endif

    enet_dbg("Initialized %s role %s\n", self->obj_name, (self->n.port_netdev_role==PORT_NETDEV_ROLE_WAN)?"wan":"lan" );

    return 0;
}

static int port_runner_port_init(enetx_port_t *self)
{
    // if port is WAN_ONLY port, but role is not set yet, don't register polling timer yet
    if (self->p.port_cap == PORT_CAP_WAN_ONLY && self->n.port_netdev_role != PORT_NETDEV_ROLE_WAN)
        self->p.handle_phy_link_change = 0;

    return _port_sf2_port_init(self);
}

// based on impl5\bcmenet_runner_inline.h:bcmeapi_pkt_xmt_dispatch()
static int dispatch_pkt_gbe_wan(dispatch_info_t *dispatch_info)
{
    rdpa_cpu_tx_extra_info_t extra_info;
    int rc;

    extra_info.u32 = 0; /* Initialize */

    {
#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
        spdsvcHook_transmit_t spdsvc_transmit;

        spdsvc_transmit.pNBuff = dispatch_info->pNBuff;
        spdsvc_transmit.dev = NULL;
        spdsvc_transmit.header_type = SPDSVC_HEADER_TYPE_ETH;
        spdsvc_transmit.phy_overhead = BCM_ENET_OVERHEAD;
        spdsvc_transmit.so_mark = 0;
        spdsvc_transmit.egress_type = SPDSVC_EGRESS_TYPE_ENET;
        spdsvc_transmit.transmit_helper = NULL;

        rc = enet_spdsvc_transmit(&spdsvc_transmit);
        if(rc < 0)
        {
            /* In case of error, NBuff will be free by spdsvc */
            return 0;
        }
        extra_info.is_spdsvc_setup_packet = rc;
        extra_info.so_mark = spdsvc_transmit.so_mark;
#endif
        extra_info.tc_id = dispatch_info->drop_eligible;

#if !defined(CONFIG_BCM963158) /* XXX: Is this code in use for 63146/4912? */
        rc = rdpa_cpu_tx_port_enet_or_dsl_wan((bdmf_sysb)dispatch_info->pNBuff, dispatch_info->egress_queue,
                                              (rdpa_flow)GBE_WAN_FLOW_ID, dispatch_info->port->priv, extra_info);
#else
        {
        rdpa_cpu_tx_info_t info = {};

        info.method = rdpa_cpu_tx_port;
        info.port_obj = dispatch_info->port->priv;
        info.cpu_port = rdpa_cpu_host;
        info.x.wan.queue_id = dispatch_info->egress_queue;
        info.drop_precedence = dispatch_info->drop_eligible;
        info.bits.no_lock = dispatch_info->no_lock;

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
        // on BCM63158 (or XRDP), spdsvc info is part of rdpa_cpu_tx_info_t
        info.bits.is_spdsvc_setup_packet = rc;
        info.spdt_so_mark = spdsvc_transmit.so_mark;
#endif
        rc = rdpa_cpu_send_sysb((bdmf_sysb)dispatch_info->pNBuff, &info);
        }
#endif
        if (rc != 0)
        {
            /* skb is already released by rdpa_cpu_tx_port_enet_or_dsl_wan() */
            INC_STAT_DBG(dispatch_info->port,tx_dropped_accelerator_wan_fail);   /* don't increment tx_dropped, which is incremented by caller */
            return -1;
        }
    }

    return 0;
}

// =========== DSL runner switch ops ===========================

#if defined(RUNNER_PFC_RX) || defined(RUNNER_PFC_TX)
/* 
    We don't have Runner timer implemented for PFC yet at this moment
    add this routine as simple timer to reset PFC timer every second
    if there is no new PFC frame received, to prevent a forever dead lock.
    Bit 31 of timer value is used as our check mark from driver.
*/
static int runner_port_pfc_timer_reset(enetx_port_t *port, void *ctx)
{
    int i, tx_enable = 0;
    int tx_timer[8];

    if (port->port_type != PORT_TYPE_RUNNER_PORT || !port->p.phy) 
        return 0;

    if (!PORT_ROLE_IS_WAN(port) || !port_pfc_capable(port))
        return 1;

    if (port_pfc_get(port, 0, &tx_enable) != 0 || tx_enable == 0)
        return 1;

    for (i=0; i<8; i++) {
        port_pfc_tx_timer_get(port, i, &tx_timer[i]);

        if (tx_timer[i] == 0)
            continue;
        else if ((tx_timer[i] & (1<<31)) == 0)   // If bit 31 is not set, set it as our check mark
        {
            tx_timer[i] |= 1<<31;
            port_pfc_tx_timer_set(port, i, tx_timer[i]);
        }
        else
        {
            // If set alreay, reset timer to zero
            port_pfc_tx_timer_set(port, i, 0);
        }
    }
    return 1;
}

static void find_runner_pfc_reset(void)
{
    port_traverse_ports(root_sw, runner_port_pfc_timer_reset, PORT_CLASS_PORT, 0);
}

static int port_runner_pfc_set(enetx_port_t *port, int pfc_rx_enable, int pfc_tx_enable)
{
    int rc;
    bdmf_object_handle port_obj;

    if (!(port_obj = _port_rdpa_object_by_port(port)))
    {
        enet_err("Failed to get port_obj for %s\n", port->obj_name);
        return -EFAULT;
    }

#if !defined(RUNNER_PFC_RX)
    if (pfc_rx_enable)
    {
        enet_err("PFC Rx is not supported yet for %s.\n", port->obj_name);
        return -EFAULT;
    }
#endif

    rc = rdpa_port_pfc_tx_enable_set(port_obj, pfc_tx_enable);
    rc = mac_dev_pfc_set(port->p.mac, 0, pfc_tx_enable, 0);

    return rc;
}

static int port_runner_pfc_get(enetx_port_t *port, int *pfc_rx_enable, int *pfc_tx_enable)
{
    bdmf_boolean v;
    int ret;

    bdmf_object_handle port_obj;

    if (!(port_obj = _port_rdpa_object_by_port(port)))
    {
        enet_err("failed to get port_obj for %s\n", port->obj_name);
        return -EFAULT;
    }

    ret = rdpa_port_pfc_tx_enable_get(port_obj, &v);
    if (ret == 0)
        *pfc_tx_enable = v;

    return ret;
}

static int port_runner_pfc_tx_timer_get(enetx_port_t *port, int priority, int *pfc_timer)
{
    bdmf_object_handle port_obj;
    bdmf_number pfc_tmr;
    int rc;

    if (!(port_obj = _port_rdpa_object_by_port(port)))
    {
        enet_err("failed to get port_obj for %s\n", port->obj_name);
        return -EFAULT;
    }

    rc = rdpa_port_pfc_tx_timer_get(port_obj, priority, &pfc_tmr);
    *pfc_timer = pfc_tmr;

    return rc;
}

static int port_runner_pfc_tx_timer_set(enetx_port_t *port, int priority, int pfc_timer)
{
    bdmf_object_handle port_obj;
    bdmf_number pfc_tmr = pfc_timer;
    int rc;

    if (!(port_obj = _port_rdpa_object_by_port(port)))
    {
        enet_err("failed to get port_obj for %s\n", port->obj_name);
        return -EFAULT;
    }

    rc = rdpa_port_pfc_tx_timer_set(port_obj, priority, pfc_tmr);

    return rc;
}
#endif  /* defined(RUNNER_PFC_RX) || defined(RUNNER_PFC_TX) */

static int enet_house_keeping_thread(void *self)
{
    while(1) {
#if !defined(CONFIG_BCM963158)
        rdpa_cpu_tx_reclaim();
#endif

#if defined(RUNNER_PFC_TX)
        find_runner_pfc_reset();
#endif
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(HZ);
    }
    enet_err("House Keeping Thread of %s stopped.\n", house_keeping_thread->comm);
    return 0;
}

static int port_runner_with_switch_sw_init(enetx_port_t *self)
{
    int rc;

    if ((rc = port_runner_sw_init(self)))
        return rc;

#if !defined(CONFIG_BCM963158) || defined(RUNNER_PFC_TX)
    house_keeping_thread = kthread_run(enet_house_keeping_thread, self, "%sHouseKeeping", self->obj_name);
#endif
    if (!house_keeping_thread)
        return -1;

    return 0;
}

static int port_runner_with_switch_sw_uninit(enetx_port_t *self)
{
    port_runner_sw_uninit(self);

    if (house_keeping_thread)
        kthread_stop(house_keeping_thread);

    return 0;
}

static int _port_runner_sw_port_id_on_sw(port_info_t *port_info, int *port_id, port_type_t *port_type)
{
#if defined(CONFIG_BCM963158)
#define IS_ETH_WAN(p)   (p==4 || p==5)
static uint32_t runner_wan_map;
    // check multiple ethernet WAN ports are defined
    if (IS_ETH_WAN(port_info->port)) {
        if (runner_wan_map && (runner_wan_map != (1 << port_info->port))) {
            runner_wan_map |= 1 << port_info->port;
            enet_err("Only one ethernet WAN port is supported!! (current map=%x)\n", runner_wan_map);
            BUG();
        } else
            runner_wan_map = 1 << port_info->port;
    }
#endif
    port_info->is_wan = 1;
    return port_runner_sw_port_id_on_sw(port_info, port_id, port_type);
}

/* only root switch does demux */
int port_sf2_mux_get_rx_index(enetx_port_t *sw, enetx_rx_info_t *rx_info, FkBuff_t *fkb, enetx_port_t **out_port)
{
    int rc;

    rc = mux_get_rx_index(sw, rx_info, fkb, out_port);
    if (rc) return rc;

    if (!(*out_port && (*out_port)->port_info.is_wan)) {
#if defined(CONFIG_BCM_KERNEL_BONDING)
        /* Packet received from LAN/SF2-port configured as WAN; No BRCM TAG */
        if (!((*out_port)->p.bond_grp && (*out_port)->p.bond_grp->is_lan_wan_cfg))
#endif
            ((BcmEnet_hdr2*)(fkb->data))->brcm_type = htons(BRCM_TYPE2);
    }
    
    return rc;
}

static int port_runner_role_set(enetx_port_t *port, port_netdev_role_t role)
{
#if defined(CONFIG_BCM_XRDP)
    int rc;

    rc = port_runner_wan_role_set(port, role);
    if (rc)

         return rc;
#endif

    return port_set_wan_role_link(port, role);
}

static void port_extlh_runner_open(enetx_port_t *self)
{ 
    extlh_mac2mac_port_handle(self);
    port_generic_open(self);
}

sw_ops_t port_runner_sw =
{
    .init = port_runner_with_switch_sw_init,
    .uninit = port_runner_with_switch_sw_uninit,
    .mux_port_rx = port_sf2_mux_get_rx_index,
    .stats_get = port_generic_stats_get,
    .port_id_on_sw = _port_runner_sw_port_id_on_sw,
    .config_trunk = port_runner_sw_config_trunk,
};

/* port operations for DSL based runner port */
port_ops_t port_runner_port =
{
    .init = port_runner_port_init,
    .post_init = port_runner_port_post_init,
    .uninit = port_runner_port_uninit,
    .dispatch_pkt = dispatch_pkt_gbe_wan,
    .stats_get = port_runner_port_stats_get,
    .stats_clear = port_runner_port_stats_clear,
    .pause_get = port_generic_pause_get,
    .pause_set = port_generic_pause_set,
#if defined(RUNNER_PFC_RX) || defined(RUNNER_PFC_TX)
    .pfc_get = port_runner_pfc_get,
    .pfc_set = port_runner_pfc_set,
    .pfc_tx_timer_set = port_runner_pfc_tx_timer_set,
    .pfc_tx_timer_get = port_runner_pfc_tx_timer_get,
#endif
    .mtu_set = port_runner_mtu_set,
    .role_set = port_runner_role_set,
    .mib_dump = port_runner_mib_dump,
    .print_status = port_print_status_verbose,
    .print_priv = port_runner_print_priv,
    .open = port_extlh_runner_open,
    .mib_dump_us = port_runner_mib_dump_us, // add by Andrew
};

// =========== sf2 port ops =============================
// based on impl5\bcmenet_runner_inline.h:bcmeapi_pkt_xmt_dispatch()
static int dispatch_pkt_sf2_lan(dispatch_info_t *dispatch_info)
{
    int rc;
    rdpa_cpu_tx_extra_info_t extra_info;

    extra_info.u32 = 0; /* Initialize */

    {
#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
        spdsvcHook_transmit_t spdsvc_transmit = {};

        spdsvc_transmit.pNBuff = dispatch_info->pNBuff;
        spdsvc_transmit.dev = NULL;
        spdsvc_transmit.header_type = SPDSVC_HEADER_TYPE_ETH_BCMTAG;
        spdsvc_transmit.phy_overhead = BCM_ENET_OVERHEAD;
        spdsvc_transmit.egress_type = SPDSVC_EGRESS_TYPE_ENET;
        spdsvc_transmit.transmit_helper = NULL;

        rc = enet_spdsvc_transmit(&spdsvc_transmit);
        if(rc < 0)
        {
            /* In case of error, NBuff will be free by spdsvc */
            return 0;
        }
        extra_info.is_spdsvc_setup_packet = rc;
        extra_info.so_mark = spdsvc_transmit.so_mark;
#endif
        extra_info.lag_port = dispatch_info->lag_port;
        extra_info.tc_id = dispatch_info->drop_eligible;

#if !defined(CONFIG_BCM963158)
        rc = rdpa_cpu_tx_port_enet_lan((bdmf_sysb)dispatch_info->pNBuff, dispatch_info->egress_queue,
                                       dispatch_info->port->priv, extra_info);
#else
        {
        rdpa_cpu_tx_info_t info = {};

        info.method = rdpa_cpu_tx_port;
        info.port_obj = dispatch_info->port->priv;
        info.cpu_port = rdpa_cpu_host;
        info.x.lan.queue_id = dispatch_info->egress_queue;
        info.drop_precedence = dispatch_info->drop_eligible;
        info.lag_index = dispatch_info->lag_port;
        info.bits.no_lock = dispatch_info->no_lock;

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
        // on BCM63158 (or XRDP), spdsvc info is part of rdpa_cpu_tx_info_t
        info.bits.is_spdsvc_setup_packet = rc;
        info.spdt_so_mark = spdsvc_transmit.so_mark;
#endif
        rc = rdpa_cpu_send_sysb((bdmf_sysb)dispatch_info->pNBuff, &info);
        }
#endif
    }
    if (rc != 0)
    {
        /* skb is already released by rdpa_cpu_tx_port_enet_lan() or cpu_queues_tx_send() */
        INC_STAT_DBG(dispatch_info->port,tx_dropped_accelerator_lan_fail);   /* don't increment tx_dropped, which is incremented by caller */
        return -1;
    }

    return 0;
}


static int tr_first_sf2_port(enetx_port_t *port, void *_ctx)
{
    uintptr_t *ptr = _ctx;

    if (!PORT_ON_ROOT_SW(port))
    {
        *ptr = (uintptr_t)port;
        return 1;           // stop scanning
    }
    return 0;
}

static int dispatch_pkt_sf2_lan_imp(dispatch_info_t *dispatch_info)
{
    uintptr_t tx_port = 0;
    dispatch_info_t new_dispatch_info;

    port_traverse_ports(root_sw, tr_first_sf2_port, PORT_CLASS_PORT, &tx_port);

    if (!tx_port) 
    {
        printk("No LAN ports -- drop IMP port (%d) packet\n", dispatch_info->port->port_info.port);
        nbuff_free(dispatch_info->pNBuff);
        return -1;
    }

    memcpy(&new_dispatch_info, dispatch_info, sizeof(new_dispatch_info));
    new_dispatch_info.port = (enetx_port_t *)tx_port;

    return dispatch_pkt_sf2_lan(&new_dispatch_info);
}

// =========== sf2 switch ops ===========================
static int port_sf2_sw_port_id_on_sw(port_info_t *port_info, int *port_id, port_type_t *port_type)
{
    *port_type = PORT_TYPE_SF2_PORT;
    *port_id = port_info->port;

    if (port_info->is_undef)
        *port_type = PORT_TYPE_SF2_MAC;
    return 0;
}

static int port_sf2_runner_sw_config_trunk(enetx_port_t *sw, enetx_port_t *port, int grp_no, int add)
{
    port_sf2_sw_config_trunk(sw, port, grp_no, add);
    return port_runner_sw_config_trunk(sw, port, grp_no, add);
}

static int port_sf2_runner_sw_init(enetx_port_t *self)
{
    port_sf2_sw_init(self);

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
    enet_spdsvc_transmit = bcmFun_get(BCM_FUN_ID_SPDSVC_TRANSMIT);
    BCM_ASSERT(enet_spdsvc_transmit != NULL);
#endif

    return port_runner_sw_init(self);
}

static int port_sf2_runner_sw_uninit(enetx_port_t *self)
{
    port_sf2_sw_uninit(self);
    return port_runner_sw_uninit(self);
}


sw_ops_t port_sf2_sw =
{
    .init = port_sf2_runner_sw_init,
    .uninit = port_sf2_runner_sw_uninit,
//  .mux_port_rx = mux_get_rx_index,    // external switch does not have demux
//  .mux_port_tx = port_sf2_sw_mux,
    .stats_get = port_generic_stats_get,
    .port_id_on_sw = port_sf2_sw_port_id_on_sw,
    .hw_sw_state_set = port_sf2_sw_hw_sw_state_set,
    .hw_sw_state_get = port_sf2_sw_hw_sw_state_get,
    .config_trunk = port_sf2_runner_sw_config_trunk,
    .update_pbvlan = port_sf2_sw_update_pbvlan,
    .rreg = extsw_rreg_wrap,
    .wreg = extsw_wreg_wrap,
    .fast_age = port_sw_fast_age,
};

port_ops_t port_sf2_port =
{
    .init = _port_sf2_port_init,
    .post_init = port_runner_port_post_init,
    .dispatch_pkt = dispatch_pkt_sf2_lan,
    .tx_pkt_mod = port_sf2_tx_pkt_mod,  /* insert brcm tag for port on external switch */
    .rx_pkt_mod = port_sf2_rx_pkt_mod,  /* remove brcm tag for port on external switch */
    .uninit = port_runner_port_uninit,
    .stats_get = port_runner_port_stats_get,
    .stats_clear = port_runner_port_stats_clear,
    .print_priv = port_runner_print_priv,
    .pause_get = port_generic_pause_get,
    .pause_set = port_generic_pause_set,
    .open = port_sf2_generic_open,
    .mtu_set = port_generic_mtu_set,
    .tx_q_remap = port_sf2_tx_q_remap,
#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
    .tx_lb_imp = port_sf2_tx_lb_imp,
#endif
    .mib_dump = port_sw_mib_dump,
    .print_status = port_print_status_verbose,
    .role_set = port_sw_port_role_set,
    .stp_set = port_sw_port_stp_set,
    .fast_age = port_sw_port_fast_age,
#if defined(CONFIG_NET_SWITCHDEV) && (LINUX_VERSION_CODE < KERNEL_VERSION(5,1,0))
    .switchdev_ops = 
    {
        .switchdev_port_attr_get = sf2_switchdev_port_attr_get,
        .switchdev_port_attr_set = sf2_switchdev_port_attr_set, 
    }
#endif
    .mib_dump_us = port_sf2_mib_dump_us, // add by Andrew
};

port_ops_t port_sf2_port_mac =
{
    .stats_get = port_runner_port_stats_get,
    .stats_clear = port_generic_stats_clear,
    .pause_get = port_generic_pause_get,
    .pause_set = port_generic_pause_set,
    .mtu_set = port_generic_mtu_set,
    .mib_dump = port_sw_mib_dump,
    .print_status = port_print_status_verbose,
    .mib_dump_us = port_sf2_mib_dump_us, // add by Andrew
};

port_ops_t port_sf2_port_imp =
{
    .init = _port_sf2_port_init,
    //.uninit = port_runner_port_uninit,
    .dispatch_pkt = dispatch_pkt_sf2_lan_imp,
    .stats_get = port_runner_port_stats_get,
    .stats_clear = port_generic_stats_clear,
    .pause_get = port_generic_pause_get,
    .pause_set = port_generic_pause_set,
    .mtu_set = port_generic_mtu_set,
    .tx_q_remap = port_sf2_tx_q_remap,
    .tx_pkt_mod = port_sf2_tx_pkt_mod,  /* insert brcm tag for port on external switch */
    .rx_pkt_mod = port_sf2_rx_pkt_mod,  /* remove brcm tag for port on external switch */
    .mib_dump = port_sw_mib_dump,
    .print_status = port_print_status_verbose,
    .role_set = port_sw_port_role_set,
    .stp_set = port_sw_port_stp_set,
    .fast_age = port_sw_port_fast_age,
    .mib_dump_us = port_sf2_mib_dump_us, // add by Andrew
};



static int tr_reassign_undef_ports(enetx_port_t *self, void *_ctx)
{
    if (self->p.mac && self->port_info.port == IMP_PORT_ID)
    {
        self->p.ops = &port_sf2_port_imp;
        strncpy(&self->name[0], "bcmswlpbk%d", IFNAMSIZ);
        blog_chnl_unit_port_set(self);
        rtnl_lock();
        enet_create_netdevice(self);
        rtnl_unlock();
        mux_set_rx_index(root_sw, self->p.port_id, self);
        return 1; // no need to traverse further
    }
    return 0;
}

int enetxapi_post_config(void)
{
    port_traverse_ports(sf2_sw, tr_reassign_undef_ports, PORT_CLASS_PORT, NULL);
    return enetxapi_post_sf2_config();
}
