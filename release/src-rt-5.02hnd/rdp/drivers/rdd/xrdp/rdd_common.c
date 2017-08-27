/*
    <:copyright-BRCM:2014:DUAL/GPL:standard
    
       Copyright (c) 2014 Broadcom 
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

#include "rdd.h"
#include "rdd_common.h"
#include "rdp_drv_proj_cntr.h"
#ifndef _CFE_
#include "rdp_mm.h"
#include "rdd_tcam_ic.h"
#endif

#define LAYER2_HEADER_COPY_ROUTINE_ARRAY(var, image, prefix) \
    uint16_t var[] = { \
        [0] = ADDRESS_OF(image, prefix##_14_bytes_8_bytes_align), \
        [1] = ADDRESS_OF(image, prefix##_18_bytes_8_bytes_align), \
        [2] = ADDRESS_OF(image, prefix##_22_bytes_8_bytes_align), \
        [3] = ADDRESS_OF(image, prefix##_26_bytes_8_bytes_align), \
        [4] = ADDRESS_OF(image, prefix##_30_bytes_8_bytes_align), \
        [5] = ADDRESS_OF(image, prefix##_14_bytes_4_bytes_align), \
        [6] = ADDRESS_OF(image, prefix##_18_bytes_4_bytes_align), \
        [7] = ADDRESS_OF(image, prefix##_22_bytes_4_bytes_align), \
        [8] = ADDRESS_OF(image, prefix##_26_bytes_4_bytes_align), \
        [9] = ADDRESS_OF(image, prefix##_30_bytes_4_bytes_align), \
        [10] = ADDRESS_OF(image, prefix##_14_bytes_8_bytes_align), \
        [11] = ADDRESS_OF(image, prefix##_18_bytes_8_bytes_align), \
        [12] = ADDRESS_OF(image, prefix##_22_bytes_8_bytes_align), \
        [13] = ADDRESS_OF(image, prefix##_26_bytes_8_bytes_align), \
        [14] = ADDRESS_OF(image, prefix##_30_bytes_8_bytes_align), \
        [15] = ADDRESS_OF(image, prefix##_14_bytes_4_bytes_align), \
        [16] = ADDRESS_OF(image, prefix##_18_bytes_4_bytes_align), \
        [17] = ADDRESS_OF(image, prefix##_22_bytes_4_bytes_align), \
        [18] = ADDRESS_OF(image, prefix##_26_bytes_4_bytes_align), \
        [19] = ADDRESS_OF(image, prefix##_30_bytes_4_bytes_align), \
        [20] = ADDRESS_OF(image, prefix##_14_bytes_8_bytes_align), \
        [21] = ADDRESS_OF(image, prefix##_18_bytes_8_bytes_align), \
        [22] = ADDRESS_OF(image, prefix##_22_bytes_8_bytes_align), \
        [23] = ADDRESS_OF(image, prefix##_26_bytes_8_bytes_align), \
        [24] = ADDRESS_OF(image, prefix##_30_bytes_8_bytes_align), \
        [25] = ADDRESS_OF(image, prefix##_14_bytes_4_bytes_align), \
        [26] = ADDRESS_OF(image, prefix##_18_bytes_4_bytes_align), \
        [27] = ADDRESS_OF(image, prefix##_22_bytes_4_bytes_align), \
        [28] = ADDRESS_OF(image, prefix##_26_bytes_4_bytes_align), \
        [29] = ADDRESS_OF(image, prefix##_30_bytes_4_bytes_align), \
    }

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

void rdd_layer2_header_copy_mapping_init(void)
{
    uint16_t i;
    LAYER2_HEADER_COPY_ROUTINE_ARRAY(layer2_header_copy_routine_arr, image_4, processing_layer2_header_copy);
    LAYER2_HEADER_COPY_DST_OFFSET_ARRAY(layer2_header_copy_dst_offset_arr, 18);

    RDD_BTRACE("\n");

    for (i = 0; i < RDD_LAYER2_HEADER_COPY_MAPPING_TABLE_SIZE; i++)
    {
        RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY_ROUTINE_WRITE_G(layer2_header_copy_routine_arr[i], RDD_LAYER2_HEADER_COPY_MAPPING_TABLE_ADDRESS_ARR, i);
        RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY_DST_BUFFER_OFFSET_WRITE_G(layer2_header_copy_dst_offset_arr[i], RDD_LAYER2_HEADER_COPY_MAPPING_TABLE_ADDRESS_ARR, i);
    }
}

void rdd_full_flow_cache_cfg(bdmf_boolean control)
{
    RDD_BTRACE("control = %d\n", control);

    RDD_SYSTEM_CONFIGURATION_ENTRY_FULL_FLOW_CACHE_MODE_WRITE_G(control, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0);
}

rx_def_flow_context_t *g_rx_flow_context_ptr;

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
            __rdd_rx_flow_cfg(i, FLOW_DEST_ETH_ID, RDD_VPORT_LAST, RX_FLOW_CNTR_GROUP_INVLID_CNTR);
    }
    __rdd_rx_flow_cfg(flow_index, destination, vport, counter_id);
}

void rdd_rx_flow_del(uint32_t flow_index)
{
    RDD_BTRACE("flow_index = %d\n", flow_index);

    RDD_RX_FLOW_ENTRY_FLOW_DEST_WRITE_G(0, RDD_RX_FLOW_TABLE_ADDRESS_ARR, flow_index);
    RDD_RX_FLOW_ENTRY_VIRTUAL_PORT_WRITE_G(0, RDD_RX_FLOW_TABLE_ADDRESS_ARR, flow_index);
    RDD_RX_FLOW_ENTRY_CNTR_ID_WRITE_G(RX_FLOW_CNTR_GROUP_INVLID_CNTR, RDD_RX_FLOW_TABLE_ADDRESS_ARR, flow_index);
}

#ifndef _CFE_ 
#define TCAM_CONTEXT_SIZE         8
uint32_t rdd_rx_flow_cntr_id_get(uint32_t flow_index)
{
    uint32_t cntr_id, core_index;
    RDD_RX_FLOW_TABLE_DTS *rx_flow_table_ptr;
    RDD_RX_FLOW_ENTRY_DTS *rx_flow_entry_ptr;

    RDD_BTRACE("flow_index = %d\n", flow_index);

    for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
    {
        /* setting first group - processing */
        if (rdp_core_to_image_map[core_index] == processing_runner_image)
        {
            rx_flow_table_ptr = RDD_RX_FLOW_TABLE_PTR(core_index);
            rx_flow_entry_ptr = &(rx_flow_table_ptr->entry[flow_index]);
            RDD_RX_FLOW_ENTRY_CNTR_ID_READ(cntr_id, rx_flow_entry_ptr);
            return cntr_id;
        }
    }
    return RX_FLOW_CNTR_GROUP_INVLID_CNTR;
}

