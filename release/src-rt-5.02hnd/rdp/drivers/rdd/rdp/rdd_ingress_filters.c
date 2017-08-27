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
#include "rdd_ingress_filters.h"

uint32_t g_ether_type_filter_mode[2][8];
uint32_t g_anti_spoofing_last_rule_idx[RDD_SRC_MAC_ANTI_SPOOFING_TABLE_SIZE] = {0};
uint32_t g_ipv4_address_last_idx = 0;
uint32_t g_ipv6_address_last_idx = 0;

#define RDD_INIT_INGRESS_FILTER_ENTRY(filter_entry, filter_lookup_table, profile_idx, filter_type, filter_name) \
    do { \
        filter_entry = &(filter_lookup_table->entry[profile_idx][FILTER_##filter_name]); \
        rdd_ingress_filter_cam_entry_disable(filter_entry); \
        RDD_INGRESS_FILTERS_LOOKUP_ENTRY_##filter_type##_WRITE(PARSER_##filter_type##_##filter_name, filter_entry); \
        RDD_INGRESS_FILTERS_LOOKUP_ENTRY_##filter_type##_MASK_WRITE(PARSER_##filter_type##_MASK, filter_entry); \
    } while (0)

#define RDD_LOCAL_SWITCHING_FILTER_CFG(filters_mode, filters_cfg_entry_ptr, mode) \
    do { \
        uint32_t filters_cfg_entry; \
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_READ(filters_cfg_entry, filters_cfg_entry_ptr); \
        filters_mode = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_READ(filters_cfg_entry) | \
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ(filters_cfg_entry) | mode; \
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_LOCAL_SWITCHING_INGRESS_FILTERS_WRITE(filters_mode ? MODE_ENABLED : \
            MODE_DISABLED, filters_cfg_entry_ptr); \
    } while (0)

void rdd_ingress_filter_cam_entry_enable(RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS *filters_cam_entry_ptr)
{
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_PTAG_WRITE(0, filters_cam_entry_ptr);
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_PTAG_MASK_WRITE(0, filters_cam_entry_ptr);
}

void rdd_ingress_filter_cam_entry_disable(RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS *filters_cam_entry_ptr)
{
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_PTAG_WRITE(1, filters_cam_entry_ptr);
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_PTAG_MASK_WRITE(0, filters_cam_entry_ptr);
}

void rdd_ingress_filters_cam_tables_init(rdpa_traffic_dir dir)
{
    RDD_INGRESS_FILTERS_LOOKUP_TABLE_DTS *filter_lookup_table;
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS *filter_lookup_entry;
    uint32_t profile_idx;

    memset(g_ether_type_filter_mode, 0, 16);

    SET_DTS_PRIVATE_MEM_GEN_PTR(dir, filter_lookup_table, INGRESS_FILTERS_LOOKUP_TABLE);

    for (profile_idx = 0; profile_idx < RDD_INGRESS_FILTERS_LOOKUP_TABLE_SIZE; profile_idx++)
    {
        RDD_INIT_INGRESS_FILTER_ENTRY(filter_lookup_entry, filter_lookup_table, profile_idx, L4_PROTOCOL, IGMP);
        RDD_INIT_INGRESS_FILTER_ENTRY(filter_lookup_entry, filter_lookup_table, profile_idx, L4_PROTOCOL, ICMPV6);
        RDD_INIT_INGRESS_FILTER_ENTRY(filter_lookup_entry, filter_lookup_table, profile_idx, L2_PROTOCOL, USER_DEFINED_0);
        RDD_INIT_INGRESS_FILTER_ENTRY(filter_lookup_entry, filter_lookup_table, profile_idx, L2_PROTOCOL, USER_DEFINED_1);
        RDD_INIT_INGRESS_FILTER_ENTRY(filter_lookup_entry, filter_lookup_table, profile_idx, L2_PROTOCOL, USER_DEFINED_2);
        RDD_INIT_INGRESS_FILTER_ENTRY(filter_lookup_entry, filter_lookup_table, profile_idx, L2_PROTOCOL, USER_DEFINED_3);
        RDD_INIT_INGRESS_FILTER_ENTRY(filter_lookup_entry, filter_lookup_table, profile_idx, L2_PROTOCOL, PPPOE_D);
        RDD_INIT_INGRESS_FILTER_ENTRY(filter_lookup_entry, filter_lookup_table, profile_idx, L2_PROTOCOL, PPPOE_S);
        RDD_INIT_INGRESS_FILTER_ENTRY(filter_lookup_entry, filter_lookup_table, profile_idx, L2_PROTOCOL, ARP);
        RDD_INIT_INGRESS_FILTER_ENTRY(filter_lookup_entry, filter_lookup_table, profile_idx, L2_PROTOCOL, _1588);
        RDD_INIT_INGRESS_FILTER_ENTRY(filter_lookup_entry, filter_lookup_table, profile_idx, L2_PROTOCOL, _802_1X);
        RDD_INIT_INGRESS_FILTER_ENTRY(filter_lookup_entry, filter_lookup_table, profile_idx, L2_PROTOCOL, _802_1AG_CFM);

        filter_lookup_entry = &(filter_lookup_table->entry[profile_idx][FILTER_BROADCAST]);

        rdd_ingress_filter_cam_entry_disable(filter_lookup_entry);

        RDD_INGRESS_FILTERS_LOOKUP_ENTRY_BROADCAST_WRITE(1, filter_lookup_entry);
        RDD_INGRESS_FILTERS_LOOKUP_ENTRY_BROADCAST_MASK_WRITE(1, filter_lookup_entry);

        filter_lookup_entry = &(filter_lookup_table->entry[profile_idx][FILTER_MULTICAST]);

        rdd_ingress_filter_cam_entry_disable(filter_lookup_entry);

        RDD_INGRESS_FILTERS_LOOKUP_ENTRY_MULTICAST_WRITE(1, filter_lookup_entry);
        RDD_INGRESS_FILTERS_LOOKUP_ENTRY_MULTICAST_MASK_WRITE(1, filter_lookup_entry);

        filter_lookup_entry = &(filter_lookup_table->entry[profile_idx][FILTER_STOP]);

        rdd_ingress_filter_cam_entry_disable(filter_lookup_entry);

        MWRITE_16(filter_lookup_entry, 0xFFFF);
    }
}

int rdd_ingress_filters_init(const rdd_module_t *module)
{
    RDD_INGRESS_FILTERS_CFG_DTS cfg_entry = {};

    cfg_entry.filters_config_table_addr = ((rdd_if_params_t *)module->params)->filters_config_table_addr;
    cfg_entry.lkp_table_addr = ((rdd_if_params_t *)module->params)->lkp_table_addr;
    cfg_entry.parameter_table_addr = ((rdd_if_params_t *)module->params)->parameter_table_addr;
    cfg_entry.res_offset = module->res_offset;

    MWRITE_GROUP_BLOCK_32(module->group, module->cfg_ptr, (void *)&cfg_entry, sizeof(RDD_INGRESS_FILTERS_CFG_DTS));

    rdd_ingress_filters_cam_tables_init(((rdd_if_params_t *)module->params)->dir);

    return 0;
}

int rdd_ip_address_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS *filters_cfg_table;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry;

    SET_DTS_PRIVATE_MEM_GEN_PTR(dir, filters_cfg_table, INGRESS_FILTERS_CONFIGURATION_TABLE);
    filters_cfg_entry = &(filters_cfg_table->entry[profile_idx]);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_ADDRESS_FILTER_WRITE(enable, filters_cfg_entry);

    return 0;
}

int rdd_tpid_detect_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS *filters_cfg_table;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry;

    SET_DTS_PRIVATE_MEM_GEN_PTR(dir, filters_cfg_table, INGRESS_FILTERS_CONFIGURATION_TABLE);
    filters_cfg_entry = &(filters_cfg_table->entry[profile_idx]);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_TPID_DETECT_FILTER_WRITE(enable, filters_cfg_entry);

    return 0;
}

