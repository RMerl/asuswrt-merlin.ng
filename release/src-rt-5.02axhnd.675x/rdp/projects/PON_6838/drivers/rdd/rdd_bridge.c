/*
    <:copyright-BRCM:2013:DUAL/GPL:standard
    
       Copyright (c) 2013 Broadcom 
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

/******************************************************************************/
/*                                                                            */
/*                            Global Variables                                */
/*                                                                            */
/******************************************************************************/

#if defined(FIRMWARE_INIT)
extern uint32_t  IPTVTableBase;
extern uint32_t  IPTVContextTableBase;
extern uint32_t  IPTVSsmContextTableBase;
#endif

uint8_t    g_static_mac_counter[ RDD_MAC_TABLE_SIZE + RDD_MAC_TABLE_CAM_SIZE - 1 ];
uint8_t    g_local_switching_filters_mode[ LILAC_RDD_NUMBER_OF_BRIDGE_PORTS + 1 ];

extern uint8_t    *g_runner_ddr_base_addr;
extern uint32_t    g_runner_tables_ptr;
extern uint32_t    g_mac_table_size;
extern uint32_t    g_mac_table_search_depth;
extern uint32_t    g_iptv_table_size;
extern uint32_t    g_iptv_table_search_depth;
extern uint32_t    g_iptv_context_tables_free_list_head;
extern uint32_t    g_iptv_context_tables_free_list_tail;
extern uint32_t    g_iptv_source_ip_counter[ LILAC_RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS ];
extern uint32_t    g_src_mac_anti_spoofing_last_rule_index[ 6 ];
extern uint32_t    g_acl_layer2_vlan_index_counter[ 6 ][ 32 ];
extern uint32_t    g_acl_layer2_last_vlan_index[ 6 ];
extern uint32_t    g_acl_layer2_last_rule_index[ 6 ];
extern uint32_t    g_acl_layer3_last_rule_index[ 6 ];
extern uint8_t     g_broadcom_switch_mode;
extern uint32_t    g_vlan_mapping_command_to_action[ rdd_max_vlan_command ][ rdd_max_pbits_command ];
extern BL_LILAC_RDD_WAN_PHYSICAL_PORT_DTE         g_wan_physical_port;
extern BL_LILAC_RDD_VLAN_ACTIONS_MATRIX_DTS       *g_vlan_actions_matrix_ptr;
extern RDD_64_BIT_TABLE_CFG                       g_hash_table_cfg[ BL_LILAC_RDD_MAX_HASH_TABLE ];
extern BL_LILAC_RDD_CONTEXT_TABLES_FREE_LIST_DTS  *g_iptv_context_tables_free_list;
extern BL_LILAC_RDD_ACL_LAYER3_FILTER_MODE_DTE    g_acl_layer3_filter_mode[ 6 ];
extern BL_LILAC_RDD_ETHER_TYPE_FILTER_MATRIX_DTS  *g_ether_type_filter_mode;

extern BL_LILAC_RDD_LOCK_CRITICAL_SECTION_FP_DTE    f_rdd_lock;
extern BL_LILAC_RDD_UNLOCK_CRITICAL_SECTION_FP_DTE  f_rdd_unlock;
extern bdmf_fastlock                                int_lock;
extern BL_LILAC_RDD_LOCK_CRITICAL_SECTION_FP_IRQ_DTE    f_rdd_lock_irq;
extern BL_LILAC_RDD_UNLOCK_CRITICAL_SECTION_FP_IRQ_DTE  f_rdd_unlock_irq;
extern bdmf_fastlock                                    int_lock_irq;

/* static functions declarations */
static inline void f_rdd_ingress_filter_enable ( RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS * );
static inline void f_rdd_ingress_filter_disable ( RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS * );

static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l3_dst_ip_src_ip_existing_any_entry_to_cache ( RDD_IPTV_ENTRY_UNION *, uint32_t );
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l3_dst_ip_src_ip_existing_any_entry_to_ddr ( RDD_IPTV_ENTRY_UNION *, uint32_t );
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l3_dst_ip_src_ip_existing_entry_to_cache ( uint32_t, uint32_t );
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l3_dst_ip_src_ip_existing_entry_to_ddr ( RDD_IPTV_ENTRY_UNION *, uint32_t, uint32_t, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l3_dst_ip_src_ip_base_entry_to_cache ( RDD_IPTV_ENTRY_UNION *, uint8_t *, uint32_t *, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l3_dst_ip_src_ip_base_entry_to_ddr ( RDD_IPTV_ENTRY_UNION *, uint8_t *, uint32_t, uint32_t, rdpa_iptv_lookup_method, bdmf_ip_t *, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l3_dst_ip_src_ip_existing_any_entry ( RDD_IPTV_ENTRY_UNION *, uint32_t, uint32_t );
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l3_dst_ip_src_ip_existing_entry ( RDD_IPTV_ENTRY_UNION *, uint32_t, uint32_t, uint32_t, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l3_dst_ip_src_ip_first_any_entry ( RDD_IPTV_ENTRY_UNION *, rdpa_iptv_lookup_method, bdmf_ip_t *, uint32_t *, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l3_dst_ip_src_ip_first_entry ( RDD_IPTV_ENTRY_UNION *, rdpa_iptv_lookup_method, bdmf_ip_t *, bdmf_ip_t *, uint32_t *, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_find_iptv_l3_dst_ip_src_ip_base_entry ( RDD_IPTV_ENTRY_UNION *, rdpa_iptv_lookup_method, bdmf_ip_t *, uint32_t *, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l2_mac_entry ( RDD_IPTV_ENTRY_UNION *, uint32_t *, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_entry_to_cache ( RDD_IPTV_ENTRY_UNION *, uint8_t *, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_entry_to_ddr ( RDD_IPTV_ENTRY_UNION *, uint8_t *, rdpa_iptv_lookup_method, uint32_t, bdmf_ip_t *, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l2_mac_vid_entry ( RDD_IPTV_ENTRY_UNION *, uint32_t *, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l3_dst_ip_src_ip_entry ( RDD_IPTV_ENTRY_UNION *, rdpa_iptv_lookup_method, bdmf_ip_t *, bdmf_ip_t *, uint32_t *, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l3_dst_ip_entry ( RDD_IPTV_ENTRY_UNION *, bdmf_ip_t *, uint32_t *, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_entry_from_cache ( uint32_t );
static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_entry_from_ddr ( uint32_t, uint32_t *, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_l2_mac_entry ( uint32_t, uint32_t	* );
static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_l2_mac_vid_entry ( uint32_t, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_free_iptv_l3_dst_ip_src_ip_context_table ( uint32_t );
static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_l3_dst_ip_src_ip_any_entry_from_cache ( uint32_t );
static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_l3_dst_ip_src_ip_any_entry_from_ddr ( uint32_t, uint32_t *, uint32_t *, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_l3_dst_ip_src_ip_non_any_entry_from_cache ( uint32_t, uint32_t );
static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_l3_dst_ip_src_ip_non_any_entry_from_ddr ( uint32_t, uint32_t, uint32_t *, uint32_t *, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_l3_dst_ip_src_ip_any_entry ( uint32_t, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_l3_dst_ip_src_ip_non_any_entry ( uint32_t, uint32_t, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_l3_dst_ip_src_ip_entry ( uint32_t, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_l3_dst_ip_entry ( uint32_t, uint32_t * );
static inline BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_l3_dst_ip_src_ip_vid_entry ( uint32_t, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_modify_iptv_entry_in_cache ( uint32_t, uint32_t, uint16_t, uint8_t );
static BL_LILAC_RDD_ERROR_DTE f_rdd_modify_iptv_entry_in_ddr ( uint32_t, uint32_t, uint16_t, uint8_t, rdpa_iptv_lookup_method );
static BL_LILAC_RDD_ERROR_DTE f_rdd_modify_iptv_ssm_context_entry_in_ddr ( uint32_t, uint32_t, uint16_t, uint8_t );
static BL_LILAC_RDD_ERROR_DTE f_rdd_modify_iptv_l2_mac_entry ( uint32_t, uint32_t, uint16_t, uint8_t );
static BL_LILAC_RDD_ERROR_DTE f_rdd_modify_iptv_l2_mac_vid_entry ( uint32_t, uint32_t, uint16_t, uint8_t );
static BL_LILAC_RDD_ERROR_DTE f_rdd_modify_iptv_l3_dst_ip_src_ip_entry ( uint32_t, uint32_t, uint16_t, uint8_t );
static BL_LILAC_RDD_ERROR_DTE f_rdd_modify_iptv_l3_dst_ip_entry ( uint32_t, uint32_t, uint16_t, uint8_t );
static BL_LILAC_RDD_ERROR_DTE f_rdd_modify_iptv_l3_dst_ip_src_ip_vid_entry ( uint32_t, uint32_t, uint16_t, uint8_t );
static BL_LILAC_RDD_ERROR_DTE f_rdd_search_iptv_ssm_entry ( RDD_IPTV_ENTRY_UNION *, bdmf_ip_t *, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_search_iptv_entry ( RDD_IPTV_ENTRY_UNION *, uint8_t *, rdpa_iptv_lookup_method, bdmf_ip_t *, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_get_iptv_l2_mac_entry_index ( RDD_IPTV_ENTRY_UNION *, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_get_iptv_l2_mac_vid_entry_index ( RDD_IPTV_ENTRY_UNION *, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_get_iptv_l3_dst_ip_src_ip_entry_index ( RDD_IPTV_ENTRY_UNION *, rdpa_iptv_lookup_method, bdmf_ip_t *, bdmf_ip_t *, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_get_iptv_l3_dst_ip_entry_index ( RDD_IPTV_ENTRY_UNION *, bdmf_ip_t *, bdmf_ip_t *, uint32_t *);
static BL_LILAC_RDD_ERROR_DTE f_rdd_read_iptv_ssm_entry ( uint32_t, RDD_IPTV_ENTRY_UNION *, rdpa_iptv_lookup_method, bdmf_ip_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_read_iptv_entry ( uint32_t, rdpa_iptv_lookup_method, RDD_IPTV_ENTRY_UNION *, bdmf_ip_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_get_iptv_l2_mac_entry ( uint32_t, RDD_IPTV_ENTRY_UNION * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_get_iptv_l2_mac_vid_entry ( uint32_t, RDD_IPTV_ENTRY_UNION * );
static inline BL_LILAC_RDD_ERROR_DTE f_rdd_get_iptv_l3_dst_ip_src_ip_entry ( uint32_t, RDD_IPTV_ENTRY_UNION *, bdmf_ip_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_get_iptv_l3_dst_ip_entry ( uint32_t, RDD_IPTV_ENTRY_UNION *, bdmf_ip_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_read_iptv_ssm_entry_counter ( uint32_t, uint16_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_read_iptv_ssm_any_entry_counter ( uint32_t, uint16_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_read_iptv_entry_counter ( uint32_t, uint16_t * );
static inline BL_LILAC_RDD_ERROR_DTE f_rdd_read_iptv_l2_mac_counter ( uint32_t, uint16_t * );
static inline BL_LILAC_RDD_ERROR_DTE f_rdd_read_iptv_l2_mac_vid_counter ( uint32_t, uint16_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_read_iptv_l3_dst_ip_src_ip_counter ( uint32_t, uint16_t * );
static inline BL_LILAC_RDD_ERROR_DTE f_rdd_read_iptv_l3_dst_ip_counter ( uint32_t, uint16_t * );
static inline BL_LILAC_RDD_ERROR_DTE f_rdd_read_iptv_l3_dst_ip_src_ip_vid_counter ( uint32_t, uint16_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_mac_entry_modify ( RDD_MAC_PARAMS  *xi_mac_params_ptr );
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_layer3_src_ip ( bdmf_ip_t *, bdmf_ip_t *, uint32_t * );
static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_layer3_src_ip ( bdmf_ip_t *, uint32_t * );

/* iptv auxiliary macros */
#define GET_BASE_INDEX_FROM_ENTRY_INDEX( entry_index )                                 ( ( entry_index ) & 0x1fff )
#define GET_PROVIDER_INDEX_FROM_ENTRY_INDEX( entry_index )                             ( ( ( ( entry_index ) >> 13 ) & 0x3f ) - 1 )
#define GET_ENTRY_INDEX_FROM_BASE_AND_PROVIDER_INDICES( base_index, provider_index )   ( ( base_index ) | ( ( ( provider_index ) + 1 ) << 13 ) )



BL_LILAC_RDD_ERROR_DTE f_rdd_mac_table_initialize ( uint32_t  xi_mac_table_size,
                                                    uint32_t  xi_iptv_table_size )
{
    RUNNER_REGS_CFG_LKUP1_CFG          hash_lkup_1_cfg_register;
    RUNNER_REGS_CFG_LKUP1_CAM_CFG      hash_lkup_1_cam_cfg_register;
    RUNNER_REGS_CFG_LKUP_GLBL_MASK1_H  hash_lkup_1_global_mask_high_register;
    RUNNER_REGS_CFG_LKUP_GLBL_MASK1_L  hash_lkup_1_global_mask_low_register;
    RUNNER_REGS_CFG_LKUP2_CFG          hash_lkup_2_cfg_register;
    RUNNER_REGS_CFG_LKUP_GLBL_MASK2_H  hash_lkup_2_global_mask_high_register;
    RUNNER_REGS_CFG_LKUP_GLBL_MASK2_L  hash_lkup_2_global_mask_low_register;
    RDD_64_BIT_TABLE_CFG               *hash_table_cfg_ptr;
    uint32_t                           index;

    if ( xi_iptv_table_size > BL_LILAC_RDD_MAC_TABLE_SIZE_256 ) 
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_IPTV_TABLE_CACHE_SIZE );
    }

    hash_lkup_1_cfg_register.base_address = ( MAC_TABLE_ADDRESS >> 3 );
    hash_lkup_1_cfg_register.table_size = xi_mac_table_size;
    hash_lkup_1_cfg_register.max_hop = BL_LILAC_RDD_MAC_TABLE_MAX_HOP_64;
    hash_lkup_1_cfg_register.hash_type = LILAC_RDD_MAC_HASH_TYPE_CRC16;

    RUNNER_REGS_0_CFG_LKUP1_CFG_WRITE ( hash_lkup_1_cfg_register );
    RUNNER_REGS_1_CFG_LKUP1_CFG_WRITE ( hash_lkup_1_cfg_register );


    hash_lkup_1_cam_cfg_register.cam_en = LILAC_RDD_TRUE;
    hash_lkup_1_cam_cfg_register.base_address = ( MAC_TABLE_CAM_ADDRESS >> 3 );

    RUNNER_REGS_0_CFG_LKUP1_CAM_CFG_WRITE ( hash_lkup_1_cam_cfg_register );
    RUNNER_REGS_1_CFG_LKUP1_CAM_CFG_WRITE ( hash_lkup_1_cam_cfg_register );


    hash_lkup_1_global_mask_high_register.base_address = 0xFFFF;

    RUNNER_REGS_0_CFG_LKUP_GLBL_MASK1_H_WRITE ( hash_lkup_1_global_mask_high_register );

    hash_lkup_1_global_mask_low_register.base_address = 0xFFFFFFFF;

    RUNNER_REGS_0_CFG_LKUP_GLBL_MASK1_L_WRITE ( hash_lkup_1_global_mask_low_register );


    /* initialize the CRC software variables */
    rdd_crc_init();

    /* initialize global variables - MAC Table Size and Search Depth, the values should be translated from HW defined constants to real values */
    hash_table_cfg_ptr = &g_hash_table_cfg[ BL_LILAC_RDD_MAC_TABLE ];

    hash_table_cfg_ptr->hash_table_size = ( 1 << xi_mac_table_size ) * 32;
    hash_table_cfg_ptr->hash_table_search_depth = ( 1 << BL_LILAC_RDD_MAC_TABLE_MAX_HOP_64 );
    hash_table_cfg_ptr->hash_table_ptr = ( RDD_64_BIT_TABLE_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_TABLE_ADDRESS );

    hash_table_cfg_ptr->is_external_context = BL_LILAC_RDD_EXTERNAL_CONTEXT_ENABLE;
    hash_table_cfg_ptr->context_size = BL_LILAC_RDD_CONTEXT_16_BIT;
    hash_table_cfg_ptr->context_table_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_CONTEXT_TABLE_ADDRESS );
    hash_table_cfg_ptr->is_extension_cam = LILAC_RDD_TRUE;
    hash_table_cfg_ptr->cam_table_size = RDD_MAC_TABLE_CAM_SIZE - 1;
    hash_table_cfg_ptr->cam_table_ptr = ( RDD_64_BIT_TABLE_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_TABLE_CAM_ADDRESS );
    hash_table_cfg_ptr->cam_context_table_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_CONTEXT_TABLE_CAM_ADDRESS );

    hash_table_cfg_ptr = &g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ];

    hash_table_cfg_ptr->hash_table_size = ( 1 << xi_iptv_table_size ) * 32;
    hash_table_cfg_ptr->hash_table_search_depth = ( 1 << BL_LILAC_RDD_MAC_TABLE_MAX_HOP_32 );
    hash_table_cfg_ptr->hash_table_ptr = ( RDD_64_BIT_TABLE_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + IPTV_LOOKUP_TABLE_ADDRESS );

    hash_table_cfg_ptr->is_external_context = BL_LILAC_RDD_EXTERNAL_CONTEXT_ENABLE;
#ifndef G9991
    hash_table_cfg_ptr->context_size = BL_LILAC_RDD_CONTEXT_16_BIT;
#else
    hash_table_cfg_ptr->context_size = BL_LILAC_RDD_CONTEXT_32_BIT;
#endif
    hash_table_cfg_ptr->context_table_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + IPTV_CONTEXT_TABLE_ADDRESS );
    hash_table_cfg_ptr->is_extension_cam = LILAC_RDD_TRUE;
    hash_table_cfg_ptr->cam_table_size = RDD_MAC_TABLE_CAM_SIZE - 1;
    hash_table_cfg_ptr->cam_table_ptr = ( RDD_64_BIT_TABLE_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + IPTV_LOOKUP_TABLE_CAM_ADDRESS );
    hash_table_cfg_ptr->cam_context_table_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + IPTV_CONTEXT_TABLE_CAM_ADDRESS );

    g_mac_table_size = ( 1 << xi_mac_table_size ) * 32;
    g_mac_table_search_depth = ( 1 << BL_LILAC_RDD_MAC_TABLE_MAX_HOP_64 );

    hash_lkup_2_cfg_register.base_address = ( IPTV_L3_SRC_IP_LOOKUP_TABLE_ADDRESS >> 3 );
    hash_lkup_2_cfg_register.table_size = BL_LILAC_RDD_MAC_TABLE_SIZE_32;
    hash_lkup_2_cfg_register.max_hop = BL_LILAC_RDD_MAC_TABLE_MAX_HOP_32;
    hash_lkup_2_cfg_register.hash_type = LILAC_RDD_MAC_HASH_TYPE_CRC16;

    RUNNER_REGS_0_CFG_LKUP2_CFG_WRITE ( hash_lkup_2_cfg_register );

    hash_lkup_2_global_mask_high_register.base_address = 0x0FFFFFFF;

    RUNNER_REGS_0_CFG_LKUP_GLBL_MASK2_H_WRITE ( hash_lkup_2_global_mask_high_register );

    hash_lkup_2_global_mask_low_register.base_address = 0xFFFFFFFF;

    RUNNER_REGS_0_CFG_LKUP_GLBL_MASK2_L_WRITE ( hash_lkup_2_global_mask_low_register );

    hash_table_cfg_ptr = &g_hash_table_cfg[ BL_LILAC_RDD_IPTV_SRC_IP_TABLE ];

    hash_table_cfg_ptr->hash_table_size = LILAC_RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS;
    hash_table_cfg_ptr->hash_table_search_depth = LILAC_RDD_IPTV_SOURCE_IP_TABLE_SEARCH_DEPTH;
    hash_table_cfg_ptr->hash_table_ptr = ( RDD_64_BIT_TABLE_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + IPTV_L3_SRC_IP_LOOKUP_TABLE_ADDRESS );
    hash_table_cfg_ptr->is_external_context = BL_LILAC_RDD_EXTERNAL_CONTEXT_DISABLE;
    hash_table_cfg_ptr->context_size = BL_LILAC_RDD_CONTEXT_8_BIT;
    hash_table_cfg_ptr->context_table_ptr = 0;
    hash_table_cfg_ptr->is_extension_cam = LILAC_RDD_FALSE;

    g_iptv_table_size = ( 1 << xi_iptv_table_size ) * 32;
    g_iptv_table_search_depth = ( 1 << BL_LILAC_RDD_MAC_TABLE_MAX_HOP_32 );

    /* reset static mac counters */
    for ( index = 0; index < ( RDD_MAC_TABLE_SIZE + RDD_MAC_TABLE_CAM_SIZE - 1 ); index++ )
    {
        g_static_mac_counter[ index ] = 0;
    }

    return ( BL_LILAC_RDD_OK );
}


static uint8_t f_rdd_extract_replication_number ( uint32_t  egress_port_vector) 
{
    uint8_t  replication_number = 0;

    while (egress_port_vector > 0)             // until all bits are zero
    {
        if ((egress_port_vector & 1) == 1)     // check lower bit
        {
            (replication_number)++;
        }
        egress_port_vector >>= 1;              // shift bits, removing lower bit
    }

    (replication_number)--;

    return replication_number;
}


BL_LILAC_RDD_ERROR_DTE f_rdd_iptv_initialize ( void )
{
    uint32_t  *iptv_table_pointer_ptr;
    uint32_t  *iptv_context_table_pointer_ptr;
    uint32_t  *iptv_ssm_context_table_address_ptr;
    uint32_t  iptv_table_address;
    uint32_t  iptv_context_table_address;
    uint32_t  iptv_ssm_context_table_address;
    uint16_t  i;

    iptv_table_address = ( g_runner_tables_ptr + IPTV_DDR_LOOKUP_TABLE_ADDRESS ) & 0x1FFFFFFF;
    iptv_context_table_address = ( g_runner_tables_ptr + IPTV_DDR_CONTEXT_TABLE_ADDRESS ) & 0x1FFFFFFF;

    iptv_table_pointer_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPTV_TABLE_POINTER_ADDRESS );
    iptv_context_table_pointer_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPTV_CONTEXT_TABLE_POINTER_ADDRESS );

    MWRITE_32( iptv_table_pointer_ptr, iptv_table_address );
    MWRITE_32( iptv_context_table_pointer_ptr, iptv_context_table_address );

    iptv_ssm_context_table_address_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPTV_SSM_CONTEXT_TABLE_PTR_ADDRESS );

    iptv_ssm_context_table_address = ( g_runner_tables_ptr + IPTV_SSM_DDR_CONTEXT_TABLE_ADDRESS ) & 0x1FFFFFFF;

    MWRITE_32( iptv_ssm_context_table_address_ptr, iptv_ssm_context_table_address );


    g_iptv_context_tables_free_list = ( BL_LILAC_RDD_CONTEXT_TABLES_FREE_LIST_DTS * )malloc( sizeof ( BL_LILAC_RDD_CONTEXT_TABLES_FREE_LIST_DTS ) );
    g_iptv_context_tables_free_list_head = 0;
    g_iptv_context_tables_free_list_tail = LILAC_RDD_IPTV_SSM_CONTEXT_ENTRY_COUNT - 1;

    for ( i = 0; i < LILAC_RDD_IPTV_SSM_CONTEXT_ENTRY_COUNT; i++ )
    {
        g_iptv_context_tables_free_list->entry[ i ] = i + 1;
    }

    for ( i = 0; i < LILAC_RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS; i++ )
    {
        g_iptv_source_ip_counter[ i ] = 0;
    }

    return ( BL_LILAC_RDD_OK );
}


static inline void f_rdd_ingress_filter_enable ( RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS  *filters_cam_entry_ptr )
{
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_PTAG_WRITE ( 0, filters_cam_entry_ptr );
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_PTAG_MASK_WRITE ( 0, filters_cam_entry_ptr );
}


static inline void f_rdd_ingress_filter_disable ( RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS  *filters_cam_entry_ptr )
{
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_PTAG_WRITE ( 1, filters_cam_entry_ptr );
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_PTAG_MASK_WRITE ( 0, filters_cam_entry_ptr );
}


BL_LILAC_RDD_ERROR_DTE f_rdd_ingress_filters_cam_initialize ( void )
{
    RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_DTS  *ds_filters_cam_table_ptr;
    RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_DTS  *us_filters_cam_table_ptr;
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS     *filters_cam_entry_ptr;
    BL_LILAC_RDD_BRIDGE_PORT_DTE             bridge_port;
    uint32_t                                 subnet_id;
    int32_t                                  bridge_port_index;

    g_ether_type_filter_mode = ( BL_LILAC_RDD_ETHER_TYPE_FILTER_MATRIX_DTS * )malloc( sizeof ( BL_LILAC_RDD_ETHER_TYPE_FILTER_MATRIX_DTS ) );

    if ( g_ether_type_filter_mode == NULL)
    {
        return ( BL_LILAC_RDD_ERROR_MALLOC_FAILED );
    }

    for ( bridge_port = BL_LILAC_RDD_WAN_BRIDGE_PORT; bridge_port <= BL_LILAC_RDD_PCI_BRIDGE_PORT; bridge_port++ )
    {
        if ( bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
        {
            ds_filters_cam_table_ptr = RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_PTR();
        }
        else
        {
            us_filters_cam_table_ptr = RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_PTR();
        }

        for ( subnet_id = BL_LILAC_RDD_SUBNET_FLOW_CACHE; subnet_id < LILAC_RDD_NUMBER_OF_SUBNETS; subnet_id++ )
        {
            if ( ( bridge_port_index = rdd_bridge_port_to_port_index ( bridge_port, subnet_id ) ) < 0 )
            {
                break;
            }

            if ( bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
            {
                filters_cam_entry_ptr = &( ds_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_IGMP_FILTER_NUMBER ] );
            }
            else
            {
                filters_cam_entry_ptr = &( us_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_IGMP_FILTER_NUMBER ] );
            }          

            f_rdd_ingress_filter_disable ( filters_cam_entry_ptr );

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_4_PROTOCOL_IGMP, filters_cam_entry_ptr );
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0xF, filters_cam_entry_ptr );

            if ( bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
            {
                filters_cam_entry_ptr = &( ds_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_ICMPV6_FILTER_NUMBER ] );
            }
            else
            {
                filters_cam_entry_ptr = &( us_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_ICMPV6_FILTER_NUMBER ] );
            }      

            f_rdd_ingress_filter_disable ( filters_cam_entry_ptr );

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_4_PROTOCOL_ICMPV6, filters_cam_entry_ptr );
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE ( 0xF, filters_cam_entry_ptr );

            if ( bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
            {
                filters_cam_entry_ptr = &( ds_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_USER_0_FILTER_NUMBER ] );
            }
            else
            {
                filters_cam_entry_ptr = &( us_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_USER_0_FILTER_NUMBER ] );
            }

            f_rdd_ingress_filter_disable ( filters_cam_entry_ptr );

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_2_PROTOCOL_USER_0, filters_cam_entry_ptr );
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_MASK_WRITE ( 0xF, filters_cam_entry_ptr );

            if ( bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
            {
                filters_cam_entry_ptr = &( ds_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_USER_1_FILTER_NUMBER ] );
            }
            else
            {
                filters_cam_entry_ptr = &( us_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_USER_1_FILTER_NUMBER ] );
            }
            
            f_rdd_ingress_filter_disable ( filters_cam_entry_ptr );

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_2_PROTOCOL_USER_1, filters_cam_entry_ptr );
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_MASK_WRITE ( 0xF, filters_cam_entry_ptr );

            if ( bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
            {
                filters_cam_entry_ptr = &( ds_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_USER_2_FILTER_NUMBER ] );
            }
            else
            {
                filters_cam_entry_ptr = &( us_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_USER_2_FILTER_NUMBER ] );
            }
            
            f_rdd_ingress_filter_disable ( filters_cam_entry_ptr );

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_2_PROTOCOL_USER_2, filters_cam_entry_ptr );
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_MASK_WRITE ( 0xF, filters_cam_entry_ptr );

            if ( bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
            {
                filters_cam_entry_ptr = &( ds_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_USER_3_FILTER_NUMBER ] );
            }
            else
            {
                filters_cam_entry_ptr = &( us_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_USER_3_FILTER_NUMBER ] );
            }
            
            f_rdd_ingress_filter_disable ( filters_cam_entry_ptr );

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_2_PROTOCOL_USER_3, filters_cam_entry_ptr );
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_MASK_WRITE ( 0xF, filters_cam_entry_ptr );

            if ( bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
            {
                filters_cam_entry_ptr = &( ds_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_PPPOE_D_FILTER_NUMBER ] );
            }
            else
            {
                filters_cam_entry_ptr = &( us_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_PPPOE_D_FILTER_NUMBER ] );
            }
            
            f_rdd_ingress_filter_disable ( filters_cam_entry_ptr );

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_2_PROTOCOL_PPPOE_D, filters_cam_entry_ptr );
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_MASK_WRITE ( 0xF, filters_cam_entry_ptr );

            if ( bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
            {
                filters_cam_entry_ptr = &( ds_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_PPPOE_S_FILTER_NUMBER ] );
            }
            else
            {
                filters_cam_entry_ptr = &( us_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_PPPOE_S_FILTER_NUMBER ] );
            }
        
            f_rdd_ingress_filter_disable ( filters_cam_entry_ptr );

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_2_PROTOCOL_PPPOE_S, filters_cam_entry_ptr );
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_MASK_WRITE ( 0xF, filters_cam_entry_ptr );

            if ( bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
            {
                RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L3_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_3_PROTOCOL_OTHER, filters_cam_entry_ptr );
                RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L3_PROTOCOL_MASK_WRITE ( 0x3, filters_cam_entry_ptr );
            }

            if ( bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
            {
                filters_cam_entry_ptr = &( ds_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_ARP_FILTER_NUMBER ] );
            }
            else
            {
                filters_cam_entry_ptr = &( us_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_ARP_FILTER_NUMBER ] );
            }

            f_rdd_ingress_filter_disable ( filters_cam_entry_ptr );

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_2_PROTOCOL_ARP, filters_cam_entry_ptr );
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_MASK_WRITE ( 0xF, filters_cam_entry_ptr );

            if ( bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
            {
                filters_cam_entry_ptr = &( ds_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_1588_FILTER_NUMBER ] );
            }
            else
            {
                filters_cam_entry_ptr = &( us_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_1588_FILTER_NUMBER ] );
            }
            
            f_rdd_ingress_filter_disable ( filters_cam_entry_ptr );

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_2_PROTOCOL_1588, filters_cam_entry_ptr );
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_MASK_WRITE ( 0xF, filters_cam_entry_ptr );

            if ( bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
            {
                filters_cam_entry_ptr = &( ds_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_802_1X_FILTER_NUMBER ] );
            }
            else
            {
                filters_cam_entry_ptr = &( us_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_802_1X_FILTER_NUMBER ] );
            }

            f_rdd_ingress_filter_disable ( filters_cam_entry_ptr );

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_2_PROTOCOL_802_1X, filters_cam_entry_ptr );
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_MASK_WRITE ( 0xF, filters_cam_entry_ptr );

            if ( bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
            {
                filters_cam_entry_ptr = &( ds_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_802_1AG_CFM_FILTER_NUMBER ] );
            }
            else
            {
                filters_cam_entry_ptr = &( us_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_802_1AG_CFM_FILTER_NUMBER ] );
            }
            
            f_rdd_ingress_filter_disable ( filters_cam_entry_ptr );

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_WRITE ( LILAC_RDD_PARSER_LAYER_2_PROTOCOL_802_1AG_CFM, filters_cam_entry_ptr );
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_MASK_WRITE ( 0xF, filters_cam_entry_ptr );

            if ( bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
            {
                filters_cam_entry_ptr = &( ds_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_BROADCAST_FILTER_NUMBER ] );
            }
            else
            {
                filters_cam_entry_ptr = &( us_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_BROADCAST_FILTER_NUMBER ] );
            }

            f_rdd_ingress_filter_disable ( filters_cam_entry_ptr );

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_BROADCAST_WRITE ( LILAC_RDD_ON, filters_cam_entry_ptr );
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_BROADCAST_MASK_WRITE ( 0x1, filters_cam_entry_ptr );

            if ( bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
            {
                filters_cam_entry_ptr = &( ds_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_MULTICAST_FILTER_NUMBER ] );
            }
            else
            {
                filters_cam_entry_ptr = &( us_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_MULTICAST_FILTER_NUMBER ] );
            }
            
            f_rdd_ingress_filter_disable ( filters_cam_entry_ptr );

            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_MULTICAST_WRITE ( LILAC_RDD_ON, filters_cam_entry_ptr );
            RDD_INGRESS_FILTERS_LOOKUP_ENTRY_MULTICAST_MASK_WRITE ( 0x1, filters_cam_entry_ptr );

            if ( bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
            {
                filters_cam_entry_ptr = &( ds_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_INGRESS_FILTERS_NUMBER ] );
            }
            else
            {
                filters_cam_entry_ptr = &( us_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_INGRESS_FILTERS_NUMBER ] );
            }
            
            MWRITE_16( filters_cam_entry_ptr, 0xFFFF );
        }
    }

    for ( bridge_port = BL_LILAC_RDD_WAN_BRIDGE_PORT; bridge_port <= BL_LILAC_RDD_PCI_BRIDGE_PORT; bridge_port++ )
    {
        g_local_switching_filters_mode[ bridge_port ] = BL_LILAC_RDD_FILTER_DISABLE; 
    }

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE f_rdd_vlan_matrix_initialize ( void )
{
    RDD_DS_VLAN_OPTIMIZATION_TABLE_DTS  *ds_vlan_optimization_table_ptr;
    RDD_US_VLAN_OPTIMIZATION_TABLE_DTS  *us_vlan_optimization_table_ptr;
    RDD_VLAN_OPTIMIZATION_ENTRY_DTS     *vlan_optimization_entry_ptr;
    RDD_DS_VLAN_COMMANDS_TABLE_DTS      *ds_vlan_commands_table_ptr;
    RDD_US_VLAN_COMMANDS_TABLE_DTS      *us_vlan_commands_table_ptr;
    RDD_VLAN_COMMAND_ENRTY_DTS          *vlan_command_entry_ptr;
    RDD_VLAN_ACTION_ENTRY_DTS           *vlan_action_entry_ptr;
    uint32_t                            vlan_action_counter;
    uint32_t                            vlan_command_id;
    uint32_t                            pbits_command_id;
    uint32_t                            tag_state;
    uint32_t                            action_exist;
    uint32_t                            runner_id;

    /* initialize vlan optimization table */
    ds_vlan_optimization_table_ptr = RDD_DS_VLAN_OPTIMIZATION_TABLE_PTR();

    for ( vlan_command_id = 0; vlan_command_id < RDD_DS_VLAN_OPTIMIZATION_TABLE_SIZE; vlan_command_id++ )
    {
        vlan_optimization_entry_ptr = &( ds_vlan_optimization_table_ptr->entry[ vlan_command_id ] );

        RDD_VLAN_OPTIMIZATION_ENTRY_OPTIMIZE_ENABLE_WRITE ( ( LILAC_RDD_TRUE << 7 ), vlan_optimization_entry_ptr );
    }

    us_vlan_optimization_table_ptr = RDD_US_VLAN_OPTIMIZATION_TABLE_PTR();

    for ( vlan_command_id = 0; vlan_command_id < RDD_US_VLAN_OPTIMIZATION_TABLE_SIZE; vlan_command_id++ )
    {
        vlan_optimization_entry_ptr = &( us_vlan_optimization_table_ptr->entry[ vlan_command_id ] );

        RDD_VLAN_OPTIMIZATION_ENTRY_OPTIMIZE_ENABLE_WRITE ( ( LILAC_RDD_TRUE << 7 ), vlan_optimization_entry_ptr );
    }

    /* initialize vlan action matrix */
    g_vlan_actions_matrix_ptr = ( BL_LILAC_RDD_VLAN_ACTIONS_MATRIX_DTS * )malloc( sizeof ( BL_LILAC_RDD_VLAN_ACTIONS_MATRIX_DTS ) );

    if ( g_vlan_actions_matrix_ptr == NULL)
    {
        return ( BL_LILAC_RDD_ERROR_MALLOC_FAILED );
    }

    memset ( g_vlan_actions_matrix_ptr, 0, sizeof ( BL_LILAC_RDD_VLAN_ACTIONS_MATRIX_DTS ) );

    /* Untagged Matrix */
    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_UNTAGGED]
                                                               [rdd_vlan_command_transparent]
                                                               [rdd_pbits_command_copy] );
    vlan_action_entry_ptr->vlan_action = 1;
    vlan_action_entry_ptr->pbits_action = 1;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_UNTAGGED]
                                                               [rdd_vlan_command_add_tag]
                                                               [rdd_pbits_command_copy] );
    vlan_action_entry_ptr->vlan_action = 1;
    vlan_action_entry_ptr->pbits_action = 1;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_UNTAGGED]
                                                               [rdd_vlan_command_add_tag]
                                                               [rdd_pbits_command_configured] );
    vlan_action_entry_ptr->vlan_action = 1;
    vlan_action_entry_ptr->pbits_action = 5;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_UNTAGGED]
                                                               [rdd_vlan_command_add_tag]
                                                               [rdd_pbits_command_remap] );
    vlan_action_entry_ptr->vlan_action = 1;
    vlan_action_entry_ptr->pbits_action = 5;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_UNTAGGED]
                                                               [rdd_vlan_command_replace_tag]
                                                               [rdd_pbits_command_transparent] );
    vlan_action_entry_ptr->vlan_action = 1;
    vlan_action_entry_ptr->pbits_action = 1;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_UNTAGGED]
                                                               [rdd_vlan_command_replace_tag]
                                                               [rdd_pbits_command_configured] );
    vlan_action_entry_ptr->vlan_action = 1;
    vlan_action_entry_ptr->pbits_action = 5;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_UNTAGGED]
                                                               [rdd_vlan_command_replace_tag]
                                                               [rdd_pbits_command_copy] );
    vlan_action_entry_ptr->vlan_action = 1;
    vlan_action_entry_ptr->pbits_action = 1;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_UNTAGGED]
                                                               [rdd_vlan_command_replace_tag]
                                                               [rdd_pbits_command_remap] );
    vlan_action_entry_ptr->vlan_action = 1;
    vlan_action_entry_ptr->pbits_action = 5;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_UNTAGGED]
                                                               [rdd_vlan_command_add_two_tags]
                                                               [rdd_pbits_command_copy] );
    vlan_action_entry_ptr->vlan_action = 5;
    vlan_action_entry_ptr->pbits_action = 7;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_UNTAGGED]
                                                               [rdd_vlan_command_add_two_tags]
                                                               [rdd_pbits_command_configured] );
    vlan_action_entry_ptr->vlan_action = 5;
    vlan_action_entry_ptr->pbits_action = 11;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_UNTAGGED]
                                                               [rdd_vlan_command_add_tag_always]
                                                               [rdd_pbits_command_configured] );
    vlan_action_entry_ptr->vlan_action = 11;
    vlan_action_entry_ptr->pbits_action = 5;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_UNTAGGED]
                                                               [rdd_vlan_command_add_3rd_tag]
                                                               [rdd_pbits_command_copy] );
    vlan_action_entry_ptr->vlan_action = 15;
    vlan_action_entry_ptr->pbits_action = 5;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_UNTAGGED]
                                                               [rdd_vlan_command_remove_tag_always]
                                                               [rdd_pbits_command_transparent] );
    vlan_action_entry_ptr->vlan_action = 4;
    vlan_action_entry_ptr->pbits_action = 6;


    /* Priority Tagged Matrix */
    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_PRIORITY]
                                                               [rdd_vlan_command_transparent]
                                                               [rdd_pbits_command_copy] );
    vlan_action_entry_ptr->vlan_action = 3;
    vlan_action_entry_ptr->pbits_action = 1;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_PRIORITY]
                                                               [rdd_vlan_command_transparent]
                                                               [rdd_pbits_command_remap] );
    vlan_action_entry_ptr->vlan_action = 3;
    vlan_action_entry_ptr->pbits_action = 2;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_PRIORITY]
                                                               [rdd_vlan_command_add_tag]
                                                               [rdd_pbits_command_copy] );
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 6;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_PRIORITY]
                                                               [rdd_vlan_command_add_tag]
                                                               [rdd_pbits_command_configured] );
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 5;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_PRIORITY]
                                                               [rdd_vlan_command_add_tag]
                                                               [rdd_pbits_command_remap] );
    vlan_action_entry_ptr->vlan_action = 2;

    vlan_action_entry_ptr->pbits_action = 2;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_PRIORITY]
                                                               [rdd_vlan_command_replace_tag]
                                                               [rdd_pbits_command_transparent] );
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 6;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_PRIORITY]
                                                               [rdd_vlan_command_replace_tag]
                                                               [rdd_pbits_command_configured] );
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 5;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_PRIORITY]
                                                               [rdd_vlan_command_replace_tag]
                                                               [rdd_pbits_command_copy] );
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 1;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_PRIORITY]
                                                               [rdd_vlan_command_replace_tag]
                                                               [rdd_pbits_command_remap] );
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 2;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_PRIORITY]
                                                               [rdd_vlan_command_add_two_tags]
                                                               [rdd_pbits_command_copy] );
    vlan_action_entry_ptr->vlan_action = 6;
    vlan_action_entry_ptr->pbits_action = 10;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_PRIORITY]
                                                               [rdd_vlan_command_add_two_tags]
                                                               [rdd_pbits_command_configured] );
    vlan_action_entry_ptr->vlan_action = 6;
    vlan_action_entry_ptr->pbits_action = 11;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_PRIORITY]
                                                               [rdd_vlan_command_remove_tag]
                                                               [rdd_pbits_command_transparent] );
    vlan_action_entry_ptr->vlan_action = 14;
    vlan_action_entry_ptr->pbits_action = 6;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_PRIORITY]
                                                               [rdd_vlan_command_remove_two_tags]
                                                               [rdd_pbits_command_transparent] );
    vlan_action_entry_ptr->vlan_action = 14;
    vlan_action_entry_ptr->pbits_action = 6;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_PRIORITY]
                                                               [rdd_vlan_command_add_outer_tag_replace_inner_tag]
                                                               [rdd_pbits_command_copy] );
    vlan_action_entry_ptr->vlan_action = 6;
    vlan_action_entry_ptr->pbits_action = 10;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_PRIORITY]
                                                               [rdd_vlan_command_add_outer_tag_replace_inner_tag]
                                                               [rdd_pbits_command_configured] );
    vlan_action_entry_ptr->vlan_action = 6;
    vlan_action_entry_ptr->pbits_action = 11;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_PRIORITY]
                                                               [rdd_vlan_command_add_outer_tag_replace_inner_tag]
                                                               [rdd_pbits_command_remap] );
    vlan_action_entry_ptr->vlan_action = 6;
    vlan_action_entry_ptr->pbits_action = 4;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_PRIORITY]
                                                               [rdd_vlan_command_add_3rd_tag]
                                                               [rdd_pbits_command_copy] );
    vlan_action_entry_ptr->vlan_action = 15;
    vlan_action_entry_ptr->pbits_action = 3;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_PRIORITY]
                                                               [rdd_vlan_command_transparent]
                                                               [rdd_pbits_command_configured] );
    vlan_action_entry_ptr->vlan_action = 3;
    vlan_action_entry_ptr->pbits_action = 5;


    /* use PBITS_REMAP entry in table, but action is PBITS transparent*/ 
    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_PRIORITY]
                                                               [rdd_vlan_command_transparent]
                                                               [rdd_pbits_command_remap] );
    vlan_action_entry_ptr->vlan_action = 3;
    vlan_action_entry_ptr->pbits_action = 6;


    /* Single Tagged Matrix */
    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_SINGLE]
                                                               [rdd_vlan_command_transparent]
                                                               [rdd_pbits_command_copy] );
    vlan_action_entry_ptr->vlan_action = 3;
    vlan_action_entry_ptr->pbits_action = 1;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_SINGLE]
                                                               [rdd_vlan_command_transparent]
                                                               [rdd_pbits_command_remap] );
    vlan_action_entry_ptr->vlan_action = 3;
    vlan_action_entry_ptr->pbits_action = 2;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_SINGLE]
                                                               [rdd_vlan_command_add_tag]
                                                               [rdd_pbits_command_copy] );
    vlan_action_entry_ptr->vlan_action = 1;
    vlan_action_entry_ptr->pbits_action = 3;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_SINGLE]
                                                               [rdd_vlan_command_add_tag]
                                                               [rdd_pbits_command_configured] );
    vlan_action_entry_ptr->vlan_action = 1;
    vlan_action_entry_ptr->pbits_action = 5;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_SINGLE]
                                                               [rdd_vlan_command_add_tag]
                                                               [rdd_pbits_command_remap] );
    vlan_action_entry_ptr->vlan_action = 1;
    vlan_action_entry_ptr->pbits_action = 9;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_SINGLE]
                                                               [rdd_vlan_command_replace_tag]
                                                               [rdd_pbits_command_transparent] );
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 6;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_SINGLE]
                                                               [rdd_vlan_command_replace_tag]
                                                               [rdd_pbits_command_configured] );
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 5;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_SINGLE]
                                                               [rdd_vlan_command_replace_tag]
                                                               [rdd_pbits_command_copy] );
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 1;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_SINGLE]
                                                               [rdd_vlan_command_replace_tag]
                                                               [rdd_pbits_command_remap] );
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 2;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_SINGLE]
                                                               [rdd_vlan_command_remove_tag]
                                                               [rdd_pbits_command_transparent] );
    vlan_action_entry_ptr->vlan_action = 14;
    vlan_action_entry_ptr->pbits_action = 6;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_SINGLE]
                                                               [rdd_vlan_command_remove_two_tags]
                                                               [rdd_pbits_command_transparent] );
    vlan_action_entry_ptr->vlan_action = 14;
    vlan_action_entry_ptr->pbits_action = 6;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_SINGLE]
                                                               [rdd_vlan_command_add_outer_tag_replace_inner_tag]
                                                               [rdd_pbits_command_copy] );
    vlan_action_entry_ptr->vlan_action = 6;
    vlan_action_entry_ptr->pbits_action = 10;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_SINGLE]
                                                               [rdd_vlan_command_add_outer_tag_replace_inner_tag]
                                                               [rdd_pbits_command_configured] );
    vlan_action_entry_ptr->vlan_action = 6;
    vlan_action_entry_ptr->pbits_action = 11;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_SINGLE]
                                                               [rdd_vlan_command_add_outer_tag_replace_inner_tag]
                                                               [rdd_pbits_command_remap] );
    vlan_action_entry_ptr->vlan_action = 6;
    vlan_action_entry_ptr->pbits_action = 4;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_SINGLE]
                                                               [rdd_vlan_command_add_3rd_tag]
                                                               [rdd_pbits_command_copy] );
    vlan_action_entry_ptr->vlan_action = 15;
    vlan_action_entry_ptr->pbits_action = 3;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_SINGLE]
                                                               [rdd_vlan_command_transparent]
                                                               [rdd_pbits_command_configured] );
    vlan_action_entry_ptr->vlan_action = 3;
    vlan_action_entry_ptr->pbits_action = 5;


    /* use PBITS_REMAP entry in table, but action is PBITS transparent*/ 
    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_SINGLE]
                                                               [rdd_vlan_command_transparent]
                                                               [rdd_pbits_command_remap] );
    vlan_action_entry_ptr->vlan_action = 3;
    vlan_action_entry_ptr->pbits_action = 6;


    /* Double Tagged Matrix */
    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_DOUBLE]
                                                               [rdd_vlan_command_transparent]
                                                               [rdd_pbits_command_remap] );
    vlan_action_entry_ptr->vlan_action = 3;
    vlan_action_entry_ptr->pbits_action = 2;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_DOUBLE]
                                                               [rdd_vlan_command_replace_tag]
                                                               [rdd_pbits_command_transparent] );
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 6;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_DOUBLE]
                                                               [rdd_vlan_command_replace_tag]
                                                               [rdd_pbits_command_configured] );
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 5;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_DOUBLE]
                                                               [rdd_vlan_command_replace_tag]
                                                               [rdd_pbits_command_copy] );
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 3;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_DOUBLE]
                                                               [rdd_vlan_command_replace_tag]
                                                               [rdd_pbits_command_remap] );
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 2;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_DOUBLE]
                                                               [rdd_vlan_command_remove_tag]
                                                               [rdd_pbits_command_transparent] );
    vlan_action_entry_ptr->vlan_action = 4;
    vlan_action_entry_ptr->pbits_action = 6;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_DOUBLE]
                                                               [rdd_vlan_command_remove_tag]
                                                               [rdd_pbits_command_remap] );
    vlan_action_entry_ptr->vlan_action = 4;
    vlan_action_entry_ptr->pbits_action = 2;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_DOUBLE]
                                                               [rdd_vlan_command_remove_two_tags]
                                                               [rdd_pbits_command_transparent] );
    vlan_action_entry_ptr->vlan_action = 8;
    vlan_action_entry_ptr->pbits_action = 6;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_DOUBLE]
                                                               [rdd_vlan_command_remove_outer_tag_replace_inner_tag]
                                                               [rdd_pbits_command_transparent] );
    vlan_action_entry_ptr->vlan_action = 9;
    vlan_action_entry_ptr->pbits_action = 6;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_DOUBLE]
                                                               [rdd_vlan_command_remove_outer_tag_replace_inner_tag]
                                                               [rdd_pbits_command_configured] );
    vlan_action_entry_ptr->vlan_action = 9;
    vlan_action_entry_ptr->pbits_action = 0;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_DOUBLE]
                                                               [rdd_vlan_command_remove_outer_tag_replace_inner_tag]
                                                               [rdd_pbits_command_copy] );
    vlan_action_entry_ptr->vlan_action = 10;
    vlan_action_entry_ptr->pbits_action = 6;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_DOUBLE]
                                                               [rdd_vlan_command_remove_outer_tag_replace_inner_tag]
                                                               [rdd_pbits_command_remap] );
    vlan_action_entry_ptr->vlan_action = 9;
    vlan_action_entry_ptr->pbits_action = 2;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_DOUBLE]
                                                               [rdd_vlan_command_replace_outer_tag_replace_inner_tag]
                                                               [rdd_pbits_command_transparent] );
    vlan_action_entry_ptr->vlan_action = 18;
    vlan_action_entry_ptr->pbits_action = 6;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_DOUBLE]
                                                               [rdd_vlan_command_replace_outer_tag_replace_inner_tag]
                                                               [rdd_pbits_command_configured] );
    vlan_action_entry_ptr->vlan_action = 18;
    vlan_action_entry_ptr->pbits_action = 11;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_DOUBLE]
                                                               [rdd_vlan_command_remove_outer_tag_copy]
                                                               [rdd_pbits_command_transparent] );
    vlan_action_entry_ptr->vlan_action = 20;
    vlan_action_entry_ptr->pbits_action = 6;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_DOUBLE]
                                                               [rdd_vlan_command_add_3rd_tag]
                                                               [rdd_pbits_command_copy] );
    vlan_action_entry_ptr->vlan_action = 15;
    vlan_action_entry_ptr->pbits_action = 3;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_DOUBLE]
                                                               [rdd_vlan_command_add_outer_tag_replace_inner_tag]
                                                               [rdd_pbits_command_copy] );
    vlan_action_entry_ptr->vlan_action = 6;
    vlan_action_entry_ptr->pbits_action = 10;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_DOUBLE]
                                                               [rdd_vlan_command_add_outer_tag_replace_inner_tag]
                                                               [rdd_pbits_command_configured] );
    vlan_action_entry_ptr->vlan_action = 6;
    vlan_action_entry_ptr->pbits_action = 11;


    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_DOUBLE]
                                                               [rdd_vlan_command_transparent]
                                                               [rdd_pbits_command_configured] );
    vlan_action_entry_ptr->vlan_action = 3;
    vlan_action_entry_ptr->pbits_action = 5;


    /* use PBITS_REMAP entry in table, but action is PBITS transparent*/ 
    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[LILAC_RDD_VLAN_TYPE_DOUBLE]
                                                               [rdd_vlan_command_transparent]
                                                               [rdd_pbits_command_remap] );
    vlan_action_entry_ptr->vlan_action = 3;
    vlan_action_entry_ptr->pbits_action = 6;


    vlan_action_counter = 1;

    for ( vlan_command_id = rdd_vlan_command_transparent; vlan_command_id < rdd_max_vlan_command; vlan_command_id++ )
    {
        for ( pbits_command_id = rdd_pbits_command_transparent; pbits_command_id < rdd_max_pbits_command; pbits_command_id++ )
        {
            action_exist = LILAC_RDD_FALSE;

            for ( tag_state = LILAC_RDD_VLAN_TYPE_UNTAGGED; tag_state <= LILAC_RDD_VLAN_TYPE_PRIORITY; tag_state++ )
            {
                if ( g_vlan_actions_matrix_ptr->entry[ tag_state ][ vlan_command_id ][ pbits_command_id ].vlan_action )
                {
                    action_exist = LILAC_RDD_TRUE;

                    break;
                }
            }

            if ( action_exist == LILAC_RDD_TRUE )
            {
                g_vlan_mapping_command_to_action[ vlan_command_id ][ pbits_command_id ] = vlan_action_counter;

                for ( runner_id = 0; runner_id <= 1; runner_id++ )
                {
                    if ( runner_id == 0 )
                    {
                        ds_vlan_commands_table_ptr = RDD_DS_VLAN_COMMANDS_TABLE_PTR();
                        vlan_command_entry_ptr = &( ds_vlan_commands_table_ptr->entry[ vlan_action_counter ] );
                    }
                    else
                    {
                        us_vlan_commands_table_ptr = RDD_US_VLAN_COMMANDS_TABLE_PTR();
                        vlan_command_entry_ptr = &( us_vlan_commands_table_ptr->entry[ vlan_action_counter ] );
                    }

                    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[ LILAC_RDD_VLAN_TYPE_UNTAGGED ][ vlan_command_id ][ pbits_command_id ] );

                    RDD_VLAN_COMMAND_ENRTY_VLAN_UNTAGGED_COMMAND_WRITE ( vlan_action_entry_ptr->vlan_action, vlan_command_entry_ptr );
                    RDD_VLAN_COMMAND_ENRTY_PBITS_UNTAGGED_COMMAND_WRITE ( vlan_action_entry_ptr->pbits_action, vlan_command_entry_ptr );

                    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[ LILAC_RDD_VLAN_TYPE_SINGLE ][ vlan_command_id ][ pbits_command_id ] );

                    RDD_VLAN_COMMAND_ENRTY_VLAN_SINGLE_TAGGED_COMMAND_WRITE ( vlan_action_entry_ptr->vlan_action, vlan_command_entry_ptr );
                    RDD_VLAN_COMMAND_ENRTY_PBITS_SINGLE_TAGGED_COMMAND_WRITE( vlan_action_entry_ptr->pbits_action, vlan_command_entry_ptr );

                    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[ LILAC_RDD_VLAN_TYPE_DOUBLE ][ vlan_command_id ][ pbits_command_id ] );

                    RDD_VLAN_COMMAND_ENRTY_VLAN_DOUBLE_TAGGED_COMMAND_WRITE( vlan_action_entry_ptr->vlan_action, vlan_command_entry_ptr );
                    RDD_VLAN_COMMAND_ENRTY_PBITS_DOUBLE_TAGGED_COMMAND_WRITE( vlan_action_entry_ptr->pbits_action, vlan_command_entry_ptr );

                    vlan_action_entry_ptr = &( g_vlan_actions_matrix_ptr->entry[ LILAC_RDD_VLAN_TYPE_PRIORITY ][ vlan_command_id ][ pbits_command_id ] );

                    RDD_VLAN_COMMAND_ENRTY_VLAN_PRIORITY_TAGGED_COMMAND_WRITE ( vlan_action_entry_ptr->vlan_action, vlan_command_entry_ptr );
                    RDD_VLAN_COMMAND_ENRTY_PBITS_PRIORITY_TAGGED_COMMAND_WRITE ( vlan_action_entry_ptr->pbits_action, vlan_command_entry_ptr );
                }

                vlan_action_counter++;
            }
            else
            {
                g_vlan_mapping_command_to_action[ vlan_command_id ][ pbits_command_id ] = 0;
            }
        }
    }

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE f_rdd_multicast_initialize ( void )
{
    RDD_MULTICAST_ACTIVE_PORTS_TABLE_DTS        *multicast_active_ports_table_ptr;
    RDD_MULTICAST_ACTIVE_PORTS_ENTRY_DTS        *multicast_active_ports_entry_ptr;
    RDD_DS_MULTICAST_VECTOR_TO_PORT_TABLE_DTS   *ds_multicast_vector_to_port_table_ptr;
    RDD_US_MULTICAST_VECTOR_TO_PORT_TABLE_DTS   *us_multicast_vector_to_port_table_ptr;
    RDD_MULTICAST_VECTOR_TO_PORT_ENTRY_DTS      *multicast_vector_to_port_entry_ptr;
    RDD_VLAN_COMMAND_INDEX_ENTRY_DTS            *vlan_cmd_idx_entry_ptr;
    RDD_DS_VLAN_OPTIMIZATION_TABLE_DTS          *ds_vlan_optimization_table_ptr;
    RDD_VLAN_OPTIMIZATION_ENTRY_DTS             *vlan_optimization_entry_ptr;
    uint32_t                                    active_ports_number;
    uint32_t                                    i, j;

    multicast_active_ports_table_ptr = RDD_MULTICAST_ACTIVE_PORTS_TABLE_PTR();

    for ( i = 1;
          i <= ( ( BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT |
                   BL_LILAC_RDD_MULTICAST_LAN1_BRIDGE_PORT |
                   BL_LILAC_RDD_MULTICAST_LAN2_BRIDGE_PORT |
                   BL_LILAC_RDD_MULTICAST_LAN3_BRIDGE_PORT |
                   BL_LILAC_RDD_MULTICAST_LAN4_BRIDGE_PORT |
                   BL_LILAC_RDD_MULTICAST_PCI_BRIDGE_PORT ) / BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT );
          i++ )
    {
        multicast_active_ports_entry_ptr = &( multicast_active_ports_table_ptr->entry[ i ] );

        active_ports_number = 0;

        for ( j = i; j; j >>= 1 )
        {
            if ( j & 1 )
            {
                active_ports_number++;
            }
        }

        RDD_MULTICAST_ACTIVE_PORTS_ENTRY_ACTIVE_PORTS_NUMBER_WRITE ( active_ports_number - 1, multicast_active_ports_entry_ptr );
    }


    /* convert multicast vector bits to bridge ports */
    ds_multicast_vector_to_port_table_ptr = RDD_DS_MULTICAST_VECTOR_TO_PORT_TABLE_PTR();

    multicast_vector_to_port_entry_ptr = &( ds_multicast_vector_to_port_table_ptr->entry[ BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT >> 5 ] );

    RDD_MULTICAST_VECTOR_TO_PORT_ENTRY_BRIDGE_PORT_WRITE ( BL_LILAC_RDD_LAN0_BRIDGE_PORT, multicast_vector_to_port_entry_ptr );

    multicast_vector_to_port_entry_ptr++;

    RDD_MULTICAST_VECTOR_TO_PORT_ENTRY_BRIDGE_PORT_WRITE ( BL_LILAC_RDD_LAN1_BRIDGE_PORT, multicast_vector_to_port_entry_ptr );

    multicast_vector_to_port_entry_ptr++;

    RDD_MULTICAST_VECTOR_TO_PORT_ENTRY_BRIDGE_PORT_WRITE ( BL_LILAC_RDD_LAN2_BRIDGE_PORT, multicast_vector_to_port_entry_ptr );

    multicast_vector_to_port_entry_ptr++;

    RDD_MULTICAST_VECTOR_TO_PORT_ENTRY_BRIDGE_PORT_WRITE ( BL_LILAC_RDD_LAN3_BRIDGE_PORT, multicast_vector_to_port_entry_ptr );

    multicast_vector_to_port_entry_ptr++;

    RDD_MULTICAST_VECTOR_TO_PORT_ENTRY_BRIDGE_PORT_WRITE ( BL_LILAC_RDD_LAN4_BRIDGE_PORT, multicast_vector_to_port_entry_ptr );

    multicast_vector_to_port_entry_ptr++;

    RDD_MULTICAST_VECTOR_TO_PORT_ENTRY_BRIDGE_PORT_WRITE ( BL_LILAC_RDD_PCI_BRIDGE_PORT, multicast_vector_to_port_entry_ptr );

    us_multicast_vector_to_port_table_ptr = RDD_US_MULTICAST_VECTOR_TO_PORT_TABLE_PTR();

    multicast_vector_to_port_entry_ptr = &( us_multicast_vector_to_port_table_ptr->entry[ BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT >> 5 ] );

    RDD_MULTICAST_VECTOR_TO_PORT_ENTRY_BRIDGE_PORT_WRITE ( BL_LILAC_RDD_LAN0_BRIDGE_PORT, multicast_vector_to_port_entry_ptr );

    multicast_vector_to_port_entry_ptr++;

    RDD_MULTICAST_VECTOR_TO_PORT_ENTRY_BRIDGE_PORT_WRITE ( BL_LILAC_RDD_LAN1_BRIDGE_PORT, multicast_vector_to_port_entry_ptr );

    multicast_vector_to_port_entry_ptr++;

    RDD_MULTICAST_VECTOR_TO_PORT_ENTRY_BRIDGE_PORT_WRITE ( BL_LILAC_RDD_LAN2_BRIDGE_PORT, multicast_vector_to_port_entry_ptr );

    multicast_vector_to_port_entry_ptr++;

    RDD_MULTICAST_VECTOR_TO_PORT_ENTRY_BRIDGE_PORT_WRITE ( BL_LILAC_RDD_LAN3_BRIDGE_PORT, multicast_vector_to_port_entry_ptr );

    multicast_vector_to_port_entry_ptr++;

    RDD_MULTICAST_VECTOR_TO_PORT_ENTRY_BRIDGE_PORT_WRITE ( BL_LILAC_RDD_LAN4_BRIDGE_PORT, multicast_vector_to_port_entry_ptr );

    multicast_vector_to_port_entry_ptr++;

    RDD_MULTICAST_VECTOR_TO_PORT_ENTRY_BRIDGE_PORT_WRITE ( BL_LILAC_RDD_PCI_BRIDGE_PORT, multicast_vector_to_port_entry_ptr );


    vlan_cmd_idx_entry_ptr = ( RDD_VLAN_COMMAND_INDEX_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + MULTICAST_DUMMY_VLAN_INDEXES_TABLE_ADDRESS );

    RDD_VLAN_COMMAND_INDEX_ENTRY_ETH0_VLAN_COMMAND_ID_WRITE ( 127, vlan_cmd_idx_entry_ptr );
    RDD_VLAN_COMMAND_INDEX_ENTRY_ETH1_VLAN_COMMAND_ID_WRITE ( 127, vlan_cmd_idx_entry_ptr );
    RDD_VLAN_COMMAND_INDEX_ENTRY_ETH2_VLAN_COMMAND_ID_WRITE ( 127, vlan_cmd_idx_entry_ptr );
    RDD_VLAN_COMMAND_INDEX_ENTRY_ETH3_VLAN_COMMAND_ID_WRITE ( 127, vlan_cmd_idx_entry_ptr );
    RDD_VLAN_COMMAND_INDEX_ENTRY_ETH4_VLAN_COMMAND_ID_WRITE ( 127, vlan_cmd_idx_entry_ptr );

    ds_vlan_optimization_table_ptr = RDD_DS_VLAN_OPTIMIZATION_TABLE_PTR();

    vlan_optimization_entry_ptr = &( ds_vlan_optimization_table_ptr->entry[ 127 ] );

    RDD_VLAN_OPTIMIZATION_ENTRY_OPTIMIZE_ENABLE_WRITE ( ( LILAC_RDD_TRUE << 7 ), vlan_optimization_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE f_rdd_vid_cam_initialize ( void )
{
    RDD_DS_LAN_VID_TABLE_DTS       *ds_vid_table_ptr;
    RDD_US_LAN_VID_TABLE_DTS       *us_vid_table_ptr;
    RDD_VID_ENTRY_DTS              *vid_entry_ptr;
    RDD_LAN_VID_CONTEXT_TABLE_DTS  *vid_context_table_ptr;
    RDD_LAN_VID_CONTEXT_ENTRY_DTS  *vid_context_entry_ptr;
    uint32_t                       entry_index;

    ds_vid_table_ptr = ( RDD_DS_LAN_VID_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_LAN_VID_TABLE_ADDRESS );

    for ( entry_index = 0; entry_index < RDD_DS_LAN_VID_TABLE_SIZE; entry_index++ )
    {
        vid_entry_ptr = &( ds_vid_table_ptr->entry[ entry_index ] );

        RDD_VID_ENTRY_VID_WRITE ( LILAC_RDD_LAN_VID_STOP_VALUE, vid_entry_ptr );
    }

    us_vid_table_ptr = ( RDD_US_LAN_VID_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_LAN_VID_TABLE_ADDRESS );

    for ( entry_index = 0; entry_index < RDD_US_LAN_VID_TABLE_SIZE; entry_index++ )
    {
        vid_entry_ptr = &( us_vid_table_ptr->entry[ entry_index ] );

        RDD_VID_ENTRY_VID_WRITE ( LILAC_RDD_LAN_VID_STOP_VALUE, vid_entry_ptr );
    }


    vid_context_table_ptr = ( RDD_LAN_VID_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + LAN_VID_CONTEXT_TABLE_ADDRESS );

    for ( entry_index = 0; entry_index < RDD_DS_LAN_VID_TABLE_SIZE; entry_index++ )
    {
        vid_context_entry_ptr = &( vid_context_table_ptr->entry[ entry_index ] );

        RDD_LAN_VID_CONTEXT_ENTRY_ISOLATION_MODE_PORT_VECTOR_WRITE ( BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN0 |
                                                             BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN1 |
                                                             BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN2 |
                                                             BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN3 |
                                                             BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN4 |
                                                             BL_LILAC_RDD_BRIDGE_PORT_VECTOR_PCI,
                                                             vid_context_entry_ptr );
        RDD_LAN_VID_CONTEXT_ENTRY_AGGREGATED_VID_IDX_WRITE ( 0, vid_context_entry_ptr );
        RDD_LAN_VID_CONTEXT_ENTRY_AGGREGATION_MODE_PORT_VECTOR_WRITE ( 0, vid_context_entry_ptr );
    }

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_iptv_classification_mode_config ( rdpa_iptv_lookup_method  xi_iptv_mode )
{
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS  *bridge_cfg_register;

    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );

    RDD_BRIDGE_CONFIGURATION_REGISTER_IPTV_CLASSIFICATION_METHOD_WRITE ( xi_iptv_mode, bridge_cfg_register );

    if ( ( xi_iptv_mode == iptv_lookup_method_group_ip_src_ip ) || ( xi_iptv_mode == iptv_lookup_method_group_ip_src_ip_vid ) )
    {
        g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ].is_external_context = BL_LILAC_RDD_EXTERNAL_CONTEXT_DISABLE;
    }
    else
    {
        g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ].is_external_context = BL_LILAC_RDD_EXTERNAL_CONTEXT_ENABLE;
    }

    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_entry_from_cache ( uint32_t  xi_entry_index )
{
    RDD_IPTV_LOOKUP_TABLE_DTS                          *iptv_table_ptr;
    RDD_IPTV_L3_LOOKUP_ENTRY_DTS                       *iptv_entry_ptr;
    RDD_IPTV_CONTEXT_TABLE_DTS                         *iptv_forward_table_ptr;
    RDD_IPTV_CONTEXT_ENTRY_DTS                         *iptv_forward_entry_ptr;
    RDD_IPTV_COUNTERS_TABLE_DTS                        *iptv_counter_table_ptr;
    RDD_IPTV_COUNTER_ENTRY_DTS                         *iptv_counter_entry_ptr;
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS  *iptv_ingress_classification_context_table_ptr; 
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS  *iptv_ingress_classification_context_entry_ptr; 
    BL_LILAC_RDD_ERROR_DTE                             rdd_error;
    uint32_t                                           iptv_table_size;

    iptv_table_ptr = ( RDD_IPTV_LOOKUP_TABLE_DTS * )( g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ].hash_table_ptr );

    iptv_table_size =  RDD_IPTV_LOOKUP_TABLE_SIZE;

    iptv_forward_table_ptr = ( RDD_IPTV_CONTEXT_TABLE_DTS * )( g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ].context_table_ptr );

    iptv_ingress_classification_context_table_ptr = ( RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_ADDRESS );

    iptv_ingress_classification_context_entry_ptr = &( iptv_ingress_classification_context_table_ptr->entry[ xi_entry_index ] );

    if ( xi_entry_index >= RDD_IPTV_LOOKUP_TABLE_SIZE )
    {
        xi_entry_index -= RDD_IPTV_LOOKUP_TABLE_SIZE;

        iptv_table_ptr = ( RDD_IPTV_LOOKUP_TABLE_DTS * )( g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ].cam_table_ptr );

        iptv_table_size =  RDD_IPTV_LOOKUP_TABLE_CAM_SIZE;

        iptv_forward_table_ptr = ( RDD_IPTV_CONTEXT_TABLE_DTS * )( g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ].cam_context_table_ptr );
    }

    iptv_entry_ptr = &( iptv_table_ptr->entry[ xi_entry_index ].iptv_l3_lookup_entry );

    iptv_forward_entry_ptr = &( iptv_forward_table_ptr->entry[ xi_entry_index ] );

    MEMSET ( iptv_forward_entry_ptr, 0, sizeof ( RDD_IPTV_CONTEXT_ENTRY_DTS ) );
    MEMSET ( iptv_entry_ptr, 0, ( sizeof ( RDD_IPTV_L3_LOOKUP_ENTRY_DTS ) - 1 ) );

    rdd_error = rdd_write_control_bits ( ( RDD_64_BIT_TABLE_ENTRY_DTS * )iptv_table_ptr, iptv_table_size, xi_entry_index, BL_LILAC_RDD_REMOVE_ENTRY );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        return ( rdd_error );
    }

    iptv_counter_table_ptr = ( RDD_IPTV_COUNTERS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPTV_COUNTERS_TABLE_ADDRESS );

    iptv_counter_entry_ptr = &( iptv_counter_table_ptr->entry[ xi_entry_index ] );

    RDD_IPTV_COUNTER_ENTRY_COUNTER_WRITE ( 0, iptv_counter_entry_ptr );

    MEMSET( iptv_ingress_classification_context_entry_ptr, 0, sizeof ( RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS ) );

    return ( rdd_error );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_read_iptv_counter_cache ( uint32_t  xi_index,
                                                              uint16_t  *xo_counter_value )
{
#if !defined(FIRMWARE_INIT)
    RDD_IPTV_COUNTER_ENTRY_DTS  *iptv_mac_counter_ptr;
    BL_LILAC_RDD_ERROR_DTE      rdd_error;
    unsigned long               flags;

    f_rdd_lock_irq ( &int_lock_irq, &flags );

    rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_IPTV_MAC_COUNTER_GET, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, xi_index, 0, 0, BL_LILAC_RDD_WAIT );

    f_rdd_unlock_irq ( &int_lock_irq, flags );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        return ( rdd_error );
    }

    iptv_mac_counter_ptr = ( RDD_IPTV_COUNTER_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPTV_COUNTERS_BUFFER_ADDRESS );

    RDD_IPTV_COUNTER_ENTRY_COUNTER_READ ( *xo_counter_value, iptv_mac_counter_ptr );
#else
    *xo_counter_value = 0;
#endif	
    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l3_dst_ip_src_ip_existing_any_entry_to_cache ( RDD_IPTV_ENTRY_UNION  *xi_entry,
                                                                                            uint32_t              xi_cache_index )
{
    RDD_IPTV_LOOKUP_TABLE_DTS                          *iptv_table_ptr;
    RDD_IPTV_L3_LOOKUP_ENTRY_DTS                       *iptv_entry_ptr;
    RDD_IPTV_CONTEXT_TABLE_DTS                         *forward_table_ptr;
    RDD_IPTV_CONTEXT_ENTRY_DTS                         *forward_entry_ptr;
    RDD_IPTV_SSID_EXTENSION_TABLE_DTS                  *iptv_ssid_table_ptr;
    RDD_IPTV_SSID_EXTENSION_ENTRY_DTS                  *iptv_ssid_entry_ptr;
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS  *iptv_ingress_classification_context_table_ptr; 
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS  *iptv_ingress_classification_context_entry_ptr; 
    uint32_t                                           any, port_vector_info = 0;

    iptv_table_ptr = ( RDD_IPTV_LOOKUP_TABLE_DTS * )g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ].hash_table_ptr;

    forward_table_ptr = ( RDD_IPTV_CONTEXT_TABLE_DTS * )( g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ].context_table_ptr );

    iptv_ssid_table_ptr = ( RDD_IPTV_SSID_EXTENSION_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPTV_SSID_EXTENSION_TABLE_ADDRESS );
	
    iptv_ingress_classification_context_table_ptr = ( RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_ADDRESS );

    iptv_ingress_classification_context_entry_ptr = &( iptv_ingress_classification_context_table_ptr->entry[ xi_cache_index ] );

    if ( xi_cache_index >= RDD_IPTV_LOOKUP_TABLE_SIZE )
    {
        xi_cache_index -= RDD_IPTV_LOOKUP_TABLE_SIZE;

        iptv_table_ptr = ( RDD_IPTV_LOOKUP_TABLE_DTS * )g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ].cam_table_ptr;

        forward_table_ptr = ( RDD_IPTV_CONTEXT_TABLE_DTS * )( g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ].cam_context_table_ptr );

        iptv_ssid_table_ptr = ( RDD_IPTV_SSID_EXTENSION_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPTV_SSID_EXTENSION_TABLE_CAM_ADDRESS );
    }

    iptv_entry_ptr = &( iptv_table_ptr->entry[ xi_cache_index ].iptv_l3_lookup_entry );

    forward_entry_ptr = &( forward_table_ptr->entry[ xi_cache_index ] );

    iptv_ssid_entry_ptr = &( iptv_ssid_table_ptr->entry[ xi_cache_index ] );

    RDD_IPTV_L3_LOOKUP_ENTRY_ANY_READ ( any, iptv_entry_ptr );

    if ( any )
    {
        return ( BL_LILAC_RDD_ERROR_IPTV_WITH_SRC_IP_ANY_EXISTS );
    }

    RDD_IPTV_L3_LOOKUP_ENTRY_ANY_WRITE ( LILAC_RDD_TRUE, iptv_entry_ptr );

    RDD_IPTV_CONTEXT_ENTRY_REPLICATION_NUMBER_L_WRITE ( port_vector_info, f_rdd_extract_replication_number ( xi_entry->l3_entry_fields.egress_port_vector ) );
    RDD_IPTV_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_L_WRITE ( port_vector_info, xi_entry->l3_entry_fields.egress_port_vector );

    RDD_IPTV_CONTEXT_ENTRY_PORT_VECTOR_INFO_WRITE ( port_vector_info, forward_entry_ptr );

    RDD_IPTV_SSID_EXTENSION_ENTRY_WLAN_MCAST_INDEX_WRITE ( xi_entry->l3_entry_fields.wlan_mcast_index, iptv_ssid_entry_ptr );

    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CONTEXT_WRITE ( xi_entry->l3_entry_fields.ingress_classification_context, iptv_ingress_classification_context_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l3_dst_ip_src_ip_existing_any_entry_to_ddr ( RDD_IPTV_ENTRY_UNION  *xi_entry,
                                                                                          uint32_t              xi_base_index )
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS   *iptv_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS   *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_TABLE_DTS  *iptv_context_table_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS  *iptv_ssm_context_entry_ptr;
    uint32_t                        any, port_vector_info = 0;

    iptv_table_ptr = ( RDD_IPTV_DDR_LOOKUP_TABLE_DTS * )IPTVTableBase;

    iptv_entry_ptr = &( iptv_table_ptr->entry[ xi_base_index ] );

    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_ANY_READ ( any, iptv_entry_ptr );

    if ( any )
    {
        return ( BL_LILAC_RDD_ERROR_IPTV_WITH_SRC_IP_ANY_EXISTS );
    }

    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_ANY_WRITE ( LILAC_RDD_TRUE, iptv_entry_ptr );

    iptv_context_table_ptr = ( RDD_IPTV_DDR_CONTEXT_TABLE_DTS * )IPTVContextTableBase;

    /* in ssm mode the context of entries with src ip = any is written according to the "regular" context table according to the ssm mode context format */
    iptv_ssm_context_entry_ptr = &( iptv_context_table_ptr->entry[ xi_base_index ] );

    RDD_IPTV_DDR_CONTEXT_ENTRY_REPLICATION_NUMBER_L_WRITE ( port_vector_info, f_rdd_extract_replication_number ( xi_entry->l3_entry_fields.egress_port_vector ) );
    RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_L_WRITE ( port_vector_info, xi_entry->l3_entry_fields.egress_port_vector );
    RDD_IPTV_DDR_CONTEXT_ENTRY_PORT_VECTOR_INFO_WRITE ( port_vector_info, iptv_ssm_context_entry_ptr );

    RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_WRITE ( 0, iptv_ssm_context_entry_ptr );
    RDD_IPTV_DDR_CONTEXT_ENTRY_WLAN_MCAST_INDEX_WRITE ( xi_entry->l3_entry_fields.wlan_mcast_index, iptv_ssm_context_entry_ptr );
    RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_WRITE ( LILAC_RDD_TRUE, iptv_ssm_context_entry_ptr );
    RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_WRITE ( xi_entry->l3_entry_fields.ingress_classification_context, iptv_ssm_context_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l3_dst_ip_src_ip_existing_entry_to_cache ( uint32_t  xi_src_ip_index,
                                                                                        uint32_t  xi_cache_index )
{
    RDD_IPTV_LOOKUP_TABLE_DTS     *iptv_table_ptr;
    RDD_IPTV_L3_LOOKUP_ENTRY_DTS  *iptv_entry_ptr;

    iptv_table_ptr = ( RDD_IPTV_LOOKUP_TABLE_DTS * )g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ].hash_table_ptr;

    if ( xi_cache_index >= RDD_IPTV_LOOKUP_TABLE_SIZE )
    {
        xi_cache_index -= RDD_IPTV_LOOKUP_TABLE_SIZE;

        iptv_table_ptr = ( RDD_IPTV_LOOKUP_TABLE_DTS * )g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ].cam_table_ptr;
    }

    iptv_entry_ptr = &( iptv_table_ptr->entry[ xi_cache_index ].iptv_l3_lookup_entry );

    RDD_IPTV_L3_LOOKUP_ENTRY_CONTEXT_VALID_WRITE ( LILAC_RDD_TRUE, iptv_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l3_dst_ip_src_ip_existing_entry_to_ddr ( RDD_IPTV_ENTRY_UNION  *xi_entry,
                                                                                      uint32_t              xi_src_ip_index,
                                                                                      uint32_t              xi_base_index,
                                                                                      uint32_t              *xo_index )
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS       *iptv_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS       *iptv_entry_ptr;
    RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS  *iptv_ssm_context_table_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS      *iptv_ssm_context_entry_ptr;
    uint32_t                            context_table_index, port_vector_info = 0;

    iptv_table_ptr = ( RDD_IPTV_DDR_LOOKUP_TABLE_DTS * )IPTVTableBase;

    iptv_entry_ptr = &( iptv_table_ptr->entry[ xi_base_index ] );

    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_VALID_WRITE ( LILAC_RDD_TRUE, iptv_entry_ptr );
    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_TABLE_READ ( context_table_index, iptv_entry_ptr );

    iptv_ssm_context_table_ptr = ( RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS * )IPTVSsmContextTableBase;

    iptv_ssm_context_entry_ptr = &( iptv_ssm_context_table_ptr->entry[ ( ( context_table_index * LILAC_RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS ) + xi_src_ip_index ) ] );

    RDD_IPTV_DDR_CONTEXT_ENTRY_REPLICATION_NUMBER_L_WRITE ( port_vector_info, f_rdd_extract_replication_number ( xi_entry->l3_entry_fields.egress_port_vector ) );
    RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_L_WRITE ( port_vector_info, xi_entry->l3_entry_fields.egress_port_vector );
    RDD_IPTV_DDR_CONTEXT_ENTRY_PORT_VECTOR_INFO_WRITE ( port_vector_info, iptv_ssm_context_entry_ptr );

    RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_WRITE ( 0, iptv_ssm_context_entry_ptr );
    RDD_IPTV_DDR_CONTEXT_ENTRY_WLAN_MCAST_INDEX_WRITE ( xi_entry->l3_entry_fields.wlan_mcast_index, iptv_ssm_context_entry_ptr );
    RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_WRITE ( LILAC_RDD_TRUE, iptv_ssm_context_entry_ptr );
    RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_WRITE ( xi_entry->l3_entry_fields.ingress_classification_context, iptv_ssm_context_entry_ptr );
    *xo_index = GET_ENTRY_INDEX_FROM_BASE_AND_PROVIDER_INDICES ( xi_base_index, xi_src_ip_index );

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l3_dst_ip_src_ip_base_entry_to_cache ( RDD_IPTV_ENTRY_UNION  *xi_entry,
                                                                                    uint8_t               *xi_hash_entry,
                                                                                    uint32_t              *xo_cache_index,
                                                                                    uint32_t              *xo_context_index )
{
    RDD_IPTV_LOOKUP_TABLE_DTS     *iptv_table_ptr;
    RDD_IPTV_L3_LOOKUP_ENTRY_DTS  *iptv_entry_ptr;
    BL_LILAC_RDD_ERROR_DTE        rdd_error;
    uint8_t                       context_entry;
    uint32_t                      entry_index;

    /* allocate a new context table */
    *xo_context_index = g_iptv_context_tables_free_list_head;

    g_iptv_context_tables_free_list_head = g_iptv_context_tables_free_list->entry[ g_iptv_context_tables_free_list_head ];

    context_entry = 0;

    rdd_error = rdd_add_hash_entry_64_bit ( &g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ],
                                            xi_hash_entry,
                                            ( uint8_t * )&context_entry,
                                            IPTV_ENTRY_KEY_MASK_HIGH,
                                            IPTV_ENTRY_KEY_MASK_LOW,
                                            0,
                                            &entry_index );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        *xo_cache_index = ( RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1 );

        return ( BL_LILAC_RDD_ERROR_ADD_LOOKUP_ENTRY );
    }

    *xo_cache_index = entry_index;

    iptv_table_ptr = ( RDD_IPTV_LOOKUP_TABLE_DTS * )g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ].hash_table_ptr;

    if ( entry_index >= RDD_IPTV_LOOKUP_TABLE_SIZE )
    {
        entry_index -= RDD_IPTV_LOOKUP_TABLE_SIZE;

        iptv_table_ptr = ( RDD_IPTV_LOOKUP_TABLE_DTS * )g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ].cam_table_ptr;
    }

    iptv_entry_ptr = &( iptv_table_ptr->entry[ entry_index ].iptv_l3_lookup_entry );

    RDD_IPTV_L3_LOOKUP_ENTRY_CONTEXT_TABLE_WRITE ( *xo_context_index, iptv_entry_ptr );

    return ( rdd_error );
}


/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l3_dst_ip_src_ip_base_entry_to_ddr ( RDD_IPTV_ENTRY_UNION     *xi_entry,
                                                                                  uint8_t                  *xi_entry_bytes,
                                                                                  uint32_t                 xi_cache_index,
                                                                                  uint32_t                 xi_context_index,
                                                                                  rdpa_iptv_lookup_method  xi_iptv_mode,
                                                                                  bdmf_ip_t                *xi_ipv6_dst_ip_ptr,
                                                                                  uint32_t                 *xo_index )
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS   *iptv_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS   *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_TABLE_DTS  *iptv_context_table_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS  *iptv_context_entry_ptr;
    uint32_t                        crc_init_value;
    uint32_t                        crc_result; 
    uint32_t                        hash_index; 
    uint32_t                        tries;
    uint32_t                        iptv_entry_index;
    uint32_t                        iptv_entry_valid;
    uint32_t                        is_entry_cached, port_vector_info = 0;

    is_entry_cached = ( xi_cache_index < ( RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1 ) );

    crc_init_value = rdd_crc_init_value_get ( RDD_CRC_TYPE_32 );

    crc_result = rdd_crc_bit_by_bit ( &xi_entry_bytes[ 4 ], 12, 0, crc_init_value, RDD_CRC_TYPE_32 );

    hash_index = crc_result & ( RDD_IPTV_DDR_LOOKUP_TABLE_SIZE / LILAC_RDD_IPTV_TABLE_SET_SIZE - 1 );

    hash_index = hash_index * LILAC_RDD_IPTV_TABLE_SET_SIZE;

    iptv_table_ptr = ( RDD_IPTV_DDR_LOOKUP_TABLE_DTS * )IPTVTableBase;

    for ( tries = 0; tries < LILAC_RDD_IPTV_TABLE_SET_SIZE; tries++ )
    {
        iptv_entry_index = ( hash_index + tries ) & ( RDD_IPTV_DDR_LOOKUP_TABLE_SIZE - 1 );

        iptv_entry_ptr = &( iptv_table_ptr->entry[ iptv_entry_index ] );

        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_VALID_READ ( iptv_entry_valid, iptv_entry_ptr );

        if ( !( iptv_entry_valid ) )
        {
            break;
        }
    }

    if ( tries == LILAC_RDD_IPTV_TABLE_SET_SIZE )
    {
        if ( is_entry_cached )
        {
            f_rdd_delete_iptv_entry_from_cache ( xi_cache_index );
        }

        return ( BL_LILAC_RDD_ERROR_HASH_TABLE_NO_EMPTY_ENTRY );
    }

    if ( xi_ipv6_dst_ip_ptr != NULL ) 
    {
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP12_WRITE ( xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 12 ], iptv_entry_ptr );
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP13_WRITE ( xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 13 ], iptv_entry_ptr );
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP14_WRITE ( xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 14 ], iptv_entry_ptr );
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP15_WRITE ( xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 15 ], iptv_entry_ptr );
    }
    
    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_WRITE ( ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >> 24 ) & 0xFF, iptv_entry_ptr );
    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_WRITE ( ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >> 16 ) & 0xFF, iptv_entry_ptr );
    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_WRITE ( ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >>  8 ) & 0xFF, iptv_entry_ptr );
    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_WRITE ( ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >>  0 ) & 0xFF, iptv_entry_ptr );
    
    if ( xi_iptv_mode == iptv_lookup_method_group_ip_src_ip_vid )
    {
        RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_VID_WRITE( xi_entry->l3_entry_fields.vid, iptv_entry_ptr );
    }

    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_TABLE_WRITE ( xi_context_index , iptv_entry_ptr );

    iptv_context_table_ptr = ( RDD_IPTV_DDR_CONTEXT_TABLE_DTS * )IPTVContextTableBase;

    iptv_context_entry_ptr = &( iptv_context_table_ptr->entry[ iptv_entry_index ] );

    RDD_IPTV_DDR_CONTEXT_ENTRY_REPLICATION_NUMBER_L_WRITE ( port_vector_info, f_rdd_extract_replication_number ( xi_entry->l2_entry_fields.egress_port_vector ) );
    RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_L_WRITE ( port_vector_info, xi_entry->l2_entry_fields.egress_port_vector ); 
    RDD_IPTV_DDR_CONTEXT_ENTRY_PORT_VECTOR_INFO_WRITE ( port_vector_info, iptv_context_entry_ptr );

    RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_WRITE ( 0, iptv_context_entry_ptr );

    if ( is_entry_cached )
    {
        RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_WRITE ( LILAC_RDD_TRUE, iptv_context_entry_ptr );
        RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_WRITE ( xi_cache_index, iptv_context_entry_ptr );
    }

    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_VALID_WRITE ( LILAC_RDD_TRUE, iptv_entry_ptr );

    *xo_index = iptv_entry_index;

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l3_dst_ip_src_ip_existing_any_entry ( RDD_IPTV_ENTRY_UNION  *xi_entry,
                                                                                   uint32_t              xi_base_index,
                                                                                   uint32_t              xi_cache_index )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error = BL_LILAC_RDD_OK;

    if ( xi_cache_index < ( RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1 ) )
    {
        rdd_error = f_rdd_add_iptv_l3_dst_ip_src_ip_existing_any_entry_to_cache ( xi_entry, xi_cache_index );
    }

    if ( rdd_error == BL_LILAC_RDD_OK )
    {
        rdd_error = f_rdd_add_iptv_l3_dst_ip_src_ip_existing_any_entry_to_ddr ( xi_entry, xi_base_index );
    }

    return ( rdd_error );
}


/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l3_dst_ip_src_ip_existing_entry ( RDD_IPTV_ENTRY_UNION  *xi_entry,
                                                                               uint32_t              xi_base_index,
                                                                               uint32_t              xi_cache_index,
                                                                               uint32_t              xi_src_ip_index,
                                                                               uint32_t              *xo_index )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error = BL_LILAC_RDD_OK;

    /* update the cache */
    if ( xi_cache_index < ( RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1 ) )
    {
        rdd_error = f_rdd_add_iptv_l3_dst_ip_src_ip_existing_entry_to_cache ( xi_src_ip_index, xi_cache_index );
    }

    if ( rdd_error == BL_LILAC_RDD_OK )
    {
        rdd_error = f_rdd_add_iptv_l3_dst_ip_src_ip_existing_entry_to_ddr ( xi_entry, xi_src_ip_index, xi_base_index, xo_index );
    }

    return ( rdd_error );
}


/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l3_dst_ip_src_ip_first_any_entry ( RDD_IPTV_ENTRY_UNION     *xi_entry,
                                                                                rdpa_iptv_lookup_method  xi_iptv_mode,
                                                                                bdmf_ip_t                *xi_ipv6_dst_ip_ptr,
                                                                                uint32_t                 *xo_index,
                                                                                uint32_t                 *xo_cache )
                                                                                         
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint8_t                 entry_bytes[ LILAC_RDD_IPTV_ENTRY_SIZE ];
    uint8_t                 hash_entry[ 8 ];
    uint32_t                cache_index;
    uint32_t                context_index;

    if ( g_iptv_context_tables_free_list_head == LILAC_RDD_IPTV_SSM_CONTEXT_ENTRY_COUNT )
    {
        return ( BL_LILAC_RDD_ERROR_IPTV_CONTEXT_TABLES_TABLE_FULL );
    }

    if ( xi_ipv6_dst_ip_ptr == NULL ) 
    {
        MEMSET ( hash_entry, 0, 8 );

        if ( xi_iptv_mode == iptv_lookup_method_group_ip_src_ip_vid )
        {
            hash_entry[ 2 ] = xi_entry->l3_entry_fields.vid >> 8;
            hash_entry[ 3 ] = xi_entry->l3_entry_fields.vid & 0xFF;
        }

        hash_entry[ 4 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >> 24 ) & 0xFF;
        hash_entry[ 5 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >> 16 ) & 0xFF;
        hash_entry[ 6 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >>  8 ) & 0xFF;
        hash_entry[ 7 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >>  0 ) & 0xFF;

        rdd_error = f_rdd_add_iptv_l3_dst_ip_src_ip_base_entry_to_cache ( xi_entry, hash_entry, &cache_index, &context_index );

        *xo_cache = ( rdd_error == BL_LILAC_RDD_OK );
    }
    else
    {
        /* allocate a new context table */
        context_index = g_iptv_context_tables_free_list_head;
        g_iptv_context_tables_free_list_head = g_iptv_context_tables_free_list->entry[ g_iptv_context_tables_free_list_head ];
        cache_index = RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1;
        *xo_cache = LILAC_RDD_FALSE;
    }

    MEMSET ( entry_bytes, 0, LILAC_RDD_IPTV_ENTRY_SIZE );

    if ( xi_ipv6_dst_ip_ptr != NULL ) 
    {
        entry_bytes[ 4 ] = xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 12 ];
        entry_bytes[ 5 ] = xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 13 ];
        entry_bytes[ 6 ] = xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 14 ];
        entry_bytes[ 7 ] = xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 15 ];
    }

    if ( xi_iptv_mode == iptv_lookup_method_group_ip_src_ip_vid )
    {
        entry_bytes[ 10 ] = xi_entry->l3_entry_fields.vid >> 8;
        entry_bytes[ 11 ] = xi_entry->l3_entry_fields.vid & 0xFF;
    }

    entry_bytes[ 12 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >> 24 ) & 0xFF;
    entry_bytes[ 13 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >> 16 ) & 0xFF;
    entry_bytes[ 14 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >>  8 ) & 0xFF;
    entry_bytes[ 15 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >>  0 ) & 0xFF;

    rdd_error = f_rdd_add_iptv_l3_dst_ip_src_ip_base_entry_to_ddr ( xi_entry, entry_bytes, cache_index, context_index, xi_iptv_mode, xi_ipv6_dst_ip_ptr, xo_index );

    if ( rdd_error == BL_LILAC_RDD_OK )
    {
        rdd_error = f_rdd_add_iptv_l3_dst_ip_src_ip_existing_any_entry ( xi_entry, *xo_index, cache_index );
    }
    else
    {
        f_rdd_free_iptv_l3_dst_ip_src_ip_context_table ( context_index );
    }

    return ( rdd_error );
}


/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l3_dst_ip_src_ip_first_entry ( RDD_IPTV_ENTRY_UNION     *xi_entry,
                                                                            rdpa_iptv_lookup_method  xi_iptv_mode,
                                                                            bdmf_ip_t                *xi_ipv6_dst_ip_ptr,
                                                                            bdmf_ip_t                *xi_ipv6_src_ip_ptr,
                                                                            uint32_t                 *xo_index,
                                                                            uint32_t                 *xo_cache )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint8_t                 entry_bytes[ LILAC_RDD_IPTV_ENTRY_SIZE ];
    uint8_t                 hash_entry[ 8 ];
    uint32_t                cache_index;
    uint32_t                base_index;
    uint32_t                src_ip_index;
    uint32_t                context_index;

    if ( g_iptv_context_tables_free_list_head == LILAC_RDD_IPTV_SSM_CONTEXT_ENTRY_COUNT )
    {
        return ( BL_LILAC_RDD_ERROR_IPTV_CONTEXT_TABLES_TABLE_FULL );
    }

    rdd_error = f_rdd_add_iptv_layer3_src_ip ( &( xi_entry->l3_entry_fields.src_ip ), xi_ipv6_src_ip_ptr, &src_ip_index );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        return ( rdd_error );
    }

    if ( xi_ipv6_dst_ip_ptr == NULL )
    {
        MEMSET ( hash_entry, 0, 8 );

        if ( xi_iptv_mode == iptv_lookup_method_group_ip_src_ip_vid )
        {
            hash_entry[ 2 ] = xi_entry->l3_entry_fields.vid >> 8;
            hash_entry[ 3 ] = xi_entry->l3_entry_fields.vid & 0xFF;
        }

        hash_entry[ 4 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >> 24 ) & 0xFF;
        hash_entry[ 5 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >> 16 ) & 0xFF;
        hash_entry[ 6 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >>  8 ) & 0xFF;
        hash_entry[ 7 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >>  0 ) & 0xFF;

        rdd_error = f_rdd_add_iptv_l3_dst_ip_src_ip_base_entry_to_cache ( xi_entry, hash_entry, &cache_index, &context_index );

        *xo_cache = ( rdd_error == BL_LILAC_RDD_OK );
    }
    else
    {
        /* allocate a new context table */
        context_index = g_iptv_context_tables_free_list_head;
        g_iptv_context_tables_free_list_head = g_iptv_context_tables_free_list->entry[ g_iptv_context_tables_free_list_head ];
        cache_index = RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1;
        *xo_cache = LILAC_RDD_FALSE;
    }

    MEMSET ( entry_bytes, 0, LILAC_RDD_IPTV_ENTRY_SIZE );

    if ( xi_ipv6_dst_ip_ptr != NULL ) 
    {
        entry_bytes[ 4 ] = xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 12 ];
        entry_bytes[ 5 ] = xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 13 ];
        entry_bytes[ 6 ] = xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 14 ];
        entry_bytes[ 7 ] = xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 15 ];
    }

    if ( xi_iptv_mode == iptv_lookup_method_group_ip_src_ip_vid )
    {
        entry_bytes[ 10 ] = xi_entry->l3_entry_fields.vid >> 8;
        entry_bytes[ 11 ] = xi_entry->l3_entry_fields.vid & 0xFF;
    }

    entry_bytes[ 12 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >> 24 ) & 0xFF;
    entry_bytes[ 13 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >> 16 ) & 0xFF;
    entry_bytes[ 14 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >>  8 ) & 0xFF;
    entry_bytes[ 15 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >>  0 ) & 0xFF;

    rdd_error = f_rdd_add_iptv_l3_dst_ip_src_ip_base_entry_to_ddr ( xi_entry, entry_bytes, cache_index, context_index, xi_iptv_mode, xi_ipv6_dst_ip_ptr, &base_index );

    if ( rdd_error == BL_LILAC_RDD_OK )
    {
        rdd_error = f_rdd_add_iptv_l3_dst_ip_src_ip_existing_entry ( xi_entry, base_index, cache_index, src_ip_index, xo_index );
    }
    else
    {
        f_rdd_delete_iptv_layer3_src_ip ( &( xi_entry->l3_entry_fields.src_ip ), &src_ip_index );
        f_rdd_free_iptv_l3_dst_ip_src_ip_context_table ( context_index );
    }

    return ( rdd_error );
}


/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_find_iptv_l3_dst_ip_src_ip_base_entry ( RDD_IPTV_ENTRY_UNION     *xi_entry,
                                                                            rdpa_iptv_lookup_method  xi_iptv_mode,
                                                                            bdmf_ip_t                *xi_ipv6_dst_ip_ptr,
                                                                            uint32_t                 *xo_base_index,
                                                                            uint32_t                 *xo_cache_index )
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS   *iptv_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS   *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_TABLE_DTS  *iptv_context_table_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS  *iptv_context_entry_ptr;
    uint32_t                        iptv_entry_index;
    uint32_t                        crc_init_value;
    uint32_t                        crc_result;
    uint32_t                        hash_index;
    uint32_t                        iptv_entry_valid;
    uint32_t                        tries;
    uint32_t                        is_entry_cached;
    uint8_t                         entry_bytes[ LILAC_RDD_IPTV_ENTRY_SIZE ];
    bdmf_ip_t                       dst_ip;
    uint32_t                        vid;
    uint8_t                         dst_ip_address[ 4 ];
    uint8_t                         ipv6_dst_ip_address[ 4 ];

    iptv_table_ptr = ( RDD_IPTV_DDR_LOOKUP_TABLE_DTS * )IPTVTableBase;

    crc_init_value = rdd_crc_init_value_get ( RDD_CRC_TYPE_32 );

    MEMSET ( entry_bytes, 0, LILAC_RDD_IPTV_ENTRY_SIZE );

    if ( xi_ipv6_dst_ip_ptr != NULL ) 
    {   
       entry_bytes[ 4 ] = xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 12 ];
       entry_bytes[ 5 ] = xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 13 ];
       entry_bytes[ 6 ] = xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 14 ];
       entry_bytes[ 7 ] = xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 15 ];
    }

    if ( xi_iptv_mode == iptv_lookup_method_group_ip_src_ip_vid )
    {
        entry_bytes[ 10 ] = xi_entry->l3_entry_fields.vid >> 8;
        entry_bytes[ 11 ] = xi_entry->l3_entry_fields.vid & 0xFF;
    }

    entry_bytes[ 12 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >> 24 ) & 0xFF;
    entry_bytes[ 13 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >> 16 ) & 0xFF;
    entry_bytes[ 14 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >>  8 ) & 0xFF;
    entry_bytes[ 15 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >>  0 ) & 0xFF;

    crc_result = rdd_crc_bit_by_bit ( &entry_bytes[ 4 ], 12, 0, crc_init_value, RDD_CRC_TYPE_32 );

    hash_index = crc_result & ( RDD_IPTV_DDR_LOOKUP_TABLE_SIZE / LILAC_RDD_IPTV_TABLE_SET_SIZE - 1 );

    hash_index = hash_index * LILAC_RDD_IPTV_TABLE_SET_SIZE;

    for ( tries = 0; tries < LILAC_RDD_IPTV_TABLE_SET_SIZE; tries++ )
    {
        iptv_entry_index = ( hash_index + tries ) & ( RDD_IPTV_DDR_LOOKUP_TABLE_SIZE - 1 );

        iptv_entry_ptr = &( iptv_table_ptr->entry[ iptv_entry_index ] );

        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_VALID_READ ( iptv_entry_valid, iptv_entry_ptr );

        if ( iptv_entry_valid )
        {
            if ( xi_iptv_mode == iptv_lookup_method_group_ip_src_ip )
            {
                if ( xi_ipv6_dst_ip_ptr != NULL )
                {
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP12_READ ( ipv6_dst_ip_address[ 0 ], iptv_entry_ptr );
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP13_READ ( ipv6_dst_ip_address[ 1 ], iptv_entry_ptr );
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP14_READ ( ipv6_dst_ip_address[ 2 ], iptv_entry_ptr );
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP15_READ ( ipv6_dst_ip_address[ 3 ], iptv_entry_ptr );
                }

                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_READ ( dst_ip_address[ 0 ], iptv_entry_ptr );
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_READ ( dst_ip_address[ 1 ], iptv_entry_ptr );
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_READ ( dst_ip_address[ 2 ], iptv_entry_ptr );
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_READ ( dst_ip_address[ 3 ], iptv_entry_ptr );

                dst_ip.addr.ipv4  = ( dst_ip_address[ 0 ] << 24 );
                dst_ip.addr.ipv4 |= ( dst_ip_address[ 1 ] << 16 );
                dst_ip.addr.ipv4 |= ( dst_ip_address[ 2 ] <<  8 );
                dst_ip.addr.ipv4 |= ( dst_ip_address[ 3 ] <<  0 );

                if ( dst_ip.addr.ipv4 == xi_entry->l3_entry_fields.dst_ip.addr.ipv4 )
                {
                    if ( xi_ipv6_dst_ip_ptr != NULL )
                    {
                        if ( ipv6_dst_ip_address[ 0 ] != xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 12 ] ||
                             ipv6_dst_ip_address[ 1 ] != xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 13 ] ||
                             ipv6_dst_ip_address[ 2 ] != xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 14 ] ||
                             ipv6_dst_ip_address[ 3 ] != xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 15 ] )
                        {
                            continue;
                        }
                    }

                    break;
                }
            }
            else
            {
                if ( xi_ipv6_dst_ip_ptr != NULL ) 
                {
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP12_READ ( ipv6_dst_ip_address[ 0 ], iptv_entry_ptr );
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP13_READ ( ipv6_dst_ip_address[ 1 ], iptv_entry_ptr );
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP14_READ ( ipv6_dst_ip_address[ 2 ], iptv_entry_ptr );
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP15_READ ( ipv6_dst_ip_address[ 3 ], iptv_entry_ptr );
                }

                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_READ ( dst_ip_address[ 0 ], iptv_entry_ptr );
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_READ ( dst_ip_address[ 1 ], iptv_entry_ptr );
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_READ ( dst_ip_address[ 2 ], iptv_entry_ptr );
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_READ ( dst_ip_address[ 3 ], iptv_entry_ptr );

                dst_ip.addr.ipv4  = ( dst_ip_address[ 0 ] << 24 );
                dst_ip.addr.ipv4 |= ( dst_ip_address[ 1 ] << 16 );
                dst_ip.addr.ipv4 |= ( dst_ip_address[ 2 ] <<  8 );
                dst_ip.addr.ipv4 |= ( dst_ip_address[ 3 ] <<  0 );

                RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_VID_READ ( vid, iptv_entry_ptr );

                if ( ( dst_ip.addr.ipv4 == xi_entry->l3_entry_fields.dst_ip.addr.ipv4 ) && ( vid ==  xi_entry->l3_entry_fields.vid ) )
                {
                    if ( xi_ipv6_dst_ip_ptr != NULL )
                    {
                        if ( ipv6_dst_ip_address[ 0 ] != xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 12 ] ||
                             ipv6_dst_ip_address[ 1 ] != xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 13 ] ||
                             ipv6_dst_ip_address[ 2 ] != xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 14 ] ||
                             ipv6_dst_ip_address[ 3 ] != xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 15 ] )
                        {
                            continue;
                        }
                    }

                    break;
                }
            }
        }
    }

    if ( tries == LILAC_RDD_IPTV_TABLE_SET_SIZE )
    {
        return ( BL_LILAC_RDD_ERROR_IPTV_TABLE_ENTRY_NOT_EXISTS );
    }

    *xo_base_index = iptv_entry_index;

    iptv_context_table_ptr = ( RDD_IPTV_DDR_CONTEXT_TABLE_DTS * )IPTVContextTableBase;
    iptv_context_entry_ptr = &( iptv_context_table_ptr->entry[ iptv_entry_index ] );

    RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_READ ( is_entry_cached, iptv_context_entry_ptr );

    if ( is_entry_cached )
    {
        RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_READ ( *xo_cache_index, iptv_context_entry_ptr );
    }
    else
    {
        *xo_cache_index = ( RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1 );
    }

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_entry_to_cache ( RDD_IPTV_ENTRY_UNION  *xi_entry,
                                                              uint8_t               *xi_hash_entry,
                                                              uint32_t              *xo_cache_index )
{
    RDD_IPTV_SSID_EXTENSION_TABLE_DTS                  *iptv_ssid_table_ptr;
    RDD_IPTV_SSID_EXTENSION_ENTRY_DTS                  *iptv_ssid_entry_ptr;
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS  *iptv_ingress_classification_context_table_ptr; 
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS  *iptv_ingress_classification_context_entry_ptr; 
    BL_LILAC_RDD_ERROR_DTE                             rdd_error;
#ifndef G9991
    uint8_t                                            context_entry[ 2 ];
#else
    uint8_t                                            context_entry[ 4 ];
#endif
    uint32_t                                           entry_index;

    context_entry[0] = f_rdd_extract_replication_number ( xi_entry->l2_entry_fields.egress_port_vector );
#ifndef G9991
    context_entry[1] = xi_entry->l2_entry_fields.egress_port_vector;
#else
    context_entry[1] = (uint8_t) ( ( xi_entry->l2_entry_fields.egress_port_vector & 0xFF0000 ) >> 16 );
    context_entry[2] = (uint8_t) ( ( xi_entry->l2_entry_fields.egress_port_vector & 0xFF00 ) >> 8 );
    context_entry[3] = (uint8_t) ( xi_entry->l2_entry_fields.egress_port_vector & 0x00FF );
#endif


    rdd_error = rdd_add_hash_entry_64_bit ( &g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ],
                                            xi_hash_entry,
                                            ( uint8_t * )&context_entry,
                                            IPTV_ENTRY_KEY_MASK_HIGH,
                                            IPTV_ENTRY_KEY_MASK_LOW,
                                            0,
                                            &entry_index );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        *xo_cache_index = ( RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1 );

        return ( BL_LILAC_RDD_ERROR_ADD_LOOKUP_ENTRY );
    }

    iptv_ingress_classification_context_table_ptr = ( RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_ADDRESS );

    iptv_ingress_classification_context_entry_ptr = &( iptv_ingress_classification_context_table_ptr->entry[ entry_index ] );

    *xo_cache_index = entry_index;

    iptv_ssid_table_ptr = ( RDD_IPTV_SSID_EXTENSION_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPTV_SSID_EXTENSION_TABLE_ADDRESS );

    if ( entry_index >= RDD_IPTV_LOOKUP_TABLE_SIZE )
    {
        entry_index -= RDD_IPTV_LOOKUP_TABLE_SIZE;

        iptv_ssid_table_ptr = ( RDD_IPTV_SSID_EXTENSION_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPTV_SSID_EXTENSION_TABLE_CAM_ADDRESS );
    }

    iptv_ssid_entry_ptr = &( iptv_ssid_table_ptr->entry[ entry_index ] );

    RDD_IPTV_SSID_EXTENSION_ENTRY_WLAN_MCAST_INDEX_WRITE ( xi_entry->l2_entry_fields.wlan_mcast_index, iptv_ssid_entry_ptr );
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CONTEXT_WRITE ( xi_entry->l2_entry_fields.ingress_classification_context, iptv_ingress_classification_context_entry_ptr );

    return ( rdd_error );
}


/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_entry_to_ddr ( RDD_IPTV_ENTRY_UNION     *xi_entry,
                                                            uint8_t                  *xi_entry_bytes,
                                                            rdpa_iptv_lookup_method  xi_iptv_classification_mode,
                                                            uint32_t                 xi_cache_index,
                                                            bdmf_ip_t                *xi_ipv6_dst_ip_ptr,
                                                            uint32_t                 *xo_index )
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS   *iptv_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS   *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_TABLE_DTS  *iptv_context_table_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS  *iptv_context_entry_ptr;
    BL_LILAC_RDD_ERROR_DTE          rdd_error;
    uint32_t                        crc_init_value;
    uint32_t                        crc_result; 
    uint32_t                        hash_index; 
    uint32_t                        tries;
    uint32_t                        iptv_entry_index;
    uint32_t                        iptv_entry_valid;
    uint32_t                        is_entry_cached, port_vector_info = 0;
    
    is_entry_cached = ( xi_cache_index < ( RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1 ) );

    rdd_error = BL_LILAC_RDD_OK;

    crc_init_value = rdd_crc_init_value_get ( RDD_CRC_TYPE_32 );

    crc_result = rdd_crc_bit_by_bit ( &xi_entry_bytes[ 4 ], 12, 0, crc_init_value, RDD_CRC_TYPE_32 );

    hash_index = crc_result & ( RDD_IPTV_DDR_LOOKUP_TABLE_SIZE / LILAC_RDD_IPTV_TABLE_SET_SIZE - 1 );

    hash_index = hash_index * LILAC_RDD_IPTV_TABLE_SET_SIZE;

    iptv_table_ptr = ( RDD_IPTV_DDR_LOOKUP_TABLE_DTS * )IPTVTableBase;

    for ( tries = 0; tries < LILAC_RDD_IPTV_TABLE_SET_SIZE; tries++ )
    {
        iptv_entry_index = ( hash_index + tries ) & ( RDD_IPTV_DDR_LOOKUP_TABLE_SIZE - 1 );

        iptv_entry_ptr = &( iptv_table_ptr->entry[ iptv_entry_index ] );

        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_VALID_READ ( iptv_entry_valid, iptv_entry_ptr );

        if ( !( iptv_entry_valid ) )
        {
            break;
        }
    }

    if ( tries == LILAC_RDD_IPTV_TABLE_SET_SIZE )
    {
        if ( is_entry_cached )
        {
            f_rdd_delete_iptv_entry_from_cache ( xi_cache_index );
        }

        return ( BL_LILAC_RDD_ERROR_HASH_TABLE_NO_EMPTY_ENTRY );
    }

    if ( xi_iptv_classification_mode == iptv_lookup_method_mac )
    {
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR5_WRITE ( xi_entry->l2_entry_fields.mac_addr.b[ 5 ], iptv_entry_ptr );
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR4_WRITE ( xi_entry->l2_entry_fields.mac_addr.b[ 4 ], iptv_entry_ptr );
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR3_WRITE ( xi_entry->l2_entry_fields.mac_addr.b[ 3 ], iptv_entry_ptr );
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR2_WRITE ( xi_entry->l2_entry_fields.mac_addr.b[ 2 ], iptv_entry_ptr );
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR1_WRITE ( xi_entry->l2_entry_fields.mac_addr.b[ 1 ], iptv_entry_ptr );
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR0_WRITE ( xi_entry->l2_entry_fields.mac_addr.b[ 0 ], iptv_entry_ptr );
    }
    else if ( xi_iptv_classification_mode == iptv_lookup_method_mac_vid )
    {
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR5_WRITE ( xi_entry->l2_entry_fields.mac_addr.b[ 5 ], iptv_entry_ptr );
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR4_WRITE ( xi_entry->l2_entry_fields.mac_addr.b[ 4 ], iptv_entry_ptr );
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR3_WRITE ( xi_entry->l2_entry_fields.mac_addr.b[ 3 ], iptv_entry_ptr );
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR2_WRITE ( xi_entry->l2_entry_fields.mac_addr.b[ 2 ], iptv_entry_ptr );
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR1_WRITE ( xi_entry->l2_entry_fields.mac_addr.b[ 1 ], iptv_entry_ptr );
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR0_WRITE ( xi_entry->l2_entry_fields.mac_addr.b[ 0 ], iptv_entry_ptr );
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_VID_WRITE ( xi_entry->l2_entry_fields.vid, iptv_entry_ptr );
    }
    else
    {
        if ( xi_ipv6_dst_ip_ptr != NULL )
        {
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP12_WRITE ( xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 12 ], iptv_entry_ptr );
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP13_WRITE ( xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 13 ], iptv_entry_ptr );
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP14_WRITE ( xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 14 ], iptv_entry_ptr );
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP15_WRITE ( xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 15 ], iptv_entry_ptr );
        }
        
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_WRITE ( ( ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >> 24 ) & 0xFF ), iptv_entry_ptr );
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_WRITE ( ( ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >> 16 ) & 0xFF ), iptv_entry_ptr );
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_WRITE ( ( ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >>  8 ) & 0xFF ), iptv_entry_ptr );
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_WRITE ( ( ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >>  0 ) & 0xFF ), iptv_entry_ptr );
    }

    iptv_context_table_ptr = ( RDD_IPTV_DDR_CONTEXT_TABLE_DTS * )IPTVContextTableBase;
    iptv_context_entry_ptr = &( iptv_context_table_ptr->entry[ iptv_entry_index ] );

    RDD_IPTV_DDR_CONTEXT_ENTRY_REPLICATION_NUMBER_L_WRITE ( port_vector_info, f_rdd_extract_replication_number ( xi_entry->l2_entry_fields.egress_port_vector ) );
    RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_L_WRITE ( port_vector_info, xi_entry->l2_entry_fields.egress_port_vector ); 
    RDD_IPTV_DDR_CONTEXT_ENTRY_PORT_VECTOR_INFO_WRITE ( port_vector_info, iptv_context_entry_ptr );

    RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_WRITE ( 0, iptv_context_entry_ptr );

    if ( is_entry_cached )
    {
        RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_WRITE ( LILAC_RDD_TRUE, iptv_context_entry_ptr );
        RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_WRITE ( xi_cache_index, iptv_context_entry_ptr );
    }

    RDD_IPTV_DDR_CONTEXT_ENTRY_WLAN_MCAST_INDEX_WRITE ( xi_entry->l2_entry_fields.wlan_mcast_index, iptv_context_entry_ptr );
    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_VALID_WRITE ( LILAC_RDD_TRUE, iptv_entry_ptr );

    if ( xi_iptv_classification_mode <= iptv_lookup_method_mac_vid )
    {
        RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_WRITE ( xi_entry->l2_entry_fields.ingress_classification_context, iptv_context_entry_ptr );
    }
    else
    {
        RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_WRITE ( xi_entry->l3_entry_fields.ingress_classification_context, iptv_context_entry_ptr );
    }

    *xo_index = iptv_entry_index;

    return ( rdd_error );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l2_mac_entry ( RDD_IPTV_ENTRY_UNION  *xi_entry,
                                                            uint32_t              *xo_index,
                                                            uint32_t              *xo_cache )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint8_t                 entry_bytes[ LILAC_RDD_IPTV_ENTRY_SIZE ];
    uint8_t                 hash_entry[ 8 ];
    uint32_t                cache_index;

    MEMSET ( hash_entry, 0, 8 );

    hash_entry[ 2 ] = xi_entry->l2_entry_fields.mac_addr.b[ 0 ];
    hash_entry[ 3 ] = xi_entry->l2_entry_fields.mac_addr.b[ 1 ];
    hash_entry[ 4 ] = xi_entry->l2_entry_fields.mac_addr.b[ 2 ];
    hash_entry[ 5 ] = xi_entry->l2_entry_fields.mac_addr.b[ 3 ];
    hash_entry[ 6 ] = xi_entry->l2_entry_fields.mac_addr.b[ 4 ];
    hash_entry[ 7 ] = xi_entry->l2_entry_fields.mac_addr.b[ 5 ];

    rdd_error = f_rdd_add_iptv_entry_to_cache ( xi_entry, hash_entry, &cache_index );

    *xo_cache = ( rdd_error == BL_LILAC_RDD_OK );

    MEMSET ( entry_bytes, 0, LILAC_RDD_IPTV_ENTRY_SIZE );

    entry_bytes[ 10 ] = xi_entry->l2_entry_fields.mac_addr.b[ 0 ];
    entry_bytes[ 11 ] = xi_entry->l2_entry_fields.mac_addr.b[ 1 ];
    entry_bytes[ 12 ] = xi_entry->l2_entry_fields.mac_addr.b[ 2 ];
    entry_bytes[ 13 ] = xi_entry->l2_entry_fields.mac_addr.b[ 3 ];
    entry_bytes[ 14 ] = xi_entry->l2_entry_fields.mac_addr.b[ 4 ];
    entry_bytes[ 15 ] = xi_entry->l2_entry_fields.mac_addr.b[ 5 ];

    rdd_error = f_rdd_add_iptv_entry_to_ddr ( xi_entry, entry_bytes, iptv_lookup_method_mac, cache_index, 0, xo_index );

    return ( rdd_error );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l2_mac_vid_entry ( RDD_IPTV_ENTRY_UNION *xi_entry,
                                                                uint32_t             *xo_index,
                                                                uint32_t             *xo_cache )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint8_t                 entry_bytes[ LILAC_RDD_IPTV_ENTRY_SIZE ];
    uint8_t                 hash_entry[ 8 ];
    uint32_t                cache_index;

    hash_entry[ 0 ] = ( ( xi_entry->l2_entry_fields.vid & 0xF00 ) >> 8 );
    hash_entry[ 1 ] = ( xi_entry->l2_entry_fields.vid & 0x0FF );
    hash_entry[ 2 ] = xi_entry->l2_entry_fields.mac_addr.b[ 0 ];
    hash_entry[ 3 ] = xi_entry->l2_entry_fields.mac_addr.b[ 1 ];
    hash_entry[ 4 ] = xi_entry->l2_entry_fields.mac_addr.b[ 2 ];
    hash_entry[ 5 ] = xi_entry->l2_entry_fields.mac_addr.b[ 3 ];
    hash_entry[ 6 ] = xi_entry->l2_entry_fields.mac_addr.b[ 4 ];
    hash_entry[ 7 ] = xi_entry->l2_entry_fields.mac_addr.b[ 5 ];

    rdd_error = f_rdd_add_iptv_entry_to_cache ( xi_entry, hash_entry, &cache_index );

    *xo_cache = ( rdd_error == BL_LILAC_RDD_OK );

    MEMSET ( entry_bytes, 0, LILAC_RDD_IPTV_ENTRY_SIZE );

    entry_bytes[ 8 ] = xi_entry->l2_entry_fields.vid >> 8;
    entry_bytes[ 9 ] = xi_entry->l2_entry_fields.vid & 0xFF;
    entry_bytes[ 10 ] = xi_entry->l2_entry_fields.mac_addr.b[ 0 ];
    entry_bytes[ 11 ] = xi_entry->l2_entry_fields.mac_addr.b[ 1 ];
    entry_bytes[ 12 ] = xi_entry->l2_entry_fields.mac_addr.b[ 2 ];
    entry_bytes[ 13 ] = xi_entry->l2_entry_fields.mac_addr.b[ 3 ];
    entry_bytes[ 14 ] = xi_entry->l2_entry_fields.mac_addr.b[ 4 ];
    entry_bytes[ 15 ] = xi_entry->l2_entry_fields.mac_addr.b[ 5 ];

    rdd_error = f_rdd_add_iptv_entry_to_ddr ( xi_entry, entry_bytes, iptv_lookup_method_mac_vid, cache_index, 0, xo_index );

    return ( rdd_error );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l3_dst_ip_src_ip_entry ( RDD_IPTV_ENTRY_UNION     *xi_entry,
                                                                      rdpa_iptv_lookup_method  xi_iptv_mode,
                                                                      bdmf_ip_t                *xi_ipv6_dst_ip_ptr,
                                                                      bdmf_ip_t                *xi_ipv6_src_ip_ptr,
                                                                      uint32_t                 *xo_index,
                                                                      uint32_t                 *xo_cache )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint32_t                base_index;
    uint32_t                cache_index;
    uint32_t                src_ip_index;

    rdd_error = f_rdd_find_iptv_l3_dst_ip_src_ip_base_entry ( xi_entry, xi_iptv_mode, xi_ipv6_dst_ip_ptr, &base_index, &cache_index );

    if ( rdd_error == BL_LILAC_RDD_OK )
    {
        if ( xi_entry->l3_entry_fields.src_ip.addr.ipv4 == 0 )
        {
            rdd_error = f_rdd_add_iptv_l3_dst_ip_src_ip_existing_any_entry ( xi_entry, base_index, cache_index );

            *xo_index = base_index;
            *xo_cache = ( cache_index < ( RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1 ) );
        }
        else
        {
            rdd_error = f_rdd_add_iptv_layer3_src_ip ( &( xi_entry->l3_entry_fields.src_ip ), xi_ipv6_src_ip_ptr, &src_ip_index );

            if ( rdd_error == BL_LILAC_RDD_OK )
            {
                rdd_error = f_rdd_add_iptv_l3_dst_ip_src_ip_existing_entry ( xi_entry, base_index, cache_index, src_ip_index, xo_index );

                if ( rdd_error != BL_LILAC_RDD_OK )
                {
                    f_rdd_delete_iptv_layer3_src_ip ( &( xi_entry->l3_entry_fields.src_ip ), &src_ip_index );
                }

                *xo_cache = ( cache_index < ( RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1 ) );
            }
        }
    }
    else
    {
        if ( xi_entry->l3_entry_fields.src_ip.addr.ipv4 == 0 )
        {
            rdd_error = f_rdd_add_iptv_l3_dst_ip_src_ip_first_any_entry ( xi_entry, xi_iptv_mode, xi_ipv6_dst_ip_ptr, xo_index, xo_cache );
        }
        else
        {
            rdd_error = f_rdd_add_iptv_l3_dst_ip_src_ip_first_entry ( xi_entry, xi_iptv_mode, xi_ipv6_dst_ip_ptr, xi_ipv6_src_ip_ptr, xo_index, xo_cache );
        }
    }

    return ( rdd_error );
}


/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_l3_dst_ip_entry ( RDD_IPTV_ENTRY_UNION  *xi_entry,
                                                               bdmf_ip_t             *xi_ipv6_dst_ip_ptr,
                                                               uint32_t              *xo_index,
                                                               uint32_t              *xo_cache )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint8_t                 entry_bytes[ LILAC_RDD_IPTV_ENTRY_SIZE ];
    uint8_t                 hash_entry[ 8 ];
    uint32_t                cache_index;

    if ( xi_ipv6_dst_ip_ptr == NULL )
    {
        MEMSET ( hash_entry, 0, 8 );

        hash_entry[ 4 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >> 24 ) & 0xFF;
        hash_entry[ 5 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >> 16 ) & 0xFF;
        hash_entry[ 6 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >>  8 ) & 0xFF;
        hash_entry[ 7 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >>  0 ) & 0xFF;

        rdd_error = f_rdd_add_iptv_entry_to_cache ( xi_entry, hash_entry, &cache_index );

        *xo_cache = ( rdd_error == BL_LILAC_RDD_OK );
    }
    else
    {
        cache_index = RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1;
        *xo_cache = LILAC_RDD_FALSE;
    }

    MEMSET ( entry_bytes, 0, LILAC_RDD_IPTV_ENTRY_SIZE );

    if ( xi_ipv6_dst_ip_ptr != NULL )
    {
        entry_bytes[ 4 ] = xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 12 ];
        entry_bytes[ 5 ] = xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 13 ];
        entry_bytes[ 6 ] = xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 14 ];
        entry_bytes[ 7 ] = xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 15 ];
    }

    entry_bytes[ 12 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >> 24 ) & 0xFF;
    entry_bytes[ 13 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >> 16 ) & 0xFF;
    entry_bytes[ 14 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >>  8 ) & 0xFF;
    entry_bytes[ 15 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >>  0 ) & 0xFF;

    rdd_error = f_rdd_add_iptv_entry_to_ddr ( xi_entry, entry_bytes, iptv_lookup_method_group_ip, cache_index, xi_ipv6_dst_ip_ptr, xo_index );

    return ( rdd_error );
}


BL_LILAC_RDD_ERROR_DTE rdd_iptv_entry_add ( RDD_IPTV_ENTRY_UNION  *xi_entry,
                                            uint32_t              *xo_index,
                                            uint32_t              *xo_cache )
{
    rdpa_iptv_lookup_method                iptv_classification_mode;
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS  *bridge_cfg_register;
    bdmf_ip_t                              ipv6_dst_ip_copy;
    bdmf_ip_t                              ipv6_src_ip_copy;
    bdmf_ip_t                              ipv6_src_ip_any;
#if !defined(FIRMWARE_INIT)
    uint32_t                               *ipv6_buffer_ptr;
    unsigned long                          flags;
#else
    uint32_t                               crc_result;
    uint32_t                               crc_init_value;
#endif
    BL_LILAC_RDD_ERROR_DTE                 rdd_error;

    rdd_error = BL_LILAC_RDD_OK;

    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );

    RDD_BRIDGE_CONFIGURATION_REGISTER_IPTV_CLASSIFICATION_METHOD_READ ( iptv_classification_mode, bridge_cfg_register );

    f_rdd_lock ( &int_lock );

#if !defined(FIRMWARE_INIT)

    if ( xi_entry->l3_entry_fields.dst_ip.family == bdmf_ip_family_ipv6 )
    {
        ipv6_buffer_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + HASH_BUFFER_ADDRESS );

        MWRITE_BLK_8( ipv6_buffer_ptr, xi_entry->l3_entry_fields.dst_ip.addr.ipv6.data, 16 );

        f_rdd_lock_irq ( &int_lock_irq, &flags );

        rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_IPV6_CRC_GET, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, 0, 0, 0, BL_LILAC_RDD_WAIT );

        f_rdd_unlock_irq ( &int_lock_irq, flags );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            f_rdd_unlock ( &int_lock );
            return ( rdd_error );
        }

        memcpy ( ipv6_dst_ip_copy.addr.ipv6.data, xi_entry->l3_entry_fields.dst_ip.addr.ipv6.data, 16 );

        xi_entry->l3_entry_fields.dst_ip.addr.ipv4 = *( volatile uint32_t * )ipv6_buffer_ptr;

        if ( ( iptv_classification_mode == iptv_lookup_method_group_ip_src_ip ) || ( iptv_classification_mode == iptv_lookup_method_group_ip_src_ip_vid ) )
        {
            MEMSET ( ipv6_src_ip_any.addr.ipv6.data, 0, 16 );

            if ( memcmp ( ipv6_src_ip_any.addr.ipv6.data, xi_entry->l3_entry_fields.src_ip.addr.ipv6.data, 16 ) )
            {
                memcpy ( ipv6_src_ip_copy.addr.ipv6.data, xi_entry->l3_entry_fields.src_ip.addr.ipv6.data, 16 );

                MWRITE_BLK_8( ipv6_buffer_ptr, xi_entry->l3_entry_fields.src_ip.addr.ipv6.data, 16 );

                f_rdd_lock_irq ( &int_lock_irq, &flags );

                rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_IPV6_CRC_GET, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, 0, 0, 0, BL_LILAC_RDD_WAIT );

                f_rdd_unlock_irq ( &int_lock_irq, flags );

                if ( rdd_error != BL_LILAC_RDD_OK )
                {
                    f_rdd_unlock ( &int_lock );
                    return ( rdd_error );
                }

                xi_entry->l3_entry_fields.src_ip.addr.ipv4 = *( volatile uint32_t * )ipv6_buffer_ptr;
            }
            else
            {
                MEMSET ( ipv6_src_ip_copy.addr.ipv6.data, 0, 16 );
                xi_entry->l3_entry_fields.src_ip.addr.ipv4 = 0;
            }
        }
    }
#else

    if ( xi_entry->l3_entry_fields.dst_ip.family == bdmf_ip_family_ipv6 )
    {
        memcpy ( ipv6_dst_ip_copy.addr.ipv6.data, xi_entry->l3_entry_fields.dst_ip.addr.ipv6.data, 16 );

        crc_init_value = rdd_crc_init_value_get ( RDD_CRC_TYPE_32 );
        crc_result = rdd_crc_bit_by_bit ( &(xi_entry->l3_entry_fields.dst_ip.addr.ipv6.data[0]), 16, 0, crc_init_value, RDD_CRC_TYPE_32 );
        xi_entry->l3_entry_fields.dst_ip.addr.ipv4 = crc_result;
        
        if ( ( iptv_classification_mode == iptv_lookup_method_group_ip_src_ip ) || ( iptv_classification_mode == iptv_lookup_method_group_ip_src_ip_vid ) )
        {
            MEMSET ( ipv6_src_ip_any.addr.ipv6.data, 0, 16 );

            if ( memcmp ( ipv6_src_ip_any.addr.ipv6.data, xi_entry->l3_entry_fields.src_ip.addr.ipv6.data, 16 ) )
            {
                memcpy ( ipv6_src_ip_copy.addr.ipv6.data, xi_entry->l3_entry_fields.src_ip.addr.ipv6.data, 16 );

                crc_init_value = rdd_crc_init_value_get ( RDD_CRC_TYPE_32 );
                crc_result = rdd_crc_bit_by_bit ( &(xi_entry->l3_entry_fields.src_ip.addr.ipv6.data[0]), 16, 0, crc_init_value, RDD_CRC_TYPE_32 );
                xi_entry->l3_entry_fields.src_ip.addr.ipv4 = crc_result;
            }
            else
            {
                MEMSET ( ipv6_src_ip_copy.addr.ipv6.data, 0, 16 );

                xi_entry->l3_entry_fields.src_ip.addr.ipv4 = 0;
            }
        }
    }

#endif
    switch ( iptv_classification_mode )
    {
    case iptv_lookup_method_mac:

         rdd_error = f_rdd_add_iptv_l2_mac_entry ( xi_entry, xo_index, xo_cache );
         break;

    case iptv_lookup_method_mac_vid:

         rdd_error = f_rdd_add_iptv_l2_mac_vid_entry ( xi_entry, xo_index, xo_cache );
         break;

    case iptv_lookup_method_group_ip_src_ip:

        if ( xi_entry->l3_entry_fields.dst_ip.family == bdmf_ip_family_ipv6 )
        {
            rdd_error = f_rdd_add_iptv_l3_dst_ip_src_ip_entry ( xi_entry, iptv_classification_mode, &ipv6_dst_ip_copy, &ipv6_src_ip_copy, xo_index, xo_cache );
        }
        else
        {
            rdd_error = f_rdd_add_iptv_l3_dst_ip_src_ip_entry ( xi_entry, iptv_classification_mode, 0, 0, xo_index, xo_cache );
        }
        break;

    case iptv_lookup_method_group_ip:

        if ( xi_entry->l3_entry_fields.dst_ip.family == bdmf_ip_family_ipv6 )
        {
            rdd_error = f_rdd_add_iptv_l3_dst_ip_entry ( xi_entry, &ipv6_dst_ip_copy, xo_index, xo_cache );
        }
        else
        {
            rdd_error = f_rdd_add_iptv_l3_dst_ip_entry ( xi_entry, 0, xo_index, xo_cache );
        }
        break;

     case iptv_lookup_method_group_ip_src_ip_vid:

        if ( xi_entry->l3_entry_fields.dst_ip.family == bdmf_ip_family_ipv6 )
        {
            rdd_error = f_rdd_add_iptv_l3_dst_ip_src_ip_entry ( xi_entry, iptv_classification_mode, &ipv6_dst_ip_copy, &ipv6_src_ip_copy, xo_index, xo_cache );
        }
        else
        {
            rdd_error = f_rdd_add_iptv_l3_dst_ip_src_ip_entry ( xi_entry, iptv_classification_mode, 0, 0, xo_index, xo_cache );
        }
        break;
    }

    f_rdd_unlock ( &int_lock );
    return ( rdd_error );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_modify_iptv_entry_in_cache ( uint32_t  xi_entry_index_in_ddr,
                                                                 uint32_t  xi_egress_port_vector,
														         uint16_t  xi_wifi_ssid_vector,
                                                                 uint8_t   xi_ingress_classification_context )
{
    RDD_IPTV_DDR_CONTEXT_TABLE_DTS                     *iptv_context_table_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS                     *iptv_context_entry_ptr;
    RDD_IPTV_CONTEXT_ENTRY_DTS                         *forward_entry_ptr;
    RDD_IPTV_CONTEXT_TABLE_DTS                         *forward_table_ptr;
    RDD_IPTV_SSID_EXTENSION_TABLE_DTS                  *iptv_ssid_table_ptr;
    RDD_IPTV_SSID_EXTENSION_ENTRY_DTS                  *iptv_ssid_entry_ptr;
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS  *iptv_ingress_classification_context_table_ptr; 
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS  *iptv_ingress_classification_context_entry_ptr; 
    BL_LILAC_RDD_ERROR_DTE                             rdd_error;
    uint32_t                                           entry_cached, entry_index_in_cache, port_vector_info = 0;

    rdd_error = BL_LILAC_RDD_OK;

    iptv_context_table_ptr = ( RDD_IPTV_DDR_CONTEXT_TABLE_DTS * )IPTVContextTableBase;

    iptv_context_entry_ptr = &( iptv_context_table_ptr->entry[ xi_entry_index_in_ddr ] );

    RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_READ ( entry_cached, iptv_context_entry_ptr );

    if ( !( entry_cached ) )
    {
        return ( BL_LILAC_RDD_OK );
    }

    RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_READ ( entry_index_in_cache, iptv_context_entry_ptr );

    forward_table_ptr = ( RDD_IPTV_CONTEXT_TABLE_DTS * )( g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ].context_table_ptr );

    iptv_ssid_table_ptr = ( RDD_IPTV_SSID_EXTENSION_TABLE_DTS * )( DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPTV_SSID_EXTENSION_TABLE_ADDRESS );

    iptv_ingress_classification_context_table_ptr = ( RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_ADDRESS );

    iptv_ingress_classification_context_entry_ptr = &( iptv_ingress_classification_context_table_ptr->entry[ entry_index_in_cache ] );

    if ( entry_index_in_cache >= g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ].hash_table_size )
    {
        entry_index_in_cache -= g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ].hash_table_size;

        forward_table_ptr = ( RDD_IPTV_CONTEXT_TABLE_DTS * )( g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ].cam_context_table_ptr );

        iptv_ssid_table_ptr = ( RDD_IPTV_SSID_EXTENSION_TABLE_DTS * )( DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPTV_SSID_EXTENSION_TABLE_CAM_ADDRESS );
    }

    forward_entry_ptr = &( forward_table_ptr->entry[ entry_index_in_cache ] );

    iptv_ssid_entry_ptr = &( iptv_ssid_table_ptr->entry[ entry_index_in_cache ] );

    RDD_IPTV_CONTEXT_ENTRY_REPLICATION_NUMBER_L_WRITE ( port_vector_info, f_rdd_extract_replication_number ( xi_egress_port_vector ) );
    RDD_IPTV_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_L_WRITE ( port_vector_info, xi_egress_port_vector );
    RDD_IPTV_CONTEXT_ENTRY_PORT_VECTOR_INFO_WRITE ( port_vector_info, forward_entry_ptr );

    RDD_IPTV_SSID_EXTENSION_ENTRY_WLAN_MCAST_INDEX_WRITE ( xi_wifi_ssid_vector, iptv_ssid_entry_ptr );
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CONTEXT_WRITE ( xi_ingress_classification_context, iptv_ingress_classification_context_entry_ptr );

    return ( rdd_error );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_modify_iptv_entry_in_ddr ( uint32_t                      xi_entry_index,
                                                               uint32_t                      xi_egress_port_vector,
														       uint16_t                      xi_wifi_ssid_vector,
                                                               uint8_t                       xi_ingress_classification_context,
                                                               rdpa_iptv_lookup_method       iptv_mode )
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS   *iptv_table_ptr;
    RDD_IPTV_DDR_CONTEXT_TABLE_DTS  *iptv_context_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS   *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS  *iptv_context_entry_ptr;
    uint32_t                        iptv_entry_valid, port_vector_info = 0;

    iptv_table_ptr = ( RDD_IPTV_DDR_LOOKUP_TABLE_DTS * )IPTVTableBase;
    iptv_entry_ptr = &( iptv_table_ptr->entry[ xi_entry_index ] );

    RDD_IPTV_L2_DDR_LOOKUP_ENTRY_VALID_READ ( iptv_entry_valid, iptv_entry_ptr );

    if ( !( iptv_entry_valid ) )
    {
        return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
    }

    iptv_context_table_ptr = ( RDD_IPTV_DDR_CONTEXT_TABLE_DTS * )IPTVContextTableBase;
    iptv_context_entry_ptr = &( iptv_context_table_ptr->entry[ xi_entry_index ] );

    RDD_IPTV_DDR_CONTEXT_ENTRY_REPLICATION_NUMBER_L_WRITE ( port_vector_info, f_rdd_extract_replication_number ( xi_egress_port_vector ) );
    RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_L_WRITE ( port_vector_info, xi_egress_port_vector );
    RDD_IPTV_DDR_CONTEXT_ENTRY_PORT_VECTOR_INFO_WRITE ( port_vector_info, iptv_context_entry_ptr );

    RDD_IPTV_DDR_CONTEXT_ENTRY_WLAN_MCAST_INDEX_WRITE ( xi_wifi_ssid_vector, iptv_context_entry_ptr );
    RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_WRITE ( xi_ingress_classification_context, iptv_context_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_modify_iptv_ssm_context_entry_in_ddr ( uint32_t  xi_entry_index,
                                                                           uint32_t  xi_egress_port_vector,
														                   uint16_t  xi_wifi_ssid_vector,
                                                                           uint8_t   xi_ingress_classification_context )
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS       *iptv_table_ptr;
    RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS  *iptv_ssm_context_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS       *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS      *iptv_context_entry_ptr;
    uint32_t                            iptv_entry_valid;
    uint32_t                            provider_index;
    uint32_t                            base_index;
    uint32_t                            context_table_index, port_vector_info = 0;

    provider_index = GET_PROVIDER_INDEX_FROM_ENTRY_INDEX ( xi_entry_index );

    base_index = GET_BASE_INDEX_FROM_ENTRY_INDEX ( xi_entry_index );

    iptv_table_ptr = ( RDD_IPTV_DDR_LOOKUP_TABLE_DTS * )IPTVTableBase;
    iptv_entry_ptr = &( iptv_table_ptr->entry[ base_index ] );

    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_VALID_READ ( iptv_entry_valid, iptv_entry_ptr );

    if ( !( iptv_entry_valid ) )
    {
        return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
    }

    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_TABLE_READ ( context_table_index, iptv_entry_ptr );

    iptv_ssm_context_table_ptr = ( RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS * )IPTVSsmContextTableBase;
    iptv_context_entry_ptr = &( iptv_ssm_context_table_ptr->entry[ context_table_index * LILAC_RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS + provider_index ] );

    RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_READ ( iptv_entry_valid, iptv_context_entry_ptr );

    if ( !( iptv_entry_valid ) )
    {
        return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
    }

    RDD_IPTV_DDR_CONTEXT_ENTRY_REPLICATION_NUMBER_L_WRITE ( port_vector_info, f_rdd_extract_replication_number ( xi_egress_port_vector ) );
    RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_L_WRITE ( port_vector_info, xi_egress_port_vector );
    RDD_IPTV_DDR_CONTEXT_ENTRY_PORT_VECTOR_INFO_WRITE ( port_vector_info, iptv_context_entry_ptr );

    RDD_IPTV_DDR_CONTEXT_ENTRY_WLAN_MCAST_INDEX_WRITE ( xi_wifi_ssid_vector, iptv_context_entry_ptr );
    RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_WRITE ( xi_ingress_classification_context, iptv_context_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_modify_iptv_l2_mac_entry ( uint32_t  xi_entry_index,
                                                               uint32_t  xi_egress_port_vector,
														       uint16_t  xi_wifi_ssid_vector,
                                                               uint8_t   xi_ingress_classification_context )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;

    rdd_error = f_rdd_modify_iptv_entry_in_ddr ( xi_entry_index, xi_egress_port_vector, xi_wifi_ssid_vector, xi_ingress_classification_context, iptv_lookup_method_mac );

    if ( rdd_error == BL_LILAC_RDD_OK )
    {
        rdd_error = f_rdd_modify_iptv_entry_in_cache ( xi_entry_index, xi_egress_port_vector, xi_wifi_ssid_vector, xi_ingress_classification_context );
    }

    return ( rdd_error );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_modify_iptv_l2_mac_vid_entry ( uint32_t  xi_entry_index,
                                                                   uint32_t  xi_egress_port_vector,
														           uint16_t  xi_wifi_ssid_vector,
                                                                   uint8_t   xi_ingress_classification_context )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;

    rdd_error = f_rdd_modify_iptv_entry_in_ddr ( xi_entry_index, xi_egress_port_vector, xi_wifi_ssid_vector, xi_ingress_classification_context, iptv_lookup_method_mac_vid );

    if ( rdd_error == BL_LILAC_RDD_OK )
    {
        rdd_error = f_rdd_modify_iptv_entry_in_cache ( xi_entry_index, xi_egress_port_vector, xi_wifi_ssid_vector, xi_ingress_classification_context );
    }

    return ( rdd_error );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_modify_iptv_l3_dst_ip_src_ip_entry ( uint32_t  xi_entry_index,
                                                                         uint32_t  xi_egress_port_vector,
														                 uint16_t  xi_wifi_ssid_vector,
                                                                         uint8_t   xi_ingress_classification_context )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;

    if ( xi_entry_index < RDD_IPTV_DDR_LOOKUP_TABLE_SIZE )
    {
        rdd_error = f_rdd_modify_iptv_entry_in_ddr ( xi_entry_index, xi_egress_port_vector, xi_wifi_ssid_vector, xi_ingress_classification_context, iptv_lookup_method_group_ip_src_ip );

        if ( rdd_error == BL_LILAC_RDD_OK )
        {
            rdd_error = f_rdd_modify_iptv_entry_in_cache ( xi_entry_index, xi_egress_port_vector, xi_wifi_ssid_vector, xi_ingress_classification_context );
        }
    }
    else
    {
        rdd_error = f_rdd_modify_iptv_ssm_context_entry_in_ddr ( xi_entry_index, xi_egress_port_vector, xi_wifi_ssid_vector, xi_ingress_classification_context );
    }

    return ( rdd_error );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_modify_iptv_l3_dst_ip_entry ( uint32_t  xi_entry_index,
                                                                  uint32_t  xi_egress_port_vector,
														          uint16_t  xi_wifi_ssid_vector,
                                                                  uint8_t   xi_ingress_classification_context )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;

    rdd_error = f_rdd_modify_iptv_entry_in_ddr ( xi_entry_index, xi_egress_port_vector, xi_wifi_ssid_vector, xi_ingress_classification_context, iptv_lookup_method_group_ip );

    if ( rdd_error == BL_LILAC_RDD_OK )
    {
        rdd_error = f_rdd_modify_iptv_entry_in_cache ( xi_entry_index, xi_egress_port_vector, xi_wifi_ssid_vector, xi_ingress_classification_context );
    }

    return ( rdd_error );
}

static inline BL_LILAC_RDD_ERROR_DTE f_rdd_modify_iptv_l3_dst_ip_src_ip_vid_entry ( uint32_t  xi_entry_index,
                                                                                    uint32_t  xi_egress_port_vector,
														                            uint16_t  xi_wifi_ssid_vector,
                                                                                    uint8_t   xi_ingress_classification_context )
{
    return ( f_rdd_modify_iptv_l3_dst_ip_src_ip_entry ( xi_entry_index, xi_egress_port_vector, xi_wifi_ssid_vector, xi_ingress_classification_context ) );
}


BL_LILAC_RDD_ERROR_DTE rdd_iptv_entry_modify ( uint32_t  xi_entry_index,
                                               uint32_t  xi_egress_port_vector,
                                               uint16_t  xi_wifi_ssid_vector,
                                               uint8_t   xi_ingress_classification_context )
{
    rdpa_iptv_lookup_method                iptv_classification_mode;
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS  *bridge_cfg_register;
    BL_LILAC_RDD_ERROR_DTE                 rdd_error;

    rdd_error = BL_LILAC_RDD_OK;

    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) +
                                                                      DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );

    RDD_BRIDGE_CONFIGURATION_REGISTER_IPTV_CLASSIFICATION_METHOD_READ ( iptv_classification_mode, bridge_cfg_register );

    f_rdd_lock ( &int_lock );

    switch ( iptv_classification_mode )
    {
    case iptv_lookup_method_mac:

         rdd_error = f_rdd_modify_iptv_l2_mac_entry ( xi_entry_index, xi_egress_port_vector, xi_wifi_ssid_vector, xi_ingress_classification_context );
         break;

    case iptv_lookup_method_mac_vid:

         rdd_error = f_rdd_modify_iptv_l2_mac_vid_entry ( xi_entry_index, xi_egress_port_vector, xi_wifi_ssid_vector, xi_ingress_classification_context );
         break;

    case iptv_lookup_method_group_ip_src_ip:

         rdd_error = f_rdd_modify_iptv_l3_dst_ip_src_ip_entry ( xi_entry_index, xi_egress_port_vector, xi_wifi_ssid_vector, xi_ingress_classification_context );
         break;

    case iptv_lookup_method_group_ip:

        rdd_error = f_rdd_modify_iptv_l3_dst_ip_entry ( xi_entry_index, xi_egress_port_vector, xi_wifi_ssid_vector, xi_ingress_classification_context );
        break;

    case iptv_lookup_method_group_ip_src_ip_vid:

         rdd_error = f_rdd_modify_iptv_l3_dst_ip_src_ip_vid_entry ( xi_entry_index, xi_egress_port_vector, xi_wifi_ssid_vector, xi_ingress_classification_context );
         break;
    }

    f_rdd_unlock ( &int_lock );
    return ( rdd_error );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_entry_from_ddr ( uint32_t  xi_entry_index,
                                                                 uint32_t  *xo_cache_index,
                                                                 uint32_t  *xo_cache )
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS   *iptv_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS   *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_TABLE_DTS  *iptv_context_table_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS  *iptv_context_entry_ptr;
    uint32_t                        iptv_entry_valid;

    iptv_table_ptr = ( RDD_IPTV_DDR_LOOKUP_TABLE_DTS * )IPTVTableBase;
    iptv_entry_ptr = &( iptv_table_ptr->entry[ xi_entry_index ] );

    RDD_IPTV_L2_DDR_LOOKUP_ENTRY_VALID_READ ( iptv_entry_valid, iptv_entry_ptr );

    if ( !( iptv_entry_valid ) )
    {
        return ( BL_LILAC_RDD_ERROR_REMOVE_LOOKUP_ENTRY );
    }

    iptv_context_table_ptr = ( RDD_IPTV_DDR_CONTEXT_TABLE_DTS * )IPTVContextTableBase;
    iptv_context_entry_ptr = &( iptv_context_table_ptr->entry[ xi_entry_index ] );

    RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_READ ( *xo_cache, iptv_context_entry_ptr );

    if ( *xo_cache )
    {
        RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_READ ( *xo_cache_index, iptv_context_entry_ptr );
    }

    MEMSET ( iptv_context_entry_ptr, 0, sizeof ( RDD_IPTV_DDR_CONTEXT_ENTRY_DTS ) );
    MEMSET ( iptv_entry_ptr, 0, sizeof ( RDD_IPTV_LOOKUP_DDR_UNION_DTS ) );

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_l2_mac_entry ( uint32_t  xi_entry_index,
                                                               uint32_t	 *xo_cache )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint32_t                cache_index;

    rdd_error = f_rdd_delete_iptv_entry_from_ddr ( xi_entry_index, &cache_index, xo_cache );

    if ( ( rdd_error == BL_LILAC_RDD_OK ) && ( *xo_cache ) )
    {
        rdd_error = f_rdd_delete_iptv_entry_from_cache ( cache_index );
    }

    return ( rdd_error );
}


/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_l2_mac_vid_entry ( uint32_t  xi_entry_index,
                                                                   uint32_t  *xo_cache )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint32_t                cache_index;

    rdd_error = f_rdd_delete_iptv_entry_from_ddr ( xi_entry_index, &cache_index, xo_cache );

    if ( ( rdd_error == BL_LILAC_RDD_OK ) && ( *xo_cache ) )
    {
        rdd_error = f_rdd_delete_iptv_entry_from_cache ( cache_index );
    }

    return ( rdd_error );
}


/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_free_iptv_l3_dst_ip_src_ip_context_table ( uint32_t  xi_context_table_index )
{
    g_iptv_context_tables_free_list->entry[ g_iptv_context_tables_free_list_tail ] = xi_context_table_index;
    g_iptv_context_tables_free_list->entry[ xi_context_table_index ] = LILAC_RDD_IPTV_SSM_CONTEXT_ENTRY_COUNT;
    g_iptv_context_tables_free_list_tail = xi_context_table_index;

    if ( g_iptv_context_tables_free_list_head == LILAC_RDD_IPTV_SSM_CONTEXT_ENTRY_COUNT )
    {
        g_iptv_context_tables_free_list_head = xi_context_table_index;
    }

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_l3_dst_ip_src_ip_any_entry_from_cache ( uint32_t  xi_cache_index )
{
    RDD_IPTV_LOOKUP_TABLE_DTS                          *iptv_table_ptr;
    RDD_IPTV_L3_LOOKUP_ENTRY_DTS                       *iptv_entry_ptr;
    RDD_IPTV_CONTEXT_TABLE_DTS                         *forward_table_ptr;
    RDD_IPTV_CONTEXT_ENTRY_DTS                         *forward_entry_ptr;
    RDD_IPTV_COUNTERS_TABLE_DTS                        *iptv_counter_table_ptr;
    RDD_IPTV_COUNTER_ENTRY_DTS                         *iptv_counter_entry_ptr;
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS  *iptv_ingress_classification_context_table_ptr; 
    RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS  *iptv_ingress_classification_context_entry_ptr; 

    iptv_table_ptr = ( RDD_IPTV_LOOKUP_TABLE_DTS * )g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ].hash_table_ptr;

    forward_table_ptr = ( RDD_IPTV_CONTEXT_TABLE_DTS * )( g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ].context_table_ptr );

    iptv_counter_table_ptr = ( RDD_IPTV_COUNTERS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPTV_COUNTERS_TABLE_ADDRESS );

    iptv_counter_entry_ptr = &( iptv_counter_table_ptr->entry[ xi_cache_index ] );

    iptv_ingress_classification_context_table_ptr = ( RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_ADDRESS );

    iptv_ingress_classification_context_entry_ptr = &( iptv_ingress_classification_context_table_ptr->entry[ xi_cache_index ] );

    if ( xi_cache_index >= RDD_IPTV_LOOKUP_TABLE_SIZE )
    {
        xi_cache_index -= RDD_IPTV_LOOKUP_TABLE_SIZE;

        iptv_table_ptr = ( RDD_IPTV_LOOKUP_TABLE_DTS * )g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ].cam_table_ptr;

        forward_table_ptr = ( RDD_IPTV_CONTEXT_TABLE_DTS * )( g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ].cam_context_table_ptr );
    }

    iptv_entry_ptr = &( iptv_table_ptr->entry[ xi_cache_index ].iptv_l3_lookup_entry );

    forward_entry_ptr = &( forward_table_ptr->entry[ xi_cache_index ] );

    RDD_IPTV_L3_LOOKUP_ENTRY_ANY_WRITE ( LILAC_RDD_FALSE, iptv_entry_ptr );
    MEMSET ( forward_entry_ptr, 0, sizeof ( RDD_IPTV_CONTEXT_ENTRY_DTS ) );
    RDD_IPTV_COUNTER_ENTRY_COUNTER_WRITE ( 0, iptv_counter_entry_ptr );

    MEMSET( iptv_ingress_classification_context_entry_ptr, 0, sizeof ( RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS ) );

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_l3_dst_ip_src_ip_any_entry_from_ddr ( uint32_t  xi_entry_index,
                                                                                      uint32_t  *xo_cache, 
                                                                                      uint32_t  *xo_cache_index,
                                                                                      uint32_t  *xo_base_deleted )
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS   *iptv_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS   *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_TABLE_DTS  *iptv_context_table_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS  *iptv_ssm_context_entry_ptr;
    BL_LILAC_RDD_ERROR_DTE          rdd_error;
    uint32_t                        iptv_entry_valid;
    uint32_t                        context_table_valid;
    uint32_t                        context_table_index;

    iptv_table_ptr = ( RDD_IPTV_DDR_LOOKUP_TABLE_DTS * )IPTVTableBase;

    iptv_entry_ptr = &( iptv_table_ptr->entry[ xi_entry_index ] );

    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_VALID_READ ( iptv_entry_valid, iptv_entry_ptr );

    if ( !( iptv_entry_valid ) )
    {
        return ( BL_LILAC_RDD_ERROR_REMOVE_LOOKUP_ENTRY );
    }

    iptv_context_table_ptr = ( RDD_IPTV_DDR_CONTEXT_TABLE_DTS * )IPTVContextTableBase;
    iptv_ssm_context_entry_ptr = &( iptv_context_table_ptr->entry[ xi_entry_index ] );

    RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_READ ( *xo_cache , iptv_ssm_context_entry_ptr );

    if ( *xo_cache )
    {
        RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_READ ( *xo_cache_index, iptv_ssm_context_entry_ptr );
    }
    else
    {
        *xo_cache_index = ( RDD_IPTV_LOOKUP_TABLE_SIZE + RDD_IPTV_LOOKUP_TABLE_CAM_SIZE - 1 );
    }

    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_ANY_WRITE ( LILAC_RDD_FALSE, iptv_entry_ptr );

    /* in ssm mode the context of entries with src ip = any is written according to the "regular" context table according to the ssm mode context format */
    RDD_IPTV_DDR_CONTEXT_ENTRY_PORT_VECTOR_INFO_WRITE ( 0, iptv_ssm_context_entry_ptr );
    RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_WRITE ( 0, iptv_ssm_context_entry_ptr );
    RDD_IPTV_DDR_CONTEXT_ENTRY_WLAN_MCAST_INDEX_WRITE ( 0, iptv_ssm_context_entry_ptr );
    RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_WRITE ( LILAC_RDD_FALSE, iptv_ssm_context_entry_ptr );
    RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_WRITE ( 0, iptv_ssm_context_entry_ptr );

    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_VALID_READ ( context_table_valid, iptv_entry_ptr );

    if ( !( context_table_valid ) )
    {
        RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_TABLE_READ ( context_table_index, iptv_entry_ptr );

        f_rdd_free_iptv_l3_dst_ip_src_ip_context_table ( context_table_index );

        rdd_error = f_rdd_delete_iptv_l3_dst_ip_entry ( xi_entry_index, xo_cache );

        *xo_base_deleted = LILAC_RDD_TRUE;
    }
    else
    {
        rdd_error = BL_LILAC_RDD_OK;

        *xo_base_deleted = LILAC_RDD_FALSE;
    }

    return ( rdd_error );
}


/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_l3_dst_ip_src_ip_non_any_entry_from_cache ( uint32_t  xi_cache_index,
                                                                                            uint32_t  xi_provider_index )
{
    RDD_IPTV_LOOKUP_TABLE_DTS     *iptv_table_ptr;
    RDD_IPTV_L3_LOOKUP_ENTRY_DTS  *iptv_entry_ptr;

    iptv_table_ptr = ( RDD_IPTV_LOOKUP_TABLE_DTS * )g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ].hash_table_ptr;

    if ( xi_cache_index >= RDD_IPTV_LOOKUP_TABLE_SIZE )
    {
        xi_cache_index -= RDD_IPTV_LOOKUP_TABLE_SIZE;

        iptv_table_ptr = ( RDD_IPTV_LOOKUP_TABLE_DTS * )g_hash_table_cfg[ BL_LILAC_RDD_IPTV_TABLE ].cam_table_ptr;
    }

    iptv_entry_ptr = &( iptv_table_ptr->entry[ xi_cache_index ].iptv_l3_lookup_entry );

    RDD_IPTV_L3_LOOKUP_ENTRY_CONTEXT_VALID_WRITE ( LILAC_RDD_FALSE, iptv_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_l3_dst_ip_src_ip_non_any_entry_from_ddr ( uint32_t  xi_entry_index,
                                                                                          uint32_t  xi_provider_index,
                                                                                          uint32_t  *xo_cache,
                                                                                          uint32_t  *xo_cache_index,
                                                                                          uint32_t  *xo_base_deleted )
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS        *iptv_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS        *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_TABLE_DTS       *iptv_context_table_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS       *iptv_context_entry_ptr;
    RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS   *iptv_ssm_context_table_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS       *iptv_ssm_context_entry_ptr;
    RDD_IPTV_L3_SRC_IP_LOOKUP_TABLE_DTS  *iptv_layer3_src_ip_table_ptr;
    RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS  *iptv_layer3_src_ip_entry_ptr;
    BL_LILAC_RDD_ERROR_DTE               rdd_error;
    uint32_t                             any;
    uint32_t                             context_table_index;
    uint32_t                             context_table_offset;
    uint32_t                             context_table_valid;
    uint32_t                             valid_entries_count;
    bdmf_ip_t                            dst_ip;
    bdmf_ip_t                            src_ip;
    uint32_t                             src_ip_index;
    uint32_t                             context_entry_valid;
    uint8_t                              src_ip_addr[ 4 ];

    iptv_table_ptr = ( RDD_IPTV_DDR_LOOKUP_TABLE_DTS * )IPTVTableBase;

    iptv_entry_ptr = &( iptv_table_ptr->entry[ xi_entry_index ] );

    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_TABLE_READ ( context_table_index, iptv_entry_ptr );

    context_table_offset = context_table_index * LILAC_RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS;

    valid_entries_count = 0;

    iptv_ssm_context_table_ptr = ( RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS * )IPTVSsmContextTableBase;

    for ( src_ip_index = 0; src_ip_index < LILAC_RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS; src_ip_index++ )
    {
        iptv_ssm_context_entry_ptr = &( iptv_ssm_context_table_ptr->entry[ context_table_offset + src_ip_index ] );

        RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_READ ( context_entry_valid, iptv_ssm_context_entry_ptr );

        if ( context_entry_valid )
        {
            valid_entries_count++;

            if ( valid_entries_count == 2 )
            {
                break;
            }
        }
    }

    if ( valid_entries_count < 2 )
    {
        RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_VALID_WRITE ( LILAC_RDD_FALSE, iptv_entry_ptr );
    }

    /* delete context */
    iptv_ssm_context_entry_ptr = &( iptv_ssm_context_table_ptr->entry[ context_table_offset + xi_provider_index ] );

    MEMSET ( iptv_ssm_context_entry_ptr, 0, sizeof ( RDD_IPTV_DDR_CONTEXT_ENTRY_DTS ) );

    /* read cache index */
    iptv_context_table_ptr = ( RDD_IPTV_DDR_CONTEXT_TABLE_DTS * )IPTVContextTableBase;

    iptv_context_entry_ptr = &( iptv_context_table_ptr->entry[ xi_entry_index ] );

    RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_READ ( *xo_cache, iptv_context_entry_ptr );
    RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_READ ( *xo_cache_index, iptv_context_entry_ptr );

    /* delete src ip */
    iptv_layer3_src_ip_table_ptr = ( RDD_IPTV_L3_SRC_IP_LOOKUP_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + IPTV_L3_SRC_IP_LOOKUP_TABLE_ADDRESS );

    iptv_layer3_src_ip_entry_ptr = &( iptv_layer3_src_ip_table_ptr->entry[ xi_provider_index ] );

    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DST_IP12_READ( dst_ip.addr.ipv6.data[ 12 ], iptv_entry_ptr );
    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DST_IP13_READ( dst_ip.addr.ipv6.data[ 13 ], iptv_entry_ptr );
    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DST_IP14_READ( dst_ip.addr.ipv6.data[ 14 ], iptv_entry_ptr );
    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DST_IP15_READ( dst_ip.addr.ipv6.data[ 15 ], iptv_entry_ptr );
    
    if ( *(( uint32_t *)&dst_ip.addr.ipv6.data[ 12 ]) != 0 ) 
    {
        src_ip.family = bdmf_ip_family_ipv6;
    }
    else
    {
        src_ip.family = bdmf_ip_family_ipv4;
    }

    if ( src_ip.family == bdmf_ip_family_ipv6 )
    {
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_12_READ ( src_ip.addr.ipv6.data[ 12 ], ( RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS * )iptv_layer3_src_ip_entry_ptr );
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_13_READ ( src_ip.addr.ipv6.data[ 13 ], ( RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS * )iptv_layer3_src_ip_entry_ptr );
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_14_READ ( src_ip.addr.ipv6.data[ 14 ], ( RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS * )iptv_layer3_src_ip_entry_ptr );
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_15_READ ( src_ip.addr.ipv6.data[ 15 ], ( RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS * )iptv_layer3_src_ip_entry_ptr );

        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_3_READ ( src_ip.addr.ipv6.data[ 0 ], ( RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS * )iptv_layer3_src_ip_entry_ptr );
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_2_READ ( src_ip.addr.ipv6.data[ 1 ], ( RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS * )iptv_layer3_src_ip_entry_ptr );
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_1_READ ( src_ip.addr.ipv6.data[ 2 ], ( RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS * )iptv_layer3_src_ip_entry_ptr );
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_0_READ ( src_ip.addr.ipv6.data[ 3 ], ( RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS * )iptv_layer3_src_ip_entry_ptr );
        
    }
    else
    {
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_0_READ ( src_ip_addr[ 0 ], ( RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS * )iptv_layer3_src_ip_entry_ptr );
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_1_READ ( src_ip_addr[ 1 ], ( RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS * )iptv_layer3_src_ip_entry_ptr );
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_2_READ ( src_ip_addr[ 2 ], ( RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS * )iptv_layer3_src_ip_entry_ptr );
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_3_READ ( src_ip_addr[ 3 ], ( RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS * )iptv_layer3_src_ip_entry_ptr );

        src_ip.addr.ipv4  = ( src_ip_addr[ 0 ] << 24 );
        src_ip.addr.ipv4 |= ( src_ip_addr[ 1 ] << 16 );
        src_ip.addr.ipv4 |= ( src_ip_addr[ 2 ] <<  8 );
        src_ip.addr.ipv4 |= ( src_ip_addr[ 3 ] <<  0 );
    }

    rdd_error = f_rdd_delete_iptv_layer3_src_ip ( &src_ip, &src_ip_index );

    /* check if base entry should be deleted as well */
    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_ANY_READ ( any, iptv_entry_ptr );
    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_VALID_READ ( context_table_valid, iptv_entry_ptr );

    if ( !( any ) && !( context_table_valid ) )
    {
        f_rdd_free_iptv_l3_dst_ip_src_ip_context_table ( context_table_index );

        rdd_error = f_rdd_delete_iptv_l3_dst_ip_entry ( xi_entry_index, xo_cache );

        *xo_base_deleted = LILAC_RDD_TRUE;
    }
    else
    {
        rdd_error = BL_LILAC_RDD_OK;

        *xo_base_deleted = LILAC_RDD_FALSE;
    }

    return ( rdd_error );
}


/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_l3_dst_ip_src_ip_any_entry ( uint32_t  xi_entry_index,
                                                                             uint32_t  *xo_cache )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint32_t                cache_index;
    uint32_t                base_deleted;

    rdd_error = f_rdd_delete_iptv_l3_dst_ip_src_ip_any_entry_from_ddr ( xi_entry_index, xo_cache, &cache_index, &base_deleted );

    if ( ( rdd_error == BL_LILAC_RDD_OK ) && !( base_deleted ) && ( *xo_cache ) )
    {
        rdd_error = f_rdd_delete_iptv_l3_dst_ip_src_ip_any_entry_from_cache ( cache_index );
    }

    return ( rdd_error );
}


/******************************************************************************/
static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_l3_dst_ip_src_ip_non_any_entry ( uint32_t  xi_entry_index,
                                                                                 uint32_t  xi_provider_index,
                                                                                 uint32_t  *xo_cache )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint32_t                cache_index;
    uint32_t                base_deleted;

    rdd_error = f_rdd_delete_iptv_l3_dst_ip_src_ip_non_any_entry_from_ddr ( xi_entry_index, xi_provider_index, xo_cache,
                                                                                     &cache_index, &base_deleted );

    if ( ( rdd_error == BL_LILAC_RDD_OK ) && ( base_deleted ) && ( *xo_cache ) )
    {
        rdd_error = f_rdd_delete_iptv_l3_dst_ip_src_ip_non_any_entry_from_cache ( cache_index, xi_provider_index );
    }

    return ( rdd_error );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_l3_dst_ip_src_ip_entry ( uint32_t  xi_entry_index,
                                                                         uint32_t  *xo_cache )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;

    if ( xi_entry_index < RDD_IPTV_DDR_LOOKUP_TABLE_SIZE )
    {
        rdd_error = f_rdd_delete_iptv_l3_dst_ip_src_ip_any_entry ( xi_entry_index, xo_cache );
    }
    else
    {
        rdd_error = f_rdd_delete_iptv_l3_dst_ip_src_ip_non_any_entry ( GET_BASE_INDEX_FROM_ENTRY_INDEX ( xi_entry_index ),
                                                                       GET_PROVIDER_INDEX_FROM_ENTRY_INDEX ( xi_entry_index ),
                                                                       xo_cache );
    }

    return ( rdd_error );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_l3_dst_ip_entry ( uint32_t  xi_entry_index,
                                                                  uint32_t  *xo_cache )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint32_t                cache_index = 0;

    rdd_error = f_rdd_delete_iptv_entry_from_ddr ( xi_entry_index, &cache_index, xo_cache );

    if ( ( rdd_error == BL_LILAC_RDD_OK ) && ( *xo_cache ) )
    {
        rdd_error = f_rdd_delete_iptv_entry_from_cache ( cache_index );
    }

    return ( rdd_error );
}


static inline BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_l3_dst_ip_src_ip_vid_entry ( uint32_t  xi_entry_index,
                                                                                    uint32_t  *xo_cache )
{
    return ( f_rdd_delete_iptv_l3_dst_ip_src_ip_entry ( xi_entry_index, xo_cache ) );
}


BL_LILAC_RDD_ERROR_DTE rdd_iptv_entry_delete ( uint32_t  xi_entry_index,
                                               uint32_t  *xo_cache )
{
    rdpa_iptv_lookup_method                iptv_classification_mode;
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS  *bridge_cfg_register;
    BL_LILAC_RDD_ERROR_DTE                 rdd_error;

    rdd_error = BL_LILAC_RDD_OK;

    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );

    RDD_BRIDGE_CONFIGURATION_REGISTER_IPTV_CLASSIFICATION_METHOD_READ ( iptv_classification_mode, bridge_cfg_register );

    f_rdd_lock ( &int_lock );

    switch ( iptv_classification_mode )
    {
    case iptv_lookup_method_mac:

         rdd_error = f_rdd_delete_iptv_l2_mac_entry ( xi_entry_index, xo_cache );
         break;

    case iptv_lookup_method_mac_vid:

         rdd_error = f_rdd_delete_iptv_l2_mac_vid_entry ( xi_entry_index, xo_cache );
         break;

    case iptv_lookup_method_group_ip_src_ip:

         rdd_error = f_rdd_delete_iptv_l3_dst_ip_src_ip_entry ( xi_entry_index, xo_cache );
         break;

    case iptv_lookup_method_group_ip:

        rdd_error = f_rdd_delete_iptv_l3_dst_ip_entry ( xi_entry_index, xo_cache );
        break;

    case iptv_lookup_method_group_ip_src_ip_vid:

         rdd_error = f_rdd_delete_iptv_l3_dst_ip_src_ip_vid_entry ( xi_entry_index, xo_cache );
         break;
    }

    f_rdd_unlock ( &int_lock );
    return ( rdd_error );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_search_iptv_ssm_entry ( RDD_IPTV_ENTRY_UNION  *xi_entry,
                                                            bdmf_ip_t             *xi_ipv6_src_ip_ptr,
                                                            uint32_t              *xo_index )
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS       *iptv_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS       *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS      *iptv_context_entry_ptr;
    RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS  *iptv_ssm_context_table_ptr;
    BL_LILAC_RDD_ERROR_DTE              rdd_error;
    uint8_t                             src_ip_hash_entry[ 8 ];
    uint32_t                            iptv_entry_valid;
    uint32_t                            src_ip_index;
    uint32_t                            context_table_index;
    uint32_t                            base_index;

	if ( xi_ipv6_src_ip_ptr == NULL )
    {
        src_ip_hash_entry[ 0 ] = 0;
        src_ip_hash_entry[ 1 ] = 0;
        src_ip_hash_entry[ 2 ] = 0;
        src_ip_hash_entry[ 3 ] = 0;
    }
    else
    {
        src_ip_hash_entry[ 0 ] = xi_ipv6_src_ip_ptr->addr.ipv6.data[ 12 ];
        src_ip_hash_entry[ 1 ] = xi_ipv6_src_ip_ptr->addr.ipv6.data[ 13 ];
        src_ip_hash_entry[ 2 ] = xi_ipv6_src_ip_ptr->addr.ipv6.data[ 14 ];
        src_ip_hash_entry[ 3 ] = xi_ipv6_src_ip_ptr->addr.ipv6.data[ 15 ];
    }

    src_ip_hash_entry[ 4 ] = ( xi_entry->l3_entry_fields.src_ip.addr.ipv4 >> 24 ) & 0xFF;
    src_ip_hash_entry[ 5 ] = ( xi_entry->l3_entry_fields.src_ip.addr.ipv4 >> 16 ) & 0xFF;
    src_ip_hash_entry[ 6 ] = ( xi_entry->l3_entry_fields.src_ip.addr.ipv4 >>  8 ) & 0xFF;
    src_ip_hash_entry[ 7 ] = ( xi_entry->l3_entry_fields.src_ip.addr.ipv4 >>  0 ) & 0xFF;

    rdd_error = rdd_find_hash_entry_64_bit ( &g_hash_table_cfg[ BL_LILAC_RDD_IPTV_SRC_IP_TABLE ],
                                             src_ip_hash_entry,
                                             IPTV_L3_SSM_SRC_IP_ENTRY_KEY_MASK_HIGH,
                                             IPTV_L3_SSM_SRC_IP_ENTRY_KEY_MASK_LOW,
                                             0,
                                             &src_ip_index );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        return ( rdd_error );
    }

	iptv_table_ptr = ( RDD_IPTV_DDR_LOOKUP_TABLE_DTS * )IPTVTableBase;
    iptv_entry_ptr = &( iptv_table_ptr->entry[ *xo_index ] );

    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_TABLE_READ ( context_table_index, iptv_entry_ptr );

    iptv_ssm_context_table_ptr = ( RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS * )IPTVSsmContextTableBase;
    iptv_context_entry_ptr = &( iptv_ssm_context_table_ptr->entry[ context_table_index * LILAC_RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS + src_ip_index ] );

    RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_READ ( iptv_entry_valid, iptv_context_entry_ptr );

    if ( !( iptv_entry_valid ) )
    {
        return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
    }

    base_index = *xo_index;
    *xo_index = GET_ENTRY_INDEX_FROM_BASE_AND_PROVIDER_INDICES( base_index, src_ip_index );

    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_search_iptv_entry ( RDD_IPTV_ENTRY_UNION     *xi_entry,
                                                        uint8_t                  *xi_entry_bytes,
                                                        rdpa_iptv_lookup_method  xi_iptv_classification_mode,
                                                        bdmf_ip_t                *xi_ipv6_dst_ip_ptr,
                                                        uint32_t                 *xo_index )
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS  *iptv_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS  *iptv_entry_ptr;
    BL_LILAC_RDD_ERROR_DTE         rdd_error;
    bdmf_mac_t                     iptv_entry_mac_addr;
    bdmf_ip_t                      iptv_entry_ip_addr;
    uint32_t                       crc_init_value;
    uint32_t                       crc_result;
    uint32_t                       hash_index; 
    uint32_t                       tries;
    uint32_t                       iptv_entry_index;
    uint32_t                       iptv_entry_valid;
    uint32_t                       any;
    uint32_t                       iptv_entry_vid;
    uint8_t                        dst_ip_addr[ 4 ];
    uint8_t                        ipv6_dst_ip_address[ 4 ];

    rdd_error = BL_LILAC_RDD_OK;

    crc_init_value = rdd_crc_init_value_get ( RDD_CRC_TYPE_32 );

    crc_result = rdd_crc_bit_by_bit ( &xi_entry_bytes[ 4 ], 12, 0, crc_init_value, RDD_CRC_TYPE_32 );

    hash_index = crc_result & ( RDD_IPTV_DDR_LOOKUP_TABLE_SIZE / LILAC_RDD_IPTV_TABLE_SET_SIZE - 1 );

    hash_index = hash_index * LILAC_RDD_IPTV_TABLE_SET_SIZE;

    iptv_table_ptr = ( RDD_IPTV_DDR_LOOKUP_TABLE_DTS * )IPTVTableBase;

    for ( tries = 0; tries < LILAC_RDD_IPTV_TABLE_SET_SIZE; tries++ )
    {
        iptv_entry_index = ( hash_index + tries ) & ( RDD_IPTV_DDR_LOOKUP_TABLE_SIZE - 1 );

        iptv_entry_ptr = &( iptv_table_ptr->entry[ iptv_entry_index ] );

        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_VALID_READ ( iptv_entry_valid, iptv_entry_ptr );

        if ( iptv_entry_valid )
        {
            if ( xi_iptv_classification_mode == iptv_lookup_method_mac )
            {
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR5_READ ( iptv_entry_mac_addr.b[ 5 ], iptv_entry_ptr );
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR4_READ ( iptv_entry_mac_addr.b[ 4 ], iptv_entry_ptr );
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR3_READ ( iptv_entry_mac_addr.b[ 3 ], iptv_entry_ptr );
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR2_READ ( iptv_entry_mac_addr.b[ 2 ], iptv_entry_ptr );
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR1_READ ( iptv_entry_mac_addr.b[ 1 ], iptv_entry_ptr );
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR0_READ ( iptv_entry_mac_addr.b[ 0 ], iptv_entry_ptr );

                if ( ( iptv_entry_mac_addr.b[ 5 ] == xi_entry->l2_entry_fields.mac_addr.b[ 5 ] ) &&
                     ( iptv_entry_mac_addr.b[ 4 ] == xi_entry->l2_entry_fields.mac_addr.b[ 4 ] ) &&
                     ( iptv_entry_mac_addr.b[ 3 ] == xi_entry->l2_entry_fields.mac_addr.b[ 3 ] ) &&
                     ( iptv_entry_mac_addr.b[ 2 ] == xi_entry->l2_entry_fields.mac_addr.b[ 2 ] ) &&
                     ( iptv_entry_mac_addr.b[ 1 ] == xi_entry->l2_entry_fields.mac_addr.b[ 1 ] ) &&
                     ( iptv_entry_mac_addr.b[ 0 ] == xi_entry->l2_entry_fields.mac_addr.b[ 0 ] ) )
                {
                     break;
                }
            }

            if ( xi_iptv_classification_mode == iptv_lookup_method_mac_vid )
            {
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR5_READ ( iptv_entry_mac_addr.b[ 5 ], iptv_entry_ptr );
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR4_READ ( iptv_entry_mac_addr.b[ 4 ], iptv_entry_ptr );
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR3_READ ( iptv_entry_mac_addr.b[ 3 ], iptv_entry_ptr );
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR2_READ ( iptv_entry_mac_addr.b[ 2 ], iptv_entry_ptr );
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR1_READ ( iptv_entry_mac_addr.b[ 1 ], iptv_entry_ptr );
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR0_READ ( iptv_entry_mac_addr.b[ 0 ], iptv_entry_ptr );
                RDD_IPTV_L2_DDR_LOOKUP_ENTRY_VID_READ ( iptv_entry_vid ,iptv_entry_ptr );

                if ( ( iptv_entry_mac_addr.b[ 5 ] == xi_entry->l2_entry_fields.mac_addr.b[ 5 ] ) &&
                     ( iptv_entry_mac_addr.b[ 4 ] == xi_entry->l2_entry_fields.mac_addr.b[ 4 ] ) &&
                     ( iptv_entry_mac_addr.b[ 3 ] == xi_entry->l2_entry_fields.mac_addr.b[ 3 ] ) &&
                     ( iptv_entry_mac_addr.b[ 2 ] == xi_entry->l2_entry_fields.mac_addr.b[ 2 ] ) &&
                     ( iptv_entry_mac_addr.b[ 1 ] == xi_entry->l2_entry_fields.mac_addr.b[ 1 ] ) &&
                     ( iptv_entry_mac_addr.b[ 0 ] == xi_entry->l2_entry_fields.mac_addr.b[ 0 ] ) &&
                     ( iptv_entry_vid == xi_entry->l2_entry_fields.vid ) )
                {
                     break;
                }
            }

            if ( xi_iptv_classification_mode == iptv_lookup_method_group_ip )
            {
                if ( xi_ipv6_dst_ip_ptr != NULL ) 
                {                 
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP12_READ ( ipv6_dst_ip_address[ 0 ], iptv_entry_ptr );
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP13_READ ( ipv6_dst_ip_address[ 1 ], iptv_entry_ptr );
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP14_READ ( ipv6_dst_ip_address[ 2 ], iptv_entry_ptr );
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP15_READ ( ipv6_dst_ip_address[ 3 ], iptv_entry_ptr );
                }

                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_READ ( dst_ip_addr[ 0 ], iptv_entry_ptr );
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_READ ( dst_ip_addr[ 1 ], iptv_entry_ptr );
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_READ ( dst_ip_addr[ 2 ], iptv_entry_ptr );
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_READ ( dst_ip_addr[ 3 ], iptv_entry_ptr );

                iptv_entry_ip_addr.addr.ipv4  = ( dst_ip_addr[ 0 ] << 24 );
                iptv_entry_ip_addr.addr.ipv4 |= ( dst_ip_addr[ 1 ] << 16 );
                iptv_entry_ip_addr.addr.ipv4 |= ( dst_ip_addr[ 2 ] <<  8 );
                iptv_entry_ip_addr.addr.ipv4 |= ( dst_ip_addr[ 3 ] <<  0 );

                if ( iptv_entry_ip_addr.addr.ipv4 == xi_entry->l3_entry_fields.dst_ip.addr.ipv4 )
                {
                    if ( xi_ipv6_dst_ip_ptr != NULL )
                    {
                        if ( ipv6_dst_ip_address[ 0 ] != xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 12 ] ||
                             ipv6_dst_ip_address[ 1 ] != xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 13 ] ||
                             ipv6_dst_ip_address[ 2 ] != xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 14 ] ||
                             ipv6_dst_ip_address[ 3 ] != xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 15 ] )
                        {
                            continue;
                        }
                    }

                    break;
                }
            }

            if ( xi_iptv_classification_mode == iptv_lookup_method_group_ip_src_ip )
            {
                if ( xi_ipv6_dst_ip_ptr != NULL ) 
                {
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP12_READ ( ipv6_dst_ip_address[ 0 ], iptv_entry_ptr );
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP13_READ ( ipv6_dst_ip_address[ 1 ], iptv_entry_ptr );
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP14_READ ( ipv6_dst_ip_address[ 2 ], iptv_entry_ptr );
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP15_READ ( ipv6_dst_ip_address[ 3 ], iptv_entry_ptr );
                }

                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_READ ( dst_ip_addr[ 0 ], iptv_entry_ptr );
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_READ ( dst_ip_addr[ 1 ], iptv_entry_ptr );
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_READ ( dst_ip_addr[ 2 ], iptv_entry_ptr );
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_READ ( dst_ip_addr[ 3 ], iptv_entry_ptr );

                iptv_entry_ip_addr.addr.ipv4  = ( dst_ip_addr[ 0 ] << 24 );
                iptv_entry_ip_addr.addr.ipv4 |= ( dst_ip_addr[ 1 ] << 16 );
                iptv_entry_ip_addr.addr.ipv4 |= ( dst_ip_addr[ 2 ] <<  8 );
                iptv_entry_ip_addr.addr.ipv4 |= ( dst_ip_addr[ 3 ] <<  0 );

                if ( iptv_entry_ip_addr.addr.ipv4 == xi_entry->l3_entry_fields.dst_ip.addr.ipv4 )
                {
                    if ( xi_ipv6_dst_ip_ptr != NULL )
                    {
                        if ( ipv6_dst_ip_address[ 0 ] != xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 12 ] ||
                             ipv6_dst_ip_address[ 1 ] != xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 13 ] ||
                             ipv6_dst_ip_address[ 2 ] != xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 14 ] ||
                             ipv6_dst_ip_address[ 3 ] != xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 15 ] )
                        {
                            continue;
                        }
                    }

                    if ( xi_entry->l3_entry_fields.src_ip.addr.ipv4 == 0 )
                    {
                        RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_ANY_READ ( any, iptv_entry_ptr );

                        if ( !( any ) )
                        {
                            return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
                        }
                    }

                    break;
                }
            }

            if ( xi_iptv_classification_mode == iptv_lookup_method_group_ip_src_ip_vid )
            {
                if ( xi_ipv6_dst_ip_ptr != NULL ) 
                {
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP12_READ ( ipv6_dst_ip_address[ 0 ], iptv_entry_ptr );
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP13_READ ( ipv6_dst_ip_address[ 1 ], iptv_entry_ptr );
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP14_READ ( ipv6_dst_ip_address[ 2 ], iptv_entry_ptr );
                    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP15_READ ( ipv6_dst_ip_address[ 3 ], iptv_entry_ptr );
                }

                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_READ ( dst_ip_addr[ 0 ], iptv_entry_ptr );
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_READ ( dst_ip_addr[ 1 ], iptv_entry_ptr );
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_READ ( dst_ip_addr[ 2 ], iptv_entry_ptr );
                RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_READ ( dst_ip_addr[ 3 ], iptv_entry_ptr );

                iptv_entry_ip_addr.addr.ipv4  = ( dst_ip_addr[ 0 ] << 24 );
                iptv_entry_ip_addr.addr.ipv4 |= ( dst_ip_addr[ 1 ] << 16 );
                iptv_entry_ip_addr.addr.ipv4 |= ( dst_ip_addr[ 2 ] <<  8 );
                iptv_entry_ip_addr.addr.ipv4 |= ( dst_ip_addr[ 3 ] <<  0 );

                RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_VID_READ ( iptv_entry_vid, iptv_entry_ptr );

                if ( ( iptv_entry_ip_addr.addr.ipv4 == xi_entry->l3_entry_fields.dst_ip.addr.ipv4 ) && ( iptv_entry_vid == xi_entry->l3_entry_fields.vid ) )
                {
                    if ( xi_ipv6_dst_ip_ptr != NULL )
                    {
                        if ( ipv6_dst_ip_address[ 0 ] != xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 12 ] ||
                             ipv6_dst_ip_address[ 1 ] != xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 13 ] ||
                             ipv6_dst_ip_address[ 2 ] != xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 14 ] ||
                             ipv6_dst_ip_address[ 3 ] != xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 15 ] )
                        {
                            continue;
                        }
                    }

                    if ( xi_entry->l3_entry_fields.src_ip.addr.ipv4 == 0 )
                    {
                        RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_ANY_READ ( any, iptv_entry_ptr );

                        if ( !( any ) )
                        {
                            return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
                        }
                    }

                    break;
                }
            }
        }
    }

    if ( tries == LILAC_RDD_CONNECTION_TABLE_SET_SIZE )
    {
        return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
    }

    *xo_index = iptv_entry_index;

    return ( rdd_error );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_get_iptv_l2_mac_entry_index ( RDD_IPTV_ENTRY_UNION  *xi_entry,
                                                                  uint32_t              *xo_index )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint8_t                 entry_bytes[ LILAC_RDD_IPTV_ENTRY_SIZE ];

    MEMSET ( entry_bytes, 0, LILAC_RDD_IPTV_ENTRY_SIZE );

    entry_bytes[ 10 ] = xi_entry->l2_entry_fields.mac_addr.b[ 0 ];
    entry_bytes[ 11 ] = xi_entry->l2_entry_fields.mac_addr.b[ 1 ];
    entry_bytes[ 12 ] = xi_entry->l2_entry_fields.mac_addr.b[ 2 ];
    entry_bytes[ 13 ] = xi_entry->l2_entry_fields.mac_addr.b[ 3 ];
    entry_bytes[ 14 ] = xi_entry->l2_entry_fields.mac_addr.b[ 4 ];
    entry_bytes[ 15 ] = xi_entry->l2_entry_fields.mac_addr.b[ 5 ];

    rdd_error = f_rdd_search_iptv_entry ( xi_entry, entry_bytes, iptv_lookup_method_mac, 0, xo_index );

    return ( rdd_error );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_get_iptv_l2_mac_vid_entry_index ( RDD_IPTV_ENTRY_UNION  *xi_entry,
                                                                      uint32_t              *xo_index )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint8_t                 entry_bytes[ LILAC_RDD_IPTV_ENTRY_SIZE ];

    MEMSET ( entry_bytes, 0, LILAC_RDD_IPTV_ENTRY_SIZE );

    entry_bytes[ 8 ] = xi_entry->l2_entry_fields.vid >> 8;
    entry_bytes[ 9 ] = xi_entry->l2_entry_fields.vid & 0xFF;
    entry_bytes[ 10 ] = xi_entry->l2_entry_fields.mac_addr.b[ 0 ];
    entry_bytes[ 11 ] = xi_entry->l2_entry_fields.mac_addr.b[ 1 ];
    entry_bytes[ 12 ] = xi_entry->l2_entry_fields.mac_addr.b[ 2 ];
    entry_bytes[ 13 ] = xi_entry->l2_entry_fields.mac_addr.b[ 3 ];
    entry_bytes[ 14 ] = xi_entry->l2_entry_fields.mac_addr.b[ 4 ];
    entry_bytes[ 15 ] = xi_entry->l2_entry_fields.mac_addr.b[ 5 ];

    rdd_error = f_rdd_search_iptv_entry ( xi_entry, entry_bytes, iptv_lookup_method_mac_vid, 0, xo_index );

    return ( rdd_error );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_get_iptv_l3_dst_ip_src_ip_entry_index ( RDD_IPTV_ENTRY_UNION     *xi_entry,
                                                                            rdpa_iptv_lookup_method  xi_iptv_mode,
                                                                            bdmf_ip_t                *xi_ipv6_dst_ip_ptr,
                                                                            bdmf_ip_t                *xi_ipv6_src_ip_ptr,
                                                                            uint32_t                 *xo_index )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint8_t                 entry_bytes[ LILAC_RDD_IPTV_ENTRY_SIZE ];

    MEMSET ( entry_bytes, 0, LILAC_RDD_IPTV_ENTRY_SIZE );

    if ( xi_ipv6_dst_ip_ptr != NULL ) 
    {
        entry_bytes[ 4 ] = xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 12 ];
        entry_bytes[ 5 ] = xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 13 ];
        entry_bytes[ 6 ] = xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 14 ];
        entry_bytes[ 7 ] = xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 15 ];
    }

    if ( xi_iptv_mode == iptv_lookup_method_group_ip_src_ip_vid )
    {
        entry_bytes[ 10 ] = xi_entry->l3_entry_fields.vid >> 8;
        entry_bytes[ 11 ] = xi_entry->l3_entry_fields.vid & 0xFF;
    }

    entry_bytes[ 12 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >> 24 ) & 0xFF;
    entry_bytes[ 13 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >> 16 ) & 0xFF;
    entry_bytes[ 14 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >>  8 ) & 0xFF;
    entry_bytes[ 15 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >>  0 ) & 0xFF;

    rdd_error = f_rdd_search_iptv_entry ( xi_entry, entry_bytes, xi_iptv_mode, xi_ipv6_dst_ip_ptr, xo_index );

    if ( ( xi_entry->l3_entry_fields.src_ip.addr.ipv4 != 0 ) && ( rdd_error == BL_LILAC_RDD_OK ) )
    {
        rdd_error = f_rdd_search_iptv_ssm_entry ( xi_entry, xi_ipv6_src_ip_ptr, xo_index );
    }

    return ( rdd_error );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_get_iptv_l3_dst_ip_entry_index ( RDD_IPTV_ENTRY_UNION  *xi_entry,
                                                                     bdmf_ip_t             *xi_ipv6_dst_ip_ptr,
                                                                     bdmf_ip_t             *xi_ipv6_src_ip_ptr,
                                                                     uint32_t              *xo_index )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint8_t                 entry_bytes[ LILAC_RDD_IPTV_ENTRY_SIZE ];

    MEMSET ( entry_bytes, 0, LILAC_RDD_IPTV_ENTRY_SIZE );

    if ( xi_ipv6_dst_ip_ptr != NULL ) 
    {
        entry_bytes[ 4 ] = xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 12 ];
        entry_bytes[ 5 ] = xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 13 ];
        entry_bytes[ 6 ] = xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 14 ];
        entry_bytes[ 7 ] = xi_ipv6_dst_ip_ptr->addr.ipv6.data[ 15 ];
    }

    entry_bytes[ 12 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >> 24 ) & 0xFF;
    entry_bytes[ 13 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >> 16 ) & 0xFF;
    entry_bytes[ 14 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >>  8 ) & 0xFF;
    entry_bytes[ 15 ] = ( xi_entry->l3_entry_fields.dst_ip.addr.ipv4 >>  0 ) & 0xFF;

    rdd_error = f_rdd_search_iptv_entry ( xi_entry, entry_bytes, iptv_lookup_method_group_ip, xi_ipv6_dst_ip_ptr, xo_index );

    return ( rdd_error );
}


BL_LILAC_RDD_ERROR_DTE rdd_iptv_entry_search ( RDD_IPTV_ENTRY_UNION  *xi_entry,
                                               uint32_t              *xo_index )
{
    rdpa_iptv_lookup_method                iptv_classification_mode;
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS  *bridge_cfg_register;
    bdmf_ip_t                              ipv6_dst_ip_copy;
    bdmf_ip_t                              ipv6_src_ip_copy;
    bdmf_ip_t                              ipv6_src_ip_any;
#if !defined(FIRMWARE_INIT)
    uint32_t                               *ipv6_buffer_ptr;
    unsigned long                          flags;
#else
    uint32_t                               crc_init_value;
    uint32_t                               crc_result;
#endif
    BL_LILAC_RDD_ERROR_DTE                 rdd_error;

    rdd_error = BL_LILAC_RDD_OK;

    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );

    RDD_BRIDGE_CONFIGURATION_REGISTER_IPTV_CLASSIFICATION_METHOD_READ ( iptv_classification_mode, bridge_cfg_register );

    f_rdd_lock ( &int_lock );

#if !defined(FIRMWARE_INIT)

    if ( xi_entry->l3_entry_fields.dst_ip.family == bdmf_ip_family_ipv6 )
    {
        ipv6_buffer_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + HASH_BUFFER_ADDRESS );

        MWRITE_BLK_8( ipv6_buffer_ptr, xi_entry->l3_entry_fields.dst_ip.addr.ipv6.data, 16 );

        f_rdd_lock_irq ( &int_lock_irq, &flags );

        rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_IPV6_CRC_GET, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, 0, 0, 0, BL_LILAC_RDD_WAIT );

        f_rdd_unlock_irq ( &int_lock_irq, flags );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            f_rdd_unlock ( &int_lock );
            return ( rdd_error );
        }
        
        memcpy ( ipv6_dst_ip_copy.addr.ipv6.data, xi_entry->l3_entry_fields.dst_ip.addr.ipv6.data, 16 );

        xi_entry->l3_entry_fields.dst_ip.addr.ipv4 = *( volatile uint32_t * )ipv6_buffer_ptr;

        if ( ( iptv_classification_mode == iptv_lookup_method_group_ip_src_ip ) || ( iptv_classification_mode == iptv_lookup_method_group_ip_src_ip_vid ) )
        {
            MEMSET ( ipv6_src_ip_any.addr.ipv6.data, 0, 16 );

            if ( memcmp ( ipv6_src_ip_any.addr.ipv6.data, xi_entry->l3_entry_fields.src_ip.addr.ipv6.data, 16 ) )
            {
                MWRITE_BLK_8( ipv6_buffer_ptr, xi_entry->l3_entry_fields.src_ip.addr.ipv6.data, 16 );

                f_rdd_lock_irq ( &int_lock_irq, &flags );

                rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_IPV6_CRC_GET, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, 0, 0, 0, BL_LILAC_RDD_WAIT );

                f_rdd_unlock_irq ( &int_lock_irq, flags );

                if ( rdd_error != BL_LILAC_RDD_OK )
                {
                    f_rdd_unlock ( &int_lock );
                    return ( rdd_error );
                }

                memcpy ( ipv6_src_ip_copy.addr.ipv6.data, xi_entry->l3_entry_fields.src_ip.addr.ipv6.data, 16 );

                xi_entry->l3_entry_fields.src_ip.addr.ipv4 = *( volatile uint32_t * )ipv6_buffer_ptr;
            }
            else
            {
                MEMSET ( ipv6_src_ip_copy.addr.ipv6.data, 0, 16 );

                xi_entry->l3_entry_fields.src_ip.addr.ipv4 = 0;
            }
        }
    }
#else

    if ( xi_entry->l3_entry_fields.dst_ip.family == bdmf_ip_family_ipv6 )
    {
        memcpy ( ipv6_dst_ip_copy.addr.ipv6.data, xi_entry->l3_entry_fields.dst_ip.addr.ipv6.data, 16 );

        crc_init_value = rdd_crc_init_value_get ( RDD_CRC_TYPE_32 );
        crc_result = rdd_crc_bit_by_bit ( &(xi_entry->l3_entry_fields.dst_ip.addr.ipv6.data[0]), 16, 0, crc_init_value, RDD_CRC_TYPE_32 );
        xi_entry->l3_entry_fields.dst_ip.addr.ipv4 = crc_result;
        
        if ( ( iptv_classification_mode == iptv_lookup_method_group_ip_src_ip ) || ( iptv_classification_mode == iptv_lookup_method_group_ip_src_ip_vid ) )
        {
            MEMSET ( ipv6_src_ip_any.addr.ipv6.data, 0, 16 );

            if ( memcmp ( ipv6_src_ip_any.addr.ipv6.data, xi_entry->l3_entry_fields.src_ip.addr.ipv6.data, 16 ) )
            {
                memcpy ( ipv6_src_ip_copy.addr.ipv6.data, xi_entry->l3_entry_fields.src_ip.addr.ipv6.data, 16 );

                crc_init_value = rdd_crc_init_value_get ( RDD_CRC_TYPE_32 );
                crc_result = rdd_crc_bit_by_bit ( &(xi_entry->l3_entry_fields.src_ip.addr.ipv6.data[0]), 16, 0, crc_init_value, RDD_CRC_TYPE_32 );
                xi_entry->l3_entry_fields.src_ip.addr.ipv4 = crc_result;
            }
            else
            {
                MEMSET ( ipv6_src_ip_copy.addr.ipv6.data, 0, 16 );

                xi_entry->l3_entry_fields.src_ip.addr.ipv4 = 0;
            }
        }
    }
#endif

    switch ( iptv_classification_mode )
    {
    case iptv_lookup_method_mac:

         rdd_error = f_rdd_get_iptv_l2_mac_entry_index ( xi_entry, xo_index );
         break;

    case iptv_lookup_method_mac_vid:

         rdd_error = f_rdd_get_iptv_l2_mac_vid_entry_index ( xi_entry, xo_index );
         break;

    case iptv_lookup_method_group_ip_src_ip:

        if ( xi_entry->l3_entry_fields.dst_ip.family == bdmf_ip_family_ipv6 )
        {
            rdd_error = f_rdd_get_iptv_l3_dst_ip_src_ip_entry_index ( xi_entry, iptv_classification_mode, &ipv6_dst_ip_copy, &ipv6_src_ip_copy, xo_index );
        }
        else
        {
            rdd_error = f_rdd_get_iptv_l3_dst_ip_src_ip_entry_index ( xi_entry, iptv_classification_mode, 0, 0, xo_index );
        }
        break;

    case iptv_lookup_method_group_ip:

        if ( xi_entry->l3_entry_fields.dst_ip.family == bdmf_ip_family_ipv6 )
        {
            rdd_error = f_rdd_get_iptv_l3_dst_ip_entry_index ( xi_entry, &ipv6_dst_ip_copy, &ipv6_src_ip_copy, xo_index );
        }
        else
        {
            rdd_error = f_rdd_get_iptv_l3_dst_ip_entry_index ( xi_entry, 0, 0, xo_index );
        }
        break;

    case iptv_lookup_method_group_ip_src_ip_vid:

        if ( xi_entry->l3_entry_fields.dst_ip.family == bdmf_ip_family_ipv6 )
        {
            rdd_error = f_rdd_get_iptv_l3_dst_ip_src_ip_entry_index ( xi_entry, iptv_classification_mode, &ipv6_dst_ip_copy, &ipv6_src_ip_copy, xo_index );
        }
        else
        {
            rdd_error = f_rdd_get_iptv_l3_dst_ip_src_ip_entry_index ( xi_entry, iptv_classification_mode, 0, 0, xo_index );
        }
        break;
    }

    f_rdd_unlock ( &int_lock );
    return ( rdd_error );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_read_iptv_ssm_entry ( uint32_t                 xi_entry_index,
                                                          RDD_IPTV_ENTRY_UNION     *xo_entry,
                                                          rdpa_iptv_lookup_method  xi_iptv_mode,
                                                          bdmf_ip_t                *xo_ipv6_dst_ip_ptr )
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS        *iptv_table_ptr;
    RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS   *iptv_ssm_context_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS        *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS       *iptv_context_entry_ptr;
    RDD_IPTV_L3_SRC_IP_LOOKUP_TABLE_DTS  *iptv_layer3_src_ip_table_ptr;
    RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS  *iptv_layer3_src_ip_entry_ptr;
    uint32_t                             iptv_entry_valid;
    uint32_t                             provider_index;
    uint32_t                             context_table_index;
    uint32_t                             base_index;
    uint8_t                              ip_addr[ 4 ];

    provider_index = GET_PROVIDER_INDEX_FROM_ENTRY_INDEX ( xi_entry_index );

    base_index = GET_BASE_INDEX_FROM_ENTRY_INDEX ( xi_entry_index );

    iptv_table_ptr = ( RDD_IPTV_DDR_LOOKUP_TABLE_DTS * )IPTVTableBase;
    iptv_entry_ptr = &( iptv_table_ptr->entry[ base_index ] );

    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_VALID_READ ( iptv_entry_valid, iptv_entry_ptr );

    if ( !( iptv_entry_valid ) )
    {
        return ( BL_LILAC_RDD_ERROR_IPTV_TABLE_ENTRY_NOT_EXISTS );
    }

    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP12_READ ( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 12 ], iptv_entry_ptr );
    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP13_READ ( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 13 ], iptv_entry_ptr );
    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP14_READ ( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 14 ], iptv_entry_ptr );
    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP15_READ ( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 15 ], iptv_entry_ptr );

    if ( *( ( uint32_t * )&( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 12 ] ) ) != 0 ) 
    {
        xo_entry->l3_entry_fields.dst_ip.family = bdmf_ip_family_ipv6;
        xo_entry->l3_entry_fields.src_ip.family = bdmf_ip_family_ipv6;
    }
    else
    {
        xo_entry->l3_entry_fields.dst_ip.family = bdmf_ip_family_ipv4;
        xo_entry->l3_entry_fields.src_ip.family = bdmf_ip_family_ipv4;
    }

    if ( xo_entry->l3_entry_fields.dst_ip.family == bdmf_ip_family_ipv6 ) 
    {
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_READ ( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 3 ], iptv_entry_ptr );
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_READ ( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 2 ], iptv_entry_ptr );
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_READ ( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 1 ], iptv_entry_ptr );
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_READ ( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 0 ], iptv_entry_ptr );
    }
    else
    {
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_READ ( ip_addr[ 0 ], iptv_entry_ptr );
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_READ ( ip_addr[ 1 ], iptv_entry_ptr );
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_READ ( ip_addr[ 2 ], iptv_entry_ptr );
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_READ ( ip_addr[ 3 ], iptv_entry_ptr );

        xo_entry->l3_entry_fields.dst_ip.addr.ipv4  = ( ip_addr[ 0 ] << 24 );
        xo_entry->l3_entry_fields.dst_ip.addr.ipv4 |= ( ip_addr[ 1 ] << 16 );
        xo_entry->l3_entry_fields.dst_ip.addr.ipv4 |= ( ip_addr[ 2 ] <<  8 );
        xo_entry->l3_entry_fields.dst_ip.addr.ipv4 |= ( ip_addr[ 3 ] <<  0 );
    }

    if ( xi_iptv_mode == iptv_lookup_method_group_ip_src_ip_vid )
    {
        RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_VID_READ ( xo_entry->l3_entry_fields.vid, iptv_entry_ptr );
    }

    iptv_layer3_src_ip_table_ptr = ( RDD_IPTV_L3_SRC_IP_LOOKUP_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + IPTV_L3_SRC_IP_LOOKUP_TABLE_ADDRESS );

    iptv_layer3_src_ip_entry_ptr = &( iptv_layer3_src_ip_table_ptr->entry[ provider_index ] );

    if ( xo_entry->l3_entry_fields.src_ip.family == bdmf_ip_family_ipv6 ) 
    {
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_12_READ ( xo_entry->l3_entry_fields.src_ip.addr.ipv6.data[ 12 ], ( RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS * )iptv_layer3_src_ip_entry_ptr );
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_13_READ ( xo_entry->l3_entry_fields.src_ip.addr.ipv6.data[ 13 ], ( RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS * )iptv_layer3_src_ip_entry_ptr );
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_14_READ ( xo_entry->l3_entry_fields.src_ip.addr.ipv6.data[ 14 ], ( RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS * )iptv_layer3_src_ip_entry_ptr );
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_15_READ ( xo_entry->l3_entry_fields.src_ip.addr.ipv6.data[ 15 ], ( RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS * )iptv_layer3_src_ip_entry_ptr );

        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_0_READ ( xo_entry->l3_entry_fields.src_ip.addr.ipv6.data[ 3 ], ( RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS * )iptv_layer3_src_ip_entry_ptr );
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_1_READ ( xo_entry->l3_entry_fields.src_ip.addr.ipv6.data[ 2 ], ( RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS * )iptv_layer3_src_ip_entry_ptr );
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_2_READ ( xo_entry->l3_entry_fields.src_ip.addr.ipv6.data[ 1 ], ( RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS * )iptv_layer3_src_ip_entry_ptr );
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_3_READ ( xo_entry->l3_entry_fields.src_ip.addr.ipv6.data[ 0 ], ( RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS * )iptv_layer3_src_ip_entry_ptr );
    }
    else
    {
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_0_READ ( ip_addr[ 0 ], ( RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS * )iptv_layer3_src_ip_entry_ptr );
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_1_READ ( ip_addr[ 1 ], ( RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS * )iptv_layer3_src_ip_entry_ptr );
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_2_READ ( ip_addr[ 2 ], ( RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS * )iptv_layer3_src_ip_entry_ptr );
        RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_3_READ ( ip_addr[ 3 ], ( RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS * )iptv_layer3_src_ip_entry_ptr );

        xo_entry->l3_entry_fields.src_ip.addr.ipv4  = ( ip_addr[ 0 ] << 24 );
        xo_entry->l3_entry_fields.src_ip.addr.ipv4 |= ( ip_addr[ 1 ] << 16 );
        xo_entry->l3_entry_fields.src_ip.addr.ipv4 |= ( ip_addr[ 2 ] <<  8 );
        xo_entry->l3_entry_fields.src_ip.addr.ipv4 |= ( ip_addr[ 3 ] <<  0 );
    }

    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_TABLE_READ ( context_table_index, iptv_entry_ptr );

    iptv_ssm_context_table_ptr = ( RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS * )IPTVSsmContextTableBase;
    iptv_context_entry_ptr = &( iptv_ssm_context_table_ptr->entry[ context_table_index * LILAC_RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS + provider_index ] );

    RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_READ ( iptv_entry_valid, iptv_context_entry_ptr );

    if ( !( iptv_entry_valid ) )
    {
        return ( BL_LILAC_RDD_ERROR_IPTV_TABLE_ENTRY_NOT_EXISTS );
    }

    RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_READ ( xo_entry->l3_entry_fields.egress_port_vector, iptv_context_entry_ptr );
    RDD_IPTV_DDR_CONTEXT_ENTRY_WLAN_MCAST_INDEX_READ ( xo_entry->l3_entry_fields.wlan_mcast_index, iptv_context_entry_ptr );
    RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_READ ( xo_entry->l3_entry_fields.ingress_classification_context, iptv_context_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_read_iptv_entry ( uint32_t                 xi_entry_index,
                                                      rdpa_iptv_lookup_method  xi_iptv_mode,
                                                      RDD_IPTV_ENTRY_UNION     *xo_entry,
                                                      bdmf_ip_t                *xo_ipv6_dst_ip_ptr )
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS   *iptv_table_ptr;
    RDD_IPTV_DDR_CONTEXT_TABLE_DTS  *iptv_context_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS   *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS  *iptv_context_entry_ptr;
    uint32_t                        iptv_entry_valid;
    uint32_t                        any;
    uint8_t                         dst_ip_addr[ 4 ];

    iptv_table_ptr = ( RDD_IPTV_DDR_LOOKUP_TABLE_DTS * )IPTVTableBase;
    iptv_entry_ptr = &( iptv_table_ptr->entry[ xi_entry_index ] );

    RDD_IPTV_L2_DDR_LOOKUP_ENTRY_VALID_READ ( iptv_entry_valid, iptv_entry_ptr );

    if ( !( iptv_entry_valid ) )
    {
        return ( BL_LILAC_RDD_ERROR_IPTV_TABLE_ENTRY_NOT_EXISTS );
    }

    if ( ( xi_iptv_mode == iptv_lookup_method_group_ip_src_ip ) || ( xi_iptv_mode == iptv_lookup_method_group_ip_src_ip_vid ) )
    {
        RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_ANY_READ ( any, iptv_entry_ptr );

        if ( !( any ) )
        {
            return ( BL_LILAC_RDD_ERROR_IPTV_TABLE_ENTRY_NOT_EXISTS );
        }
    }

    if ( ( xi_iptv_mode == iptv_lookup_method_group_ip_src_ip ) || ( xi_iptv_mode == iptv_lookup_method_group_ip ) )
    {
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP12_READ ( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 12 ], iptv_entry_ptr );
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP13_READ ( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 13 ], iptv_entry_ptr );
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP14_READ ( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 14 ], iptv_entry_ptr );
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP15_READ ( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 15 ], iptv_entry_ptr );
        
        if ( *( ( uint32_t * )&( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 12 ] ) ) != 0 ) 
        {
            xo_entry->l3_entry_fields.dst_ip.family = bdmf_ip_family_ipv6;
            xo_entry->l3_entry_fields.src_ip.family = bdmf_ip_family_ipv6;
        }
        else
        {
            xo_entry->l3_entry_fields.dst_ip.family = bdmf_ip_family_ipv4;
            xo_entry->l3_entry_fields.src_ip.family = bdmf_ip_family_ipv4;
        }

        if ( xo_entry->l3_entry_fields.dst_ip.family == bdmf_ip_family_ipv6 ) 
        {
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_READ ( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 3 ], iptv_entry_ptr );
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_READ ( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 2 ], iptv_entry_ptr );
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_READ ( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 1 ], iptv_entry_ptr );
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_READ ( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 0 ], iptv_entry_ptr );
        }
        else
        {
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_READ ( dst_ip_addr[ 0 ], iptv_entry_ptr );
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_READ ( dst_ip_addr[ 1 ], iptv_entry_ptr );
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_READ ( dst_ip_addr[ 2 ], iptv_entry_ptr );
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_READ ( dst_ip_addr[ 3 ], iptv_entry_ptr );

            xo_entry->l3_entry_fields.dst_ip.addr.ipv4  = ( dst_ip_addr[ 0 ] << 24 );
            xo_entry->l3_entry_fields.dst_ip.addr.ipv4 |= ( dst_ip_addr[ 1 ] << 16 );
            xo_entry->l3_entry_fields.dst_ip.addr.ipv4 |= ( dst_ip_addr[ 2 ] <<  8 );
            xo_entry->l3_entry_fields.dst_ip.addr.ipv4 |= ( dst_ip_addr[ 3 ] <<  0 );
        }
    }
    else if ( xi_iptv_mode == iptv_lookup_method_group_ip_src_ip_vid )
    {
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP12_READ ( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 12 ], iptv_entry_ptr );
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP13_READ ( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 13 ], iptv_entry_ptr );
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP14_READ ( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 14 ], iptv_entry_ptr );
        RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP15_READ ( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 15 ], iptv_entry_ptr );
        
        if ( *((uint32_t *)&(xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 12 ])) != 0 ) 
        {
            xo_entry->l3_entry_fields.dst_ip.family = bdmf_ip_family_ipv6;
            xo_entry->l3_entry_fields.src_ip.family = bdmf_ip_family_ipv6;
        }
        else
        {
            xo_entry->l3_entry_fields.dst_ip.family = bdmf_ip_family_ipv4;
            xo_entry->l3_entry_fields.src_ip.family = bdmf_ip_family_ipv4;
        }

        if ( xo_entry->l3_entry_fields.dst_ip.family == bdmf_ip_family_ipv6 ) 
        {
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_READ ( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 3 ], iptv_entry_ptr );
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_READ ( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 2 ], iptv_entry_ptr );
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_READ ( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 1 ], iptv_entry_ptr );
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_READ ( xo_entry->l3_entry_fields.dst_ip.addr.ipv6.data[ 0 ], iptv_entry_ptr );
        }
        else
        {
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_READ ( dst_ip_addr[ 0 ], iptv_entry_ptr );
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_READ ( dst_ip_addr[ 1 ], iptv_entry_ptr );
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_READ ( dst_ip_addr[ 2 ], iptv_entry_ptr );
            RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_READ ( dst_ip_addr[ 3 ], iptv_entry_ptr );

            xo_entry->l3_entry_fields.dst_ip.addr.ipv4  = ( dst_ip_addr[ 0 ] << 24 );
            xo_entry->l3_entry_fields.dst_ip.addr.ipv4 |= ( dst_ip_addr[ 1 ] << 16 );
            xo_entry->l3_entry_fields.dst_ip.addr.ipv4 |= ( dst_ip_addr[ 2 ] <<  8 );
            xo_entry->l3_entry_fields.dst_ip.addr.ipv4 |= ( dst_ip_addr[ 3 ] <<  0 );
        }

        RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_VID_READ ( xo_entry->l3_entry_fields.vid, iptv_entry_ptr );
    }
    else
    {
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR5_READ ( xo_entry->l2_entry_fields.mac_addr.b[ 5 ], iptv_entry_ptr );
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR4_READ ( xo_entry->l2_entry_fields.mac_addr.b[ 4 ], iptv_entry_ptr );
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR3_READ ( xo_entry->l2_entry_fields.mac_addr.b[ 3 ], iptv_entry_ptr );
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR2_READ ( xo_entry->l2_entry_fields.mac_addr.b[ 2 ], iptv_entry_ptr );
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR1_READ ( xo_entry->l2_entry_fields.mac_addr.b[ 1 ], iptv_entry_ptr );
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR0_READ ( xo_entry->l2_entry_fields.mac_addr.b[ 0 ], iptv_entry_ptr );
        RDD_IPTV_L2_DDR_LOOKUP_ENTRY_VID_READ ( xo_entry->l2_entry_fields.vid, iptv_entry_ptr );
    }

    if ( ( xi_iptv_mode == iptv_lookup_method_group_ip ) ||
         ( ( xi_iptv_mode == iptv_lookup_method_group_ip_src_ip ) && ( xi_entry_index < RDD_IPTV_DDR_LOOKUP_TABLE_SIZE ) ) ||
         ( ( xi_iptv_mode == iptv_lookup_method_group_ip_src_ip_vid ) && ( xi_entry_index < RDD_IPTV_DDR_LOOKUP_TABLE_SIZE ) ) )
    {
        if ( xo_entry->l3_entry_fields.dst_ip.family == bdmf_ip_family_ipv6 ) 
        {
            MEMSET ( xo_entry->l3_entry_fields.src_ip.addr.ipv6.data, 0, 16 );
        }
        else
        {
            xo_entry->l3_entry_fields.src_ip.addr.ipv4 = 0;
        }
    }

    iptv_context_table_ptr = ( RDD_IPTV_DDR_CONTEXT_TABLE_DTS * )IPTVContextTableBase;
    iptv_context_entry_ptr = &( iptv_context_table_ptr->entry[ xi_entry_index ] );

    RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_READ ( xo_entry->l2_entry_fields.egress_port_vector, iptv_context_entry_ptr );

    RDD_IPTV_DDR_CONTEXT_ENTRY_WLAN_MCAST_INDEX_READ ( xo_entry->l2_entry_fields.wlan_mcast_index, iptv_context_entry_ptr );

    RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_READ ( xo_entry->l3_entry_fields.ingress_classification_context, iptv_context_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_get_iptv_l2_mac_entry ( uint32_t              xi_entry_index,
                                                            RDD_IPTV_ENTRY_UNION  *xo_entry )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;

    rdd_error = f_rdd_read_iptv_entry ( xi_entry_index, iptv_lookup_method_mac, xo_entry, 0 );

    return ( rdd_error );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_get_iptv_l2_mac_vid_entry ( uint32_t              xi_entry_index,
                                                                RDD_IPTV_ENTRY_UNION  *xo_entry )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;

    rdd_error = f_rdd_read_iptv_entry ( xi_entry_index, iptv_lookup_method_mac_vid, xo_entry, 0 );

    return ( rdd_error );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_get_iptv_ssm_entry ( uint32_t                 xi_entry_index,
                                                         RDD_IPTV_ENTRY_UNION     *xo_entry,
                                                         rdpa_iptv_lookup_method  iptv_mode,
                                                         bdmf_ip_t                *xo_ipv6_dst_ip )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;

    if ( xi_entry_index < RDD_IPTV_DDR_LOOKUP_TABLE_SIZE )
    {
        rdd_error = f_rdd_read_iptv_entry ( xi_entry_index, iptv_mode, xo_entry, xo_ipv6_dst_ip );
    }
    else
    {
        rdd_error = f_rdd_read_iptv_ssm_entry ( xi_entry_index, xo_entry, iptv_mode, xo_ipv6_dst_ip );
    }

    return ( rdd_error );
}


static inline BL_LILAC_RDD_ERROR_DTE f_rdd_get_iptv_l3_dst_ip_src_ip_entry ( uint32_t              xi_entry_index,
                                                                             RDD_IPTV_ENTRY_UNION  *xo_entry,
                                                                             bdmf_ip_t             *xo_ipv6_dst_ip )
{
    return ( f_rdd_get_iptv_ssm_entry( xi_entry_index, xo_entry, iptv_lookup_method_group_ip_src_ip, xo_ipv6_dst_ip ) );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_get_iptv_l3_dst_ip_entry ( uint32_t              xi_entry_index,
                                                               RDD_IPTV_ENTRY_UNION  *xo_entry,
                                                               bdmf_ip_t             *xo_ipv6_dst_ip )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;

    rdd_error = f_rdd_read_iptv_entry ( xi_entry_index, iptv_lookup_method_group_ip, xo_entry, xo_ipv6_dst_ip );

    return ( rdd_error );
}


static inline BL_LILAC_RDD_ERROR_DTE f_rdd_get_iptv_l3_dst_ip_src_ip_vid_entry ( uint32_t              xi_entry_index,
                                                                                 RDD_IPTV_ENTRY_UNION  *xo_entry,
                                                                                 bdmf_ip_t             *xo_ipv6_dst_ip )
{
    return ( f_rdd_get_iptv_ssm_entry( xi_entry_index, xo_entry, iptv_lookup_method_group_ip_src_ip_vid, xo_ipv6_dst_ip ) );
}


BL_LILAC_RDD_ERROR_DTE rdd_iptv_entry_get ( uint32_t              xi_entry_index,
                                            RDD_IPTV_ENTRY_UNION  *xo_entry )
{
    rdpa_iptv_lookup_method                iptv_classification_mode;
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS  *bridge_cfg_register;
    BL_LILAC_RDD_ERROR_DTE                 rdd_error;
    bdmf_ip_t                              ipv6_dst_ip_ptr;

    rdd_error = BL_LILAC_RDD_OK;

    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );

    RDD_BRIDGE_CONFIGURATION_REGISTER_IPTV_CLASSIFICATION_METHOD_READ ( iptv_classification_mode, bridge_cfg_register );

    f_rdd_lock ( &int_lock );

    switch ( iptv_classification_mode )
    {
    case iptv_lookup_method_mac:

         rdd_error = f_rdd_get_iptv_l2_mac_entry ( xi_entry_index, xo_entry );
         break;

    case iptv_lookup_method_mac_vid:

         rdd_error = f_rdd_get_iptv_l2_mac_vid_entry ( xi_entry_index, xo_entry );
         break;

    case iptv_lookup_method_group_ip_src_ip:

         rdd_error = f_rdd_get_iptv_l3_dst_ip_src_ip_entry ( xi_entry_index, xo_entry, &ipv6_dst_ip_ptr );
         break;

    case iptv_lookup_method_group_ip:

        rdd_error = f_rdd_get_iptv_l3_dst_ip_entry ( xi_entry_index, xo_entry, &ipv6_dst_ip_ptr );
        break;

    case iptv_lookup_method_group_ip_src_ip_vid:

        rdd_error = f_rdd_get_iptv_l3_dst_ip_src_ip_vid_entry ( xi_entry_index, xo_entry, &ipv6_dst_ip_ptr );
        break;
    }

    f_rdd_unlock ( &int_lock );
    return ( rdd_error );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_read_iptv_ssm_entry_counter ( uint32_t  xi_entry_index,
                                                                  uint16_t  *xo_counter_value )
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS       *iptv_table_ptr;
    RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS  *iptv_ssm_context_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS       *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS      *iptv_context_entry_ptr;
    uint32_t                            iptv_entry_valid;
    uint32_t                            provider_index;
    uint32_t                            context_table_index;
    uint32_t                            base_index;

    provider_index = GET_PROVIDER_INDEX_FROM_ENTRY_INDEX ( xi_entry_index );

    base_index =  GET_BASE_INDEX_FROM_ENTRY_INDEX ( xi_entry_index );

    iptv_table_ptr = ( RDD_IPTV_DDR_LOOKUP_TABLE_DTS * )IPTVTableBase;
    iptv_entry_ptr = &( iptv_table_ptr->entry[ base_index ] );

    RDD_IPTV_L3_DDR_LOOKUP_ENTRY_VALID_READ ( iptv_entry_valid, iptv_entry_ptr );

    if ( !( iptv_entry_valid ) )
    {
        return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
    }

    RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_TABLE_READ ( context_table_index, iptv_entry_ptr );

    iptv_ssm_context_table_ptr = ( RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS * )IPTVSsmContextTableBase;
    iptv_context_entry_ptr = &( iptv_ssm_context_table_ptr->entry[ context_table_index * LILAC_RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS + provider_index ] );

    RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_READ ( iptv_entry_valid, iptv_context_entry_ptr );

    if ( !( iptv_entry_valid ) )
    {
        return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
    }

    RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_READ ( *xo_counter_value, iptv_context_entry_ptr );
    RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_WRITE ( 0, iptv_context_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}

static BL_LILAC_RDD_ERROR_DTE f_rdd_read_iptv_ssm_any_entry_counter ( uint32_t  xi_entry_index,
                                                                      uint16_t  *xo_counter_value )
{
    RDD_IPTV_DDR_CONTEXT_TABLE_DTS  *iptv_context_table_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS  *iptv_context_entry_ptr;
    BL_LILAC_RDD_ERROR_DTE          rdd_error;
    uint32_t                        iptv_context_entry_cache;
    uint32_t                        cache_index;

    iptv_context_table_ptr = ( RDD_IPTV_DDR_CONTEXT_TABLE_DTS * )IPTVContextTableBase;
    iptv_context_entry_ptr = &( iptv_context_table_ptr->entry[ xi_entry_index ] );

    RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_READ ( iptv_context_entry_cache, iptv_context_entry_ptr );

    if ( iptv_context_entry_cache )
    {
        RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_READ ( cache_index, iptv_context_entry_ptr );

        rdd_error = f_rdd_read_iptv_counter_cache ( cache_index, xo_counter_value );

        return ( rdd_error );
    }

    RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_READ ( *xo_counter_value, iptv_context_entry_ptr );
    RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_WRITE ( 0, iptv_context_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_read_iptv_entry_counter ( uint32_t  xi_entry_index,
                                                              uint16_t  *xo_counter_value )
{
    RDD_IPTV_DDR_LOOKUP_TABLE_DTS   *iptv_table_ptr;
    RDD_IPTV_DDR_CONTEXT_TABLE_DTS  *iptv_context_table_ptr;
    RDD_IPTV_LOOKUP_DDR_UNION_DTS   *iptv_entry_ptr;
    RDD_IPTV_DDR_CONTEXT_ENTRY_DTS  *iptv_context_entry_ptr;
    BL_LILAC_RDD_ERROR_DTE          rdd_error;
    uint32_t                        iptv_entry_valid;
    uint32_t                        iptv_context_entry_cache;
    uint32_t                        cache_index;

    iptv_context_table_ptr = ( RDD_IPTV_DDR_CONTEXT_TABLE_DTS * )IPTVContextTableBase;
    iptv_context_entry_ptr = &( iptv_context_table_ptr->entry[ xi_entry_index ] );

    RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_READ ( iptv_context_entry_cache, iptv_context_entry_ptr );

    if ( iptv_context_entry_cache )
    {
        RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_READ ( cache_index, iptv_context_entry_ptr );

        rdd_error = f_rdd_read_iptv_counter_cache ( cache_index, xo_counter_value );

        return ( rdd_error );
    }

    iptv_table_ptr = ( RDD_IPTV_DDR_LOOKUP_TABLE_DTS * )IPTVTableBase;
    iptv_entry_ptr = &( iptv_table_ptr->entry[ xi_entry_index ] );

    RDD_IPTV_L2_DDR_LOOKUP_ENTRY_VALID_READ ( iptv_entry_valid, iptv_entry_ptr );

    if ( !( iptv_entry_valid ) )
    {
        return ( BL_LILAC_RDD_ERROR_GET_LOOKUP_ENTRY );
    }

    RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_READ ( *xo_counter_value, iptv_context_entry_ptr );
    RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_WRITE ( 0, iptv_context_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


static inline BL_LILAC_RDD_ERROR_DTE f_rdd_read_iptv_l2_mac_counter ( uint32_t  xi_entry_index,
                                                                      uint16_t  *xo_counter_value )
{
    return ( f_rdd_read_iptv_entry_counter ( xi_entry_index, xo_counter_value ) );
}


static inline BL_LILAC_RDD_ERROR_DTE f_rdd_read_iptv_l2_mac_vid_counter ( uint32_t  xi_entry_index,
                                                                          uint16_t  *xo_counter_value )
{
    return ( f_rdd_read_iptv_entry_counter ( xi_entry_index, xo_counter_value ) );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_read_iptv_l3_dst_ip_src_ip_counter ( uint32_t  xi_entry_index,
                                                                         uint16_t  *xo_counter_value )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;

    if ( xi_entry_index < RDD_IPTV_DDR_LOOKUP_TABLE_SIZE )
    {
        rdd_error = f_rdd_read_iptv_ssm_any_entry_counter ( xi_entry_index, xo_counter_value );
    }
    else
    {
        rdd_error = f_rdd_read_iptv_ssm_entry_counter ( xi_entry_index, xo_counter_value );
    }

    return ( rdd_error );
}


static inline BL_LILAC_RDD_ERROR_DTE f_rdd_read_iptv_l3_dst_ip_counter ( uint32_t  xi_entry_index,
                                                                         uint16_t  *xo_counter_value )
{
    return ( f_rdd_read_iptv_entry_counter ( xi_entry_index, xo_counter_value ) );
}


static inline BL_LILAC_RDD_ERROR_DTE f_rdd_read_iptv_l3_dst_ip_src_ip_vid_counter ( uint32_t  xi_entry_index,
                                                                                    uint16_t  *xo_counter_value )
{
    return ( f_rdd_read_iptv_l3_dst_ip_src_ip_counter ( xi_entry_index, xo_counter_value ) );
}


BL_LILAC_RDD_ERROR_DTE rdd_iptv_counter_get ( uint32_t  xi_entry_index,
                                              uint16_t  *xo_counter_value )
{
    rdpa_iptv_lookup_method                iptv_classification_mode;
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS  *bridge_cfg_register;
    BL_LILAC_RDD_ERROR_DTE                 rdd_error;

    rdd_error = BL_LILAC_RDD_OK;

    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );

    RDD_BRIDGE_CONFIGURATION_REGISTER_IPTV_CLASSIFICATION_METHOD_READ ( iptv_classification_mode, bridge_cfg_register );

    switch ( iptv_classification_mode )
    {
    case iptv_lookup_method_mac:

         rdd_error = f_rdd_read_iptv_l2_mac_counter ( xi_entry_index, xo_counter_value );
         break;

    case iptv_lookup_method_mac_vid:

         rdd_error = f_rdd_read_iptv_l2_mac_vid_counter ( xi_entry_index, xo_counter_value );
         break;

    case iptv_lookup_method_group_ip_src_ip:

         rdd_error = f_rdd_read_iptv_l3_dst_ip_src_ip_counter ( xi_entry_index, xo_counter_value );
         break;

    case iptv_lookup_method_group_ip:

        rdd_error = f_rdd_read_iptv_l3_dst_ip_counter ( xi_entry_index, xo_counter_value );
        break;

    case iptv_lookup_method_group_ip_src_ip_vid:

         rdd_error = f_rdd_read_iptv_l3_dst_ip_src_ip_vid_counter ( xi_entry_index, xo_counter_value );
         break;
    }

    return ( rdd_error );
}


/* Tal Meged Debug functions */
BL_LILAC_RDD_ERROR_DTE bl_lilac_rdd_set_iptv_mac_table ( void )
{
    RDD_MAC_TABLE_DTS  *mac_table_ptr;
    RDD_MAC_ENTRY_DTS  *mac_entry_ptr;
    uint32_t           i;

    mac_table_ptr = ( RDD_MAC_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + IPTV_LOOKUP_TABLE_ADDRESS );

    for ( i = 0; i < g_iptv_table_size; i++ )
    {
        /* read MAC table entry and clear it */
        mac_entry_ptr = &( mac_table_ptr->entry[ i ] );

        /* reset the whole MAC table */
        RDD_MAC_ENTRY_VALID_WRITE ( LILAC_RDD_ON, mac_entry_ptr );
    }

    mac_table_ptr = ( RDD_MAC_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + IPTV_LOOKUP_TABLE_CAM_ADDRESS );

    for ( i = 0; i < 32; i++ )
    {
        /* read MAC table entry and clear it */
        mac_entry_ptr = &( mac_table_ptr->entry[ i ] );

        /* reset the whole MAC table */
        RDD_MAC_ENTRY_VALID_WRITE ( LILAC_RDD_ON, mac_entry_ptr );
    }

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE f_rdd_add_iptv_layer3_src_ip ( bdmf_ip_t   *xi_src_ip,
                                                      bdmf_ip_t   *xi_ipv6_src_ip_ptr, 
                                                      uint32_t    *xo_entry_index )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint8_t                 hash_entry[ 8 ];
    uint32_t                entry_index;

    if ( xi_ipv6_src_ip_ptr == NULL ) 
    {
        hash_entry[ 0 ] = 0;
        hash_entry[ 1 ] = 0;
        hash_entry[ 2 ] = 0;
        hash_entry[ 3 ] = 0;
    }
    else
    {
        hash_entry[ 0 ] = xi_ipv6_src_ip_ptr->addr.ipv6.data[ 12 ];
        hash_entry[ 1 ] = xi_ipv6_src_ip_ptr->addr.ipv6.data[ 13 ];
        hash_entry[ 2 ] = xi_ipv6_src_ip_ptr->addr.ipv6.data[ 14 ];
        hash_entry[ 3 ] = xi_ipv6_src_ip_ptr->addr.ipv6.data[ 15 ];
    }

    hash_entry[ 4 ] = ( xi_src_ip->addr.ipv4 >> 24 ) & 0xFF;
    hash_entry[ 5 ] = ( xi_src_ip->addr.ipv4 >> 16 ) & 0xFF;
    hash_entry[ 6 ] = ( xi_src_ip->addr.ipv4 >>  8 ) & 0xFF;
    hash_entry[ 7 ] = ( xi_src_ip->addr.ipv4 >>  0 ) & 0xFF;

    rdd_error = rdd_find_hash_entry_64_bit ( &g_hash_table_cfg[ BL_LILAC_RDD_IPTV_SRC_IP_TABLE ],
                                             hash_entry,
                                             IPTV_L3_SSM_SRC_IP_ENTRY_KEY_MASK_HIGH,
                                             IPTV_L3_SSM_SRC_IP_ENTRY_KEY_MASK_LOW,
                                             0,
                                             &entry_index );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        rdd_error = rdd_add_hash_entry_64_bit ( &g_hash_table_cfg[ BL_LILAC_RDD_IPTV_SRC_IP_TABLE ],
                                                hash_entry,
                                                NULL,
                                                IPTV_L3_SSM_SRC_IP_ENTRY_KEY_MASK_HIGH,
                                                IPTV_L3_SSM_SRC_IP_ENTRY_KEY_MASK_LOW,
                                                0,
                                                &entry_index );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }
    }

    g_iptv_source_ip_counter[ entry_index ]++;

    *xo_entry_index = entry_index;

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE f_rdd_delete_iptv_layer3_src_ip ( bdmf_ip_t   *xi_src_ip,
                                                         uint32_t    *xo_entry_index )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint8_t                 hash_entry[ 8 ];
    uint32_t                entry_index;

    if ( xi_src_ip == NULL )
    {
        return ( BL_LILAC_RDD_ERROR_IPTV_TABLE_ENTRY_NOT_EXISTS );
    }

    if ( xi_src_ip->family == bdmf_ip_family_ipv6 )
    {
        hash_entry[ 0 ] = xi_src_ip->addr.ipv6.data[ 12 ];
        hash_entry[ 1 ] = xi_src_ip->addr.ipv6.data[ 13 ];
        hash_entry[ 2 ] = xi_src_ip->addr.ipv6.data[ 14 ];
        hash_entry[ 3 ] = xi_src_ip->addr.ipv6.data[ 15 ];
        hash_entry[ 4 ] = xi_src_ip->addr.ipv6.data[ 3 ];
        hash_entry[ 5 ] = xi_src_ip->addr.ipv6.data[ 2 ];
        hash_entry[ 6 ] = xi_src_ip->addr.ipv6.data[ 1 ];
        hash_entry[ 7 ] = xi_src_ip->addr.ipv6.data[ 0 ];
    }
    else 
    {
        hash_entry[ 0 ] = 0;
        hash_entry[ 1 ] = 0;
        hash_entry[ 2 ] = 0;
        hash_entry[ 3 ] = 0;
        hash_entry[ 4 ] = ( xi_src_ip->addr.ipv4 >> 24 ) & 0xFF;
        hash_entry[ 5 ] = ( xi_src_ip->addr.ipv4 >> 16 ) & 0xFF;
        hash_entry[ 6 ] = ( xi_src_ip->addr.ipv4 >>  8 ) & 0xFF;
        hash_entry[ 7 ] = ( xi_src_ip->addr.ipv4 >>  0 ) & 0xFF;
    }


    rdd_error = rdd_find_hash_entry_64_bit ( &g_hash_table_cfg[ BL_LILAC_RDD_IPTV_SRC_IP_TABLE ],
                                             hash_entry,
                                             IPTV_L3_SSM_SRC_IP_ENTRY_KEY_MASK_HIGH,
                                             IPTV_L3_SSM_SRC_IP_ENTRY_KEY_MASK_LOW,
                                             0,
                                             &entry_index );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        return ( rdd_error );
    }

    g_iptv_source_ip_counter[ entry_index ]--;

    if ( g_iptv_source_ip_counter[ entry_index ] == 0 )
    {
        rdd_error = rdd_remove_hash_entry_64_bit ( &g_hash_table_cfg[ BL_LILAC_RDD_IPTV_SRC_IP_TABLE ],
                                                   hash_entry,
                                                   IPTV_L3_SSM_SRC_IP_ENTRY_KEY_MASK_HIGH,
                                                   IPTV_L3_SSM_SRC_IP_ENTRY_KEY_MASK_LOW,
                                                   0,
                                                   BL_LILAC_RDD_CAM_OPTIMIZATION_DISABLE,
                                                   &entry_index );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            return ( rdd_error );
        }
    }

    *xo_entry_index = entry_index;

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_vlan_command_config ( rdpa_traffic_dir                        xi_direction,
                                                 rdd_vlan_command_params                 *xi_vlan_command_params )
{
    RDD_DS_VLAN_OPTIMIZATION_TABLE_DTS  *ds_vlan_optimization_table_ptr;
    RDD_US_VLAN_OPTIMIZATION_TABLE_DTS  *us_vlan_optimization_table_ptr;
    RDD_VLAN_OPTIMIZATION_ENTRY_DTS     *vlan_optimization_entry_ptr;
    RDD_DS_VLAN_PARAMETER_TABLE_DTS     *ds_vlan_parameters_table_ptr;
    RDD_US_VLAN_PARAMETER_TABLE_DTS     *us_vlan_parameters_table_ptr;
    RDD_VLAN_PARAMETER_ENTRY_DTS        *vlan_parameters_entry_ptr;
    RDD_DS_PBITS_PARAMETER_TABLE_DTS    *ds_pbits_parameters_table_ptr;
    RDD_US_PBITS_PARAMETER_TABLE_DTS    *us_pbits_parameters_table_ptr;
    RDD_PBITS_PARAMETER_ENTRY_DTS       *pbits_parameters_entry_ptr;
    uint32_t                            vlan_optimize;

    if ( g_vlan_mapping_command_to_action[ xi_vlan_command_params->vlan_command ][ xi_vlan_command_params->pbits_command ] == 0 )
    {
        vlan_optimize = ( LILAC_RDD_TRUE << 7 );
    }
    else
    {
        vlan_optimize = ( LILAC_RDD_FALSE << 7 );
    }

    if ( xi_vlan_command_params->vlan_command == rdd_vlan_command_add_tag_always || xi_vlan_command_params->vlan_command == rdd_vlan_command_remove_tag_always )
    {
        vlan_optimize |= ( LILAC_RDD_TRUE << 6 );
    }

    vlan_optimize |= g_vlan_mapping_command_to_action[ xi_vlan_command_params->vlan_command ][ xi_vlan_command_params->pbits_command ];

    /* write vlan and P-bits parameters */
    if ( xi_direction == rdpa_dir_ds )
    {
         ds_vlan_parameters_table_ptr = RDD_DS_VLAN_PARAMETER_TABLE_PTR();
         vlan_parameters_entry_ptr = &( ds_vlan_parameters_table_ptr->entry[ xi_vlan_command_params->vlan_command_id ] );
         ds_pbits_parameters_table_ptr = RDD_DS_PBITS_PARAMETER_TABLE_PTR();
         pbits_parameters_entry_ptr = &( ds_pbits_parameters_table_ptr->entry[ xi_vlan_command_params->vlan_command_id ] );
         ds_vlan_optimization_table_ptr = RDD_DS_VLAN_OPTIMIZATION_TABLE_PTR();
         vlan_optimization_entry_ptr = &( ds_vlan_optimization_table_ptr->entry[ xi_vlan_command_params->vlan_command_id ] );    }
    else
    {
        us_vlan_parameters_table_ptr = RDD_US_VLAN_PARAMETER_TABLE_PTR();
        vlan_parameters_entry_ptr = &( us_vlan_parameters_table_ptr->entry[ xi_vlan_command_params->vlan_command_id ] );
        us_pbits_parameters_table_ptr = RDD_US_PBITS_PARAMETER_TABLE_PTR();
        pbits_parameters_entry_ptr = &( us_pbits_parameters_table_ptr->entry[ xi_vlan_command_params->vlan_command_id ] );
        us_vlan_optimization_table_ptr = RDD_US_VLAN_OPTIMIZATION_TABLE_PTR();
        vlan_optimization_entry_ptr = &( us_vlan_optimization_table_ptr->entry[ xi_vlan_command_params->vlan_command_id ] );
    }

    RDD_VLAN_PARAMETER_ENTRY_OUTER_VID_WRITE ( xi_vlan_command_params->outer_vid, vlan_parameters_entry_ptr );
    RDD_VLAN_PARAMETER_ENTRY_OUTER_TPID_ID_WRITE ( xi_vlan_command_params->outer_tpid_id, vlan_parameters_entry_ptr );
    RDD_VLAN_PARAMETER_ENTRY_OUTER_TPID_OVERWRITE_ENABLE_WRITE ( xi_vlan_command_params->outer_tpid_overwrite_enable, vlan_parameters_entry_ptr );

    RDD_VLAN_PARAMETER_ENTRY_INNER_VID_WRITE ( xi_vlan_command_params->inner_vid, vlan_parameters_entry_ptr );
    RDD_VLAN_PARAMETER_ENTRY_INNER_TPID_ID_WRITE ( xi_vlan_command_params->inner_tpid_id, vlan_parameters_entry_ptr );
    RDD_VLAN_PARAMETER_ENTRY_INNER_TPID_OVERWRITE_ENABLE_WRITE ( xi_vlan_command_params->inner_tpid_overwrite_enable, vlan_parameters_entry_ptr );

    RDD_PBITS_PARAMETER_ENTRY_OUTER_PBITS_WRITE ( xi_vlan_command_params->outer_pbits, pbits_parameters_entry_ptr );
    RDD_PBITS_PARAMETER_ENTRY_INNER_PBITS_WRITE ( xi_vlan_command_params->inner_pbits, pbits_parameters_entry_ptr );



    RDD_VLAN_OPTIMIZATION_ENTRY_OPTIMIZE_ENABLE_WRITE ( vlan_optimize, vlan_optimization_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_us_vlan_aggregation_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE       xi_bridge_port,
                                                        BL_LILAC_RDD_AGGREGATION_MODE_DTE  xi_vlan_aggregation_mode )
{
    RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *filters_cfg_table_ptr;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS     *filters_cfg_entry_ptr;
    uint32_t                                        bridge_port_index;

    if ( ( xi_bridge_port < BL_LILAC_RDD_LAN0_BRIDGE_PORT ) || ( xi_bridge_port > BL_LILAC_RDD_LAN4_BRIDGE_PORT ) )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID );
    }

    bridge_port_index = rdd_bridge_port_to_port_index ( xi_bridge_port, 0 );

    /* set filter in upstream */
    filters_cfg_table_ptr = RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();

    filters_cfg_entry_ptr = &( filters_cfg_table_ptr->entry[ bridge_port_index ] );

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_VLAN_US_AGGREGATION_FILTER_WRITE ( xi_vlan_aggregation_mode, filters_cfg_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_vlan_switching_config ( BL_LILAC_RDD_VLAN_SWITCHING_CONFIG_DTE  xi_vlan_switching_mode,
                                                   BL_LILAC_RDD_VLAN_BINDING_CONFIG_DTE    xi_vlan_binding_mode )
{
    RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *us_filters_cfg_table_ptr;
    RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *ds_filters_cfg_table_ptr;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS  *filters_cfg_entry_ptr;
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS        *bridge_cfg_register;
    BL_LILAC_RDD_BRIDGE_PORT_DTE                 bridge_port;
    uint32_t                                     bridge_port_index;

    for ( bridge_port = BL_LILAC_RDD_LAN0_BRIDGE_PORT; bridge_port <= BL_LILAC_RDD_PCI_BRIDGE_PORT; bridge_port++ )
    {
        if ( ( bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT ) || ( bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT ) )
        {
            continue;
        }

        bridge_port_index = rdd_bridge_port_to_port_index ( bridge_port, 0 );

        /* set filter in upstream */
        us_filters_cfg_table_ptr = RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();

        filters_cfg_entry_ptr = &( us_filters_cfg_table_ptr->entry[ bridge_port_index ] );

        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_VLAN_US_AGGREGATION_FILTER_WRITE ( xi_vlan_switching_mode, filters_cfg_entry_ptr );
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_VLAN_SWITCHING_FILTER_WRITE ( xi_vlan_switching_mode, filters_cfg_entry_ptr );
    }

    for ( bridge_port_index = BL_LILAC_RDD_SUBNET_FLOW_CACHE; bridge_port_index < LILAC_RDD_NUMBER_OF_SUBNETS; bridge_port_index++ )
    {
        /* set filter in downstream */
        ds_filters_cfg_table_ptr = RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();

        filters_cfg_entry_ptr = &( ds_filters_cfg_table_ptr->entry[ bridge_port_index ] );

        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_VLAN_SWITCHING_FILTER_WRITE( xi_vlan_switching_mode, filters_cfg_entry_ptr );
    }

    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );

    RDD_BRIDGE_CONFIGURATION_REGISTER_VLAN_BINDING_MODE_WRITE ( xi_vlan_binding_mode, bridge_cfg_register );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_lan_vid_entry_add ( RDD_LAN_VID_PARAMS  *xi_lan_vid_params_ptr,
                                               uint32_t            *xo_entry_index )
{
    RDD_VID_ENTRY_DTS              *vid_entry_ptr;
    RDD_DS_LAN_VID_TABLE_DTS       *vid_table_ptr;
    RDD_LAN_VID_CONTEXT_ENTRY_DTS  *vid_context_entry_ptr;
    RDD_LAN_VID_CONTEXT_TABLE_DTS  *vid_context_table_ptr;
    uint32_t                       entry_index;
    uint16_t                       vid_entry;

    vid_table_ptr = ( RDD_DS_LAN_VID_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_LAN_VID_TABLE_ADDRESS );

    for ( entry_index = 0; entry_index < RDD_DS_LAN_VID_TABLE_SIZE; entry_index++ )
    {
        vid_entry_ptr = &( vid_table_ptr->entry[ entry_index ] );

        RDD_VID_ENTRY_VID_READ ( vid_entry, vid_entry_ptr );

        if ( vid_entry == xi_lan_vid_params_ptr->vid )
        {
            return ( BL_LILAC_RDD_ERROR_CAM_INSERTION_FAILED );
        }

        if ( !( vid_entry & LILAC_RDD_LAN_VID_SKIP_VALUE ) )
        {
            continue;
        }

        /* found empty entry - fill tables */
        RDD_VID_ENTRY_VID_WRITE ( xi_lan_vid_params_ptr->vid, vid_entry_ptr );

        vid_table_ptr = ( RDD_DS_LAN_VID_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_LAN_VID_TABLE_ADDRESS );

        vid_entry_ptr = &( vid_table_ptr->entry[ entry_index ] );

        RDD_VID_ENTRY_VID_WRITE ( xi_lan_vid_params_ptr->vid, vid_entry_ptr );

        vid_context_table_ptr = ( RDD_LAN_VID_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + LAN_VID_CONTEXT_TABLE_ADDRESS );

        vid_context_entry_ptr = &( vid_context_table_ptr->entry[ entry_index ] );

        RDD_LAN_VID_CONTEXT_ENTRY_ISOLATION_MODE_PORT_VECTOR_WRITE ( xi_lan_vid_params_ptr->isolation_mode_port_vector, vid_context_entry_ptr );
        RDD_LAN_VID_CONTEXT_ENTRY_AGGREGATED_VID_IDX_WRITE ( xi_lan_vid_params_ptr->aggregation_vid_index, vid_context_entry_ptr );
        RDD_LAN_VID_CONTEXT_ENTRY_AGGREGATION_MODE_PORT_VECTOR_WRITE ( xi_lan_vid_params_ptr->aggregation_mode_port_vector, vid_context_entry_ptr );

        *xo_entry_index = entry_index;

        return ( BL_LILAC_RDD_OK );
    }

    return ( BL_LILAC_RDD_ERROR_CAM_LOOKUP_TABLE_FULL );
}


BL_LILAC_RDD_ERROR_DTE rdd_lan_vid_entry_delete ( uint32_t  xi_entry_index )
{
    RDD_VID_ENTRY_DTS              *vid_entry_ptr;
    RDD_DS_LAN_VID_TABLE_DTS       *vid_table_ptr;
    RDD_LAN_VID_CONTEXT_ENTRY_DTS  *vid_context_entry_ptr;
    RDD_LAN_VID_CONTEXT_TABLE_DTS  *vid_context_table_ptr;

    vid_table_ptr = ( RDD_DS_LAN_VID_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_LAN_VID_TABLE_ADDRESS );

    /* invalidate CAM entry */
    vid_entry_ptr = &( vid_table_ptr->entry[ xi_entry_index ] );

    RDD_VID_ENTRY_VID_WRITE ( LILAC_RDD_LAN_VID_SKIP_VALUE, vid_entry_ptr );

    vid_table_ptr = ( RDD_DS_LAN_VID_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_LAN_VID_TABLE_ADDRESS );

    vid_entry_ptr = &( vid_table_ptr->entry[ xi_entry_index ] );

    RDD_VID_ENTRY_VID_WRITE ( LILAC_RDD_LAN_VID_SKIP_VALUE, vid_entry_ptr );

    /* reset context */
    vid_context_table_ptr = ( RDD_LAN_VID_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + LAN_VID_CONTEXT_TABLE_ADDRESS );

    vid_context_entry_ptr = &( vid_context_table_ptr->entry[ xi_entry_index ] );

    RDD_LAN_VID_CONTEXT_ENTRY_ISOLATION_MODE_PORT_VECTOR_WRITE ( BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN0 |
                                                         BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN1 |
                                                         BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN2 |
                                                         BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN3 |
                                                         BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN4 |
                                                         BL_LILAC_RDD_BRIDGE_PORT_VECTOR_PCI,
                                                         vid_context_entry_ptr );
    RDD_LAN_VID_CONTEXT_ENTRY_AGGREGATED_VID_IDX_WRITE ( 0, vid_context_entry_ptr );
    RDD_LAN_VID_CONTEXT_ENTRY_AGGREGATION_MODE_PORT_VECTOR_WRITE ( 0, vid_context_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_lan_vid_entry_modify ( uint32_t            xi_entry_index,
                                                  RDD_LAN_VID_PARAMS  *xi_lan_vid_params_ptr )
{
    RDD_LAN_VID_CONTEXT_ENTRY_DTS  *vid_context_entry_ptr;
    RDD_LAN_VID_CONTEXT_TABLE_DTS  *vid_context_table_ptr;

    vid_context_table_ptr = ( RDD_LAN_VID_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + LAN_VID_CONTEXT_TABLE_ADDRESS );

    vid_context_entry_ptr = &( vid_context_table_ptr->entry[ xi_entry_index ] );

    RDD_LAN_VID_CONTEXT_ENTRY_ISOLATION_MODE_PORT_VECTOR_WRITE ( xi_lan_vid_params_ptr->isolation_mode_port_vector, vid_context_entry_ptr );
    RDD_LAN_VID_CONTEXT_ENTRY_AGGREGATED_VID_IDX_WRITE ( xi_lan_vid_params_ptr->aggregation_vid_index, vid_context_entry_ptr );
    RDD_LAN_VID_CONTEXT_ENTRY_AGGREGATION_MODE_PORT_VECTOR_WRITE ( xi_lan_vid_params_ptr->aggregation_mode_port_vector, vid_context_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_lan_vid_entry_search ( RDD_LAN_VID_PARAMS  *xi_lan_vid_params_ptr, 
                                                  uint32_t            *xo_entry_index )
{
    RDD_VID_ENTRY_DTS         *vid_entry_ptr;
    RDD_DS_LAN_VID_TABLE_DTS  *vid_table_ptr;
    uint32_t                  entry_index;
    uint16_t                  vid_entry;

    vid_table_ptr = ( RDD_DS_LAN_VID_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_LAN_VID_TABLE_ADDRESS );

    for ( entry_index = 0; entry_index < RDD_DS_LAN_VID_TABLE_SIZE; entry_index++ )
    {
        vid_entry_ptr = &( vid_table_ptr->entry[ entry_index ] );

        RDD_VID_ENTRY_VID_READ ( vid_entry, vid_entry_ptr );

        if ( vid_entry == LILAC_RDD_LAN_VID_STOP_VALUE )
        {
            return ( BL_LILAC_RDD_ERROR_CAM_LOOKUP_FAILED );
        }

        if ( vid_entry != xi_lan_vid_params_ptr->vid )
        {
            continue;
        }

        *xo_entry_index = entry_index;

        return ( BL_LILAC_RDD_OK );
    }

    return ( BL_LILAC_RDD_ERROR_CAM_LOOKUP_FAILED );
}


BL_LILAC_RDD_ERROR_DTE rdd_wan_vid_config ( uint8_t   xi_wan_vid_index,
                                            uint16_t  xi_wan_vid )
{
    RDD_VID_ENTRY_DTS      *vid_entry_ptr;
    RDD_WAN_VID_TABLE_DTS  *vid_table_ptr;

    vid_table_ptr = ( RDD_WAN_VID_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + WAN_VID_TABLE_ADDRESS );

    vid_entry_ptr = &( vid_table_ptr->entry[ xi_wan_vid_index ] );

    RDD_VID_ENTRY_VID_WRITE ( xi_wan_vid, vid_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_vlan_switching_isolation_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                             rdpa_traffic_dir              xi_direction,
                                                             BL_LILAC_RDD_FILTER_MODE_DTE  xi_vlan_switching_isolation_mode )
{
    RDD_INGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_DTS  *vlan_switching_isolation_cfg_table_ptr;
    RDD_VLAN_SWITCHING_ISOLATION_CONFIG_ENTRY_DTS     *vlan_switching_isolation_cfg_entry_ptr;

    if ( xi_direction == rdpa_dir_ds )
    {
        vlan_switching_isolation_cfg_table_ptr = ( RDD_INGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + EGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_ADDRESS - sizeof ( RUNNER_COMMON ) );

        vlan_switching_isolation_cfg_entry_ptr = &( vlan_switching_isolation_cfg_table_ptr->entry[ xi_bridge_port - 1 ] );

        RDD_VLAN_SWITCHING_ISOLATION_CONFIG_ENTRY_EGRESS_CFG_WRITE ( xi_vlan_switching_isolation_mode, vlan_switching_isolation_cfg_entry_ptr);
    }
    else
    {
        vlan_switching_isolation_cfg_table_ptr = ( RDD_INGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + INGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_ADDRESS );

        vlan_switching_isolation_cfg_entry_ptr = &( vlan_switching_isolation_cfg_table_ptr->entry[ xi_bridge_port - 1 ] );

        RDD_VLAN_SWITCHING_ISOLATION_CONFIG_ENTRY_INGRESS_CFG_WRITE ( xi_vlan_switching_isolation_mode, vlan_switching_isolation_cfg_entry_ptr );
    }

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_dscp_to_pbits_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                  uint32_t                      xi_dscp,
                                                  uint32_t                      xi_pbits )
{
    RDD_DS_DSCP_TO_PBITS_TABLE_DTS  *ds_dscp_to_pbits_table_ptr;
    RDD_US_DSCP_TO_PBITS_TABLE_DTS  *us_dscp_to_pbits_table_ptr;
    RDD_DSCP_TO_PBITS_ENTRY_DTS     *dscp_to_pbits_entry_ptr;

    if ( xi_bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT )
    {
        ds_dscp_to_pbits_table_ptr = ( RDD_DS_DSCP_TO_PBITS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_DSCP_TO_PBITS_TABLE_ADDRESS );

        dscp_to_pbits_entry_ptr = &( ds_dscp_to_pbits_table_ptr->entry[ xi_dscp ] );
    }
    else
    {
        us_dscp_to_pbits_table_ptr = ( RDD_US_DSCP_TO_PBITS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_DSCP_TO_PBITS_TABLE_ADDRESS );

#ifndef G9991
        if ( xi_bridge_port == BL_LILAC_RDD_PCI_BRIDGE_PORT )
        {
            xi_bridge_port = 0;
        }
#else
        /* support only first entry */
        xi_bridge_port = 0;
#endif
        dscp_to_pbits_entry_ptr = &( us_dscp_to_pbits_table_ptr->entry[ xi_bridge_port ][ xi_dscp ] );
    }

    RDD_DSCP_TO_PBITS_ENTRY_PBITS_WRITE ( xi_pbits, dscp_to_pbits_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_pbits_to_pbits_config ( uint32_t  xi_table_number,
                                                   uint32_t  xi_input_pbits,
                                                   uint32_t  xi_output_pbits )
{
    RDD_DS_PBITS_TO_PBITS_TABLE_DTS  *ds_pbits_to_pbits_table_ptr;
    RDD_US_PBITS_TO_PBITS_TABLE_DTS  *us_pbits_to_pbits_table_ptr;
	RDD_PBITS_TO_PBITS_ENTRY_DTS  *pbits_to_pbits_entry_ptr;

    ds_pbits_to_pbits_table_ptr = ( RDD_DS_PBITS_TO_PBITS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PBITS_TO_PBITS_TABLE_ADDRESS );

    pbits_to_pbits_entry_ptr = &( ds_pbits_to_pbits_table_ptr->entry[ xi_table_number ][ xi_input_pbits ] );

    RDD_PBITS_TO_PBITS_ENTRY_PBITS_WRITE ( xi_output_pbits, pbits_to_pbits_entry_ptr );


    us_pbits_to_pbits_table_ptr = ( RDD_US_PBITS_TO_PBITS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PBITS_TO_PBITS_TABLE_ADDRESS );

    pbits_to_pbits_entry_ptr = &( us_pbits_to_pbits_table_ptr->entry[ xi_table_number ][ xi_input_pbits ] );

    RDD_PBITS_TO_PBITS_ENTRY_PBITS_WRITE ( xi_output_pbits, pbits_to_pbits_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_tpid_overwrite_table_config ( uint16_t          *tpid_overwrite_array,
                                                         rdpa_traffic_dir  xi_direction )
{

    RDD_DS_TPID_OVERWRITE_TABLE_DTS  *ds_tpid_overwrite_table_ptr;
    RDD_US_TPID_OVERWRITE_TABLE_DTS  *us_tpid_overwrite_table_ptr;
    RDD_TPID_OVERWRITE_ENTRY_DTS     *tpid_overwrite_entry_ptr;
    uint32_t                         i;
    uint32_t                         rdd_tpid_table_size;

    if ( xi_direction == rdpa_dir_ds )
    {
        ds_tpid_overwrite_table_ptr = RDD_DS_TPID_OVERWRITE_TABLE_PTR();
        rdd_tpid_table_size = RDD_DS_TPID_OVERWRITE_TABLE_SIZE;
    }
    else
    {
        us_tpid_overwrite_table_ptr = RDD_US_TPID_OVERWRITE_TABLE_PTR();
        rdd_tpid_table_size = RDD_US_TPID_OVERWRITE_TABLE_SIZE;
    }

    for ( i = 0; i < rdd_tpid_table_size; i++ )
    {
        if ( xi_direction == rdpa_dir_ds )
        {
            tpid_overwrite_entry_ptr = &(ds_tpid_overwrite_table_ptr->entry[i]); 
        }
        else
        {
            tpid_overwrite_entry_ptr = &(us_tpid_overwrite_table_ptr->entry[i]); 
        }
        RDD_TPID_OVERWRITE_ENTRY_TPID_WRITE ( tpid_overwrite_array[ i ], tpid_overwrite_entry_ptr );
    }

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_tpid_detect_filter_value_config ( rdpa_traffic_dir  xi_direction,
                                                             uint16_t          tpid_detect_filter_value )
{
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS  *bridge_cfg_register;

    if ( xi_direction == rdpa_dir_ds )
    {
        bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );
    }
    else
    {
        bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );
    }

    RDD_BRIDGE_CONFIGURATION_REGISTER_TPID_DETECT_VALUE_WRITE ( tpid_detect_filter_value, bridge_cfg_register );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_tpid_detect_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                       BL_LILAC_RDD_SUBNET_ID_DTE    xi_subnet_id,
                                                       BL_LILAC_RDD_FILTER_MODE_DTE  xi_tpid_filter_mode )
{
    RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *ds_filters_cfg_table_ptr;
    RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *us_filters_cfg_table_ptr;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS     *filters_cfg_entry_ptr;
    int32_t                                         bridge_port_index;
    
    if ( ( bridge_port_index = rdd_bridge_port_to_port_index ( xi_bridge_port, xi_subnet_id ) ) < 0 )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID );
    }

    if ( xi_bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || xi_bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || xi_bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
    {
        ds_filters_cfg_table_ptr = RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();
        filters_cfg_entry_ptr = &( ds_filters_cfg_table_ptr->entry[ bridge_port_index ] );
    }
    else
    {
        us_filters_cfg_table_ptr = RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();
        filters_cfg_entry_ptr = &( us_filters_cfg_table_ptr->entry[ bridge_port_index ] );
    }
    
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_TPID_DETECT_FILTER_WRITE( xi_tpid_filter_mode, filters_cfg_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_dhcp_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                BL_LILAC_RDD_SUBNET_ID_DTE    xi_subnet_id,
                                                BL_LILAC_RDD_FILTER_MODE_DTE  xi_dhcp_filter_mode )
{
    RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *ds_filters_cfg_table_ptr;
    RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *us_filters_cfg_table_ptr;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS     *filters_cfg_entry_ptr;
    int32_t                                         bridge_port_index;
    uint32_t                                        filters_mode;
    uint32_t                                        filters_cfg_entry;

    if ( ( bridge_port_index = rdd_bridge_port_to_port_index ( xi_bridge_port, xi_subnet_id ) ) < 0 )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID );
    }

    if ( xi_bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || xi_bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || xi_bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
    {
        ds_filters_cfg_table_ptr = RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();
        filters_cfg_entry_ptr = &( ds_filters_cfg_table_ptr->entry[ bridge_port_index ] );
    }
    else
    {
        us_filters_cfg_table_ptr = RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();
        filters_cfg_entry_ptr = &( us_filters_cfg_table_ptr->entry[ bridge_port_index ] );
    }

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_READ ( filters_cfg_entry,  filters_cfg_entry_ptr );

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DHCP_INGRESS_FILTER_L_WRITE ( filters_cfg_entry, xi_dhcp_filter_mode );

    filters_mode = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ ( filters_cfg_entry ) |
                   RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_READ ( filters_cfg_entry );

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_LOCAL_SWITCHING_INGRESS_FILTERS_L_WRITE ( filters_cfg_entry, g_local_switching_filters_mode[ bridge_port_index ] && filters_mode );

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_WRITE ( filters_cfg_entry, filters_cfg_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_mld_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                               BL_LILAC_RDD_SUBNET_ID_DTE    xi_subnet_id,
                                               BL_LILAC_RDD_FILTER_MODE_DTE  xi_mld_filter_mode )
{
    RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *ds_filters_cfg_table_ptr;
    RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *us_filters_cfg_table_ptr;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS     *filters_cfg_entry_ptr;
    int32_t                                         bridge_port_index;
    uint32_t                                        filters_mode;
    uint32_t                                        filters_cfg_entry;

    if ( ( bridge_port_index = rdd_bridge_port_to_port_index ( xi_bridge_port, xi_subnet_id ) ) < 0 )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID );
    }

    if ( xi_bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || xi_bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || xi_bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
    {
        ds_filters_cfg_table_ptr = RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();
        filters_cfg_entry_ptr = &( ds_filters_cfg_table_ptr->entry[ bridge_port_index ] );
    }
    else
    {
        us_filters_cfg_table_ptr = RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();
        filters_cfg_entry_ptr = &( us_filters_cfg_table_ptr->entry[ bridge_port_index ] );
    }

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_READ ( filters_cfg_entry,  filters_cfg_entry_ptr );

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_WRITE ( filters_cfg_entry, xi_mld_filter_mode );

    filters_mode = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_READ ( filters_cfg_entry ) | xi_mld_filter_mode;

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_LOCAL_SWITCHING_INGRESS_FILTERS_L_WRITE ( filters_cfg_entry, g_local_switching_filters_mode[ bridge_port_index ] && filters_mode );

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_WRITE ( filters_cfg_entry, filters_cfg_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_1588_layer4_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                       BL_LILAC_RDD_SUBNET_ID_DTE    xi_subnet_id,
                                                       BL_LILAC_RDD_FILTER_MODE_DTE  xi_1588_filter_mode )
{
    RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *ds_filters_cfg_table_ptr;
    RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *us_filters_cfg_table_ptr;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS     *filters_cfg_entry_ptr;
    int32_t                                         bridge_port_index;

    if ( ( bridge_port_index = rdd_bridge_port_to_port_index ( xi_bridge_port, xi_subnet_id ) ) < 0 )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID );
    }

    if ( xi_bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || xi_bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || xi_bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
    {
        ds_filters_cfg_table_ptr = RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();
        filters_cfg_entry_ptr = &( ds_filters_cfg_table_ptr->entry[ bridge_port_index ] );
    }
    else
    {
        us_filters_cfg_table_ptr = RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();
        filters_cfg_entry_ptr = &( us_filters_cfg_table_ptr->entry[ bridge_port_index ] );
    }

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_TIMING_1588_INGRESS_FILTER_WRITE ( xi_1588_filter_mode, filters_cfg_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_igmp_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE    xi_bridge_port,
                                                BL_LILAC_RDD_SUBNET_ID_DTE      xi_subnet_id,
                                                BL_LILAC_RDD_FILTER_MODE_DTE    xi_igmp_filter_mode,
                                                BL_LILAC_RDD_FILTER_ACTION_DTE  xi_filter_action )
{
    RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *ds_filters_cfg_table_ptr;
    RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *us_filters_cfg_table_ptr;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS     *filters_cfg_entry_ptr;
    RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_DTS         *ds_filters_cam_table_ptr;
    RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_DTS         *us_filters_cam_table_ptr;
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS            *filters_cam_entry_ptr;
    RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_DTS      *ds_filters_cam_parameter_table_ptr;
    RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_DTS      *us_filters_cam_parameter_table_ptr;
    RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DTS         *filters_cam_parameter_entry_ptr;
    int32_t                                         bridge_port_index;
    uint32_t                                        filters_mode;
    uint32_t                                        filters_cfg_entry;

    if ( ( bridge_port_index = rdd_bridge_port_to_port_index ( xi_bridge_port, xi_subnet_id ) ) < 0 )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID );
    }

    if ( xi_bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || xi_bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || xi_bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
    {
        ds_filters_cfg_table_ptr = RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();

        ds_filters_cam_table_ptr = RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_PTR();

        ds_filters_cam_parameter_table_ptr = RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_PTR();

        filters_cfg_entry_ptr = &( ds_filters_cfg_table_ptr->entry[ bridge_port_index ] );

        filters_cfg_entry_ptr = &( ds_filters_cfg_table_ptr->entry[ bridge_port_index ] );

        filters_cam_entry_ptr = &( ds_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_IGMP_FILTER_NUMBER ] );

        filters_cam_parameter_entry_ptr = &( ds_filters_cam_parameter_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_IGMP_FILTER_NUMBER ] );
    }
    else
    {
        us_filters_cfg_table_ptr = RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();

        us_filters_cam_table_ptr = RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_PTR();

        us_filters_cam_parameter_table_ptr = RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_PTR();

        filters_cfg_entry_ptr = &( us_filters_cfg_table_ptr->entry[ bridge_port_index ] );

        filters_cam_entry_ptr = &( us_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_IGMP_FILTER_NUMBER ] );

        filters_cam_parameter_entry_ptr = &( us_filters_cam_parameter_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_IGMP_FILTER_NUMBER ] );

    }

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_READ ( filters_cfg_entry, filters_cfg_entry_ptr );

    if ( xi_igmp_filter_mode == BL_LILAC_RDD_FILTER_ENABLE )
    {
        f_rdd_ingress_filter_enable ( filters_cam_entry_ptr );

        if ( xi_filter_action == BL_LILAC_RDD_FILTER_ACTION_CPU_TRAP )
        {
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_PARAMETER_WRITE ( rdpa_cpu_rx_reason_igmp,
                filters_cam_parameter_entry_ptr );
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_TRUE, filters_cam_parameter_entry_ptr );

            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DROP_WRITE ( LILAC_RDD_FALSE, filters_cam_parameter_entry_ptr );
        }
        else
        {
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DROP_WRITE ( LILAC_RDD_TRUE, filters_cam_parameter_entry_ptr );

            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_PARAMETER_WRITE ( 0, filters_cam_parameter_entry_ptr );
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_FALSE, filters_cam_parameter_entry_ptr );
        }

        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IGMP_FILTER_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE );
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE );
    }
    else
    {
        f_rdd_ingress_filter_disable ( filters_cam_entry_ptr );

        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IGMP_FILTER_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE );

        filters_mode  = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ ( filters_cfg_entry ) |
                        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IGMP_FILTER_L_READ ( filters_cfg_entry )        |
                        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ICMPV6_FILTER_L_READ ( filters_cfg_entry )      |
                        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ETHERTYPE_FILTER_L_READ ( filters_cfg_entry )   |
                        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_BROADCAST_FILTER_L_READ ( filters_cfg_entry )   |
                        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MULTICAST_FILTER_L_READ ( filters_cfg_entry );

        if ( filters_mode == 0 )
        {
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE );
        }
    }

    /* Update local switching filter mode */
    filters_mode = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ ( filters_cfg_entry ) |
                   RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_READ ( filters_cfg_entry );

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_LOCAL_SWITCHING_INGRESS_FILTERS_L_WRITE ( filters_cfg_entry, g_local_switching_filters_mode[ bridge_port_index ] && filters_mode );

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_WRITE ( filters_cfg_entry, filters_cfg_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_icmpv6_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                  BL_LILAC_RDD_SUBNET_ID_DTE    xi_subnet_id,
                                                  BL_LILAC_RDD_FILTER_MODE_DTE  xi_icmpv6_filter_mode )
{
    RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *ds_filters_cfg_table_ptr;
    RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *us_filters_cfg_table_ptr;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS     *filters_cfg_entry_ptr;
    RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_DTS         *ds_filters_cam_table_ptr;
    RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_DTS         *us_filters_cam_table_ptr;
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS            *filters_cam_entry_ptr;
    RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_DTS      *ds_filters_cam_parameter_table_ptr;
    RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_DTS      *us_filters_cam_parameter_table_ptr;
    RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DTS         *filters_cam_parameter_entry_ptr;
    int32_t                                         bridge_port_index;
    uint32_t                                        filters_mode;
    uint32_t                                        filters_cfg_entry;

    if ( ( bridge_port_index = rdd_bridge_port_to_port_index ( xi_bridge_port, xi_subnet_id ) ) < 0 )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID );
    }

    if ( xi_bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || xi_bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || xi_bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
    {
        ds_filters_cfg_table_ptr = RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();

        ds_filters_cam_table_ptr = RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_PTR();

        ds_filters_cam_parameter_table_ptr = RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_PTR();

        filters_cfg_entry_ptr = &( ds_filters_cfg_table_ptr->entry[ bridge_port_index ] );

        filters_cam_entry_ptr = &( ds_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_ICMPV6_FILTER_NUMBER ] );

        filters_cam_parameter_entry_ptr = &( ds_filters_cam_parameter_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_ICMPV6_FILTER_NUMBER ] );

    }
    else
    {
        us_filters_cfg_table_ptr = RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();

        us_filters_cam_table_ptr = RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_PTR();

        us_filters_cam_parameter_table_ptr = RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_PTR();

        filters_cfg_entry_ptr = &( us_filters_cfg_table_ptr->entry[ bridge_port_index ] );

        filters_cam_entry_ptr = &( us_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_ICMPV6_FILTER_NUMBER ] );

        filters_cam_parameter_entry_ptr = &( us_filters_cam_parameter_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_ICMPV6_FILTER_NUMBER ] );

    }

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_READ ( filters_cfg_entry, filters_cfg_entry_ptr );

    if ( xi_icmpv6_filter_mode == BL_LILAC_RDD_FILTER_ENABLE )
    {
        f_rdd_ingress_filter_enable ( filters_cam_entry_ptr );

        RDD_INGRESS_FILTERS_PARAMETER_ENTRY_PARAMETER_WRITE ( rdpa_cpu_rx_reason_icmpv6,
            filters_cam_parameter_entry_ptr );
        RDD_INGRESS_FILTERS_PARAMETER_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_TRUE, filters_cam_parameter_entry_ptr );

        RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DROP_WRITE ( LILAC_RDD_FALSE, filters_cam_parameter_entry_ptr );

        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ICMPV6_FILTER_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE );
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE );
    }
    else
    {
        f_rdd_ingress_filter_disable ( filters_cam_entry_ptr );
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ICMPV6_FILTER_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE );

        filters_mode  = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ ( filters_cfg_entry ) |
                        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IGMP_FILTER_L_READ ( filters_cfg_entry )        |
                        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ICMPV6_FILTER_L_READ ( filters_cfg_entry )      |
                        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ETHERTYPE_FILTER_L_READ ( filters_cfg_entry )   |
                        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_BROADCAST_FILTER_L_READ ( filters_cfg_entry )   |
                        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MULTICAST_FILTER_L_READ ( filters_cfg_entry );

        if ( filters_mode == 0 )
        {
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE );
        }
    }

    /* Update local switching filter mode */
    filters_mode = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ ( filters_cfg_entry ) |
                   RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_READ ( filters_cfg_entry );

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_LOCAL_SWITCHING_INGRESS_FILTERS_L_WRITE ( filters_cfg_entry, g_local_switching_filters_mode[ bridge_port_index ] && filters_mode );

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_WRITE ( filters_cfg_entry, filters_cfg_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_ether_type_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE               xi_bridge_port,
                                                      BL_LILAC_RDD_SUBNET_ID_DTE                 xi_subnet_id,
                                                      BL_LILAC_RDD_FILTER_MODE_DTE               xi_ether_type_filter_mode,
                                                      BL_LILAC_RDD_ETHER_TYPE_FILTER_NUMBER_DTE  xi_ether_type_filter_number,
                                                      BL_LILAC_RDD_FILTER_ACTION_DTE             xi_ether_type_action )
{
    RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *ds_filters_cfg_table_ptr;
    RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *us_filters_cfg_table_ptr;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS     *filters_cfg_entry_ptr;
    RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_DTS         *ds_filters_cam_table_ptr;
    RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_DTS         *us_filters_cam_table_ptr;
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS            *filters_cam_entry_ptr;
    RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_DTS      *ds_filters_cam_parameter_table_ptr;
    RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_DTS      *us_filters_cam_parameter_table_ptr;
    RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DTS         *filters_cam_parameter_entry_ptr;
    int32_t                                         bridge_port_index;
    int32_t                                         ether_type_filter_index;
    uint32_t                                        filters_mode;
    uint32_t                                        filters_cfg_entry;
    uint8_t                                         cpu_reason;

    if ( ( bridge_port_index = rdd_bridge_port_to_port_index ( xi_bridge_port, xi_subnet_id ) ) < 0 )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID );
    }

    if ( xi_bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || xi_bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || xi_bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
    {
        ds_filters_cfg_table_ptr = RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();

        ds_filters_cam_table_ptr = RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_PTR();

        ds_filters_cam_parameter_table_ptr = RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_PTR();

        filters_cfg_entry_ptr = &( ds_filters_cfg_table_ptr->entry[ bridge_port_index ] );

        filters_cam_entry_ptr = &( ds_filters_cam_table_ptr->entry[ bridge_port_index ][ xi_ether_type_filter_number ] );

        filters_cam_parameter_entry_ptr = &( ds_filters_cam_parameter_table_ptr->entry[ bridge_port_index ][ xi_ether_type_filter_number ] );
    }
    else
    {
        us_filters_cfg_table_ptr = RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();

        us_filters_cam_table_ptr = RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_PTR();

        us_filters_cam_parameter_table_ptr = RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_PTR();

        filters_cfg_entry_ptr = &( us_filters_cfg_table_ptr->entry[ bridge_port_index ] );

        filters_cam_entry_ptr = &( us_filters_cam_table_ptr->entry[ bridge_port_index ][ xi_ether_type_filter_number ] );

        filters_cam_parameter_entry_ptr = &( us_filters_cam_parameter_table_ptr->entry[ bridge_port_index ][ xi_ether_type_filter_number ] );

    }

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_READ ( filters_cfg_entry, filters_cfg_entry_ptr );

    if ( xi_ether_type_filter_mode == BL_LILAC_RDD_FILTER_ENABLE )
    {
        if ( xi_ether_type_action == BL_LILAC_RDD_FILTER_ACTION_CPU_TRAP )
        {
            cpu_reason = rdpa_cpu_rx_reason_etype_udef_0 + xi_ether_type_filter_number -
                BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_0;

            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_PARAMETER_WRITE ( cpu_reason, filters_cam_parameter_entry_ptr );
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_TRUE, filters_cam_parameter_entry_ptr );

            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DROP_WRITE ( LILAC_RDD_FALSE, filters_cam_parameter_entry_ptr );
        }
        else if ( xi_ether_type_action == BL_LILAC_RDD_FILTER_ACTION_DROP )
        {
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DROP_WRITE ( LILAC_RDD_TRUE, filters_cam_parameter_entry_ptr );

            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_PARAMETER_WRITE ( 0, filters_cam_parameter_entry_ptr );
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_FALSE, filters_cam_parameter_entry_ptr );
        }

        f_rdd_ingress_filter_enable ( filters_cam_entry_ptr );

        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ETHERTYPE_FILTER_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE );
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE );

        g_ether_type_filter_mode->entry[ xi_bridge_port ][ xi_subnet_id ][ xi_ether_type_filter_number - BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_0 ] = BL_LILAC_RDD_FILTER_ENABLE;
    }
    else
    {
        f_rdd_ingress_filter_disable ( filters_cam_entry_ptr );

        g_ether_type_filter_mode->entry[ xi_bridge_port ][ xi_subnet_id ][ xi_ether_type_filter_number - BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_0 ] = BL_LILAC_RDD_FILTER_DISABLE;

        for ( ether_type_filter_index = BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_0; ether_type_filter_index <= BL_LILAC_RDD_ETHER_TYPE_FILTER_802_1AG_CFM; ether_type_filter_index++ )
        {
            if ( g_ether_type_filter_mode->entry[ xi_bridge_port][ xi_subnet_id ][ ether_type_filter_index - BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_0 ] == BL_LILAC_RDD_FILTER_ENABLE )
            {
                break;
            }
        }

        if ( ether_type_filter_index > BL_LILAC_RDD_ETHER_TYPE_FILTER_802_1AG_CFM )
        {
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ETHERTYPE_FILTER_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE );

            filters_mode  = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ ( filters_cfg_entry ) |
                            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IGMP_FILTER_L_READ ( filters_cfg_entry )        |
                            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ICMPV6_FILTER_L_READ ( filters_cfg_entry )      |
                            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ETHERTYPE_FILTER_L_READ ( filters_cfg_entry )   |
                            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_BROADCAST_FILTER_L_READ ( filters_cfg_entry )   |
                            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MULTICAST_FILTER_L_READ ( filters_cfg_entry );

            if ( filters_mode == 0 )
            {
                RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE );
            }
        }
    }

    /* Update local switching filter mode */
    filters_mode = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ ( filters_cfg_entry ) |
                   RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_READ ( filters_cfg_entry );

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_LOCAL_SWITCHING_INGRESS_FILTERS_L_WRITE ( filters_cfg_entry, g_local_switching_filters_mode[ bridge_port_index ] && filters_mode );

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_WRITE ( filters_cfg_entry, filters_cfg_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_broadcast_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE    xi_bridge_port,
                                                     BL_LILAC_RDD_SUBNET_ID_DTE      xi_subnet_id,
                                                     BL_LILAC_RDD_FILTER_MODE_DTE    xi_broadcast_filter_mode,
                                                     BL_LILAC_RDD_FILTER_ACTION_DTE  xi_filter_action )
{
    RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *ds_filters_cfg_table_ptr;
    RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *us_filters_cfg_table_ptr;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS     *filters_cfg_entry_ptr;
    RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_DTS         *ds_filters_cam_table_ptr;
    RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_DTS         *us_filters_cam_table_ptr;
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS            *filters_cam_entry_ptr;
    RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_DTS      *ds_filters_cam_parameter_table_ptr;
    RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_DTS      *us_filters_cam_parameter_table_ptr;
    RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DTS         *filters_cam_parameter_entry_ptr;
    int32_t                                         bridge_port_index;
    uint32_t                                        filters_mode;
    uint32_t                                        filters_cfg_entry;

    if ( ( bridge_port_index = rdd_bridge_port_to_port_index ( xi_bridge_port, xi_subnet_id ) ) < 0 )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID );
    }

    if ( xi_bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || xi_bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || xi_bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
    {
        ds_filters_cfg_table_ptr = RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();

        ds_filters_cam_table_ptr = RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_PTR();

        ds_filters_cam_parameter_table_ptr = RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_PTR();

        filters_cfg_entry_ptr = &( ds_filters_cfg_table_ptr->entry[ bridge_port_index ] );

        filters_cam_entry_ptr = &( ds_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_BROADCAST_FILTER_NUMBER ] );

        filters_cam_parameter_entry_ptr = &( ds_filters_cam_parameter_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_BROADCAST_FILTER_NUMBER ] );

    }
    else
    {
        us_filters_cfg_table_ptr = RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();

        us_filters_cam_table_ptr = RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_PTR();

        us_filters_cam_parameter_table_ptr = RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_PTR();

        filters_cfg_entry_ptr = &( us_filters_cfg_table_ptr->entry[ bridge_port_index ] );

        filters_cam_entry_ptr = &( us_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_BROADCAST_FILTER_NUMBER ] );

        filters_cam_parameter_entry_ptr = &( us_filters_cam_parameter_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_BROADCAST_FILTER_NUMBER ] );

    }

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_READ ( filters_cfg_entry,  filters_cfg_entry_ptr );

    if ( xi_broadcast_filter_mode == BL_LILAC_RDD_FILTER_ENABLE )
    {
        f_rdd_ingress_filter_enable ( filters_cam_entry_ptr );

        if ( xi_filter_action == BL_LILAC_RDD_FILTER_ACTION_CPU_TRAP )
        {
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_PARAMETER_WRITE ( rdpa_cpu_rx_reason_bcast,
                filters_cam_parameter_entry_ptr );
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_TRUE, filters_cam_parameter_entry_ptr );

            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DROP_WRITE ( LILAC_RDD_FALSE, filters_cam_parameter_entry_ptr );
        }
        else
        {
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DROP_WRITE ( LILAC_RDD_TRUE, filters_cam_parameter_entry_ptr );

            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_PARAMETER_WRITE ( 0, filters_cam_parameter_entry_ptr );
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_FALSE, filters_cam_parameter_entry_ptr );
        }

        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_BROADCAST_FILTER_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE );
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE );
    }
    else
    {
        f_rdd_ingress_filter_disable ( filters_cam_entry_ptr );

        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_BROADCAST_FILTER_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE );

        filters_mode  = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ ( filters_cfg_entry ) |
                        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IGMP_FILTER_L_READ ( filters_cfg_entry )        |
                        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ICMPV6_FILTER_L_READ ( filters_cfg_entry )      |
                        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ETHERTYPE_FILTER_L_READ ( filters_cfg_entry )   |
                        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_BROADCAST_FILTER_L_READ ( filters_cfg_entry )   |
                        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MULTICAST_FILTER_L_READ ( filters_cfg_entry );

        if ( filters_mode == 0 )
        {
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE );
        }
    }

    /* Update local switching filter mode */
    filters_mode = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ ( filters_cfg_entry ) |
                   RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_READ ( filters_cfg_entry );

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_LOCAL_SWITCHING_INGRESS_FILTERS_L_WRITE ( filters_cfg_entry, g_local_switching_filters_mode[ bridge_port_index ] && filters_mode );

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_WRITE ( filters_cfg_entry, filters_cfg_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_multicast_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE    xi_bridge_port,
                                                     BL_LILAC_RDD_SUBNET_ID_DTE      xi_subnet_id,
                                                     BL_LILAC_RDD_FILTER_MODE_DTE    xi_multicast_filter_mode,
                                                     BL_LILAC_RDD_FILTER_ACTION_DTE  xi_filter_action )
{
    RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *ds_filters_cfg_table_ptr;
    RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *us_filters_cfg_table_ptr;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS  *filters_cfg_entry_ptr;
    RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_DTS      *ds_filters_cam_table_ptr;
    RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_DTS      *us_filters_cam_table_ptr;
    RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS         *filters_cam_entry_ptr;
    RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_DTS   *ds_filters_cam_parameter_table_ptr;
    RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_DTS   *us_filters_cam_parameter_table_ptr;
    RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DTS      *filters_cam_parameter_entry_ptr;
    int32_t                                      bridge_port_index;
    uint32_t                                     filters_mode;
    uint32_t                                     filters_cfg_entry;

    if ( ( bridge_port_index = rdd_bridge_port_to_port_index ( xi_bridge_port, xi_subnet_id ) ) < 0 )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID );
    }

    if ( xi_bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || xi_bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || xi_bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
    {
        ds_filters_cfg_table_ptr = RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();

        ds_filters_cam_table_ptr = RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_PTR();

        ds_filters_cam_parameter_table_ptr = RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_PTR();

        filters_cfg_entry_ptr = &( ds_filters_cfg_table_ptr->entry[ bridge_port_index ] );

        filters_cam_entry_ptr = &( ds_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_MULTICAST_FILTER_NUMBER ] );

        filters_cam_parameter_entry_ptr = &( ds_filters_cam_parameter_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_MULTICAST_FILTER_NUMBER ] );

    }
    else
    {
        us_filters_cfg_table_ptr = RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();

        us_filters_cam_table_ptr = RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_PTR();

        us_filters_cam_parameter_table_ptr = RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_PTR();

        filters_cfg_entry_ptr = &( us_filters_cfg_table_ptr->entry[ bridge_port_index ] );

        filters_cam_entry_ptr = &( us_filters_cam_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_MULTICAST_FILTER_NUMBER ] );

        filters_cam_parameter_entry_ptr = &( us_filters_cam_parameter_table_ptr->entry[ bridge_port_index ][ BL_LILAC_RDD_MULTICAST_FILTER_NUMBER ] );
    }

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_READ ( filters_cfg_entry,  filters_cfg_entry_ptr );

    if ( xi_multicast_filter_mode == BL_LILAC_RDD_FILTER_ENABLE )
    {
        f_rdd_ingress_filter_enable ( filters_cam_entry_ptr );

        if ( xi_filter_action == BL_LILAC_RDD_FILTER_ACTION_CPU_TRAP )
        {
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_PARAMETER_WRITE ( rdpa_cpu_rx_reason_mcast,
                filters_cam_parameter_entry_ptr );
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_TRUE, filters_cam_parameter_entry_ptr );

            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DROP_WRITE ( LILAC_RDD_FALSE, filters_cam_parameter_entry_ptr );
        }
        else
        {
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DROP_WRITE ( LILAC_RDD_TRUE, filters_cam_parameter_entry_ptr );

            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_PARAMETER_WRITE ( 0, filters_cam_parameter_entry_ptr );
            RDD_INGRESS_FILTERS_PARAMETER_ENTRY_CPU_TRAP_WRITE ( LILAC_RDD_FALSE, filters_cam_parameter_entry_ptr );
        }

        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MULTICAST_FILTER_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE );
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE );
    }
    else
    {
        f_rdd_ingress_filter_disable ( filters_cam_entry_ptr );

        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MULTICAST_FILTER_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE );

        filters_mode  = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ ( filters_cfg_entry ) |
                        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IGMP_FILTER_L_READ ( filters_cfg_entry )        |
                        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ICMPV6_FILTER_L_READ ( filters_cfg_entry )      |
                        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ETHERTYPE_FILTER_L_READ ( filters_cfg_entry )   |
                        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_BROADCAST_FILTER_L_READ ( filters_cfg_entry )   |
                        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MULTICAST_FILTER_L_READ ( filters_cfg_entry );

        if ( filters_mode == 0 )
        {
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE );
        }
    }

    /* Update local switching filter mode */
    filters_mode = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ ( filters_cfg_entry ) |
                   RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_READ ( filters_cfg_entry );

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_LOCAL_SWITCHING_INGRESS_FILTERS_L_WRITE ( filters_cfg_entry, g_local_switching_filters_mode[ bridge_port_index ] && filters_mode );

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_WRITE ( filters_cfg_entry, filters_cfg_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_local_switching_filters_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                            BL_LILAC_RDD_FILTER_MODE_DTE  xi_local_switching_filters_mode )
{
    RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *us_filters_cfg_table_ptr;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS     *filters_cfg_entry_ptr;
    int32_t                                         bridge_port_index;

    if ( xi_bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || xi_bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || xi_bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID );
    }

    if ( ( bridge_port_index = rdd_bridge_port_to_port_index ( xi_bridge_port, 0 ) ) < 0 )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID );
    }

    g_local_switching_filters_mode[ bridge_port_index ] = xi_local_switching_filters_mode;

    us_filters_cfg_table_ptr = RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();

    filters_cfg_entry_ptr = &( us_filters_cfg_table_ptr->entry[ bridge_port_index ] );

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_LOCAL_SWITCHING_INGRESS_FILTERS_WRITE ( xi_local_switching_filters_mode,filters_cfg_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_ip_fragments_ingress_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE    xi_bridge_port,
                                                                BL_LILAC_RDD_SUBNET_ID_DTE      xi_subnet_id,
                                                                BL_LILAC_RDD_FILTER_MODE_DTE    xi_ip_fragments_filter_mode,
                                                                BL_LILAC_RDD_FILTER_ACTION_DTE  xi_filter_action )
{
    RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *ds_filters_cfg_table_ptr;
    RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *us_filters_cfg_table_ptr;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS     *filters_cfg_entry_ptr;
    int32_t                                         bridge_port_index;
    uint32_t                                        header_error_filter;
    uint32_t                                        filters_cfg_entry;

    if ( ( bridge_port_index = rdd_bridge_port_to_port_index ( xi_bridge_port, xi_subnet_id ) ) < 0 )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID );
    }

    if ( xi_bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || xi_bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || xi_bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
    {
        ds_filters_cfg_table_ptr = RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();
        filters_cfg_entry_ptr = &( ds_filters_cfg_table_ptr->entry[ bridge_port_index ] );
    }
    else
    {
        us_filters_cfg_table_ptr = RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();
        filters_cfg_entry_ptr = &( us_filters_cfg_table_ptr->entry[ bridge_port_index ] );
    }

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_READ ( filters_cfg_entry, filters_cfg_entry_ptr );

    if ( xi_ip_fragments_filter_mode == BL_LILAC_RDD_FILTER_ENABLE )
    {
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_FRAGMENT_INGRESS_FILTER_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE );
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_VALIDATION_FILTER_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE );

        if ( xi_filter_action == BL_LILAC_RDD_FILTER_ACTION_CPU_TRAP )
        {
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_FRAGMENT_INGRESS_FILTER_TRAP_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE );
        }
        else
        {
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_FRAGMENT_INGRESS_FILTER_TRAP_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE );
        }
    }
    else
    {
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_FRAGMENT_INGRESS_FILTER_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE );

        header_error_filter = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_HEADER_ERROR_INGRESS_FILTER_L_READ ( filters_cfg_entry );

        if ( header_error_filter == 0 )
        {
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_VALIDATION_FILTER_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE );
        }
    }

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_WRITE ( filters_cfg_entry, filters_cfg_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_header_error_ingress_filter_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE    xi_bridge_port,
                                                                BL_LILAC_RDD_SUBNET_ID_DTE      xi_subnet_id,
                                                                BL_LILAC_RDD_FILTER_MODE_DTE    xi_header_error_filter_mode,
                                                                BL_LILAC_RDD_FILTER_ACTION_DTE  xi_filter_action )
{
    RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *ds_filters_cfg_table_ptr;
    RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *us_filters_cfg_table_ptr;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS     *filters_cfg_entry_ptr;
    int32_t                                         bridge_port_index;
    uint32_t                                        ip_fragment_error_filter;
    uint32_t                                        filters_cfg_entry;

    if ( ( bridge_port_index = rdd_bridge_port_to_port_index ( xi_bridge_port, xi_subnet_id ) ) < 0 )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID );
    }

    if ( xi_bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT || xi_bridge_port == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT || xi_bridge_port == BL_LILAC_RDD_WAN_ROUTER_PORT )
    {
        ds_filters_cfg_table_ptr = RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();
        filters_cfg_entry_ptr = &( ds_filters_cfg_table_ptr->entry[ bridge_port_index ] );
    }
    else
    {
        us_filters_cfg_table_ptr = RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();
        filters_cfg_entry_ptr = &( us_filters_cfg_table_ptr->entry[ bridge_port_index ] );
    }

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_READ ( filters_cfg_entry, filters_cfg_entry_ptr );

    if ( xi_header_error_filter_mode == BL_LILAC_RDD_FILTER_ENABLE )
    {
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_HEADER_ERROR_INGRESS_FILTER_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE );
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_VALIDATION_FILTER_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE );

        if ( xi_filter_action == BL_LILAC_RDD_FILTER_ACTION_CPU_TRAP )
        {
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_HEADER_ERROR_INGRESS_FILTER_TRAP_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_ENABLE );
        }
        else
        {
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_HEADER_ERROR_INGRESS_FILTER_TRAP_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE );
        }
    }
    else
    {
        RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_HEADER_ERROR_INGRESS_FILTER_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE );

        ip_fragment_error_filter = RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_FRAGMENT_INGRESS_FILTER_L_READ( filters_cfg_entry );

        if ( ip_fragment_error_filter == 0 )
        {
            RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_VALIDATION_FILTER_L_WRITE ( filters_cfg_entry, BL_LILAC_RDD_FILTER_DISABLE );
        }
    }

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_WRITE ( filters_cfg_entry, filters_cfg_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_virtual_port_config ( BL_LILAC_RDD_BRIDGE_PORT_VECTOR_DTE  xi_lan_port_vector )
{
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS     *bridge_cfg_register_ptr;
    RDD_HASH_BASED_FORWARDING_PORT_TABLE_DTS  *port_table_ptr;
    RDD_HASH_BASED_FORWARDING_PORT_ENTRY_DTS  *port_entry_ptr;
    BL_LILAC_RDD_BRIDGE_PORT_VECTOR_DTE       bridge_port_vector;
    BL_LILAC_RDD_BRIDGE_PORT_DTE              bridge_port;
    uint32_t                                  egress_port_count;

    /* fill ports mapping table */
    port_table_ptr = ( RDD_HASH_BASED_FORWARDING_PORT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + HASH_BASED_FORWARDING_PORT_TABLE_ADDRESS );

    for ( bridge_port_vector = BL_LILAC_RDD_BRIDGE_PORT_VECTOR_PCI, egress_port_count = 0;
          ( bridge_port_vector <= BL_LILAC_RDD_BRIDGE_PORT_VECTOR_LAN4 ) && ( egress_port_count < RDD_HASH_BASED_FORWARDING_PORT_TABLE_SIZE );
          bridge_port_vector <<= 1 )
    {
        if ( xi_lan_port_vector & bridge_port_vector )
        {
            port_entry_ptr = &( port_table_ptr->entry[ egress_port_count++ ] );

            bridge_port = rdd_bridge_port_vector_to_bridge_port ( bridge_port_vector );

            RDD_HASH_BASED_FORWARDING_PORT_ENTRY_EGRESS_PORT_WRITE ( bridge_port, port_entry_ptr );
        }
    }

    /* write number of ports participating in hash based forwarding */
    bridge_cfg_register_ptr = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );

    RDD_BRIDGE_CONFIGURATION_REGISTER_HASH_BASED_FORWARDING_PORT_COUNT_WRITE ( egress_port_count, bridge_cfg_register_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_ds_mac_unknown_da_forwarding_cfg ( uint32_t                      xi_mac_prefix,
                                                              BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                              uint16_t                      xi_wifi_ssid_vector,
                                                              BL_LILAC_RDD_POLICER_ID_DTE   xi_policer_id )
{
    RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_DTS  *unknown_da_forwarding_entry_ptr;

    unknown_da_forwarding_entry_ptr = ( RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_UNKNOWN_DA_FORWARDING_TABLE_ADDRESS );

    if ( xi_bridge_port >= BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT )
    {
        xi_bridge_port = ( ( xi_bridge_port / BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT ) | ( LILAC_RDD_MAC_CONTEXT_MULTICAST << 7 ) );
    }

    RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_BRIDGE_PORT_WRITE ( xi_bridge_port, unknown_da_forwarding_entry_ptr );
    RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_SSID_VECTOR_WRITE ( xi_wifi_ssid_vector, unknown_da_forwarding_entry_ptr );

    if ( xi_policer_id == BL_LILAC_RDD_POLICER_DISABLED )
    {
        RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_POLICER_ID_WRITE ( 0, unknown_da_forwarding_entry_ptr );
        RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_POLICER_ENABLE_WRITE ( LILAC_RDD_FALSE, unknown_da_forwarding_entry_ptr );
    }
    else
    {
        RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_POLICER_ID_WRITE ( xi_policer_id, unknown_da_forwarding_entry_ptr );
        RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_POLICER_ENABLE_WRITE ( LILAC_RDD_TRUE, unknown_da_forwarding_entry_ptr );
    }

    if ( xi_mac_prefix == MAC_UNKNOWN_DA_FORWARDING_FILTER_DISABLED )
    {
        RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_FILTER_ENABLE_WRITE ( LILAC_RDD_FALSE, unknown_da_forwarding_entry_ptr );
        RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_MAC_PREFIX_WRITE ( 0, unknown_da_forwarding_entry_ptr );
    }
    else
    {
        RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_FILTER_ENABLE_WRITE ( LILAC_RDD_TRUE, unknown_da_forwarding_entry_ptr );
        RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_MAC_PREFIX_WRITE ( xi_mac_prefix, unknown_da_forwarding_entry_ptr );
    }

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_upstream_unknown_da_flooding_bridge_port_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port )
{
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS  *bridge_cfg_register;

    if ( ( xi_bridge_port != BL_LILAC_RDD_WAN_BRIDGE_PORT ) && ( xi_bridge_port != BL_LILAC_RDD_CPU_BRIDGE_PORT ) )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID );
    }

    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );

    RDD_BRIDGE_CONFIGURATION_REGISTER_US_UNKNOWN_DA_FLOODING_BRIDGE_PORT_WRITE ( xi_bridge_port, bridge_cfg_register );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_bridge_flooding_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_ports_vector,
                                                    uint16_t                      xi_wifi_ssid_vector )
{
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS  *bridge_cfg_register;
    BL_LILAC_RDD_BRIDGE_PORT_DTE           us_bridge_ports_vector = xi_bridge_ports_vector;

    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );

    if ( xi_bridge_ports_vector >= BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT )
    {
        xi_bridge_ports_vector = ( ( xi_bridge_ports_vector / BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT ) | ( LILAC_RDD_MAC_CONTEXT_MULTICAST << 7 ) );
    }

    RDD_BRIDGE_CONFIGURATION_REGISTER_FLOODING_BRIDGE_PORTS_VECTOR_WRITE ( xi_bridge_ports_vector, bridge_cfg_register );
    RDD_BRIDGE_CONFIGURATION_REGISTER_FLOODING_WIFI_SSID_VECTOR_WRITE ( xi_wifi_ssid_vector, bridge_cfg_register );

    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );

    if ( us_bridge_ports_vector <= BL_LILAC_RDD_PCI_BRIDGE_PORT )
    {
        if ( us_bridge_ports_vector == BL_LILAC_RDD_LAN0_BRIDGE_PORT )
        {
            us_bridge_ports_vector = BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT;
        }
        else if ( us_bridge_ports_vector == BL_LILAC_RDD_LAN1_BRIDGE_PORT )
        {
            us_bridge_ports_vector = BL_LILAC_RDD_MULTICAST_LAN1_BRIDGE_PORT;
        }
        else if ( us_bridge_ports_vector == BL_LILAC_RDD_LAN2_BRIDGE_PORT )
        {
            us_bridge_ports_vector = BL_LILAC_RDD_MULTICAST_LAN2_BRIDGE_PORT;
        }
        else if ( us_bridge_ports_vector == BL_LILAC_RDD_LAN3_BRIDGE_PORT )
        {
            us_bridge_ports_vector = BL_LILAC_RDD_MULTICAST_LAN3_BRIDGE_PORT;
        }
        else if ( us_bridge_ports_vector == BL_LILAC_RDD_LAN4_BRIDGE_PORT )
        {
            us_bridge_ports_vector = BL_LILAC_RDD_MULTICAST_LAN4_BRIDGE_PORT;
        }
        else if ( us_bridge_ports_vector == BL_LILAC_RDD_PCI_BRIDGE_PORT )
        {
            us_bridge_ports_vector = BL_LILAC_RDD_MULTICAST_PCI_BRIDGE_PORT;
        }
    }

    if ( us_bridge_ports_vector >= BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT )
    {
        us_bridge_ports_vector = ( ( us_bridge_ports_vector / BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT ) | ( LILAC_RDD_MAC_CONTEXT_MULTICAST << 7 ) );
    }
    RDD_BRIDGE_CONFIGURATION_REGISTER_FLOODING_BRIDGE_PORTS_VECTOR_WRITE ( us_bridge_ports_vector, bridge_cfg_register );
    RDD_BRIDGE_CONFIGURATION_REGISTER_FLOODING_WIFI_SSID_VECTOR_WRITE ( xi_wifi_ssid_vector, bridge_cfg_register );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_mac_entry_add ( RDD_MAC_PARAMS  *xi_mac_params_ptr,
                                           uint32_t        *xo_mac_entry_index )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint8_t                 hash_entry[ 8 ];
    uint8_t                 context_entry[ 2 ];

    /* build hash entry */
    hash_entry[ 0 ] = 0;
    hash_entry[ 1 ] = 0;
    hash_entry[ 2 ] = xi_mac_params_ptr->mac_addr.b[ 0 ];
    hash_entry[ 3 ] = xi_mac_params_ptr->mac_addr.b[ 1 ];
    hash_entry[ 4 ] = xi_mac_params_ptr->mac_addr.b[ 2 ];
    hash_entry[ 5 ] = xi_mac_params_ptr->mac_addr.b[ 3 ];
    hash_entry[ 6 ] = xi_mac_params_ptr->mac_addr.b[ 4 ];
    hash_entry[ 7 ] = xi_mac_params_ptr->mac_addr.b[ 5 ];

    context_entry[ 0 ] = p_rdd_bridge_port_to_mac_context_bridge_port ( xi_mac_params_ptr->bridge_port, g_wan_physical_port );
    context_entry[ 1 ] = ( xi_mac_params_ptr->sa_action << 0 ) | ( xi_mac_params_ptr->da_action << 3 );

    f_rdd_lock ( &int_lock );

    /* check if entry exists, than manipulate it according to type (static/bridge) */
    rdd_error = rdd_find_entry_64_bit ( &g_hash_table_cfg[ BL_LILAC_RDD_MAC_TABLE ],
                                        hash_entry,
                                        MAC_ENTRY_KEY_MASK_HIGH,
                                        MAC_ENTRY_KEY_MASK_LOW,
                                        0,
                                        xo_mac_entry_index );

    if ( rdd_error == BL_LILAC_RDD_OK )
    {
        rdd_error = p_rdd_add_mac_entry_type_handle ( &( xi_mac_params_ptr->mac_addr ),
                                                      xi_mac_params_ptr->bridge_port,
                                                      xi_mac_params_ptr->aggregation_mode,
                                                      xi_mac_params_ptr->extension_entry,
                                                      xi_mac_params_ptr->sa_action,
                                                      xi_mac_params_ptr->da_action,
                                                      *xo_mac_entry_index );

        f_rdd_unlock ( &int_lock );
        return ( rdd_error );
    }

    if ( xi_mac_params_ptr->bridge_port < BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT )
    {
        if ( g_broadcom_switch_mode && xi_mac_params_ptr->bridge_port >= BL_LILAC_RDD_LAN0_BRIDGE_PORT && xi_mac_params_ptr->bridge_port <= BL_LILAC_RDD_LAN4_BRIDGE_PORT )
        {
            hash_entry[ 0 ] = ( BL_LILAC_RDD_LAN4_BRIDGE_PORT >> 4 ) & 0x01;
            hash_entry[ 1 ] = ( BL_LILAC_RDD_LAN4_BRIDGE_PORT << 4 ) & 0xF0;
        }
        else
        {
            hash_entry[ 0 ] = ( xi_mac_params_ptr->bridge_port >> 4 ) & 0x01;
            hash_entry[ 1 ] = ( xi_mac_params_ptr->bridge_port << 4 ) & 0xF0;
        }
    }

    /* add entry to table */
    rdd_error = rdd_add_hash_entry_64_bit ( &g_hash_table_cfg[ BL_LILAC_RDD_MAC_TABLE ],
                                            hash_entry,
                                            context_entry,
                                            MAC_ENTRY_KEY_MASK_HIGH,
                                            MAC_ENTRY_KEY_MASK_LOW,
                                            0,
                                            xo_mac_entry_index );

    if ( rdd_error )
    {
        f_rdd_unlock ( &int_lock );
        return ( rdd_error );
    }

    /* if added entry is static, mark corresponding bit in entry */
    if ( xi_mac_params_ptr->bridge_port == BL_LILAC_RDD_CPU_BRIDGE_PORT )
    {
        f_rdd_mac_entry_set_type_bit ( *xo_mac_entry_index, BL_LILAC_RDD_STATIC_MAC_ADDRESS );

        g_static_mac_counter[ *xo_mac_entry_index ]++;
    }
    else
    {
        f_rdd_mac_entry_set_type_bit ( *xo_mac_entry_index, BL_LILAC_RDD_BRIDGE_MAC_ADDRESS );
    }

    /* set extension entry (ssid or aggregation vid) */
    if ( ( xi_mac_params_ptr->bridge_port != BL_LILAC_RDD_PCI_BRIDGE_PORT ) && !( xi_mac_params_ptr->bridge_port >= BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT ) )
    {
        xi_mac_params_ptr->extension_entry |= ( xi_mac_params_ptr->aggregation_mode << 7 );
    }

    rdd_error = p_rdd_write_mac_extension_entry ( *xo_mac_entry_index, xi_mac_params_ptr->extension_entry );

    f_rdd_unlock ( &int_lock );
    return ( rdd_error );
}


BL_LILAC_RDD_ERROR_DTE rdd_mac_entry_delete ( bdmf_mac_t                       *xi_mac_addr,
                                              BL_LILAC_RDD_MAC_ENTRY_TYPE_DTE  xi_entry_type )
{
    BL_LILAC_RDD_ERROR_DTE       rdd_error;
    RDD_MAC_EXTENSION_TABLE_DTS  *mac_extension_table_ptr;
    RDD_MAC_EXTENSION_ENTRY_DTS  *mac_extension_entry_ptr;
    uint8_t                      hash_entry[ 8 ];
    uint8_t                      extension_entry;
    uint32_t                     new_entry_index;
    uint32_t                     entry_index;
    uint32_t                     type_handle_status;

    hash_entry[ 0 ] = 0;
    hash_entry[ 1 ] = 0;
    hash_entry[ 2 ] = xi_mac_addr->b[ 0 ];
    hash_entry[ 3 ] = xi_mac_addr->b[ 1 ];
    hash_entry[ 4 ] = xi_mac_addr->b[ 2 ];
    hash_entry[ 5 ] = xi_mac_addr->b[ 3 ];
    hash_entry[ 6 ] = xi_mac_addr->b[ 4 ];
    hash_entry[ 7 ] = xi_mac_addr->b[ 5 ];

    f_rdd_lock ( &int_lock );

    /* get index of deleted entry */
    rdd_error = rdd_find_entry_64_bit ( &g_hash_table_cfg[ BL_LILAC_RDD_MAC_TABLE ],
                                        hash_entry,
                                        MAC_ENTRY_KEY_MASK_HIGH,
                                        MAC_ENTRY_KEY_MASK_LOW,
                                        0,
                                        &new_entry_index );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        f_rdd_unlock ( &int_lock );
        return ( BL_LILAC_RDD_ERROR_MAC_ENTRY_DOESNT_EXIST );
    }

    /* perform logic for static/bridge entry */
    rdd_error = p_rdd_delete_mac_entry_type_handle ( xi_mac_addr, new_entry_index, xi_entry_type, &type_handle_status );

    if ( type_handle_status == LILAC_RDD_TRUE )
    {
        f_rdd_unlock ( &int_lock );
        return ( rdd_error );
    }

    rdd_error = rdd_remove_hash_entry_64_bit ( &g_hash_table_cfg[ BL_LILAC_RDD_MAC_TABLE ],
                                               hash_entry,
                                               MAC_ENTRY_KEY_MASK_HIGH,
                                               MAC_ENTRY_KEY_MASK_LOW,
                                               0,
                                               BL_LILAC_RDD_CAM_OPTIMIZATION_DISABLE,
                                               &entry_index );

    /* if deleted entry was in CAM, set extension entry of last entry to the deleted slot */
    if ( ( rdd_error == BL_LILAC_RDD_OK ) && ( entry_index >= g_mac_table_size ) && ( entry_index != new_entry_index ) )
    {
        mac_extension_table_ptr = ( RDD_MAC_EXTENSION_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_EXTENSION_TABLE_CAM_ADDRESS );

        /* get extension entry from last entry */
        mac_extension_entry_ptr = &( mac_extension_table_ptr->entry[ entry_index % g_mac_table_size ] );

        RDD_MAC_EXTENSION_ENTRY_EXTENSION_ENTRY_READ ( extension_entry, mac_extension_entry_ptr );
        RDD_MAC_EXTENSION_ENTRY_EXTENSION_ENTRY_WRITE ( 0, mac_extension_entry_ptr );

        /* set extension entry to optimized entry */
        mac_extension_entry_ptr = &( mac_extension_table_ptr->entry[ new_entry_index % g_mac_table_size ] );

        RDD_MAC_EXTENSION_ENTRY_EXTENSION_ENTRY_WRITE ( extension_entry, mac_extension_entry_ptr );

        g_static_mac_counter[ new_entry_index ] = g_static_mac_counter[ entry_index ];
    }

    g_static_mac_counter[ entry_index ] = 0;

    f_rdd_unlock ( &int_lock );
    return ( rdd_error );
}


BL_LILAC_RDD_ERROR_DTE rdd_mac_entry_modify ( RDD_MAC_PARAMS  *xi_mac_params_ptr )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;

    f_rdd_lock ( &int_lock );

    rdd_error = f_rdd_mac_entry_modify ( xi_mac_params_ptr );

    f_rdd_unlock ( &int_lock );
    return ( rdd_error );
}


BL_LILAC_RDD_ERROR_DTE rdd_mac_entry_search ( RDD_MAC_PARAMS  *xi_mac_params_ptr,
                                              uint32_t        *xo_entry_index )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint8_t                 hash_entry[ 8 ];

    hash_entry[ 0 ] = 0;
    hash_entry[ 1 ] = 0;
    hash_entry[ 2 ] = xi_mac_params_ptr->mac_addr.b[ 0 ];
    hash_entry[ 3 ] = xi_mac_params_ptr->mac_addr.b[ 1 ];
    hash_entry[ 4 ] = xi_mac_params_ptr->mac_addr.b[ 2 ];
    hash_entry[ 5 ] = xi_mac_params_ptr->mac_addr.b[ 3 ];
    hash_entry[ 6 ] = xi_mac_params_ptr->mac_addr.b[ 4 ];
    hash_entry[ 7 ] = xi_mac_params_ptr->mac_addr.b[ 5 ];

    rdd_error = rdd_find_entry_64_bit ( &g_hash_table_cfg[ BL_LILAC_RDD_MAC_TABLE ],
                                        hash_entry,
                                        MAC_ENTRY_KEY_MASK_HIGH,
                                        MAC_ENTRY_KEY_MASK_LOW,
                                        0,
                                        xo_entry_index );

    if ( rdd_error )
    {
        return ( BL_LILAC_RDD_ERROR_GET_MAC_ENTRY );
    }

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_mac_entry_get ( uint32_t        xi_entry_index,
                                           RDD_MAC_PARAMS  *xo_mac_params_ptr,
                                           uint32_t        *xo_valid_bit,
                                           uint32_t        *xo_skip_bit,
                                           uint32_t        *xo_aging_bit )
{
    RDD_MAC_TABLE_DTS            *mac_table_ptr;
    RDD_MAC_ENTRY_DTS            *mac_entry_ptr;
    RDD_MAC_EXTENSION_TABLE_DTS  *mac_extension_table_ptr;
    RDD_MAC_EXTENSION_ENTRY_DTS  *mac_extension_entry_ptr;
    RDD_MAC_CONTEXT_TABLE_DTS    *forward_table_ptr;
    RDD_MAC_CONTEXT_ENTRY_DTS    *forward_entry_ptr;
    uint32_t                     forward_entry_multicast;
    uint32_t                     multicast_vector_msb;

    /* check the validity of the input parameters - MAC entry index */
    if ( xi_entry_index >= ( g_mac_table_size + RDD_MAC_TABLE_CAM_SIZE ) )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_MAC_ENTRY_ID );
    }

    if ( xi_entry_index < g_mac_table_size )
    {
        mac_table_ptr = ( RDD_MAC_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_TABLE_ADDRESS );

        forward_table_ptr = ( RDD_MAC_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_CONTEXT_TABLE_ADDRESS );

        mac_extension_table_ptr = ( RDD_MAC_EXTENSION_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_EXTENSION_TABLE_ADDRESS );
    }
    else
    {
        mac_table_ptr = ( RDD_MAC_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_TABLE_CAM_ADDRESS );

        forward_table_ptr = ( RDD_MAC_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_CONTEXT_TABLE_CAM_ADDRESS );

        mac_extension_table_ptr = ( RDD_MAC_EXTENSION_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_EXTENSION_TABLE_CAM_ADDRESS );

        xi_entry_index %= g_mac_table_size;
    }

    /* read MAC table entry */
    mac_entry_ptr = &( mac_table_ptr->entry[ xi_entry_index ] );

    /* retreive the MAC entry information - MAC address */
    RDD_MAC_ENTRY_MAC_ADDR0_READ ( xo_mac_params_ptr->mac_addr.b[ 0 ], mac_entry_ptr );
    RDD_MAC_ENTRY_MAC_ADDR1_READ ( xo_mac_params_ptr->mac_addr.b[ 1 ], mac_entry_ptr );
    RDD_MAC_ENTRY_MAC_ADDR2_READ ( xo_mac_params_ptr->mac_addr.b[ 2 ], mac_entry_ptr );
    RDD_MAC_ENTRY_MAC_ADDR3_READ ( xo_mac_params_ptr->mac_addr.b[ 3 ], mac_entry_ptr );
    RDD_MAC_ENTRY_MAC_ADDR4_READ ( xo_mac_params_ptr->mac_addr.b[ 4 ], mac_entry_ptr );
    RDD_MAC_ENTRY_MAC_ADDR5_READ ( xo_mac_params_ptr->mac_addr.b[ 5 ], mac_entry_ptr );

    /* retrieve forwarding information */
    forward_entry_ptr = &( forward_table_ptr->entry[ xi_entry_index ] );

    RDD_MAC_CONTEXT_ENTRY_EGRESS_PORT_READ ( xo_mac_params_ptr->bridge_port, forward_entry_ptr );
    RDD_MAC_CONTEXT_ENTRY_SA_ACTION_READ ( xo_mac_params_ptr->sa_action, forward_entry_ptr );
    RDD_MAC_CONTEXT_ENTRY_DA_ACTION_READ ( xo_mac_params_ptr->da_action, forward_entry_ptr );
    RDD_MAC_CONTEXT_ENTRY_MULTICAST_READ ( forward_entry_multicast, forward_entry_ptr );

    if ( forward_entry_multicast )
    {
        /* multicast egress ports vector consists of two fields in forward entry */
        RDD_MAC_CONTEXT_ENTRY_MULTICAST_VECTOR_READ ( multicast_vector_msb, forward_entry_ptr );

        xo_mac_params_ptr->bridge_port = ( xo_mac_params_ptr->bridge_port | ( multicast_vector_msb << 5 ) ) << 4;
    }

    RDD_MAC_CONTEXT_ENTRY_MAC_TYPE_READ ( xo_mac_params_ptr->entry_type, forward_entry_ptr );

    mac_extension_entry_ptr = &( mac_extension_table_ptr->entry[ xi_entry_index ] );

    RDD_MAC_EXTENSION_ENTRY_EXTENSION_ENTRY_READ ( xo_mac_params_ptr->extension_entry, mac_extension_entry_ptr );

    if ( xo_mac_params_ptr->extension_entry & ( 1 << 7 ) )
    {
        xo_mac_params_ptr->extension_entry &= ~( 1 << 7 );
        xo_mac_params_ptr->aggregation_mode = BL_LILAC_RDD_AGGREGATION_MODE_ENABLED;
    }
    else
    {
        xo_mac_params_ptr->aggregation_mode = BL_LILAC_RDD_AGGREGATION_MODE_DISABLED;
    }

    /* retreive the entry's control bits */
    RDD_MAC_ENTRY_VALID_READ ( *xo_valid_bit, mac_entry_ptr );
    RDD_MAC_ENTRY_SKIP_READ ( *xo_skip_bit, mac_entry_ptr );
    RDD_MAC_ENTRY_AGING_READ ( *xo_aging_bit, mac_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_mac_entry_aging_set ( uint32_t  xi_entry_index,
                                                 uint32_t  *xo_activity_status )
{
    RDD_MAC_TABLE_DTS  *mac_table_ptr;
    RDD_MAC_ENTRY_DTS  *mac_entry_ptr;

    /* check the validity of the input parameters - MAC entry index */
    if ( xi_entry_index >= ( g_mac_table_size + RDD_MAC_TABLE_CAM_SIZE - 1 ) )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_MAC_ENTRY_ID );
    }

    f_rdd_lock ( &int_lock );

    if ( xi_entry_index < g_mac_table_size )
    {
        mac_table_ptr = ( RDD_MAC_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_TABLE_ADDRESS );
    }
    else
    {
        mac_table_ptr = ( RDD_MAC_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_TABLE_CAM_ADDRESS );

        xi_entry_index %= g_mac_table_size;
    }

    /* read MAC table entry and set it's aging bit */
    mac_entry_ptr = &( mac_table_ptr->entry[ xi_entry_index ] );

    RDD_MAC_ENTRY_AGING_READ ( *xo_activity_status, mac_entry_ptr );
    RDD_MAC_ENTRY_AGING_WRITE ( LILAC_RDD_ON, mac_entry_ptr );

    f_rdd_unlock ( &int_lock );
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_clear_mac_table ( void )
{
    RDD_MAC_TABLE_DTS  *mac_table_ptr;
    RDD_MAC_ENTRY_DTS  *mac_entry_ptr;
    uint32_t           i;

    f_rdd_lock ( &int_lock );

    mac_table_ptr = ( RDD_MAC_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_TABLE_ADDRESS );

    for ( i = 0; i < g_mac_table_size; i++ )
    {
        /* read MAC table entry and clear it */
        mac_entry_ptr = &( mac_table_ptr->entry[ i ] );

        /* reset the whole MAC table */
        MEMSET ( mac_entry_ptr, 0, sizeof ( RDD_MAC_ENTRY_DTS ) );
    }

    mac_table_ptr = ( RDD_MAC_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_TABLE_CAM_ADDRESS );

    for ( i = 0; i < RDD_MAC_TABLE_CAM_SIZE; i++ )
    {
        /* read MAC table CAM entry and clear it */
        mac_entry_ptr = &( mac_table_ptr->entry[ i ] );

        /* reset the whole CAM table */
        MEMSET ( mac_entry_ptr, 0, sizeof ( RDD_MAC_ENTRY_DTS ) );
    }

    f_rdd_unlock ( &int_lock );
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE p_rdd_write_mac_extension_entry ( uint32_t  xi_entry_index,
                                                         uint8_t   xi_extension_entry )
{
    RDD_MAC_EXTENSION_TABLE_DTS  *mac_extension_table_ptr;
    RDD_MAC_EXTENSION_ENTRY_DTS  *mac_extension_entry_ptr;

    if ( xi_entry_index < g_mac_table_size )
    {
        mac_extension_table_ptr = ( RDD_MAC_EXTENSION_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_EXTENSION_TABLE_ADDRESS );

        mac_extension_entry_ptr = &( mac_extension_table_ptr->entry[ xi_entry_index ] );
    }
    else
    {
        mac_extension_table_ptr = ( RDD_MAC_EXTENSION_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_EXTENSION_TABLE_CAM_ADDRESS );

        mac_extension_entry_ptr = &( mac_extension_table_ptr->entry[ xi_entry_index % g_mac_table_size ] );
    }

    RDD_MAC_EXTENSION_ENTRY_EXTENSION_ENTRY_WRITE ( xi_extension_entry, mac_extension_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE p_rdd_add_mac_entry_type_handle ( bdmf_mac_t                         *xi_mac_addr,
                                                         BL_LILAC_RDD_BRIDGE_PORT_DTE       xi_bridge_port,
                                                         BL_LILAC_RDD_AGGREGATION_MODE_DTE  xi_aggregation_mode,
                                                         uint8_t                            xi_extension_entry,
                                                         BL_LILAC_RDD_MAC_FWD_ACTION_DTE    xi_sa_action,
                                                         BL_LILAC_RDD_MAC_FWD_ACTION_DTE    xi_da_action,
                                                         uint32_t                           xi_entry_index )
{
    RDD_MAC_PARAMS          mac_params;
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint32_t                dummy_entry;
    
    rdd_mac_entry_get ( xi_entry_index,
                        &mac_params,
                        &dummy_entry,
                        &dummy_entry,
                        &dummy_entry );

    if ( xi_bridge_port == BL_LILAC_RDD_CPU_BRIDGE_PORT )
    {
        /* adding static entry - increment static counter */
        g_static_mac_counter[ xi_entry_index ]++;

        return ( BL_LILAC_RDD_OK );
    }
    else if ( mac_params.bridge_port == BL_LILAC_RDD_CPU_BRIDGE_PORT )
    {
        /* adding bridge entry to entry that was static - change entry parameters */

        memcpy ( mac_params.mac_addr.b, xi_mac_addr->b, 6 );
        mac_params.bridge_port = xi_bridge_port;
        mac_params.aggregation_mode = xi_aggregation_mode;
        mac_params.extension_entry = xi_extension_entry;
        mac_params.sa_action = xi_sa_action;
        mac_params.da_action = xi_da_action;

        rdd_error = f_rdd_mac_entry_modify ( &mac_params );

        f_rdd_mac_entry_set_type_bit ( xi_entry_index, BL_LILAC_RDD_BRIDGE_MAC_ADDRESS );

        return ( rdd_error );
    }
    else
    {
        return ( BL_LILAC_RDD_ERROR_MAC_ENTRY_EXISTS );
    }
}


BL_LILAC_RDD_ERROR_DTE p_rdd_delete_mac_entry_type_handle ( bdmf_mac_t                       *xi_mac_addr,
                                                            uint32_t                         xi_entry_index,
                                                            BL_LILAC_RDD_MAC_ENTRY_TYPE_DTE  xi_entry_type,
                                                            uint32_t                         *xo_type_handle_status )
{
    RDD_MAC_PARAMS          mac_params;
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint32_t                dummy_entry;

    *xo_type_handle_status = LILAC_RDD_TRUE;

    rdd_mac_entry_get ( xi_entry_index,
                        &mac_params,
                        &dummy_entry,
                        &dummy_entry,
                        &dummy_entry );

    if ( xi_entry_type == BL_LILAC_RDD_BRIDGE_MAC_ADDRESS )
    {
        /* deleting bridge entry */
        if ( mac_params.bridge_port == BL_LILAC_RDD_CPU_BRIDGE_PORT )
        {
            /* given mac not associated to bridge */
            return ( BL_LILAC_RDD_ERROR_MAC_ENTRY_DOESNT_EXIST );
        }

        if ( g_static_mac_counter[ xi_entry_index ] > 0 )
        {
            /* entry is also static - remove bridge entry */
            memcpy ( mac_params.mac_addr.b, xi_mac_addr->b, 6 );
            mac_params.bridge_port = BL_LILAC_RDD_CPU_BRIDGE_PORT;
            mac_params.entry_type = 0;
            mac_params.aggregation_mode = 0;
            mac_params.extension_entry = 0;
            mac_params.sa_action = 0;
            mac_params.da_action = 0;
            rdd_error = f_rdd_mac_entry_modify ( &mac_params );

            f_rdd_mac_entry_set_type_bit ( xi_entry_index, BL_LILAC_RDD_STATIC_MAC_ADDRESS );

            return ( rdd_error );
        }
    }
    else
    {
        /* deleting static entry */
        if ( g_static_mac_counter[ xi_entry_index ] == 0 )
        {
            /* entry isn't static */
            return ( BL_LILAC_RDD_ERROR_MAC_ENTRY_DOESNT_EXIST );
        }

        if ( g_static_mac_counter[ xi_entry_index ] > 1 )
        {
            /* more than one static entry associated - decrement static counter */
            g_static_mac_counter[ xi_entry_index ]--;

            return ( BL_LILAC_RDD_OK );
        }

        if ( g_static_mac_counter[ xi_entry_index ] == 1 && mac_params.bridge_port != BL_LILAC_RDD_CPU_BRIDGE_PORT )
        {
            /* last associated static entry deleted - leave only bridge entry */
            g_static_mac_counter[ xi_entry_index ] = 0;

            return ( BL_LILAC_RDD_OK );
        }
    }

    /* no condition reached, proceed with deleting entry from table */
    *xo_type_handle_status = LILAC_RDD_FALSE;
    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE f_rdd_mac_entry_modify ( RDD_MAC_PARAMS  *xi_mac_params_ptr )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint8_t                 hash_entry[ 8 ];
    uint8_t                 context_entry[ 2 ];
    uint32_t                entry_index;

    if ( xi_mac_params_ptr->bridge_port < BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT )
    {
        if ( g_broadcom_switch_mode && xi_mac_params_ptr->bridge_port >= BL_LILAC_RDD_LAN0_BRIDGE_PORT && xi_mac_params_ptr->bridge_port <= BL_LILAC_RDD_LAN4_BRIDGE_PORT )
        {
            hash_entry[ 0 ] = ( BL_LILAC_RDD_LAN4_BRIDGE_PORT >> 4 ) & 0x01;
            hash_entry[ 1 ] = ( BL_LILAC_RDD_LAN4_BRIDGE_PORT << 4 ) & 0xF0;
        }
        else
        {
            hash_entry[ 0 ] = ( xi_mac_params_ptr->bridge_port >> 4 ) & 0x01;
            hash_entry[ 1 ] = ( xi_mac_params_ptr->bridge_port << 4 ) & 0xF0;
        }
    }
    else
    {
        hash_entry[ 0 ] = 0;
        hash_entry[ 1 ] = 0;
    }
    hash_entry[ 2 ] = xi_mac_params_ptr->mac_addr.b[ 0 ];
    hash_entry[ 3 ] = xi_mac_params_ptr->mac_addr.b[ 1 ];
    hash_entry[ 4 ] = xi_mac_params_ptr->mac_addr.b[ 2 ];
    hash_entry[ 5 ] = xi_mac_params_ptr->mac_addr.b[ 3 ];
    hash_entry[ 6 ] = xi_mac_params_ptr->mac_addr.b[ 4 ];
    hash_entry[ 7 ] = xi_mac_params_ptr->mac_addr.b[ 5 ];

    context_entry[ 0 ] = p_rdd_bridge_port_to_mac_context_bridge_port ( xi_mac_params_ptr->bridge_port, g_wan_physical_port );
    context_entry[ 1 ] = ( xi_mac_params_ptr->sa_action << 0 ) | (xi_mac_params_ptr->da_action << 3 ) | LILAC_RDD_MAC_CONTEXT_ENTRY_TYPE_MASK;

    rdd_error = rdd_modify_hash_entry_64_bit ( &g_hash_table_cfg[ BL_LILAC_RDD_MAC_TABLE ],
                                               hash_entry,
                                               context_entry,
                                               MAC_ENTRY_KEY_MASK_HIGH,
                                               MAC_ENTRY_KEY_MASK_LOW,
                                               MAC_ENTRY_INTERNAL_CONTEXT_MASK_HIGH,
                                               MAC_ENTRY_INTERNAL_CONTEXT_MASK_LOW,
                                               0,
                                               &entry_index );

    if ( rdd_error )
    {
        return ( rdd_error );
    }

    /* write extension entry (ssid or aggregation vid index) */
    if ( ( xi_mac_params_ptr->bridge_port != BL_LILAC_RDD_PCI_BRIDGE_PORT ) && !( xi_mac_params_ptr->bridge_port >= BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT ) )
    {
        if ( xi_mac_params_ptr->aggregation_mode == BL_LILAC_RDD_AGGREGATION_MODE_DISABLED )
        {
            xi_mac_params_ptr->extension_entry &= ~( 1 << 7 );
        }
        else
        {
            xi_mac_params_ptr->extension_entry |= ( 1 << 7 );
        }
    }

    rdd_error = p_rdd_write_mac_extension_entry ( entry_index, xi_mac_params_ptr->extension_entry );

    return ( rdd_error );
}


void f_rdd_mac_entry_set_type_bit ( uint32_t                         xi_entry_index,
                                    BL_LILAC_RDD_MAC_ENTRY_TYPE_DTE  xi_entry_type )
{
    RDD_MAC_CONTEXT_TABLE_DTS  *mac_context_table_ptr;
    RDD_MAC_CONTEXT_ENTRY_DTS  *mac_context_entry_ptr;

    if ( xi_entry_index < g_mac_table_size )
    {
        mac_context_table_ptr = ( RDD_MAC_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_CONTEXT_TABLE_ADDRESS );
    }
    else
    {
        mac_context_table_ptr = ( RDD_MAC_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_CONTEXT_TABLE_CAM_ADDRESS );
    }

    mac_context_entry_ptr = &( mac_context_table_ptr->entry[ xi_entry_index % g_mac_table_size ] );

    RDD_MAC_CONTEXT_ENTRY_MAC_TYPE_WRITE ( xi_entry_type, mac_context_entry_ptr );
}


/* Tal Meged Debug functions */
BL_LILAC_RDD_ERROR_DTE bl_lilac_rdd_set_mac_table ( void )
{
    RDD_MAC_TABLE_DTS  *mac_table_ptr;
    RDD_MAC_ENTRY_DTS  *mac_entry_ptr;
    uint32_t           i;

    f_rdd_lock ( &int_lock );

    mac_table_ptr = ( RDD_MAC_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_TABLE_ADDRESS );

    for ( i = 0; i < g_mac_table_size; i++ )
    {
        /* read MAC table entry and clear it */
        mac_entry_ptr = &( mac_table_ptr->entry[ i ] );

        /* reset the whole MAC table */
        RDD_MAC_ENTRY_VALID_WRITE ( LILAC_RDD_ON, mac_entry_ptr );
    }

    f_rdd_unlock ( &int_lock );
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_forwarding_matrix_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE     xi_src_bridge_port,
                                                      BL_LILAC_RDD_BRIDGE_PORT_DTE     xi_dst_bridge_port,
                                                      BL_LILAC_RDD_FORWARD_MATRIX_DTE  xi_enable )
{
    RDD_DS_FORWARDING_MATRIX_TABLE_DTS  *ds_forwarding_matrix_table_ptr;
    RDD_US_FORWARDING_MATRIX_TABLE_DTS  *us_forwarding_matrix_table_ptr;
    RDD_FORWARDING_MATRIX_ENTRY_DTS  *forwarding_matrix_entry_ptr;
    uint8_t                          forward_matrix_enable;

    ds_forwarding_matrix_table_ptr = RDD_DS_FORWARDING_MATRIX_TABLE_PTR();

    forwarding_matrix_entry_ptr = &( ds_forwarding_matrix_table_ptr->entry[ xi_src_bridge_port ][ xi_dst_bridge_port ] );

    forward_matrix_enable = xi_enable;

    RDD_FORWARDING_MATRIX_ENTRY_ENABLE_WRITE ( forward_matrix_enable, forwarding_matrix_entry_ptr );


    us_forwarding_matrix_table_ptr = RDD_US_FORWARDING_MATRIX_TABLE_PTR();

    forwarding_matrix_entry_ptr = &( us_forwarding_matrix_table_ptr->entry[ xi_src_bridge_port ][ xi_dst_bridge_port ] );

    forward_matrix_enable = xi_enable;

    RDD_FORWARDING_MATRIX_ENTRY_ENABLE_WRITE ( forward_matrix_enable, forwarding_matrix_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_wifi_ssid_forwarding_matrix_config ( uint16_t                      xi_wifi_ssid_vector,
                                                                BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_dst_bridge_port )
{
    RDD_WIFI_SSID_FORWARDING_MATRIX_TABLE_DTS  *forwarding_matrix_table_ptr;
    RDD_WIFI_SSID_FORWARDING_MATRIX_ENTRY_DTS  *forwarding_matrix_entry_ptr;

    forwarding_matrix_table_ptr = ( RDD_WIFI_SSID_FORWARDING_MATRIX_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + WIFI_SSID_FORWARDING_MATRIX_TABLE_ADDRESS - sizeof ( RUNNER_COMMON ) );

    forwarding_matrix_entry_ptr = &( forwarding_matrix_table_ptr->entry[ xi_dst_bridge_port ] );

    RDD_WIFI_SSID_FORWARDING_MATRIX_ENTRY_WIFI_SSID_VECTOR_WRITE( xi_wifi_ssid_vector, forwarding_matrix_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_egress_ethertype_config ( uint16_t  xi_1st_ether_type,
                                                     uint16_t  xi_2nd_ether_type )
{
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS  *bridge_cfg_register;

    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );

    RDD_BRIDGE_CONFIGURATION_REGISTER_EGRESS_ETHER_TYPE_1_WRITE ( xi_1st_ether_type, bridge_cfg_register );
    RDD_BRIDGE_CONFIGURATION_REGISTER_EGRESS_ETHER_TYPE_2_WRITE ( xi_2nd_ether_type, bridge_cfg_register );

    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );

    RDD_BRIDGE_CONFIGURATION_REGISTER_EGRESS_ETHER_TYPE_1_WRITE ( xi_1st_ether_type, bridge_cfg_register );
    RDD_BRIDGE_CONFIGURATION_REGISTER_EGRESS_ETHER_TYPE_2_WRITE ( xi_2nd_ether_type, bridge_cfg_register );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_vlan_command_always_egress_ether_type_config ( uint16_t  xi_3rd_ether_type )
{
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS  *bridge_cfg_register;

    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );

    RDD_BRIDGE_CONFIGURATION_REGISTER_EGRESS_ETHER_TYPE_3_WRITE ( xi_3rd_ether_type, bridge_cfg_register );

    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );

    RDD_BRIDGE_CONFIGURATION_REGISTER_EGRESS_ETHER_TYPE_3_WRITE ( xi_3rd_ether_type, bridge_cfg_register );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_src_mac_anti_spoofing_lookup_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                                 BL_LILAC_RDD_FILTER_MODE_DTE  xi_filter_mode )
{
    RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS  *us_filters_cfg_table_ptr;
    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS     *filters_cfg_entry_ptr;
    int32_t                                         bridge_port_index;

    us_filters_cfg_table_ptr = RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR();

    if ( ( bridge_port_index = rdd_bridge_port_to_port_index ( xi_bridge_port, 0 ) ) < 0 )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID );
    }

    filters_cfg_entry_ptr = &( us_filters_cfg_table_ptr->entry[ bridge_port_index ] );

    RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_SRC_MAC_ANTI_SPOOFING_LOOKUP_WRITE ( xi_filter_mode, filters_cfg_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_src_mac_anti_spoofing_entry_add ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                             uint32_t                      xi_src_mac_prefix )
{
    RDD_SRC_MAC_ANTI_SPOOFING_TABLE_DTS  *src_mac_anti_spoofing_table_ptr;
    RDD_SRC_MAC_ANTI_SPOOFING_ENTRY_DTS  *src_mac_anti_spoofing_entry_ptr;
    int32_t                              bridge_port_index;

    bridge_port_index = rdd_bridge_port_to_port_index ( xi_bridge_port, 0 );

    if ( g_src_mac_anti_spoofing_last_rule_index[ bridge_port_index ] == RDD_SRC_MAC_ANTI_SPOOFING_TABLE_SIZE2 )
    {
        return ( BL_LILAC_RDD_ERROR_CAM_LOOKUP_TABLE_FULL );
    }

    src_mac_anti_spoofing_table_ptr = ( RDD_SRC_MAC_ANTI_SPOOFING_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + SRC_MAC_ANTI_SPOOFING_TABLE_ADDRESS - sizeof ( RUNNER_COMMON ) );

    src_mac_anti_spoofing_entry_ptr = &( src_mac_anti_spoofing_table_ptr->entry[ bridge_port_index ][ g_src_mac_anti_spoofing_last_rule_index[ bridge_port_index ] ] );

    RDD_SRC_MAC_ANTI_SPOOFING_ENTRY_MAC_PREFIX_WRITE ( xi_src_mac_prefix, src_mac_anti_spoofing_entry_ptr );

    g_src_mac_anti_spoofing_last_rule_index[ bridge_port_index ]++;

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_src_mac_anti_spoofing_entry_delete  ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                                 uint32_t                      xi_src_mac_prefix )
{
    RDD_SRC_MAC_ANTI_SPOOFING_TABLE_DTS  *src_mac_anti_spoofing_table_ptr;
    RDD_SRC_MAC_ANTI_SPOOFING_ENTRY_DTS  *src_mac_anti_spoofing_entry_ptr;
    int32_t                              bridge_port_index;
    uint32_t                             src_mac_anti_spoofing_entry;
    uint32_t                             i;

    src_mac_anti_spoofing_table_ptr = ( RDD_SRC_MAC_ANTI_SPOOFING_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + SRC_MAC_ANTI_SPOOFING_TABLE_ADDRESS - sizeof ( RUNNER_COMMON ) );

    bridge_port_index = rdd_bridge_port_to_port_index ( xi_bridge_port, 0 );

    for ( i = 0; i < g_src_mac_anti_spoofing_last_rule_index[ bridge_port_index ]; i++ )
    {
        src_mac_anti_spoofing_entry_ptr = &( src_mac_anti_spoofing_table_ptr->entry[ bridge_port_index ][ i ] );

        RDD_SRC_MAC_ANTI_SPOOFING_ENTRY_MAC_PREFIX_READ ( src_mac_anti_spoofing_entry, src_mac_anti_spoofing_entry_ptr );

        if ( src_mac_anti_spoofing_entry == xi_src_mac_prefix )
        {
            break;
        }
    }

    /* the requested entry was not found */
    if ( i == g_src_mac_anti_spoofing_last_rule_index[ bridge_port_index ] )
    {
        return ( BL_LILAC_RDD_ERROR_CAM_LOOKUP_FAILED );
    }

    /* the deleted entry is the last entry in the cam */
    if ( i == ( g_src_mac_anti_spoofing_last_rule_index[ bridge_port_index ] - 1 ) )
    {
        RDD_SRC_MAC_ANTI_SPOOFING_ENTRY_MAC_PREFIX_WRITE ( 0xFFFFFFFF, src_mac_anti_spoofing_entry_ptr );

        g_src_mac_anti_spoofing_last_rule_index[ bridge_port_index ]--;

        return ( BL_LILAC_RDD_OK );
    }

    /* replace the last entry in the cam with the one to be deleted */
    g_src_mac_anti_spoofing_last_rule_index[ bridge_port_index ]--;

    /* invalidate the deleted entry */
    src_mac_anti_spoofing_entry_ptr = &( src_mac_anti_spoofing_table_ptr->entry[ bridge_port_index ][ i ] );

    RDD_SRC_MAC_ANTI_SPOOFING_ENTRY_MAC_PREFIX_WRITE ( 0, src_mac_anti_spoofing_entry_ptr );

    /* copy last cam entry over deleted entry */
    src_mac_anti_spoofing_entry_ptr =
        &( src_mac_anti_spoofing_table_ptr->entry[ bridge_port_index ][ g_src_mac_anti_spoofing_last_rule_index[ bridge_port_index ] ] );

    RDD_SRC_MAC_ANTI_SPOOFING_ENTRY_MAC_PREFIX_READ ( src_mac_anti_spoofing_entry, src_mac_anti_spoofing_entry_ptr );

    src_mac_anti_spoofing_entry_ptr = &( src_mac_anti_spoofing_table_ptr->entry[ bridge_port_index ][ i ] );

    RDD_SRC_MAC_ANTI_SPOOFING_ENTRY_MAC_PREFIX_WRITE ( src_mac_anti_spoofing_entry, src_mac_anti_spoofing_entry_ptr );

    src_mac_anti_spoofing_entry_ptr = &( src_mac_anti_spoofing_table_ptr->entry[ bridge_port_index ][ g_src_mac_anti_spoofing_last_rule_index[ bridge_port_index ] ] );

    RDD_SRC_MAC_ANTI_SPOOFING_ENTRY_MAC_PREFIX_WRITE ( 0xFFFFFFFF, src_mac_anti_spoofing_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_1588_master_rx_get_entry ( uint16_t  xi_entry_index,
                                                      uint32_t  *xo_tod_high,
                                                      uint32_t  *xo_tod_low,
                                                      uint16_t  *xo_local_clock_delta )
{
    RDD_IP_SYNC_1588_DESCRIPTOR_ENTRY_DTS    *ip_sync_1588_descriptor_queue_ptr;

    ip_sync_1588_descriptor_queue_ptr = ( RDD_IP_SYNC_1588_DESCRIPTOR_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + IP_SYNC_1588_DESCRIPTOR_QUEUE_ADDRESS  - sizeof ( RUNNER_COMMON ) );
    ip_sync_1588_descriptor_queue_ptr = ip_sync_1588_descriptor_queue_ptr + xi_entry_index;

    RDD_IP_SYNC_1588_DESCRIPTOR_ENTRY_TOD_HIGH_READ ( *xo_tod_high, ip_sync_1588_descriptor_queue_ptr );
    RDD_IP_SYNC_1588_DESCRIPTOR_ENTRY_TOD_LOW_READ ( *xo_tod_low, ip_sync_1588_descriptor_queue_ptr );
    RDD_IP_SYNC_1588_DESCRIPTOR_ENTRY_LOCAL_TIME_DELTA_READ ( *xo_local_clock_delta, ip_sync_1588_descriptor_queue_ptr );

	return ( BL_LILAC_RDD_OK );
}

#ifdef G9991
BL_LILAC_RDD_ERROR_DTE rdd_ingress_filters_profile_config ( BL_LILAC_RDD_EMAC_ID_DTE    bridge_port,
                                                            uint8_t	                    xi_profile_idx )
{
    RDD_INGRESS_FILTERS_PROFILE_TABLE_DTS   *ingress_filter_profile_table_ptr;
    uint8_t                                 *ingress_filter_profile_table_entry_ptr;

    ingress_filter_profile_table_ptr = RDD_INGRESS_FILTERS_PROFILE_TABLE_PTR();
    ingress_filter_profile_table_entry_ptr = (uint8_t *)&( ingress_filter_profile_table_ptr->entry[ bridge_port ] );
    MWRITE_8( ingress_filter_profile_table_entry_ptr, xi_profile_idx );

 	return ( BL_LILAC_RDD_OK );
}
#endif
