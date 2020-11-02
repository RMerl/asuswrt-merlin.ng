/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */


#include "rdd.h"
#include "rdd_common.h"
#include "rdp_drv_proj_cntr.h"
#ifdef CONFIG_MCAST_TASK_LIMIT
#include "rdp_drv_cnpl.h"
#endif
#include "rdd_runner_proj_defs.h"
#ifndef _CFE_
#include "rdd_tcam_ic.h"
#include "rdd_defs.h"
#include "rdp_mm.h"
#endif

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


#ifdef XRDP_EMULATION
int get_runner_idx(rdp_runner_image_e module_idx)
{
    int i;

    for (i = 0; i < NUM_OF_RUNNER_CORES; ++i)
        if (rdp_core_to_image_map[i] == module_idx)
            return i;

    return NUM_OF_RUNNER_CORES;
}
#endif

void rdd_mac_type_cfg(rdd_mac_type wan_mac_type)
{
     RDD_MAC_TYPE_ENTRY_TYPE_WRITE_G(wan_mac_type, RDD_MAC_TYPE_ADDRESS_ARR, 0);
}

#if !defined(BCM_DSL_XRDP)
void rdd_layer2_header_copy_mapping_init(void)
{
    uint16_t i;
    LAYER2_HEADER_COPY_DST_OFFSET_ARRAY(layer2_header_copy_dst_offset_arr, 18);

    RDD_BTRACE("\n");

    for (i = 0; i < RDD_LAYER2_HEADER_COPY_MAPPING_TABLE_SIZE; i++)
        RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY_DST_BUFFER_OFFSET_WRITE_G(layer2_header_copy_dst_offset_arr[i], RDD_LAYER2_HEADER_COPY_MAPPING_TABLE_ADDRESS_ARR, i);
}
#endif

void rdd_full_flow_cache_cfg(bdmf_boolean control)
{
    RDD_BTRACE("control = %d\n", control);

    RDD_SYSTEM_CONFIGURATION_ENTRY_FULL_FLOW_CACHE_MODE_WRITE_G(control, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0);
}

void rdd_drop_precedence_cfg(rdpa_traffic_dir dir, uint16_t eligibility_vector)
{
    RDD_BTRACE("eligibility_vector = %d\n", eligibility_vector);

    if (dir == rdpa_dir_ds)
        RDD_SYSTEM_CONFIGURATION_ENTRY_DS_DROP_PRECEDENCE_ELIGIBILITY_VECTOR_WRITE_G(eligibility_vector, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0);
	else
		RDD_SYSTEM_CONFIGURATION_ENTRY_US_DROP_PRECEDENCE_ELIGIBILITY_VECTOR_WRITE_G(eligibility_vector, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0);
}

rx_def_flow_context_t *g_rx_flow_context_ptr;

void rdd_rx_flow_entry_set(uint32_t flow_index, RDD_RX_FLOW_ENTRY_DTS *entry_ptr)
{
    RDD_BTRACE("flow_index = %d, destination = %d, vport = %d, counter_id = %d\n", flow_index,
        entry_ptr->flow_dest, entry_ptr->virtual_port, entry_ptr->cntr_id);

    RDD_RX_FLOW_ENTRY_FLOW_DEST_WRITE_G(entry_ptr->flow_dest, RDD_RX_FLOW_TABLE_ADDRESS_ARR, flow_index);
    RDD_RX_FLOW_ENTRY_VIRTUAL_PORT_WRITE_G(entry_ptr->virtual_port, RDD_RX_FLOW_TABLE_ADDRESS_ARR, flow_index);
    RDD_RX_FLOW_ENTRY_CNTR_ID_WRITE_G(entry_ptr->cntr_id, RDD_RX_FLOW_TABLE_ADDRESS_ARR, flow_index);
#if defined(BCM_PON_XRDP)
    RDD_RX_FLOW_ENTRY_IS_LAN_WRITE_G(entry_ptr->is_lan, RDD_RX_FLOW_TABLE_ADDRESS_ARR, flow_index);
#endif
}

#ifndef _CFE_

void rdd_rx_flow_exception_cfg(uint32_t flow_index, bdmf_boolean exception)
{
    RDD_BTRACE("flow_index = %d, control = %d\n", flow_index, exception);

    RDD_RX_FLOW_ENTRY_EXCEPTION_WRITE_G(exception, RDD_RX_FLOW_TABLE_ADDRESS_ARR, flow_index);
}