void rdd_tpid_detect_filter_value_cfg(rdd_module_t *module, uint16_t tpid_detect_value)
{
    if (((rdd_if_params_t *)module->params)->dir == rdpa_dir_ds)
        MWRITE_GROUP_BLOCK_16(module->group, DS_INGRESS_FILTERS_TPID_VALUE_ADDRESS, (void *)&tpid_detect_value,
            DS_INGRESS_FILTERS_TPID_VALUE_BYTE_SIZE);
    else
        MWRITE_GROUP_BLOCK_16(module->group, US_INGRESS_FILTERS_TPID_VALUE_ADDRESS, (void *)&tpid_detect_value,
            DS_INGRESS_FILTERS_TPID_VALUE_BYTE_SIZE);
}

int rdd_dhcp_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    bdmf_boolean local_switch_en)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS *filters_cfg_table;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry_ptr;
    uint32_t filters_mode;

    /* TODO: remove this macro and use define from table manager */
    SET_DTS_PRIVATE_MEM_GEN_PTR(dir, filters_cfg_table, INGRESS_FILTERS_CONFIGURATION_TABLE);

    filters_cfg_entry_ptr = &(filters_cfg_table->entry[profile_idx]);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DHCP_INGRESS_FILTER_WRITE(enable, filters_cfg_entry_ptr);

    if (local_switch_en)
        RDD_LOCAL_SWITCHING_FILTER_CFG(filters_mode, filters_cfg_entry_ptr, 0);

    return 0;
}

