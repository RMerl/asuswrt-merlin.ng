/*
   <:copyright-BRCM:2018:DUAL/GPL:standard

      Copyright (c) 2018 Broadcom
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
 *      Author: steven.hsieh@broadcom.com
 */

#include "bcmenet_common.h"
#include "syspvsw.h"
#include "mux_index.h"

#include "sf2.h"
#include "sf2_common.h"

#include "archer_enet.h"

// =========== sysp port ops =============================

// same as dispatch_pkt_sf2_lan() for now
static int dispatch_pkt_sysp_lan(dispatch_info_t *dispatch_info)
{
    int rc;
    /* TODO : hardcode the dispatch is through LAN interface for now */
    rc = cpu_queues_tx_send (LAN_CPU_TX, dispatch_info);

    if (rc < 0)
    {
        /* skb is already released by rdpa_cpu_tx_port_enet_lan() or cpu_queues_tx_send() */
        INC_STAT_DBG(dispatch_info->port,tx_dropped_accelerator_lan_fail);   /* don't increment tx_dropped, which is incremented by caller */
        return -1;
    }

    return 0;
}


// =========== sysp switch ops ===========================
static int port_sysp_sw_port_id_on_sw(port_info_t *port_info, int *port_id, port_type_t *port_type)
{
    *port_type = (port_info->is_undef) ? PORT_TYPE_SYSP_MAC : PORT_TYPE_SYSP_PORT;
    *port_id = port_info->port;

    return 0;
}

int sw_config_trunk_chnl_rx(enetx_port_t *sw, enetx_port_t *port, int grp_no, int add);

sw_ops_t port_sysp_sw =
{
    .init = port_sysp_sw_init,
    .uninit = port_sysp_sw_uninit,
    .mux_port_rx = mux_get_rx_index,    // external switch does not have demux
    .mux_free = mux_index_sw_free,
    .stats_get = port_generic_stats_get,
    .port_id_on_sw = port_sysp_sw_port_id_on_sw,
    .config_trunk = sw_config_trunk_chnl_rx,
};

port_ops_t port_sysp_port =
{
    .init = port_sysp_port_init,
    .dispatch_pkt = dispatch_pkt_sysp_lan,
    .stats_clear = port_generic_stats_clear,
    .stats_get = port_generic_stats_get,
    .pause_get = port_generic_pause_get,
    .pause_set = port_generic_pause_set,
    .open = port_sf2_generic_open,
    .mtu_set = port_sysp_mtu_set,
    .mib_dump = port_sysp_mib_dump,
    .print_status = port_print_status_verbose,
    .role_set = port_sysp_port_role_set,
    .mib_dump_us = port_sysp_mib_dump_us, // add by Andrew
};

// =========== sf2 port ops =============================

// based on impl5\bcmenet_runner_inline.h:bcmeapi_pkt_xmt_dispatch()
static int dispatch_pkt_sf2_lan(dispatch_info_t *dispatch_info)
{
    int rc;
    /* TODO : hardcode the dispatch is through LAN interface for now */
    rc = cpu_queues_tx_send (LAN_CPU_TX, dispatch_info);

    if (rc < 0)
    {
        /* skb is already released by rdpa_cpu_tx_port_enet_lan() or cpu_queues_tx_send() */
        INC_STAT_DBG(dispatch_info->port,tx_dropped_accelerator_lan_fail);   /* don't increment tx_dropped, which is incremented by caller */
        return -1;
    }

    return 0;
}

// =========== sf2 switch ops ===========================
/* map SF2 external switch phyical port ID to rdpa_if */
static int port_sf2_sw_port_id_on_sw(port_info_t *port_info, int *port_id, port_type_t *port_type)
{
    *port_type = PORT_TYPE_SF2_PORT;
    *port_id = PHYSICAL_PORT_TO_LOGICAL_PORT(port_info->port, 1);

    if (port_info->is_undef)
        *port_type = PORT_TYPE_SF2_MAC;

    return 0;
}


sw_ops_t port_sf2_sw =
{
    .init = port_sf2_sw_init,
    .uninit = port_sf2_sw_uninit,
    .mux_port_rx = mux_get_rx_index,    // external switch does not have demux
//  .mux_port_tx = port_sf2_sw_mux,
    .mux_free = mux_index_sw_free,
    .stats_get = port_generic_stats_get,
    .port_id_on_sw = port_sf2_sw_port_id_on_sw,
    .hw_sw_state_set = port_sf2_sw_hw_sw_state_set,
    .hw_sw_state_get = port_sf2_sw_hw_sw_state_get,
    .config_trunk = port_sf2_sw_config_trunk,
    .update_pbvlan = port_sf2_sw_update_pbvlan,
    .rreg = extsw_rreg_wrap,
    .wreg = extsw_wreg_wrap,
    .fast_age = port_sw_fast_age,
};

port_ops_t port_sf2_port =
{
    .init = port_sf2_port_init,
    .dispatch_pkt = dispatch_pkt_sf2_lan,
    .stats_clear = port_generic_stats_clear,
#ifdef EMBEDDED_BRCMTAG_TX_INSERT
    .tx_pkt_mod = port_sf2_tx_pkt_mod,  /* insert brcm tag for port on external switch */
#endif
    .stats_get = port_generic_stats_get,
    .pause_get = port_generic_pause_get,
    .pause_set = port_generic_pause_set,
    .open = port_sf2_generic_open,
    .mtu_set = port_generic_mtu_set,
    .tx_q_remap = port_sf2_tx_q_remap,
    .mib_dump = port_sw_mib_dump,
    .print_status = port_print_status_verbose,
    .role_set = port_sw_port_role_set,
    .stp_set = port_sw_port_stp_set,
    .fast_age = port_sw_port_fast_age,
    .mib_dump_us = port_sf2_mib_dump_us, // add by Andrew
#if defined(CONFIG_NET_SWITCHDEV)
    .switchdev_ops = 
    {
        .switchdev_port_attr_get = sf2_switchdev_port_attr_get,
        .switchdev_port_attr_set = sf2_switchdev_port_attr_set, 
    }
#endif
};

port_ops_t port_sf2_port_mac =
{
    .stats_get = port_generic_stats_get,
    .stats_clear = port_generic_stats_clear,
    .pause_get = port_generic_pause_get,
    .pause_set = port_generic_pause_set,
    .mtu_set = port_generic_mtu_set,
    .mib_dump = port_sw_mib_dump,
    .print_status = port_print_status_verbose,
    .mib_dump_us = port_sf2_mib_dump_us,
};

int enetxapi_post_config(void)
{
    return enetxapi_post_sf2_config();
}
