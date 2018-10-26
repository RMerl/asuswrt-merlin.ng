/*
    <:copyright-BRCM:2014-2016:DUAL/GPL:standard
    
       Copyright (c) 2014-2016 Broadcom 
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
#include "rdd_init.h"
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
#include "rdd_crc.h"
#endif

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
uint8_t *g_runner_ddr_base_addr;
#else
uint32_t g_runner_ddr0_base_addr;
uint32_t g_runner_ddr1_base_addr;
#endif
uint8_t *g_runner_psram_base_addr;
uint32_t g_runner_nat_cache_key_ptr;
uint32_t g_runner_nat_cache_context_ptr;
uint32_t g_runner_ddr_phy_iptv_tables_base_ptr;
uint32_t g_runner_ddr0_iptv_lookup_ptr;
uint32_t g_runner_ddr0_iptv_context_ptr;
uint32_t g_runner_ddr0_iptv_ssm_context_ptr;
uint32_t g_cpu_message_queue_write_ptr[2];
uint32_t g_vlan_mapping_command_to_action[RDD_MAX_VLAN_CMD][RDD_MAX_PBITS_CMD];
rdd_vlan_actions_matrix_t *g_vlan_actions_matrix_ptr;
uint32_t g_ddr_packet_headroom_size;
uint32_t g_psram_packet_headroom_size;
rdd_wan_tx_pointers_table_t *wan_tx_pointers_table;
uint32_t g_rate_cntrls_pool_idx;

extern bdmf_fastlock int_lock_irq;
extern bdmf_fastlock cpu_message_lock;

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
uint32_t g_ddr_headroom_size;
rdd_wan_tx_pointers_table_t *wan_tx_pointers_table;
uint8_t *g_runner_extra_ddr_base_addr;
rdd_wan_physical_port_t g_wan_physical_port;
int g_dbg_lvl;
#ifndef WL4908
RDD_CONNECTION_TABLE_DTS *g_ds_connection_table_ptr;
#endif
RDD_FC_MCAST_CONNECTION2_TABLE_DTS *g_fc_mcast_connection2_table_ptr;
uint8_t   g_broadcom_switch_mode = 0;
rdd_bridge_port_t g_broadcom_switch_physical_port = 0;
uint32_t  g_bridge_flow_cache_mode;
uint32_t  g_chip_revision;

typedef struct
{
    bdmf_ipv6_t ipv6_address;
    uint16_t    ref_count;
} ipv6_host_table_t;

/*missing-braces warning is enabled, so can't use {0} initializer. Relying on BSS zero init rule instead.*/
static ipv6_host_table_t g_ipv6_host_table[RDD_IPV6_HOST_ADDRESS_CRC_TABLE_SIZE];
static uint16_t g_ipv4_host_ref_count_table[RDD_IPV4_HOST_ADDRESS_TABLE_SIZE];
#endif /*DSL*/

void _rdd_cam_lookup_init(void)
{
    RUNNER_REGS_CFG_CAM_CFG cam_cfg_reg;

    cam_cfg_reg.stop_value = CAM_STOP_VALUE;

    RUNNER_REGS_0_CFG_CAM_CFG_WRITE(cam_cfg_reg);
    RUNNER_REGS_1_CFG_CAM_CFG_WRITE(cam_cfg_reg);
}

#if defined(DSL_63138) || defined(DSL_63148)
static int rdd_ddr_optimized_base_config(uint8_t  *runner_ddr_pool_ptr,
                                         uint32_t  ddr_packet_headroom_size)
{
    uint32_t  *bpm_ddr_optimized_base_ptr;

    bpm_ddr_optimized_base_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_BPM_DDR_OPTIMIZED_PACKET_BUFFERS_BASE_ADDRESS);

    MWRITE_32(bpm_ddr_optimized_base_ptr, RDD_VIRT_TO_PHYS(runner_ddr_pool_ptr + ddr_packet_headroom_size + RDD_PACKET_DDR_OFFSET));

    bpm_ddr_optimized_base_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_BPM_DDR_OPTIMIZED_PACKET_BUFFERS_BASE_ADDRESS);

    MWRITE_32(bpm_ddr_optimized_base_ptr, RDD_VIRT_TO_PHYS(runner_ddr_pool_ptr + ddr_packet_headroom_size + RDD_PACKET_DDR_OFFSET));

    return BDMF_ERR_OK;
}
#endif

#if defined(WL4908)
static int rdd_ddr_optimized_base_config(uint8_t  *runner_ddr_pool_ptr,
                                         uint32_t  ddr_packet_headroom_size)
{
    uint32_t *bpm_ddr_optimized_base_ptr, *bpm_ddr_base_ptr;
    uint32_t start_addr;

    /* pool#0 */
    bpm_ddr_base_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_BPM_DDR_PACKET_BUFFERS_BASE_ADDRESS);
    MREAD_32(bpm_ddr_base_ptr, start_addr);
    start_addr += ddr_packet_headroom_size + RDD_PACKET_DDR_OFFSET;

    bpm_ddr_optimized_base_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_BPM_DDR_OPTIMIZED_PACKET_BUFFERS_BASE_ADDRESS);
    MWRITE_32(bpm_ddr_optimized_base_ptr, start_addr);
    bpm_ddr_optimized_base_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_BPM_DDR_OPTIMIZED_PACKET_BUFFERS_BASE_ADDRESS);
    MWRITE_32(bpm_ddr_optimized_base_ptr, start_addr);

    /* pool#1 */
    bpm_ddr_base_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_BPM_DDR_1_PACKET_BUFFERS_BASE_ADDRESS);
    MREAD_32(bpm_ddr_base_ptr, start_addr);
    start_addr += ddr_packet_headroom_size + RDD_PACKET_DDR_OFFSET;

    bpm_ddr_optimized_base_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_BPM_DDR_1_OPTIMIZED_PACKET_BUFFERS_BASE_ADDRESS);
    MWRITE_32(bpm_ddr_optimized_base_ptr, start_addr);
    bpm_ddr_optimized_base_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_BPM_DDR_1_OPTIMIZED_PACKET_BUFFERS_BASE_ADDRESS);
    MWRITE_32(bpm_ddr_optimized_base_ptr, start_addr);

    return BDMF_ERR_OK;
}
#endif

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
static void f_rdd_ddr_headroom_size_private_config(uint32_t  ddr_headroom_size)
{
    uint16_t  *headroom_size_ptr;

    headroom_size_ptr = (uint16_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_BPM_DDR_BUFFER_HEADROOM_SIZE_ADDRESS);
    MWRITE_16(headroom_size_ptr, ddr_headroom_size);

    headroom_size_ptr = (uint16_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_BPM_DDR_BUFFER_HEADROOM_SIZE_ADDRESS);
    MWRITE_16(headroom_size_ptr, ddr_headroom_size);
}

