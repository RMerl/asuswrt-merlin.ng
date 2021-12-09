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
static int port_sf2_port_init(enetx_port_t *self)
{
    bdmf_error_t rc;
    rdpa_if rdpaif = self->p.port_id;
    rdpa_emac emac = rdpa_emac_none;
    bdmf_object_handle switch_port_obj = NULL;

    if (mux_set_rx_index(self->p.parent_sw, self->p.port_id, self))
        return -1;

    /* also register demux at root for receive processing if port not on root sw */
    if (!PORT_ON_ROOT_SW(self))
        if (mux_set_rx_index(root_sw, self->p.port_id, self))
            return -1;

#ifdef CONFIG_BLOG
    self->n.blog_phy = BLOG_ENETPHY;
    self->n.blog_chnl = self->n.blog_chnl_rx = PHYSICAL_PORT_TO_LOGICAL_PORT(self->p.mac->mac_id, PORT_ON_ROOT_SW(self)?0:1);
    enet_dbgv("%s blog_chnl=%x\n", self->obj_name, self->n.blog_chnl);
#endif
    /* create_rdpa_port only for port on external switch */
    if (PORT_ON_ROOT_SW(self))
    {
        if (rdpa_if_is_wan(rdpaif))
            emac = rdpa_emac0 + self->p.mac->mac_id;
        else
            goto PORT_INIT_CONT;
    }
    else
    {
        /* get the rdpa switch port in order to configure as owner to extswitch lan ports */
        rc = rdpa_port_get(rdpa_if_switch, &switch_port_obj);
        if (rc)
        {
            enet_err("Failed to get rdpa switch port. rc=%d\n", rc);
            return -1;
        }
    }

    if (!(self->priv = create_rdpa_port(rdpaif, emac, switch_port_obj, rdpa_if_none)))
    {
        enet_err("Failed to create RDPA port object for %s\n", self->obj_name);
        return -1;
    }

    if (rdpa_if_is_lag_and_switch(rdpaif) && (rc = link_switch_to_rdpa_port(self->priv)))
    {
        enet_err("Failed to link RDPA switch to port object %s. rc =%d\n", self->obj_name, rc);
        return rc;
    }

PORT_INIT_CONT:
    enet_dbg("Initialized %s role %s\n", self->obj_name, (self->n.port_netdev_role==PORT_NETDEV_ROLE_WAN)?"wan":"lan" );

    return 0;
}

int _demux_id_runner_port(enetx_port_t *self)
{
    return self->p.port_id; // rdpa_if
}

static int port_runner_port_init(enetx_port_t *self)
{
    // if port is WAN_ONLY port, but role is not set yet, don't register polling timer yet
    if (self->p.port_cap == PORT_CAP_WAN_ONLY && self->n.port_netdev_role != PORT_NETDEV_ROLE_WAN)
        self->p.handle_phy_link_change = 0;

    return port_sf2_port_init(self);
}