int rdd_mld_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    bdmf_boolean local_switch_en)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS *filters_cfg_table;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry_ptr;
    uint32_t filters_mode;

    /* TODO: remove this macro and use define from table manager */
    SET_DTS_PRIVATE_MEM_GEN_PTR(dir, filters_cfg_table, INGRESS_FILTERS_CONFIGURATION_TABLE);

    filters_cfg_entry_ptr = &(filters_cfg_table->entry[profile_idx]);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_WRITE(enable, filters_cfg_entry_ptr);

    if (local_switch_en)
        RDD_LOCAL_SWITCHING_FILTER_CFG(filters_mode, filters_cfg_entry_ptr, enable);

    return 0;
}

int rdd_1588_layer4_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS *filters_cfg_table;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry;

    /* TODO: remove this macro and use define from table manager */
    SET_DTS_PRIVATE_MEM_GEN_PTR(dir, filters_cfg_table, INGRESS_FILTERS_CONFIGURATION_TABLE);

    filters_cfg_entry = &(filters_cfg_table->entry[profile_idx]);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_TIMING_1588_INGRESS_FILTER_WRITE(enable, &filters_cfg_entry);

    return 0;
}

static void rdd_ingress_filters_ptrs_set(rdpa_traffic_dir dir, rdd_port_profile_t profile_idx, rdd_filter filter_num,
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS **filters_cfg_entry_ptr,
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS **filters_lookup_entry_ptr,
    RDD_FILTERS_CONTEXT_ENTRY_DTS **filters_context_entry_ptr)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS *filters_cfg_table;
    RDD_INGRESS_FILTERS_LOOKUP_TABLE_DTS *filter_lookup_table;
    RDD_INGRESS_FILTERS_PARAMETER_TABLE_DTS *filters_context_table;

    SET_DTS_PRIVATE_MEM_GEN_PTR(dir, filters_cfg_table, INGRESS_FILTERS_CONFIGURATION_TABLE);

    *filters_cfg_entry_ptr = &(filters_cfg_table->entry[profile_idx]);

    SET_DTS_PRIVATE_MEM_GEN_PTR(dir, filter_lookup_table, INGRESS_FILTERS_LOOKUP_TABLE);

    *filters_lookup_entry_ptr = &(filter_lookup_table->entry[profile_idx][filter_num]);

    SET_DTS_PRIVATE_MEM_GEN_PTR(dir, filters_context_table, INGRESS_FILTERS_PARAMETER_TABLE);

    *filters_context_entry_ptr = &(filters_context_table->entry[profile_idx][filter_num]);
}

static void rdd_ingress_filter_enable(rdd_action filter_action, rdpa_cpu_reason cpu_reason, bdmf_boolean local_switch_en,
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry_ptr,
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS *filters_lookup_entry_ptr,
    RDD_FILTERS_CONTEXT_ENTRY_DTS *filters_context_entry_ptr)
{
    uint32_t filters_mode;

    if (filter_action == ACTION_TRAP)
    {
        RDD_FILTERS_CONTEXT_ENTRY_TRAP_REASON_WRITE(cpu_reason, filters_context_entry_ptr);
        RDD_FILTERS_CONTEXT_ENTRY_ACTION_WRITE(ACTION_TRAP, filters_context_entry_ptr);
    }
    else if (filter_action == ACTION_DROP)
    {
        RDD_FILTERS_CONTEXT_ENTRY_DROP_REASON_WRITE(0, filters_context_entry_ptr);
        RDD_FILTERS_CONTEXT_ENTRY_ACTION_WRITE(ACTION_DROP, filters_context_entry_ptr);
    }

    rdd_ingress_filter_cam_entry_enable(filters_lookup_entry_ptr);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_WRITE(MODE_ENABLED, filters_cfg_entry_ptr);

    if (local_switch_en)
        RDD_LOCAL_SWITCHING_FILTER_CFG(filters_mode, filters_cfg_entry_ptr, 0);
}