void rdd_rx_flow_del(uint32_t flow_index)
{
    RDD_BTRACE("flow_index = %d\n", flow_index);

    RDD_RX_FLOW_ENTRY_FLOW_DEST_WRITE_G(0, RDD_RX_FLOW_TABLE_ADDRESS_ARR, flow_index);
    RDD_RX_FLOW_ENTRY_VIRTUAL_PORT_WRITE_G(0, RDD_RX_FLOW_TABLE_ADDRESS_ARR, flow_index);
    RDD_RX_FLOW_ENTRY_CNTR_ID_WRITE_G(RX_FLOW_CNTR_GROUP_INVLID_CNTR, RDD_RX_FLOW_TABLE_ADDRESS_ARR, flow_index);
    RDD_RX_FLOW_ENTRY_EXCEPTION_WRITE_G(0, RDD_RX_FLOW_TABLE_ADDRESS_ARR, flow_index);
}

#define TCAM_CONTEXT_SIZE         8
uint32_t rdd_rx_flow_cntr_id_get(uint32_t flow_index)
{
    uint32_t cntr_id;

    RDD_BTRACE("flow_index = %d\n", flow_index);

    if (flow_index >= RDD_RX_FLOW_TABLE_SIZE)
    {
        return BDMF_ERR_PARM;
    }

    RDD_RX_FLOW_ENTRY_CNTR_ID_READ_G(cntr_id, RDD_RX_FLOW_TABLE_ADDRESS_ARR, flow_index);
    return cntr_id;
}

uint32_t rdd_rx_flow_entry_get(uint32_t flow_index, RDD_RX_FLOW_ENTRY_DTS *entry_ptr)
{
    uint32_t cntr_id, vport, flow_dest, flow_exc;

    RDD_BTRACE("flow_index = %d, entry_ptr %p \n", flow_index, entry_ptr);

    if (flow_index >= RDD_RX_FLOW_TABLE_SIZE)
    {
        return BDMF_ERR_PARM;
    }

    RDD_RX_FLOW_ENTRY_CNTR_ID_READ_G(cntr_id, RDD_RX_FLOW_TABLE_ADDRESS_ARR, flow_index);
    RDD_RX_FLOW_ENTRY_VIRTUAL_PORT_READ_G(vport, RDD_RX_FLOW_TABLE_ADDRESS_ARR, flow_index);
    RDD_RX_FLOW_ENTRY_FLOW_DEST_READ_G(flow_dest, RDD_RX_FLOW_TABLE_ADDRESS_ARR, flow_index);
    RDD_RX_FLOW_ENTRY_EXCEPTION_READ_G(flow_exc, RDD_RX_FLOW_TABLE_ADDRESS_ARR, flow_index);

    entry_ptr->cntr_id = cntr_id;
    entry_ptr->virtual_port = vport;
    entry_ptr->flow_dest = flow_dest;
    entry_ptr->exception = flow_exc;
    return BDMF_ERR_OK;
}

void rdd_rx_flow_init()
{
    uint32_t i;
    RDD_RX_FLOW_ENTRY_DTS flow_entry;

    RDD_BTRACE("initializing %d flows counters to TX_FLOW_CNTR_GROUP_INVLID_CNTR\n", (RDD_TM_FLOW_CNTR_TABLE_SIZE * 2));

    for (i = 0; i < (RDD_TM_FLOW_CNTR_TABLE_SIZE * 2); i++)
    {
        rdd_tm_flow_cntr_cfg(i, TX_FLOW_CNTR_GROUP_INVLID_CNTR);
    }

#ifdef G9991
    /* vport field is 5 bits - use system port as invalid vport*/
    flow_entry.virtual_port = RDD_SYSTEM_VPORT;
#else
    flow_entry.virtual_port = PROJ_DEFS_NUMBER_OF_VPORTS;
#endif
    flow_entry.cntr_id = RX_FLOW_CNTR_GROUP_INVLID_CNTR;
    flow_entry.flow_dest = FLOW_DEST_ETH_ID;
#if !defined(BCM_DSL_XRDP)
    flow_entry.is_lan = 0;
#endif
    for (i = 0; i < RX_FLOW_CONTEXTS_NUMBER; i++)
    {
        rdd_rx_flow_entry_set(i, &flow_entry);
    }

#if defined(BCM_DSL_XRDP)
    flow_entry.virtual_port = RDD_PON_WAN_VPORT;
    rdd_rx_flow_entry_set(RDD_WAN_FLOW_PLOAM, &flow_entry);
#endif
}