uint32_t rdd_rx_flow_params_get(uint32_t flow_index, RDD_RX_FLOW_ENTRY_DTS *entry_ptr)
{
    uint32_t core_index, cntr_id, vport;
    RDD_RX_FLOW_TABLE_DTS *rx_flow_table_ptr;
    RDD_RX_FLOW_ENTRY_DTS *rx_flow_entry_ptr;
    
    RDD_BTRACE("flow_index = %d, entry_ptr %p \n", flow_index, entry_ptr);

    if (flow_index >= RDD_RX_FLOW_TABLE_SIZE)
    {
        return BDMF_ERR_PARM;
    }

    for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
    {
        /* setting first group - processing */
        if (rdp_core_to_image_map[core_index] == processing_runner_image)
        {
            rx_flow_table_ptr = RDD_RX_FLOW_TABLE_PTR(core_index);
            rx_flow_entry_ptr = &(rx_flow_table_ptr->entry[flow_index]);
            RDD_RX_FLOW_ENTRY_CNTR_ID_READ(cntr_id, rx_flow_entry_ptr);
            RDD_RX_FLOW_ENTRY_VIRTUAL_PORT_READ(vport, rx_flow_entry_ptr);
            entry_ptr->cntr_id = cntr_id;
            entry_ptr->virtual_port = vport;
            return BDMF_ERR_OK;
        }
    }
    return BDMF_ERR_NOENT;
}

void rdd_rx_def_flow_ext_context_compose(RDD_IC_EXT_CONTEXT_ENTRY_DTS ctx, uint8_t *entry)
{
    RDD_BTRACE("ctx.cntr_id = %d, entry = %p\n", ctx.cntr_id, entry);

    memcpy(entry, &ctx, sizeof(RDD_IC_EXT_CONTEXT_ENTRY_DTS));
}

static void _rdd_default_flow_cntr_id_init(uint32_t entries_num)
{
    uint32_t i;
    rdd_ic_context_t context = {};
    
    RDD_BTRACE("entries_num = %d\n", entries_num);

    for (i = 0; i < entries_num; i++)
    {
        rdd_rx_default_flow_cfg(i, &context, TCAM_IPTV_DEF_CNTR_GROUP_INVLID_CNTR);
    }
}