static void rdd_ingress_filter_disable(bdmf_boolean local_switch_en, RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry_ptr,
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS *filters_lookup_entry_ptr, RDD_FILTERS_CONTEXT_ENTRY_DTS *filters_context_entry_ptr)
{
    uint32_t filters_cfg_entry;
    uint32_t filters_mode;

    rdd_ingress_filter_cam_entry_disable(filters_lookup_entry_ptr);

    MWRITE_8(filters_context_entry_ptr, 0);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_READ(filters_cfg_entry, filters_cfg_entry_ptr);

    filters_mode = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ(filters_cfg_entry)
        | RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IGMP_FILTER_L_READ(filters_cfg_entry)
        | RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ICMPV6_FILTER_L_READ(filters_cfg_entry)
        | RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ETHERTYPE_FILTER_L_READ(filters_cfg_entry)
        | RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_BROADCAST_FILTER_L_READ(filters_cfg_entry)
        | RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MULTICAST_FILTER_L_READ(filters_cfg_entry);

    if (!filters_mode)
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_WRITE(MODE_DISABLED, filters_cfg_entry_ptr);

    if (local_switch_en)
        RDD_LOCAL_SWITCHING_FILTER_CFG(filters_mode, filters_cfg_entry_ptr, 0);
}

int rdd_igmp_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable, rdd_action filter_action, bdmf_boolean local_switch_en)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry_ptr;
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS *filters_lookup_entry_ptr;
    RDD_FILTERS_CONTEXT_ENTRY_DTS *filters_context_entry_ptr;

    rdd_ingress_filters_ptrs_set(dir, profile_idx, FILTER_IGMP, &filters_cfg_entry_ptr, &filters_lookup_entry_ptr, &filters_context_entry_ptr);

    if (enable)
    {
        rdd_ingress_filter_enable(filter_action, rdpa_cpu_rx_reason_igmp, local_switch_en, filters_cfg_entry_ptr, filters_lookup_entry_ptr, filters_context_entry_ptr);

        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IGMP_FILTER_WRITE(MODE_ENABLED, filters_cfg_entry_ptr);
    }
    else
    {
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IGMP_FILTER_WRITE(MODE_DISABLED, filters_cfg_entry_ptr);

        rdd_ingress_filter_disable(local_switch_en, filters_cfg_entry_ptr, filters_lookup_entry_ptr, filters_context_entry_ptr);
    }

    return BDMF_ERR_OK;
}

int rdd_icmpv6_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable, bdmf_boolean local_switch_en)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry_ptr;
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS *filters_lookup_entry_ptr;
    RDD_FILTERS_CONTEXT_ENTRY_DTS *filters_context_entry_ptr;

    rdd_ingress_filters_ptrs_set(dir, profile_idx, FILTER_ICMPV6, &filters_cfg_entry_ptr, &filters_lookup_entry_ptr, &filters_context_entry_ptr);

    if (enable)
    {
        rdd_ingress_filter_enable(ACTION_TRAP, rdpa_cpu_rx_reason_icmpv6, local_switch_en, filters_cfg_entry_ptr, filters_lookup_entry_ptr, filters_context_entry_ptr);

        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ICMPV6_FILTER_WRITE(MODE_ENABLED, filters_cfg_entry_ptr);
    }
    else
    {
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ICMPV6_FILTER_WRITE(MODE_DISABLED, filters_cfg_entry_ptr);

        rdd_ingress_filter_disable(local_switch_en, filters_cfg_entry_ptr, filters_lookup_entry_ptr, filters_context_entry_ptr);
    }

    return BDMF_ERR_OK;
}

