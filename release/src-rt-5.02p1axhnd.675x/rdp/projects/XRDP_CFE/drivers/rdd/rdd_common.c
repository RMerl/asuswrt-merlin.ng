/*
    <:copyright-BRCM:2014:DUAL/GPL:standard
    
       Copyright (c) 2014 Broadcom 
       All Rights Reserved
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
:>
*/

#include "rdd.h"
#include "rdd_common.h"
#include "rdd_runner_proj_defs.h"

#define LAYER2_HEADER_COPY_DST_OFFSET_ARRAY(var, offset) \
    uint16_t var[] = { \
        [0] = offset - 2,  \
        [1] = offset - 10, \
        [2] = offset - 10, \
        [3] = offset - 18, \
        [4] = offset - 18, \
        [5] = offset - 2,  \
        [6] = offset - 2,  \
        [7] = offset - 10, \
        [8] = offset - 10, \
        [9] = offset - 18, \
        [10] = offset + 6,  \
        [11] = offset - 2,  \
        [12] = offset - 2,  \
        [13] = offset - 10, \
        [14] = offset - 10, \
        [15] = offset + 6,  \
        [16] = offset + 6,  \
        [17] = offset - 2,  \
        [18] = offset - 2,  \
        [19] = offset - 10, \
        [20] = offset + 14, \
        [21] = offset + 6,  \
        [22] = offset + 6,  \
        [23] = offset - 2,  \
        [24] = offset - 2,  \
        [25] = offset + 14, \
        [26] = offset + 14, \
        [27] = offset + 6,  \
        [28] = offset + 6,  \
        [29] = offset - 2,  \
    }

void rdd_mac_type_cfg(rdd_mac_type wan_mac_type)
{
     RDD_MAC_TYPE_ENTRY_TYPE_WRITE_G(wan_mac_type, RDD_MAC_TYPE_ADDRESS_ARR, 0);
}

static void __rdd_rx_flow_cfg(uint32_t flow_index, rdd_flow_dest destination, rdd_rdd_vport vport, uint32_t counter_id)
{
    RDD_RX_FLOW_ENTRY_FLOW_DEST_WRITE_G(destination, RDD_RX_FLOW_TABLE_ADDRESS_ARR, flow_index);
    RDD_RX_FLOW_ENTRY_VIRTUAL_PORT_WRITE_G(vport, RDD_RX_FLOW_TABLE_ADDRESS_ARR, flow_index);
    RDD_RX_FLOW_ENTRY_CNTR_ID_WRITE_G(counter_id, RDD_RX_FLOW_TABLE_ADDRESS_ARR, flow_index);
}

void rdd_rx_flow_cfg(uint32_t flow_index, rdd_flow_dest destination, rdd_rdd_vport vport, uint32_t counter_id)
{
    static uint8_t first_time = 1;

    RDD_BTRACE("flow_index = %d, destination = %d, vport = %d, counter_id = %d, first_time %d\n", flow_index,
        destination, vport, counter_id, first_time);

    if (first_time)
    {
        int i;

        first_time = 0;
        for (i = 0; i < RX_FLOW_CONTEXTS_NUMBER; i++)
            __rdd_rx_flow_cfg(i, FLOW_DEST_ETH_ID, RDD_VPORT_LAST, 256);
    }
    __rdd_rx_flow_cfg(flow_index, destination, vport, counter_id);
}

void rdd_tx_flow_enable(uint16_t port_or_wan_flow, rdpa_traffic_dir dir, bdmf_boolean enable)
{
    uint16_t tx_flow = PORT_OR_WAN_FLOW_TO_TX_FLOW(port_or_wan_flow, dir);

    RDD_BTRACE("Invalidating port/wan_flow: %d ; tx_flow: %d, dir: %d; enable: %d\n",
               port_or_wan_flow, tx_flow, dir, enable);
    RDD_TX_FLOW_ENTRY_VALID_WRITE_G(enable, RDD_TX_FLOW_TABLE_ADDRESS_ARR, tx_flow);
}
