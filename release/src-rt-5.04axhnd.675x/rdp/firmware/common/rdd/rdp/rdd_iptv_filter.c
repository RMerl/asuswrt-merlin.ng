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
#include "rdd_iptv_filter.h"
#include "rdd_lookup_engine.h"

/******************************************************************************/
/*                                                                            */
/*                            Global Variables                                */
/*                                                                            */
/******************************************************************************/

#if defined(FIRMWARE_INIT)
extern uint32_t IPTV_TABLE_PTR;
extern uint32_t IPTV_CONTEXT_TABLE_PTR;
extern uint32_t IPTV_SSM_CONTEXT_TABLE_PTR;
#endif
extern uint8_t *g_runner_ddr_base_addr;
extern uint32_t g_runner_tables_ptr;
extern rdd_vlan_actions_matrix_t *g_vlan_actions_matrix_ptr;
extern uint32_t g_vlan_mapping_command_to_action[RDD_MAX_VLAN_CMD][RDD_MAX_PBITS_CMD];
extern uint32_t g_runner_ddr0_base_addr;
extern uint32_t g_runner_ddr0_iptv_lookup_ptr;
extern uint32_t g_runner_ddr0_iptv_context_ptr;
extern uint32_t g_runner_ddr0_iptv_ssm_context_ptr;
extern uint32_t g_runner_ddr_phy_iptv_tables_base_ptr;
#if 0
extern uint32_t g_mac_table_size;
extern uint32_t g_mac_table_search_depth;
#endif
uint32_t g_iptv_table_size;
uint32_t g_iptv_table_search_depth;
uint32_t g_iptv_context_tables_free_list_head;
uint32_t g_iptv_context_tables_free_list_tail;
uint32_t g_iptv_source_ip_counter[RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS];
rdd_64_bit_table_cfg_t g_hash_table_cfg[RDD_MAX_HASH_TABLE];
RDD_CONTEXT_TABLES_FREE_LIST_DTS *g_iptv_context_tables_free_list;

extern rdd_wan_physical_port_t g_wan_physical_port;
extern bdmf_fastlock int_lock;
extern bdmf_fastlock cpu_message_lock;

extern void _rdd_vlan_matrix_initialize(void);

/* static functions declarations */
static int _rdd_add_iptv_l3_dst_ip_src_ip_existing_any_entry_to_cache(rdd_iptv_entry_t *, uint32_t);
static int _rdd_add_iptv_l3_dst_ip_src_ip_existing_any_entry_to_ddr(rdd_iptv_entry_t *, uint32_t);
static void _rdd_add_iptv_l3_dst_ip_src_ip_existing_entry_to_cache(uint32_t, uint32_t);
static void _rdd_add_iptv_l3_dst_ip_src_ip_existing_entry_to_ddr(rdd_iptv_entry_t *, uint32_t, uint32_t, uint32_t *);
static int _rdd_add_iptv_l3_dst_ip_src_ip_base_entry_to_cache(rdd_iptv_entry_t *, uint8_t *, uint32_t *, uint32_t *);
static int _rdd_add_iptv_l3_dst_ip_src_ip_base_entry_to_ddr(rdd_iptv_entry_t *, uint8_t *, uint32_t, uint32_t, rdpa_iptv_lookup_method, bdmf_ip_t *, uint32_t *);
static int _rdd_add_iptv_l3_dst_ip_src_ip_existing_any_entry(rdd_iptv_entry_t *, uint32_t, uint32_t);
static void _rdd_add_iptv_l3_dst_ip_src_ip_existing_entry(rdd_iptv_entry_t *, uint32_t, uint32_t, uint32_t, uint32_t *);
static int _rdd_add_iptv_l3_dst_ip_src_ip_first_any_entry(rdd_iptv_entry_t *, rdpa_iptv_lookup_method, bdmf_ip_t *, uint32_t *, uint32_t *);
static int _rdd_add_iptv_l3_dst_ip_src_ip_first_entry(rdd_iptv_entry_t *, rdpa_iptv_lookup_method, bdmf_ip_t *, bdmf_ip_t *, uint32_t *, uint32_t *);
static int _rdd_find_iptv_l3_dst_ip_src_ip_base_entry(rdd_iptv_entry_t *, rdpa_iptv_lookup_method, bdmf_ip_t *, uint32_t *, uint32_t *);
static int _rdd_add_iptv_l2_mac_entry(rdd_iptv_entry_t *, uint32_t *, uint32_t *);
static int _rdd_add_iptv_entry_to_cache(rdd_iptv_entry_t *, uint8_t *, uint32_t *);
static int _rdd_add_iptv_entry_to_ddr(rdd_iptv_entry_t *, uint8_t *, rdpa_iptv_lookup_method, uint32_t, bdmf_ip_t *, uint32_t *);
static int _rdd_add_iptv_l2_mac_vid_entry(rdd_iptv_entry_t *, uint32_t *, uint32_t *);
static int _rdd_add_iptv_l3_dst_ip_src_ip_entry(rdd_iptv_entry_t *, rdpa_iptv_lookup_method, bdmf_ip_t *, bdmf_ip_t *, uint32_t *, uint32_t *);
static int _rdd_add_iptv_l3_dst_ip_entry(rdd_iptv_entry_t *, bdmf_ip_t *, uint32_t *, uint32_t *);
static int _rdd_delete_iptv_entry_from_cache(uint32_t);
static int _rdd_delete_iptv_entry_from_ddr(uint32_t, uint32_t *, uint32_t *);
static int _rdd_delete_iptv_l2_mac_entry(uint32_t, uint32_t	*);
static int _rdd_delete_iptv_l2_mac_vid_entry(uint32_t, uint32_t *);
static int _rdd_free_iptv_l3_dst_ip_src_ip_context_table(uint32_t);
static int _rdd_delete_iptv_l3_dst_ip_src_ip_any_entry_from_cache(uint32_t);
static int _rdd_delete_iptv_l3_dst_ip_src_ip_any_entry_from_ddr(uint32_t, uint32_t *, uint32_t *, uint32_t *);
static int _rdd_delete_iptv_l3_dst_ip_src_ip_non_any_entry_from_cache(uint32_t, uint32_t);
static int _rdd_delete_iptv_l3_dst_ip_src_ip_non_any_entry_from_ddr(uint32_t, uint32_t, uint32_t *, uint32_t *, uint32_t *);
static int _rdd_delete_iptv_l3_dst_ip_src_ip_any_entry(uint32_t, uint32_t *);
static int _rdd_delete_iptv_l3_dst_ip_src_ip_non_any_entry(uint32_t, uint32_t, uint32_t *);
static int _rdd_delete_iptv_l3_dst_ip_src_ip_entry(uint32_t, uint32_t *);
static int _rdd_delete_iptv_l3_dst_ip_entry(uint32_t, uint32_t *);
static inline int _rdd_delete_iptv_l3_dst_ip_src_ip_vid_entry(uint32_t, uint32_t *);
static int _rdd_modify_iptv_entry_in_cache(uint32_t, uint32_t, uint16_t, uint8_t);
static int _rdd_modify_iptv_entry_in_ddr(uint32_t, uint32_t, uint16_t, uint8_t, rdpa_iptv_lookup_method);
static int _rdd_modify_iptv_ssm_context_entry_in_ddr(uint32_t, uint32_t, uint16_t, uint8_t);
static int _rdd_modify_iptv_l2_mac_entry(uint32_t, uint16_t, uint16_t, uint8_t);
static int _rdd_modify_iptv_l2_mac_vid_entry(uint32_t, uint16_t, uint16_t, uint8_t);
static int _rdd_modify_iptv_l3_dst_ip_src_ip_entry(uint32_t, uint32_t, uint16_t, uint8_t);
static int _rdd_modify_iptv_l3_dst_ip_entry(uint32_t, uint32_t, uint16_t, uint8_t);
static int _rdd_modify_iptv_l3_dst_ip_src_ip_vid_entry(uint32_t, uint32_t, uint16_t, uint8_t);
static int _rdd_search_iptv_ssm_entry(rdpa_iptv_channel_key_t *, bdmf_ip_t *, uint32_t *);
static int _rdd_search_iptv_entry(rdpa_iptv_channel_key_t *, uint8_t *, rdpa_iptv_lookup_method, bdmf_ip_t *, uint32_t *);
static int _rdd_get_iptv_l2_mac_entry_index(rdpa_iptv_channel_key_t *, uint32_t *);
static int _rdd_get_iptv_l2_mac_vid_entry_index(rdpa_iptv_channel_key_t *, uint32_t *);
static int _rdd_get_iptv_l3_dst_ip_src_ip_entry_index(rdpa_iptv_channel_key_t *, rdpa_iptv_lookup_method, bdmf_ip_t *, bdmf_ip_t *, uint32_t *);
static int _rdd_get_iptv_l3_dst_ip_entry_index(rdpa_iptv_channel_key_t *key, bdmf_ip_t *, bdmf_ip_t *, uint32_t *);
static int _rdd_read_iptv_ssm_entry(uint32_t, rdd_iptv_entry_t *, rdpa_iptv_lookup_method, bdmf_ip_t *);
static int _rdd_read_iptv_entry(uint32_t, rdpa_iptv_lookup_method, rdd_iptv_entry_t *, bdmf_ip_t *);
static int _rdd_get_iptv_l2_mac_entry(uint32_t, rdd_iptv_entry_t *);
static int _rdd_get_iptv_l2_mac_vid_entry(uint32_t, rdd_iptv_entry_t *);
static inline int _rdd_get_iptv_l3_dst_ip_src_ip_entry(uint32_t, rdd_iptv_entry_t *, bdmf_ip_t *);
static int _rdd_get_iptv_l3_dst_ip_entry(uint32_t, rdd_iptv_entry_t *, bdmf_ip_t *);
static int _rdd_read_iptv_ssm_entry_counter(uint32_t, uint16_t *);
static int _rdd_read_iptv_ssm_any_entry_counter(uint32_t, uint16_t *);
static int _rdd_read_iptv_entry_counter(uint32_t, uint16_t *);
static inline int _rdd_read_iptv_l2_mac_counter(uint32_t, uint16_t *);
static inline int _rdd_read_iptv_l2_mac_vid_counter(uint32_t, uint16_t *);
static int _rdd_read_iptv_l3_dst_ip_src_ip_counter(uint32_t, uint16_t *);
static inline int _rdd_read_iptv_l3_dst_ip_counter(uint32_t, uint16_t *);
static inline int _rdd_read_iptv_l3_dst_ip_src_ip_vid_counter(uint32_t, uint16_t *);
static int _rdd_add_iptv_layer3_src_ip(bdmf_ip_t *, bdmf_ip_t *, uint32_t *);
static int _rdd_delete_iptv_layer3_src_ip(bdmf_ip_t *, uint32_t *);


/* iptv auxiliary macros */
#define GET_BASE_INDEX_FROM_ENTRY_INDEX(entry_index)                                ((entry_index) & 0x1fff)
#define GET_PROVIDER_INDEX_FROM_ENTRY_INDEX(entry_index)                            ((((entry_index) >> 13) & 0x3f) - 1)
#define GET_ENTRY_INDEX_FROM_BASE_AND_PROVIDER_INDICES(base_index, provider_index)  ((base_index) | (((provider_index) + 1) << 13))

void _rdd_iptv_mac_table_init(uint32_t iptv_table_size)
{
    RUNNER_REGS_CFG_LKUP_GLBL_MASK2_H hash_lkup_2_global_mask_high_register;
    RUNNER_REGS_CFG_LKUP_GLBL_MASK2_L hash_lkup_2_global_mask_low_register;
    RUNNER_REGS_CFG_LKUP2_CFG hash_lkup_2_cfg_register;
    rdd_64_bit_table_cfg_t *hash_table_cfg_ptr;

    /* initialize the CRC software variables */
    rdd_crc_init();

    hash_table_cfg_ptr = &g_hash_table_cfg[RDD_IPTV_TABLE];

    hash_table_cfg_ptr->hash_table_size = (1 << iptv_table_size) * 32;
    hash_table_cfg_ptr->hash_table_search_depth = (1 << MAC_TABLE_MAX_HOP_32);
    hash_table_cfg_ptr->hash_table_ptr = (rdd_64_bit_table_entry_t *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + IPTV_LOOKUP_TABLE_ADDRESS);

    hash_table_cfg_ptr->is_external_context = RDD_EXTERNAL_CONTEXT_ENABLE;
    hash_table_cfg_ptr->context_size = RDD_CONTEXT_16_BIT;
    hash_table_cfg_ptr->context_table_ptr = (uint8_t *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + IPTV_CONTEXT_TABLE_ADDRESS);
    hash_table_cfg_ptr->is_extension_cam = 1;
    hash_table_cfg_ptr->cam_table_size = RDD_MAC_TABLE_CAM_SIZE - 1;
    hash_table_cfg_ptr->cam_table_ptr = (rdd_64_bit_table_entry_t *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + IPTV_LOOKUP_TABLE_CAM_ADDRESS);
    hash_table_cfg_ptr->cam_context_table_ptr = (uint8_t *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + IPTV_CONTEXT_TABLE_CAM_ADDRESS);

    hash_lkup_2_cfg_register.base_address = (IPTV_L3_SRC_IP_LOOKUP_TABLE_ADDRESS >> 3);
    hash_lkup_2_cfg_register.table_size = RDD_MAC_TABLE_SIZE_32;
    hash_lkup_2_cfg_register.max_hop = MAC_TABLE_MAX_HOP_32;
    hash_lkup_2_cfg_register.hash_type = RDD_MAC_HASH_TYPE_CRC16;

    RUNNER_REGS_0_CFG_LKUP2_CFG_WRITE(hash_lkup_2_cfg_register);

    hash_lkup_2_global_mask_high_register.base_address = 0x0FFFFFFF;

    RUNNER_REGS_0_CFG_LKUP_GLBL_MASK2_H_WRITE(hash_lkup_2_global_mask_high_register);

    hash_lkup_2_global_mask_low_register.base_address = 0xFFFFFFFF;

    RUNNER_REGS_0_CFG_LKUP_GLBL_MASK2_L_WRITE(hash_lkup_2_global_mask_low_register);

    hash_table_cfg_ptr = &g_hash_table_cfg[RDD_IPTV_SRC_IP_TABLE];

    hash_table_cfg_ptr->hash_table_size = RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS;
    hash_table_cfg_ptr->hash_table_search_depth = RDD_IPTV_SOURCE_IP_TABLE_SEARCH_DEPTH;
    hash_table_cfg_ptr->hash_table_ptr = (rdd_64_bit_table_entry_t *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + IPTV_L3_SRC_IP_LOOKUP_TABLE_ADDRESS);
    hash_table_cfg_ptr->is_external_context = RDD_EXTERNAL_CONTEXT_DISABLE;
    hash_table_cfg_ptr->context_size = RDD_CONTEXT_8_BIT;
    hash_table_cfg_ptr->context_table_ptr = 0;
    hash_table_cfg_ptr->is_extension_cam = 0;

    g_iptv_table_size = (1 << iptv_table_size) * 32;
    g_iptv_table_search_depth = (1 << MAC_TABLE_MAX_HOP_32);
}

static uint8_t _rdd_extract_replication_number(uint32_t egress_port_vector)
{
    uint8_t replication_number = 0;

    /* until all bits are zero */
    while (egress_port_vector > 0)             
    {
        /* check lower bit */
        if ((egress_port_vector & 1) == 1)
           (replication_number)++;

        /* removing lower bit */
        egress_port_vector >>= 1;
    }

    (replication_number)--;

    return replication_number;
}

int rdd_iptv_init(const rdd_module_t *module)
{
    RDD_IPTV_FILTER_CFG_DTS cfg_entry = {};
    uint32_t *iptv_table_pointer_ptr, *iptv_context_table_pointer_ptr, *iptv_ssm_context_table_address_ptr;
    uint32_t iptv_table_address, iptv_context_table_address, iptv_ssm_context_table_address;
    uint16_t i;

    cfg_entry.res_offset = module->res_offset;
    cfg_entry.context_offset = module->context_offset;

    MWRITE_GROUP_BLOCK_32(module->group, module->cfg_ptr, (void *)&cfg_entry, sizeof(RDD_IPTV_FILTER_CFG_DTS));

    MEMSET((void *)IPTV_TABLE_PTR, 0, sizeof(RDD_IPTV_DDR_LOOKUP_TABLE_DTS));
    MEMSET((void *)IPTV_CONTEXT_TABLE_PTR, 0, sizeof(RDD_IPTV_DDR_CONTEXT_TABLE_DTS));
    MEMSET((void *)IPTV_SSM_CONTEXT_TABLE_PTR, 0, sizeof(RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS));

    _rdd_vlan_matrix_initialize();

    _rdd_iptv_mac_table_init(((iptv_params_t *)module->params)->iptv_table_size);

#if defined(FIRMWARE_INIT)
    iptv_table_address = IPTV_DDR_LOOKUP_TABLE_ADDRESS;
    iptv_context_table_address = IPTV_DDR_CONTEXT_TABLE_ADDRESS;
    iptv_ssm_context_table_address = IPTV_SSM_DDR_CONTEXT_TABLE_ADDRESS;
#else
    iptv_context_table_address = g_runner_ddr_phy_iptv_tables_base_ptr;
    iptv_table_address = g_runner_ddr_phy_iptv_tables_base_ptr + 0x40000;
    iptv_ssm_context_table_address = g_runner_ddr_phy_iptv_tables_base_ptr + 0x80000;
#endif
    iptv_table_pointer_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + IPTV_TABLE_POINTER_ADDRESS);
    iptv_context_table_pointer_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + IPTV_CONTEXT_TABLE_POINTER_ADDRESS);
    MWRITE_32(iptv_table_pointer_ptr, iptv_table_address);
    MWRITE_32(iptv_context_table_pointer_ptr, iptv_context_table_address);

    iptv_ssm_context_table_address_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + IPTV_SSM_CONTEXT_TABLE_PTR_ADDRESS);
    MWRITE_32(iptv_ssm_context_table_address_ptr, iptv_ssm_context_table_address);

    g_iptv_context_tables_free_list = (RDD_CONTEXT_TABLES_FREE_LIST_DTS *)bdmf_alloc(sizeof(RDD_CONTEXT_TABLES_FREE_LIST_DTS));
    g_iptv_context_tables_free_list_head = 0;
    g_iptv_context_tables_free_list_tail = RDD_IPTV_SSM_CONTEXT_ENTRY_COUNT - 1;

    for (i = 0; i < RDD_IPTV_SSM_CONTEXT_ENTRY_COUNT; i++)
        g_iptv_context_tables_free_list->entry[i] = i + 1;

    for (i = 0; i < RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS; i++)
        g_iptv_source_ip_counter[i] = 0;

    return BDMF_ERR_OK;
}

void rdd_iptv_lkp_method_cfg(rdpa_iptv_lookup_method iptv_mode)
{
    RDD_SYSTEM_CONFIGURATION_DTS *system_cfg;

    system_cfg = (RDD_SYSTEM_CONFIGURATION_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_SYSTEM_CONFIGURATION_ADDRESS);

    RDD_SYSTEM_CONFIGURATION_IPTV_CLASSIFICATION_METHOD_WRITE(iptv_mode, system_cfg);

    if ((iptv_mode == iptv_lookup_method_group_ip_src_ip) || (iptv_mode == iptv_lookup_method_group_ip_src_ip_vid))
        g_hash_table_cfg[RDD_IPTV_TABLE].is_external_context = RDD_EXTERNAL_CONTEXT_DISABLE;
    else
        g_hash_table_cfg[RDD_IPTV_TABLE].is_external_context = RDD_EXTERNAL_CONTEXT_ENABLE;
}