int rdd_ether_type_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    uint8_t ether_type_filter_num, rdd_action filter_action, bdmf_boolean local_switch_en)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry_ptr;
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS *filters_lookup_entry_ptr;
    RDD_FILTERS_CONTEXT_ENTRY_DTS *filters_context_entry_ptr;
    uint8_t cpu_reason;

    rdd_ingress_filters_ptrs_set(dir, profile_idx, ether_type_filter_num, &filters_cfg_entry_ptr, &filters_lookup_entry_ptr, &filters_context_entry_ptr);

    cpu_reason = rdpa_cpu_rx_reason_etype_udef_0 + ether_type_filter_num - RDD_ETHER_TYPE_FILTER_USER_0;

    if (enable)
    {
        rdd_ingress_filter_enable(filter_action, cpu_reason, local_switch_en, filters_cfg_entry_ptr, filters_lookup_entry_ptr, filters_context_entry_ptr);

        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ETHERTYPE_FILTER_WRITE(MODE_ENABLED, filters_cfg_entry_ptr);

        g_ether_type_filter_mode[dir][profile_idx] |= (1 << (ether_type_filter_num - RDD_ETHER_TYPE_FILTER_USER_0));
    }
    else
    {
        g_ether_type_filter_mode[dir][profile_idx] &= ~(1 << (ether_type_filter_num - RDD_ETHER_TYPE_FILTER_USER_0));

        if (g_ether_type_filter_mode[dir][profile_idx] == 0)
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ETHERTYPE_FILTER_WRITE(MODE_DISABLED, filters_cfg_entry_ptr);

        rdd_ingress_filter_disable(local_switch_en, filters_cfg_entry_ptr, filters_lookup_entry_ptr, filters_context_entry_ptr);
    }

    return BDMF_ERR_OK;
}

int rdd_bcast_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    rdd_action filter_action, bdmf_boolean local_switch_en)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry_ptr;
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS *filters_lookup_entry_ptr;
    RDD_FILTERS_CONTEXT_ENTRY_DTS *filters_context_entry_ptr;

    rdd_ingress_filters_ptrs_set(dir, profile_idx, FILTER_BROADCAST, &filters_cfg_entry_ptr, &filters_lookup_entry_ptr, &filters_context_entry_ptr);

    if (enable)
    {
        rdd_ingress_filter_enable(filter_action, rdpa_cpu_rx_reason_bcast, local_switch_en, filters_cfg_entry_ptr, filters_lookup_entry_ptr, filters_context_entry_ptr);

        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_BROADCAST_FILTER_WRITE(MODE_ENABLED, filters_cfg_entry_ptr);
    }
    else
    {
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_BROADCAST_FILTER_WRITE(MODE_DISABLED, filters_cfg_entry_ptr);

        rdd_ingress_filter_disable(local_switch_en, filters_cfg_entry_ptr, filters_lookup_entry_ptr, filters_context_entry_ptr);
    }

    return BDMF_ERR_OK;
}

int rdd_mcast_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    rdd_action filter_action, bdmf_boolean local_switch_en)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry_ptr;
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS *filters_lookup_entry_ptr;
    RDD_FILTERS_CONTEXT_ENTRY_DTS *filters_context_entry_ptr;

    rdd_ingress_filters_ptrs_set(dir, profile_idx, FILTER_MULTICAST, &filters_cfg_entry_ptr, &filters_lookup_entry_ptr, &filters_context_entry_ptr);

    if (enable)
    {
        rdd_ingress_filter_enable(filter_action, rdpa_cpu_rx_reason_mcast, local_switch_en, filters_cfg_entry_ptr, filters_lookup_entry_ptr, filters_context_entry_ptr);

        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_BROADCAST_FILTER_WRITE(MODE_ENABLED, filters_cfg_entry_ptr);
    }
    else
    {
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_BROADCAST_FILTER_WRITE(MODE_DISABLED, filters_cfg_entry_ptr);

        rdd_ingress_filter_disable(local_switch_en, filters_cfg_entry_ptr, filters_lookup_entry_ptr, filters_context_entry_ptr);
    }

    return BDMF_ERR_OK;
}

int rdd_local_switching_ingress_filters_cfg(rdd_port_profile_t profile_idx, bdmf_boolean enable)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS *filters_cfg_table;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry;

    /* TODO: remove this macro and use define from table manager */
    SET_DTS_PRIVATE_MEM_GEN_PTR(rdpa_dir_us, filters_cfg_table, INGRESS_FILTERS_CONFIGURATION_TABLE);

    filters_cfg_entry = &(filters_cfg_table->entry[profile_idx]);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_LOCAL_SWITCHING_INGRESS_FILTERS_WRITE(enable, filters_cfg_entry);

    return 0;
}