void rdd_rx_default_flow_init()
{
#ifndef XRDP_EMULATION
    uint32_t i;
    rdd_ic_context_t context = {.cntr_id = TCAM_DEF_CNTR_GROUP_INVLID_CNTR};
    bdmf_phys_addr_t phys_addr_p = 0;
    uintptr_t addr_hi, addr_lo;

    context.action = rdpa_forward_action_drop;
    context.policer = -1;

    g_rx_flow_context_ptr = rdp_mm_aligned_alloc(sizeof(rx_def_flow_context_t) * RX_FLOW_CONTEXTS_NUMBER,
        &phys_addr_p);
    if (!g_rx_flow_context_ptr)
        BDMF_TRACE_ERR("Can't allocate Memory space for default flow context");
    GET_ADDR_HIGH_LOW(addr_hi, addr_lo, phys_addr_p);
    RDD_BTRACE("phys addr 0x%08x%08x", (uint32_t)addr_hi, (uint32_t)addr_lo);

    RDD_DEF_FLOW_CONTEXT_DDR_ADDR_HIGH_WRITE_G(addr_hi, RDD_RX_FLOW_CONTEXT_DDR_ADDR_ADDRESS_ARR, 0);
    RDD_DEF_FLOW_CONTEXT_DDR_ADDR_LOW_WRITE_G(addr_lo, RDD_RX_FLOW_CONTEXT_DDR_ADDR_ADDRESS_ARR, 0);

    for (i = 0; i < RX_FLOW_CONTEXTS_NUMBER; i++)
    {
        rdd_rx_default_flow_cfg(i, 0, &context);
    }
#endif
}

void rdd_rx_default_flow_cfg(uint32_t flow_index, uint16_t ctx_index, rdd_ic_context_t *context)
{
    rx_def_flow_context_t *rx_def_flow_context_p;
    uint8_t entry[16] = {};

    RDD_BTRACE("flow_index = %d, context = %p = { tx_flow = %d, egress_port = %d, qos_method = %d, priority = %d, "
        "action = %d, vlan_command_id = %d}, cntr_id = %d\n",
        flow_index, context, context->tx_flow, context->egress_port, context->qos_method, context->priority,
        context->action, context->vlan_command_id.us_vlan_command, context->cntr_id);

    /* remove default flow -> drop */
    if (context->cntr_id == TCAM_DEF_CNTR_GROUP_INVLID_CNTR)
        context->action = rdpa_forward_action_drop;

    /* keep context in DDR as TCAM entry. Cast to uint16_t is safe, since index value cannot exceed 16 bits */
    rdd_tcam_ic_result_entry_compose(ctx_index, context, &entry[0]);

#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(entry, sizeof(rx_def_flow_context_t));
#endif

    rx_def_flow_context_p = g_rx_flow_context_ptr + flow_index;
    memcpy(rx_def_flow_context_p, entry, sizeof(rx_def_flow_context_t));
}

void _rdd_rx_default_flow_context_get(uint32_t entry_index, rx_def_flow_context_t *rx_flow_context)
{
    rx_def_flow_context_t *rx_flow_context_p;
    uint8_t entry[16] = {};

    rx_flow_context_p = g_rx_flow_context_ptr + entry_index;
    memcpy(entry, rx_flow_context_p, sizeof(rx_def_flow_context_t));

#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(entry, sizeof(rx_def_flow_context_t));
#endif

    memcpy(rx_flow_context, entry, sizeof(rx_def_flow_context_t));
}

void rdd_rx_default_flow_context_get(uint32_t flow_index, RDD_RULE_BASED_CONTEXT_ENTRY_DTS *context)
{
    rx_def_flow_context_t rx_flow_context;

    RDD_BTRACE("flow_index = %d}\n", flow_index);

    _rdd_rx_default_flow_context_get(flow_index, &rx_flow_context);

    memcpy(context, &rx_flow_context.rule_base_context, sizeof(RDD_RULE_BASED_CONTEXT_ENTRY_DTS));
}

uint32_t rdd_rx_default_flow_cntr_id_get(uint32_t flow_index)
{
    rx_def_flow_context_t rx_flow_context;

    RDD_BTRACE("flow_index = %d}\n", flow_index);

    _rdd_rx_default_flow_context_get(flow_index, &rx_flow_context);

    return rx_flow_context.rule_base_context.cntr_id;
}

void rdd_rx_mirroring_cfg(rdd_rdd_vport vport, bdmf_boolean control)
{
    RDD_BTRACE("vport = %d, control = %d\n", vport, control);
#if !defined(BCM_DSL_XRDP)
    RDD_VPORT_CFG_EX_ENTRY_MIRRORING_EN_WRITE_G(control, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, vport);
#endif
}