void rdd_rx_default_flow_cfg(uint32_t flow_index, rdd_ic_context_t *context, uint32_t cntr_id)
{
    RDD_IC_EXT_CONTEXT_ENTRY_DTS rule_based_ext_context = {.cntr_id = cntr_id};
    rx_def_flow_context_t *rx_def_flow_context_p;
    uint8_t entry[16];
    static uint8_t first_time = 1;

    RDD_BTRACE("flow_index = %d, context = %p = { tx_flow = %d, egress_port = %d, qos_method = %d, priority = %d, "
        "action = %d, vlan_command_id = %d}, cntr_id = %d\n",
        flow_index, context, context->tx_flow, context->egress_port, context->qos_method, context->priority,
        context->action, context->vlan_command_id.us_vlan_command, cntr_id);

    if (first_time)
    {
        bdmf_phys_addr_t phys_addr_p = 0;
        uintptr_t addr_hi, addr_lo;

        g_rx_flow_context_ptr = rdp_mm_aligned_alloc(sizeof(rx_def_flow_context_t) * RX_FLOW_CONTEXTS_NUMBER,
            &phys_addr_p);
        if (!g_rx_flow_context_ptr)
            BDMF_TRACE_ERR("Can't allocate Memory space for feault flow context");
        GET_ADDR_HIGH_LOW(addr_hi, addr_lo, phys_addr_p);
        bdmf_trace("phys addr 0x%08x%08x", (uint32_t)addr_hi, (uint32_t)addr_lo);

        RDD_DEF_FLOW_CONTEXT_DDR_ADDR_HIGH_WRITE_G(addr_hi, RDD_RX_FLOW_CONTEXT_DDR_ADDR_ADDRESS_ARR, 0);
        RDD_DEF_FLOW_CONTEXT_DDR_ADDR_LOW_WRITE_G(addr_lo, RDD_RX_FLOW_CONTEXT_DDR_ADDR_ADDRESS_ARR, 0);

        first_time = 0;
        _rdd_default_flow_cntr_id_init(RX_FLOW_CONTEXTS_NUMBER);
    }

    /* keep context in DDR as TCAM entry. Cast to uint16_t is safe, since index value cannot exceed 16 bits */ 
    rdd_tcam_ic_result_entry_compose((uint16_t)flow_index, context, &entry[0]);
    rdd_rx_def_flow_ext_context_compose(rule_based_ext_context, &entry[sizeof(RDD_RULE_BASED_CONTEXT_ENTRY_DTS)]);

#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(entry, sizeof(rx_def_flow_context_t));
#endif

    rx_def_flow_context_p = g_rx_flow_context_ptr + flow_index;
    memcpy(rx_def_flow_context_p, entry, sizeof(rx_def_flow_context_t));
}

void rdd_rx_default_flow_context_get(uint32_t flow_index, RDD_RULE_BASED_CONTEXT_ENTRY_DTS *entry)
{
    rx_def_flow_context_t *rx_flow_context_p;

    RDD_BTRACE("flow_index = %d, entry = %p = { wan_flow = %d, valid = %d, table_id = %d, queue = %d, port = %d, "
        "rule = %d }\n",
        flow_index, entry, entry->wan_flow, entry->valid, entry->table_id, entry->queue, entry->port, entry->rule);

    rx_flow_context_p = g_rx_flow_context_ptr + flow_index;
    memcpy(&entry, rx_flow_context_p, sizeof(RDD_RULE_BASED_CONTEXT_ENTRY_DTS));
}

uint32_t rdd_rx_default_flow_cntr_id_get(uint32_t entry_index)
{
    rx_def_flow_context_t *rx_def_flow_context_p;
    rx_def_flow_context_t rx_def_flow_context;

    RDD_BTRACE("entry_index = %d\n", entry_index);

    rx_def_flow_context_p = g_rx_flow_context_ptr + entry_index;
    memcpy(&rx_def_flow_context, rx_def_flow_context_p, sizeof(rx_def_flow_context_t));
    
    return rx_def_flow_context.rule_base_ext_context.cntr_id;
}

static void _rdd_tm_flow_cntr_id_init(uint32_t entries_num)
{
    uint32_t i;

    RDD_BTRACE("entries_num = %d\n", entries_num);

    for (i = 0; i < entries_num; i++)
    {
        rdd_tm_flow_cntr_cfg(i, TX_FLOW_CNTR_GROUP_INVLID_CNTR);
    }
}