extern int speed_macro_2_mbps(phy_speed_t spd);
static int port_set_wan_role_link(enetx_port_t *port, port_netdev_role_t role)
{
    phy_dev_t *end_phy = get_active_phy(port->p.phy);

    if (end_phy)
        end_phy = cascade_phy_get_last(end_phy);

    if (role == PORT_NETDEV_ROLE_WAN)
    {
        /* Start PHY polling timer */
        if (port->p.port_cap == PORT_CAP_WAN_ONLY)
		{
			port->p.handle_phy_link_change = 1;
            phy_register_polling_timer(port->p.phy, dslbase_phy_link_change_cb);
            /* Force status to link down to trigger link up event */
            port->p.phy->link = 0;

            if(end_phy)
                end_phy->link = 0;

            phy_dev_link_change_notify(port->p.phy);
        }
    }
    else
    {
        /* Stop PHY polling timer */
        if (port->p.port_cap == PORT_CAP_WAN_ONLY)
        {
            port->p.handle_phy_link_change = 0;
            phy_unregister_polling_timer(port->p.phy);

            /* Force port link down if physical is up */
            if(end_phy && end_phy->link) {
                end_phy->link = 0;
                phy_dev_status_propagate(end_phy);
                dslbase_phy_link_change_cb(end_phy);
            }
        }
    }

    return 0;
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

        rc = enet_spdsvc_transmit(&spdsvc_transmit);
        if(rc < 0)
        {
            /* In case of error, NBuff will be free by spdsvc */
            return 0;
        }
        extra_info.is_spdsvc_setup_packet = rc;
#endif
        extra_info.tc_id = dispatch_info->drop_eligible;

#if !defined(CONFIG_BCM963158)
        rc = rdpa_cpu_tx_port_enet_or_dsl_wan((bdmf_sysb)dispatch_info->pNBuff, dispatch_info->egress_queue,
                                              (rdpa_flow)GBE_WAN_FLOW_ID, dispatch_info->port->p.port_id, extra_info);
#else
        {
        rdpa_cpu_tx_info_t info = {};

        info.method = rdpa_cpu_tx_port;
        info.port = dispatch_info->port->p.port_id;
        info.cpu_port = rdpa_cpu_host;
        info.x.wan.queue_id = dispatch_info->egress_queue;
        info.drop_precedence = dispatch_info->drop_eligible;
        info.bits.no_lock = dispatch_info->no_lock;

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
        // on BCM63158 (or XRDP), spdsvc info is part of rdpa_cpu_tx_info_t
        info.bits.is_spdsvc_setup_packet = rc;
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

int port_runner_sw_config_trunk(enetx_port_t *sw, enetx_port_t *port, int grp_no, int add)
{
    /* based on impl5\bcmsw_runner.c:bcm_enet_rdp_config_bond() */
    rdpa_if rdpa_port = port->p.port_id;
    rdpa_if rdpa_bond = rdpa_if_bond0 + grp_no;
    bdmf_object_handle port_obj = NULL;
    bdmf_object_handle bond_obj = NULL;
    int rc;

    if ( rdpa_port == rdpa_if_none )
    {
        enet_err("Invalid rdpa port for %s\n\n", port->obj_name);
        return -1;
    }

    if ( rdpa_bond > rdpa_if_bond_max )
    {
        enet_err("Invalid rdpa bond %d for grp_no=%d %s\n\n", rdpa_bond, grp_no, port->obj_name);
        return -1;
    }

    rc = rdpa_port_get(rdpa_port, &port_obj);
    if (rc)
    {
        enet_err("NO rdpa port for rdpa_if %d %s\n\n", rdpa_port, port->obj_name);
        return -1;
    }

    /* get the rdpa bond port in order to link to lan ports */
    rc = rdpa_port_get(rdpa_bond, &bond_obj);
    if (rc)
    {
        if (add)
        {
            /* Bond object does not exist - Create one */
            BDMF_MATTR(rdpa_port_attrs, rdpa_port_drv());
            rdpa_port_index_set(rdpa_port_attrs, rdpa_bond);
            rc = bdmf_new_and_set(rdpa_port_drv(), NULL, rdpa_port_attrs, &bond_obj);
            if (rc)
            {
                enet_err("Failed to create bond port rc(%d)\n",  rc);
                goto error_exit;
            }
        }
        else
        {
            enet_err("No rdpa bond %d for grp_no=%d %s\n\n", rdpa_bond, grp_no, port->obj_name);
            goto error_exit;
        }
    }

    if (add)
    {
        /* Link the port with bond object */
        rc = bdmf_link(bond_obj, port_obj, NULL);
        if (rc)
        {
            enet_err("Failed to link bond port %d for grp_no=%d %s\n", rdpa_bond, grp_no, port->obj_name);
            goto error_exit;
        }
    }
    else
    {
        /* UnLink the port from bond object */
        rc = bdmf_unlink(bond_obj, port_obj);
        if (rc)
        {
            enet_err("Failed to unlink bond port %d for grp_no=%d %s\n", rdpa_bond, grp_no, port->obj_name);
            goto error_exit;
        }

        if (bdmf_get_next_us_link(bond_obj, NULL) == NULL)
        {
            /* No More linked objects to this bond object - destroy */
            if ( bdmf_destroy(bond_obj) )
            {
                enet_err("Failed to destroy bond port %d for grp_no=%d %s\n", rdpa_bond, grp_no, port->obj_name);
            }
            else
            {
                bond_obj = NULL;
            }
        }
    }

error_exit:
    if (port_obj)
    {
        bdmf_put(port_obj);
    }
    if (bond_obj)
    {
        bdmf_put(bond_obj);
    }
    return rc;
}

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

/* only root switch does demux */
int port_sf2_mux_get_rx_index(enetx_port_t *sw, enetx_rx_info_t *rx_info, FkBuff_t *fkb, enetx_port_t **out_port)
{
    if (!rdpa_if_is_wan(rx_info->src_port))
    {
        int ret = mux_get_rx_index(sf2_sw, rx_info, fkb, out_port);
        
#if defined(CONFIG_BCM_KERNEL_BONDING)
        /* Packet received from LAN/SF2-port configured as WAN; No BRCM TAG */
        if (!(*out_port && (*out_port)->p.bond_grp && (*out_port)->p.bond_grp->is_lan_wan_cfg))
#endif
            ((BcmEnet_hdr2*)(fkb->data))->brcm_type = htons(BRCM_TYPE2);

        return ret;
    }
    return mux_get_rx_index(sw, rx_info, fkb, out_port);
}

sw_ops_t port_runner_sw =
{
    .init = port_runner_with_switch_sw_init,
    .uninit = port_runner_with_switch_sw_uninit,
    .mux_port_rx = port_sf2_mux_get_rx_index,
    .mux_free = mux_index_sw_free,
    .stats_get = port_generic_stats_get,
    .port_id_on_sw = port_runner_sw_port_id_on_sw,
    .config_trunk = port_runner_sw_config_trunk,
};

/* port operations for DSL based runner port */
port_ops_t port_runner_port =
{
    .init = port_runner_port_init,
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
    .role_set = port_set_wan_role_link,
    .mib_dump = port_runner_mib_dump,
    .print_status = port_sf2_print_status,
    .print_priv = port_runner_print_priv,
    .mib_dump_us = port_runner_mib_dump_us, // add by Andrew
};

// =========== sf2 port ops =============================
static void port_sf2_port_open(enetx_port_t *self)
{
    PORT_SET_EXT_SW(self);
    // port is on external switch, also enable connected runner port
    port_open(sf2_sw->s.parent_port);
    port_generic_open(self);
}

// based on impl5\bcmenet_runner_inline.h:bcmeapi_pkt_xmt_dispatch()
static int dispatch_pkt_sf2_lan(dispatch_info_t *dispatch_info)
{
    int rc;
    rdpa_cpu_tx_extra_info_t extra_info;

    extra_info.u32 = 0; /* Initialize */

    {
#if !defined(CONFIG_BCM963158)
        uint32_t phys_port = dispatch_info->port->p.mac->mac_id;
#endif
#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
        spdsvcHook_transmit_t spdsvc_transmit;

        spdsvc_transmit.pNBuff = dispatch_info->pNBuff;
        spdsvc_transmit.dev = NULL;
        spdsvc_transmit.header_type = SPDSVC_HEADER_TYPE_ETH_BCMTAG;
        spdsvc_transmit.phy_overhead = BCM_ENET_OVERHEAD;

        rc = enet_spdsvc_transmit(&spdsvc_transmit);
        if(rc < 0)
        {
            /* In case of error, NBuff will be free by spdsvc */
            return 0;
        }
        extra_info.is_spdsvc_setup_packet = rc;
#endif
        extra_info.lag_port = dispatch_info->lag_port;
        extra_info.tc_id = dispatch_info->drop_eligible;

#if !defined(CONFIG_BCM963158)
        rc = rdpa_cpu_tx_port_enet_lan((bdmf_sysb)dispatch_info->pNBuff, dispatch_info->egress_queue,
                                       phys_port, extra_info);
#else
        {
        rdpa_cpu_tx_info_t info = {};

        info.method = rdpa_cpu_tx_port;
        info.port = dispatch_info->port->p.port_id;
        info.cpu_port = rdpa_cpu_host;
        info.x.lan.queue_id = dispatch_info->egress_queue;
        info.drop_precedence = dispatch_info->drop_eligible;
        info.lag_index = dispatch_info->lag_port;
        info.bits.no_lock = dispatch_info->no_lock;

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
        // on BCM63158 (or XRDP), spdsvc info is part of rdpa_cpu_tx_info_t
        info.bits.is_spdsvc_setup_packet = rc;
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
        printk("No LAN ports -- drop IMP port (%d) packet\n", dispatch_info->port->p.mac->mac_id);
        nbuff_free(dispatch_info->pNBuff);
        return -1;
    }

    memcpy(&new_dispatch_info, dispatch_info, sizeof(new_dispatch_info));
    new_dispatch_info.port = (enetx_port_t *)tx_port;

    return dispatch_pkt_sf2_lan(&new_dispatch_info);
}

// =========== sf2 switch ops ===========================
/* map SF2 external switch phyical port ID to rdpa_if */
static int port_sf2_sw_port_id_on_sw(port_info_t *port_info, int *port_id, port_type_t *port_type)
{
    *port_type = PORT_TYPE_SF2_PORT;
    *port_id = rdpa_if_lan0 + port_info->port;

    if (port_info->is_undef)
        *port_type = PORT_TYPE_SF2_MAC;
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
    if (*port_id == rdpa_if_lan7)
        *port_id = rdpa_if_lan6;
#elif defined(CONFIG_BCM94908)
    if (*port_id == rdpa_if_lan7)
        *port_id = rdpa_if_lan4;
#elif defined(CONFIG_BCM963158)
    if (*port_id == rdpa_if_lan6)
        *port_id = rdpa_if_lan5;
#endif
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
    .fast_age = port_sf2_sw_fast_age,
};

port_ops_t port_sf2_port =
{
    .init = port_sf2_port_init,
    .dispatch_pkt = dispatch_pkt_sf2_lan,
    .stats_clear = port_generic_stats_clear,
    .tx_pkt_mod = port_sf2_tx_pkt_mod,  /* insert brcm tag for port on external switch */
    .rx_pkt_mod = port_sf2_rx_pkt_mod,  /* remove brcm tag for port on external switch */
    .uninit = port_runner_port_uninit,
    .stats_get = port_runner_port_stats_get,
    .print_priv = port_runner_print_priv,
    .pause_get = port_generic_pause_get,
    .pause_set = port_generic_pause_set,
    .open = port_sf2_port_open,
    .mtu_set = port_generic_mtu_set,
    .tx_q_remap = port_sf2_tx_q_remap,
#if defined(CONFIG_BCM_ENET_MULTI_IMP_SUPPORT)
    .tx_lb_imp = port_sf2_tx_lb_imp,
#endif
    .mib_dump = port_sf2_mib_dump,
    .print_status = port_sf2_print_status,
    .role_set = port_sf2_port_role_set,
    .stp_set = port_sf2_port_stp_set,
    .fast_age = port_sf2_fast_age,
#if defined(CONFIG_NET_SWITCHDEV)
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
    .mib_dump = port_sf2_mib_dump,
    .print_status = port_sf2_print_status,
    .mib_dump_us = port_sf2_mib_dump_us, // add by Andrew
};

port_ops_t port_sf2_port_imp =
{
    .init = port_sf2_port_init,
    //.uninit = port_runner_port_uninit,
    .dispatch_pkt = dispatch_pkt_sf2_lan_imp,
    .stats_get = port_runner_port_stats_get,
    .stats_clear = port_generic_stats_clear,
    .pause_get = port_generic_pause_get,
    .pause_set = port_generic_pause_set,
    //.open = port_sf2_port_open,
    .mtu_set = port_generic_mtu_set,
    .tx_q_remap = port_sf2_tx_q_remap,
    .tx_pkt_mod = port_sf2_tx_pkt_mod,  /* insert brcm tag for port on external switch */
    .rx_pkt_mod = port_sf2_rx_pkt_mod,  /* remove brcm tag for port on external switch */
    .mib_dump = port_sf2_mib_dump,
    .print_status = port_sf2_print_status,
    .role_set = port_sf2_port_role_set,
    .stp_set = port_sf2_port_stp_set,
    .fast_age = port_sf2_fast_age,
    .mib_dump_us = port_sf2_mib_dump_us, // add by Andrew
};



static int tr_reassign_undef_ports(enetx_port_t *self, void *_ctx)
{
    if (self->p.mac && self->p.mac->mac_id == IMP_PORT_ID)
    {
        self->p.ops = &port_sf2_port_imp;
        strncpy(&self->name[0], "bcmswlpbk%d", IFNAMSIZ);
        self->p.port_id = rdpa_if_switch;
#ifdef CONFIG_BLOG
        self->n.blog_phy = BLOG_ENETPHY;
        self->n.blog_chnl = self->n.blog_chnl_rx = PHYSICAL_PORT_TO_LOGICAL_PORT(self->p.mac->mac_id, PORT_ON_ROOT_SW(self)?0:1);
        enet_dbgv("%s blog_chnl=%x\n", self->obj_name, self->n.blog_chnl);
#endif
        rtnl_lock();
        enet_create_netdevice(self);
        rtnl_unlock();
        mux_set_rx_index(self->p.parent_sw, self->p.port_id, self);
        /* also register demux at root for receive processing if port not on root sw */
        if (!PORT_ON_ROOT_SW(self))
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