void rdd_rx_mirroring_direct_cfg(bdmf_boolean enable)
{
    RDD_BTRACE("enable = %d\n", enable);

    GROUP_MWRITE_8(RDD_RX_MIRRORING_DIRECT_ENABLE_ADDRESS_ARR, 0, enable);
}

void rdd_loopback_cfg(rdd_rdd_vport vport, bdmf_boolean control)
{
    RDD_BTRACE("vport = %d, control = %d\n", vport, control);
#if !defined(BCM_DSL_XRDP)
    RDD_VPORT_CFG_EX_ENTRY_LOOPBACK_EN_WRITE_G(control, RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR, vport);
#endif
}

void rdd_loopback_queue_set(rdd_rdd_vport vport, uint32_t queue_id)
{
    RDD_BTRACE("vport = %d, queue = %d\n", vport, queue_id);

    GROUP_MWRITE_8(RDD_LOOPBACK_QUEUE_TABLE_ADDRESS_ARR, vport, queue_id);
}

void rdd_loopback_wan_flow_set(uint32_t flow)
{
    RDD_BTRACE("wan_flow = %d\n", flow);
#if !defined(BCM_DSL_XRDP)
    GROUP_MWRITE_8(RDD_LOOPBACK_WAN_FLOW_TABLE_ADDRESS_ARR, 0, flow);
#endif    
}

void rdd_ingress_qos_drop_miss_ratio_set(uint32_t drop_miss_ratio)
{
    RDD_BTRACE("drop_miss_ratio = %d\n", drop_miss_ratio);

    GROUP_MWRITE_8(RDD_INGRESS_QOS_DONT_DROP_RATIO_ADDRESS_ARR, 0, drop_miss_ratio);
}

void rdd_ingress_qos_wan_untagged_priority_set(bdmf_boolean wan_untagged_priority)
{
    RDD_BTRACE("wan_untagged_priority = %d\n", wan_untagged_priority);

#if !defined(BCM_DSL_XRDP)
    GROUP_MWRITE_8(RDD_INGRESS_QOS_WAN_UNTAGGED_PRIORITY_ADDRESS_ARR, 0, wan_untagged_priority);
#endif
}

void rdd_tm_flow_cntr_cfg(uint32_t cntr_entry, uint32_t cntr_id)
{

    RDD_BTRACE("cntr_entry = %d, cntr_id = %d\n", cntr_entry, cntr_id);

#if defined(CONFIG_MULTI_WAN_SUPPORT) && !defined(BCM_PON_XRDP)
    if (RDD_IS_TM_PON_FLOW_ID(cntr_entry))
        RDD_TM_FLOW_CNTR_ENTRY_CNTR_ID_WRITE_G(cntr_id, RDD_US_TM_PON_TM_FLOW_CNTR_TABLE_ADDRESS_ARR, RDD_TX_PON_FLOW_ID(cntr_entry));
    else if (RDD_IS_TM_DSL_FLOW_ID(cntr_entry))
        RDD_TM_FLOW_CNTR_ENTRY_CNTR_ID_WRITE_G(cntr_id, RDD_US_TM_DSL_TM_FLOW_CNTR_TABLE_ADDRESS_ARR, RDD_TX_DSL_FLOW_ID(cntr_entry));
    else if (cntr_entry < RDD_TM_FLOW_CNTR_TABLE_SIZE)
        RDD_TM_FLOW_CNTR_ENTRY_CNTR_ID_WRITE_G(cntr_id, RDD_US_TM_ETH_TM_FLOW_CNTR_TABLE_ADDRESS_ARR, 0);
#else
    if (cntr_entry < RDD_TM_FLOW_CNTR_TABLE_SIZE)
        RDD_TM_FLOW_CNTR_ENTRY_CNTR_ID_WRITE_G(cntr_id, RDD_US_TM_TM_FLOW_CNTR_TABLE_ADDRESS_ARR, cntr_entry);
#endif
    else if (cntr_entry-RDD_TM_FLOW_CNTR_TABLE_SIZE < RDD_IMAGE_0_DS_TM_TM_FLOW_CNTR_TABLE_SIZE)
        RDD_TM_FLOW_CNTR_ENTRY_CNTR_ID_WRITE_G(cntr_id, RDD_DS_TM_TM_FLOW_CNTR_TABLE_ADDRESS_ARR, cntr_entry-RDD_TM_FLOW_CNTR_TABLE_SIZE);
}