int rdd_ip_fragments_ingress_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    rdd_action filter_action)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS *filters_cfg_table;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry_ptr;
    uint32_t header_error_filter;

    /* TODO: remove this macro and use define from table manager */
    SET_DTS_PRIVATE_MEM_GEN_PTR(dir, filters_cfg_table, INGRESS_FILTERS_CONFIGURATION_TABLE);

    filters_cfg_entry_ptr = &(filters_cfg_table->entry[profile_idx]);

    if (enable)
    {
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_FRAGMENT_INGRESS_FILTER_WRITE(MODE_ENABLED, filters_cfg_entry_ptr);
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_VALIDATION_FILTER_WRITE(MODE_ENABLED, filters_cfg_entry_ptr);

        if (filter_action == ACTION_TRAP)
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_FRAGMENT_INGRESS_FILTER_TRAP_WRITE(MODE_ENABLED,
                filters_cfg_entry_ptr);
        else
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_FRAGMENT_INGRESS_FILTER_TRAP_WRITE(MODE_DISABLED,
                filters_cfg_entry_ptr);
    }
    else
    {
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_FRAGMENT_INGRESS_FILTER_WRITE(MODE_DISABLED, filters_cfg_entry_ptr);
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_HEADER_ERROR_INGRESS_FILTER_READ(header_error_filter,
            filters_cfg_entry_ptr);

        if (!header_error_filter)
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_VALIDATION_FILTER_WRITE(MODE_DISABLED, filters_cfg_entry_ptr);
    }

    return 0;
}

int rdd_hdr_err_ingress_filter_cfg(rdd_port_profile_t profile_idx, rdpa_traffic_dir dir, bdmf_boolean enable,
    rdd_action filter_action)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS *filters_cfg_table;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry_ptr;
    uint32_t ip_fragment_error_filter;

    /* TODO: remove this macro and use define from table manager */
    SET_DTS_PRIVATE_MEM_GEN_PTR(dir, filters_cfg_table, INGRESS_FILTERS_CONFIGURATION_TABLE);

    filters_cfg_entry_ptr = &(filters_cfg_table->entry[profile_idx]);

    if (enable)
    {
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_HEADER_ERROR_INGRESS_FILTER_WRITE(MODE_ENABLED, filters_cfg_entry_ptr);
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_VALIDATION_FILTER_WRITE(MODE_ENABLED, filters_cfg_entry_ptr);

        if (filter_action == ACTION_TRAP)
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_HEADER_ERROR_INGRESS_FILTER_TRAP_WRITE(MODE_ENABLED,
                filters_cfg_entry_ptr);
        else
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_HEADER_ERROR_INGRESS_FILTER_TRAP_WRITE(MODE_DISABLED,
                filters_cfg_entry_ptr);
    }
    else
    {
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_HEADER_ERROR_INGRESS_FILTER_WRITE(MODE_DISABLED,
            filters_cfg_entry_ptr);
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_FRAGMENT_INGRESS_FILTER_READ(ip_fragment_error_filter,
            filters_cfg_entry_ptr);

        if (!ip_fragment_error_filter)
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_VALIDATION_FILTER_WRITE(MODE_DISABLED, filters_cfg_entry_ptr);
    }

    return 0;
}

int rdd_src_mac_anti_spoofing_lookup_cfg(rdd_port_profile_t profile_idx, bdmf_boolean enable)
{
    RDD_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS *filters_cfg_table;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS *filters_cfg_entry;

    /* TODO: remove this macro and use define from table manager */
    SET_DTS_PRIVATE_MEM_GEN_PTR(rdpa_dir_us, filters_cfg_table, INGRESS_FILTERS_CONFIGURATION_TABLE);

    filters_cfg_entry = &(filters_cfg_table->entry[profile_idx]);

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_SRC_MAC_ANTI_SPOOFING_LOOKUP_WRITE(enable, filters_cfg_entry);

    return 0;
}

int rdd_src_mac_anti_spoofing_entry_add(rdd_port_profile_t profile_idx, uint32_t src_mac_prefix)
{
    RDD_SRC_MAC_ANTI_SPOOFING_TABLE_DTS *mac_table_ptr;
    RDD_SRC_MAC_ANTI_SPOOFING_ENTRY_DTS *mac_entry_ptr;

    if (g_anti_spoofing_last_rule_idx[profile_idx] == RDD_SRC_MAC_ANTI_SPOOFING_TABLE_SIZE2)
        return BDMF_ERR_NORES;

    mac_table_ptr = (RDD_SRC_MAC_ANTI_SPOOFING_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_1_OFFSET)
        + SRC_MAC_ANTI_SPOOFING_TABLE_ADDRESS - sizeof(RUNNER_COMMON));
    mac_entry_ptr = &(mac_table_ptr->entry[profile_idx][g_anti_spoofing_last_rule_idx[profile_idx]]);
    RDD_SRC_MAC_ANTI_SPOOFING_ENTRY_MAC_PREFIX_WRITE(src_mac_prefix, mac_entry_ptr);
    g_anti_spoofing_last_rule_idx[profile_idx]++;

    return 0;
}