void rdd_tm_flow_cntr_cfg(uint32_t cntr_entry, uint32_t cntr_id)
{
    uint32_t core_index;
    RDD_TM_FLOW_CNTR_TABLE_DTS *tx_flow_table_ptr;
    RDD_TM_FLOW_CNTR_ENTRY_DTS *tx_flow_entry_ptr;
    static uint8_t first_time = 1;

    RDD_BTRACE("cntr_entry = %d, cntr_id = %d\n", cntr_entry, cntr_id);

    if (first_time)
    {
        first_time = 0;
        /* init US/DS tables */
        _rdd_tm_flow_cntr_id_init(RDD_TM_FLOW_CNTR_TABLE_SIZE * 2);
    }

    for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
    {
        /* setting first group - processing */
        if (cntr_entry < RDD_TM_FLOW_CNTR_TABLE_SIZE)
        {
            /* Upstream TX */
            if (rdp_core_to_image_map[core_index] == us_tm_runner_image)
            {
                tx_flow_table_ptr = RDD_TM_FLOW_CNTR_TABLE_PTR(core_index);
                tx_flow_entry_ptr = &(tx_flow_table_ptr->entry[cntr_entry]);
                RDD_TM_FLOW_CNTR_ENTRY_CNTR_ID_WRITE(cntr_id, tx_flow_entry_ptr);
            }
        }
        else
        {
            /* Downstream TX */
            if (rdp_core_to_image_map[core_index] == ds_tm_runner_image)
            {
                tx_flow_table_ptr = RDD_TM_FLOW_CNTR_TABLE_PTR(core_index);
                tx_flow_entry_ptr = &(tx_flow_table_ptr->entry[cntr_entry-RDD_TM_FLOW_CNTR_TABLE_SIZE]);
                RDD_TM_FLOW_CNTR_ENTRY_CNTR_ID_WRITE(cntr_id, tx_flow_entry_ptr);
            }
        }
    }
}

uint32_t rdd_tm_flow_cntr_id_get(uint32_t cntr_entry)
{
    uint32_t cntr_id, core_index;
    RDD_TM_FLOW_CNTR_TABLE_DTS *tx_flow_table_ptr;
    RDD_TM_FLOW_CNTR_ENTRY_DTS *tx_flow_entry_ptr;

    RDD_BTRACE("cntr_entry = %d\n", cntr_entry);

    for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
    {
        /* setting first group - processing */
        if (cntr_entry < RDD_TM_FLOW_CNTR_TABLE_SIZE)
        {
            if (rdp_core_to_image_map[core_index] == us_tm_runner_image)
            {
                tx_flow_table_ptr = RDD_TM_FLOW_CNTR_TABLE_PTR(core_index);
                tx_flow_entry_ptr = &(tx_flow_table_ptr->entry[cntr_entry]);
                RDD_TM_FLOW_CNTR_ENTRY_CNTR_ID_READ(cntr_id, tx_flow_entry_ptr);
                return cntr_id;
            }
        }
        else
        {
            if (rdp_core_to_image_map[core_index] == ds_tm_runner_image)
            {
                tx_flow_table_ptr = RDD_TM_FLOW_CNTR_TABLE_PTR(core_index);
                tx_flow_entry_ptr = &(tx_flow_table_ptr->entry[cntr_entry-RDD_TM_FLOW_CNTR_TABLE_SIZE]);
                RDD_TM_FLOW_CNTR_ENTRY_CNTR_ID_READ(cntr_id, tx_flow_entry_ptr);
                return cntr_id;
            }
        }
    }
    return TX_FLOW_CNTR_GROUP_INVLID_CNTR;
}
#endif

void rdd_fpm_pool_number_mapping_cfg(uint16_t fpm_base_token_size)
{
    uint8_t i;
    uint8_t shifted_fpm_base_num = fpm_base_token_size >> 8;

    /*  This loop fills packet length to FPM pool number mapping
        Table entry i holds the pool number for the following pakcet length:
        256 * (i-1) < packet_length < 256 * i 
        The pool number is resolved using the configured fpm base token size - usually 512 bytes
    */

    for (i = 0; i < shifted_fpm_base_num; i++) 
        RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(0, RDD_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, i);
    for (i = shifted_fpm_base_num; i < shifted_fpm_base_num << 1; i++) 
        RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(1, RDD_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, i);
    for (i = shifted_fpm_base_num << 1; i < shifted_fpm_base_num << 2; i++) 
        RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(2, RDD_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, i);
    for (i = shifted_fpm_base_num << 2; i < RDD_FPM_POOL_NUMBER_MAPPING_TABLE_SIZE; i++) 
        RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(3, RDD_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR, i);
}