uint32_t rdd_tm_flow_cntr_id_get(uint32_t cntr_entry)
{
    uint32_t cntr_id = TX_FLOW_CNTR_GROUP_INVLID_CNTR;

    RDD_BTRACE("cntr_entry = %d\n", cntr_entry);
#if defined(CONFIG_MULTI_WAN_SUPPORT) && !defined(BCM_PON_XRDP)
    if (RDD_IS_TM_PON_FLOW_ID(cntr_entry))
        RDD_TM_FLOW_CNTR_ENTRY_CNTR_ID_READ_G(cntr_id, RDD_US_TM_PON_TM_FLOW_CNTR_TABLE_ADDRESS_ARR, RDD_TX_PON_FLOW_ID(cntr_entry));
    else if (RDD_IS_TM_DSL_FLOW_ID(cntr_entry))
        RDD_TM_FLOW_CNTR_ENTRY_CNTR_ID_READ_G(cntr_id, RDD_US_TM_DSL_TM_FLOW_CNTR_TABLE_ADDRESS_ARR, RDD_TX_DSL_FLOW_ID(cntr_entry));
    else if (cntr_entry < RDD_TM_FLOW_CNTR_TABLE_SIZE)
        RDD_TM_FLOW_CNTR_ENTRY_CNTR_ID_READ_G(cntr_id, RDD_US_TM_ETH_TM_FLOW_CNTR_TABLE_ADDRESS_ARR, 0);
#else
    if (cntr_entry < RDD_TM_FLOW_CNTR_TABLE_SIZE)
        RDD_TM_FLOW_CNTR_ENTRY_CNTR_ID_READ_G(cntr_id, RDD_US_TM_TM_FLOW_CNTR_TABLE_ADDRESS_ARR, cntr_entry);
#endif
    else if (cntr_entry-RDD_TM_FLOW_CNTR_TABLE_SIZE < RDD_IMAGE_0_DS_TM_TM_FLOW_CNTR_TABLE_SIZE)
        RDD_TM_FLOW_CNTR_ENTRY_CNTR_ID_READ_G(cntr_id, RDD_DS_TM_TM_FLOW_CNTR_TABLE_ADDRESS_ARR, cntr_entry-RDD_TM_FLOW_CNTR_TABLE_SIZE);
    return cntr_id;
}
#endif

int rdd_port_or_wan_flow_to_tx_flow_and_table_ptr(uint16_t port_or_wan_flow,
                                                  rdpa_traffic_dir dir, uint32_t **table_arr_ptr)
{
#if defined(CONFIG_MULTI_WAN_SUPPORT) && !defined(BCM_PON_XRDP)
    int tx_flow;

    if ((dir == rdpa_dir_us) && RDD_IS_TM_PON_FLOW_ID(port_or_wan_flow))
    {
        /* PON */
        tx_flow = RDD_TX_PON_FLOW_ID(port_or_wan_flow);
        *table_arr_ptr = RDD_PON_TX_FLOW_TABLE_ADDRESS_ARR;
    }
    else if ((dir == rdpa_dir_us) && RDD_IS_TM_DSL_FLOW_ID(port_or_wan_flow))
    {
        /* DSL */
        tx_flow = RDD_TX_DSL_FLOW_ID(port_or_wan_flow);
        *table_arr_ptr = RDD_DSL_TX_FLOW_TABLE_ADDRESS_ARR;
    }
    else
    {
        /* VPORT, which include GBE WAN port */
        if (port_or_wan_flow >= RDD_VPORT_TX_FLOW_TABLE_SIZE)
        {
            BDMF_TRACE_ERR("port_or_wan_flow %d is over supported VPORT range %d",
                            port_or_wan_flow, RDD_IMAGE_1_VPORT_TX_FLOW_TABLE_SIZE);
            return -1;
        }
        tx_flow = port_or_wan_flow;
        *table_arr_ptr = RDD_VPORT_TX_FLOW_TABLE_ADDRESS_ARR;
    }
    return tx_flow;
#else
    return 0;
#endif
}