#endif /*DSL*/

void rdd_ddr_packet_headroom_size_cfg(uint32_t ddr_packet_headroom_size)
{
    void *addr;
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    unsigned long flags;
#if !defined(FIRMWARE_INIT)
    int rdd_error = BDMF_ERR_OK;
#endif
#endif

    addr = (void *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_DDR_CFG_PACKET_HEADROOM_SIZE_ADDRESS);
    MWRITE_16(addr, ddr_packet_headroom_size);

    addr = (void *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_DDR_CFG_PACKET_HEADROOM_SIZE_ADDRESS);
    MWRITE_16(addr, ddr_packet_headroom_size);

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    /* updating global var */
    g_ddr_headroom_size = ddr_packet_headroom_size;

    /* updating DDR buffers base address (taken from rdd_bpm_initialize()) */
    rdd_ddr_optimized_base_config(g_runner_ddr_base_addr, ddr_packet_headroom_size);

    /* updating actual ddr headroom size to runner memory (taken from f_rdd_ddr_initialize) */
    f_rdd_ddr_headroom_size_private_config(ddr_packet_headroom_size);

    /*sending message to runner to update io memory for dma access*/
    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

#if !defined(FIRMWARE_INIT)
    rdd_error = rdd_cpu_tx_send_message(RDD_CPU_TX_MESSAGE_DDR_HEADROOM_SIZE_SET, FAST_RUNNER_B, RUNNER_PRIVATE_1_OFFSET, 0, 0, 0, 1 /*wait*/);
    if (rdd_error == BDMF_ERR_OK)
        rdd_error = rdd_cpu_tx_send_message(RDD_CPU_TX_MESSAGE_DDR_HEADROOM_SIZE_SET, PICO_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, 0, 0, 0, 1 /*wait*/);

    assert(rdd_error == BDMF_ERR_OK);
#endif /*FIRMWARE_INIT*/
    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
#endif /*DSL_63138 || DSL_63148 || WL4908*/
}

void rdd_psram_packet_headroom_size_cfg(uint32_t psram_packet_headroom_size)
{
}

int rdd_packet_headroom_size_cfg(uint32_t ddr_packet_headroom_size, uint32_t psram_packet_headroom_size)
{
    int rdd_error = BDMF_ERR_OK;

    g_ddr_packet_headroom_size = ddr_packet_headroom_size;
    g_psram_packet_headroom_size = psram_packet_headroom_size;

    rdd_ddr_packet_headroom_size_cfg(ddr_packet_headroom_size);
    rdd_psram_packet_headroom_size_cfg(psram_packet_headroom_size);

    return rdd_error;
}

int rdd_4_bytes_counter_get(uint32_t counter_group, uint32_t counter_num, uint32_t *counter)
{
    uint32_t *pm_counters_buffer_ptr;
    int rdd_error;

    bdmf_fastlock_lock(&cpu_message_lock);

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    rdd_error = rdd_cpu_tx_send_message(RDD_CPU_TX_MESSAGE_PM_COUNTER_GET, PICO_RUNNER_A, RUNNER_PRIVATE_0_OFFSET,
                                          counter_group + counter_num / 16, counter_num % 16, 1, 1 /*wait*/);
#else
    /* read user counter and reset its value */
    rdd_error = _rdd_cpu_message_send(RDD_CPU_MESSAGE_PM_COUNTER_GET, RDD_CLUSTER_0, counter_group + counter_num / 16, counter_num % 16, 1, 1);
#endif

    if (rdd_error != BDMF_ERR_OK)
    {
        bdmf_fastlock_unlock(&cpu_message_lock);
        return rdd_error;
    }

    pm_counters_buffer_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + PM_COUNTERS_BUFFER_ADDRESS);

    MREAD_32(pm_counters_buffer_ptr, *counter);

    bdmf_fastlock_unlock(&cpu_message_lock);
    return rdd_error;
}

int rdd_2_bytes_counter_get(uint32_t counter_group, uint32_t counter_num, uint16_t *counter)
{
    uint16_t *pm_counters_buffer_ptr;
    int rdd_error;

    bdmf_fastlock_lock(&cpu_message_lock);

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    rdd_error = rdd_cpu_tx_send_message(RDD_CPU_TX_MESSAGE_PM_COUNTER_GET, PICO_RUNNER_A, RUNNER_PRIVATE_0_OFFSET,
                                          counter_group + counter_num / 32, counter_num % 32, 0, 1 /*wait*/);
#else
    /* read user counter and reset its value */
    rdd_error = _rdd_cpu_message_send(RDD_CPU_MESSAGE_PM_COUNTER_GET, RDD_CLUSTER_0, counter_group + counter_num / 32, counter_num % 32, 0, 1);
#endif

    if (rdd_error != BDMF_ERR_OK)
    {
        bdmf_fastlock_unlock(&cpu_message_lock);
        return rdd_error;
    }

    pm_counters_buffer_ptr = (uint16_t *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + PM_COUNTERS_BUFFER_ADDRESS);

    MREAD_16(pm_counters_buffer_ptr, *counter);

    bdmf_fastlock_unlock(&cpu_message_lock);
    return rdd_error;
}

void rdd_us_padding_cfg(bdmf_boolean control_enabled, bdmf_boolean cpu_control_enabled, uint16_t size)
{
    RDD_SYSTEM_CONFIGURATION_DTS *system_cfg;
    uint16_t cpu_size;

    /* configuration will be made only in US direction for the time being*/
    system_cfg = (RDD_SYSTEM_CONFIGURATION_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_SYSTEM_CONFIGURATION_ADDRESS);

    if (control_enabled == 0)
    {
        size = 0;
        cpu_size = 0;
    }
    else
    {
        cpu_size = size;

        if (cpu_control_enabled == 0)
            cpu_size = 0;
    }
    RDD_SYSTEM_CONFIGURATION_US_PADDING_MAX_SIZE_WRITE(size, system_cfg);
    RDD_SYSTEM_CONFIGURATION_US_PADDING_CPU_MAX_SIZE_WRITE(cpu_size, system_cfg);
}