static int _rdd_delete_iptv_entry_from_cache(uint32_t entry_index)
{
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS *iptv_ingress_classification_context_table_ptr; 
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS *iptv_ingress_classification_context_entry_ptr; 
    RDD_IPTV_LOOKUP_TABLE_DTS *iptv_table_ptr;
    RDD_IPTV_L3_LOOKUP_ENTRY_DTS *iptv_entry_ptr;
    RDD_IPTV_CONTEXT_TABLE_DTS *iptv_forward_table_ptr;
    RDD_IPTV_CONTEXT_ENTRY_DTS *iptv_forward_entry_ptr;
    RDD_IPTV_COUNTERS_TABLE_DTS *iptv_counter_table_ptr;
    RDD_IPTV_COUNTER_ENTRY_DTS *iptv_counter_entry_ptr;
    uint32_t iptv_table_size;
    int rdd_error;

    iptv_table_ptr = (RDD_IPTV_LOOKUP_TABLE_DTS *)(g_hash_table_cfg[RDD_IPTV_TABLE].hash_table_ptr);

    iptv_table_size =  RDD_IPTV_LOOKUP_TABLE_SIZE;

    iptv_forward_table_ptr = (RDD_IPTV_CONTEXT_TABLE_DTS *)(g_hash_table_cfg[RDD_IPTV_TABLE].context_table_ptr);

    iptv_ingress_classification_context_table_ptr = RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_PTR();

    iptv_ingress_classification_context_entry_ptr = &(iptv_ingress_classification_context_table_ptr->entry[entry_index]);

    if (entry_index >= RDD_IPTV_LOOKUP_TABLE_SIZE)
    {
       entry_index -= RDD_IPTV_LOOKUP_TABLE_SIZE;

        iptv_table_ptr = (RDD_IPTV_LOOKUP_TABLE_DTS *)(g_hash_table_cfg[RDD_IPTV_TABLE].cam_table_ptr);

        iptv_table_size =  RDD_IPTV_LOOKUP_TABLE_CAM_SIZE;

        iptv_forward_table_ptr = (RDD_IPTV_CONTEXT_TABLE_DTS *)(g_hash_table_cfg[RDD_IPTV_TABLE].cam_context_table_ptr);
    }

    iptv_entry_ptr = &(iptv_table_ptr->entry[entry_index].iptv_l3_lookup_entry);

    iptv_forward_entry_ptr = &(iptv_forward_table_ptr->entry[entry_index]);

    MEMSET(iptv_forward_entry_ptr, 0, sizeof(RDD_IPTV_CONTEXT_ENTRY_DTS));
    MEMSET(iptv_entry_ptr, 0, (sizeof(RDD_IPTV_L3_LOOKUP_ENTRY_DTS) - 1));

    rdd_error = rdd_write_control_bits((rdd_64_bit_table_entry_t *)iptv_table_ptr, iptv_table_size, entry_index, RDD_REMOVE_ENTRY);

    if (rdd_error)
        return rdd_error;

    iptv_counter_table_ptr = RDD_IPTV_COUNTERS_TABLE_PTR();

    iptv_counter_entry_ptr = &(iptv_counter_table_ptr->entry[entry_index]);

    RDD_IPTV_COUNTER_ENTRY_COUNTER_WRITE(0, iptv_counter_entry_ptr);

    MEMSET(iptv_ingress_classification_context_entry_ptr, 0, sizeof(RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS));

    return rdd_error;
}

static int _rdd_read_iptv_counter_cache(uint32_t index, uint16_t *counter_value)
{
#if !defined(FIRMWARE_INIT)
    RDD_IPTV_COUNTER_ENTRY_DTS *iptv_mac_counter_ptr;
    int rdd_error;

    bdmf_fastlock_lock(&cpu_message_lock);

    rdd_error = _rdd_cpu_message_send(RDD_CPU_MESSAGE_IPTV_COUNTER_GET, RDD_CLUSTER_0, 0, index, 0, 1);

    if (rdd_error)
    {
        bdmf_fastlock_unlock(&cpu_message_lock);
        return rdd_error;
    }

    iptv_mac_counter_ptr = (RDD_IPTV_COUNTER_ENTRY_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + IPTV_COUNTERS_BUFFER_ADDRESS);

    RDD_IPTV_COUNTER_ENTRY_COUNTER_READ(*counter_value, iptv_mac_counter_ptr);

    bdmf_fastlock_unlock(&cpu_message_lock);
#else
    *counter_value = 0;
#endif
    return BDMF_ERR_OK;
}

/******************************************************************************/
static int _rdd_add_iptv_l3_dst_ip_src_ip_existing_any_entry_to_cache(rdd_iptv_entry_t *entry, uint32_t cache_index)
{
    RDD_IPTV_LOOKUP_TABLE_DTS *iptv_table_ptr;
    RDD_IPTV_L3_LOOKUP_ENTRY_DTS *iptv_entry_ptr;
    RDD_IPTV_CONTEXT_TABLE_DTS *forward_table_ptr;
    RDD_IPTV_CONTEXT_ENTRY_DTS *forward_entry_ptr;
    RDD_IPTV_SSID_EXTENSION_TABLE_DTS *iptv_ssid_table_ptr;
    RDD_IPTV_SSID_EXTENSION_ENTRY_DTS *iptv_ssid_entry_ptr;
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS *iptv_ingress_classification_context_table_ptr; 
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS *iptv_ingress_classification_context_entry_ptr; 
    uint32_t any;

    iptv_table_ptr = (RDD_IPTV_LOOKUP_TABLE_DTS *)g_hash_table_cfg[RDD_IPTV_TABLE].hash_table_ptr;

    forward_table_ptr = (RDD_IPTV_CONTEXT_TABLE_DTS *)(g_hash_table_cfg[RDD_IPTV_TABLE].context_table_ptr);

    iptv_ssid_table_ptr = RDD_IPTV_SSID_EXTENSION_TABLE_PTR();

    iptv_ingress_classification_context_table_ptr = RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_PTR();

    iptv_ingress_classification_context_entry_ptr = &(iptv_ingress_classification_context_table_ptr->entry[cache_index]);

    if (cache_index >= RDD_IPTV_LOOKUP_TABLE_SIZE)
    {
        cache_index -= RDD_IPTV_LOOKUP_TABLE_SIZE;

        iptv_table_ptr = (RDD_IPTV_LOOKUP_TABLE_DTS *)g_hash_table_cfg[RDD_IPTV_TABLE].cam_table_ptr;

        forward_table_ptr = (RDD_IPTV_CONTEXT_TABLE_DTS *)(g_hash_table_cfg[RDD_IPTV_TABLE].cam_context_table_ptr);

        iptv_ssid_table_ptr = (RDD_IPTV_SSID_EXTENSION_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + IPTV_SSID_EXTENSION_TABLE_CAM_ADDRESS);
    }

    iptv_entry_ptr = &(iptv_table_ptr->entry[cache_index].iptv_l3_lookup_entry);

    forward_entry_ptr = &(forward_table_ptr->entry[cache_index]);

    iptv_ssid_entry_ptr = &(iptv_ssid_table_ptr->entry[cache_index]);

    RDD_IPTV_L3_LOOKUP_ENTRY_ANY_READ(any, iptv_entry_ptr);

    if (any)
        return BDMF_ERR_NOENT;

    RDD_IPTV_L3_LOOKUP_ENTRY_ANY_WRITE(1, iptv_entry_ptr);
    RDD_IPTV_CONTEXT_ENTRY_REPLICATION_NUMBER_WRITE(_rdd_extract_replication_number(entry->egress_port_vector), forward_entry_ptr);
    RDD_IPTV_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_WRITE(entry->egress_port_vector, forward_entry_ptr);
    RDD_IPTV_SSID_EXTENSION_ENTRY_WLAN_MCAST_INDEX_WRITE(entry->wlan_mcast_index, iptv_ssid_entry_ptr);
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CONTEXT_WRITE(entry->ic_context, iptv_ingress_classification_context_entry_ptr);

    return BDMF_ERR_OK;
}

static int _rdd_add_iptv_l3_dst_ip_src_ip_existing_any_entry_to_ddr(rdd_iptv_entry_t *entry, uint32_t base_index)
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS *iptv_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_TABLE_DTS *iptv_context_table_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *iptv_ssm_context_entry_ptr;
    uint32_t any;

    iptv_table_ptr = (RDD_IPTV_DDR_LOOKUP_TABLE_DTS *)IPTV_TABLE_PTR;

    iptv_entry_ptr = &(iptv_table_ptr->entry[base_index]);

    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_ANY_READ(any, iptv_entry_ptr);

    if (any)
        return BDMF_ERR_NOENT;

    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_ANY_WRITE(1, iptv_entry_ptr);

    iptv_context_table_ptr = (RDD_IPTV_DDR_CONTEXT_TABLE_DTS *)IPTV_CONTEXT_TABLE_PTR;

    /* in ssm mode the context of entries with src ip = any is written according to the "regular" context table according to the ssm mode context format */
    iptv_ssm_context_entry_ptr = &(iptv_context_table_ptr->entry[base_index]);

    RDD_IPTV_DDR_CONTEXT_ENTRY_REPLICATION_NUMBER_WRITE(_rdd_extract_replication_number(entry->egress_port_vector), iptv_ssm_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_WRITE(entry->egress_port_vector, iptv_ssm_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_WRITE(0, iptv_ssm_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_WLAN_MCAST_INDEX_WRITE(entry->wlan_mcast_index, iptv_ssm_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_WRITE(1, iptv_ssm_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_WRITE(entry->ic_context, iptv_ssm_context_entry_ptr);

    return BDMF_ERR_OK;
}

static void _rdd_add_iptv_l3_dst_ip_src_ip_existing_entry_to_cache(uint32_t src_ip_index, uint32_t cache_index)
{
    RDD_IPTV_LOOKUP_TABLE_DTS *iptv_table_ptr;
    RDD_IPTV_L3_LOOKUP_ENTRY_DTS *iptv_entry_ptr;

    iptv_table_ptr = (RDD_IPTV_LOOKUP_TABLE_DTS *)g_hash_table_cfg[RDD_IPTV_TABLE].hash_table_ptr;

    if (cache_index >= RDD_IPTV_LOOKUP_TABLE_SIZE)
    {
        cache_index -= RDD_IPTV_LOOKUP_TABLE_SIZE;

        iptv_table_ptr = (RDD_IPTV_LOOKUP_TABLE_DTS *)g_hash_table_cfg[RDD_IPTV_TABLE].cam_table_ptr;
    }

    iptv_entry_ptr = &(iptv_table_ptr->entry[cache_index].iptv_l3_lookup_entry);

    RDD_IPTV_L3_LOOKUP_ENTRY_CONTEXT_VALID_WRITE(1, iptv_entry_ptr);
}

static void _rdd_add_iptv_l3_dst_ip_src_ip_existing_entry_to_ddr(rdd_iptv_entry_t *entry, uint32_t src_ip_index,
    uint32_t base_index, uint32_t *index)
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS *iptv_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS *iptv_entry_ptr;
    RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS *iptv_ssm_context_table_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *iptv_ssm_context_entry_ptr;
    uint32_t context_table_index;

    iptv_table_ptr = (RDD_IPTV_DDR_LOOKUP_TABLE_DTS *)IPTV_TABLE_PTR;

    iptv_entry_ptr = &(iptv_table_ptr->entry[base_index]);

    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_VALID_WRITE(1, iptv_entry_ptr);
    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_TABLE_READ(context_table_index, iptv_entry_ptr);

    iptv_ssm_context_table_ptr = (RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS *)IPTV_SSM_CONTEXT_TABLE_PTR;

    iptv_ssm_context_entry_ptr = &(iptv_ssm_context_table_ptr->entry[((context_table_index * RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS) + src_ip_index)]);

    RDD_IPTV_DDR_CONTEXT_ENTRY_REPLICATION_NUMBER_WRITE(_rdd_extract_replication_number(entry->egress_port_vector), iptv_ssm_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_WRITE(entry->egress_port_vector, iptv_ssm_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_WRITE(0, iptv_ssm_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_WLAN_MCAST_INDEX_WRITE(entry->wlan_mcast_index, iptv_ssm_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_WRITE(1, iptv_ssm_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_WRITE(entry->ic_context, iptv_ssm_context_entry_ptr);

    *index = GET_ENTRY_INDEX_FROM_BASE_AND_PROVIDER_INDICES(base_index, src_ip_index);
}

static int _rdd_add_iptv_l3_dst_ip_src_ip_base_entry_to_cache(rdd_iptv_entry_t *entry, uint8_t *hash_entry,
    uint32_t *cache_index, uint32_t *context_index)
{
    RDD_IPTV_LOOKUP_TABLE_DTS *iptv_table_ptr;
    RDD_IPTV_L3_LOOKUP_ENTRY_DTS *iptv_entry_ptr;
    uint8_t context_entry;
    uint32_t entry_index;
    int rdd_error;

    /* allocate a new context table */
    *context_index = g_iptv_context_tables_free_list_head;

    g_iptv_context_tables_free_list_head = g_iptv_context_tables_free_list->entry[g_iptv_context_tables_free_list_head];

    context_entry = 0;

    rdd_error = rdd_add_hash_entry_64_bit(&g_hash_table_cfg[RDD_IPTV_TABLE], hash_entry, (uint8_t *)&context_entry,
        IPTV_ENTRY_KEY_MASK_HIGH, IPTV_ENTRY_KEY_MASK_LOW, 0, &entry_index);

    if (rdd_error)
    {
        *cache_index = (RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1);

        return BDMF_ERR_NORES;
    }

    *cache_index = entry_index;

    iptv_table_ptr = (RDD_IPTV_LOOKUP_TABLE_DTS *)g_hash_table_cfg[RDD_IPTV_TABLE].hash_table_ptr;

    if (entry_index >= RDD_IPTV_LOOKUP_TABLE_SIZE)
    {
        entry_index -= RDD_IPTV_LOOKUP_TABLE_SIZE;

        iptv_table_ptr = (RDD_IPTV_LOOKUP_TABLE_DTS *)g_hash_table_cfg[RDD_IPTV_TABLE].cam_table_ptr;
    }

    iptv_entry_ptr = &(iptv_table_ptr->entry[entry_index].iptv_l3_lookup_entry);

    RDD_IPTV_L3_LOOKUP_ENTRY_CONTEXT_TABLE_WRITE(*context_index, iptv_entry_ptr);

    return rdd_error;
}

static int _rdd_add_iptv_l3_dst_ip_src_ip_base_entry_to_ddr(rdd_iptv_entry_t *entry, uint8_t *entry_bytes,
    uint32_t cache_index, uint32_t context_index, rdpa_iptv_lookup_method iptv_mode, bdmf_ip_t *ipv6_dst_ip_ptr,
    uint32_t *index)
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS *iptv_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_TABLE_DTS *iptv_context_table_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *iptv_context_entry_ptr;
    uint32_t crc_init_value;
    uint32_t crc_result; 
    uint32_t hash_index; 
    uint32_t tries;
    uint32_t iptv_entry_index;
    uint32_t iptv_entry_valid;
    uint32_t is_entry_cached;

    is_entry_cached = (cache_index < (RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1));

    crc_init_value = rdd_crc_init_value_get(RDD_CRC_TYPE_32);

    crc_result = rdd_crc_bit_by_bit(&entry_bytes[4], 12, 0, crc_init_value, RDD_CRC_TYPE_32);

    hash_index = crc_result & (RDD_IPTV_DDR_LOOKUP_TABLE_SIZE / RDD_IPTV_TABLE_SET_SIZE - 1);

    hash_index = hash_index * RDD_IPTV_TABLE_SET_SIZE;

    iptv_table_ptr = (RDD_IPTV_DDR_LOOKUP_TABLE_DTS *)IPTV_TABLE_PTR;

    for (tries = 0; tries < RDD_IPTV_TABLE_SET_SIZE; tries++)
    {
        iptv_entry_index = (hash_index + tries) & (RDD_IPTV_DDR_LOOKUP_TABLE_SIZE - 1);

        iptv_entry_ptr = &(iptv_table_ptr->entry[iptv_entry_index]);

        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_VALID_READ(iptv_entry_valid, iptv_entry_ptr);

        if (!(iptv_entry_valid))
            break;
    }

    if (tries == RDD_IPTV_TABLE_SET_SIZE)
    {
        if (is_entry_cached)
            _rdd_delete_iptv_entry_from_cache(cache_index);

        return BDMF_ERR_NO_MORE;
    }

    if (ipv6_dst_ip_ptr != NULL) 
    {
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP12_WRITE(ipv6_dst_ip_ptr->addr.ipv6.data[12], iptv_entry_ptr);
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP13_WRITE(ipv6_dst_ip_ptr->addr.ipv6.data[13], iptv_entry_ptr);
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP14_WRITE(ipv6_dst_ip_ptr->addr.ipv6.data[14], iptv_entry_ptr);
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP15_WRITE(ipv6_dst_ip_ptr->addr.ipv6.data[15], iptv_entry_ptr);
    }

    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_WRITE((entry->key.mcast_group.l3.gr_ip.addr.ipv4 >> 24) & 0xFF, iptv_entry_ptr);
    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_WRITE((entry->key.mcast_group.l3.gr_ip.addr.ipv4 >> 16) & 0xFF, iptv_entry_ptr);
    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_WRITE((entry->key.mcast_group.l3.gr_ip.addr.ipv4 >>  8) & 0xFF, iptv_entry_ptr);
    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_WRITE((entry->key.mcast_group.l3.gr_ip.addr.ipv4 >>  0) & 0xFF, iptv_entry_ptr);

    if (iptv_mode == iptv_lookup_method_group_ip_src_ip_vid)
        RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_VID_WRITE(entry->key.vid, iptv_entry_ptr);

    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_TABLE_WRITE(context_index , iptv_entry_ptr);

    iptv_context_table_ptr = (RDD_IPTV_DDR_CONTEXT_TABLE_DTS *)IPTV_CONTEXT_TABLE_PTR;

    iptv_context_entry_ptr = &(iptv_context_table_ptr->entry[iptv_entry_index]);

    RDD_IPTV_DDR_CONTEXT_ENTRY_REPLICATION_NUMBER_WRITE(_rdd_extract_replication_number(entry->egress_port_vector), iptv_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_WRITE(entry->egress_port_vector, iptv_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_WRITE(0, iptv_context_entry_ptr);

    if (is_entry_cached)
    {
        RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_WRITE(1, iptv_context_entry_ptr);
        RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_WRITE(cache_index, iptv_context_entry_ptr);
    }

    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_VALID_WRITE(1, iptv_entry_ptr);

    *index = iptv_entry_index;

    return BDMF_ERR_OK;
}

static int _rdd_add_iptv_l3_dst_ip_src_ip_existing_any_entry(rdd_iptv_entry_t  *entry, uint32_t base_index,
    uint32_t cache_index)
{
    int  rdd_error = 0;

    if (cache_index < (RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1))
        rdd_error = _rdd_add_iptv_l3_dst_ip_src_ip_existing_any_entry_to_cache(entry, cache_index);

    if (!rdd_error)
        rdd_error = _rdd_add_iptv_l3_dst_ip_src_ip_existing_any_entry_to_ddr(entry, base_index);

    return rdd_error;
}

static void _rdd_add_iptv_l3_dst_ip_src_ip_existing_entry(rdd_iptv_entry_t *entry, uint32_t base_index,
    uint32_t cache_index, uint32_t src_ip_index, uint32_t *index)
{
    /* update the cache */
    if (cache_index < (RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1))
        _rdd_add_iptv_l3_dst_ip_src_ip_existing_entry_to_cache(src_ip_index, cache_index);

    _rdd_add_iptv_l3_dst_ip_src_ip_existing_entry_to_ddr(entry, src_ip_index, base_index, index);
}

static int _rdd_add_iptv_l3_dst_ip_src_ip_first_any_entry(rdd_iptv_entry_t *entry,
    rdpa_iptv_lookup_method iptv_mode, bdmf_ip_t *ipv6_dst_ip_ptr, uint32_t *index, uint32_t *cache)
{
    uint8_t entry_bytes[RDD_IPTV_ENTRY_SIZE];
    uint8_t hash_entry[8];
    uint32_t cache_index;
    uint32_t context_index;
    int rdd_error;

    if (g_iptv_context_tables_free_list_head == RDD_IPTV_SSM_CONTEXT_ENTRY_COUNT)
        return BDMF_ERR_NO_MORE;

    if (ipv6_dst_ip_ptr == NULL) 
    {
        MEMSET(hash_entry, 0, 8);

        if (iptv_mode == iptv_lookup_method_group_ip_src_ip_vid)
        {
            hash_entry[2] = entry->key.vid >> 8;
            hash_entry[3] = entry->key.vid & 0xFF;
        }

        hash_entry[4] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >> 24) & 0xFF;
        hash_entry[5] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >> 16) & 0xFF;
        hash_entry[6] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >>  8) & 0xFF;
        hash_entry[7] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >>  0) & 0xFF;

        rdd_error = _rdd_add_iptv_l3_dst_ip_src_ip_base_entry_to_cache(entry, hash_entry, &cache_index, &context_index);

        *cache = (rdd_error == BDMF_ERR_OK);
    }
    else
    {
        /* allocate a new context table */
        context_index = g_iptv_context_tables_free_list_head;
        g_iptv_context_tables_free_list_head = g_iptv_context_tables_free_list->entry[g_iptv_context_tables_free_list_head];
        cache_index = RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1;
        *cache = 0;
    }

    MEMSET(entry_bytes, 0, RDD_IPTV_ENTRY_SIZE);

    if (ipv6_dst_ip_ptr != NULL) 
    {
        entry_bytes[4] = ipv6_dst_ip_ptr->addr.ipv6.data[12];
        entry_bytes[5] = ipv6_dst_ip_ptr->addr.ipv6.data[13];
        entry_bytes[6] = ipv6_dst_ip_ptr->addr.ipv6.data[14];
        entry_bytes[7] = ipv6_dst_ip_ptr->addr.ipv6.data[15];
    }

    if (iptv_mode == iptv_lookup_method_group_ip_src_ip_vid)
    {
        entry_bytes[10] = entry->key.vid >> 8;
        entry_bytes[11] = entry->key.vid & 0xFF;
    }

    entry_bytes[12] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >> 24) & 0xFF;
    entry_bytes[13] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >> 16) & 0xFF;
    entry_bytes[14] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >>  8) & 0xFF;
    entry_bytes[15] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >>  0) & 0xFF;

    rdd_error = _rdd_add_iptv_l3_dst_ip_src_ip_base_entry_to_ddr(entry, entry_bytes, cache_index, context_index, iptv_mode, ipv6_dst_ip_ptr, index);

    if (rdd_error == BDMF_ERR_OK)
        rdd_error = _rdd_add_iptv_l3_dst_ip_src_ip_existing_any_entry(entry, *index, cache_index);
    else
        _rdd_free_iptv_l3_dst_ip_src_ip_context_table(context_index);

    return rdd_error;
}

static int _rdd_add_iptv_l3_dst_ip_src_ip_first_entry(rdd_iptv_entry_t *entry, rdpa_iptv_lookup_method iptv_mode,
    bdmf_ip_t *ipv6_dst_ip_ptr, bdmf_ip_t *ipv6_src_ip_ptr, uint32_t *index, uint32_t *cache)
{
    uint8_t  entry_bytes[RDD_IPTV_ENTRY_SIZE], hash_entry[8];
    uint32_t cache_index, base_index, src_ip_index, context_index;
    int  rdd_error;

    if (g_iptv_context_tables_free_list_head == RDD_IPTV_SSM_CONTEXT_ENTRY_COUNT)
        return BDMF_ERR_NO_MORE;

    rdd_error = _rdd_add_iptv_layer3_src_ip(&(entry->key.mcast_group.l3.src_ip), ipv6_src_ip_ptr, &src_ip_index);

    if (rdd_error != BDMF_ERR_OK)
        return rdd_error;

    if (ipv6_dst_ip_ptr == NULL)
    {
        MEMSET(hash_entry, 0, 8);

        if (iptv_mode == iptv_lookup_method_group_ip_src_ip_vid)
        {
            hash_entry[2] = entry->key.vid >> 8;
            hash_entry[3] = entry->key.vid & 0xFF;
        }

        hash_entry[4] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >> 24) & 0xFF;
        hash_entry[5] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >> 16) & 0xFF;
        hash_entry[6] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >>  8) & 0xFF;
        hash_entry[7] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >>  0) & 0xFF;

        rdd_error = _rdd_add_iptv_l3_dst_ip_src_ip_base_entry_to_cache(entry, hash_entry, &cache_index, &context_index);

        *cache = (rdd_error == BDMF_ERR_OK);
    }
    else
    {
        /* allocate a new context table */
        context_index = g_iptv_context_tables_free_list_head;
        g_iptv_context_tables_free_list_head = g_iptv_context_tables_free_list->entry[g_iptv_context_tables_free_list_head];
        cache_index = RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1;
        *cache = 0;
    }

    MEMSET(entry_bytes, 0, RDD_IPTV_ENTRY_SIZE);

    if (ipv6_dst_ip_ptr != NULL)
    {
        entry_bytes[4] = ipv6_dst_ip_ptr->addr.ipv6.data[12];
        entry_bytes[5] = ipv6_dst_ip_ptr->addr.ipv6.data[13];
        entry_bytes[6] = ipv6_dst_ip_ptr->addr.ipv6.data[14];
        entry_bytes[7] = ipv6_dst_ip_ptr->addr.ipv6.data[15];
    }

    if (iptv_mode == iptv_lookup_method_group_ip_src_ip_vid)
    {
        entry_bytes[10] = entry->key.vid >> 8;
        entry_bytes[11] = entry->key.vid & 0xFF;
    }

    entry_bytes[12] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >> 24) & 0xFF;
    entry_bytes[13] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >> 16) & 0xFF;
    entry_bytes[14] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >>  8) & 0xFF;
    entry_bytes[15] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >>  0) & 0xFF;

    rdd_error = _rdd_add_iptv_l3_dst_ip_src_ip_base_entry_to_ddr(entry, entry_bytes, cache_index, context_index, iptv_mode, ipv6_dst_ip_ptr, &base_index);

    if (rdd_error == BDMF_ERR_OK)
        _rdd_add_iptv_l3_dst_ip_src_ip_existing_entry(entry, base_index, cache_index, src_ip_index, index);
    else
    {
        _rdd_delete_iptv_layer3_src_ip(&(entry->key.mcast_group.l3.src_ip), &src_ip_index);
        _rdd_free_iptv_l3_dst_ip_src_ip_context_table(context_index);
    }

    return rdd_error;
}