void rdd_tx_flow_enable(uint16_t port_or_wan_flow, rdpa_traffic_dir dir, bdmf_boolean enable)
{
#if defined(CONFIG_MULTI_WAN_SUPPORT) && !defined(BCM_PON_XRDP)
    int tx_flow;
    uint32_t *table_addr_arr;

    tx_flow = rdd_port_or_wan_flow_to_tx_flow_and_table_ptr(port_or_wan_flow, dir, &table_addr_arr);
    if (tx_flow < 0)
        return;

    RDD_BTRACE("Invalidating port/wan_flow: %d ; tx_flow: %d, dir: %d; enable: %d\n",
               port_or_wan_flow, tx_flow, dir, enable);
    RDD_TX_FLOW_ENTRY_VALID_WRITE_G(enable, table_addr_arr, tx_flow);
#else
    uint16_t tx_flow = PORT_OR_WAN_FLOW_TO_TX_FLOW(port_or_wan_flow, dir);

    RDD_BTRACE("Invalidating port/wan_flow: %d ; tx_flow: %d, dir: %d; enable: %d\n",
               port_or_wan_flow, tx_flow, dir, enable);
    RDD_TX_FLOW_ENTRY_VALID_WRITE_G(enable, RDD_TX_FLOW_TABLE_ADDRESS_ARR, tx_flow);
#endif
}


void rdd_qm_queue_to_tx_flow_tbl_cfg(uint16_t qm_queue, rdpa_traffic_dir dir, rdpa_wan_type wan_type)
{
#if defined(BCM_DSL_XRDP)
    static uint8_t qm_queue_to_tx_flow_first_time = 1;
    int i;
    uint16_t tx_flow_table_addr;

    if (qm_queue_to_tx_flow_first_time)
    {
        qm_queue_to_tx_flow_first_time = 0;
        /* TODO: if we decide to improve tx_flow_check for all processing_images,
         * then the folloing needs to be improved */
        for (i = 0; i < RDD_IMAGE_2_QM_QUEUE_TO_TX_FLOW_TABLE_PTR_TABLE_SIZE; i++)
            RDD_BYTES_2_BITS_WRITE_G(IMAGE_2_VPORT_TX_FLOW_TABLE_ADDRESS, RDD_QM_QUEUE_TO_TX_FLOW_TABLE_PTR_TABLE_ADDRESS_ARR, i);
    }

    if (qm_queue >= RDD_QM_QUEUE_TO_TX_FLOW_TABLE_PTR_TABLE_SIZE)
        return;

    /* TODO: if we decide to improve tx_flow_check for all processing_images,
     * then the folloing needs to be improved */
    if ((dir == rdpa_dir_us) && (wan_type == rdpa_wan_dsl))
        tx_flow_table_addr = IMAGE_2_DSL_TX_FLOW_TABLE_ADDRESS;
    else if ((dir == rdpa_dir_us) && (wan_type >= rdpa_wan_gpon) && (wan_type <= rdpa_wan_xepon))
        tx_flow_table_addr = IMAGE_2_PON_TX_FLOW_TABLE_ADDRESS;
    else
        tx_flow_table_addr = IMAGE_2_VPORT_TX_FLOW_TABLE_ADDRESS;

    RDD_BYTES_2_BITS_WRITE_G(tx_flow_table_addr, RDD_QM_QUEUE_TO_TX_FLOW_TABLE_PTR_TABLE_ADDRESS_ARR, qm_queue);
#endif
}