int rdd_mtu_cfg(uint16_t mtu_size)
{
    RDD_SYSTEM_CONFIGURATION_DTS  *system_cfg;

    /* configuration will be made only in US direction for the time being*/
    system_cfg = (RDD_SYSTEM_CONFIGURATION_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_SYSTEM_CONFIGURATION_ADDRESS);

    if (mtu_size <= RDD_IPV6_HEADER_SIZE)
        return BDMF_ERR_PARM;

    RDD_SYSTEM_CONFIGURATION_MTU_MINUS_40_WRITE((mtu_size - RDD_IPV6_HEADER_SIZE), system_cfg);

    return BDMF_ERR_OK;
}

#if !defined(DSL_63138) && !defined(DSL_63148) && !defined(WL4908)
int rdd_ds_wan_flow_cfg(uint32_t wan_flow, rdpa_cpu_reason cpu_reason, bdmf_boolean is_pkt_based,
    uint8_t ingress_flow)
{
    RDD_DS_WAN_FLOW_TABLE_DTS *wan_flow_table_ptr;
    RDD_DS_WAN_FLOW_ENTRY_DTS *wan_flow_entry_ptr;

    wan_flow_table_ptr = RDD_DS_WAN_FLOW_TABLE_PTR();
    wan_flow_entry_ptr = &(wan_flow_table_ptr->entry[wan_flow]);

    RDD_DS_WAN_FLOW_ENTRY_INGRESS_FLOW_WRITE(ingress_flow, wan_flow_entry_ptr);
    RDD_DS_WAN_FLOW_ENTRY_INGRESS_CLASSIFY_MODE_WRITE(is_pkt_based, wan_flow_entry_ptr);
    RDD_DS_WAN_FLOW_ENTRY_CPU_REASON_WRITE(cpu_reason, wan_flow_entry_ptr);

    return BDMF_ERR_OK;
}

void rdd_prop_tag_vport_cfg(rdd_runner_group_t *group, rdpa_traffic_dir dir, uint32_t vector)
{
    if (dir == rdpa_dir_ds)
        MWRITE_GROUP_BLOCK_32(group, DS_BRCM_SWITCH_VECTOR_ADDRESS, (void *)&vector, DS_BRCM_SWITCH_VECTOR_BYTE_SIZE);
    else
        MWRITE_GROUP_BLOCK_32(group, US_BRCM_SWITCH_VECTOR_ADDRESS, (void *)&vector, US_BRCM_SWITCH_VECTOR_BYTE_SIZE);
}
#endif /*DSL*/

void rdd_us_wan_flow_cfg(uint32_t wan_flow, rdd_wan_channel_id_t wan_channel, uint32_t wan_port,
    bdmf_boolean crc_calc_en, bdmf_boolean ptm_bonding_enabled, uint8_t pbits_to_queue_table_idx,
    uint8_t tc_to_queue_table_idx)
{
    RDD_US_WAN_FLOW_TABLE_DTS *wan_flow_table;
    RDD_US_WAN_FLOW_ENTRY_DTS *wan_flow_entry;

    wan_flow_table = RDD_US_WAN_FLOW_TABLE_PTR();
    wan_flow_entry = &(wan_flow_table->entry[wan_flow]);

    RDD_US_WAN_FLOW_ENTRY_WAN_PORT_ID_WRITE(wan_port, wan_flow_entry);
    RDD_US_WAN_FLOW_ENTRY_CRC_CALC_WRITE(crc_calc_en, wan_flow_entry);
    RDD_US_WAN_FLOW_ENTRY_WAN_CHANNEL_ID_WRITE(wan_channel, wan_flow_entry);
    RDD_US_WAN_FLOW_ENTRY_PBITS_TO_QUEUE_TABLE_INDEX_WRITE(pbits_to_queue_table_idx, wan_flow_entry);
    RDD_US_WAN_FLOW_ENTRY_TRAFFIC_CLASS_TO_QUEUE_TABLE_INDEX_WRITE(tc_to_queue_table_idx, wan_flow_entry);
}

void rdd_us_wan_flow_get(uint32_t wan_flow, rdd_wan_channel_id_t *wan_channel, uint32_t *wan_port,
    bdmf_boolean *crc_calc_en, uint8_t *pbits_to_queue_table_idx, uint8_t *tc_to_queue_table_idx)
{
    RDD_US_WAN_FLOW_TABLE_DTS *wan_flow_table;
    RDD_US_WAN_FLOW_ENTRY_DTS *wan_flow_entry;

    wan_flow_table = RDD_US_WAN_FLOW_TABLE_PTR();
    wan_flow_entry = &(wan_flow_table->entry[wan_flow]);

    RDD_US_WAN_FLOW_ENTRY_WAN_PORT_ID_READ(*wan_port , wan_flow_entry);
    RDD_US_WAN_FLOW_ENTRY_CRC_CALC_READ(*crc_calc_en, wan_flow_entry);
    RDD_US_WAN_FLOW_ENTRY_WAN_CHANNEL_ID_READ(*wan_channel, wan_flow_entry);
    RDD_US_WAN_FLOW_ENTRY_PBITS_TO_QUEUE_TABLE_INDEX_READ(*pbits_to_queue_table_idx, wan_flow_entry);
    RDD_US_WAN_FLOW_ENTRY_TRAFFIC_CLASS_TO_QUEUE_TABLE_INDEX_READ(*tc_to_queue_table_idx, wan_flow_entry);
}

void rdd_dscp_to_pbits_cfg(rdpa_traffic_dir direction, rdd_vport_id_t vport_id, uint32_t dscp, uint32_t pbits)
{
#if !defined(DSL_63138) && !defined(DSL_63148) && !defined(WL4908)
    RDD_DS_DSCP_TO_PBITS_TABLE_DTS *ds_dscp_to_pbits_table_ptr;
    RDD_US_DSCP_TO_PBITS_TABLE_DTS *us_dscp_to_pbits_table_ptr;
    RDD_BYTE_1_DTS *dscp_to_pbits_entry_ptr;

    if (direction == rdpa_dir_ds)
    {
        ds_dscp_to_pbits_table_ptr = RDD_DS_DSCP_TO_PBITS_TABLE_PTR();

        dscp_to_pbits_entry_ptr = &(ds_dscp_to_pbits_table_ptr->entry[dscp]);
    }
    else
    {
        us_dscp_to_pbits_table_ptr = RDD_US_DSCP_TO_PBITS_TABLE_PTR();

        dscp_to_pbits_entry_ptr = &(us_dscp_to_pbits_table_ptr->entry[vport_id][dscp]);
    }

    MWRITE_8(dscp_to_pbits_entry_ptr, pbits);
#else
    return;
#endif
}