static int _rdd_find_iptv_l3_dst_ip_src_ip_base_entry(rdd_iptv_entry_t *entry, rdpa_iptv_lookup_method iptv_mode,
    bdmf_ip_t *ipv6_dst_ip_ptr, uint32_t *base_index, uint32_t *cache_index)
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS *iptv_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_TABLE_DTS *iptv_context_table_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *iptv_context_entry_ptr;
    uint32_t iptv_entry_index, crc_init_value, crc_result, hash_index, iptv_entry_valid, tries, is_entry_cached, vid;
    uint8_t entry_bytes[RDD_IPTV_ENTRY_SIZE], dst_ip_address[4], ipv6_dst_ip_address[4];
    bdmf_ip_t dst_ip;

    iptv_table_ptr = (RDD_IPTV_DDR_LOOKUP_TABLE_DTS *)IPTV_TABLE_PTR;

    crc_init_value = rdd_crc_init_value_get(RDD_CRC_TYPE_32);

    MEMSET(entry_bytes, 0, RDD_IPTV_ENTRY_SIZE);

    if (ipv6_dst_ip_ptr != NULL)
    {
        entry_bytes[4] = ipv6_dst_ip_ptr->addr.ipv6.data[12];
        entry_bytes[5] = ipv6_dst_ip_ptr->addr.ipv6.data[13];
        entry_bytes[6] = ipv6_dst_ip_ptr->addr.ipv6.data[14];
        entry_bytes[7] = ipv6_dst_ip_ptr->addr.ipv6.data[15];
    }

    if (iptv_mode == iptv_lookup_method_group_ip_src_ip_vid)
    {
        entry_bytes[10] = entry->key.vid >> 8;
        entry_bytes[11] = entry->key.vid & 0xFF;
    }

    entry_bytes[12] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >> 24) & 0xFF;
    entry_bytes[13] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >> 16) & 0xFF;
    entry_bytes[14] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >>  8) & 0xFF;
    entry_bytes[15] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >>  0) & 0xFF;

    crc_result = rdd_crc_bit_by_bit(&entry_bytes[4], 12, 0, crc_init_value, RDD_CRC_TYPE_32);

    hash_index = crc_result & (RDD_IPTV_DDR_LOOKUP_TABLE_SIZE / RDD_IPTV_TABLE_SET_SIZE - 1);

    hash_index = hash_index * RDD_IPTV_TABLE_SET_SIZE;

    for (tries = 0; tries < RDD_IPTV_TABLE_SET_SIZE; tries++)
    {
        iptv_entry_index = (hash_index + tries) & (RDD_IPTV_DDR_LOOKUP_TABLE_SIZE - 1);

        iptv_entry_ptr = &(iptv_table_ptr->entry[iptv_entry_index]);

        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_VALID_READ(iptv_entry_valid, iptv_entry_ptr);

        if (iptv_entry_valid)
        {
            if (iptv_mode == iptv_lookup_method_group_ip_src_ip)
            {
                if (ipv6_dst_ip_ptr != NULL)
                {
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP12_READ(ipv6_dst_ip_address[0], iptv_entry_ptr);
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP13_READ(ipv6_dst_ip_address[1], iptv_entry_ptr);
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP14_READ(ipv6_dst_ip_address[2], iptv_entry_ptr);
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP15_READ(ipv6_dst_ip_address[3], iptv_entry_ptr);
                }

                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_READ(dst_ip_address[0], iptv_entry_ptr);
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_READ(dst_ip_address[1], iptv_entry_ptr);
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_READ(dst_ip_address[2], iptv_entry_ptr);
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_READ(dst_ip_address[3], iptv_entry_ptr);

                dst_ip.addr.ipv4  = (dst_ip_address[0] << 24);
                dst_ip.addr.ipv4 |= (dst_ip_address[1] << 16);
                dst_ip.addr.ipv4 |= (dst_ip_address[2] <<  8);
                dst_ip.addr.ipv4 |= (dst_ip_address[3] <<  0);

                if (dst_ip.addr.ipv4 == entry->key.mcast_group.l3.gr_ip.addr.ipv4)
                {
                    if (ipv6_dst_ip_ptr != NULL)
                    {
                        if (ipv6_dst_ip_address[0] != ipv6_dst_ip_ptr->addr.ipv6.data[12] ||
                            ipv6_dst_ip_address[1] != ipv6_dst_ip_ptr->addr.ipv6.data[13] ||
                            ipv6_dst_ip_address[2] != ipv6_dst_ip_ptr->addr.ipv6.data[14] ||
                            ipv6_dst_ip_address[3] != ipv6_dst_ip_ptr->addr.ipv6.data[15])
                        {
                            continue;
                        }
                    }

                    break;
                }
            }
            else
            {
                if (ipv6_dst_ip_ptr != NULL)
                {
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP12_READ(ipv6_dst_ip_address[0], iptv_entry_ptr);
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP13_READ(ipv6_dst_ip_address[1], iptv_entry_ptr);
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP14_READ(ipv6_dst_ip_address[2], iptv_entry_ptr);
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP15_READ(ipv6_dst_ip_address[3], iptv_entry_ptr);
                }

                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_READ(dst_ip_address[0], iptv_entry_ptr);
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_READ(dst_ip_address[1], iptv_entry_ptr);
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_READ(dst_ip_address[2], iptv_entry_ptr);
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_READ(dst_ip_address[3], iptv_entry_ptr);

                dst_ip.addr.ipv4  = (dst_ip_address[0] << 24);
                dst_ip.addr.ipv4 |= (dst_ip_address[1] << 16);
                dst_ip.addr.ipv4 |= (dst_ip_address[2] <<  8);
                dst_ip.addr.ipv4 |= (dst_ip_address[3] <<  0);

                RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_VID_READ(vid, iptv_entry_ptr);

                if ((dst_ip.addr.ipv4 == entry->key.mcast_group.l3.gr_ip.addr.ipv4) && (vid == entry->key.vid))
                {
                    if (ipv6_dst_ip_ptr != NULL)
                    {
                        if (ipv6_dst_ip_address[0] != ipv6_dst_ip_ptr->addr.ipv6.data[12] ||
                            ipv6_dst_ip_address[1] != ipv6_dst_ip_ptr->addr.ipv6.data[13] ||
                            ipv6_dst_ip_address[2] != ipv6_dst_ip_ptr->addr.ipv6.data[14] ||
                            ipv6_dst_ip_address[3] != ipv6_dst_ip_ptr->addr.ipv6.data[15])
                        {
                            continue;
                        }
                    }

                    break;
                }
            }
        }
    }

    if (tries == RDD_IPTV_TABLE_SET_SIZE)
        return BDMF_ERR_NOENT;

    *base_index = iptv_entry_index;

    iptv_context_table_ptr = (RDD_IPTV_DDR_CONTEXT_TABLE_DTS *)IPTV_CONTEXT_TABLE_PTR;
    iptv_context_entry_ptr = &(iptv_context_table_ptr->entry[iptv_entry_index]);

    RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_READ(is_entry_cached, iptv_context_entry_ptr);

    if (is_entry_cached)
        RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_READ(*cache_index, iptv_context_entry_ptr);
    else
        *cache_index = (RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1);

    return BDMF_ERR_OK;
}

static int _rdd_add_iptv_entry_to_cache(rdd_iptv_entry_t *entry, uint8_t *hash_entry, uint32_t *cache_index)
{
    RDD_IPTV_SSID_EXTENSION_TABLE_DTS *iptv_ssid_table_ptr;
    RDD_IPTV_SSID_EXTENSION_ENTRY_DTS *iptv_ssid_entry_ptr;
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS *iptv_ingress_classification_context_table_ptr; 
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS *iptv_ingress_classification_context_entry_ptr; 
#ifndef G9991
    uint8_t context_entry[2];
#else
    uint8_t context_entry[4];
#endif
    uint32_t entry_index;
    int rdd_error;

    context_entry[0] = _rdd_extract_replication_number(entry->egress_port_vector);
#ifndef G9991
    context_entry[1] = entry->egress_port_vector;
#else
    context_entry[1] = (uint8_t)((entry->egress_port_vector & 0xFF0000) >> 16);
    context_entry[2] = (uint8_t)((entry->egress_port_vector & 0xFF00) >> 8);
    context_entry[3] = (uint8_t)(entry->egress_port_vector & 0x00FF);
#endif

    rdd_error = rdd_add_hash_entry_64_bit(&g_hash_table_cfg[RDD_IPTV_TABLE], hash_entry, (uint8_t *)&context_entry,
        IPTV_ENTRY_KEY_MASK_HIGH, IPTV_ENTRY_KEY_MASK_LOW, 0, &entry_index);

    if (rdd_error)
    {
        *cache_index = (RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1);

        return BDMF_ERR_NORES;
    }

    iptv_ingress_classification_context_table_ptr = RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_PTR();

    iptv_ingress_classification_context_entry_ptr = &(iptv_ingress_classification_context_table_ptr->entry[entry_index]);

    *cache_index = entry_index;

    iptv_ssid_table_ptr = (RDD_IPTV_SSID_EXTENSION_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + IPTV_SSID_EXTENSION_TABLE_ADDRESS);

    if (entry_index >= RDD_IPTV_LOOKUP_TABLE_SIZE)
    {
        entry_index -= RDD_IPTV_LOOKUP_TABLE_SIZE;

        iptv_ssid_table_ptr = (RDD_IPTV_SSID_EXTENSION_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + IPTV_SSID_EXTENSION_TABLE_CAM_ADDRESS);
    }

    iptv_ssid_entry_ptr = &(iptv_ssid_table_ptr->entry[entry_index]);

    RDD_IPTV_SSID_EXTENSION_ENTRY_WLAN_MCAST_INDEX_WRITE(entry->wlan_mcast_index, iptv_ssid_entry_ptr);
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CONTEXT_WRITE(entry->ic_context, iptv_ingress_classification_context_entry_ptr);

    return rdd_error;
}