void rdd_fpm_pool_number_mapping_cfg(uint16_t fpm_base_token_size)
{
    uint8_t i;
    uint8_t exp_fpm_base;

#if !defined(BCM_DSL_XRDP)
    uint8_t headers_per_fpm;
#endif

    /* exponent of fpm_base_token_size { 2K, 1K, 512, 256 } */
    if (fpm_base_token_size == 2048)
        exp_fpm_base = 11;
    else
        exp_fpm_base = ((fpm_base_token_size/2) >> 8) + 8;

    /*
        This is a mapping function from packet size to FPM number according to configured fpm_base_token_size
        The pool number is resolved using the configured fpm base token size - usually 512 bytes
        From FW we get pool :
           fpm_pool = PM_POOL_NUMBER_MAPPING_TABLE [ find first MSB of packet_len ]
    */

    for (i = 0; i < exp_fpm_base; i++)
        RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(3, RDD_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, i);

    RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(2, RDD_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, exp_fpm_base);
    RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(1, RDD_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, (exp_fpm_base + 1));

    for (i = exp_fpm_base + 2; i < RDD_FPM_POOL_NUMBER_MAPPING_TABLE_SIZE; i++)
        RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(0, RDD_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, i);

    /*  After table manager bug is fixed and will be possible to define same table on 2 different modules,
        belonging to the same core this 2 tables can be the same */
    for (i = 0; i < exp_fpm_base; i++)
        RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(3, RDD_CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, i);

    RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(2, RDD_CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, exp_fpm_base);
    RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(1, RDD_CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, (exp_fpm_base + 1));

    for (i = exp_fpm_base + 2; i < RDD_CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE_SIZE; i++)
        RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(0, RDD_CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, i);

#if defined(CONFIG_DHD_RUNNER) && !defined(_CFE_)
    /* For DHD the content indicates a number of basic FPMs for given length */
    for (i = 0; i < exp_fpm_base; i++)
        RDD_BYTE_1_BITS_WRITE_G(1, RDD_DHD_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, i);

    RDD_BYTE_1_BITS_WRITE_G(2, RDD_DHD_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, exp_fpm_base);
    RDD_BYTE_1_BITS_WRITE_G(4, RDD_DHD_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, (exp_fpm_base + 1));

    for (i = exp_fpm_base + 2; i < RDD_DHD_FPM_POOL_NUMBER_MAPPING_TABLE_SIZE; i++)
        RDD_BYTE_1_BITS_WRITE_G(8, RDD_DHD_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, i);
#endif

#if !defined(BCM_DSL_XRDP)
    /*
        This is a mapping function from number of flooding replications to FPM pool number
        according to configured fpm_base_token_size. The pool number is calculated using the
        configured fpm base token size - usually 512 bytes, the replication header size and the pool
        indication indexing (0:8 1:4 2:2 3:1 FPMs).
    */

    /* How many headers are in an FPM */
    headers_per_fpm = fpm_base_token_size / RDD_MCAST_FPM_HEADER_SLOT_HEADER_MAX_SIZE_NUMBER;

    /* Since FW holds replications_number = (replication number - 1) we set the thresholds accordingly.
       We assume table was initialized with zeros (8 FPMs) */
    for (i = 0; i < RDD_FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_SIZE; i++)
    {
        if (i < headers_per_fpm)
            RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(3, RDD_FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, i);
        else if (i < 2 * headers_per_fpm)
            RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(2, RDD_FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, i);
        else if (i < 4 * headers_per_fpm)
            RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(1, RDD_FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, i);
        else if (i < 8 * headers_per_fpm)
            RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(0, RDD_FLOODING_HEADER_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, i);
    }
#endif
}

#if !defined(BCM_DSL_XRDP)
void rdd_max_pkt_len_table_init(void)
{
    /* index 0 in the table is used by invalid pathstat_idx and dummy flows (natc_miss bypass) */
    RDD_BYTES_2_BITS_WRITE_G(DUMMY_MAX_PKT_LEN,RDD_MAX_PKT_LEN_TABLE_ADDRESS_ARR,0);
}
#endif

void rdd_rate_limit_overhead_cfg(uint8_t  xi_rate_limit_overhead)
{
    RDD_BTRACE("xi_rate_limit_overhead=%d\n", xi_rate_limit_overhead);
    RDD_BYTE_1_BITS_WRITE_G(xi_rate_limit_overhead, RDD_RATE_LIMIT_OVERHEAD_ADDRESS_ARR, 0);
}

#ifndef _CFE_
void rdd_multicast_filter_cfg(rdpa_mcast_filter_method mcast_prefix_filter)
{
    if (mcast_prefix_filter == rdpa_mcast_filter_method_mac || mcast_prefix_filter == rdpa_mcast_filter_method_mac_and_ip)
    	RDD_IPTV_CLASSIFICATION_CFG_ENTRY_DA_PREFIX_MODE_MAC_WRITE_G(1, RDD_IPTV_CLASSIFICATION_CFG_TABLE_ADDRESS_ARR, 0);
    else
    	RDD_IPTV_CLASSIFICATION_CFG_ENTRY_DA_PREFIX_MODE_MAC_WRITE_G(0, RDD_IPTV_CLASSIFICATION_CFG_TABLE_ADDRESS_ARR, 0);


	if (mcast_prefix_filter == rdpa_mcast_filter_method_ip || mcast_prefix_filter == rdpa_mcast_filter_method_mac_and_ip)
		RDD_IPTV_CLASSIFICATION_CFG_ENTRY_DA_PREFIX_MODE_IP_WRITE_G(1, RDD_IPTV_CLASSIFICATION_CFG_TABLE_ADDRESS_ARR, 0);
	else
		RDD_IPTV_CLASSIFICATION_CFG_ENTRY_DA_PREFIX_MODE_IP_WRITE_G(0, RDD_IPTV_CLASSIFICATION_CFG_TABLE_ADDRESS_ARR, 0);

	if (mcast_prefix_filter == rdpa_mcast_filter_method_none)
	{
        RDD_IPTV_CLASSIFICATION_CFG_ENTRY_DA_PREFIX_MODE_MAC_WRITE_G(0, RDD_IPTV_CLASSIFICATION_CFG_TABLE_ADDRESS_ARR, 0);
        RDD_IPTV_CLASSIFICATION_CFG_ENTRY_DA_PREFIX_MODE_IP_WRITE_G(0, RDD_IPTV_CLASSIFICATION_CFG_TABLE_ADDRESS_ARR, 0);
    }
}