/* The reserve FFI table is a byte lookup table used by Runner FW that returns the bit position of the highest '1' of the given byte key. */
void rdd_reverse_ffi_table_init(void)
{
    uint32_t i;
    RDD_REVERSE_FFI_TABLE_DTS *reverse_ffi_table = RDD_REVERSE_FFI_TABLE_PTR();

    /* Assumption is that this table has 256 entries. Otherwise precondition for below algo fails. */
    reverse_ffi_table->entry[0].bits = 8;
  
    for (i = 1; i < RDD_REVERSE_FFI_TABLE_SIZE; ++i)
    {
        uint32_t exp = 8;

        /* This loop finds the bit position of the highest '1' in i. */
        while ((1 << exp) > i)
            --exp;

        MWRITE_8(&(reverse_ffi_table->entry[i]), exp);
    }
}

#define LAYER2_HEADER_COPY_ROUTINE_ARRAY(var, runner, prefix) \
{ \
    var[0]  = ADDRESS_OF(runner, prefix##_14_bytes_8_bytes_align), \
    var[1]  = ADDRESS_OF(runner, prefix##_18_bytes_8_bytes_align), \
    var[2]  = ADDRESS_OF(runner, prefix##_22_bytes_8_bytes_align), \
    var[3]  = ADDRESS_OF(runner, prefix##_26_bytes_8_bytes_align), \
    var[4]  = ADDRESS_OF(runner, prefix##_30_bytes_8_bytes_align), \
    var[5]  = ADDRESS_OF(runner, prefix##_14_bytes_4_bytes_align), \
    var[6]  = ADDRESS_OF(runner, prefix##_18_bytes_4_bytes_align), \
    var[7]  = ADDRESS_OF(runner, prefix##_22_bytes_4_bytes_align), \
    var[8]  = ADDRESS_OF(runner, prefix##_26_bytes_4_bytes_align), \
    var[9]  = ADDRESS_OF(runner, prefix##_30_bytes_4_bytes_align), \
    var[10] = ADDRESS_OF(runner, prefix##_14_bytes_8_bytes_align), \
    var[11] = ADDRESS_OF(runner, prefix##_18_bytes_8_bytes_align), \
    var[12] = ADDRESS_OF(runner, prefix##_22_bytes_8_bytes_align), \
    var[13] = ADDRESS_OF(runner, prefix##_26_bytes_8_bytes_align), \
    var[14] = ADDRESS_OF(runner, prefix##_30_bytes_8_bytes_align), \
    var[15] = ADDRESS_OF(runner, prefix##_14_bytes_4_bytes_align), \
    var[16] = ADDRESS_OF(runner, prefix##_18_bytes_4_bytes_align), \
    var[17] = ADDRESS_OF(runner, prefix##_22_bytes_4_bytes_align), \
    var[18] = ADDRESS_OF(runner, prefix##_26_bytes_4_bytes_align), \
    var[19] = ADDRESS_OF(runner, prefix##_30_bytes_4_bytes_align), \
    var[20] = ADDRESS_OF(runner, prefix##_14_bytes_8_bytes_align), \
    var[21] = ADDRESS_OF(runner, prefix##_18_bytes_8_bytes_align), \
    var[22] = ADDRESS_OF(runner, prefix##_22_bytes_8_bytes_align), \
    var[23] = ADDRESS_OF(runner, prefix##_26_bytes_8_bytes_align), \
    var[24] = ADDRESS_OF(runner, prefix##_30_bytes_8_bytes_align), \
    var[25] = ADDRESS_OF(runner, prefix##_14_bytes_4_bytes_align), \
    var[26] = ADDRESS_OF(runner, prefix##_18_bytes_4_bytes_align), \
    var[27] = ADDRESS_OF(runner, prefix##_22_bytes_4_bytes_align), \
    var[28] = ADDRESS_OF(runner, prefix##_26_bytes_4_bytes_align), \
    var[29] = ADDRESS_OF(runner, prefix##_30_bytes_4_bytes_align), \
};

#define LAYER2_HEADER_COPY_DST_OFFSET_ARRAY(var, offset) \
{ \
    var[0]  = offset - 2,  \
    var[1]  = offset - 10, \
    var[2]  = offset - 10, \
    var[3]  = offset - 18, \
    var[4]  = offset - 18, \
    var[5]  = offset - 2,  \
    var[6]  = offset - 2,  \
    var[7]  = offset - 10, \
    var[8]  = offset - 10, \
    var[9]  = offset - 18, \
    var[10] = offset + 6,  \
    var[11] = offset - 2,  \
    var[12] = offset - 2,  \
    var[13] = offset - 10, \
    var[14] = offset - 10, \
    var[15] = offset + 6,  \
    var[16] = offset + 6,  \
    var[17] = offset - 2,  \
    var[18] = offset - 2,  \
    var[19] = offset - 10, \
    var[20] = offset + 14, \
    var[21] = offset + 6,  \
    var[22] = offset + 6,  \
    var[23] = offset - 2,  \
    var[24] = offset - 2,  \
    var[25] = offset + 14, \
    var[26] = offset + 14, \
    var[27] = offset + 6,  \
    var[28] = offset + 6,  \
    var[29] = offset - 2,  \
};

#if !defined(DSL_63138) && !defined(DSL_63148) && !defined(WL4908)
void rdd_layer2_header_copy_mapping_init(void)
{
    RDD_DS_MAIN_LAYER2_HEADER_COPY_MAPPING_TABLE_DTS *ds_main_layer2_header_copy_mapping_table_ptr;
    RDD_DS_PICO_LAYER2_HEADER_COPY_MAPPING_TABLE_DTS *ds_pico_layer2_header_copy_mapping_table_ptr;
    RDD_US_MAIN_LAYER2_HEADER_COPY_MAPPING_TABLE_DTS *us_main_layer2_header_copy_mapping_table_ptr;
    RDD_US_PICO_LAYER2_HEADER_COPY_MAPPING_TABLE_DTS *us_pico_layer2_header_copy_mapping_table_ptr;
    RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY_DTS *layer2_header_copy_mapping_entry_ptr;
    uint16_t ds_main_layer2_header_copy_routine_arr[] = LAYER2_HEADER_COPY_ROUTINE_ARRAY(ds_main_layer2_header_copy_routine_arr, runner_a, ds_processing_layer2_header_copy);
    uint16_t us_main_layer2_header_copy_routine_arr[] = LAYER2_HEADER_COPY_ROUTINE_ARRAY(us_main_layer2_header_copy_routine_arr, runner_b, us_processing_layer2_header_copy);
    uint16_t ds_pico_layer2_header_copy_routine_arr[] = LAYER2_HEADER_COPY_ROUTINE_ARRAY(ds_pico_layer2_header_copy_routine_arr, runner_c, ds_processing_layer2_header_copy);
    uint16_t us_pico_layer2_header_copy_routine_arr[] = LAYER2_HEADER_COPY_ROUTINE_ARRAY(us_pico_layer2_header_copy_routine_arr, runner_d, us_processing_layer2_header_copy);
    uint16_t ds_layer2_header_copy_dst_offset_arr[] = LAYER2_HEADER_COPY_DST_OFFSET_ARRAY(ds_layer2_header_copy_dst_offset_arr, RDD_DS_IH_PACKET_HEADROOM_OFFSET);
    uint16_t us_layer2_header_copy_dst_offset_arr[] = LAYER2_HEADER_COPY_DST_OFFSET_ARRAY(us_layer2_header_copy_dst_offset_arr, RDD_US_IH_PACKET_HEADROOM_OFFSET);
    uint16_t i;

    ds_main_layer2_header_copy_mapping_table_ptr = RDD_DS_MAIN_LAYER2_HEADER_COPY_MAPPING_TABLE_PTR();

    for (i = 0; i < RDD_DS_MAIN_LAYER2_HEADER_COPY_MAPPING_TABLE_SIZE; i++) {
        layer2_header_copy_mapping_entry_ptr = &(ds_main_layer2_header_copy_mapping_table_ptr->entry[i]);

        RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY_ROUTINE_WRITE(ds_main_layer2_header_copy_routine_arr[i], layer2_header_copy_mapping_entry_ptr);
        RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY_DST_BUFFER_OFFSET_WRITE(ds_layer2_header_copy_dst_offset_arr[i], layer2_header_copy_mapping_entry_ptr);
    }

    ds_pico_layer2_header_copy_mapping_table_ptr = RDD_DS_PICO_LAYER2_HEADER_COPY_MAPPING_TABLE_PTR();

    for (i = 0; i < RDD_DS_PICO_LAYER2_HEADER_COPY_MAPPING_TABLE_SIZE; i++) {
        layer2_header_copy_mapping_entry_ptr = &(ds_pico_layer2_header_copy_mapping_table_ptr->entry[i]);

        RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY_ROUTINE_WRITE(ds_pico_layer2_header_copy_routine_arr[i], layer2_header_copy_mapping_entry_ptr);
        RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY_DST_BUFFER_OFFSET_WRITE(ds_layer2_header_copy_dst_offset_arr[i], layer2_header_copy_mapping_entry_ptr);
    }

    us_main_layer2_header_copy_mapping_table_ptr = RDD_US_MAIN_LAYER2_HEADER_COPY_MAPPING_TABLE_PTR();

    for (i = 0; i < RDD_US_MAIN_LAYER2_HEADER_COPY_MAPPING_TABLE_SIZE; i++) {
        layer2_header_copy_mapping_entry_ptr = &(us_main_layer2_header_copy_mapping_table_ptr->entry[i]);

        RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY_ROUTINE_WRITE(us_main_layer2_header_copy_routine_arr[i], layer2_header_copy_mapping_entry_ptr);
        RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY_DST_BUFFER_OFFSET_WRITE(us_layer2_header_copy_dst_offset_arr[i], layer2_header_copy_mapping_entry_ptr);
    }

    us_pico_layer2_header_copy_mapping_table_ptr = RDD_US_PICO_LAYER2_HEADER_COPY_MAPPING_TABLE_PTR();

    for (i = 0; i < RDD_US_PICO_LAYER2_HEADER_COPY_MAPPING_TABLE_SIZE; i++) {
        layer2_header_copy_mapping_entry_ptr = &(us_pico_layer2_header_copy_mapping_table_ptr->entry[i]);

        RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY_ROUTINE_WRITE(us_pico_layer2_header_copy_routine_arr[i], layer2_header_copy_mapping_entry_ptr);
        RDD_LAYER2_HEADER_COPY_MAPPING_ENTRY_DST_BUFFER_OFFSET_WRITE(us_layer2_header_copy_dst_offset_arr[i], layer2_header_copy_mapping_entry_ptr);
    }
}
#endif /*DSL*/

int rdd_timer_task_config(rdpa_traffic_dir direction, uint16_t task_period_in_usec, uint16_t firmware_routine_address_id)
{
#if !defined(DSL_63138) && !defined(DSL_63148) && !defined(WL4908)
    RDD_TIMER_TASK_DESCRIPTOR_TABLE_DTS *timer_tasks_table_ptr;
    RDD_TIMER_TASK_DESCRIPTOR_ENTRY_DTS *timer_tasks_entry_ptr;
    RDD_TIMER_CONTROL_DESCRIPTOR_DTS *timer_control_descriptor_ptr;
#if !defined(FIRMWARE_INIT)
    RUNNER_REGS_CFG_CPU_WAKEUP runner_cpu_wakeup_register;
#endif
    uint16_t number_of_active_tasks;
    uint16_t task_period_reload;

    if ((task_period_in_usec % TIMER_SCHEDULER_TASK_PERIOD) != 0)
        return BDMF_ERR_PARM;

    if (direction == rdpa_dir_ds)
    {
        if (firmware_routine_address_id == CPU_RX_METER_BUDGET_ALLOCATE_CODE_ID)
        {
            timer_control_descriptor_ptr = (RDD_TIMER_CONTROL_DESCRIPTOR_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_TIMER_CONTROL_DESCRIPTOR_ADDRESS);

            timer_tasks_table_ptr = (RDD_TIMER_TASK_DESCRIPTOR_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_TIMER_TASK_DESCRIPTOR_TABLE_ADDRESS);
        }
    }
    else
    {
        if (firmware_routine_address_id == CPU_RX_METER_BUDGET_ALLOCATE_CODE_ID)
        {
            timer_control_descriptor_ptr = (RDD_TIMER_CONTROL_DESCRIPTOR_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_TIMER_CONTROL_DESCRIPTOR_ADDRESS);

            timer_tasks_table_ptr = (RDD_TIMER_TASK_DESCRIPTOR_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_TIMER_TASK_DESCRIPTOR_TABLE_ADDRESS);
        }
    }

    RDD_TIMER_CONTROL_DESCRIPTOR_NUMBER_OF_ACTIVE_TASKS_READ(number_of_active_tasks, timer_control_descriptor_ptr);

    if (number_of_active_tasks == RDD_TIMER_TASK_DESCRIPTOR_TABLE_SIZE)
        return BDMF_ERR_RANGE;

    if (firmware_routine_address_id == CPU_RX_METER_BUDGET_ALLOCATE_CODE_ID)
        timer_tasks_entry_ptr = &timer_tasks_table_ptr->entry[number_of_active_tasks];

    RDD_TIMER_TASK_DESCRIPTOR_ENTRY_FIRMWARE_PTR_WRITE(firmware_routine_address_id, timer_tasks_entry_ptr);

    task_period_reload = task_period_in_usec / TIMER_SCHEDULER_TASK_PERIOD;

    RDD_TIMER_TASK_DESCRIPTOR_ENTRY_COUNTER_RELOAD_WRITE(task_period_reload, timer_tasks_entry_ptr);
    RDD_TIMER_TASK_DESCRIPTOR_ENTRY_PERIOD_WRITE(1, timer_tasks_entry_ptr);

    number_of_active_tasks++;

    RDD_TIMER_CONTROL_DESCRIPTOR_NUMBER_OF_ACTIVE_TASKS_WRITE(number_of_active_tasks, timer_control_descriptor_ptr);

#if !defined(FIRMWARE_INIT)
    if (number_of_active_tasks == 1)
    {
        runner_cpu_wakeup_register.req_trgt = TIMER_SCHEDULER_THREAD_NUMBER / 32;
        runner_cpu_wakeup_register.thread_num = TIMER_SCHEDULER_THREAD_NUMBER % 32;

        if (direction == rdpa_dir_ds)
            RUNNER_REGS_0_CFG_CPU_WAKEUP_WRITE(runner_cpu_wakeup_register);
        else
            RUNNER_REGS_1_CFG_CPU_WAKEUP_WRITE(runner_cpu_wakeup_register);
    }
#endif
#else /*DSL:*/
    RDD_TIMER_CONTROL_DESCRIPTOR_DTS       *timer_control_descriptor_ptr;
    RDD_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_DTS    *timer_tasks_table_ptr;
    RDD_TIMER_TASK_DESCRIPTOR_ENTRY_DTS    *timer_tasks_entry_ptr;
    RDD_SYSTEM_CONFIGURATION_DTS           *system_cfg;
#if !defined(FIRMWARE_INIT)
    RUNNER_REGS_CFG_CPU_WAKEUP             runner_cpu_wakeup_register;
#endif
    uint16_t                               number_of_active_tasks;
    uint16_t                               task_period_reload;

    if ((task_period_in_usec % TIMER_SCHEDULER_TASK_PERIOD) != 0)
    {
        return BDMF_ERR_PARM;
    }

    if (direction == rdpa_dir_ds)
    {
        system_cfg = (RDD_SYSTEM_CONFIGURATION_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_SYSTEM_CONFIGURATION_ADDRESS);

        if ((firmware_routine_address_id == CPU_RX_METER_BUDGET_ALLOCATE_CODE_ID) ||
             (firmware_routine_address_id == FREE_SKB_INDEX_ALLOCATE_CODE_ID))
        {
            timer_control_descriptor_ptr = (RDD_TIMER_CONTROL_DESCRIPTOR_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_MAIN_TIMER_CONTROL_DESCRIPTOR_ADDRESS);

            timer_tasks_table_ptr = (RDD_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_ADDRESS);
        }
        else
        {
            timer_control_descriptor_ptr = (RDD_TIMER_CONTROL_DESCRIPTOR_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_PICO_TIMER_CONTROL_DESCRIPTOR_ADDRESS);

            timer_tasks_table_ptr = (RDD_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_PICO_TIMER_TASK_DESCRIPTOR_TABLE_ADDRESS);
        }
    }
    else
    {
        system_cfg = (RDD_SYSTEM_CONFIGURATION_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_SYSTEM_CONFIGURATION_ADDRESS);

        if ((firmware_routine_address_id == CPU_RX_METER_BUDGET_ALLOCATE_CODE_ID) || (firmware_routine_address_id == UPSTREAM_RATE_LIMITER_BUDGET_ALLOCATE_CODE_ID) ||
             (firmware_routine_address_id == FREE_SKB_INDEX_ALLOCATE_CODE_ID))
        {
            timer_control_descriptor_ptr = (RDD_TIMER_CONTROL_DESCRIPTOR_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_MAIN_TIMER_CONTROL_DESCRIPTOR_ADDRESS);

            timer_tasks_table_ptr = (RDD_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_ADDRESS);
        }
        else
        {
            timer_control_descriptor_ptr = (RDD_TIMER_CONTROL_DESCRIPTOR_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_PICO_TIMER_CONTROL_DESCRIPTOR_ADDRESS);

            timer_tasks_table_ptr = (RDD_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_PICO_TIMER_TASK_DESCRIPTOR_TABLE_ADDRESS);
        }
    }

    /* Want the timer task to have a period of 1000us */
#ifdef RUNNER_FWTRACE
    RDD_SYSTEM_CONFIGURATION_TIMER_SCHEDULER_PERIOD_WRITE((TIMER_SCHEDULER_TASK_PERIOD*(1000/TIMER_PERIOD_NS)) - 1, system_cfg);
#else
    RDD_SYSTEM_CONFIGURATION_TIMER_SCHEDULER_PERIOD_WRITE(TIMER_SCHEDULER_TASK_PERIOD - 1, system_cfg);
#endif

    RDD_TIMER_CONTROL_DESCRIPTOR_NUMBER_OF_ACTIVE_TASKS_READ(number_of_active_tasks, timer_control_descriptor_ptr);

    if (number_of_active_tasks == RDD_NUMBER_OF_TIMER_TASKS)
    {
        return BDMF_ERR_NORES;
    }

    timer_tasks_entry_ptr = &timer_tasks_table_ptr->entry[number_of_active_tasks];

    RDD_TIMER_TASK_DESCRIPTOR_ENTRY_FIRMWARE_PTR_WRITE(firmware_routine_address_id, timer_tasks_entry_ptr);

    task_period_reload = task_period_in_usec / TIMER_SCHEDULER_TASK_PERIOD;

    RDD_TIMER_TASK_DESCRIPTOR_ENTRY_COUNTER_RELOAD_WRITE(task_period_reload, timer_tasks_entry_ptr);
    RDD_TIMER_TASK_DESCRIPTOR_ENTRY_PERIOD_WRITE(1, timer_tasks_entry_ptr);

    number_of_active_tasks++;

    RDD_TIMER_CONTROL_DESCRIPTOR_NUMBER_OF_ACTIVE_TASKS_WRITE(number_of_active_tasks, timer_control_descriptor_ptr);

#if !defined(FIRMWARE_INIT)
    if (number_of_active_tasks == 1)
    {
        if ((firmware_routine_address_id == UPSTREAM_INGRESS_RATE_LIMITER_BUDGET_ALLOCATE_CODE_ID) ||
             (firmware_routine_address_id == UPSTREAM_QUASI_BUDGET_ALLOCATE_CODE_ID) ||
             (firmware_routine_address_id == DOWNSTREAM_RATE_LIMITER_BUDGET_ALLOCATE_CODE_ID))
        {
            runner_cpu_wakeup_register.req_trgt = TIMER_SCHEDULER_PICO_THREAD_NUMBER / 32;
            runner_cpu_wakeup_register.thread_num = TIMER_SCHEDULER_PICO_THREAD_NUMBER % 32;
        }
        else
        {
            runner_cpu_wakeup_register.req_trgt = TIMER_SCHEDULER_MAIN_THREAD_NUMBER / 32;
            runner_cpu_wakeup_register.thread_num = TIMER_SCHEDULER_MAIN_THREAD_NUMBER % 32;
        }

        if (direction == rdpa_dir_ds)
        {
            RUNNER_REGS_0_CFG_CPU_WAKEUP_WRITE(runner_cpu_wakeup_register);
        }
        else
        {
            RUNNER_REGS_1_CFG_CPU_WAKEUP_WRITE(runner_cpu_wakeup_register);
        }
    }
#endif
#endif /*DSL*/

    return BDMF_ERR_OK;
}

void rdd_interrupt_mask(uint32_t intr, uint32_t sub_intr)
{
#if !defined(FIRMWARE_INIT)
    unsigned long flags;
    RUNNER_REGS_CFG_INT_MASK int_mask_reg;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    RUNNER_REGS_0_CFG_INT_MASK_READ(int_mask_reg);

    /* mask the interrupt by writing "1" to the corresponding bit in the Runner's interrupt register */
    switch (intr)
    {
    case 0:
        int_mask_reg.int0_mask &= ~(1 << sub_intr);
        break;
    case 1:
        int_mask_reg.int1_mask &= ~(1 << sub_intr);
        break;
    case 2:
        int_mask_reg.int2_mask = 0;
        break;
    case 3:
        int_mask_reg.int3_mask = 0;
        break;
    case 4:
        int_mask_reg.int4_mask = 0;
        break;
    case 5:
        int_mask_reg.int5_mask = 0;
        break;
    case 6:
        int_mask_reg.int6_mask = 0;
        break;
    case 7:
        int_mask_reg.int7_mask = 0;
        break;
    case 8:
        int_mask_reg.int8_mask = 0;
        break;
    case 9:
        int_mask_reg.int9_mask = 0;
        break;
    default:
        break;
    }

    RUNNER_REGS_0_CFG_INT_MASK_WRITE(int_mask_reg);
    RUNNER_REGS_1_CFG_INT_MASK_WRITE(int_mask_reg);

    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
#endif
}

void rdd_interrupt_unmask(uint32_t intr, uint32_t sub_intr)
{
#if !defined(FIRMWARE_INIT)
    unsigned long flags;
    RUNNER_REGS_CFG_INT_MASK int_mask_reg;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    RUNNER_REGS_0_CFG_INT_MASK_READ(int_mask_reg);

    /* mask the interrupt by writing "1" to the corresponding bit in the Runner's interrupt register */
    switch (intr)
    {
    case 0:
        int_mask_reg.int0_mask |= (1 << sub_intr);
        break;
    case 1:
        int_mask_reg.int1_mask |= (1 << sub_intr);
        break;
    case 2:
        int_mask_reg.int2_mask = 1;
        break;
    case 3:
        int_mask_reg.int3_mask = 1;
        break;
    case 4:
        int_mask_reg.int4_mask = 1;
        break;
    case 5:
        int_mask_reg.int5_mask = 1;
        break;
    case 6:
        int_mask_reg.int6_mask = 1;
        break;
    case 7:
        int_mask_reg.int7_mask = 1;
        break;
    case 8:
        int_mask_reg.int8_mask = 1;
        break;
    case 9:
        int_mask_reg.int9_mask = 1;
        break;
    default:
        break;
    }

    RUNNER_REGS_0_CFG_INT_MASK_WRITE(int_mask_reg);
    RUNNER_REGS_1_CFG_INT_MASK_WRITE(int_mask_reg);

    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
#endif
}

void rdd_interrupt_vector_get(uint32_t intr, uint8_t *sub_intr_vector)
{
#if !defined(FIRMWARE_INIT)
    RUNNER_REGS_CFG_INT_CTRL int_ctrl_0_reg, int_ctrl_1_reg;
    RUNNER_REGS_CFG_INT_MASK int_mask_reg;

    RUNNER_REGS_0_CFG_INT_CTRL_READ(int_ctrl_0_reg);
    RUNNER_REGS_1_CFG_INT_CTRL_READ(int_ctrl_1_reg);

    RUNNER_REGS_0_CFG_INT_MASK_READ(int_mask_reg);
    if (intr == 0)
        *sub_intr_vector = (int_ctrl_0_reg.int0_sts | int_ctrl_1_reg.int0_sts) & int_mask_reg.int0_mask;
    else
        *sub_intr_vector = (int_ctrl_0_reg.int1_sts | int_ctrl_1_reg.int1_sts) & int_mask_reg.int1_mask;
#else
    *sub_intr_vector = 0;
#endif
}

void rdd_interrupt_clear(uint32_t intr, uint32_t sub_intr)
{
#if !defined(FIRMWARE_INIT)
    RUNNER_REGS_CFG_INT_CTRL int_ctrl_reg;

    RDD_CLEAR_REGISTER(&int_ctrl_reg);

    /* clear the interrupt by writing "1" to the corresponding bit in the Runner's interrupt register */
    switch (intr)
    {
    case 0:
        int_ctrl_reg.int0_sts = 1 << sub_intr;
        break;
    case 1:
        int_ctrl_reg.int1_sts = 1 << sub_intr;
        break;
    case 2:
        int_ctrl_reg.int2_sts = 1;
        break;
    case 3:
        int_ctrl_reg.int3_sts = 1;
        break;
    case 4:
        int_ctrl_reg.int4_sts = 1;
        break;
    case 5:
        int_ctrl_reg.int5_sts = 1;
        break;
    case 6:
        int_ctrl_reg.int6_sts = 1;
        break;
    case 7:
        int_ctrl_reg.int7_sts = 1;
        break;
    case 8:
        int_ctrl_reg.int8_sts = 1;
        break;
    case 9:
        int_ctrl_reg.int9_sts = 1;
        break;
    }

    RUNNER_REGS_0_CFG_INT_CTRL_WRITE(int_ctrl_reg);
    RUNNER_REGS_1_CFG_INT_CTRL_WRITE(int_ctrl_reg);
#endif
}

void rdd_interrupt_mask_get(uint32_t intr, uint8_t *sub_intr_mask)
{
#if !defined(FIRMWARE_INIT)
    RUNNER_REGS_CFG_INT_MASK int_mask_reg;

    if (intr == 0)
    {
        RUNNER_REGS_0_CFG_INT_MASK_READ(int_mask_reg);
        *sub_intr_mask = int_mask_reg.int0_mask;
    }
    else
    {
        RUNNER_REGS_1_CFG_INT_MASK_READ(int_mask_reg);
        *sub_intr_mask = int_mask_reg.int1_mask;
    }
#else
    *sub_intr_mask = 0;
#endif
}

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
int rdd_ipv4_host_address_table_set(uint32_t table_index, 
                                    bdmf_ipv4 ipv4_host_addr,
                                    uint16_t ref_count)
{
    RDD_IPV4_HOST_ADDRESS_TABLE_DTS *host_table = RDD_IPV4_HOST_ADDRESS_TABLE_PTR();

    if (table_index >= RDD_IPV4_HOST_ADDRESS_TABLE_SIZE)
    {
      return BDMF_ERR_RANGE;
    }

    MWRITE_32(host_table->entry + table_index, ipv4_host_addr);
    g_ipv4_host_ref_count_table[table_index] = ref_count;

    return BDMF_ERR_OK;
}

int rdd_ipv4_host_address_table_get(uint32_t table_index, 
                                    bdmf_ipv4 *ipv4_host_addr,
                                    uint16_t *ref_count)
{
    RDD_IPV4_HOST_ADDRESS_TABLE_DTS *host_table = RDD_IPV4_HOST_ADDRESS_TABLE_PTR();

    if (table_index >= RDD_IPV4_HOST_ADDRESS_TABLE_SIZE)
    {
        return BDMF_ERR_RANGE;
    }

    /*Retrieve host address from RDD table*/
    MREAD_32(host_table->entry + table_index, *ipv4_host_addr);
    /*Retrieve reference count from local table*/
    *ref_count = g_ipv4_host_ref_count_table[table_index];

    return BDMF_ERR_OK;
}
 
int rdd_ipv6_host_address_table_set(uint32_t table_index, 
                                    const bdmf_ipv6_t *ipv6_host_addr,
                                    uint16_t ref_count)
{
    RDD_IPV6_HOST_ADDRESS_CRC_TABLE_DTS *host_table = RDD_IPV6_HOST_ADDRESS_CRC_TABLE_PTR();
    uint32_t ipv6_crc, crc_init_value;

    if (table_index >= RDD_IPV6_HOST_ADDRESS_CRC_TABLE_SIZE)
    {
        return BDMF_ERR_RANGE;
    }

    /*Reduce IPV6 address to a 32-bit value using CRC. This reduced value is what RDP FW will be using for lookup.*/
    crc_init_value = rdd_crc_init_value_get(RDD_CRC_TYPE_32);
    ipv6_crc = rdd_crc_bit_by_bit((uint8_t *)ipv6_host_addr->data, 16, 0, crc_init_value, RDD_CRC_TYPE_32);

    /*Store ipv6 address in a local table so we can return in the get accessor*/
    g_ipv6_host_table[table_index].ipv6_address = *ipv6_host_addr;
    g_ipv6_host_table[table_index].ref_count = ref_count;

    /*Store the CRC in the RDP FW table*/
    MWRITE_32(host_table->entry + table_index, ipv6_crc);

    return BDMF_ERR_OK;
}

int rdd_ipv6_host_address_table_get(uint32_t table_index, 
                                    bdmf_ipv6_t *ipv6_host_addr,
                                    uint16_t *ref_count)
{
    if (table_index >= RDD_IPV6_HOST_ADDRESS_CRC_TABLE_SIZE)
    {
        return BDMF_ERR_RANGE;
    }

    /*Look up address in local table. The full IP address is not stored in an RDP table, only the CRC is.*/
    *ipv6_host_addr = g_ipv6_host_table[table_index].ipv6_address;
    *ref_count = g_ipv6_host_table[table_index].ref_count;

    return BDMF_ERR_OK;
}

int rdd_lan_get_stats(uint32_t lan_port,
                      uint32_t *rx_packet,
                      uint32_t *tx_packet,
                      uint16_t *tx_discard)
{
    int rdd_error = BDMF_ERR_OK;

    uint32_t counter_offset = lan_port + 1;  /* skip the first one which is used for WAN */
    
    rdd_error = rdd_2_bytes_counter_get(BRIDGE_DOWNSTREAM_TX_CONGESTION_GROUP, counter_offset, tx_discard);
    if (!rdd_error)
        rdd_error = rdd_4_bytes_counter_get(LAN_TX_PACKETS_GROUP, counter_offset + 8, tx_packet);
    if (!rdd_error)
        rdd_error = rdd_4_bytes_counter_get(LAN_RX_PACKETS_GROUP, counter_offset + 8, rx_packet);

    return rdd_error;
}
#endif /*DSL*/