static int _rdd_add_iptv_entry_to_ddr(rdd_iptv_entry_t *entry, uint8_t *entry_bytes,
    rdpa_iptv_lookup_method iptv_classification_mode, uint32_t cache_index, bdmf_ip_t *ipv6_dst_ip_ptr, uint32_t *index)
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS *iptv_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_TABLE_DTS *iptv_context_table_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *iptv_context_entry_ptr;
    uint32_t crc_init_value, crc_result, hash_index, tries, iptv_entry_index, iptv_entry_valid, is_entry_cached;
    int rdd_error = 0;

    is_entry_cached = (cache_index < (RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1));

    crc_init_value = rdd_crc_init_value_get(RDD_CRC_TYPE_32);

    crc_result = rdd_crc_bit_by_bit(&entry_bytes[4], 12, 0, crc_init_value, RDD_CRC_TYPE_32);

    hash_index = crc_result & (RDD_IPTV_DDR_LOOKUP_TABLE_SIZE / RDD_IPTV_TABLE_SET_SIZE - 1);

    hash_index = hash_index * RDD_IPTV_TABLE_SET_SIZE;

    iptv_table_ptr = (RDD_IPTV_DDR_LOOKUP_TABLE_DTS *)IPTV_TABLE_PTR;

    for (tries = 0; tries < RDD_IPTV_TABLE_SET_SIZE; tries++)
    {
        iptv_entry_index = (hash_index + tries) & (RDD_IPTV_DDR_LOOKUP_TABLE_SIZE - 1);

        iptv_entry_ptr = &(iptv_table_ptr->entry[iptv_entry_index]);

        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_VALID_READ(iptv_entry_valid, iptv_entry_ptr);

        if (!(iptv_entry_valid))
            break;
    }

    if (tries == RDD_IPTV_TABLE_SET_SIZE)
    {
        if (is_entry_cached)
            _rdd_delete_iptv_entry_from_cache(cache_index);

        return BDMF_ERR_NO_MORE;
    }

    if (iptv_classification_mode == iptv_lookup_method_mac)
    {
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR5_WRITE(entry->key.mcast_group.mac.b[5], iptv_entry_ptr);
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR4_WRITE(entry->key.mcast_group.mac.b[4], iptv_entry_ptr);
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR3_WRITE(entry->key.mcast_group.mac.b[3], iptv_entry_ptr);
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR2_WRITE(entry->key.mcast_group.mac.b[2], iptv_entry_ptr);
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR1_WRITE(entry->key.mcast_group.mac.b[1], iptv_entry_ptr);
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR0_WRITE(entry->key.mcast_group.mac.b[0], iptv_entry_ptr);
    }
    else if (iptv_classification_mode == iptv_lookup_method_mac_vid)
    {
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR5_WRITE(entry->key.mcast_group.mac.b[5], iptv_entry_ptr);
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR4_WRITE(entry->key.mcast_group.mac.b[4], iptv_entry_ptr);
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR3_WRITE(entry->key.mcast_group.mac.b[3], iptv_entry_ptr);
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR2_WRITE(entry->key.mcast_group.mac.b[2], iptv_entry_ptr);
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR1_WRITE(entry->key.mcast_group.mac.b[1], iptv_entry_ptr);
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR0_WRITE(entry->key.mcast_group.mac.b[0], iptv_entry_ptr);
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_VID_WRITE(entry->key.vid, iptv_entry_ptr);
    }
    else
    {
        if (ipv6_dst_ip_ptr != NULL)
        {
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP12_WRITE(ipv6_dst_ip_ptr->addr.ipv6.data[12], iptv_entry_ptr);
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP13_WRITE(ipv6_dst_ip_ptr->addr.ipv6.data[13], iptv_entry_ptr);
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP14_WRITE(ipv6_dst_ip_ptr->addr.ipv6.data[14], iptv_entry_ptr);
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP15_WRITE(ipv6_dst_ip_ptr->addr.ipv6.data[15], iptv_entry_ptr);
        }

        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_WRITE(((entry->key.mcast_group.l3.gr_ip.addr.ipv4 >> 24) & 0xFF), iptv_entry_ptr);
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_WRITE(((entry->key.mcast_group.l3.gr_ip.addr.ipv4 >> 16) & 0xFF), iptv_entry_ptr);
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_WRITE(((entry->key.mcast_group.l3.gr_ip.addr.ipv4 >>  8) & 0xFF), iptv_entry_ptr);
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_WRITE(((entry->key.mcast_group.l3.gr_ip.addr.ipv4 >>  0) & 0xFF), iptv_entry_ptr);
    }

    iptv_context_table_ptr = (RDD_IPTV_DDR_CONTEXT_TABLE_DTS *)IPTV_CONTEXT_TABLE_PTR;
    iptv_context_entry_ptr = &(iptv_context_table_ptr->entry[iptv_entry_index]);

    RDD_IPTV_DDR_CONTEXT_ENTRY_REPLICATION_NUMBER_WRITE(_rdd_extract_replication_number(entry->egress_port_vector), iptv_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_WRITE(entry->egress_port_vector, iptv_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_WRITE(0, iptv_context_entry_ptr);

    if (is_entry_cached)
    {
        RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_WRITE(1, iptv_context_entry_ptr);
        RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_WRITE(cache_index, iptv_context_entry_ptr);
    }

    RDD_IPTV_DDR_CONTEXT_ENTRY_WLAN_MCAST_INDEX_WRITE(entry->wlan_mcast_index, iptv_context_entry_ptr);
    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_VALID_WRITE(1, iptv_entry_ptr);

    if (iptv_classification_mode <= iptv_lookup_method_mac_vid)
        RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_WRITE(entry->ic_context, iptv_context_entry_ptr);
    else
        RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_WRITE(entry->ic_context, iptv_context_entry_ptr);

    *index = iptv_entry_index;

    return rdd_error;
}

static int _rdd_add_iptv_l2_mac_entry(rdd_iptv_entry_t *entry, uint32_t *index, uint32_t *cache)
{
    uint8_t entry_bytes[RDD_IPTV_ENTRY_SIZE], hash_entry[8];
    uint32_t cache_index;
    int rdd_error;

    MEMSET(hash_entry, 0, 8);

    hash_entry[2] = entry->key.mcast_group.mac.b[0];
    hash_entry[3] = entry->key.mcast_group.mac.b[1];
    hash_entry[4] = entry->key.mcast_group.mac.b[2];
    hash_entry[5] = entry->key.mcast_group.mac.b[3];
    hash_entry[6] = entry->key.mcast_group.mac.b[4];
    hash_entry[7] = entry->key.mcast_group.mac.b[5];

    rdd_error = _rdd_add_iptv_entry_to_cache(entry, hash_entry, &cache_index);

    *cache = (rdd_error == BDMF_ERR_OK);

    MEMSET(entry_bytes, 0, RDD_IPTV_ENTRY_SIZE);

    entry_bytes[10] = entry->key.mcast_group.mac.b[0];
    entry_bytes[11] = entry->key.mcast_group.mac.b[1];
    entry_bytes[12] = entry->key.mcast_group.mac.b[2];
    entry_bytes[13] = entry->key.mcast_group.mac.b[3];
    entry_bytes[14] = entry->key.mcast_group.mac.b[4];
    entry_bytes[15] = entry->key.mcast_group.mac.b[5];

    rdd_error = _rdd_add_iptv_entry_to_ddr(entry, entry_bytes, iptv_lookup_method_mac, cache_index, 0, index);

    return rdd_error;
}

static int _rdd_add_iptv_l2_mac_vid_entry(rdd_iptv_entry_t *entry, uint32_t *index, uint32_t *cache)
{
    uint8_t entry_bytes[RDD_IPTV_ENTRY_SIZE], hash_entry[8];
    uint32_t cache_index;
    int rdd_error;

    hash_entry[0] = ((entry->key.vid & 0xF00) >> 8);
    hash_entry[1] = (entry->key.vid & 0x0FF);
    hash_entry[2] = entry->key.mcast_group.mac.b[0];
    hash_entry[3] = entry->key.mcast_group.mac.b[1];
    hash_entry[4] = entry->key.mcast_group.mac.b[2];
    hash_entry[5] = entry->key.mcast_group.mac.b[3];
    hash_entry[6] = entry->key.mcast_group.mac.b[4];
    hash_entry[7] = entry->key.mcast_group.mac.b[5];

    rdd_error = _rdd_add_iptv_entry_to_cache(entry, hash_entry, &cache_index);

    *cache = (rdd_error == BDMF_ERR_OK);

    MEMSET(entry_bytes, 0, RDD_IPTV_ENTRY_SIZE);

    entry_bytes[8] = entry->key.vid >> 8;
    entry_bytes[9] = entry->key.vid & 0xFF;
    entry_bytes[10] = entry->key.mcast_group.mac.b[0];
    entry_bytes[11] = entry->key.mcast_group.mac.b[1];
    entry_bytes[12] = entry->key.mcast_group.mac.b[2];
    entry_bytes[13] = entry->key.mcast_group.mac.b[3];
    entry_bytes[14] = entry->key.mcast_group.mac.b[4];
    entry_bytes[15] = entry->key.mcast_group.mac.b[5];

    rdd_error = _rdd_add_iptv_entry_to_ddr(entry, entry_bytes, iptv_lookup_method_mac_vid, cache_index, 0, index);

    return rdd_error;
}

static int _rdd_add_iptv_l3_dst_ip_src_ip_entry(rdd_iptv_entry_t *entry, rdpa_iptv_lookup_method iptv_mode,
    bdmf_ip_t *ipv6_dst_ip_ptr, bdmf_ip_t *ipv6_src_ip_ptr, uint32_t *index, uint32_t *cache)
{
    uint32_t base_index, cache_index, src_ip_index;
    int rdd_error;

    rdd_error = _rdd_find_iptv_l3_dst_ip_src_ip_base_entry(entry, iptv_mode, ipv6_dst_ip_ptr, &base_index, &cache_index);

    if (rdd_error == BDMF_ERR_OK)
    {
        if (entry->key.mcast_group.l3.src_ip.addr.ipv4 == 0)
        {
            rdd_error = _rdd_add_iptv_l3_dst_ip_src_ip_existing_any_entry(entry, base_index, cache_index);

            *index = base_index;
            *cache = (cache_index < (RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1));
        }
        else
        {
            rdd_error = _rdd_add_iptv_layer3_src_ip(&(entry->key.mcast_group.l3.src_ip), ipv6_src_ip_ptr, &src_ip_index);

            if (rdd_error == BDMF_ERR_OK)
            {
                _rdd_add_iptv_l3_dst_ip_src_ip_existing_entry(entry, base_index, cache_index, src_ip_index, index);

                *cache = (cache_index < (RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1));
            }
        }
    }
    else
    {
        if (entry->key.mcast_group.l3.src_ip.addr.ipv4 == 0)
            rdd_error = _rdd_add_iptv_l3_dst_ip_src_ip_first_any_entry(entry, iptv_mode, ipv6_dst_ip_ptr, index, cache);
        else
            rdd_error = _rdd_add_iptv_l3_dst_ip_src_ip_first_entry(entry, iptv_mode, ipv6_dst_ip_ptr, ipv6_src_ip_ptr, index, cache);
    }

    return rdd_error;
}

static int _rdd_add_iptv_l3_dst_ip_entry(rdd_iptv_entry_t  *entry, bdmf_ip_t *ipv6_dst_ip_ptr, uint32_t *index, uint32_t *cache)
{
    uint8_t entry_bytes[RDD_IPTV_ENTRY_SIZE], hash_entry[8];
    uint32_t cache_index;
    int rdd_error;

    if (ipv6_dst_ip_ptr == NULL)
    {
        MEMSET(hash_entry, 0, 8);

        hash_entry[4] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >> 24) & 0xFF;
        hash_entry[5] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >> 16) & 0xFF;
        hash_entry[6] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >>  8) & 0xFF;
        hash_entry[7] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >>  0) & 0xFF;

        rdd_error = _rdd_add_iptv_entry_to_cache(entry, hash_entry, &cache_index);

        *cache = (rdd_error == BDMF_ERR_OK);
    }
    else
    {
        cache_index = RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1;
        *cache = 0;
    }

    MEMSET(entry_bytes, 0, RDD_IPTV_ENTRY_SIZE);

    if (ipv6_dst_ip_ptr != NULL)
    {
        entry_bytes[4] = ipv6_dst_ip_ptr->addr.ipv6.data[12];
        entry_bytes[5] = ipv6_dst_ip_ptr->addr.ipv6.data[13];
        entry_bytes[6] = ipv6_dst_ip_ptr->addr.ipv6.data[14];
        entry_bytes[7] = ipv6_dst_ip_ptr->addr.ipv6.data[15];
    }

    entry_bytes[12] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >> 24) & 0xFF;
    entry_bytes[13] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >> 16) & 0xFF;
    entry_bytes[14] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >>  8) & 0xFF;
    entry_bytes[15] = (entry->key.mcast_group.l3.gr_ip.addr.ipv4 >>  0) & 0xFF;

    rdd_error = _rdd_add_iptv_entry_to_ddr(entry, entry_bytes, iptv_lookup_method_group_ip, cache_index, ipv6_dst_ip_ptr, index);

    return rdd_error;
}

int rdd_iptv_entry_add(rdd_iptv_entry_t *entry, uint32_t *index, uint32_t *cache)
{
    rdpa_iptv_lookup_method iptv_classification_mode;
    RDD_SYSTEM_CONFIGURATION_DTS *system_cfg;
    bdmf_ip_t ipv6_dst_ip_copy, ipv6_src_ip_copy, ipv6_src_ip_any;
#if !defined(FIRMWARE_INIT)
    uint32_t *ipv6_buffer_ptr;
#else
    uint32_t crc_result, crc_init_value;
#endif
    int rdd_error = 0;

    system_cfg = (RDD_SYSTEM_CONFIGURATION_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_SYSTEM_CONFIGURATION_ADDRESS);

    RDD_SYSTEM_CONFIGURATION_IPTV_CLASSIFICATION_METHOD_READ(iptv_classification_mode, system_cfg);

#if !defined(FIRMWARE_INIT)
    if (entry->key.mcast_group.l3.gr_ip.family == bdmf_ip_family_ipv6)
    {
        ipv6_buffer_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + IPV6_CRC_BUFFER_ADDRESS);

        bdmf_fastlock_lock(&cpu_message_lock);

        MWRITE_BLK_8(ipv6_buffer_ptr, entry->key.mcast_group.l3.gr_ip.addr.ipv6.data, 16);

        rdd_error = _rdd_cpu_message_send(RDD_CPU_MESSAGE_IPV6_CRC_GET, RDD_CLUSTER_0, 0, 0, 0, 1);

        if (rdd_error)
        {
            bdmf_fastlock_unlock(&cpu_message_lock);
            return rdd_error;
        }

        memcpy(ipv6_dst_ip_copy.addr.ipv6.data, entry->key.mcast_group.l3.gr_ip.addr.ipv6.data, 16);

        entry->key.mcast_group.l3.gr_ip.addr.ipv4 = swap4bytes(*(volatile uint32_t *)ipv6_buffer_ptr);

        if ((iptv_classification_mode == iptv_lookup_method_group_ip_src_ip) || (iptv_classification_mode == iptv_lookup_method_group_ip_src_ip_vid))
        {
            MEMSET(ipv6_src_ip_any.addr.ipv6.data, 0, 16);

            if (memcmp(ipv6_src_ip_any.addr.ipv6.data, entry->key.mcast_group.l3.src_ip.addr.ipv6.data, 16))
            {
                memcpy(ipv6_src_ip_copy.addr.ipv6.data, entry->key.mcast_group.l3.src_ip.addr.ipv6.data, 16);

                MWRITE_BLK_8(ipv6_buffer_ptr, entry->key.mcast_group.l3.src_ip.addr.ipv6.data, 16);

                rdd_error = _rdd_cpu_message_send(RDD_CPU_MESSAGE_IPV6_CRC_GET, RDD_CLUSTER_0, 0, 0, 0, 1);

                if (rdd_error)
                {
                    bdmf_fastlock_unlock(&cpu_message_lock);
                    return rdd_error;
                }

                entry->key.mcast_group.l3.src_ip.addr.ipv4 = swap4bytes(*(volatile uint32_t *)ipv6_buffer_ptr);
            }
            else
            {
                MEMSET(ipv6_src_ip_copy.addr.ipv6.data, 0, 16);
                entry->key.mcast_group.l3.src_ip.addr.ipv4 = 0;
            }
        }

        bdmf_fastlock_unlock(&cpu_message_lock);
    }
#else
    if (entry->key.mcast_group.l3.gr_ip.family == bdmf_ip_family_ipv6)
    {
        memcpy(ipv6_dst_ip_copy.addr.ipv6.data, entry->key.mcast_group.l3.gr_ip.addr.ipv6.data, 16);

        crc_init_value = rdd_crc_init_value_get(RDD_CRC_TYPE_32);
        crc_result = rdd_crc_bit_by_bit(&(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[0]), 16, 0, crc_init_value, RDD_CRC_TYPE_32);
       entry->key.mcast_group.l3.gr_ip.addr.ipv4 = crc_result;
        
        if ((iptv_classification_mode == iptv_lookup_method_group_ip_src_ip) || (iptv_classification_mode == iptv_lookup_method_group_ip_src_ip_vid))
        {
            MEMSET(ipv6_src_ip_any.addr.ipv6.data, 0, 16);

            if (memcmp(ipv6_src_ip_any.addr.ipv6.data, entry->key.mcast_group.l3.src_ip.addr.ipv6.data, 16))
            {
                memcpy(ipv6_src_ip_copy.addr.ipv6.data, entry->key.mcast_group.l3.src_ip.addr.ipv6.data, 16);

                crc_init_value = rdd_crc_init_value_get(RDD_CRC_TYPE_32);
                crc_result = rdd_crc_bit_by_bit(&(entry->key.mcast_group.l3.src_ip.addr.ipv6.data[0]), 16, 0, crc_init_value, RDD_CRC_TYPE_32);
               entry->key.mcast_group.l3.src_ip.addr.ipv4 = crc_result;
            }
            else
            {
                MEMSET(ipv6_src_ip_copy.addr.ipv6.data, 0, 16);

                entry->key.mcast_group.l3.src_ip.addr.ipv4 = 0;
            }
        }
    }
#endif
    bdmf_fastlock_lock(&int_lock);

    switch (iptv_classification_mode)
    {
    case iptv_lookup_method_mac:

         rdd_error = _rdd_add_iptv_l2_mac_entry(entry, index, cache);
         break;

    case iptv_lookup_method_mac_vid:

         rdd_error = _rdd_add_iptv_l2_mac_vid_entry(entry, index, cache);
         break;

    case iptv_lookup_method_group_ip_src_ip:

        if (entry->key.mcast_group.l3.gr_ip.family == bdmf_ip_family_ipv6)
            rdd_error = _rdd_add_iptv_l3_dst_ip_src_ip_entry(entry, iptv_classification_mode, &ipv6_dst_ip_copy, &ipv6_src_ip_copy, index, cache);
        else
            rdd_error = _rdd_add_iptv_l3_dst_ip_src_ip_entry(entry, iptv_classification_mode, 0, 0, index, cache);
        break;

    case iptv_lookup_method_group_ip:

        if (entry->key.mcast_group.l3.gr_ip.family == bdmf_ip_family_ipv6)
            rdd_error = _rdd_add_iptv_l3_dst_ip_entry(entry, &ipv6_dst_ip_copy, index, cache);
        else
            rdd_error = _rdd_add_iptv_l3_dst_ip_entry(entry, 0, index, cache);
        break;

     case iptv_lookup_method_group_ip_src_ip_vid:

        if (entry->key.mcast_group.l3.gr_ip.family == bdmf_ip_family_ipv6)
            rdd_error = _rdd_add_iptv_l3_dst_ip_src_ip_entry(entry, iptv_classification_mode, &ipv6_dst_ip_copy, &ipv6_src_ip_copy, index, cache);
        else
            rdd_error = _rdd_add_iptv_l3_dst_ip_src_ip_entry(entry, iptv_classification_mode, 0, 0, index, cache);
        break;
    }

    bdmf_fastlock_unlock(&int_lock);
    return rdd_error;
}

static int _rdd_modify_iptv_entry_in_cache(uint32_t entry_index_in_ddr, uint32_t egress_port_vector, uint16_t wifi_ssid_vector,
    uint8_t  ingress_classification_context)
{
    RDD_IPTV_DDR_CONTEXT_TABLE_DTS *iptv_context_table_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *iptv_context_entry_ptr;
    RDD_IPTV_CONTEXT_ENTRY_DTS *forward_entry_ptr;
    RDD_IPTV_CONTEXT_TABLE_DTS *forward_table_ptr;
    RDD_IPTV_SSID_EXTENSION_TABLE_DTS *iptv_ssid_table_ptr;
    RDD_IPTV_SSID_EXTENSION_ENTRY_DTS *iptv_ssid_entry_ptr;
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS *iptv_ingress_classification_context_table_ptr; 
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS *iptv_ingress_classification_context_entry_ptr; 
    uint32_t entry_cached, entry_index_in_cache;
    int rdd_error = 0;

    iptv_context_table_ptr = (RDD_IPTV_DDR_CONTEXT_TABLE_DTS *)IPTV_CONTEXT_TABLE_PTR;

    iptv_context_entry_ptr = &(iptv_context_table_ptr->entry[entry_index_in_ddr]);

    RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_READ(entry_cached, iptv_context_entry_ptr);

    if (!(entry_cached))
        return BDMF_ERR_OK;

    RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_READ(entry_index_in_cache, iptv_context_entry_ptr);

    forward_table_ptr = (RDD_IPTV_CONTEXT_TABLE_DTS *)(g_hash_table_cfg[RDD_IPTV_TABLE].context_table_ptr);

    iptv_ssid_table_ptr = (RDD_IPTV_SSID_EXTENSION_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + IPTV_SSID_EXTENSION_TABLE_ADDRESS);

    iptv_ingress_classification_context_table_ptr = RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_PTR();

    iptv_ingress_classification_context_entry_ptr = &(iptv_ingress_classification_context_table_ptr->entry[entry_index_in_cache]);

    if (entry_index_in_cache >= g_hash_table_cfg[RDD_IPTV_TABLE].hash_table_size)
    {
        entry_index_in_cache -= g_hash_table_cfg[RDD_IPTV_TABLE].hash_table_size;

        forward_table_ptr = (RDD_IPTV_CONTEXT_TABLE_DTS *)(g_hash_table_cfg[RDD_IPTV_TABLE].cam_context_table_ptr);

        iptv_ssid_table_ptr = (RDD_IPTV_SSID_EXTENSION_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + IPTV_SSID_EXTENSION_TABLE_CAM_ADDRESS);
    }

    forward_entry_ptr = &(forward_table_ptr->entry[entry_index_in_cache]);

    iptv_ssid_entry_ptr = &(iptv_ssid_table_ptr->entry[entry_index_in_cache]);

    RDD_IPTV_CONTEXT_ENTRY_REPLICATION_NUMBER_WRITE(_rdd_extract_replication_number(egress_port_vector), forward_entry_ptr);
    RDD_IPTV_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_WRITE(egress_port_vector, forward_entry_ptr);
    RDD_IPTV_SSID_EXTENSION_ENTRY_WLAN_MCAST_INDEX_WRITE(wifi_ssid_vector, iptv_ssid_entry_ptr);
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CONTEXT_WRITE(ingress_classification_context, iptv_ingress_classification_context_entry_ptr);

    return rdd_error;
}