void rdd_iptv_status_cfg(bdmf_boolean iptv_status)
{
    RDD_IPTV_CLASSIFICATION_CFG_ENTRY_IPTV_EN_WRITE_G(iptv_status, RDD_IPTV_CLASSIFICATION_CFG_TABLE_ADDRESS_ARR, 0);
}

bdmf_error_t rdd_iptv_lkp_miss_action_cfg(rdpa_forward_action new_lookup_miss_action)
{
    if (new_lookup_miss_action == rdpa_forward_action_drop)
    {
        RDD_SYSTEM_CONFIGURATION_ENTRY_IPTV_LOOKUP_MISS_ACTION_WRITE_G(0, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0);
    }
    else if (new_lookup_miss_action == rdpa_forward_action_host)
    {
        RDD_SYSTEM_CONFIGURATION_ENTRY_IPTV_LOOKUP_MISS_ACTION_WRITE_G(1, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0);
    }
    else
        return BDMF_ERR_PARM;

    return BDMF_ERR_OK;
}

void rdd_ecn_remark_enable_cfg(bdmf_boolean ecn_remark_enable)
{
#if !defined(BCM_DSL_XRDP)
    RDD_ECN_IPV6_REMARK_ENTRY_ENABLED_WRITE_G(ecn_remark_enable, RDD_ECN_IPV6_REMARK_TABLE_ADDRESS_ARR, 0);
#endif
}

bdmf_boolean rdd_ecn_remark_enable_get(void)
{
    bdmf_boolean ecn_remark_enable = 0;
#if !defined(BCM_DSL_XRDP)
    RDD_ECN_IPV6_REMARK_ENTRY_ENABLED_READ_G(ecn_remark_enable, RDD_ECN_IPV6_REMARK_TABLE_ADDRESS_ARR, 0);
#endif
    return ecn_remark_enable;
}

void rdd_tcp_ack_priority_flow_set(bdmf_boolean enable)
{
    RDD_BTRACE("enable = %d\n", (int)enable);

    RDD_SYSTEM_CONFIGURATION_ENTRY_TCP_PURE_ACK_FLOWS_WRITE_G(enable, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0);
}

bdmf_boolean rdd_tcp_ack_priority_flow_get(void)
{
    bdmf_boolean enable = 0;

    RDD_SYSTEM_CONFIGURATION_ENTRY_TCP_PURE_ACK_FLOWS_READ_G(enable, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0);
    return enable;
}

#endif

void rdd_system_congestion_ctrl_enable(bdmf_boolean enable)
{
#if defined(BCM_DSL_XRDP)
    RDD_SYSTEM_CONFIGURATION_ENTRY_CONGESTION_CTRL_WRITE_G(enable, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0);
#endif
}

bdmf_boolean rdd_system_congestion_ctrl_get(void)
{
#if defined(BCM_DSL_XRDP)
    bdmf_boolean enable;

    RDD_SYSTEM_CONFIGURATION_ENTRY_CONGESTION_CTRL_READ_G(enable, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0);
    return enable;
#else
    /* for BCM_PON_XRDP, congestion ctrl is always enabled */
    return 1;
#endif
}

#ifdef CONFIG_MCAST_TASK_LIMIT
void rdd_mcast_max_tasks_limit_cfg(uint16_t xi_mcast_max_tasks_limit)
{
    RDD_BYTES_2_BITS_WRITE_G(xi_mcast_max_tasks_limit, RDD_MCAST_BBH_OVERRUN_MAX_TASKS_LIMIT_ADDRESS_ARR, 0);
#ifndef RDP_SIM
    drv_cnpl_counter_set(CNPL_GROUP_TWO_BYTE_CNTR, COUNTER_IPTV_PROCESSING_TASKS_LIMIT, xi_mcast_max_tasks_limit);
#endif
}

void rdd_mcast_min_tasks_limit_cfg(uint16_t xi_mcast_min_tasks_limit)
{
    /* both values min/max are written to the data SRAM, but the CNPL which contain single value for task limit */
    /* will be set with the max value */
    RDD_BYTES_2_BITS_WRITE_G(xi_mcast_min_tasks_limit, RDD_MCAST_BBH_OVERRUN_MIN_TASKS_LIMIT_ADDRESS_ARR, 0);
}
#endif