int rdd_src_mac_anti_spoofing_entry_delete(rdd_port_profile_t profile_idx, uint32_t src_mac_prefix)
{
    RDD_SRC_MAC_ANTI_SPOOFING_TABLE_DTS *mac_table_ptr;
    RDD_SRC_MAC_ANTI_SPOOFING_ENTRY_DTS *mac_entry_ptr;
    uint32_t i, src_mac_anti_spoofing_entry, last_rule_idx;

    mac_table_ptr = (RDD_SRC_MAC_ANTI_SPOOFING_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_1_OFFSET)
        + SRC_MAC_ANTI_SPOOFING_TABLE_ADDRESS - sizeof(RUNNER_COMMON));
    last_rule_idx = g_anti_spoofing_last_rule_idx[profile_idx];

    for (i = 0; i < last_rule_idx; i++)
    {
        mac_entry_ptr = &(mac_table_ptr->entry[profile_idx][i]);

        RDD_SRC_MAC_ANTI_SPOOFING_ENTRY_MAC_PREFIX_READ(src_mac_anti_spoofing_entry, mac_entry_ptr);

        if (src_mac_anti_spoofing_entry == src_mac_prefix)
            break;
    }

    if (i == last_rule_idx)
        return BDMF_ERR_NOENT;

    g_anti_spoofing_last_rule_idx[profile_idx]--;

    if (i == (last_rule_idx - 1))
    {
        RDD_SRC_MAC_ANTI_SPOOFING_ENTRY_MAC_PREFIX_WRITE(0xFFFFFFFF, mac_entry_ptr);
        return 0;
    }

    /* replace the last entry in the cam with the one to be deleted invalidate the deleted entry */
    RDD_SRC_MAC_ANTI_SPOOFING_ENTRY_MAC_PREFIX_WRITE(0, mac_entry_ptr);

    /* copy last cam entry over deleted entry */
    mac_entry_ptr = &(mac_table_ptr->entry[profile_idx][g_anti_spoofing_last_rule_idx[profile_idx]]);

    RDD_SRC_MAC_ANTI_SPOOFING_ENTRY_MAC_PREFIX_READ(src_mac_anti_spoofing_entry, mac_entry_ptr);

    mac_entry_ptr = &(mac_table_ptr->entry[profile_idx][i]);

    RDD_SRC_MAC_ANTI_SPOOFING_ENTRY_MAC_PREFIX_WRITE(src_mac_anti_spoofing_entry, mac_entry_ptr);

    /* invalidate the deleted entry */
    mac_entry_ptr = &(mac_table_ptr->entry[profile_idx][g_anti_spoofing_last_rule_idx[profile_idx]]);

    RDD_SRC_MAC_ANTI_SPOOFING_ENTRY_MAC_PREFIX_WRITE(0xFFFFFFFF, mac_entry_ptr);

    return 0;
}

int rdd_ingress_filter_ip_address_entry_add(bdmf_ip_t ip_address)
{
    RDD_INGRESS_FILTER_IPV4_ADDRESS_TABLE_DTS *ipv4_address_table_ptr;
    RDD_INGRESS_FILTER_IPV6_ADDRESS_TABLE_DTS *ipv6_address_table_ptr;
    uint32_t crc_init_value;
    uint32_t ipv6_crc;

    if (ip_address.family == bdmf_ip_family_ipv4)
    {
        if (g_ipv4_address_last_idx == RDD_INGRESS_FILTER_IPV4_ADDRESS_TABLE_SIZE)
            return BDMF_ERR_NO_MORE;

        ipv4_address_table_ptr = RDD_INGRESS_FILTER_IPV4_ADDRESS_TABLE_PTR();

        MWRITE_32(&(ipv4_address_table_ptr->entry[g_ipv4_address_last_idx++]), ip_address.addr.ipv4);

        return BDMF_ERR_OK;
    }
    else if (ip_address.family == bdmf_ip_family_ipv6)
    {
        if (g_ipv6_address_last_idx == RDD_INGRESS_FILTER_IPV6_ADDRESS_TABLE_SIZE)
            return BDMF_ERR_NO_MORE;

        ipv6_address_table_ptr = RDD_INGRESS_FILTER_IPV6_ADDRESS_TABLE_PTR();

        crc_init_value = rdd_crc_init_value_get(RDD_CRC_TYPE_32);

        ipv6_crc = rdd_crc_bit_by_bit(ip_address.addr.ipv6.data, 16, 0, crc_init_value, RDD_CRC_TYPE_32);

        MWRITE_32(&(ipv6_address_table_ptr->entry[g_ipv6_address_last_idx++]), ipv6_crc);

        return BDMF_ERR_OK;
    }

    return BDMF_ERR_RANGE;
}