static int _rdd_modify_iptv_entry_in_ddr(uint32_t entry_index, uint32_t egress_port_vector, uint16_t wifi_ssid_vector,
    uint8_t ingress_classification_context, rdpa_iptv_lookup_method iptv_mode)
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS *iptv_table_ptr;
    RDD_IPTV_DDR_CONTEXT_TABLE_DTS *iptv_context_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *iptv_context_entry_ptr;
    uint32_t iptv_entry_valid;

    iptv_table_ptr = (RDD_IPTV_DDR_LOOKUP_TABLE_DTS *)IPTV_TABLE_PTR;
    iptv_entry_ptr = &(iptv_table_ptr->entry[entry_index]);

    RDD_IPTV_L2_DDR_LOOKUP_ENTRY_VALID_READ(iptv_entry_valid, iptv_entry_ptr);

    if (!(iptv_entry_valid))
        return BDMF_ERR_NOENT;

    iptv_context_table_ptr = (RDD_IPTV_DDR_CONTEXT_TABLE_DTS *)IPTV_CONTEXT_TABLE_PTR;
    iptv_context_entry_ptr = &(iptv_context_table_ptr->entry[entry_index]);

    RDD_IPTV_DDR_CONTEXT_ENTRY_REPLICATION_NUMBER_WRITE(_rdd_extract_replication_number(egress_port_vector), iptv_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_WRITE(egress_port_vector, iptv_context_entry_ptr);
    RDD_IPTV_SSID_EXTENSION_ENTRY_WLAN_MCAST_INDEX_WRITE(wifi_ssid_vector, iptv_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_WRITE(ingress_classification_context, iptv_context_entry_ptr);

    return BDMF_ERR_OK;
}

static int _rdd_modify_iptv_ssm_context_entry_in_ddr(uint32_t entry_index, uint32_t egress_port_vector,
    uint16_t wifi_ssid_vector, uint8_t ingress_classification_context)
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS *iptv_table_ptr;
    RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS *iptv_ssm_context_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *iptv_context_entry_ptr;
    uint32_t iptv_entry_valid, provider_index, base_index, context_table_index;

    provider_index = GET_PROVIDER_INDEX_FROM_ENTRY_INDEX(entry_index);

    base_index = GET_BASE_INDEX_FROM_ENTRY_INDEX(entry_index);

    iptv_table_ptr = (RDD_IPTV_DDR_LOOKUP_TABLE_DTS *)IPTV_TABLE_PTR;
    iptv_entry_ptr = &(iptv_table_ptr->entry[base_index]);

    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_VALID_READ(iptv_entry_valid, iptv_entry_ptr);

    if (!(iptv_entry_valid))
        return BDMF_ERR_NOENT;

    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_TABLE_READ(context_table_index, iptv_entry_ptr);

    iptv_ssm_context_table_ptr = (RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS *)IPTV_SSM_CONTEXT_TABLE_PTR;
    iptv_context_entry_ptr = &(iptv_ssm_context_table_ptr->entry[context_table_index * RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS + provider_index]);

    RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_READ(iptv_entry_valid, iptv_context_entry_ptr);

    if (!(iptv_entry_valid))
        return BDMF_ERR_NOENT;

    RDD_IPTV_DDR_CONTEXT_ENTRY_REPLICATION_NUMBER_WRITE(_rdd_extract_replication_number(egress_port_vector), iptv_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_WRITE(egress_port_vector, iptv_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_WLAN_MCAST_INDEX_WRITE(wifi_ssid_vector, iptv_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_WRITE(ingress_classification_context, iptv_context_entry_ptr);

    return BDMF_ERR_OK;
}

static int _rdd_modify_iptv_l2_mac_entry(uint32_t entry_index, uint16_t egress_port_vector,
    uint16_t wifi_ssid_vector, uint8_t ingress_classification_context)
{
    int rdd_error;

    rdd_error = _rdd_modify_iptv_entry_in_ddr(entry_index, egress_port_vector, wifi_ssid_vector, ingress_classification_context, iptv_lookup_method_mac);

    if (!rdd_error)
        rdd_error = _rdd_modify_iptv_entry_in_cache(entry_index, egress_port_vector, wifi_ssid_vector, ingress_classification_context);

    return rdd_error;
}

static int _rdd_modify_iptv_l2_mac_vid_entry(uint32_t entry_index, uint16_t egress_port_vector,
    uint16_t wifi_ssid_vector, uint8_t ingress_classification_context)
{
    int rdd_error;

    rdd_error = _rdd_modify_iptv_entry_in_ddr(entry_index, egress_port_vector, wifi_ssid_vector, ingress_classification_context, iptv_lookup_method_mac_vid);

    if (!rdd_error)
        rdd_error = _rdd_modify_iptv_entry_in_cache(entry_index, egress_port_vector, wifi_ssid_vector, ingress_classification_context);

    return rdd_error;
}

static int _rdd_modify_iptv_l3_dst_ip_src_ip_entry(uint32_t entry_index, uint32_t egress_port_vector,
    uint16_t wifi_ssid_vector, uint8_t ingress_classification_context)
{
    int rdd_error;

    if (entry_index < RDD_IPTV_DDR_LOOKUP_TABLE_SIZE)
    {
        rdd_error = _rdd_modify_iptv_entry_in_ddr(entry_index, egress_port_vector, wifi_ssid_vector, ingress_classification_context, iptv_lookup_method_group_ip_src_ip);

        if (rdd_error == BDMF_ERR_OK)
            rdd_error = _rdd_modify_iptv_entry_in_cache(entry_index, egress_port_vector, wifi_ssid_vector, ingress_classification_context);
    }
    else
        rdd_error = _rdd_modify_iptv_ssm_context_entry_in_ddr(entry_index, egress_port_vector, wifi_ssid_vector, ingress_classification_context);

    return rdd_error;
}

static int _rdd_modify_iptv_l3_dst_ip_entry(uint32_t entry_index, uint32_t egress_port_vector,
    uint16_t wifi_ssid_vector, uint8_t ingress_classification_context)
{
    int rdd_error;

    rdd_error = _rdd_modify_iptv_entry_in_ddr(entry_index, egress_port_vector, wifi_ssid_vector, ingress_classification_context, iptv_lookup_method_group_ip);

    if (!rdd_error)
        rdd_error = _rdd_modify_iptv_entry_in_cache(entry_index, egress_port_vector, wifi_ssid_vector, ingress_classification_context);

    return rdd_error;
}

static inline int _rdd_modify_iptv_l3_dst_ip_src_ip_vid_entry(uint32_t entry_index, uint32_t egress_port_vector,
    uint16_t wifi_ssid_vector, uint8_t ingress_classification_context)
{
    return _rdd_modify_iptv_l3_dst_ip_src_ip_entry(entry_index, egress_port_vector, wifi_ssid_vector, ingress_classification_context);
}

int rdd_iptv_entry_modify(uint32_t entry_index, uint32_t egress_port_vector,
    uint16_t wifi_ssid_vector, uint8_t ingress_classification_context)
{
    rdpa_iptv_lookup_method iptv_classification_mode;
    RDD_SYSTEM_CONFIGURATION_DTS *system_cfg;
    int rdd_error;

    rdd_error = BDMF_ERR_OK;

    system_cfg = (RDD_SYSTEM_CONFIGURATION_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_SYSTEM_CONFIGURATION_ADDRESS);

    RDD_SYSTEM_CONFIGURATION_IPTV_CLASSIFICATION_METHOD_READ(iptv_classification_mode, system_cfg);

    bdmf_fastlock_lock(&int_lock);

    switch (iptv_classification_mode)
    {
    case iptv_lookup_method_mac:

         rdd_error = _rdd_modify_iptv_l2_mac_entry(entry_index, egress_port_vector, wifi_ssid_vector, ingress_classification_context);
         break;

    case iptv_lookup_method_mac_vid:

         rdd_error = _rdd_modify_iptv_l2_mac_vid_entry(entry_index, egress_port_vector, wifi_ssid_vector, ingress_classification_context);
         break;

    case iptv_lookup_method_group_ip_src_ip:

         rdd_error = _rdd_modify_iptv_l3_dst_ip_src_ip_entry(entry_index, egress_port_vector, wifi_ssid_vector, ingress_classification_context);
         break;

    case iptv_lookup_method_group_ip:

        rdd_error = _rdd_modify_iptv_l3_dst_ip_entry(entry_index, egress_port_vector, wifi_ssid_vector, ingress_classification_context);
        break;

    case iptv_lookup_method_group_ip_src_ip_vid:

         rdd_error = _rdd_modify_iptv_l3_dst_ip_src_ip_vid_entry(entry_index, egress_port_vector, wifi_ssid_vector, ingress_classification_context);
         break;
    }

    bdmf_fastlock_unlock(&int_lock);
    return rdd_error;
}

static int _rdd_delete_iptv_entry_from_ddr(uint32_t entry_index, uint32_t *cache_index,
    uint32_t *cache)
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS *iptv_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_TABLE_DTS *iptv_context_table_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *iptv_context_entry_ptr;
    uint32_t iptv_entry_valid;

    iptv_table_ptr = (RDD_IPTV_DDR_LOOKUP_TABLE_DTS *)IPTV_TABLE_PTR;
    iptv_entry_ptr = &(iptv_table_ptr->entry[entry_index]);

    RDD_IPTV_L2_DDR_LOOKUP_ENTRY_VALID_READ(iptv_entry_valid, iptv_entry_ptr);

    if (!(iptv_entry_valid))
        return BDMF_ERR_NOENT;

    iptv_context_table_ptr = (RDD_IPTV_DDR_CONTEXT_TABLE_DTS *)IPTV_CONTEXT_TABLE_PTR;
    iptv_context_entry_ptr = &(iptv_context_table_ptr->entry[entry_index]);

    RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_READ(*cache, iptv_context_entry_ptr);

    if (*cache)
        RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_READ(*cache_index, iptv_context_entry_ptr);

    MEMSET(iptv_context_entry_ptr, 0, sizeof(RDD_IPTV_DDR_CONTEXT_ENTRY_DTS));
    MEMSET(iptv_entry_ptr, 0, sizeof(RDD_IPTV_LOOKUP_DDR_UNION_DTS));

    return BDMF_ERR_OK;
}

static int _rdd_delete_iptv_l2_mac_entry(uint32_t entry_index, uint32_t	*cache)
{
    int rdd_error;
    uint32_t cache_index;

    rdd_error = _rdd_delete_iptv_entry_from_ddr(entry_index, &cache_index, cache);

    if (!rdd_error && (*cache))
        rdd_error = _rdd_delete_iptv_entry_from_cache(cache_index);

    return rdd_error;
}

static int _rdd_delete_iptv_l2_mac_vid_entry(uint32_t entry_index, uint32_t *cache)
{
    int rdd_error;
    uint32_t cache_index;

    rdd_error = _rdd_delete_iptv_entry_from_ddr(entry_index, &cache_index, cache);

    if (!rdd_error && (*cache))
        rdd_error = _rdd_delete_iptv_entry_from_cache(cache_index);

    return rdd_error;
}

static int _rdd_free_iptv_l3_dst_ip_src_ip_context_table(uint32_t context_table_index)
{
    g_iptv_context_tables_free_list->entry[g_iptv_context_tables_free_list_tail] = context_table_index;
    g_iptv_context_tables_free_list->entry[context_table_index] = RDD_IPTV_SSM_CONTEXT_ENTRY_COUNT;
    g_iptv_context_tables_free_list_tail = context_table_index;

    if (g_iptv_context_tables_free_list_head == RDD_IPTV_SSM_CONTEXT_ENTRY_COUNT)
        g_iptv_context_tables_free_list_head = context_table_index;

    return BDMF_ERR_OK;
}

static int _rdd_delete_iptv_l3_dst_ip_src_ip_any_entry_from_cache(uint32_t cache_index)
{
    RDD_IPTV_LOOKUP_TABLE_DTS *iptv_table_ptr;
    RDD_IPTV_L3_LOOKUP_ENTRY_DTS *iptv_entry_ptr;
    RDD_IPTV_CONTEXT_TABLE_DTS *forward_table_ptr;
    RDD_IPTV_CONTEXT_ENTRY_DTS *forward_entry_ptr;
    RDD_IPTV_COUNTERS_TABLE_DTS *iptv_counter_table_ptr;
    RDD_IPTV_COUNTER_ENTRY_DTS *iptv_counter_entry_ptr;
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS *iptv_ingress_classification_context_table_ptr; 
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS *iptv_ingress_classification_context_entry_ptr;

    iptv_table_ptr = (RDD_IPTV_LOOKUP_TABLE_DTS *)g_hash_table_cfg[RDD_IPTV_TABLE].hash_table_ptr;

    forward_table_ptr = (RDD_IPTV_CONTEXT_TABLE_DTS *)(g_hash_table_cfg[RDD_IPTV_TABLE].context_table_ptr);

    iptv_counter_table_ptr = (RDD_IPTV_COUNTERS_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + IPTV_COUNTERS_TABLE_ADDRESS);

    iptv_counter_entry_ptr = &(iptv_counter_table_ptr->entry[cache_index]);

    iptv_ingress_classification_context_table_ptr = RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_PTR();

    iptv_ingress_classification_context_entry_ptr = &(iptv_ingress_classification_context_table_ptr->entry[cache_index]);

    if (cache_index >= RDD_IPTV_LOOKUP_TABLE_SIZE)
    {
        cache_index -= RDD_IPTV_LOOKUP_TABLE_SIZE;

        iptv_table_ptr = (RDD_IPTV_LOOKUP_TABLE_DTS *)g_hash_table_cfg[RDD_IPTV_TABLE].cam_table_ptr;

        forward_table_ptr = (RDD_IPTV_CONTEXT_TABLE_DTS *)(g_hash_table_cfg[RDD_IPTV_TABLE].cam_context_table_ptr);
    }

    iptv_entry_ptr = &(iptv_table_ptr->entry[cache_index].iptv_l3_lookup_entry);

    forward_entry_ptr = &(forward_table_ptr->entry[cache_index]);

    RDD_IPTV_L3_LOOKUP_ENTRY_ANY_WRITE(0, iptv_entry_ptr);
    MEMSET(forward_entry_ptr, 0, sizeof(RDD_IPTV_CONTEXT_ENTRY_DTS));
    RDD_IPTV_COUNTER_ENTRY_COUNTER_WRITE(0, iptv_counter_entry_ptr);
    MEMSET(iptv_ingress_classification_context_entry_ptr, 0, sizeof(RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS));

    return BDMF_ERR_OK;
}

static int _rdd_delete_iptv_l3_dst_ip_src_ip_any_entry_from_ddr(uint32_t entry_index, uint32_t *cache,
    uint32_t *cache_index, uint32_t *base_deleted)
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS *iptv_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_TABLE_DTS *iptv_context_table_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *iptv_ssm_context_entry_ptr;
    uint32_t iptv_entry_valid, context_table_valid, context_table_index;
    int rdd_error;

    iptv_table_ptr = (RDD_IPTV_DDR_LOOKUP_TABLE_DTS *)IPTV_TABLE_PTR;

    iptv_entry_ptr = &(iptv_table_ptr->entry[entry_index]);

    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_VALID_READ(iptv_entry_valid, iptv_entry_ptr);

    if (!(iptv_entry_valid))
        return BDMF_ERR_NOENT;

    iptv_context_table_ptr = (RDD_IPTV_DDR_CONTEXT_TABLE_DTS *)IPTV_CONTEXT_TABLE_PTR;
    iptv_ssm_context_entry_ptr = &(iptv_context_table_ptr->entry[entry_index]);

    RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_READ(*cache, iptv_ssm_context_entry_ptr);

    if (*cache)
        RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_READ(*cache_index, iptv_ssm_context_entry_ptr);
    else
        *cache_index = (RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1);

    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_ANY_WRITE(0, iptv_entry_ptr);

    /* in ssm mode the context of entries with src ip = any is written according to the "regular" context table according to the ssm mode context format */
    RDD_IPTV_DDR_CONTEXT_ENTRY_REPLICATION_NUMBER_WRITE(0, iptv_ssm_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_WRITE(0, iptv_ssm_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_WRITE(0, iptv_ssm_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_WLAN_MCAST_INDEX_WRITE(0, iptv_ssm_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_WRITE(0, iptv_ssm_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_WRITE(0, iptv_ssm_context_entry_ptr);

    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_VALID_READ(context_table_valid, iptv_entry_ptr);

    if (!(context_table_valid))
    {
        RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_TABLE_READ(context_table_index, iptv_entry_ptr);

        _rdd_free_iptv_l3_dst_ip_src_ip_context_table(context_table_index);

        rdd_error = _rdd_delete_iptv_l3_dst_ip_entry(entry_index, cache);

        *base_deleted = 1;
    }
    else
    {
        rdd_error = 0;

        *base_deleted = 0;
    }

    return rdd_error;
}

static int _rdd_delete_iptv_l3_dst_ip_src_ip_non_any_entry_from_cache(uint32_t cache_index, uint32_t provider_index)
{
    RDD_IPTV_LOOKUP_TABLE_DTS *iptv_table_ptr;
    RDD_IPTV_L3_LOOKUP_ENTRY_DTS *iptv_entry_ptr;

    iptv_table_ptr = (RDD_IPTV_LOOKUP_TABLE_DTS *)g_hash_table_cfg[RDD_IPTV_TABLE].hash_table_ptr;

    if (cache_index >= RDD_IPTV_LOOKUP_TABLE_SIZE)
    {
        cache_index -= RDD_IPTV_LOOKUP_TABLE_SIZE;

        iptv_table_ptr = (RDD_IPTV_LOOKUP_TABLE_DTS *)g_hash_table_cfg[RDD_IPTV_TABLE].cam_table_ptr;
    }

    iptv_entry_ptr = &(iptv_table_ptr->entry[cache_index].iptv_l3_lookup_entry);

    RDD_IPTV_L3_LOOKUP_ENTRY_CONTEXT_VALID_WRITE(0, iptv_entry_ptr);

    return BDMF_ERR_OK;
}

static int _rdd_delete_iptv_l3_dst_ip_src_ip_non_any_entry_from_ddr(uint32_t entry_index, uint32_t provider_index,
    uint32_t *cache, uint32_t *cache_index, uint32_t *base_deleted)
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS *iptv_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_TABLE_DTS *iptv_context_table_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *iptv_context_entry_ptr;
    RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS *iptv_ssm_context_table_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *iptv_ssm_context_entry_ptr;
    RDD_IPTV_L3_SRC_IP_LOOKUP_TABLE_DTS *iptv_layer3_src_ip_table_ptr;
    RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *iptv_layer3_src_ip_entry_ptr;
    uint32_t any, context_table_index, context_table_offset, context_table_valid, valid_entries_count;
    bdmf_ip_t dst_ip, src_ip;
    uint32_t src_ip_index, context_entry_valid;
    uint8_t src_ip_addr[4];
    int rdd_error;

    iptv_table_ptr = (RDD_IPTV_DDR_LOOKUP_TABLE_DTS *)IPTV_TABLE_PTR;

    iptv_entry_ptr = &(iptv_table_ptr->entry[entry_index]);

    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_TABLE_READ(context_table_index, iptv_entry_ptr);

    context_table_offset = context_table_index * RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS;

    valid_entries_count = 0;

    iptv_ssm_context_table_ptr = (RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS *)IPTV_SSM_CONTEXT_TABLE_PTR;

    for (src_ip_index = 0; src_ip_index < RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS; src_ip_index++)
    {
        iptv_ssm_context_entry_ptr = &(iptv_ssm_context_table_ptr->entry[context_table_offset + src_ip_index]);

        RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_READ(context_entry_valid, iptv_ssm_context_entry_ptr);

        if (context_entry_valid)
        {
            valid_entries_count++;

            if (valid_entries_count == 2)
                break;
        }
    }

    if (valid_entries_count < 2)
        RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_VALID_WRITE(0, iptv_entry_ptr);

    /* delete context */
    iptv_ssm_context_entry_ptr = &(iptv_ssm_context_table_ptr->entry[context_table_offset + provider_index]);

    MEMSET(iptv_ssm_context_entry_ptr, 0, sizeof(RDD_IPTV_DDR_CONTEXT_ENTRY_DTS));

    /* read cache index */
    iptv_context_table_ptr = (RDD_IPTV_DDR_CONTEXT_TABLE_DTS *)IPTV_CONTEXT_TABLE_PTR;

    iptv_context_entry_ptr = &(iptv_context_table_ptr->entry[entry_index]);

    RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_READ(*cache, iptv_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_READ(*cache_index, iptv_context_entry_ptr);

    /* delete src ip */
    iptv_layer3_src_ip_table_ptr = RDD_IPTV_L3_SRC_IP_LOOKUP_TABLE_PTR();

    iptv_layer3_src_ip_entry_ptr = &(iptv_layer3_src_ip_table_ptr->entry[provider_index]);

    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DST_IP12_READ(dst_ip.addr.ipv6.data[12], iptv_entry_ptr);
    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DST_IP13_READ(dst_ip.addr.ipv6.data[13], iptv_entry_ptr);
    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DST_IP14_READ(dst_ip.addr.ipv6.data[14], iptv_entry_ptr);
    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DST_IP15_READ(dst_ip.addr.ipv6.data[15], iptv_entry_ptr);

    if (*((uint32_t *)&dst_ip.addr.ipv6.data[12]) != 0)
        src_ip.family = bdmf_ip_family_ipv6;
    else
        src_ip.family = bdmf_ip_family_ipv4;

    if (src_ip.family == bdmf_ip_family_ipv6)
    {
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_12_READ(src_ip.addr.ipv6.data[12], (RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *)iptv_layer3_src_ip_entry_ptr);
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_13_READ(src_ip.addr.ipv6.data[13], (RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *)iptv_layer3_src_ip_entry_ptr);
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_14_READ(src_ip.addr.ipv6.data[14], (RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *)iptv_layer3_src_ip_entry_ptr);
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_15_READ(src_ip.addr.ipv6.data[15], (RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *)iptv_layer3_src_ip_entry_ptr);
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_3_READ(src_ip.addr.ipv6.data[0], (RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *)iptv_layer3_src_ip_entry_ptr);
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_2_READ(src_ip.addr.ipv6.data[1], (RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *)iptv_layer3_src_ip_entry_ptr);
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_1_READ(src_ip.addr.ipv6.data[2], (RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *)iptv_layer3_src_ip_entry_ptr);
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_0_READ(src_ip.addr.ipv6.data[3], (RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *)iptv_layer3_src_ip_entry_ptr);
    }
    else
    {
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_0_READ(src_ip_addr[0], (RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *)iptv_layer3_src_ip_entry_ptr);
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_1_READ(src_ip_addr[1], (RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *)iptv_layer3_src_ip_entry_ptr);
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_2_READ(src_ip_addr[2], (RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *)iptv_layer3_src_ip_entry_ptr);
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_3_READ(src_ip_addr[3], (RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *)iptv_layer3_src_ip_entry_ptr);

        src_ip.addr.ipv4  = (src_ip_addr[0] << 24);
        src_ip.addr.ipv4 |= (src_ip_addr[1] << 16);
        src_ip.addr.ipv4 |= (src_ip_addr[2] <<  8);
        src_ip.addr.ipv4 |= (src_ip_addr[3] <<  0);
    }

    rdd_error = _rdd_delete_iptv_layer3_src_ip(&src_ip, &src_ip_index);

    /* check if base entry should be deleted as well */
    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_ANY_READ(any, iptv_entry_ptr);
    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_VALID_READ(context_table_valid, iptv_entry_ptr);

    if (!(any) && !(context_table_valid))
    {
        _rdd_free_iptv_l3_dst_ip_src_ip_context_table(context_table_index);

        rdd_error = _rdd_delete_iptv_l3_dst_ip_entry(entry_index, cache);

        *base_deleted = 1;
    }
    else
    {
        rdd_error = 0;

        *base_deleted = 0;
    }

    return rdd_error;
}

static int _rdd_delete_iptv_l3_dst_ip_src_ip_any_entry(uint32_t entry_index, uint32_t *cache)
{
    uint32_t cache_index, base_deleted;
    int rdd_error;

    rdd_error = _rdd_delete_iptv_l3_dst_ip_src_ip_any_entry_from_ddr(entry_index, cache, &cache_index, &base_deleted);

    if (!rdd_error && !(base_deleted) && (*cache))
        rdd_error = _rdd_delete_iptv_l3_dst_ip_src_ip_any_entry_from_cache(cache_index);

    return rdd_error;
}

static int _rdd_delete_iptv_l3_dst_ip_src_ip_non_any_entry(uint32_t entry_index, uint32_t provider_index,
    uint32_t *cache)
{
    uint32_t cache_index, base_deleted;
    int rdd_error;

    rdd_error = _rdd_delete_iptv_l3_dst_ip_src_ip_non_any_entry_from_ddr(entry_index, provider_index, cache, &cache_index, &base_deleted);

    if (!rdd_error && (base_deleted) && (*cache))
        rdd_error = _rdd_delete_iptv_l3_dst_ip_src_ip_non_any_entry_from_cache(cache_index, provider_index);

    return rdd_error;
}

static int _rdd_delete_iptv_l3_dst_ip_src_ip_entry(uint32_t entry_index, uint32_t *cache)
{
    int rdd_error;

    if (entry_index < RDD_IPTV_DDR_LOOKUP_TABLE_SIZE)
        rdd_error = _rdd_delete_iptv_l3_dst_ip_src_ip_any_entry(entry_index, cache);
    else
        rdd_error = _rdd_delete_iptv_l3_dst_ip_src_ip_non_any_entry(GET_BASE_INDEX_FROM_ENTRY_INDEX(entry_index),
            GET_PROVIDER_INDEX_FROM_ENTRY_INDEX(entry_index), cache);

    return rdd_error;
}

static int _rdd_delete_iptv_l3_dst_ip_entry(uint32_t entry_index, uint32_t *cache)
{
    uint32_t cache_index;
    int rdd_error;

    rdd_error = _rdd_delete_iptv_entry_from_ddr(entry_index, &cache_index, cache);

    if (!rdd_error && (*cache))
        rdd_error = _rdd_delete_iptv_entry_from_cache(cache_index);

    return rdd_error;
}

static inline int _rdd_delete_iptv_l3_dst_ip_src_ip_vid_entry(uint32_t entry_index, uint32_t *cache)
{
    return _rdd_delete_iptv_l3_dst_ip_src_ip_entry(entry_index, cache);
}

int rdd_iptv_entry_delete(uint32_t entry_index, uint32_t *cache)
{
    rdpa_iptv_lookup_method iptv_classification_mode;
    RDD_SYSTEM_CONFIGURATION_DTS *system_cfg;
    int rdd_error = 0;

    system_cfg = (RDD_SYSTEM_CONFIGURATION_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_SYSTEM_CONFIGURATION_ADDRESS);

    RDD_SYSTEM_CONFIGURATION_IPTV_CLASSIFICATION_METHOD_READ(iptv_classification_mode, system_cfg);

    bdmf_fastlock_lock(&int_lock);

    switch (iptv_classification_mode)
    {
    case iptv_lookup_method_mac:

         rdd_error = _rdd_delete_iptv_l2_mac_entry(entry_index, cache);
         break;

    case iptv_lookup_method_mac_vid:

         rdd_error = _rdd_delete_iptv_l2_mac_vid_entry(entry_index, cache);
         break;

    case iptv_lookup_method_group_ip_src_ip:

         rdd_error = _rdd_delete_iptv_l3_dst_ip_src_ip_entry(entry_index, cache);
         break;

    case iptv_lookup_method_group_ip:

        rdd_error = _rdd_delete_iptv_l3_dst_ip_entry(entry_index, cache);
        break;

    case iptv_lookup_method_group_ip_src_ip_vid:

         rdd_error = _rdd_delete_iptv_l3_dst_ip_src_ip_vid_entry(entry_index, cache);
         break;
    }

    bdmf_fastlock_unlock(&int_lock);
    return rdd_error;
}

static int _rdd_search_iptv_ssm_entry(rdpa_iptv_channel_key_t *key, bdmf_ip_t *ipv6_src_ip_ptr, uint32_t *index)
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS *iptv_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *iptv_context_entry_ptr;
    RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS *iptv_ssm_context_table_ptr;
    uint32_t iptv_entry_valid, src_ip_index, context_table_index, base_index;
    uint8_t src_ip_hash_entry[8];
    int rdd_error;

    if (ipv6_src_ip_ptr == NULL)
    {
        src_ip_hash_entry[0] = 0;
        src_ip_hash_entry[1] = 0;
        src_ip_hash_entry[2] = 0;
        src_ip_hash_entry[3] = 0;
    }
    else
    {
        src_ip_hash_entry[0] = ipv6_src_ip_ptr->addr.ipv6.data[12];
        src_ip_hash_entry[1] = ipv6_src_ip_ptr->addr.ipv6.data[13];
        src_ip_hash_entry[2] = ipv6_src_ip_ptr->addr.ipv6.data[14];
        src_ip_hash_entry[3] = ipv6_src_ip_ptr->addr.ipv6.data[15];
    }

    src_ip_hash_entry[4] = (key->mcast_group.l3.src_ip.addr.ipv4 >> 24) & 0xFF;
    src_ip_hash_entry[5] = (key->mcast_group.l3.src_ip.addr.ipv4 >> 16) & 0xFF;
    src_ip_hash_entry[6] = (key->mcast_group.l3.src_ip.addr.ipv4 >>  8) & 0xFF;
    src_ip_hash_entry[7] = (key->mcast_group.l3.src_ip.addr.ipv4 >>  0) & 0xFF;

    rdd_error = rdd_find_hash_entry_64_bit(&g_hash_table_cfg[RDD_IPTV_SRC_IP_TABLE], src_ip_hash_entry,
        IPTV_L3_SSM_SRC_IP_ENTRY_KEY_MASK_HIGH, IPTV_L3_SSM_SRC_IP_ENTRY_KEY_MASK_LOW, 0, &src_ip_index);

    if (rdd_error)
        return rdd_error;

    iptv_table_ptr = (RDD_IPTV_DDR_LOOKUP_TABLE_DTS *)IPTV_TABLE_PTR;
    iptv_entry_ptr = &(iptv_table_ptr->entry[*index]);

    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_TABLE_READ(context_table_index, iptv_entry_ptr);

    iptv_ssm_context_table_ptr = (RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS *)IPTV_SSM_CONTEXT_TABLE_PTR;
    iptv_context_entry_ptr = &(iptv_ssm_context_table_ptr->entry[context_table_index * RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS + src_ip_index]);

    RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_READ(iptv_entry_valid, iptv_context_entry_ptr);

    if (!(iptv_entry_valid))
        return BDMF_ERR_NOENT;

    base_index = *index;
    *index = GET_ENTRY_INDEX_FROM_BASE_AND_PROVIDER_INDICES(base_index, src_ip_index);

    return BDMF_ERR_OK;
}

static int _rdd_search_iptv_entry(rdpa_iptv_channel_key_t *key, uint8_t *entry_bytes,
    rdpa_iptv_lookup_method iptv_classification_mode, bdmf_ip_t *ipv6_dst_ip_ptr, uint32_t *index)
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS *iptv_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS *iptv_entry_ptr;
    bdmf_mac_t iptv_entry_mac_addr;
    bdmf_ip_t iptv_entry_ip_addr;
    uint32_t crc_init_value, crc_result, hash_index, tries, iptv_entry_index, iptv_entry_valid, any, iptv_entry_vid;
    uint8_t dst_ip_addr[4], ipv6_dst_ip_address[4];
    int rdd_error = 0;

    crc_init_value = rdd_crc_init_value_get(RDD_CRC_TYPE_32);

    crc_result = rdd_crc_bit_by_bit(&entry_bytes[4], 12, 0, crc_init_value, RDD_CRC_TYPE_32);

    hash_index = crc_result & (RDD_IPTV_DDR_LOOKUP_TABLE_SIZE / RDD_IPTV_TABLE_SET_SIZE - 1);

    hash_index = hash_index * RDD_IPTV_TABLE_SET_SIZE;

    iptv_table_ptr = (RDD_IPTV_DDR_LOOKUP_TABLE_DTS *)IPTV_TABLE_PTR;

    for (tries = 0; tries < RDD_IPTV_TABLE_SET_SIZE; tries++)
    {
        iptv_entry_index = (hash_index + tries) & (RDD_IPTV_DDR_LOOKUP_TABLE_SIZE - 1);

        iptv_entry_ptr = &(iptv_table_ptr->entry[iptv_entry_index]);

        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_VALID_READ(iptv_entry_valid, iptv_entry_ptr);

        if (iptv_entry_valid)
        {
            if (iptv_classification_mode == iptv_lookup_method_mac)
            {
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR5_READ(iptv_entry_mac_addr.b[5], iptv_entry_ptr);
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR4_READ(iptv_entry_mac_addr.b[4], iptv_entry_ptr);
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR3_READ(iptv_entry_mac_addr.b[3], iptv_entry_ptr);
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR2_READ(iptv_entry_mac_addr.b[2], iptv_entry_ptr);
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR1_READ(iptv_entry_mac_addr.b[1], iptv_entry_ptr);
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR0_READ(iptv_entry_mac_addr.b[0], iptv_entry_ptr);

                if ((iptv_entry_mac_addr.b[5] == key->mcast_group.mac.b[5]) &&
                    (iptv_entry_mac_addr.b[4] == key->mcast_group.mac.b[4]) &&
                    (iptv_entry_mac_addr.b[3] == key->mcast_group.mac.b[3]) &&
                    (iptv_entry_mac_addr.b[2] == key->mcast_group.mac.b[2]) &&
                    (iptv_entry_mac_addr.b[1] == key->mcast_group.mac.b[1]) &&
                    (iptv_entry_mac_addr.b[0] == key->mcast_group.mac.b[0]))
                {
                     break;
                }
            }

            if (iptv_classification_mode == iptv_lookup_method_mac_vid)
            {
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR5_READ(iptv_entry_mac_addr.b[5], iptv_entry_ptr);
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR4_READ(iptv_entry_mac_addr.b[4], iptv_entry_ptr);
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR3_READ(iptv_entry_mac_addr.b[3], iptv_entry_ptr);
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR2_READ(iptv_entry_mac_addr.b[2], iptv_entry_ptr);
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR1_READ(iptv_entry_mac_addr.b[1], iptv_entry_ptr);
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR0_READ(iptv_entry_mac_addr.b[0], iptv_entry_ptr);
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_VID_READ(iptv_entry_vid , iptv_entry_ptr);

                if ((iptv_entry_mac_addr.b[5] == key->mcast_group.mac.b[5]) &&
                    (iptv_entry_mac_addr.b[4] == key->mcast_group.mac.b[4]) &&
                    (iptv_entry_mac_addr.b[3] == key->mcast_group.mac.b[3]) &&
                    (iptv_entry_mac_addr.b[2] == key->mcast_group.mac.b[2]) &&
                    (iptv_entry_mac_addr.b[1] == key->mcast_group.mac.b[1]) &&
                    (iptv_entry_mac_addr.b[0] == key->mcast_group.mac.b[0]) &&
                    (iptv_entry_vid == key->vid))
                {
                     break;
                }
            }

            if (iptv_classification_mode == iptv_lookup_method_group_ip)
            {
                if (ipv6_dst_ip_ptr != NULL) 
                {
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP12_READ(ipv6_dst_ip_address[0], iptv_entry_ptr);
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP13_READ(ipv6_dst_ip_address[1], iptv_entry_ptr);
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP14_READ(ipv6_dst_ip_address[2], iptv_entry_ptr);
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP15_READ(ipv6_dst_ip_address[3], iptv_entry_ptr);
                }

                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_READ(dst_ip_addr[0], iptv_entry_ptr);
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_READ(dst_ip_addr[1], iptv_entry_ptr);
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_READ(dst_ip_addr[2], iptv_entry_ptr);
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_READ(dst_ip_addr[3], iptv_entry_ptr);

                iptv_entry_ip_addr.addr.ipv4  = (dst_ip_addr[0] << 24);
                iptv_entry_ip_addr.addr.ipv4 |= (dst_ip_addr[1] << 16);
                iptv_entry_ip_addr.addr.ipv4 |= (dst_ip_addr[2] <<  8);
                iptv_entry_ip_addr.addr.ipv4 |= (dst_ip_addr[3] <<  0);

                if (iptv_entry_ip_addr.addr.ipv4 == key->mcast_group.l3.gr_ip.addr.ipv4)
                {
                    if (ipv6_dst_ip_ptr != NULL)
                    {
                        if (ipv6_dst_ip_address[0] != ipv6_dst_ip_ptr->addr.ipv6.data[12] ||
                            ipv6_dst_ip_address[1] != ipv6_dst_ip_ptr->addr.ipv6.data[13] ||
                            ipv6_dst_ip_address[2] != ipv6_dst_ip_ptr->addr.ipv6.data[14] ||
                            ipv6_dst_ip_address[3] != ipv6_dst_ip_ptr->addr.ipv6.data[15])
                        {
                            continue;
                        }
                    }

                    break;
                }
            }

            if (iptv_classification_mode == iptv_lookup_method_group_ip_src_ip)
            {
                if (ipv6_dst_ip_ptr != NULL) 
                {
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP12_READ(ipv6_dst_ip_address[0], iptv_entry_ptr);
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP13_READ(ipv6_dst_ip_address[1], iptv_entry_ptr);
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP14_READ(ipv6_dst_ip_address[2], iptv_entry_ptr);
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP15_READ(ipv6_dst_ip_address[3], iptv_entry_ptr);
                }

                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_READ(dst_ip_addr[0], iptv_entry_ptr);
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_READ(dst_ip_addr[1], iptv_entry_ptr);
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_READ(dst_ip_addr[2], iptv_entry_ptr);
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_READ(dst_ip_addr[3], iptv_entry_ptr);

                iptv_entry_ip_addr.addr.ipv4  = (dst_ip_addr[0] << 24);
                iptv_entry_ip_addr.addr.ipv4 |= (dst_ip_addr[1] << 16);
                iptv_entry_ip_addr.addr.ipv4 |= (dst_ip_addr[2] <<  8);
                iptv_entry_ip_addr.addr.ipv4 |= (dst_ip_addr[3] <<  0);

                if (iptv_entry_ip_addr.addr.ipv4 == key->mcast_group.l3.gr_ip.addr.ipv4)
                {
                    if (ipv6_dst_ip_ptr != NULL)
                    {
                        if (ipv6_dst_ip_address[0] != ipv6_dst_ip_ptr->addr.ipv6.data[12] ||
                            ipv6_dst_ip_address[1] != ipv6_dst_ip_ptr->addr.ipv6.data[13] ||
                            ipv6_dst_ip_address[2] != ipv6_dst_ip_ptr->addr.ipv6.data[14] ||
                            ipv6_dst_ip_address[3] != ipv6_dst_ip_ptr->addr.ipv6.data[15])
                        {
                            continue;
                        }
                    }

                    if (key->mcast_group.l3.src_ip.addr.ipv4 == 0)
                    {
                        RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_ANY_READ(any, iptv_entry_ptr);

                        if (!(any))
                            return BDMF_ERR_NOENT;
                    }

                    break;
                }
            }

            if (iptv_classification_mode == iptv_lookup_method_group_ip_src_ip_vid)
            {
                if (ipv6_dst_ip_ptr != NULL)
                {
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP12_READ(ipv6_dst_ip_address[0], iptv_entry_ptr);
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP13_READ(ipv6_dst_ip_address[1], iptv_entry_ptr);
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP14_READ(ipv6_dst_ip_address[2], iptv_entry_ptr);
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP15_READ(ipv6_dst_ip_address[3], iptv_entry_ptr);
                }

                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_READ(dst_ip_addr[0], iptv_entry_ptr);
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_READ(dst_ip_addr[1], iptv_entry_ptr);
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_READ(dst_ip_addr[2], iptv_entry_ptr);
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_READ(dst_ip_addr[3], iptv_entry_ptr);

                iptv_entry_ip_addr.addr.ipv4  = (dst_ip_addr[0] << 24);
                iptv_entry_ip_addr.addr.ipv4 |= (dst_ip_addr[1] << 16);
                iptv_entry_ip_addr.addr.ipv4 |= (dst_ip_addr[2] <<  8);
                iptv_entry_ip_addr.addr.ipv4 |= (dst_ip_addr[3] <<  0);

                RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_VID_READ(iptv_entry_vid, iptv_entry_ptr);

                if ((iptv_entry_ip_addr.addr.ipv4 == key->mcast_group.l3.gr_ip.addr.ipv4) && (iptv_entry_vid == key->vid))
                {
                    if (ipv6_dst_ip_ptr != NULL)
                    {
                        if (ipv6_dst_ip_address[0] != ipv6_dst_ip_ptr->addr.ipv6.data[12] ||
                            ipv6_dst_ip_address[1] != ipv6_dst_ip_ptr->addr.ipv6.data[13] ||
                            ipv6_dst_ip_address[2] != ipv6_dst_ip_ptr->addr.ipv6.data[14] ||
                            ipv6_dst_ip_address[3] != ipv6_dst_ip_ptr->addr.ipv6.data[15])
                        {
                            continue;
                        }
                    }

                    if (key->mcast_group.l3.src_ip.addr.ipv4 == 0)
                    {
                        RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_ANY_READ(any, iptv_entry_ptr);

                        if (!(any))
                            return BDMF_ERR_NOENT;
                    }

                    break;
                }
            }
        }
    }

    if (tries == RDD_CONNECTION_TABLE_SET_SIZE)
        return BDMF_ERR_NOENT;

    *index = iptv_entry_index;

    return rdd_error;
}

static int _rdd_get_iptv_l2_mac_entry_index(rdpa_iptv_channel_key_t *key, uint32_t *index)
{
    int rdd_error;
    uint8_t entry_bytes[RDD_IPTV_ENTRY_SIZE];

    MEMSET(entry_bytes, 0, RDD_IPTV_ENTRY_SIZE);

    entry_bytes[10] = key->mcast_group.mac.b[0];
    entry_bytes[11] = key->mcast_group.mac.b[1];
    entry_bytes[12] = key->mcast_group.mac.b[2];
    entry_bytes[13] = key->mcast_group.mac.b[3];
    entry_bytes[14] = key->mcast_group.mac.b[4];
    entry_bytes[15] = key->mcast_group.mac.b[5];

    rdd_error = _rdd_search_iptv_entry(key, entry_bytes, iptv_lookup_method_mac, 0, index);

    return rdd_error;
}

static int _rdd_get_iptv_l2_mac_vid_entry_index(rdpa_iptv_channel_key_t *key, uint32_t *index)
{
    int rdd_error;
    uint8_t entry_bytes[RDD_IPTV_ENTRY_SIZE];

    MEMSET(entry_bytes, 0, RDD_IPTV_ENTRY_SIZE);

    entry_bytes[8] = key->vid >> 8;
    entry_bytes[9] = key->vid & 0xFF;
    entry_bytes[10] = key->mcast_group.mac.b[0];
    entry_bytes[11] = key->mcast_group.mac.b[1];
    entry_bytes[12] = key->mcast_group.mac.b[2];
    entry_bytes[13] = key->mcast_group.mac.b[3];
    entry_bytes[14] = key->mcast_group.mac.b[4];
    entry_bytes[15] = key->mcast_group.mac.b[5];

    rdd_error = _rdd_search_iptv_entry(key, entry_bytes, iptv_lookup_method_mac_vid, 0, index);

    return rdd_error;
}

static int _rdd_get_iptv_l3_dst_ip_src_ip_entry_index(rdpa_iptv_channel_key_t *key, rdpa_iptv_lookup_method iptv_mode,
    bdmf_ip_t *ipv6_dst_ip_ptr, bdmf_ip_t *ipv6_src_ip_ptr, uint32_t *index)
{
    int rdd_error;
    uint8_t entry_bytes[RDD_IPTV_ENTRY_SIZE];

    MEMSET(entry_bytes, 0, RDD_IPTV_ENTRY_SIZE);

    if (ipv6_dst_ip_ptr != NULL)
    {
        entry_bytes[4] = ipv6_dst_ip_ptr->addr.ipv6.data[12];
        entry_bytes[5] = ipv6_dst_ip_ptr->addr.ipv6.data[13];
        entry_bytes[6] = ipv6_dst_ip_ptr->addr.ipv6.data[14];
        entry_bytes[7] = ipv6_dst_ip_ptr->addr.ipv6.data[15];
    }

    if (iptv_mode == iptv_lookup_method_group_ip_src_ip_vid)
    {
        entry_bytes[10] = key->vid >> 8;
        entry_bytes[11] = key->vid & 0xFF;
    }

    entry_bytes[12] = (key->mcast_group.l3.gr_ip.addr.ipv4 >> 24) & 0xFF;
    entry_bytes[13] = (key->mcast_group.l3.gr_ip.addr.ipv4 >> 16) & 0xFF;
    entry_bytes[14] = (key->mcast_group.l3.gr_ip.addr.ipv4 >>  8) & 0xFF;
    entry_bytes[15] = (key->mcast_group.l3.gr_ip.addr.ipv4 >>  0) & 0xFF;

    rdd_error = _rdd_search_iptv_entry(key, entry_bytes, iptv_mode, ipv6_dst_ip_ptr, index);

    if ((key->mcast_group.l3.src_ip.addr.ipv4 != 0) && (!rdd_error))
        rdd_error = _rdd_search_iptv_ssm_entry(key, ipv6_src_ip_ptr, index);

    return rdd_error;
}

static int _rdd_get_iptv_l3_dst_ip_entry_index(rdpa_iptv_channel_key_t *key, bdmf_ip_t *ipv6_dst_ip_ptr,
    bdmf_ip_t *ipv6_src_ip_ptr, uint32_t *index)
{
    int rdd_error;
    uint8_t entry_bytes[RDD_IPTV_ENTRY_SIZE];

    MEMSET(entry_bytes, 0, RDD_IPTV_ENTRY_SIZE);

    if (ipv6_dst_ip_ptr != NULL)
    {
        entry_bytes[4] = ipv6_dst_ip_ptr->addr.ipv6.data[12];
        entry_bytes[5] = ipv6_dst_ip_ptr->addr.ipv6.data[13];
        entry_bytes[6] = ipv6_dst_ip_ptr->addr.ipv6.data[14];
        entry_bytes[7] = ipv6_dst_ip_ptr->addr.ipv6.data[15];
    }

    entry_bytes[12] = (key->mcast_group.l3.gr_ip.addr.ipv4 >> 24) & 0xFF;
    entry_bytes[13] = (key->mcast_group.l3.gr_ip.addr.ipv4 >> 16) & 0xFF;
    entry_bytes[14] = (key->mcast_group.l3.gr_ip.addr.ipv4 >>  8) & 0xFF;
    entry_bytes[15] = (key->mcast_group.l3.gr_ip.addr.ipv4 >>  0) & 0xFF;

    rdd_error = _rdd_search_iptv_entry(key, entry_bytes, iptv_lookup_method_group_ip, ipv6_dst_ip_ptr, index);

    return rdd_error;
}

int rdd_iptv_entry_search(rdpa_iptv_channel_key_t *key, uint32_t *index)
{
    rdpa_iptv_lookup_method iptv_classification_mode;
    RDD_SYSTEM_CONFIGURATION_DTS *system_cfg;
    bdmf_ip_t ipv6_dst_ip_copy, ipv6_src_ip_copy, ipv6_src_ip_any;
#if !defined(FIRMWARE_INIT)
    uint32_t *ipv6_buffer_ptr;
#else
    uint32_t crc_init_value, crc_result;
#endif
    int rdd_error = 0;

    system_cfg = (RDD_SYSTEM_CONFIGURATION_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_SYSTEM_CONFIGURATION_ADDRESS);

    RDD_SYSTEM_CONFIGURATION_IPTV_CLASSIFICATION_METHOD_READ(iptv_classification_mode, system_cfg);

#if !defined(FIRMWARE_INIT)

    if (key->mcast_group.l3.gr_ip.family == bdmf_ip_family_ipv6)
    {
        ipv6_buffer_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + IPV6_CRC_BUFFER_ADDRESS);

        bdmf_fastlock_lock(&cpu_message_lock);

        MWRITE_BLK_8(ipv6_buffer_ptr, key->mcast_group.l3.gr_ip.addr.ipv6.data, 16);

        rdd_error = _rdd_cpu_message_send(RDD_CPU_MESSAGE_IPV6_CRC_GET, RDD_CLUSTER_0, 0, 0, 0, 1);

        if (rdd_error)
        {
            bdmf_fastlock_unlock(&cpu_message_lock);
            return rdd_error;
        }

        memcpy(ipv6_dst_ip_copy.addr.ipv6.data, key->mcast_group.l3.gr_ip.addr.ipv6.data, 16);

        key->mcast_group.l3.gr_ip.addr.ipv4 = swap4bytes(*(volatile uint32_t *)ipv6_buffer_ptr);

        if ((iptv_classification_mode == iptv_lookup_method_group_ip_src_ip) || (iptv_classification_mode == iptv_lookup_method_group_ip_src_ip_vid))
        {
            MEMSET(ipv6_src_ip_any.addr.ipv6.data, 0, 16);

            if (memcmp(ipv6_src_ip_any.addr.ipv6.data, key->mcast_group.l3.src_ip.addr.ipv6.data, 16))
            {
                MWRITE_BLK_8(ipv6_buffer_ptr, key->mcast_group.l3.src_ip.addr.ipv6.data, 16);

                rdd_error = _rdd_cpu_message_send(RDD_CPU_MESSAGE_IPV6_CRC_GET, RDD_CLUSTER_0, 0, 0, 0, 1);

                if (rdd_error)
                {
                    bdmf_fastlock_unlock(&cpu_message_lock);
                    return rdd_error;
                }

                memcpy(ipv6_src_ip_copy.addr.ipv6.data, key->mcast_group.l3.src_ip.addr.ipv6.data, 16);

                key->mcast_group.l3.src_ip.addr.ipv4 = swap4bytes(*(volatile uint32_t *)ipv6_buffer_ptr);
            }
            else
            {
                MEMSET(ipv6_src_ip_copy.addr.ipv6.data, 0, 16);
                key->mcast_group.l3.src_ip.addr.ipv4 = 0;
            }
        }

        bdmf_fastlock_unlock(&cpu_message_lock);
    }
#else

    if (key->mcast_group.l3.gr_ip.family == bdmf_ip_family_ipv6)
    {
        memcpy(ipv6_dst_ip_copy.addr.ipv6.data, key->mcast_group.l3.gr_ip.addr.ipv6.data, 16);

        crc_init_value = rdd_crc_init_value_get(RDD_CRC_TYPE_32);
        crc_result = rdd_crc_bit_by_bit(&(key->mcast_group.l3.gr_ip.addr.ipv6.data[0]), 16, 0, crc_init_value, RDD_CRC_TYPE_32);
        key->mcast_group.l3.gr_ip.addr.ipv4 = crc_result;

        if ((iptv_classification_mode == iptv_lookup_method_group_ip_src_ip) || (iptv_classification_mode == iptv_lookup_method_group_ip_src_ip_vid))
        {
            MEMSET(ipv6_src_ip_any.addr.ipv6.data, 0, 16);

            if (memcmp(ipv6_src_ip_any.addr.ipv6.data, key->mcast_group.l3.src_ip.addr.ipv6.data, 16))
            {
                memcpy(ipv6_src_ip_copy.addr.ipv6.data, key->mcast_group.l3.src_ip.addr.ipv6.data, 16);

                crc_init_value = rdd_crc_init_value_get(RDD_CRC_TYPE_32);
                crc_result = rdd_crc_bit_by_bit(&(key->mcast_group.l3.src_ip.addr.ipv6.data[0]), 16, 0, crc_init_value, RDD_CRC_TYPE_32);
                key->mcast_group.l3.src_ip.addr.ipv4 = crc_result;
            }
            else
            {
                MEMSET(ipv6_src_ip_copy.addr.ipv6.data, 0, 16);
                key->mcast_group.l3.src_ip.addr.ipv4 = 0;
            }
        }
    }
#endif
    bdmf_fastlock_lock(&int_lock);

    switch (iptv_classification_mode)
    {
    case iptv_lookup_method_mac:

         rdd_error = _rdd_get_iptv_l2_mac_entry_index(key, index);
         break;

    case iptv_lookup_method_mac_vid:

         rdd_error = _rdd_get_iptv_l2_mac_vid_entry_index(key, index);
         break;

    case iptv_lookup_method_group_ip_src_ip:

        if (key->mcast_group.l3.gr_ip.family == bdmf_ip_family_ipv6)
            rdd_error = _rdd_get_iptv_l3_dst_ip_src_ip_entry_index(key, iptv_classification_mode, &ipv6_dst_ip_copy, &ipv6_src_ip_copy, index);
        else
            rdd_error = _rdd_get_iptv_l3_dst_ip_src_ip_entry_index(key, iptv_classification_mode, 0, 0, index);

        break;

    case iptv_lookup_method_group_ip:

        if (key->mcast_group.l3.gr_ip.family == bdmf_ip_family_ipv6)
            rdd_error = _rdd_get_iptv_l3_dst_ip_entry_index(key, &ipv6_dst_ip_copy, &ipv6_src_ip_copy, index);
        else
            rdd_error = _rdd_get_iptv_l3_dst_ip_entry_index(key, 0, 0, index);

        break;

    case iptv_lookup_method_group_ip_src_ip_vid:

        if (key->mcast_group.l3.gr_ip.family == bdmf_ip_family_ipv6)
            rdd_error = _rdd_get_iptv_l3_dst_ip_src_ip_entry_index(key, iptv_classification_mode, &ipv6_dst_ip_copy, &ipv6_src_ip_copy, index);
        else
            rdd_error = _rdd_get_iptv_l3_dst_ip_src_ip_entry_index(key, iptv_classification_mode, 0, 0, index);

        break;
    }

    bdmf_fastlock_unlock(&int_lock);
    return rdd_error;
}

static int _rdd_read_iptv_ssm_entry(uint32_t entry_index, rdd_iptv_entry_t *entry,
    rdpa_iptv_lookup_method iptv_mode, bdmf_ip_t *ipv6_dst_ip_ptr)
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS *iptv_table_ptr;
    RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS *iptv_ssm_context_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *iptv_context_entry_ptr;
    RDD_IPTV_L3_SRC_IP_LOOKUP_TABLE_DTS *iptv_layer3_src_ip_table_ptr;
    RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *iptv_layer3_src_ip_entry_ptr;
    uint32_t iptv_entry_valid, provider_index, context_table_index, base_index;
    uint8_t ip_addr[4];

    provider_index = GET_PROVIDER_INDEX_FROM_ENTRY_INDEX(entry_index);

    base_index = GET_BASE_INDEX_FROM_ENTRY_INDEX(entry_index);

    iptv_table_ptr = (RDD_IPTV_DDR_LOOKUP_TABLE_DTS *)IPTV_TABLE_PTR;
    iptv_entry_ptr = &(iptv_table_ptr->entry[base_index]);

    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_VALID_READ(iptv_entry_valid, iptv_entry_ptr);

    if (!(iptv_entry_valid))
        return BDMF_ERR_NOENT;

    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP12_READ(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[12], iptv_entry_ptr);
    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP13_READ(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[13], iptv_entry_ptr);
    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP14_READ(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[14], iptv_entry_ptr);
    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP15_READ(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[15], iptv_entry_ptr);

    if (*((uint32_t *)&(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[12])) != 0)
    {
        entry->key.mcast_group.l3.gr_ip.family = bdmf_ip_family_ipv6;
        entry->key.mcast_group.l3.src_ip.family = bdmf_ip_family_ipv6;
    }
    else
    {
        entry->key.mcast_group.l3.gr_ip.family = bdmf_ip_family_ipv4;
        entry->key.mcast_group.l3.src_ip.family = bdmf_ip_family_ipv4;
    }

    if (entry->key.mcast_group.l3.gr_ip.family == bdmf_ip_family_ipv6)
    {
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_READ(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[3], iptv_entry_ptr);
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_READ(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[2], iptv_entry_ptr);
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_READ(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[1], iptv_entry_ptr);
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_READ(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[0], iptv_entry_ptr);
    }
    else
    {
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_READ(ip_addr[0], iptv_entry_ptr);
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_READ(ip_addr[1], iptv_entry_ptr);
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_READ(ip_addr[2], iptv_entry_ptr);
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_READ(ip_addr[3], iptv_entry_ptr);

        entry->key.mcast_group.l3.gr_ip.addr.ipv4  = (ip_addr[0] << 24);
        entry->key.mcast_group.l3.gr_ip.addr.ipv4 |= (ip_addr[1] << 16);
        entry->key.mcast_group.l3.gr_ip.addr.ipv4 |= (ip_addr[2] <<  8);
        entry->key.mcast_group.l3.gr_ip.addr.ipv4 |= (ip_addr[3] <<  0);
    }

    if (iptv_mode == iptv_lookup_method_group_ip_src_ip_vid)
        RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_VID_READ(entry->key.vid, iptv_entry_ptr);

    iptv_layer3_src_ip_table_ptr = (RDD_IPTV_L3_SRC_IP_LOOKUP_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + IPTV_L3_SRC_IP_LOOKUP_TABLE_ADDRESS);

    iptv_layer3_src_ip_entry_ptr = &(iptv_layer3_src_ip_table_ptr->entry[provider_index]);

    if (entry->key.mcast_group.l3.src_ip.family == bdmf_ip_family_ipv6) 
    {
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_12_READ(entry->key.mcast_group.l3.src_ip.addr.ipv6.data[12], (RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *)iptv_layer3_src_ip_entry_ptr);
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_13_READ(entry->key.mcast_group.l3.src_ip.addr.ipv6.data[13], (RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *)iptv_layer3_src_ip_entry_ptr);
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_14_READ(entry->key.mcast_group.l3.src_ip.addr.ipv6.data[14], (RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *)iptv_layer3_src_ip_entry_ptr);
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_15_READ(entry->key.mcast_group.l3.src_ip.addr.ipv6.data[15], (RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *)iptv_layer3_src_ip_entry_ptr);

        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_0_READ(entry->key.mcast_group.l3.src_ip.addr.ipv6.data[3], (RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *)iptv_layer3_src_ip_entry_ptr);
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_1_READ(entry->key.mcast_group.l3.src_ip.addr.ipv6.data[2], (RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *)iptv_layer3_src_ip_entry_ptr);
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_2_READ(entry->key.mcast_group.l3.src_ip.addr.ipv6.data[1], (RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *)iptv_layer3_src_ip_entry_ptr);
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_3_READ(entry->key.mcast_group.l3.src_ip.addr.ipv6.data[0], (RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *)iptv_layer3_src_ip_entry_ptr);
    }
    else
    {
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_0_READ(ip_addr[0], (RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *)iptv_layer3_src_ip_entry_ptr);
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_1_READ(ip_addr[1], (RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *)iptv_layer3_src_ip_entry_ptr);
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_2_READ(ip_addr[2], (RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *)iptv_layer3_src_ip_entry_ptr);
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_3_READ(ip_addr[3], (RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS *)iptv_layer3_src_ip_entry_ptr);

        entry->key.mcast_group.l3.src_ip.addr.ipv4  = (ip_addr[0] << 24);
        entry->key.mcast_group.l3.src_ip.addr.ipv4 |= (ip_addr[1] << 16);
        entry->key.mcast_group.l3.src_ip.addr.ipv4 |= (ip_addr[2] <<  8);
        entry->key.mcast_group.l3.src_ip.addr.ipv4 |= (ip_addr[3] <<  0);
    }

    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_TABLE_READ(context_table_index, iptv_entry_ptr);

    iptv_ssm_context_table_ptr = (RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS *)IPTV_SSM_CONTEXT_TABLE_PTR;
    iptv_context_entry_ptr = &(iptv_ssm_context_table_ptr->entry[context_table_index * RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS + provider_index]);

    RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_READ(iptv_entry_valid, iptv_context_entry_ptr);

    if (!(iptv_entry_valid))
        return BDMF_ERR_NOENT;

    RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_READ(entry->egress_port_vector, iptv_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_WLAN_MCAST_INDEX_READ(entry->wlan_mcast_index, iptv_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_READ(entry->ic_context, iptv_context_entry_ptr);

    return BDMF_ERR_OK;
}

static int _rdd_read_iptv_entry(uint32_t entry_index, rdpa_iptv_lookup_method iptv_mode,
    rdd_iptv_entry_t *entry, bdmf_ip_t *ipv6_dst_ip_ptr)
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS *iptv_table_ptr;
    RDD_IPTV_DDR_CONTEXT_TABLE_DTS *iptv_context_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *iptv_context_entry_ptr;
    uint32_t iptv_entry_valid, any;
    uint8_t dst_ip_addr[4];

    iptv_table_ptr = (RDD_IPTV_DDR_LOOKUP_TABLE_DTS *)IPTV_TABLE_PTR;
    iptv_entry_ptr = &(iptv_table_ptr->entry[entry_index]);

    RDD_IPTV_L2_DDR_LOOKUP_ENTRY_VALID_READ(iptv_entry_valid, iptv_entry_ptr);

    if (!(iptv_entry_valid))
        return BDMF_ERR_NOENT;

    if ((iptv_mode == iptv_lookup_method_group_ip_src_ip) || (iptv_mode == iptv_lookup_method_group_ip_src_ip_vid))
    {
        RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_ANY_READ(any, iptv_entry_ptr);

        if (!(any))
            return BDMF_ERR_NOENT;
    }

    if ((iptv_mode == iptv_lookup_method_group_ip_src_ip) || (iptv_mode == iptv_lookup_method_group_ip))
    {
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP12_READ(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[12], iptv_entry_ptr);
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP13_READ(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[13], iptv_entry_ptr);
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP14_READ(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[14], iptv_entry_ptr);
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP15_READ(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[15], iptv_entry_ptr);

        if (*((uint32_t *)&(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[12])) != 0)
        {
            entry->key.mcast_group.l3.gr_ip.family = bdmf_ip_family_ipv6;
            entry->key.mcast_group.l3.src_ip.family = bdmf_ip_family_ipv6;
        }
        else
        {
            entry->key.mcast_group.l3.gr_ip.family = bdmf_ip_family_ipv4;
            entry->key.mcast_group.l3.src_ip.family = bdmf_ip_family_ipv4;
        }

        if (entry->key.mcast_group.l3.gr_ip.family == bdmf_ip_family_ipv6)
        {
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_READ(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[3], iptv_entry_ptr);
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_READ(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[2], iptv_entry_ptr);
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_READ(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[1], iptv_entry_ptr);
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_READ(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[0], iptv_entry_ptr);
        }
        else
        {
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_READ(dst_ip_addr[0], iptv_entry_ptr);
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_READ(dst_ip_addr[1], iptv_entry_ptr);
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_READ(dst_ip_addr[2], iptv_entry_ptr);
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_READ(dst_ip_addr[3], iptv_entry_ptr);

            entry->key.mcast_group.l3.gr_ip.addr.ipv4  = (dst_ip_addr[0] << 24);
            entry->key.mcast_group.l3.gr_ip.addr.ipv4 |= (dst_ip_addr[1] << 16);
            entry->key.mcast_group.l3.gr_ip.addr.ipv4 |= (dst_ip_addr[2] <<  8);
            entry->key.mcast_group.l3.gr_ip.addr.ipv4 |= (dst_ip_addr[3] <<  0);
        }
    }
    else if (iptv_mode == iptv_lookup_method_group_ip_src_ip_vid)
    {
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP12_READ(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[12], iptv_entry_ptr);
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP13_READ(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[13], iptv_entry_ptr);
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP14_READ(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[14], iptv_entry_ptr);
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP15_READ(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[15], iptv_entry_ptr);

        if (*((uint32_t *)&(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[12])) != 0)
        {
            entry->key.mcast_group.l3.gr_ip.family = bdmf_ip_family_ipv6;
            entry->key.mcast_group.l3.src_ip.family = bdmf_ip_family_ipv6;
        }
        else
        {
            entry->key.mcast_group.l3.gr_ip.family = bdmf_ip_family_ipv4;
            entry->key.mcast_group.l3.src_ip.family = bdmf_ip_family_ipv4;
        }

        if (entry->key.mcast_group.l3.gr_ip.family == bdmf_ip_family_ipv6)
        {
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_READ(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[3], iptv_entry_ptr);
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_READ(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[2], iptv_entry_ptr);
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_READ(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[1], iptv_entry_ptr);
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_READ(entry->key.mcast_group.l3.gr_ip.addr.ipv6.data[0], iptv_entry_ptr);
        }
        else
        {
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_READ(dst_ip_addr[0], iptv_entry_ptr);
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_READ(dst_ip_addr[1], iptv_entry_ptr);
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_READ(dst_ip_addr[2], iptv_entry_ptr);
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_READ(dst_ip_addr[3], iptv_entry_ptr);

            entry->key.mcast_group.l3.gr_ip.addr.ipv4  = (dst_ip_addr[0] << 24);
            entry->key.mcast_group.l3.gr_ip.addr.ipv4 |= (dst_ip_addr[1] << 16);
            entry->key.mcast_group.l3.gr_ip.addr.ipv4 |= (dst_ip_addr[2] <<  8);
            entry->key.mcast_group.l3.gr_ip.addr.ipv4 |= (dst_ip_addr[3] <<  0);
        }

        RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_VID_READ(entry->key.vid, iptv_entry_ptr);
    }
    else
    {
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR5_READ(entry->key.mcast_group.mac.b[5], iptv_entry_ptr);
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR4_READ(entry->key.mcast_group.mac.b[4], iptv_entry_ptr);
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR3_READ(entry->key.mcast_group.mac.b[3], iptv_entry_ptr);
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR2_READ(entry->key.mcast_group.mac.b[2], iptv_entry_ptr);
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR1_READ(entry->key.mcast_group.mac.b[1], iptv_entry_ptr);
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR0_READ(entry->key.mcast_group.mac.b[0], iptv_entry_ptr);
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_VID_READ(entry->key.vid, iptv_entry_ptr);
    }

    if ((iptv_mode == iptv_lookup_method_group_ip) ||
        ((iptv_mode == iptv_lookup_method_group_ip_src_ip) && (entry_index < RDD_IPTV_DDR_LOOKUP_TABLE_SIZE)) ||
        ((iptv_mode == iptv_lookup_method_group_ip_src_ip_vid) && (entry_index < RDD_IPTV_DDR_LOOKUP_TABLE_SIZE)))
    {
        if (entry->key.mcast_group.l3.gr_ip.family == bdmf_ip_family_ipv6) 
            MEMSET(entry->key.mcast_group.l3.src_ip.addr.ipv6.data, 0, 16);
        else
            entry->key.mcast_group.l3.src_ip.addr.ipv4 = 0;
    }

    iptv_context_table_ptr = (RDD_IPTV_DDR_CONTEXT_TABLE_DTS *)IPTV_CONTEXT_TABLE_PTR;
    iptv_context_entry_ptr = &(iptv_context_table_ptr->entry[entry_index]);

    RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_READ(entry->egress_port_vector, iptv_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_WLAN_MCAST_INDEX_READ(entry->wlan_mcast_index, iptv_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_READ(entry->ic_context, iptv_context_entry_ptr);

    return BDMF_ERR_OK;
}

static int _rdd_get_iptv_l2_mac_entry(uint32_t entry_index, rdd_iptv_entry_t *entry)
{
    return _rdd_read_iptv_entry(entry_index, iptv_lookup_method_mac, entry, 0);
}

static int _rdd_get_iptv_l2_mac_vid_entry(uint32_t entry_index, rdd_iptv_entry_t *entry)
{
    return _rdd_read_iptv_entry(entry_index, iptv_lookup_method_mac_vid, entry, 0);
}

static int _rdd_get_iptv_ssm_entry(uint32_t entry_index, rdd_iptv_entry_t *entry,
    rdpa_iptv_lookup_method iptv_mode, bdmf_ip_t *ipv6_dst_ip)
{
    int rdd_error;

    if (entry_index < RDD_IPTV_DDR_LOOKUP_TABLE_SIZE)
        rdd_error = _rdd_read_iptv_entry(entry_index, iptv_mode, entry, ipv6_dst_ip);
    else
        rdd_error = _rdd_read_iptv_ssm_entry(entry_index, entry, iptv_mode, ipv6_dst_ip);

    return rdd_error;
}

static inline int _rdd_get_iptv_l3_dst_ip_src_ip_entry(uint32_t entry_index, rdd_iptv_entry_t *entry,
    bdmf_ip_t *ipv6_dst_ip)
{
    return _rdd_get_iptv_ssm_entry(entry_index, entry, iptv_lookup_method_group_ip_src_ip, ipv6_dst_ip);
}

static int _rdd_get_iptv_l3_dst_ip_entry(uint32_t entry_index, rdd_iptv_entry_t *entry, bdmf_ip_t *ipv6_dst_ip)
{
    return _rdd_read_iptv_entry(entry_index, iptv_lookup_method_group_ip, entry, ipv6_dst_ip);
}

static inline int _rdd_get_iptv_l3_dst_ip_src_ip_vid_entry(uint32_t entry_index, rdd_iptv_entry_t *entry,
    bdmf_ip_t *ipv6_dst_ip)
{
    return _rdd_get_iptv_ssm_entry(entry_index, entry, iptv_lookup_method_group_ip_src_ip_vid, ipv6_dst_ip);
}

int rdd_iptv_entry_get(uint32_t entry_index, rdd_iptv_entry_t *entry)
{
    rdpa_iptv_lookup_method iptv_classification_mode;
    RDD_SYSTEM_CONFIGURATION_DTS *system_cfg;
    bdmf_ip_t ipv6_dst_ip_ptr;
    int rdd_error;

    rdd_error = BDMF_ERR_OK;

    system_cfg = (RDD_SYSTEM_CONFIGURATION_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_SYSTEM_CONFIGURATION_ADDRESS);

    RDD_SYSTEM_CONFIGURATION_IPTV_CLASSIFICATION_METHOD_READ(iptv_classification_mode, system_cfg);

    bdmf_fastlock_lock(&int_lock);

    switch (iptv_classification_mode)
    {
    case iptv_lookup_method_mac:

         rdd_error = _rdd_get_iptv_l2_mac_entry(entry_index, entry);
         break;

    case iptv_lookup_method_mac_vid:

         rdd_error = _rdd_get_iptv_l2_mac_vid_entry(entry_index, entry);
         break;

    case iptv_lookup_method_group_ip_src_ip:

         rdd_error = _rdd_get_iptv_l3_dst_ip_src_ip_entry(entry_index, entry, &ipv6_dst_ip_ptr);
         break;

    case iptv_lookup_method_group_ip:

        rdd_error = _rdd_get_iptv_l3_dst_ip_entry(entry_index, entry, &ipv6_dst_ip_ptr);
        break;

    case iptv_lookup_method_group_ip_src_ip_vid:

        rdd_error = _rdd_get_iptv_l3_dst_ip_src_ip_vid_entry(entry_index, entry, &ipv6_dst_ip_ptr);
        break;
    }

    bdmf_fastlock_unlock(&int_lock);
    return rdd_error;
}

static int _rdd_read_iptv_ssm_entry_counter(uint32_t entry_index, uint16_t *counter_value)
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS *iptv_table_ptr;
    RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS *iptv_ssm_context_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *iptv_context_entry_ptr;
    uint32_t iptv_entry_valid, provider_index, context_table_index, base_index;

    provider_index = GET_PROVIDER_INDEX_FROM_ENTRY_INDEX(entry_index);

    base_index =  GET_BASE_INDEX_FROM_ENTRY_INDEX(entry_index);

    iptv_table_ptr = (RDD_IPTV_DDR_LOOKUP_TABLE_DTS *)IPTV_TABLE_PTR;
    iptv_entry_ptr = &(iptv_table_ptr->entry[base_index]);

    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_VALID_READ(iptv_entry_valid, iptv_entry_ptr);

    if (!(iptv_entry_valid))
        return BDMF_ERR_NOENT;

    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_TABLE_READ(context_table_index, iptv_entry_ptr);

    iptv_ssm_context_table_ptr = (RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS *)IPTV_SSM_CONTEXT_TABLE_PTR;
    iptv_context_entry_ptr = &(iptv_ssm_context_table_ptr->entry[context_table_index * RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS + provider_index]);

    RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_READ(iptv_entry_valid, iptv_context_entry_ptr);

    if (!(iptv_entry_valid))
        return BDMF_ERR_NOENT;

    RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_READ(*counter_value, iptv_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_WRITE(0, iptv_context_entry_ptr);

    return BDMF_ERR_OK;
}

static int _rdd_read_iptv_ssm_any_entry_counter(uint32_t entry_index, uint16_t *counter_value)
{
    RDD_IPTV_DDR_CONTEXT_TABLE_DTS *iptv_context_table_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *iptv_context_entry_ptr;
    uint32_t iptv_context_entry_cache, cache_index;
    int rdd_error;

    iptv_context_table_ptr = (RDD_IPTV_DDR_CONTEXT_TABLE_DTS *)IPTV_CONTEXT_TABLE_PTR;
    iptv_context_entry_ptr = &(iptv_context_table_ptr->entry[entry_index]);

    RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_READ(iptv_context_entry_cache, iptv_context_entry_ptr);

    if (iptv_context_entry_cache)
    {
        RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_READ(cache_index, iptv_context_entry_ptr);

        rdd_error = _rdd_read_iptv_counter_cache(cache_index, counter_value);

        return rdd_error;
    }

    RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_READ(*counter_value, iptv_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_WRITE(0, iptv_context_entry_ptr);

    return BDMF_ERR_OK;
}

static int _rdd_read_iptv_entry_counter(uint32_t entry_index, uint16_t *counter_value)
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS *iptv_table_ptr;
    RDD_IPTV_DDR_CONTEXT_TABLE_DTS *iptv_context_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS *iptv_context_entry_ptr;
    uint32_t iptv_entry_valid, iptv_context_entry_cache, cache_index;
    int rdd_error;

    iptv_context_table_ptr = (RDD_IPTV_DDR_CONTEXT_TABLE_DTS *)IPTV_CONTEXT_TABLE_PTR;
    iptv_context_entry_ptr = &(iptv_context_table_ptr->entry[entry_index]);

    RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_READ(iptv_context_entry_cache, iptv_context_entry_ptr);

    if (iptv_context_entry_cache)
    {
        RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_READ(cache_index, iptv_context_entry_ptr);

        rdd_error = _rdd_read_iptv_counter_cache(cache_index, counter_value);

        return rdd_error;
    }

    iptv_table_ptr = (RDD_IPTV_DDR_LOOKUP_TABLE_DTS *)IPTV_TABLE_PTR;
    iptv_entry_ptr = &(iptv_table_ptr->entry[entry_index]);

    RDD_IPTV_L2_DDR_LOOKUP_ENTRY_VALID_READ(iptv_entry_valid, iptv_entry_ptr);

    if (!(iptv_entry_valid))
        return BDMF_ERR_NOENT;

    RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_READ(*counter_value, iptv_context_entry_ptr);
    RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_WRITE(0, iptv_context_entry_ptr);

    return BDMF_ERR_OK;
}

static inline int _rdd_read_iptv_l2_mac_counter(uint32_t entry_index, uint16_t *counter_value)
{
    return _rdd_read_iptv_entry_counter(entry_index, counter_value);
}

static inline int _rdd_read_iptv_l2_mac_vid_counter(uint32_t entry_index, uint16_t *counter_value)
{
    return _rdd_read_iptv_entry_counter(entry_index, counter_value);
}

static int _rdd_read_iptv_l3_dst_ip_src_ip_counter(uint32_t entry_index, uint16_t *counter_value)
{
    int rdd_error;

    if (entry_index < RDD_IPTV_DDR_LOOKUP_TABLE_SIZE)
        rdd_error = _rdd_read_iptv_ssm_any_entry_counter(entry_index, counter_value);
    else
        rdd_error = _rdd_read_iptv_ssm_entry_counter(entry_index, counter_value);

    return rdd_error;
}

static inline int _rdd_read_iptv_l3_dst_ip_counter(uint32_t entry_index, uint16_t *counter_value)
{
    return _rdd_read_iptv_entry_counter(entry_index, counter_value);
}

static inline int _rdd_read_iptv_l3_dst_ip_src_ip_vid_counter(uint32_t entry_index, uint16_t *counter_value)
{
    return _rdd_read_iptv_l3_dst_ip_src_ip_counter(entry_index, counter_value);
}

int rdd_iptv_counter_get(uint32_t entry_index, uint16_t *counter_value)
{
    rdpa_iptv_lookup_method iptv_classification_mode;
    RDD_SYSTEM_CONFIGURATION_DTS *system_cfg;
    int rdd_error;

    rdd_error = BDMF_ERR_OK;

    system_cfg = (RDD_SYSTEM_CONFIGURATION_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_SYSTEM_CONFIGURATION_ADDRESS);

    RDD_SYSTEM_CONFIGURATION_IPTV_CLASSIFICATION_METHOD_READ(iptv_classification_mode, system_cfg);

    switch (iptv_classification_mode)
    {
    case iptv_lookup_method_mac:

         rdd_error = _rdd_read_iptv_l2_mac_counter(entry_index, counter_value);
         break;

    case iptv_lookup_method_mac_vid:

         rdd_error = _rdd_read_iptv_l2_mac_vid_counter(entry_index, counter_value);
         break;

    case iptv_lookup_method_group_ip_src_ip:

         rdd_error = _rdd_read_iptv_l3_dst_ip_src_ip_counter(entry_index, counter_value);
         break;

    case iptv_lookup_method_group_ip:

        rdd_error = _rdd_read_iptv_l3_dst_ip_counter(entry_index, counter_value);
        break;

    case iptv_lookup_method_group_ip_src_ip_vid:

         rdd_error = _rdd_read_iptv_l3_dst_ip_src_ip_vid_counter(entry_index, counter_value);
         break;
    }

    return rdd_error;
}

int _rdd_add_iptv_layer3_src_ip(bdmf_ip_t *src_ip, bdmf_ip_t *ipv6_src_ip_ptr, uint32_t *result_entry_index)
{
    uint32_t entry_index;
    uint8_t hash_entry[8];
    int rdd_error;

    if (ipv6_src_ip_ptr == NULL)
    {
        hash_entry[0] = 0;
        hash_entry[1] = 0;
        hash_entry[2] = 0;
        hash_entry[3] = 0;
    }
    else
    {
        hash_entry[0] = ipv6_src_ip_ptr->addr.ipv6.data[12];
        hash_entry[1] = ipv6_src_ip_ptr->addr.ipv6.data[13];
        hash_entry[2] = ipv6_src_ip_ptr->addr.ipv6.data[14];
        hash_entry[3] = ipv6_src_ip_ptr->addr.ipv6.data[15];
    }

    hash_entry[4] = (src_ip->addr.ipv4 >> 24) & 0xFF;
    hash_entry[5] = (src_ip->addr.ipv4 >> 16) & 0xFF;
    hash_entry[6] = (src_ip->addr.ipv4 >>  8) & 0xFF;
    hash_entry[7] = (src_ip->addr.ipv4 >>  0) & 0xFF;

    rdd_error = rdd_find_hash_entry_64_bit(&g_hash_table_cfg[RDD_IPTV_SRC_IP_TABLE], hash_entry,
        IPTV_L3_SSM_SRC_IP_ENTRY_KEY_MASK_HIGH, IPTV_L3_SSM_SRC_IP_ENTRY_KEY_MASK_LOW, 0, &entry_index);

    if (rdd_error)
    {
        rdd_error = rdd_add_hash_entry_64_bit(&g_hash_table_cfg[RDD_IPTV_SRC_IP_TABLE], hash_entry, NULL,
            IPTV_L3_SSM_SRC_IP_ENTRY_KEY_MASK_HIGH, IPTV_L3_SSM_SRC_IP_ENTRY_KEY_MASK_LOW, 0, &entry_index);

        if (rdd_error)
            return rdd_error;
    }

    g_iptv_source_ip_counter[entry_index]++;

    *result_entry_index = entry_index;

    return BDMF_ERR_OK;
}

int _rdd_delete_iptv_layer3_src_ip(bdmf_ip_t *src_ip, uint32_t *result_entry_index)
{
    uint32_t entry_index;
    uint8_t hash_entry[8];
    int rdd_error;

    if (src_ip == NULL)
        return BDMF_ERR_NOENT;

    if (src_ip->family == bdmf_ip_family_ipv6)
    {
        hash_entry[0] = src_ip->addr.ipv6.data[12];
        hash_entry[1] = src_ip->addr.ipv6.data[13];
        hash_entry[2] = src_ip->addr.ipv6.data[14];
        hash_entry[3] = src_ip->addr.ipv6.data[15];
        hash_entry[4] = src_ip->addr.ipv6.data[3];
        hash_entry[5] = src_ip->addr.ipv6.data[2];
        hash_entry[6] = src_ip->addr.ipv6.data[1];
        hash_entry[7] = src_ip->addr.ipv6.data[0];
    }
    else
    {
        hash_entry[0] = 0;
        hash_entry[1] = 0;
        hash_entry[2] = 0;
        hash_entry[3] = 0;
        hash_entry[4] = (src_ip->addr.ipv4 >> 24) & 0xFF;
        hash_entry[5] = (src_ip->addr.ipv4 >> 16) & 0xFF;
        hash_entry[6] = (src_ip->addr.ipv4 >>  8) & 0xFF;
        hash_entry[7] = (src_ip->addr.ipv4 >>  0) & 0xFF;
    }

    rdd_error = rdd_find_hash_entry_64_bit(&g_hash_table_cfg[RDD_IPTV_SRC_IP_TABLE], hash_entry,
        IPTV_L3_SSM_SRC_IP_ENTRY_KEY_MASK_HIGH, IPTV_L3_SSM_SRC_IP_ENTRY_KEY_MASK_LOW, 0, &entry_index);

    if (rdd_error)
        return rdd_error;

    g_iptv_source_ip_counter[entry_index]--;

    if (g_iptv_source_ip_counter[entry_index] == 0)
    {
        rdd_error = rdd_remove_hash_entry_64_bit(&g_hash_table_cfg[RDD_IPTV_SRC_IP_TABLE], hash_entry,
            IPTV_L3_SSM_SRC_IP_ENTRY_KEY_MASK_HIGH, IPTV_L3_SSM_SRC_IP_ENTRY_KEY_MASK_LOW, 0, RDD_CAM_OPTIMIZATION_DISABLE, &entry_index);

        if (rdd_error)
            return rdd_error;
    }

    *result_entry_index = entry_index;
    return BDMF_ERR_OK;
}
