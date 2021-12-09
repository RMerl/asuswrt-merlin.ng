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

#include "archer.h"

// =========== sysp port ops =============================

// TODO47622: same as dispatch_pkt_sf2_lan() for now
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
    *port_type = PORT_TYPE_SYSP_PORT;
    *port_id = port_info->port;

    return 0;
}


sw_ops_t port_sysp_sw =
{
    .init = port_sysp_sw_init,
    .uninit = port_sysp_sw_uninit,
    .mux_port_rx = mux_get_rx_index,    // external switch does not have demux
    .mux_free = mux_index_sw_free,
    .stats_get = port_generic_stats_get,
    .port_id_on_sw = port_sysp_sw_port_id_on_sw,
};

port_ops_t port_sysp_port =
{
    .init = port_sysp_port_init,
    .dispatch_pkt = dispatch_pkt_sysp_lan,
    .stats_clear = port_generic_stats_clear,
    .stats_get = port_generic_stats_get,
    .pause_get = port_generic_pause_get,
    .pause_set = port_generic_pause_set,
    .open = port_generic_open,
    .mtu_set = port_generic_mtu_set,
    .mib_dump = port_sysp_mib_dump,
    .print_status = port_sysp_print_status,
    .role_set = port_sysp_port_role_set,
    .mib_dump_us = port_sysp_mib_dump_us, // add by Andrew
};

int enetxapi_post_config(void)
{
    return enetxapi_post_sysp_config();
}