int rdd_ingress_filter_ip_address_entry_delete(bdmf_ip_t ip_address)
{
    RDD_INGRESS_FILTER_IPV4_ADDRESS_TABLE_DTS *ipv4_address_table_ptr;
    RDD_INGRESS_FILTER_IPV6_ADDRESS_TABLE_DTS *ipv6_address_table_ptr;
    uint32_t ipv4_entry, ipv6_entry;
    uint32_t crc_init_value;
    uint32_t ipv6_crc;
    uint32_t i;

    if (ip_address.family == bdmf_ip_family_ipv4)
    {
        ipv4_address_table_ptr = RDD_INGRESS_FILTER_IPV4_ADDRESS_TABLE_PTR();

        for (i = 0; i < g_ipv4_address_last_idx; i++)
        {
            MREAD_32(&(ipv4_address_table_ptr->entry[i]), ipv4_entry);

            if (ipv4_entry == ip_address.addr.ipv4)
                break;
        }

        if (i == g_ipv4_address_last_idx)
            return BDMF_ERR_NOENT;

        g_ipv4_address_last_idx--;

        if (g_ipv4_address_last_idx != 0)
            MWRITE_32(&(ipv4_address_table_ptr->entry[i]), ipv4_address_table_ptr->entry[g_ipv4_address_last_idx].bits);

        MWRITE_32(&(ipv4_address_table_ptr->entry[g_ipv4_address_last_idx]), 0);

        return BDMF_ERR_OK;
    }
    else if (ip_address.family == bdmf_ip_family_ipv6)
    {
        ipv6_address_table_ptr = RDD_INGRESS_FILTER_IPV6_ADDRESS_TABLE_PTR();

        crc_init_value = rdd_crc_init_value_get(RDD_CRC_TYPE_32);

        ipv6_crc = rdd_crc_bit_by_bit(ip_address.addr.ipv6.data, 16, 0, crc_init_value, RDD_CRC_TYPE_32);

        for (i = 0; i < g_ipv6_address_last_idx; i++)
        {
            MREAD_32(&(ipv6_address_table_ptr->entry[i]), ipv6_entry);

            if (ipv6_entry == ipv6_crc)
                break;
        }

        if (i == g_ipv6_address_last_idx)
            return BDMF_ERR_NOENT;

        g_ipv6_address_last_idx--;

        if (g_ipv6_address_last_idx != 0)
            MWRITE_32(&(ipv6_address_table_ptr->entry[i]), ipv6_address_table_ptr->entry[g_ipv6_address_last_idx].bits);

        MWRITE_32(&(ipv6_address_table_ptr->entry[g_ipv6_address_last_idx]), 0);

        return BDMF_ERR_OK;
    }

    return BDMF_ERR_RANGE;
}

/*** Port Profile ***/
void rdd_port_profile_map_set(rdpa_traffic_dir dir, rdd_vport_id_t port_idx, rdd_port_profile_t profile_idx)
{
    uint8_t *table;

    /* TODO: remove this macro and use define from table manager */
    SET_PRIVATE_MEM_GEN_PTR(dir, table, VPORT_TO_PORT_PROFILE, uint8_t);

    table[port_idx] = profile_idx;
}

rdd_port_profile_t rdd_port_profile_map_get(rdpa_traffic_dir dir, rdd_vport_id_t port_idx)
{
    uint8_t *table;

    /* TODO: remove this macro and use define from table manager */
    SET_PRIVATE_MEM_GEN_PTR(dir, table, VPORT_TO_PORT_PROFILE, uint8_t);

    return table[port_idx];
}
