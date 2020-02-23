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

#ifdef USE_BDMF_SHELL

#include "bdmf_shell.h"
#include "rdd.h"
#include "rdd_runner_reg_dump.h"
#include "rdd_runner_reg_dump_addrs.h"
#ifdef CONFIG_DHD_RUNNER
#include "rdd_dhd_helper.h"
#endif
#include "rdd_legacy_conv.h"
#include "rdd_service_queues.h"
#ifdef CONFIG_BCM_PKTRUNNER_GSO
#include "rdd_gso.h"
#endif

extern uint8_t  *g_runner_ddr_base_addr;
extern uint32_t  g_runner_tables_ptr;
extern uint32_t  g_cpu_tx_skb_free_indexes_counter;
extern uint32_t  g_cpu_tx_released_skb_counter;
extern uint32_t  g_cpu_tx_no_free_skb_counter;
extern uint32_t  g_cpu_tx_sent_abs_packets_counter;
extern uint16_t  *g_free_skb_indexes_fifo_table_physical_address;
extern uint16_t  *g_free_skb_indexes_fifo_table_physical_address_last_idx;
extern RDD_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTE  g_ingress_classification_rule_cfg_table[ 2 ];
#ifdef FIRMWARE_INIT
extern uint32_t  UsConnectionTableBase;
extern uint32_t  DsConnectionTableBase;
extern uint32_t  ContextTableBase;
#endif
extern BL_LILAC_RDD_LOCK_CRITICAL_SECTION_FP_IRQ_DTE    f_rdd_lock_irq;
extern BL_LILAC_RDD_UNLOCK_CRITICAL_SECTION_FP_IRQ_DTE  f_rdd_unlock_irq;
extern bdmf_fastlock                                    int_lock_irq;
extern rdpa_bpm_buffer_size_t                           g_bpm_buffer_size;
/**** Software Debug Routines ****/

#define RDD_BBH_GPON_RX_DESCRIPTORS_NORMAL_QUEUE_SIZE          32
#define RDD_BBH_GPON_RX_DESCRIPTORS_DIRECT_QUEUE_SIZE          32
#define RDD_BBH_ETH_RX_DESCRIPTORS_NORMAL_QUEUE_SIZE           32
#define RDD_BBH_ETH_RX_DESCRIPTORS_DIRECT_QUEUE_SIZE           32
#define RDD_INGRESS_QUEUE_SIZE                                 64
#define RDD_IH_NUMBER_OF_BUFFERS                               32
#define RDD_IH_BUFFER_SIZE                                     256

/* macros for ingress queues' parsing */
#define RDD_INGRESS_QUEUE_ENTRY_VALID_READ( r, p )              FIELD_MREAD_8( ( (uint8_t *)p + 0), 7, 1, r )
#define RDD_INGRESS_QUEUE_ENTRY_BRIDGE_OR_ROUTER_READ( r, p )   FIELD_MREAD_8( ( (uint8_t *)p + 0), 6, 1, r )
#define RDD_INGRESS_QUEUE_ENTRY_IH_BUFFER_NUM_READ( r, p )      FIELD_MREAD_8( ( (uint8_t *)p + 0), 0, 6, r )

#define SSM_ENTRY_INDEX( context_table , provider )                   ( ( context_table ) | ( ( ( provider ) + 1 ) << 13 ) )

#define D_NUM_OF_RING_DESCRIPTORS                               12

typedef struct
{
    uint32_t five_tup_valid   :1 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t ip_first_frag    :1 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t ip_frag          :1 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t ip_filter_match  :1 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t ip_flt_match_num :2 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t tcp_udp          :1 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t ipv6_ext_hdr_flt :1 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t tcp_flag         :1 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t wan              :1 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t vid_fil_hit      :1 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t exp              :1 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t da_filt_num      :3 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t da_filter        :1 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t l4_protocol      :4 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t error            :1 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t p_tag            :1 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t vlan_num         :2 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t broadcast        :1 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t multicast        :1 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t l3_protocol      :2 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t l2_protocol      :4 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PARSER_SUMMARY_DTS;


#define RDD_PARSER_SUMMARY_FIVE_TUP_VLD_READ( r, p )          FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 31, 1, r )
#define RDD_PARSER_SUMMARY_IP_FRST_FRAG_READ( r, p )          FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 30, 1, r )
#define RDD_PARSER_SUMMARY_IP_FRAG_READ( r, p )               FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 29, 1, r )
#define RDD_PARSER_SUMMARY_IP_FILTER_MATCH_READ( r, p )       FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 28, 1, r )
#define RDD_PARSER_SUMMARY_IP_FILTER_MATCH_NUM_READ( r, p )   FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 26, 2, r )
#define RDD_PARSER_SUMMARY_TCP_UDP_READ( r, p )               FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 25, 1, r )
#define RDD_PARSER_SUMMARY_IPV6_EXT_HDR_FLT_READ( r, p )      FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 24, 1, r )
#define RDD_PARSER_SUMMARY_TCP_FLAG_READ( r, p )              FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 23, 1, r )
#define RDD_PARSER_SUMMARY_WAN_READ( r, p )                   FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 22, 1, r )
#define RDD_PARSER_SUMMARY_VID_FIL_HIT_READ( r, p )           FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 21, 1, r )
#define RDD_PARSER_SUMMARY_EXP_READ( r, p )                   FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 20, 1, r )
#define RDD_PARSER_SUMMARY_DA_FILT_NUM_READ( r, p )           FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 17, 3, r )
#define RDD_PARSER_SUMMARY_DA_FILTER_READ( r, p )             FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 16, 1, r )
#define RDD_PARSER_SUMMARY_L4_PROTOCOL_READ( r, p )           FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 12, 4, r )
#define RDD_PARSER_SUMMARY_ERROR_READ( r, p )                 FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 11, 1, r )
#define RDD_PARSER_SUMMARY_PTAG_READ( r, p )                  FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 10, 1, r )
#define RDD_PARSER_SUMMARY_VLAN_NUM_READ( r, p )              FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 8,  2, r )
#define RDD_PARSER_SUMMARY_BROADCAST_READ( r, p )             FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 7,  1, r )
#define RDD_PARSER_SUMMARY_MULTICAST_READ( r, p )             FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 6,  1, r )
#define RDD_PARSER_SUMMARY_L3_PROTOCOL_READ( r, p )           FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 4,  2, r )
#define RDD_PARSER_SUMMARY_L2_PROTOCOL_READ( r, p )           FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 0,  4, r )

#define RDD_IH_LKP_RESULT_MATCH_READ( r, p )                  FIELD_MREAD_32( ( ( uint8_t * ) p + 4 ), 0, 1, r )
#define RDD_IH_LKP_RESULT_CAM_READ( r, p )                    FIELD_MREAD_32( ( ( uint8_t * ) p + 4 ), 1, 1, r )
#define RDD_IH_LKP_RESULT_MOVE_READ( r, p )                   FIELD_MREAD_32( ( ( uint8_t * ) p + 4 ), 2, 1, r )
#define RDD_IH_LKP_RESULT_MATCH_OFFSET_READ( r, p )           FIELD_MREAD_32( ( ( uint8_t * ) p + 4 ), 4, 12, r )
#define RDD_IH_LKP_RESULT_PORT_READ( r, p )                   FIELD_MREAD_32( ( ( uint8_t * ) p + 4 ), 16, 16, r )
#define RDD_IH_LKP_RESULT_CONTEXT_ENTRY_READ( r, p )          MREAD_32( ( ( uint8_t * ) p + 0 ), r )

#define RDD_IH_EGRESS_HEADER_RB_VALID_READ( r, p )            FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 13, 1, r )
#define RDD_IH_EGRESS_HEADER_FORWARD_EN_READ( r, p )          FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 12, 1, r )
#define RDD_IH_EGRESS_HEADER_DIRECT_MODE_READ( r, p )         FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 11, 1, r )
#define RDD_IH_EGRESS_HEADER_TARGET_MEMORY_READ( r, p )       FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 10, 1, r )
#define RDD_IH_EGRESS_HEADER_INGRESS_QOS_READ( r, p )         FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 8,  2, r )
#define RDD_IH_EGRESS_HEADER_1588_TOD_READ( r, p )            { uint32_t temp; FIELD_MREAD_32( ( ( uint8_t * ) p + 0 ), 0, 8, temp ); FIELD_MREAD_32( ( ( uint8_t * ) p + 4 ), 28, 4, r ); r |= ( temp << 4 ); }
#define RDD_IH_EGRESS_HEADER_WAN_FLOW_ID_READ( r, p )         FIELD_MREAD_32( ( ( uint8_t * ) p + 4 ), 20,  8, r )
#define RDD_IH_EGRESS_HEADER_PTI_READ( r, p )                 FIELD_MREAD_32( ( ( uint8_t * ) p + 4 ), 18, 2, r )
#define RDD_IH_EGRESS_HEADER_CLASS_OVERRIDE_EN_READ( r, p )   FIELD_MREAD_32( ( ( uint8_t * ) p + 4 ), 17, 1, r )
#define RDD_IH_EGRESS_HEADER_CLASS_READ( r, p )               FIELD_MREAD_32( ( ( uint8_t * ) p + 4 ), 13, 4, r )
#define RDD_IH_EGRESS_HEADER_HLEN_READ( r, p )                FIELD_MREAD_32( ( ( uint8_t * ) p + 4 ), 5, 8, r )
#define RDD_IH_EGRESS_HEADER_SRC_PORT_READ( r, p )            FIELD_MREAD_32( ( ( uint8_t * ) p + 4 ), 0, 5, r )



typedef struct
{
    uint32_t  reserved1           :17 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t  allocated_ribs_num  :7  __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t  reserved2           :3  __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t  cong_state          :5  __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t  reserved3           :32 __PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_RUNNER_CONGESTION_STATE_DTS;


#define RDD_RUNNER_CONGESTION_STATE_STATE_READ( r, p )                   FIELD_MREAD_16( ( ( uint8_t * ) p + 0 ), 0,  5, r )
#define RDD_RUNNER_CONGESTION_STATE_ALLOCATED_RIBS_NUMBER_READ( r, p )   FIELD_MREAD_16( ( ( uint8_t * ) p + 0 ), 8,  7, r )



static int p_lilac_rdd_print_gpon_rx_descriptors_normal ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_gpon_rx_descriptors_direct ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_eth_rx_descriptors_normal ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_eth_rx_descriptors_direct ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static void p_lilac_rdd_print_rx_descriptors_helper ( bdmf_session_handle, RDD_BBH_RX_DESCRIPTOR_DTS *, uint32_t );
static int p_lilac_rdd_print_ddr_buffer ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_emac_queues_parameters ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_tcont_descriptor ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_active_rc ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_free_pd_pool_info ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_cpu_rx_queues_status ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_ih_buffer ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_congestion_state ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_global_registers ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_start_profiling ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_stop_profiling ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_profiling_registers ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
#if !defined(FIRMWARE_INIT)
static int p_lilac_rdd_runner_enable ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_breakpoint_config ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_set_breakpoint ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_breakpoint_status ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
#endif
static int p_lilac_rdd_check_lists ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static void p_lilac_rdd_check_lists_ds_free_pool ( bdmf_session_handle, uint8_t );
static void p_lilac_rdd_check_lists_ds_queues ( bdmf_session_handle, uint8_t );
static void p_lilac_rdd_check_lists_us_free_pool ( bdmf_session_handle, uint8_t );
static void p_lilac_rdd_check_lists_us_queues ( bdmf_session_handle, uint8_t );
static int p_lilac_rdd_print_tables_list(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int p_lilac_rdd_print_table_entries(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
#if 0
static void p_lilac_rdd_check_lists_cpu_free_pool ( bdmf_session_handle, uint8_t );
#endif
static uint32_t p_lilac_rdd_check_lists_get_list_size ( bdmf_session_handle  session,
														  uint16_t           xi_head_address,
												          uint16_t           xi_tail_address,
														  uint32_t           xi_max_list_size,
														  uint32_t           xi_memory_segment_offset,
														  uint8_t            xi_print_descriptors );

static int p_lilac_rdd_print_ds_wan_flow ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_us_wan_flow ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_global_dscp_to_pbits ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_dscp_to_pbits ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_mac_table ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_iptv_table ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_upstream_pbits_to_qos ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_downstream_pbits_to_qos ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_us_connections_table ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_ds_connections_table ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_ds_connection ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_us_connection ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_context ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int f_lilac_rdd_print_ds_connection_helper ( bdmf_session_handle , RDD_CONNECTION_TABLE_DTS *, int, int );
static int f_lilac_rdd_print_us_connection_helper ( bdmf_session_handle , RDD_CONNECTION_TABLE_DTS *, int, int );
static int f_lilac_rdd_print_fc_context_helper ( bdmf_session_handle, int );
static int p_lilac_rdd_print_ds_context_entry_number ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_us_context_entry_number ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_context_entry_number_helper ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], int us_flag );
static int p_lilac_rdd_print_connections_number ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );


static int p_lilac_rdd_1588_mode_config ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_1588_time_stamp ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_upstream_padding_config ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_interrupt_vector ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );

static int p_lilac_rdd_tcont_byte_counter_read ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_iptv_mac_counter_read ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_iptv_layer3_counter_read ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_flow_pm_counters_get ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_bridge_port_pm_counters_get ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_service_queue_counters_get ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_various_counters_get ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_skb_debug_counters_get ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_parallel_processing_debug_counters_get ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_parallel_processing_context_cache_mode_set ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );

static int p_lilac_rdd_print_ingress_classification_rule_cfgs ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_ingress_classification_context ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_general_information ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
#ifdef G9991
static int p_lilac_rdd_g9991_debug_counters_get ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_g9991_flow_counters_get ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
#endif
static int p_lilac_rdd_print_pbits_to_wan_flow_table ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_print_cpu_rx_interrupt_coalescing_information ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_ack_prioritization_enable ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_ack_packet_size_threshold_config ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms );
static int p_lilac_rdd_ack_packets_queue_index_config (bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
static int32_t rdd_trace_enable(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms);
uint32_t g_rdd_trace = 0;


#define MAKE_BDMF_SHELL_CMD_NOPARM(dir, cmd, help, cb) \
    bdmfmon_cmd_add(dir, cmd, cb, help, BDMF_ACCESS_ADMIN, NULL, NULL)

#define MAKE_BDMF_SHELL_CMD(dir, cmd, help, cb, parms...)   \
{                                                           \
    static bdmfmon_cmd_parm_t cmd_parms[]={                 \
        parms,                                              \
        BDMFMON_PARM_LIST_TERMINATOR                        \
    };                                                      \
    bdmfmon_cmd_add(dir, cmd, cb, help, BDMF_ACCESS_ADMIN, NULL, cmd_parms); \
}

BL_LILAC_RDD_ERROR_DTE f_rdd_make_shell_commands ( void )
{
    bdmfmon_handle_t driver_dir, rdd_dir;

    if ( !( driver_dir = bdmfmon_dir_find ( NULL, "driver" ) ) )
    {
        driver_dir = bdmfmon_dir_add ( NULL, "driver", "Device Drivers", BDMF_ACCESS_ADMIN, NULL );

        if ( !driver_dir )
            return ( BL_LILAC_RDD_ERROR_MALLOC_FAILED );
    }

    rdd_dir = bdmfmon_dir_add ( driver_dir, "rdd", "Runner Device Driver", BDMF_ACCESS_ADMIN, NULL );

    if ( !rdd_dir )
        return ( BL_LILAC_RDD_ERROR_MALLOC_FAILED );

    MAKE_BDMF_SHELL_CMD( rdd_dir, "pgr",     "print global registers", p_lilac_rdd_print_global_registers,
                         BDMFMON_MAKE_PARM_RANGE( "runner", "Runner id: Pico_A=0, Pico_B=1", BDMFMON_PARM_NUMBER, 0, 0, 1 ) );	
    MAKE_BDMF_SHELL_CMD_NOPARM( rdd_dir, "pgrdn", "print gpon rx descriptors - normal", p_lilac_rdd_print_gpon_rx_descriptors_normal );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "pgrdd", "print gpon rx descriptors - direct", p_lilac_rdd_print_gpon_rx_descriptors_direct,
                         BDMFMON_MAKE_PARM_RANGE( "runner", "Runner id", BDMFMON_PARM_NUMBER, 0, 0, 1 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "perdn", "print eth rx descriptors - normal", p_lilac_rdd_print_eth_rx_descriptors_normal,
                         BDMFMON_MAKE_PARM_RANGE( "emac", "emac number", BDMFMON_PARM_NUMBER, 0, 0, 4 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "perdd", "print eth rx descriptors - direct", p_lilac_rdd_print_eth_rx_descriptors_direct,
                         BDMFMON_MAKE_PARM_RANGE( "emac", "emac number", BDMFMON_PARM_NUMBER, 0, 0, 4 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "pdb",    "print ddr buffer packet data", p_lilac_rdd_print_ddr_buffer,
                         BDMFMON_MAKE_PARM( "buffer", "Buffer number (hex)", BDMFMON_PARM_HEX, 0 ),
                         BDMFMON_MAKE_PARM( "length", "Buffer length (hex)", BDMFMON_PARM_HEX, 0 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "peqp",   "print emac queues parameters", p_lilac_rdd_print_emac_queues_parameters,
                         BDMFMON_MAKE_PARM_RANGE( "emac", "emac number", BDMFMON_PARM_NUMBER, 0, 0, 4 ),
                         BDMFMON_MAKE_PARM_RANGE( "queue", "queue number", BDMFMON_PARM_NUMBER, 0, 0, 7 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "ptq",     "print tcont queue", p_lilac_rdd_print_tcont_descriptor,
                         BDMFMON_MAKE_PARM_RANGE( "tcont", "tcont number", BDMFMON_PARM_NUMBER, 0, 0, 39 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "parc",    "print active rate controllers", p_lilac_rdd_print_active_rc,
                         BDMFMON_MAKE_PARM_RANGE( "tcont", "tcont number", BDMFMON_PARM_NUMBER, 0, 0, 39 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "pfpdp",   "print free packet descriptor pool info", p_lilac_rdd_print_free_pd_pool_info,
                         BDMFMON_MAKE_PARM_RANGE( "pool", "Buffer pool: ds=0, us=1", BDMFMON_PARM_NUMBER, 0, 0, 2 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "pc",       "prints context entry by context index", p_lilac_rdd_print_context,
                         BDMFMON_MAKE_PARM("index", "Context entry index", BDMFMON_PARM_NUMBER, 0 ),
                         BDMFMON_MAKE_PARM_RANGE( "us_ds", "ds=0, us=1", BDMFMON_PARM_NUMBER, 0, 0, 1 ) );
    MAKE_BDMF_SHELL_CMD_NOPARM( rdd_dir, "pcrqs",   "print cpu rx queues status", p_lilac_rdd_print_cpu_rx_queues_status );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "pihb",    "print ih buffer", p_lilac_rdd_print_ih_buffer,
                         BDMFMON_MAKE_PARM_RANGE( "us_ds", "ds=0, us=1", BDMFMON_PARM_NUMBER, 0, 0, 1 ),
                         BDMFMON_MAKE_PARM_RANGE( "buffer", "buffer number", BDMFMON_PARM_NUMBER, 0, 0, 39 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "pcs",     "print congestion state", p_lilac_rdd_print_congestion_state,
                         BDMFMON_MAKE_PARM_RANGE( "runner", "Runner id: A=0, B=1", BDMFMON_PARM_NUMBER, 0, 0, 1 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "sp",      "start profiling", p_lilac_rdd_start_profiling,
                         BDMFMON_MAKE_PARM_RANGE( "runner", "Runner id: A=0, B=1", BDMFMON_PARM_NUMBER, 0, 0, 1 ),
                         BDMFMON_MAKE_PARM_RANGE( "main_pico", "Main=0, Pico=1", BDMFMON_PARM_NUMBER, 0, 0, 1 ),
                         BDMFMON_MAKE_PARM_RANGE( "task1", "Task #1 (up to 15 in Pico)", BDMFMON_PARM_NUMBER, 0, 0, 31 ),
                         BDMFMON_MAKE_PARM_RANGE( "task2", "Task #2 (up to 15 in Pico)", BDMFMON_PARM_NUMBER, 0, 0, 31 ),
                         BDMFMON_MAKE_PARM_RANGE( "trace_pc", "Trace PC enable", BDMFMON_PARM_NUMBER, 0, 0, 31 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "sp1",     "stop profiling", p_lilac_rdd_stop_profiling,
                         BDMFMON_MAKE_PARM_RANGE( "main_pico", "Main=0, Pico=1", BDMFMON_PARM_NUMBER, 0, 0, 1 ) );
    MAKE_BDMF_SHELL_CMD_NOPARM( rdd_dir, "ppr",     "print profiling registers", p_lilac_rdd_print_profiling_registers );
#if !defined(FIRMWARE_INIT)
    MAKE_BDMF_SHELL_CMD( rdd_dir, "re",      "enable or disable runner",  p_lilac_rdd_runner_enable,
                         BDMFMON_MAKE_PARM_RANGE( "runner", "Runner id: A=0, B=1", BDMFMON_PARM_NUMBER, 0, 0, 1 ),
                         BDMFMON_MAKE_PARM_RANGE( "main_pico", "Main=0, Pico=1", BDMFMON_PARM_NUMBER, 0, 0, 1 ),
                         BDMFMON_MAKE_PARM_RANGE( "enable", "enable=1, disable=0", BDMFMON_PARM_NUMBER, 0, 0, 1 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "bc",      "configure breakpoint", p_lilac_rdd_breakpoint_config,
                         BDMFMON_MAKE_PARM_RANGE( "runner", "Runner id: A=0, B=1", BDMFMON_PARM_NUMBER, 0, 0, 1 ),
                         BDMFMON_MAKE_PARM_RANGE( "main_pico", "Main=0, Pico=1", BDMFMON_PARM_NUMBER, 0, 0, 1 ),
                         BDMFMON_MAKE_PARM_RANGE( "step_mode", "enable=1, disable=0", BDMFMON_PARM_NUMBER, 0, 0, 1 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "bs",      "set breakpoint", p_lilac_rdd_set_breakpoint,
                         BDMFMON_MAKE_PARM_RANGE( "runner", "Runner id: A=0, B=1", BDMFMON_PARM_NUMBER, 0, 0, 1 ),
                         BDMFMON_MAKE_PARM_RANGE( "main_pico", "Main=0, Pico=1", BDMFMON_PARM_NUMBER, 0, 0, 1 ),
                         BDMFMON_MAKE_PARM_RANGE( "index", "Breakpoint index", BDMFMON_PARM_NUMBER, 0, 0, 3 ),
                         BDMFMON_MAKE_PARM_RANGE( "enable", "enable=1, disable=0", BDMFMON_PARM_NUMBER, 0, 0, 1 ),
                         BDMFMON_MAKE_PARM_RANGE( "address", "Address (16 bit)", BDMFMON_PARM_HEX, 0, 0, 0xffff ),
                         BDMFMON_MAKE_PARM_RANGE( "use_thread", "enable=1, disable=0", BDMFMON_PARM_NUMBER, 0, 0, 1 ),
                         BDMFMON_MAKE_PARM_RANGE( "thread", "Thread number", BDMFMON_PARM_NUMBER, 0, 0, 31 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "pbs",     "print breakpoint status", p_lilac_rdd_print_breakpoint_status,
                         BDMFMON_MAKE_PARM_RANGE( "runner", "Runner id: A=0, B=1", BDMFMON_PARM_NUMBER, 0, 0, 1 ),
                         BDMFMON_MAKE_PARM_RANGE( "main_pico", "Main=0, Pico=1", BDMFMON_PARM_NUMBER, 0, 0, 1 ) );
#endif
    MAKE_BDMF_SHELL_CMD( rdd_dir, "cl",      "check lists", p_lilac_rdd_check_lists,
                         BDMFMON_MAKE_PARM( "ds_free", "Number of PDs to print in DS free PD pool", BDMFMON_PARM_NUMBER, 0 ),
                         BDMFMON_MAKE_PARM( "eth_pci_tx", "Number of PDs to print in ETH/PCI TX queues", BDMFMON_PARM_NUMBER, 0 ),
                         BDMFMON_MAKE_PARM( "us_free", "Number of PDs to print in US free PD pool", BDMFMON_PARM_NUMBER, 0 ),
                         BDMFMON_MAKE_PARM( "gpon_tx", "Number of PDs to print in GPON TX queues", BDMFMON_PARM_NUMBER, 0 ),
                         BDMFMON_MAKE_PARM( "cpu_free", "Number of PDs to print in CPU free PD pool", BDMFMON_PARM_NUMBER, 0 ),
                         BDMFMON_MAKE_PARM( "cpu_rx", "Number of PDs to print in CPU RX queues", BDMFMON_PARM_NUMBER, 0 ) );
    MAKE_BDMF_SHELL_CMD_NOPARM( rdd_dir, "pdswf",   "print downstream wan flow", p_lilac_rdd_print_ds_wan_flow );
    MAKE_BDMF_SHELL_CMD_NOPARM( rdd_dir, "puswf", "print upstream wan flow", p_lilac_rdd_print_us_wan_flow );
    MAKE_BDMF_SHELL_CMD_NOPARM( rdd_dir, "pgdtp", "print global dscp to pbits", p_lilac_rdd_print_global_dscp_to_pbits );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "pdtpp", "print dscp to pbits for port number", p_lilac_rdd_print_dscp_to_pbits,
                         BDMFMON_MAKE_PARM_RANGE( "bridge_port", "0:WAN,1-5:ETH,8:PCI", BDMFMON_PARM_NUMBER, 0, 0, 8 ) );
    MAKE_BDMF_SHELL_CMD_NOPARM( rdd_dir, "pmt",  "print mac table", p_lilac_rdd_print_mac_table );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "pit",      "print iptv table", p_lilac_rdd_print_iptv_table,
                         BDMFMON_MAKE_PARM_RANGE( "l2_l3", "L2=0, L3=1", BDMFMON_PARM_NUMBER, 0, 0, 1 ) );
    MAKE_BDMF_SHELL_CMD_NOPARM( rdd_dir, "pusptq", "print upstream pbits to qos", p_lilac_rdd_print_upstream_pbits_to_qos );
    MAKE_BDMF_SHELL_CMD_NOPARM( rdd_dir, "pdsptq", "print downstream pbits to qos", p_lilac_rdd_print_downstream_pbits_to_qos );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "pdsc",     "print downstream connection by index", p_lilac_rdd_print_ds_connection,
                         BDMFMON_MAKE_PARM("index", "Connection index", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD( rdd_dir, "pdsct",    "print downstream connections table", p_lilac_rdd_print_ds_connections_table,
                         BDMFMON_MAKE_PARM( "start", "Start index", BDMFMON_PARM_NUMBER, 0 ),
                         BDMFMON_MAKE_PARM( "number", "Number of connections", BDMFMON_PARM_NUMBER, 0 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "pusc",     "print upstream connection by index", p_lilac_rdd_print_us_connection,
                         BDMFMON_MAKE_PARM("index", "Connection index", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD( rdd_dir, "pusct",    "print upstream connections table", p_lilac_rdd_print_us_connections_table,
                         BDMFMON_MAKE_PARM( "start", "Start index", BDMFMON_PARM_NUMBER, 0 ),
                         BDMFMON_MAKE_PARM( "number", "Number of connections", BDMFMON_PARM_NUMBER, 0 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "pdscn", "print downstream context number by 5-tuple", p_lilac_rdd_print_ds_context_entry_number,
                         BDMFMON_MAKE_PARM_RANGE( "protocol", "Protocol (Hex)", BDMFMON_PARM_HEX, 0, 0, 0xff ),
                         BDMFMON_MAKE_PARM_RANGE( "src_port", "Source port (hex)", BDMFMON_PARM_HEX, 0, 0, 0xffff ),
                         BDMFMON_MAKE_PARM_RANGE( "dst_port", "Dst port (hex)", BDMFMON_PARM_HEX, 0, 0, 0xffff ),
                         BDMFMON_MAKE_PARM( "src_ip", "Source IP (hex)", BDMFMON_PARM_HEX, 0 ),
                         BDMFMON_MAKE_PARM( "dst_ip", "Destination IP (hex)", BDMFMON_PARM_HEX, 0 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "puscn",   "print upstream context number by 5-tuple", p_lilac_rdd_print_us_context_entry_number,
                         BDMFMON_MAKE_PARM_RANGE( "protocol", "Protocol (Hex)", BDMFMON_PARM_HEX, 0, 0, 0xff ),
                         BDMFMON_MAKE_PARM_RANGE( "src_port", "Source port (hex)", BDMFMON_PARM_HEX, 0, 0, 0xffff ),
                         BDMFMON_MAKE_PARM_RANGE( "dst_port", "Dst port (hex)", BDMFMON_PARM_HEX, 0, 0, 0xffff ),
                         BDMFMON_MAKE_PARM( "src_ip", "Source IP (hex)", BDMFMON_PARM_HEX, 0 ),
                         BDMFMON_MAKE_PARM( "dst_ip", "Destination IP (hex)", BDMFMON_PARM_HEX, 0 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "pcn", "print number of US and DS connections", p_lilac_rdd_print_connections_number,
                         BDMFMON_MAKE_PARM_RANGE( "us_ds", "ds=0, us=1", BDMFMON_PARM_NUMBER, 0, 0, 1 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "mc1588", "configure 1588 mode", p_lilac_rdd_1588_mode_config,
                         BDMFMON_MAKE_PARM_RANGE( "enable", "enable=1, disable=0", BDMFMON_PARM_NUMBER, 0, 0, 1 ) );
    MAKE_BDMF_SHELL_CMD_NOPARM(rdd_dir, "pts1588", "print 1588 timestamp", p_lilac_rdd_print_1588_time_stamp );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "uspc", "configure upstream padding", p_lilac_rdd_upstream_padding_config,
                         BDMFMON_MAKE_PARM_RANGE( "control", "enable=1, disable=0", BDMFMON_PARM_NUMBER, 0, 0, 1 ),
                         BDMFMON_MAKE_PARM_RANGE( "cpu_control", "enable=1, disable=0", BDMFMON_PARM_NUMBER, 0, 0, 1 ),
                         BDMFMON_MAKE_PARM( "max size", "max size", BDMFMON_PARM_NUMBER, 0 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "piv", "print interrupt vector & mask", p_lilac_rdd_print_interrupt_vector,
                         BDMFMON_MAKE_PARM_RANGE( "vector", "Interrupt vector", BDMFMON_PARM_NUMBER, 0, 0, 1 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "ptbc", "print tcont byte counter", p_lilac_rdd_tcont_byte_counter_read,
                         BDMFMON_MAKE_PARM_RANGE( "tcont", "tcont number", BDMFMON_PARM_NUMBER, 0, 0, 39 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "pil2c", "print iptv layer2 counter", p_lilac_rdd_iptv_mac_counter_read,
                         BDMFMON_MAKE_PARM( "mac", "MAC address xx:xx:xx:xx:xx:xx", BDMFMON_PARM_MAC, 0 ),
                         BDMFMON_MAKE_PARM_RANGE( "vid", "VID", BDMFMON_PARM_NUMBER, 0, 0, 0xfff ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "pil3c", "print iptv layer3 counter", p_lilac_rdd_iptv_layer3_counter_read,
                         BDMFMON_MAKE_PARM( "src_ip", "Source IP", BDMFMON_PARM_IP, 0 ),
                         BDMFMON_MAKE_PARM( "dst_ip", "Destination IP", BDMFMON_PARM_IP, 0 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "pfc", "print flow pm counters", p_lilac_rdd_flow_pm_counters_get,
                         BDMFMON_MAKE_PARM_RANGE( "flow", "Flow id", BDMFMON_PARM_NUMBER, 0, 0, 255 ),
                         BDMFMON_MAKE_PARM_RANGE( "pm_type", "1-RX,2-TX,3-both", BDMFMON_PARM_NUMBER, 0, 1, 3 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "pbpc", "print bridge port pm counters", p_lilac_rdd_bridge_port_pm_counters_get,
                         BDMFMON_MAKE_PARM_RANGE( "bridge_port", "0:WAN-B,1-5:LAN,6:WAN-R,7-WAN-IPTV,8-PCI,14-Quasy", BDMFMON_PARM_NUMBER, 0, 0, 14 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "psqc", "print service queue pm counters", p_lilac_rdd_service_queue_counters_get,
                         BDMFMON_MAKE_PARM_RANGE( "Serice queue", "0-7", BDMFMON_PARM_NUMBER, 0, 0, 7 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "pvdc", "print various drop counters", p_lilac_rdd_various_counters_get,
                         BDMFMON_MAKE_PARM_RANGE( "us_ds", "ds=0, us=1", BDMFMON_PARM_NUMBER, 0, 0, 1 ) );
    MAKE_BDMF_SHELL_CMD_NOPARM( rdd_dir, "psdc",  "print SKB debug counters", p_lilac_rdd_skb_debug_counters_get );
    MAKE_BDMF_SHELL_CMD_NOPARM( rdd_dir, "pppdc",  "print parallel processing debug counters", p_lilac_rdd_parallel_processing_debug_counters_get );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "sppcm", "set parallel processing cache mode", p_lilac_rdd_parallel_processing_context_cache_mode_set,
                         BDMFMON_MAKE_PARM_RANGE( "mode", "disable=0, enable=1", BDMFMON_PARM_NUMBER, 0, 0, 1 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "picc", "print ingress classification context", p_lilac_rdd_print_ingress_classification_context, 
                         BDMFMON_MAKE_PARM_RANGE("index", "Context entry index", BDMFMON_PARM_NUMBER, 0, 0, 255 ), 
                         BDMFMON_MAKE_PARM_RANGE( "us_ds", "ds=0, us=1", BDMFMON_PARM_NUMBER, 0, 0, 1 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "picrc", "print ingress classification rule cfgs", p_lilac_rdd_print_ingress_classification_rule_cfgs,
                         BDMFMON_MAKE_PARM_RANGE( "us_ds", "ds=0, us=1", BDMFMON_PARM_NUMBER, 0, 0, 1 ) );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "pptwft", "print pbits to wan flow table", p_lilac_rdd_print_pbits_to_wan_flow_table,
                         BDMFMON_MAKE_PARM_RANGE( "table", "0-7", BDMFMON_PARM_NUMBER, 0, 0, 7 ) );
    MAKE_BDMF_SHELL_CMD_NOPARM( rdd_dir, "pgi", "print general information", p_lilac_rdd_print_general_information );
    MAKE_BDMF_SHELL_CMD(rdd_dir, "ptl",   "print tables list", p_lilac_rdd_print_tables_list,
        BDMFMON_MAKE_PARM("name", "Table name, may have wildcards", BDMFMON_PARM_STRING, BDMFMON_PARM_FLAG_OPTIONAL));
    MAKE_BDMF_SHELL_CMD(rdd_dir, "pte",   "print table entries", p_lilac_rdd_print_table_entries,
        BDMFMON_MAKE_PARM("name", "Full Table name", BDMFMON_PARM_STRING, 0),
        BDMFMON_MAKE_PARM("index/address", "0 for entry index to start, 1 for entry address to start",
            BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("start entry index", "Start entry index", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("start entry address", "Start entry address", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM("number", "Number of entries", BDMFMON_PARM_NUMBER, 0));
#ifdef G9991
    MAKE_BDMF_SHELL_CMD_NOPARM( rdd_dir, "pgdc",  "print G9991 debug counters", p_lilac_rdd_g9991_debug_counters_get );
    MAKE_BDMF_SHELL_CMD( rdd_dir, "pgfc", "print g9991 flow counters", p_lilac_rdd_g9991_flow_counters_get,
						 BDMFMON_MAKE_PARM_RANGE( "flow", "Flow index", BDMFMON_PARM_NUMBER, 0, 0, 23 ) );
#endif
    MAKE_BDMF_SHELL_CMD( rdd_dir, "pcrici",   "print cpu rx interrupt coalescing information", p_lilac_rdd_print_cpu_rx_interrupt_coalescing_information,
                        BDMFMON_MAKE_PARM_RANGE( "ring_id", "ring id", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL, 0, D_NUM_OF_RING_DESCRIPTORS) );

    MAKE_BDMF_SHELL_CMD(rdd_dir, "acken", "enable or disable ack prioritization",  p_lilac_rdd_ack_prioritization_enable,
                         BDMFMON_MAKE_PARM_RANGE( "enable", "ack prioritization: disable=0, enable=1", BDMFMON_PARM_NUMBER, 0, 0, 1 ));

    MAKE_BDMF_SHELL_CMD(rdd_dir, "ackth", "ack packet size threshold",  p_lilac_rdd_ack_packet_size_threshold_config,
                         BDMFMON_MAKE_PARM_RANGE( "threshold", "ack packet size threshold: ", BDMFMON_PARM_NUMBER, 0, 64, 255 ));
    
    MAKE_BDMF_SHELL_CMD(rdd_dir, "ackqi", "config queue index for upstream ack packets",  p_lilac_rdd_ack_packets_queue_index_config,
                        BDMFMON_MAKE_PARM_RANGE("channel id", "channel id", BDMFMON_PARM_NUMBER, 0, 0, 39),
                        BDMFMON_MAKE_PARM_RANGE("queue index", "queue index", BDMFMON_PARM_HEX, 0, 0, 7),
                        BDMFMON_MAKE_PARM_RANGE("rate controller", "rate controller", BDMFMON_PARM_NUMBER, 0, 0, 31));

#if defined(CONFIG_BCM_PKTRUNNER_GSO)
    MAKE_BDMF_SHELL_CMD_NOPARM(rdd_dir, "pgsoc",  "print GSO counters", p_lilac_rdd_gso_counters_get);
    MAKE_BDMF_SHELL_CMD_NOPARM(rdd_dir, "pgsod",  "print GSO debug info", p_lilac_rdd_gso_debug_info_get);
#endif

#ifdef CONFIG_DHD_RUNNER
    rdd_dhd_helper_shell_cmds_init( rdd_dir );
#endif

    MAKE_BDMF_SHELL_CMD(rdd_dir, "rtrc",   "Enable/Disable RDD Trace", rdd_trace_enable,
        BDMFMON_MAKE_PARM("enable", "0 - disable, 1 - enable", BDMFMON_PARM_NUMBER, 0));

    return ( BL_LILAC_RDD_OK );
}

static int p_lilac_rdd_print_gpon_rx_descriptors_normal ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_BBH_RX_DESCRIPTOR_DTS  *gpon_rx_descriptor_ptr;

    gpon_rx_descriptor_ptr = ( RDD_BBH_RX_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + GPON_RX_NORMAL_DESCRIPTORS_ADDRESS );

    bdmf_session_print ( session, "GPON RX Descriptors - Normal:\n" );
    bdmf_session_print ( session, "------------------------------\n" );

    p_lilac_rdd_print_rx_descriptors_helper ( session, gpon_rx_descriptor_ptr, RDD_BBH_GPON_RX_DESCRIPTORS_NORMAL_QUEUE_SIZE );

    return ( 0 );
}


static int p_lilac_rdd_print_gpon_rx_descriptors_direct ( bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_BBH_RX_DESCRIPTOR_DTS  *gpon_rx_descriptor_ptr;
    uint8_t                    runner;

    runner = ( uint8_t )parm[ 0 ].value.unumber;

    if ( runner == 0 )
    {
        gpon_rx_descriptor_ptr = ( RDD_BBH_RX_DESCRIPTOR_DTS * )(DEVICE_ADDRESS ( RUNNER_PRIVATE_0_OFFSET ) + GPON_RX_DIRECT_DESCRIPTORS_ADDRESS );
    }
    else
    {
        gpon_rx_descriptor_ptr = ( RDD_BBH_RX_DESCRIPTOR_DTS * )(DEVICE_ADDRESS ( RUNNER_PRIVATE_1_OFFSET ) + GPON_RX_DIRECT_DESCRIPTORS_ADDRESS );
    }

    bdmf_session_print ( session, "GPON direct RX Descriptors:\n" );
    bdmf_session_print ( session, "------------------------------\n" );

    p_lilac_rdd_print_rx_descriptors_helper ( session, gpon_rx_descriptor_ptr, RDD_BBH_GPON_RX_DESCRIPTORS_DIRECT_QUEUE_SIZE );

    return ( 0 );
}


static int p_lilac_rdd_print_eth_rx_descriptors_normal ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_BBH_RX_DESCRIPTOR_DTS  *eth_rx_descriptor_ptr;
    uint32_t                   eth_rx_queue;

    eth_rx_queue = ( int )parm[ 0 ].value.unumber;

    switch ( eth_rx_queue )
    {
    case 0 :
        eth_rx_descriptor_ptr = ( RDD_BBH_RX_DESCRIPTOR_DTS * )(DEVICE_ADDRESS ( RUNNER_PRIVATE_1_OFFSET ) + ETH0_RX_DESCRIPTORS_ADDRESS );
        break;

    case 1 :
        eth_rx_descriptor_ptr = ( RDD_BBH_RX_DESCRIPTOR_DTS * )(DEVICE_ADDRESS ( RUNNER_PRIVATE_1_OFFSET ) + ETH1_RX_DESCRIPTORS_ADDRESS );
        break;

    case 2:
        eth_rx_descriptor_ptr = ( RDD_BBH_RX_DESCRIPTOR_DTS * )(DEVICE_ADDRESS ( RUNNER_PRIVATE_1_OFFSET ) + ETH2_RX_DESCRIPTORS_ADDRESS );
        break;

    case 3:
        eth_rx_descriptor_ptr = ( RDD_BBH_RX_DESCRIPTOR_DTS * )(DEVICE_ADDRESS ( RUNNER_PRIVATE_1_OFFSET ) + ETH3_RX_DESCRIPTORS_ADDRESS );
        break;

    case 4:
        eth_rx_descriptor_ptr = ( RDD_BBH_RX_DESCRIPTOR_DTS * )(DEVICE_ADDRESS ( RUNNER_PRIVATE_1_OFFSET ) + ETH4_RX_DESCRIPTORS_ADDRESS );
        break;

    default:
        bdmf_session_print ( session, "UT: Not enough parameters\n\n\r" );
        return ( BDMF_ERR_PARM );
    }

    bdmf_session_print ( session, "EMAC%u RX Descriptors (normal):\n", ( unsigned int )eth_rx_queue );
    bdmf_session_print ( session, "-------------------------------\n" );

    p_lilac_rdd_print_rx_descriptors_helper ( session, eth_rx_descriptor_ptr, RDD_BBH_ETH_RX_DESCRIPTORS_NORMAL_QUEUE_SIZE );

    return ( 0 );
}


static int p_lilac_rdd_print_eth_rx_descriptors_direct ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
#ifdef UNDEF // Tal Meged

    RDD_BBH_RX_DESCRIPTOR_DTS  *eth_rx_descriptor_ptr;
    uint32_t                   eth_rx_queue;

    eth_rx_queue = ( int )parm[ 0 ].value.unumber;

    switch ( eth_rx_queue )
    {
    case 0 :
        eth_rx_descriptor_ptr = ( RDD_BBH_RX_DESCRIPTOR_DTS * )(DEVICE_ADDRESS ( RUNNER_PRIVATE_0_OFFSET ) + ETH0_RX_DIRECT_DESCRIPTORS_ADDRESS );
        break;

    case 1 :
        eth_rx_descriptor_ptr = ( RDD_BBH_RX_DESCRIPTOR_DTS * )(DEVICE_ADDRESS ( RUNNER_PRIVATE_0_OFFSET ) + ETH1_RX_DIRECT_DESCRIPTORS_ADDRESS );
        break;

    case 2:
        eth_rx_descriptor_ptr = ( RDD_BBH_RX_DESCRIPTOR_DTS * )(DEVICE_ADDRESS ( RUNNER_PRIVATE_0_OFFSET ) + ETH2_RX_DIRECT_DESCRIPTORS_ADDRESS );
        break;

    case 3:
        eth_rx_descriptor_ptr = ( RDD_BBH_RX_DESCRIPTOR_DTS * )(DEVICE_ADDRESS ( RUNNER_PRIVATE_0_OFFSET ) + ETH3_RX_DIRECT_DESCRIPTORS_ADDRESS );
        break;

    case 4:
        eth_rx_descriptor_ptr = ( RDD_BBH_RX_DESCRIPTOR_DTS * )(DEVICE_ADDRESS ( RUNNER_PRIVATE_0_OFFSET ) + ETH4_RX_DIRECT_DESCRIPTORS_ADDRESS );
        break;

    default:
        bdmf_session_print ( session, "UT: Not enough parameters\n\n\r" );
        p_lilac_rdd_print_eth_rx_descriptors_direct_help ( session );
        return;
    }

    bdmf_session_print ( session, "EMAC%u RX Descriptors (direct):\n", ( unsigned int )eth_rx_queue );
    bdmf_session_print ( session, "-------------------------------\n" );

    p_lilac_rdd_print_rx_descriptors_helper ( session, eth_rx_descriptor_ptr, RDD_BBH_ETH_RX_DESCRIPTORS_DIRECT_QUEUE_SIZE );

#endif
    return ( 0 );
}


static void p_lilac_rdd_print_rx_descriptors_helper ( bdmf_session_handle session, RDD_BBH_RX_DESCRIPTOR_DTS  *rx_descriptor_ptr, uint32_t size )
{
    uint16_t  last_sbn;
    uint8_t   flow_id;
    uint16_t  packet_length;
    uint8_t   error;
    uint8_t   ploam;
    uint16_t  error_type;
    uint8_t   ih_buffer_num;
    uint8_t   target_memory;
    uint16_t  buffer_num;
    uint32_t  i;

    bdmf_session_print ( session, "last SBN | Flow ID | Packet length | Error | Ploam | Error type | IH BN | Target Memory | Buffer Number\n" );

    for ( i = 0; i < size; i++ )
    {
        RDD_BBH_RX_DESCRIPTOR_LAST_SBN_READ ( last_sbn, rx_descriptor_ptr );
        RDD_BBH_RX_DESCRIPTOR_FLOW_ID_READ ( flow_id, rx_descriptor_ptr );
        RDD_BBH_RX_DESCRIPTOR_PACKET_LENGTH_READ ( packet_length, rx_descriptor_ptr );
        RDD_BBH_RX_DESCRIPTOR_ERROR_READ ( error, rx_descriptor_ptr );
        RDD_BBH_RX_DESCRIPTOR_PLOAM_READ ( ploam, rx_descriptor_ptr );
        RDD_BBH_RX_DESCRIPTOR_ERROR_TYPE_READ ( error_type, rx_descriptor_ptr );
        RDD_BBH_RX_DESCRIPTOR_IH_BUFFER_NUMBER_READ ( ih_buffer_num, rx_descriptor_ptr );
        RDD_BBH_RX_DESCRIPTOR_TARGET_MEMORY_READ ( target_memory, rx_descriptor_ptr );
        RDD_BBH_RX_DESCRIPTOR_BUFFER_NUMBER_READ ( buffer_num, rx_descriptor_ptr );

        bdmf_session_print ( session, " 0x%-5x | 0x%-5x |    %-7u    |   %1u   |   %1u   |   0x%-4x   |  %-3u  |       %1u       |    0x%-5x   \n",
                             ( unsigned int )last_sbn, ( unsigned char )flow_id, ( unsigned int )packet_length, ( unsigned char )error, ( unsigned char )ploam,
                             ( unsigned int )error_type, ( unsigned char )ih_buffer_num, ( unsigned char )target_memory, ( unsigned int )buffer_num );

        rx_descriptor_ptr++;
    }

    return;
}


static int p_lilac_rdd_print_ddr_buffer ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    uint8_t   *packet_buffer_ptr;
    uint8_t   value;
    uint32_t  i;

    packet_buffer_ptr = ( uint8_t * )( g_runner_ddr_base_addr + parm[ 0 ].value.unumber * g_bpm_buffer_size );

    for ( i = 0; i < parm[ 1 ].value.unumber; i++, packet_buffer_ptr++ )
    {
        if ( ( i % 16 ) == 0 )
        {
            bdmf_session_print ( session, "%p  ", packet_buffer_ptr );
        }

        MREAD_8( packet_buffer_ptr, value );
        bdmf_session_print ( session, "%02x ", value );

        if ( ( ( i + 1 ) % 16 ) == 0 )
        {
            bdmf_session_print ( session, "\n" );
        }
    }

    bdmf_session_print ( session, "\n" );

    return ( 0 );
}


static int p_lilac_rdd_print_emac_queues_parameters ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_ETH_TX_QUEUE_DESCRIPTOR_DTS       *eth_tx_queue_descriptor_ptr;
    RDD_ETH_TX_QUEUES_POINTERS_TABLE_DTS  *eth_tx_queues_pointers_table_ptr;
    RDD_ETH_TX_QUEUE_POINTERS_ENTRY_DTS   *eth_tx_queue_pointers_entry_ptr;
    uint16_t                              eth_tx_queue_descriptor_offset;
    uint32_t                              emac_index;
    uint32_t                              queue_index;
    uint16_t                              head_ptr;
    uint16_t                              tail_ptr;
    uint16_t                              packet_counter;
    uint16_t                              threshold;
    uint16_t                              queue_mask;

    emac_index = ( int )parm[ 0 ].value.unumber;
    queue_index = ( int )parm[ 1 ].value.unumber;

    bdmf_session_print ( session, "Emac-%d : Queue-%d\n", ( int )emac_index, ( int )queue_index );
    bdmf_session_print ( session, "------------------\n" );

    /* read emac queue parameters */
    eth_tx_queues_pointers_table_ptr = ( RDD_ETH_TX_QUEUES_POINTERS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + ETH_TX_QUEUES_POINTERS_TABLE_ADDRESS );

    eth_tx_queue_pointers_entry_ptr =
        &( eth_tx_queues_pointers_table_ptr->entry[ emac_index * LILAC_RDD_EMAC_NUMBER_OF_QUEUES + queue_index ] );

    RDD_ETH_TX_QUEUE_POINTERS_ENTRY_TX_QUEUE_POINTER_READ ( eth_tx_queue_descriptor_offset, eth_tx_queue_pointers_entry_ptr );

    eth_tx_queue_descriptor_ptr = ( RDD_ETH_TX_QUEUE_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + eth_tx_queue_descriptor_offset );

    RDD_ETH_TX_QUEUE_DESCRIPTOR_HEAD_PTR_READ( head_ptr, eth_tx_queue_descriptor_ptr );
    RDD_ETH_TX_QUEUE_DESCRIPTOR_TAIL_PTR_READ( tail_ptr, eth_tx_queue_descriptor_ptr );
    RDD_ETH_TX_QUEUE_DESCRIPTOR_INGRESS_PACKET_COUNTER_READ( packet_counter, eth_tx_queue_descriptor_ptr );
    RDD_ETH_TX_QUEUE_DESCRIPTOR_PACKET_THRESHOLD_READ( threshold, eth_tx_queue_descriptor_ptr );
    RDD_ETH_TX_QUEUE_DESCRIPTOR_QUEUE_MASK_READ( queue_mask, eth_tx_queue_descriptor_ptr );

    bdmf_session_print ( session, "Head pointer      : %d\n", ( unsigned int )head_ptr );
    bdmf_session_print ( session, "Tail pointer      : %d\n", ( unsigned int )tail_ptr );
    bdmf_session_print ( session, "Packet counter    : %d\n", ( unsigned int )packet_counter );
    bdmf_session_print ( session, "Threshold         : %d\n", ( unsigned int )threshold );
    bdmf_session_print ( session, "Status Offset     : %d\n", ( unsigned int )queue_mask );

    return ( 0 );
}


static int p_lilac_rdd_print_tcont_descriptor ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_WAN_CHANNELS_0_7_TABLE_DTS       *tconts_07_table_ptr;
    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_DTS   *tcont_07_descriptor_ptr;
    RDD_WAN_CHANNELS_8_39_TABLE_DTS      *tconts_839_table_ptr;
    RDD_WAN_CHANNEL_8_39_DESCRIPTOR_DTS  *tcont_839_descriptor_ptr;
    int                                  i;
    int                                  ptr_num;
    uint8_t                              schedule;
    uint16_t                             bb_dest;
    uint32_t                             tcont_index;
    uint32_t                             rc_status;
    uint32_t                             rc_sus_vector;
    uint32_t                             rc_peak_vector;
    uint8_t                              ack_pending;
    uint8_t                              peak_offset;
    uint16_t                             peak_burst_counter;
    uint32_t                             byte_counter;
    uint32_t                             rc_ptr[ 32 ];

    tcont_index = ( int )parm[ 0 ].value.unumber;

    bdmf_session_print ( session, "Tcont %d:\n", ( int )tcont_index );
    bdmf_session_print ( session, "---------\n" );

    /* read tcont parameters */
    if ( tcont_index <= RDD_WAN_CHANNEL_7 )
    {
        tconts_07_table_ptr = ( RDD_WAN_CHANNELS_0_7_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + WAN_CHANNELS_0_7_TABLE_ADDRESS );

        tcont_07_descriptor_ptr = &( tconts_07_table_ptr->entry[ tcont_index ] );

        ptr_num = 32;

        RDD_WAN_CHANNEL_0_7_DESCRIPTOR_SCHEDULE_READ( schedule, tcont_07_descriptor_ptr );
        RDD_WAN_CHANNEL_0_7_DESCRIPTOR_BBH_DESTINATION_READ( bb_dest, tcont_07_descriptor_ptr );
        RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLERS_STATUS_READ( rc_status, tcont_07_descriptor_ptr );
        RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLERS_SUSTAIN_VECTOR_READ( rc_sus_vector, tcont_07_descriptor_ptr );
        RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLERS_PEAK_VECTOR_READ( rc_peak_vector, tcont_07_descriptor_ptr );
        RDD_WAN_CHANNEL_0_7_DESCRIPTOR_ACK_PENDING_READ( ack_pending, tcont_07_descriptor_ptr );
        RDD_WAN_CHANNEL_0_7_DESCRIPTOR_PEAK_OFFSET_READ( peak_offset, tcont_07_descriptor_ptr );
        RDD_WAN_CHANNEL_0_7_DESCRIPTOR_PEAK_BURST_COUNTER_READ( peak_burst_counter, tcont_07_descriptor_ptr );
        RDD_WAN_CHANNEL_0_7_DESCRIPTOR_BYTE_COUNTER_READ( byte_counter, tcont_07_descriptor_ptr );

        for ( i = 0; i < ptr_num; i++ )
        {
            RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLER_ADDR_READ( rc_ptr[ i ], tcont_07_descriptor_ptr, i );
        }
    }
    else
    {
        tconts_839_table_ptr = ( RDD_WAN_CHANNELS_8_39_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + WAN_CHANNELS_8_39_TABLE_ADDRESS );

        tcont_839_descriptor_ptr = &( tconts_839_table_ptr->entry[ tcont_index ] );

        ptr_num = 4;

        RDD_WAN_CHANNEL_8_39_DESCRIPTOR_SCHEDULE_READ( schedule, tcont_839_descriptor_ptr );
        RDD_WAN_CHANNEL_8_39_DESCRIPTOR_BBH_DESTINATION_READ( bb_dest, tcont_839_descriptor_ptr );
        RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_CONTROLLERS_STATUS_READ( rc_status, tcont_839_descriptor_ptr );
        RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_CONTROLLERS_SUSTAIN_VECTOR_READ( rc_sus_vector, tcont_839_descriptor_ptr );
        RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_CONTROLLERS_PEAK_VECTOR_READ( rc_peak_vector, tcont_839_descriptor_ptr );
        RDD_WAN_CHANNEL_8_39_DESCRIPTOR_ACK_PENDING_READ( ack_pending, tcont_839_descriptor_ptr );
        RDD_WAN_CHANNEL_8_39_DESCRIPTOR_PEAK_OFFSET_READ( peak_offset, tcont_839_descriptor_ptr );
        RDD_WAN_CHANNEL_8_39_DESCRIPTOR_PEAK_BURST_COUNTER_READ( peak_burst_counter, tcont_839_descriptor_ptr );
        RDD_WAN_CHANNEL_8_39_DESCRIPTOR_BYTE_COUNTER_READ( byte_counter, tcont_839_descriptor_ptr );

        for ( i = 0; i < ptr_num; i++ )
        {
            RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_CONTROLLER_ADDR_READ( rc_ptr[ i ], tcont_839_descriptor_ptr, i );
        }
    }

    bdmf_session_print ( session, "schedule         : %d\n", ( unsigned int )schedule );
    bdmf_session_print ( session, "BB destination   : %d\n", ( unsigned int )bb_dest );
    bdmf_session_print ( session, "RC status read   : %d\n", ( unsigned int )rc_status );
    bdmf_session_print ( session, "RC sust. vector  : %d\n", ( unsigned int )rc_sus_vector );
    bdmf_session_print ( session, "RC peak vector   : %d\n", ( unsigned int )rc_peak_vector );
    bdmf_session_print ( session, "ACK pending      : %d\n", ( unsigned int )ack_pending );
    bdmf_session_print ( session, "Peak offset      : %d\n", ( unsigned int )peak_offset );
    bdmf_session_print ( session, "Peak burst cntr  : %d\n", ( unsigned int )peak_burst_counter );
    bdmf_session_print ( session, "Byte counter     : %d\n", ( unsigned int )byte_counter );

    for ( i = 0; i < ptr_num; i++ )
    {
        bdmf_session_print ( session, "RC pointer %2d     : %d\n", i, ( unsigned int )rc_ptr[ i ] );
    }

    return ( 0 );
}


static int p_lilac_rdd_print_active_rc ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_WAN_CHANNELS_0_7_TABLE_DTS         *tconts_07_table_ptr;
    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_DTS     *tcont_07_descriptor_ptr;
    RDD_WAN_CHANNELS_8_39_TABLE_DTS        *tconts_839_table_ptr;
    RDD_WAN_CHANNEL_8_39_DESCRIPTOR_DTS    *tcont_839_descriptor_ptr;
    RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS  *rate_controllers_descriptor_ptr;
    RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS        *gpon_tx_queue_descriptor_ptr;
    uint32_t                               *head_packet_descriptor_ptr;
    int                                    ptr_num;
    uint16_t                               tx_queue_ptr;
    uint16_t                               packet_counter;
    uint16_t                               head_packet_ptr;
    uint32_t                               tcont_index;
    uint32_t                               rc_status;
    uint16_t                               queues_status;
    uint32_t                               rc_ptr[ 32 ];
    uint32_t                               value1;
    uint32_t                               value2;
    int                                    i;
    int                                    j;
    int                                    k;
    int                                    l;

    tcont_index = ( int )parm[ 0 ].value.unumber;

    bdmf_session_print ( session, "TCont %d:\n", ( int )tcont_index );
    bdmf_session_print ( session, "---------\n" );

    /* read tcont parameters */
    if ( tcont_index <= RDD_WAN_CHANNEL_7 )
    {
        tconts_07_table_ptr = ( RDD_WAN_CHANNELS_0_7_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + WAN_CHANNELS_0_7_TABLE_ADDRESS );

        tcont_07_descriptor_ptr = &( tconts_07_table_ptr->entry[ tcont_index ] );

        ptr_num = 32;

        RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLERS_STATUS_READ( rc_status, tcont_07_descriptor_ptr );

        for ( i = 0; i < ptr_num; i++ )
        {
            RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLER_ADDR_READ( rc_ptr[ i ], tcont_07_descriptor_ptr, i );
        }
    }
    else
    {
        tconts_839_table_ptr = ( RDD_WAN_CHANNELS_8_39_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + WAN_CHANNELS_8_39_TABLE_ADDRESS );

        tcont_839_descriptor_ptr = &( tconts_839_table_ptr->entry[ tcont_index ] );

        ptr_num = 4;

        RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_CONTROLLERS_STATUS_READ( rc_status, tcont_839_descriptor_ptr );

        for ( i = 0; i < ptr_num; i++ )
        {
            RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_CONTROLLER_ADDR_READ( rc_ptr[ i ], tcont_839_descriptor_ptr, i );
        }
    }

    /* Go through active RCs */
    for ( i = 0, j = 1; i < ptr_num; i++, j *= 2 )
    {
        if ( ( rc_status & j ) != 0 )
        {
            bdmf_session_print ( session, "Rate Controller %2d is active. Active Queues:\n", i );
            rate_controllers_descriptor_ptr = ( RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + rc_ptr[ i ] );

            RDD_US_RATE_CONTROLLER_DESCRIPTOR_PRIORITY_QUEUES_STATUS_READ( queues_status, rate_controllers_descriptor_ptr );
            /* For each RC, go through active queues */
            for ( k = 0, l = 1; k < 8; k++, l *= 2 )
            {
                if ( ( queues_status & l ) != 0 )
                {
                    RDD_US_RATE_CONTROLLER_DESCRIPTOR_TX_QUEUE_ADDR_READ( tx_queue_ptr, rate_controllers_descriptor_ptr, k );

                    gpon_tx_queue_descriptor_ptr = ( RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + tx_queue_ptr );

                    RDD_WAN_TX_QUEUE_DESCRIPTOR_HEAD_PTR_READ( head_packet_ptr, gpon_tx_queue_descriptor_ptr );
                    RDD_WAN_TX_QUEUE_DESCRIPTOR_PACKET_COUNTER_READ( packet_counter, gpon_tx_queue_descriptor_ptr );
                    head_packet_descriptor_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + head_packet_ptr );

                    MREAD_32( head_packet_descriptor_ptr,  value1 );
                    MREAD_32( head_packet_descriptor_ptr + 1,  value2 );
                    bdmf_session_print ( session, "  %4d : has %d packets enqueued.\n", k, packet_counter );
                    bdmf_session_print ( session, "  Head packet descriptor: %8x %8x\n", ( unsigned int )value1, ( unsigned int )value2 );
                }
            }
        }
    }

    return ( 0 );
}


static int p_lilac_rdd_print_free_pd_pool_info ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
#ifndef G9991
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_DTS  *pool_descriptor_ptr = NULL;
    BL_LILAC_RDD_ERROR_DTE                                 rdd_error;
    uint8_t                                                *us_packet_descriptor_ptr;
    uint32_t                                               free_pd_pool;
    uint32_t                                               pool_size_mask = 0;
    uint16_t                                               head_ptr;
    uint16_t                                               tail_ptr;
    uint16_t                                               ingress_counter;
    uint16_t                                               egress_counter;
    uint16_t                                               pool_size;
    unsigned long                                          flags;

    free_pd_pool = ( int )parm[ 0 ].value.unumber;

    switch ( free_pd_pool )
    {
    case 0:
        pool_descriptor_ptr = ( RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ADDRESS );
        pool_size_mask = RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_SIZE - 1;
        bdmf_session_print ( session, "Downstream free buffer pool\n" );
        bdmf_session_print ( session, "-------------------\n" );
        break;
    case 1:
        us_packet_descriptor_ptr = ( uint8_t * )(DEVICE_ADDRESS ( RUNNER_PRIVATE_1_OFFSET ) + US_CPU_TX_MESSAGE_DATA_BUFFER_ADDRESS );
        bdmf_session_print ( session, "Upstream free buffer pool\n" );
        bdmf_session_print ( session, "-------------------\n" );
        break;
    default:
        return BDMF_ERR_PARM;
    }

    switch ( free_pd_pool )
    {
    case 1:
        f_rdd_lock_irq ( &int_lock_irq, &flags );

        rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_GLOBAL_REGISTERS_GET, FAST_RUNNER_A, RUNNER_PRIVATE_1_OFFSET, 0, 0, 0, BL_LILAC_RDD_WAIT );

        f_rdd_unlock_irq ( &int_lock_irq, flags );

        if ( rdd_error != BL_LILAC_RDD_OK )
        {
            bdmf_session_print ( session, "Error: RDD couldn't send message to runner B firmware\n" );
            return ( BDMF_ERR_IO );
        }

        MREAD_32( us_packet_descriptor_ptr + 16,  pool_size );
        bdmf_session_print ( session, "Free buffer pool size  :  %d\n", ( unsigned int )pool_size );
        break;
    case 0:
        RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_HEAD_POINTER_READ ( head_ptr, pool_descriptor_ptr );
        RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_TAIL_POINTER_READ ( tail_ptr, pool_descriptor_ptr );
        RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_INGRESS_COUNTER_READ ( ingress_counter, pool_descriptor_ptr );
        RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_EGRESS_COUNTER_READ ( egress_counter, pool_descriptor_ptr );

        pool_size = ( ingress_counter - egress_counter ) & pool_size_mask;

        bdmf_session_print ( session, "Head pointer              : %x\n", ( unsigned int )head_ptr );
        bdmf_session_print ( session, "Tail pointer              : %x\n", ( unsigned int )tail_ptr );
        bdmf_session_print ( session, "Free buffer pool size     : %d\n", ( unsigned int )pool_size );
        break;
    }
#endif
    return ( 0 );
}


static int p_lilac_rdd_print_cpu_rx_queues_status ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_RING_DESCRIPTORS_TABLE_DTS   *ring_table_ptr;
    RDD_RING_DESCRIPTOR_DTS          *ring_descriptor_ptr;
    uint8_t                          cpu_queue;
    uint32_t                         head_ptr;
    uint16_t                         drop_counter;
    uint16_t                         interrupt;
    uint16_t                         max_size;

    bdmf_session_print ( session, "CPU RX rings:\n" );
    bdmf_session_print ( session, "--------------\n" );

    ring_table_ptr = ( RDD_RING_DESCRIPTORS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + RING_DESCRIPTORS_TABLE_ADDRESS );

    bdmf_session_print ( session, "queue | ring ptr | drop ctr | max size | interrupt\n" );

    /* Go through all CPU RX queues and prints packet descriptor heads and tails */
    for ( cpu_queue = BL_LILAC_RDD_CPU_RX_QUEUE_0; cpu_queue <= BL_LILAC_RDD_PCI_TX_QUEUE_3; cpu_queue++ )
    {
        ring_descriptor_ptr = &( ring_table_ptr->entry[ cpu_queue ] );

        RDD_RING_DESCRIPTOR_RING_POINTER_READ ( head_ptr, ring_descriptor_ptr );
        RDD_RING_DESCRIPTOR_DROP_COUNTER_READ( drop_counter, ring_descriptor_ptr );
        RDD_RING_DESCRIPTOR_INTERRUPT_ID_READ( interrupt, ring_descriptor_ptr );
        RDD_RING_DESCRIPTOR_NUMBER_OF_ENTRIES_READ( max_size, ring_descriptor_ptr );

        bdmf_session_print ( session, "  %1u   |  0x%-8x  |   %-6u  |  %-6u  | %-2u\n", ( unsigned int )cpu_queue,
                             ( unsigned int )head_ptr,  
							 ( unsigned int )drop_counter, ( unsigned int )max_size, ( unsigned int )interrupt );
    }

    return ( 0 );
}


static int p_lilac_rdd_print_ih_buffer ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    uint8_t       *ih_buffer_ptr;
    uint32_t      buffer_number;
    uint8_t       five_tup_vld;
    uint8_t       ip_first_frag;
    uint8_t       ip_frag;
    uint8_t       ip_filter_match;
    uint8_t       ip_filter_match_num;
    uint8_t       tcp_upd;
    uint8_t       ipv6_ext_hdr_flt;
    uint8_t       tcp_flag;
    uint8_t       wan;
    uint8_t       vid_fil_hit;
    uint8_t       exp;
    uint8_t       da_filt_num;
    uint8_t       da_filter;
    uint8_t       l4_protocol;
    uint8_t       error;
    uint8_t       ptag;
    uint8_t       vlan_num;
    uint8_t       broadcast;
    uint8_t       multicast;
    uint8_t       l3_protocol;
    uint8_t       l2_protocol;
    uint8_t       lkp_result_match;
    uint8_t       lkp_result_cam;
    uint8_t       lkp_result_move;
    uint16_t      lkp_result_match_offset;
    uint16_t      lkp_result_port;
    uint32_t      lkp_result_context;
    uint8_t       value;
    uint8_t       src_port;
    uint8_t       header_length;
    uint8_t       ih_class;
    uint8_t       ih_class_override_en;
    uint8_t       pti;
    uint8_t       wan_flow_id;
    uint16_t      tod;
    uint8_t       ingress_qos;
    uint8_t       target_memory;
    uint8_t       direct_mode;
    uint8_t       forward_en;
    uint8_t       rb_valid;
    unsigned int  i;

    buffer_number = ( int )parm[ 1 ].value.unumber;

    switch ( ( int )parm[ 0 ].value.unumber )
    {
    case 0:
        bdmf_session_print ( session, "Downstream IH buffer number %d:\n", ( int )buffer_number );
        ih_buffer_ptr = ( uint8_t * )(DEVICE_ADDRESS ( RUNNER_PRIVATE_0_OFFSET ) + INGRESS_HANDLER_BUFFER_ADDRESS + RDD_IH_BUFFER_SIZE * buffer_number );
        break;
    case 1:
        bdmf_session_print ( session, "Upstream IH buffer number %d:\n", ( int )buffer_number );
        ih_buffer_ptr = ( uint8_t * )(DEVICE_ADDRESS ( RUNNER_PRIVATE_1_OFFSET ) + INGRESS_HANDLER_BUFFER_ADDRESS + RDD_IH_BUFFER_SIZE * buffer_number );
        break;
    default:
        bdmf_session_print ( session, "UT: Not valid parameters\n\n\r" );
        return ( BDMF_ERR_PARM );
    }

    bdmf_session_print ( session, "--------------------------------\n\n" );

    /* Print header */
    bdmf_session_print ( session, "IH Buffer:\n" );
    bdmf_session_print ( session, "----------" );

    for ( i = 0; i < 256; i++ )
    {
        if( i % 8 == 0 )
        {
            bdmf_session_print ( session, "\n %3d: ", i );
        }
        MREAD_8( ih_buffer_ptr + i,  value );
        bdmf_session_print ( session, " %02x ", value );
    }

    /*Print parser summary*/
    RDD_PARSER_SUMMARY_FIVE_TUP_VLD_READ ( five_tup_vld, ih_buffer_ptr + 136u );
    RDD_PARSER_SUMMARY_IP_FRST_FRAG_READ ( ip_first_frag, ih_buffer_ptr + 136u );
    RDD_PARSER_SUMMARY_IP_FRAG_READ ( ip_frag, ih_buffer_ptr + 136u );
    RDD_PARSER_SUMMARY_IP_FILTER_MATCH_READ ( ip_filter_match, ih_buffer_ptr + 136u );
    RDD_PARSER_SUMMARY_IP_FILTER_MATCH_NUM_READ ( ip_filter_match_num, ih_buffer_ptr + 136u );
    RDD_PARSER_SUMMARY_TCP_UDP_READ ( tcp_upd, ih_buffer_ptr + 136u );
    RDD_PARSER_SUMMARY_IPV6_EXT_HDR_FLT_READ ( ipv6_ext_hdr_flt, ih_buffer_ptr + 136u );
    RDD_PARSER_SUMMARY_TCP_FLAG_READ ( tcp_flag, ih_buffer_ptr + 136u );
    RDD_PARSER_SUMMARY_WAN_READ ( wan, ih_buffer_ptr + 136u );
    RDD_PARSER_SUMMARY_VID_FIL_HIT_READ ( vid_fil_hit, ih_buffer_ptr + 136u );
    RDD_PARSER_SUMMARY_EXP_READ ( exp, ih_buffer_ptr + 136u );
    RDD_PARSER_SUMMARY_DA_FILT_NUM_READ ( da_filt_num, ih_buffer_ptr + 136u );
    RDD_PARSER_SUMMARY_DA_FILTER_READ ( da_filter, ih_buffer_ptr + 136u );
    RDD_PARSER_SUMMARY_L4_PROTOCOL_READ ( l4_protocol, ih_buffer_ptr + 136u );
    RDD_PARSER_SUMMARY_ERROR_READ ( error, ih_buffer_ptr + 136u );
    RDD_PARSER_SUMMARY_PTAG_READ ( ptag, ih_buffer_ptr + 136u );
    RDD_PARSER_SUMMARY_VLAN_NUM_READ ( vlan_num, ih_buffer_ptr + 136u );
    RDD_PARSER_SUMMARY_BROADCAST_READ ( broadcast, ih_buffer_ptr + 136u );
    RDD_PARSER_SUMMARY_MULTICAST_READ ( multicast, ih_buffer_ptr + 136u );
    RDD_PARSER_SUMMARY_L3_PROTOCOL_READ ( l3_protocol, ih_buffer_ptr + 136u );
    RDD_PARSER_SUMMARY_L2_PROTOCOL_READ ( l2_protocol, ih_buffer_ptr + 136u );

    bdmf_session_print ( session, "\n\nParser summary:\n" );
    bdmf_session_print ( session, "---------------\n" );
    bdmf_session_print ( session, "Five tupple valid         : %d\n",   ( unsigned int )five_tup_vld );
    bdmf_session_print ( session, "IP first fragment         : %d\n",   ( unsigned int )ip_first_frag );
    bdmf_session_print ( session, "IP fragment               : %d\n",   ( unsigned int )ip_frag );
    bdmf_session_print ( session, "IP filter match           : %d\n",   ( unsigned int )ip_filter_match );
    bdmf_session_print ( session, "IP filter match number    : %d\n",   ( unsigned int )ip_filter_match_num );
    bdmf_session_print ( session, "TCP/UDP                   : %d\n",   ( unsigned int )tcp_upd );
    bdmf_session_print ( session, "IPV6 EXT HEADER FLT       : %d\n",   ( unsigned int )ipv6_ext_hdr_flt );
    bdmf_session_print ( session, "TCP FLAG                  : %d\n",   ( unsigned int )tcp_flag );
    bdmf_session_print ( session, "WAN                       : %d\n",   ( unsigned int )wan );
    bdmf_session_print ( session, "VID fil hit               : %d\n",   ( unsigned int )vid_fil_hit );
    bdmf_session_print ( session, "Exp                       : %d\n",   ( unsigned int )exp );
    bdmf_session_print ( session, "DA filter number          : %d\n",   ( unsigned int )da_filt_num );
    bdmf_session_print ( session, "DA filter                 : %d\n",   ( unsigned int )da_filter );

    switch ( ( unsigned int )l4_protocol )
    {
    case 0:
        bdmf_session_print ( session, "L4 protocol               : Other  (0)\n" );
        break;
    case 1:
        bdmf_session_print ( session, "L4 protocol               : TCP  (1)\n" );
        break;
    case 2:
        bdmf_session_print ( session, "L4 protocol               : UDP  (2)\n" );
        break;
    case 3:
        bdmf_session_print ( session, "L4 protocol               : IGMP  (3)\n" );
        break;
    case 4:
        bdmf_session_print ( session, "L4 protocol               : ICMP  (4)\n" );
        break;
    case 5:
        bdmf_session_print ( session, "L4 protocol               : ICMPv6  (5)\n" );
        break;
    case 6:
        bdmf_session_print ( session, "L4 protocol               : ESP  (6)\n" );
        break;
    case 7:
        bdmf_session_print ( session, "L4 protocol               : GRE  (7)\n" );
        break;
    case 8:
    case 9:
    case 10:
    case 11:
        bdmf_session_print ( session, "L4 protocol               : User defined  (%d)\n", ( unsigned int )l4_protocol );
        break;
    case 13:
        bdmf_session_print ( session, "L4 protocol               : IPv6  (13)\n" );
        break;
    case 14:
        bdmf_session_print ( session, "L4 protocol               : AH  (14)\n" );
        break;
    case 15:
        bdmf_session_print ( session, "L4 protocol               : Not parsed  (15)\n" );
        break;
    default:
        bdmf_session_print ( session, "L4 protocol               : Invalid field  (%d)\n", ( unsigned int )l4_protocol );
        break;
    }

    bdmf_session_print ( session, "Error                     : %d\n", ( unsigned int )error );
    bdmf_session_print ( session, "P-Tag                     : %d\n", ( unsigned int )ptag );
    bdmf_session_print ( session, "VLAN number               : %d\n", ( unsigned int )vlan_num );
    bdmf_session_print ( session, "Broadcast                 : %d\n", ( unsigned int )broadcast );
    bdmf_session_print ( session, "Multicast                 : %d\n", ( unsigned int )multicast );

    switch ( ( unsigned int )l3_protocol )
    {
    case 0:
        bdmf_session_print ( session, "L3 Protocol               : Other/Not parsed  (0)\n" );
        break;
    case 1:
        bdmf_session_print ( session, "L3 Protocol               : IPv4  (1)\n" );
        break;
    case 2:
        bdmf_session_print ( session, "L3 Protocol               : IPv6  (2)\n" );
        break;
    default:
        bdmf_session_print ( session, "L3 Protocol               : Invalid field  (%d)\n", ( unsigned int )l3_protocol );
        break;
    }

    switch ( ( unsigned int )l2_protocol )
    {
    case 0:
        bdmf_session_print ( session, "L2 Protocol               : Unknown/Unparsed  (0)\n\n" );
        break;
    case 1:
        bdmf_session_print ( session, "L2 Protocol               : PPPoE discovery  (1)\n\n" );
        break;
    case 2:
        bdmf_session_print ( session, "L2 Protocol               : PPPoE session  (2)\n\n" );
        break;
    case 4:
        bdmf_session_print ( session, "L2 Protocol               : IPv4 over Ethernet  (4)\n\n" );
        break;
    case 5:
        bdmf_session_print ( session, "L2 Protocol               : IPv6 over Ethernet  (5)\n\n" );
        break;
    case 8:
    case 9:
    case 10:
    case 11:
        bdmf_session_print ( session, "L2 Protocol               : User defined  (%d)\n\n", ( unsigned int )l2_protocol - 8 );
        break;
    case 12:
        bdmf_session_print ( session, "L2 Protocol               : ARP (Ethertype 0x0806) (12)\n\n" );
        break;
    case 13:
        bdmf_session_print ( session, "L2 Protocol               : 1588 (Ethertype 0x88F7) (13)\n\n" );
        break;
    case 14:
        bdmf_session_print ( session, "L2 Protocol               : 802.1X (Ethertype 0x888E) (14)\n\n" );
        break;
    case 15:
        bdmf_session_print ( session, "L2 Protocol               : 801.1AG CFM (Ethertype 0x8902) (15)\n\n" );
        break;
    default:
        bdmf_session_print ( session, "L2 Protocol               : Invalid field  (%d)\n\n", ( unsigned int )l2_protocol );
        break;
    }

    bdmf_session_print ( session, "Lookup results:\n" );
    bdmf_session_print ( session, "---------------\n" );
    bdmf_session_print ( session, "index | match | cam | move | match offset |  port  | context entry\n" );

    /*Print lookup results' status */
    for ( i = 0; i < 4; i++ )
    {
        RDD_IH_LKP_RESULT_MATCH_READ ( lkp_result_match, ih_buffer_ptr + 200u + i * 8 );
        RDD_IH_LKP_RESULT_CAM_READ ( lkp_result_cam, ih_buffer_ptr + 200u + i * 8 );
        RDD_IH_LKP_RESULT_MOVE_READ ( lkp_result_move, ih_buffer_ptr + 200u + i * 8 );
        RDD_IH_LKP_RESULT_MATCH_OFFSET_READ ( lkp_result_match_offset, ih_buffer_ptr + 200u + i * 8 );
        RDD_IH_LKP_RESULT_PORT_READ ( lkp_result_port, ih_buffer_ptr + 200u + i * 8 );
        RDD_IH_LKP_RESULT_CONTEXT_ENTRY_READ ( lkp_result_context, ih_buffer_ptr + 200u + i * 8 );

        bdmf_session_print ( session, "  %1u   |   %1u   |  %1u  |  %1u   |    0x%-4x    | 0x%-4x | 0x%-8x\n", i + 1,
                           ( unsigned int )lkp_result_match, ( unsigned int )lkp_result_cam, ( unsigned int )lkp_result_move,
                           ( unsigned int )lkp_result_match_offset, ( unsigned int )lkp_result_port, ( unsigned int )lkp_result_context );
    }

    /* Print egress header */
    bdmf_session_print ( session, "\nEgress HD:\n" );
    bdmf_session_print ( session, "------------\n" );

    RDD_IH_EGRESS_HEADER_RB_VALID_READ ( rb_valid, ih_buffer_ptr + 248u );
    RDD_IH_EGRESS_HEADER_FORWARD_EN_READ ( forward_en, ih_buffer_ptr + 248u );
    RDD_IH_EGRESS_HEADER_DIRECT_MODE_READ ( direct_mode, ih_buffer_ptr + 248u );
    RDD_IH_EGRESS_HEADER_TARGET_MEMORY_READ ( target_memory, ih_buffer_ptr + 248u );
    RDD_IH_EGRESS_HEADER_INGRESS_QOS_READ ( ingress_qos, ih_buffer_ptr + 248u );
    RDD_IH_EGRESS_HEADER_1588_TOD_READ ( tod, ih_buffer_ptr + 248u );
    RDD_IH_EGRESS_HEADER_WAN_FLOW_ID_READ ( wan_flow_id, ih_buffer_ptr + 248u );
    RDD_IH_EGRESS_HEADER_PTI_READ ( pti, ih_buffer_ptr + 248u );
    RDD_IH_EGRESS_HEADER_CLASS_OVERRIDE_EN_READ ( ih_class_override_en, ih_buffer_ptr + 248u );
    RDD_IH_EGRESS_HEADER_CLASS_READ ( ih_class, ih_buffer_ptr + 248u );
    RDD_IH_EGRESS_HEADER_HLEN_READ ( header_length, ih_buffer_ptr + 248u );
    RDD_IH_EGRESS_HEADER_SRC_PORT_READ ( src_port, ih_buffer_ptr + 248u );

    bdmf_session_print ( session, "runner buffer valid     %-4u\n", ( unsigned int )rb_valid );
    bdmf_session_print ( session, "forward enable          %-4u\n", ( unsigned int )forward_en );
    bdmf_session_print ( session, "direct mode             %-4u\n", ( unsigned int )direct_mode );
    bdmf_session_print ( session, "target memory           %-s\n", ( target_memory == 0 ) ? "DDR" : "SRAM" );

    switch ( ingress_qos )
    {
    case 0:
        bdmf_session_print ( session, "ingress qos             LOW\n" );
        break;
    case 1:
        bdmf_session_print ( session, "ingress qos             HIGH\n" );
        break;
    default:
        bdmf_session_print ( session, "ingress qos             EXCLUSIVE\n" );
        break;
    }
    bdmf_session_print ( session, "1588 ToD                %-4u\n", ( unsigned int )tod );
    bdmf_session_print ( session, "WAN flow ID             %-4u\n", ( unsigned int )wan_flow_id );
    bdmf_session_print ( session, "PTI                     %-4u\n", ( unsigned int )pti );
    bdmf_session_print ( session, "IH class override en    %-4u\n", ( unsigned int )ih_class_override_en );
    bdmf_session_print ( session, "IH class                %-4u\n", ( unsigned int )ih_class );
    bdmf_session_print ( session, "header length           %-4u\n", ( unsigned int )header_length );
    bdmf_session_print ( session, "source port             %-4u\n\n", ( unsigned int )src_port );

    return ( 0 );
}


static int p_lilac_rdd_print_congestion_state ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_RUNNER_CONGESTION_STATE_DTS  *cond_state_ptr;
    uint8_t                          runner_num;
    uint8_t                          congestion_state;
    uint8_t                          allocated_ribs_num;

    runner_num = ( int )parm[ 0 ].value.unumber;

    switch ( runner_num )
    {
    case 0:
        bdmf_session_print ( session, "Runner A congestion state:\n" );
        cond_state_ptr = ( RDD_RUNNER_CONGESTION_STATE_DTS * )(DEVICE_ADDRESS ( RUNNER_PRIVATE_0_OFFSET ) + DS_RUNNER_CONGESTION_STATE_ADDRESS );
        break;
    case 1:
        bdmf_session_print ( session, "Runner B congestion state:\n" );
        cond_state_ptr = ( RDD_RUNNER_CONGESTION_STATE_DTS * )(DEVICE_ADDRESS ( RUNNER_PRIVATE_1_OFFSET ) + US_RUNNER_CONGESTION_STATE_ADDRESS );
        break;
    default:
        bdmf_session_print ( session, "UT: Not valid parameters\n\n\r" );
        return ( BDMF_ERR_PARM );
    }

    bdmf_session_print ( session, "--------------------\n" );

    RDD_RUNNER_CONGESTION_STATE_STATE_READ ( congestion_state, cond_state_ptr );
    RDD_RUNNER_CONGESTION_STATE_ALLOCATED_RIBS_NUMBER_READ ( allocated_ribs_num, cond_state_ptr );

    /* Print congestion state and number of allocated RIBs */
    bdmf_session_print ( session, "Congestion state         : " );

    switch ( ( unsigned int )congestion_state )
    {
    case 0:
        bdmf_session_print ( session, "Normal (0)\n" );
        break;
    case 1:
        bdmf_session_print ( session, "Congestion state (1)\n" );
        break;
    case 2:
        bdmf_session_print ( session, "High congestion state (2)\n" );
        break;
    case 3:
        bdmf_session_print ( session, "Exclusive congestion state (3)\n" );
        break;
    case 4:
        bdmf_session_print ( session, "Full congestion state - no RIBs (4)\n" );
        break;
    default:
        bdmf_session_print ( session, "Invalid value (%d)\n", ( int )congestion_state );
        break;
    }
    bdmf_session_print ( session, "Number of allocated RIBs : %d\n", ( unsigned int )allocated_ribs_num );
    return ( 0 );
}


static int p_lilac_rdd_print_global_registers ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint8_t                 *global_registers_ptr;
    uint8_t                 runner_num;
    uint32_t                global_register;
    int                     i;
    unsigned long           flags;

    runner_num = ( int )parm[ 0 ].value.unumber;

    switch ( runner_num )
    {
    case 0:
        bdmf_session_print ( session, "Pico Runner A global registers:\n" );
        global_registers_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_CPU_TX_MESSAGE_DATA_BUFFER_ADDRESS );

        f_rdd_lock_irq ( &int_lock_irq, &flags );

        rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_GLOBAL_REGISTERS_GET, PICO_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, 0, 0, 0, BL_LILAC_RDD_WAIT );

        f_rdd_unlock_irq ( &int_lock_irq, flags );
        break;
    case 1:
        bdmf_session_print ( session, "Pico Runner B global registers:\n" );
        global_registers_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CPU_TX_MESSAGE_DATA_BUFFER_ADDRESS );

        f_rdd_lock_irq ( &int_lock_irq, &flags );

        rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_GLOBAL_REGISTERS_GET, PICO_RUNNER_B, RUNNER_PRIVATE_1_OFFSET, 0, 0, 0, BL_LILAC_RDD_WAIT );

        f_rdd_unlock_irq ( &int_lock_irq, flags );
        break;
    default:
        bdmf_session_print ( session, "UT: Not valid parameters\n\n\r" );
        return ( BDMF_ERR_PARM );
    }

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        bdmf_session_print ( session, "Error: RDD couldn't send message to runner %d firmware\n", runner_num );
        return ( BDMF_ERR_IO );
    }

    bdmf_session_print ( session, "--------------------\n" );

    for ( i = 0; i < 8; i++ )
    {
        MREAD_32( global_registers_ptr + 4 * i,  global_register );
        bdmf_session_print ( session, "R%d = %08x\n", i, ( unsigned int )global_register );
    }

    return ( 0 );
}


static int p_lilac_rdd_start_profiling ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RUNNER_REGS_CFG_MAIN_PROFILING_CFG  runner_profiling_cfg_register;
    uint32_t                            runner;
    uint32_t                            is_pico;
    uint32_t                            task1;
    uint32_t                            task2;
    uint32_t                            trace_pc_en;

    runner = ( uint32_t )parm[ 0 ].value.unumber;
    is_pico = ( uint32_t )parm[ 1 ].value.unumber;
    task1 = ( uint32_t )parm[ 2 ].value.unumber;
    task2 = ( uint32_t )parm[ 3 ].value.unumber;
    trace_pc_en = ( uint32_t )parm[ 4 ].value.unumber;

    if ( task1 >= ( 32 / ( is_pico + 1 ) ) )
    {
        bdmf_session_print ( session, "UT: Invalid task 1 value\n\n\r" );
        return ( BDMF_ERR_PARM );
    }

    if ( task1 >= ( 32 / ( is_pico + 1 ) ) )
    {
        bdmf_session_print (  session, "UT: Invalid task 2 value\n\n\r" );
        return ( BDMF_ERR_PARM );
    }

    /* Fill profiling configuration register fields (inc. start profiling bit) */
    if ( is_pico )
    {
        if (runner) 
        {
            runner_profiling_cfg_register.trace_base_addr = (US_PROFILING_BUFFER_PICO_RUNNER_ADDRESS >> 3); 
        }
        else
        {
            runner_profiling_cfg_register.trace_base_addr = (DS_PROFILING_BUFFER_PICO_RUNNER_ADDRESS >> 3); 
        }
    }
    else
    {
        runner_profiling_cfg_register.trace_base_addr = ( PROFILING_BUFFER_MAIN_RUNNER_ADDRESS >> 3 );
    }

    runner_profiling_cfg_register.trace_pc_en = trace_pc_en;
    runner_profiling_cfg_register.prof_task2 = task2;
    runner_profiling_cfg_register.prof_task1 = task1;
    runner_profiling_cfg_register.prof_start = LILAC_RDD_ON;
    runner_profiling_cfg_register.prof_stop = LILAC_RDD_OFF;
	runner_profiling_cfg_register.rsv1 = 0;
	runner_profiling_cfg_register.rsv2 = 0;

    /* Write profiling configuration register */
    if ( is_pico == 0 )
    {
        if ( runner == 0 )
        {
            RUNNER_REGS_0_CFG_MAIN_PROFILING_CFG_WRITE ( runner_profiling_cfg_register );
        }
        else
        {
            RUNNER_REGS_1_CFG_MAIN_PROFILING_CFG_WRITE ( runner_profiling_cfg_register );
        }
    }
    else
    {
        if ( runner == 0 )
        {
            RUNNER_REGS_0_CFG_PICO_PROFILING_CFG_WRITE ( runner_profiling_cfg_register );
        }
        else
        {
            RUNNER_REGS_1_CFG_PICO_PROFILING_CFG_WRITE ( runner_profiling_cfg_register );
        }
    }

    return ( 0 );
}


static int p_lilac_rdd_stop_profiling ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RUNNER_REGS_CFG_MAIN_PROFILING_CFG  runner_profiling_cfg_register;
    uint32_t                            runner;
    uint32_t                            is_pico;

    runner = ( int )parm[ 0 ].value.unumber;
    is_pico = ( int )parm[ 1 ].value.unumber;

    /* Read profiling configuration register */
    if ( is_pico == 0 )
    {
        if ( runner == 0 )
        {
            RUNNER_REGS_0_CFG_MAIN_PROFILING_CFG_READ ( runner_profiling_cfg_register );
        }
        else
        {
            RUNNER_REGS_1_CFG_MAIN_PROFILING_CFG_READ ( runner_profiling_cfg_register );
        }
    }
    else
    {
        if ( runner == 0 )
        {
            RUNNER_REGS_0_CFG_PICO_PROFILING_CFG_READ ( runner_profiling_cfg_register );
        }
        else
        {
            RUNNER_REGS_1_CFG_PICO_PROFILING_CFG_READ ( runner_profiling_cfg_register );
        }
    }

    /* Update profiling configuration register fields (stop profiling bit) */
    runner_profiling_cfg_register.prof_start = LILAC_RDD_OFF;
    runner_profiling_cfg_register.prof_stop = LILAC_RDD_ON;

    /* Write updated profiling configuration register */
    if ( is_pico == 0 )
    {
        if ( runner == 0 )
        {
            RUNNER_REGS_0_CFG_MAIN_PROFILING_CFG_WRITE ( runner_profiling_cfg_register );
        }
        else
        {
            RUNNER_REGS_1_CFG_MAIN_PROFILING_CFG_WRITE ( runner_profiling_cfg_register );
        }
    }
    else
    {
        if ( runner == 0 )
        {
            RUNNER_REGS_0_CFG_PICO_PROFILING_CFG_WRITE ( runner_profiling_cfg_register );
        }
        else
        {
            RUNNER_REGS_1_CFG_PICO_PROFILING_CFG_WRITE ( runner_profiling_cfg_register );
        }
    }

    return ( 0 );
}


static int p_lilac_rdd_print_profiling_registers ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RUNNER_REGS_CFG_MAIN_PROFILING_STS  profiling;
    RUNNER_REGS_CFG_MAIN_STALL_CNT1     stall1_cnt;
    RUNNER_REGS_CFG_MAIN_STALL_CNT2     stall2_cnt;
    RUNNER_REGS_CFG_MAIN_TASK1_CNT      task1_cnt;
    RUNNER_REGS_CFG_MAIN_TASK2_CNT      task2_cnt;
    RUNNER_REGS_CFG_MAIN_IDLE_CNT1      idle_cnt;
    RUNNER_REGS_CFG_MAIN_JMP_CNT        jmp;

    /* Print profiling registers for all runners */
    RUNNER_REGS_0_CFG_MAIN_PROFILING_STS_READ ( profiling );
    RUNNER_REGS_0_CFG_MAIN_STALL_CNT1_READ ( stall1_cnt );
    RUNNER_REGS_0_CFG_MAIN_STALL_CNT2_READ ( stall2_cnt );
    RUNNER_REGS_0_CFG_MAIN_TASK1_CNT_READ ( task1_cnt );
    RUNNER_REGS_0_CFG_MAIN_TASK2_CNT_READ ( task2_cnt );
    RUNNER_REGS_0_CFG_MAIN_IDLE_CNT1_READ ( idle_cnt );
    RUNNER_REGS_0_CFG_MAIN_JMP_CNT_READ ( jmp );

    bdmf_session_print (  session, "Fast Runner A :\n" );
    bdmf_session_print (  session, "Next PC                    : 0x%04x\n", ( unsigned int )( profiling.agu_next_pc << 3 ) );
    bdmf_session_print (  session, "Current thread             : %u\n", ( unsigned int )profiling.curr_thread_num );
    bdmf_session_print (  session, "Idle                       : %u\n", ( unsigned int )profiling.idle_no_active_task );
    bdmf_session_print (  session, "Stall count - ACC          : %u\n", ( unsigned int )stall1_cnt.acc_stall_cnt );
    bdmf_session_print (  session, "Stall count - LD           : %u\n", ( unsigned int )stall1_cnt.ld_stall_cnt );
    bdmf_session_print (  session, "Stall count - STORE        : %u\n", ( unsigned int )stall2_cnt.store_stall_cnt );
    bdmf_session_print (  session, "Stall count - LDIO         : %u\n", ( unsigned int )stall2_cnt.ldio_stall_cnt );
    bdmf_session_print (  session, "Task 1 task count          : %u\n", ( unsigned int )task1_cnt.task1_cnt );
    bdmf_session_print (  session, "Task 2 task count          : %u\n", ( unsigned int )task2_cnt.task2_cnt );
    bdmf_session_print (  session, "Idle count                 : %u\n", ( unsigned int )idle_cnt.idle_cnt );
    bdmf_session_print (  session, "Mispredicted(predict bit=1): %u\n", ( unsigned int )jmp.taken_jmp_cnt );
    bdmf_session_print (  session, "Mispredicted(predict bit=0): %u\n", ( unsigned int )jmp.untaken_jmp_cnt );


    RUNNER_REGS_0_CFG_PICO_PROFILING_STS_READ ( profiling );
    RUNNER_REGS_0_CFG_PICO_STALL_CNT1_READ ( stall1_cnt );
    RUNNER_REGS_0_CFG_PICO_STALL_CNT2_READ ( stall2_cnt );
    RUNNER_REGS_0_CFG_PICO_TASK_CNT1_READ ( task1_cnt );
    RUNNER_REGS_0_CFG_PICO_TASK_CNT2_READ ( task2_cnt );
    RUNNER_REGS_0_CFG_PICO_IDLE_CNT1_READ ( idle_cnt );
    RUNNER_REGS_0_CFG_PICO_JMP_CNT_READ ( jmp );

    bdmf_session_print (  session, "Pico Runner A :\n" );
    bdmf_session_print (  session, "Next PC                    : 0x%04x\n", ( unsigned int )( profiling.agu_next_pc << 3 ) );
    bdmf_session_print (  session, "Current thread             : %u\n", ( unsigned int )profiling.curr_thread_num );
    bdmf_session_print (  session, "Idle                       : %u\n", ( unsigned int )profiling.idle_no_active_task );
    bdmf_session_print (  session, "Stall count - ACC          : %u\n", ( unsigned int )stall1_cnt.acc_stall_cnt );
    bdmf_session_print (  session, "Stall count - LD           : %u\n", ( unsigned int )stall1_cnt.ld_stall_cnt );
    bdmf_session_print (  session, "Stall count - STORE        : %u\n", ( unsigned int )stall2_cnt.store_stall_cnt );
    bdmf_session_print (  session, "Stall count - LDIO         : %u\n", ( unsigned int )stall2_cnt.ldio_stall_cnt );
    bdmf_session_print (  session, "Task 1 task count          : %u\n", ( unsigned int )task1_cnt.task1_cnt );
    bdmf_session_print (  session, "Task 2 task count          : %u\n", ( unsigned int )task2_cnt.task2_cnt );
    bdmf_session_print (  session, "Idle count                 : %u\n", ( unsigned int )idle_cnt.idle_cnt );
    bdmf_session_print (  session, "Mispredicted(predict bit=1): %u\n", ( unsigned int )jmp.taken_jmp_cnt );
    bdmf_session_print (  session, "Mispredicted(predict bit=0): %u\n", ( unsigned int )jmp.untaken_jmp_cnt );


    RUNNER_REGS_1_CFG_MAIN_PROFILING_STS_READ ( profiling );
    RUNNER_REGS_1_CFG_MAIN_STALL_CNT1_READ ( stall1_cnt );
    RUNNER_REGS_1_CFG_MAIN_STALL_CNT2_READ ( stall2_cnt );
    RUNNER_REGS_1_CFG_MAIN_TASK1_CNT_READ ( task1_cnt );
    RUNNER_REGS_1_CFG_MAIN_TASK2_CNT_READ ( task2_cnt );
    RUNNER_REGS_1_CFG_MAIN_IDLE_CNT1_READ ( idle_cnt );
    RUNNER_REGS_1_CFG_MAIN_JMP_CNT_READ ( jmp );

    bdmf_session_print (  session, "Fast Runner B :\n" );
    bdmf_session_print (  session, "Next PC                   : 0x%04x\n", ( unsigned int )( profiling.agu_next_pc << 3 ) );
    bdmf_session_print (  session, "Current thread            : %u\n", ( unsigned int )profiling.curr_thread_num );
    bdmf_session_print ( session, "Idle                       : %u\n", ( unsigned int )profiling.idle_no_active_task );
    bdmf_session_print ( session, "Stall count - ACC          : %u\n", ( unsigned int )stall1_cnt.acc_stall_cnt );
    bdmf_session_print ( session, "Stall count - LD           : %u\n", ( unsigned int )stall1_cnt.ld_stall_cnt );
    bdmf_session_print ( session, "Stall count - STORE        : %u\n", ( unsigned int )stall2_cnt.store_stall_cnt );
    bdmf_session_print ( session, "Stall count - LDIO         : %u\n", ( unsigned int )stall2_cnt.ldio_stall_cnt );
    bdmf_session_print ( session, "Task 1 task count          : %u\n", ( unsigned int )task1_cnt.task1_cnt );
    bdmf_session_print ( session, "Task 2 task count          : %u\n", ( unsigned int )task2_cnt.task2_cnt );
    bdmf_session_print ( session, "Idle count                 : %u\n", ( unsigned int )idle_cnt.idle_cnt );
    bdmf_session_print ( session, "Mispredicted(predict bit=1): %u\n", ( unsigned int )jmp.taken_jmp_cnt );
    bdmf_session_print ( session, "Mispredicted(predict bit=0): %u\n", ( unsigned int )jmp.untaken_jmp_cnt );
 

    RUNNER_REGS_1_CFG_PICO_PROFILING_STS_READ ( profiling );
    RUNNER_REGS_1_CFG_PICO_STALL_CNT1_READ ( stall1_cnt );
    RUNNER_REGS_1_CFG_PICO_STALL_CNT2_READ ( stall2_cnt );
    RUNNER_REGS_1_CFG_PICO_TASK_CNT1_READ ( task1_cnt );
    RUNNER_REGS_1_CFG_PICO_TASK_CNT2_READ ( task2_cnt );
    RUNNER_REGS_1_CFG_PICO_IDLE_CNT1_READ ( idle_cnt );
    RUNNER_REGS_1_CFG_PICO_JMP_CNT_READ ( jmp );

    bdmf_session_print ( session, "Pico Runner B :\n" );
    bdmf_session_print ( session, "Next PC                    : 0x%04x\n", ( unsigned int )( profiling.agu_next_pc << 3 ) );
    bdmf_session_print ( session, "Current thread             : %u\n", ( unsigned int )profiling.curr_thread_num );
    bdmf_session_print ( session, "Idle                       : %u\n", ( unsigned int )profiling.idle_no_active_task );
    bdmf_session_print ( session, "Stall count - ACC          : %u\n", ( unsigned int )stall1_cnt.acc_stall_cnt );
    bdmf_session_print ( session, "Stall count - LD           : %u\n", ( unsigned int )stall1_cnt.ld_stall_cnt );
    bdmf_session_print ( session, "Stall count - STORE        : %u\n", ( unsigned int )stall2_cnt.store_stall_cnt );
    bdmf_session_print ( session, "Stall count - LDIO         : %u\n", ( unsigned int )stall2_cnt.ldio_stall_cnt );
    bdmf_session_print ( session, "Task 1 task count          : %u\n", ( unsigned int )task1_cnt.task1_cnt );
    bdmf_session_print ( session, "Task 2 task count          : %u\n", ( unsigned int )task2_cnt.task2_cnt );
    bdmf_session_print ( session, "Idle count                 : %u\n", ( unsigned int )idle_cnt.idle_cnt );
    bdmf_session_print ( session, "Mispredicted(predict bit=1): %u\n", ( unsigned int )jmp.taken_jmp_cnt );
    bdmf_session_print ( session, "Mispredicted(predict bit=0): %u\n", ( unsigned int )jmp.untaken_jmp_cnt );

    return ( 0 );
}

#if !defined(FIRMWARE_INIT)
static int p_lilac_rdd_runner_enable ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RUNNER_REGS_CFG_GLOBAL_CTRL  runner_global_ctrl_register;
    uint32_t                     runner;
    uint32_t                     is_pico;
    uint32_t                     runner_en;

    runner = ( unsigned int )parm[ 0 ].value.unumber;
    is_pico = ( unsigned int )parm[ 1 ].value.unumber;
    runner_en = ( unsigned int )parm[ 2 ].value.unumber;

    RUNNER_REGS_CFG_GLOBAL_CTRL_READ ( runner, runner_global_ctrl_register );

    /* Update runner enabled register field (main or pico) */
    if ( is_pico == 0 )
    {
        runner_global_ctrl_register.main_en = runner_en;
    }
    else
    {
        runner_global_ctrl_register.pico_en = runner_en;
    }

    RUNNER_REGS_CFG_GLOBAL_CTRL_WRITE ( runner, runner_global_ctrl_register );

    return ( 0 );
}


static int p_lilac_rdd_breakpoint_config ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RUNNER_REGS_CFG_MAIN_BKPT_CFG  bkpt_cfg_register;
    uint32_t                       runner;
    uint32_t                       is_pico;
    uint32_t                       step_mode;

    runner    = ( unsigned int )parm[ 0 ].value.unumber;
    is_pico   = ( unsigned int )parm[ 1 ].value.unumber;
    step_mode = ( unsigned int )parm[ 2 ].value.unumber;

    bkpt_cfg_register.step_mode = step_mode;
    bkpt_cfg_register.rsv = 0x0;
    bkpt_cfg_register.new_pc_val = 0x0;
    bkpt_cfg_register.new_flags_val = 0x0;

    if ( runner )
    {
        bkpt_cfg_register.handler_addr = ( is_pico ? ADDRESS_OF(runner_d, debug_routine) : ADDRESS_OF(runner_b, debug_routine) ) >> 2;
    }
    else
    {
        bkpt_cfg_register.handler_addr = ( is_pico ? ADDRESS_OF(runner_c, debug_routine) : ADDRESS_OF(runner_a, debug_routine) ) >> 2;
    }

    /* Update breakpoint cfg register (main or pico) */
    if ( is_pico == 0 )
    {
        RUNNER_REGS_CFG_MAIN_BKPT_CFG_WRITE ( runner, bkpt_cfg_register );
    }
    else
    {
        RUNNER_REGS_CFG_PICO_BKPT_CFG_WRITE ( runner, bkpt_cfg_register );
    }

    return ( 0 );
}


static int p_lilac_rdd_set_breakpoint ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RUNNER_REGS_CFG_MAIN_BKPT_0  bkpt_cfg_register;
    uint32_t                     runner;
    uint32_t                     is_pico;
    uint32_t                     bkpt_index;
    uint32_t                     bkpt_en;
    uint32_t                     address;
    uint32_t                     use_thread;
    uint32_t                     thread;

    runner     = ( unsigned int )parm[ 0 ].value.unumber;
    is_pico    = ( unsigned int )parm[ 1 ].value.unumber;
    bkpt_index = ( unsigned int )parm[ 2 ].value.unumber;
    bkpt_en    = ( unsigned int )parm[ 3 ].value.unumber;
    address    = ( unsigned int )parm[ 4 ].value.unumber;
    use_thread = ( unsigned int )parm[ 5 ].value.unumber;
    thread     = ( unsigned int )parm[ 6 ].value.unumber;

    if ( thread > 31 || ( thread > 15 && is_pico == 1 ) )
    {
        bdmf_session_print ( session, "UT: Invalid parameter: thread\n\n\r" );
        return ( BDMF_ERR_PARM );
    }

    /* Fill breakpoint register fields */
    bkpt_cfg_register.addr = address >> 2;
    bkpt_cfg_register.enable = bkpt_en;
    bkpt_cfg_register.rsv = 0x0;
    bkpt_cfg_register.thread = thread;
    bkpt_cfg_register.use_thread = use_thread;

    /* Read breakpoint register */
    switch ( bkpt_index )
    {
    case 0:
        if ( is_pico == LILAC_RDD_FALSE )
        {
            RUNNER_REGS_CFG_MAIN_BKPT_0_WRITE ( runner, bkpt_cfg_register );
        }
        else
        {
            RUNNER_REGS_CFG_PICO_BKPT_0_WRITE ( runner, bkpt_cfg_register );
        }
        break;
    case 1:
        if ( is_pico == LILAC_RDD_FALSE )
        {
            RUNNER_REGS_CFG_MAIN_BKPT_1_WRITE ( runner, bkpt_cfg_register );
        }
        else
        {
            RUNNER_REGS_CFG_PICO_BKPT_1_WRITE ( runner, bkpt_cfg_register );
        }
        break;
    case 2:
        if ( is_pico == LILAC_RDD_FALSE )
        {
            RUNNER_REGS_CFG_MAIN_BKPT_2_WRITE ( runner, bkpt_cfg_register );
        }
        else
        {
            RUNNER_REGS_CFG_PICO_BKPT_2_WRITE ( runner, bkpt_cfg_register );
        }
        break;
    case 3:
        if ( is_pico == LILAC_RDD_FALSE )
        {
            RUNNER_REGS_CFG_MAIN_BKPT_3_WRITE ( runner, bkpt_cfg_register );
        }
        else
        {
            RUNNER_REGS_CFG_PICO_BKPT_3_WRITE ( runner, bkpt_cfg_register );
        }
        break;
    default:
        bdmf_session_print ( session, "UT: Invalid parameter\n\n\r" );
        return ( BDMF_ERR_PARM );
    }

    return ( 0 );
}


static int p_lilac_rdd_print_breakpoint_status ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RUNNER_REGS_CFG_MAIN_BKPT_STS  bkpt_sts_register;
    uint32_t                       runner;
    uint32_t                       is_pico;
    uint32_t                       i;
    uint32_t                       *debug_buffer_ptr;
    uint32_t                       *peripheral_register_ptr;
    uint32_t                       register_value;
    uint32_t                       current_thread_number;


    runner  = ( unsigned int )parm[ 0 ].value.unumber;
    is_pico = ( unsigned int )parm[ 1 ].value.unumber;

    /* Update breakpoint cfg register (main or pico) */
    if ( is_pico == 0 )
    {
        RUNNER_REGS_CFG_MAIN_BKPT_STS_READ ( runner, bkpt_sts_register );
    }
    else
    {
        RUNNER_REGS_CFG_PICO_BKPT_STS_READ ( runner, bkpt_sts_register );
    }

    if ( bkpt_sts_register.active == LILAC_RDD_TRUE )
    {
        bdmf_session_print ( session, "Runner %u %s Breakpoint active\n", ( unsigned )runner, ( is_pico == LILAC_RDD_FALSE ) ? "FAST" : "PICO" );
    }
    else
    {
        bdmf_session_print ( session, "Runner %u %s Breakpoint not active\n", ( unsigned )runner, ( is_pico == LILAC_RDD_FALSE ) ? "FAST" : "PICO" );
    }

    if ( runner == 0 )
    {
        debug_buffer_ptr = ( uint32_t * )( DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_DEBUG_BUFFER_ADDRESS );
        peripheral_register_ptr = ( uint32_t * )( DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_DEBUG_PERIPHERALS_STATUS_REGISTER_ADDRESS );
    }
    else
    {
        debug_buffer_ptr = ( uint32_t * )( DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_DEBUG_BUFFER_ADDRESS );
        peripheral_register_ptr = ( uint32_t * )( DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_DEBUG_PERIPHERALS_STATUS_REGISTER_ADDRESS );
    }

    MREAD_8( ( ( uint8_t * )peripheral_register_ptr + 1 ), current_thread_number );
    current_thread_number = current_thread_number & 31;

    bdmf_session_print ( session, "----------------\n" );
    bdmf_session_print ( session, "Current thread number: 0x%-8x\n\n", ( unsigned )current_thread_number );
    bdmf_session_print ( session, "Breakpoint address:  0x0%-4x\n\n", bkpt_sts_register.bkpt_addr << 2 );

    for ( i = 0; i < 32; i++ )
    {
        MREAD_32( debug_buffer_ptr + i, register_value );
        bdmf_session_print ( session, "register %2u = 0x%-8x\n", ( unsigned )i, ( unsigned )register_value );
    }

    bdmf_session_print ( session, "\n" );

    return ( 0 );
}
#endif /* !defined(FIRMWARE_INIT) */

static int p_lilac_rdd_check_lists ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    uint8_t  print_ds_free_pool_pd;
    uint8_t  print_ds_tx_queues_pd;
	uint8_t  print_us_free_pool_pd;
    uint8_t  print_us_tx_queues_pd;

    print_ds_free_pool_pd = ( uint8_t )parm[ 0 ].value.unumber;
    print_ds_tx_queues_pd = ( uint8_t )parm[ 1 ].value.unumber;
    print_us_free_pool_pd = ( uint8_t )parm[ 2 ].value.unumber;
    print_us_tx_queues_pd = ( uint8_t )parm[ 3 ].value.unumber;

	bdmf_session_print ( session, "Lists Integrity check:\n" );
	bdmf_session_print ( session, "----------------------\n\n" );

	/* DS free PD pool */
	p_lilac_rdd_check_lists_ds_free_pool ( session, print_ds_free_pool_pd );
	bdmf_session_print ( session, "\n" );

	/* ETH/PCI TX queues */
	p_lilac_rdd_check_lists_ds_queues ( session, print_ds_tx_queues_pd );
	bdmf_session_print ( session, "\n" );

	/* US free PD pool */
	p_lilac_rdd_check_lists_us_free_pool ( session, print_us_free_pool_pd );
	bdmf_session_print ( session, "\n" );

	/* GPON TX queues */
	p_lilac_rdd_check_lists_us_queues ( session, print_us_tx_queues_pd );
	bdmf_session_print ( session, "\n" );

#if 0
	/* CPU free PD pool */
	p_lilac_rdd_check_lists_cpu_free_pool ( session, print_cpu_free_pool_pd );
	bdmf_session_print ( session, "\n" );
#endif

    return ( 0 );
}


static void p_lilac_rdd_check_lists_ds_free_pool ( bdmf_session_handle session, uint8_t xi_print_descriptors )
{
#ifndef G9991
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_DTS  *free_packet_descriptors_pool_descriptor_ptr;
	uint32_t                                               list_size;
	uint16_t                                               head_address;
	uint16_t                                               tail_address;
	uint16_t                                               ingress_counter;
	uint16_t                                               egress_counter;

    free_packet_descriptors_pool_descriptor_ptr = ( RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ADDRESS );

    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_HEAD_POINTER_READ ( head_address, free_packet_descriptors_pool_descriptor_ptr );
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_TAIL_POINTER_READ ( tail_address, free_packet_descriptors_pool_descriptor_ptr );
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_INGRESS_COUNTER_READ ( ingress_counter, free_packet_descriptors_pool_descriptor_ptr );
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_EGRESS_COUNTER_READ ( egress_counter, free_packet_descriptors_pool_descriptor_ptr );

	bdmf_session_print ( session, "DS free PD pool:\n" );

	list_size = p_lilac_rdd_check_lists_get_list_size ( session, head_address, tail_address, RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_SIZE, (int)(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET )), xi_print_descriptors );

	bdmf_session_print ( session, "counter         %-4u\n", ingress_counter - egress_counter );

	if ( list_size == 0 )
	{
		bdmf_session_print ( session, "PDs             ERROR\n" );
	}
	else
	{
		bdmf_session_print ( session, "PDs             %-4u\n", ( unsigned )list_size );
	}
#endif
	return;
}


static void p_lilac_rdd_check_lists_ds_queues ( bdmf_session_handle session, uint8_t xi_print_descriptors )
{
#ifndef G9991
    RDD_ETH_TX_QUEUE_DESCRIPTOR_DTS  *eth_tx_queue_descriptor_ptr;
	uint32_t                         emac_id;
	uint32_t                         tx_queue;
    uint32_t                         list_size;
	uint16_t                         eth_tx_queue_address;
	uint16_t                         head_address;
	uint16_t                         tail_address;
	uint16_t                         packet_counter;
    uint32_t                         queue_number;


	bdmf_session_print ( session, "ETH TX / PCI TX queues:\n" );

    for ( emac_id = BL_LILAC_RDD_EMAC_ID_START; emac_id <= BL_LILAC_RDD_EMAC_ID_COUNT; emac_id++ )
    {
#ifdef G9991
        queue_number = LILAC_RDD_EMAC_NUMBER_OF_QUEUES;
#else
        queue_number = ( ( emac_id == BL_LILAC_RDD_EMAC_ID_PCI )? LILAC_RDD_PCI_TX_NUMBER_OF_FIFOS : LILAC_RDD_EMAC_NUMBER_OF_QUEUES );
#endif

        for ( tx_queue = 0; tx_queue < queue_number; tx_queue++ )
        {
            eth_tx_queue_address = ETH_TX_QUEUES_TABLE_ADDRESS + ( emac_id * LILAC_RDD_EMAC_NUMBER_OF_QUEUES + tx_queue ) *
                                   sizeof ( RDD_ETH_TX_QUEUE_DESCRIPTOR_DTS );

            eth_tx_queue_descriptor_ptr = ( RDD_ETH_TX_QUEUE_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + eth_tx_queue_address );

            RDD_ETH_TX_QUEUE_DESCRIPTOR_HEAD_PTR_READ ( head_address, eth_tx_queue_descriptor_ptr );
            RDD_ETH_TX_QUEUE_DESCRIPTOR_TAIL_PTR_READ ( tail_address, eth_tx_queue_descriptor_ptr );
		    RDD_ETH_TX_QUEUE_DESCRIPTOR_INGRESS_PACKET_COUNTER_READ ( packet_counter, eth_tx_queue_descriptor_ptr );

			bdmf_session_print ( session, "EMAC ID %u queue %u:\n", ( unsigned )emac_id, ( unsigned )tx_queue );

			list_size = p_lilac_rdd_check_lists_get_list_size ( session, head_address, tail_address, RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_SIZE, (int)(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET )), xi_print_descriptors );

			bdmf_session_print ( session, "counter         %-4u\n", packet_counter );

			if ( list_size == 0 )
			{
				bdmf_session_print ( session, "PDs             ERROR\n" );
			}
			else
			{
				bdmf_session_print ( session, "PDs             %-4u\n", ( unsigned )list_size );
			}
			bdmf_session_print ( session, "\n" );
		}
	}
#endif
	return;
}


static void p_lilac_rdd_check_lists_us_free_pool ( bdmf_session_handle session, uint8_t xi_print_descriptors )
{
    RDD_PACKET_DESCRIPTOR_DTS  *packet_descriptor_ptr;
    BL_LILAC_RDD_ERROR_DTE     rdd_error;
    uint8_t                    *global_registers_ptr;
    uint32_t                   head_address;
    uint32_t                   counter;
    uint16_t                   next_pd_address;
    uint32_t                   i;
    unsigned long              flags;

	global_registers_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CPU_TX_MESSAGE_DATA_BUFFER_ADDRESS );

    f_rdd_lock_irq ( &int_lock_irq, &flags );

    rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_GLOBAL_REGISTERS_GET, PICO_RUNNER_B, RUNNER_PRIVATE_1_OFFSET, 0, 0, 0, BL_LILAC_RDD_WAIT );

    f_rdd_unlock_irq ( &int_lock_irq, flags );

    if ( rdd_error != BL_LILAC_RDD_OK )
    {
        bdmf_session_print ( session, "Error: RDD couldn't send message to fw - can't fetch US free PD pool data\n" );
        return;
    }

    MREAD_32( global_registers_ptr + 4 * 2,  head_address );
    MREAD_32( global_registers_ptr + 4 * 4,  counter );

    next_pd_address = ( uint16_t ) head_address;

    bdmf_session_print ( session, "US free PD pool:\n" );

    for ( i = 0; i <= RDD_US_FREE_PACKET_DESCRIPTORS_POOL_SIZE; i++ )
    {
        packet_descriptor_ptr = ( RDD_PACKET_DESCRIPTOR_DTS * )( DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + next_pd_address );

        if ( xi_print_descriptors > i )
        {
            bdmf_session_print ( session, "PD %-4u: 0x%04X\n", ( unsigned )( i + 1 ), next_pd_address );
        }

        if ( next_pd_address == 0 )
        {
            break;
        }

        RDD_PACKET_DESCRIPTOR_NEXT_PACKET_DESCRIPTOR_POINTER_READ ( next_pd_address, packet_descriptor_ptr );
    }

    bdmf_session_print ( session, "counter         %-4u\n", ( unsigned )counter );

    if ( i == RDD_US_FREE_PACKET_DESCRIPTORS_POOL_SIZE + 1 )
    {
        bdmf_session_print ( session, "PDs             ERROR\n" );
    }
    else
    {
        bdmf_session_print ( session, "PDs             %-4u\n", ( unsigned )( i + 1 ) );
    }

    return;
}


static void p_lilac_rdd_check_lists_us_queues ( bdmf_session_handle session, uint8_t xi_print_descriptors )
{
	RDD_WAN_CHANNELS_0_7_TABLE_DTS         *tconts_0_7_table_ptr = NULL;
	RDD_WAN_CHANNEL_0_7_DESCRIPTOR_DTS     *tcont_0_7_descriptor_ptr = NULL;
	RDD_WAN_CHANNELS_8_39_TABLE_DTS        *tconts_8_39_table_ptr = NULL;
	RDD_WAN_CHANNEL_8_39_DESCRIPTOR_DTS    *tcont_8_39_descriptor_ptr = NULL;
	RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS  *rate_controller_descriptor_ptr = NULL;
	RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS        *gpon_tx_queue_descriptor_ptr = NULL;
	uint32_t                               tcont_id;
	uint32_t                               rc_status;
	uint32_t                               rc_id;
	uint32_t                               queue_id;
	uint32_t                               list_size;
	uint16_t                               counter;
	uint16_t                               rc_address;
	uint16_t                               queues_status;
	uint16_t                               queue_address;
	uint16_t                               head_address;
	uint16_t                               tail_address;


	tconts_0_7_table_ptr = ( RDD_WAN_CHANNELS_0_7_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + WAN_CHANNELS_0_7_TABLE_ADDRESS );

	tconts_8_39_table_ptr = ( RDD_WAN_CHANNELS_8_39_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + WAN_CHANNELS_8_39_TABLE_ADDRESS );

	bdmf_session_print ( session, "GPON TX queues:\n" );

	for ( tcont_id = RDD_WAN_CHANNEL_0; tcont_id <= RDD_WAN_CHANNEL_39; tcont_id++ )
	{
		if ( tcont_id <= RDD_WAN_CHANNEL_7 )
		{
			tcont_0_7_descriptor_ptr = &( tconts_0_7_table_ptr->entry[ tcont_id ] );
			
		    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLERS_STATUS_READ ( rc_status, tcont_0_7_descriptor_ptr );
		}
		else
		{
			tcont_8_39_descriptor_ptr = &( tconts_8_39_table_ptr->entry[ tcont_id - RDD_WAN_CHANNEL_8 ] );

		    RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_CONTROLLERS_STATUS_READ ( rc_status, tcont_8_39_descriptor_ptr );
		}

		for ( rc_id = BL_LILAC_RDD_RATE_CONTROLLER_0; rc_id <= ( ( tcont_id <= RDD_WAN_CHANNEL_7 ) ? BL_LILAC_RDD_RATE_CONTROLLER_31 : BL_LILAC_RDD_RATE_CONTROLLER_3 ); rc_id++ )
		{
			if ( ( ( rc_status >> rc_id ) & 0x01 ) == 0 )
			{
				break;
			}

			if ( tcont_id <= RDD_WAN_CHANNEL_7 )
			{
			    RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLER_ADDR_READ ( rc_address, tcont_0_7_descriptor_ptr, rc_id );
			}
			else
			{
			    RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_CONTROLLER_ADDR_READ ( rc_address, tcont_8_39_descriptor_ptr, rc_id );
			}

			rate_controller_descriptor_ptr = ( RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + rc_address - sizeof ( RUNNER_COMMON ) );

		    RDD_US_RATE_CONTROLLER_DESCRIPTOR_PRIORITY_QUEUES_STATUS_READ ( queues_status, rate_controller_descriptor_ptr );

			for ( queue_id = BL_LILAC_RDD_QUEUE_0; queue_id <= BL_LILAC_RDD_QUEUE_7; queue_id++ )
			{
				if ( ( ( queues_status >> queue_id ) & 0x01 ) == 0 )
				{
					break;
				}

			    RDD_US_RATE_CONTROLLER_DESCRIPTOR_TX_QUEUE_ADDR_READ ( queue_address, rate_controller_descriptor_ptr, queue_id );

				gpon_tx_queue_descriptor_ptr = ( RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + queue_address - sizeof ( RUNNER_COMMON ) );

			    RDD_WAN_TX_QUEUE_DESCRIPTOR_HEAD_PTR_READ ( head_address, gpon_tx_queue_descriptor_ptr );
			    RDD_WAN_TX_QUEUE_DESCRIPTOR_TAIL_PTR_READ ( tail_address, gpon_tx_queue_descriptor_ptr );
			    RDD_WAN_TX_QUEUE_DESCRIPTOR_PACKET_COUNTER_READ ( counter, gpon_tx_queue_descriptor_ptr );

				bdmf_session_print ( session, "TCONT  %3u; rate controller %3u; queue %3u:\n", ( unsigned )tcont_id, ( unsigned )rc_id, ( unsigned )queue_id );

				list_size = p_lilac_rdd_check_lists_get_list_size ( session, head_address, tail_address, RDD_US_FREE_PACKET_DESCRIPTORS_POOL_SIZE, (int)(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET )), xi_print_descriptors );

				bdmf_session_print ( session, "counter         %-4u\n", counter );

				if ( list_size == 0 )
				{
					bdmf_session_print ( session, "PDs             ERROR\n\n" );
				}
				else
				{
					bdmf_session_print ( session, "PDs             %-4u\n\n", ( unsigned )list_size );
				}
			}
		}
	}

	return;
}


#if 0
static void p_lilac_rdd_check_lists_cpu_free_pool ( bdmf_session_handle session, uint8_t xi_print_descriptors )
{
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_DTS  *free_packet_descriptors_pool_descriptor_ptr;
    uint32_t                                               list_size;
	uint16_t                                               head_address;
	uint16_t                                               tail_address;
	uint16_t                                               ingress_counter;
	uint16_t                                               egress_counter;

    free_packet_descriptors_pool_descriptor_ptr =
        ( RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + CPU_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ADDRESS );

    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_HEAD_POINTER_READ ( head_address, free_packet_descriptors_pool_descriptor_ptr );
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_TAIL_POINTER_READ ( tail_address, free_packet_descriptors_pool_descriptor_ptr );
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_INGRESS_COUNTER_READ ( ingress_counter, free_packet_descriptors_pool_descriptor_ptr );
    RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_EGRESS_COUNTER_READ ( egress_counter, free_packet_descriptors_pool_descriptor_ptr );

	bdmf_session_print ( session, "CPU free PD pool:\n" );

	list_size = p_lilac_rdd_check_lists_get_list_size ( session, head_address, tail_address,
		                                                RDD_CPU_FREE_PACKET_DESCRIPTORS_POOL_SIZE,
											            DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) - sizeof ( RUNNER_COMMON ),
														xi_print_descriptors );

	bdmf_session_print ( session, "counter         %-4u\n", ingress_counter - egress_counter );

	if ( list_size == 0 )
	{
		bdmf_session_print ( session, "PDs             ERROR\n" );
	}
	else
	{
		bdmf_session_print ( session, "PDs             %-4u\n", ( unsigned )list_size );
	}

	return;
}
#endif



static uint32_t p_lilac_rdd_check_lists_get_list_size ( bdmf_session_handle  session,
														  uint16_t           xi_head_address,
												          uint16_t           xi_tail_address,
														  uint32_t           xi_max_list_size,
														  uint32_t           xi_memory_segment_offset,
														  uint8_t            xi_print_descriptors )
{
    RDD_PACKET_DESCRIPTOR_DTS  *packet_descriptor_ptr;
    uint16_t                   next_packet_descriptor_address;
	uint32_t                   i;

	if ( xi_print_descriptors > 0 )
	{
		bdmf_session_print ( session, "PD    1: 0x%04X\n", xi_head_address );
	}

	packet_descriptor_ptr = ( RDD_PACKET_DESCRIPTOR_DTS * )( xi_memory_segment_offset + xi_head_address );

	if ( xi_head_address == xi_tail_address )
	{
		return ( 1 );
	}

	for ( i = 0; i < xi_max_list_size; i++ )
	{
	    RDD_PACKET_DESCRIPTOR_NEXT_PACKET_DESCRIPTOR_POINTER_READ ( next_packet_descriptor_address, packet_descriptor_ptr );

		if ( next_packet_descriptor_address == xi_tail_address )
		{
			return ( i + 2 );
		}

		if ( xi_print_descriptors > i )
		{
			bdmf_session_print ( session, "PD %-4u: 0x%04X\n", ( unsigned )( i + 2 ), xi_head_address );
		}

		packet_descriptor_ptr = ( RDD_PACKET_DESCRIPTOR_DTS * )( xi_memory_segment_offset + next_packet_descriptor_address );
	}

	return ( 0 );
}


static int p_lilac_rdd_print_ds_wan_flow ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_DS_WAN_FLOW_TABLE_DTS  *wan_flow_table_ptr;
    RDD_DS_WAN_FLOW_ENTRY_DTS  *wan_flow_entry_ptr;
    uint32_t                   index;
    uint8_t                    ingress_flow;
    uint8_t                    icm;
    uint8_t                    cpu_reason;

    wan_flow_table_ptr = ( RDD_DS_WAN_FLOW_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_WAN_FLOW_TABLE_ADDRESS );

    bdmf_session_print ( session, "WAN flow | Ingress Flow | Classify Mode | CPU Reason\n" );

    for ( index = 0; index < RDD_DS_WAN_FLOW_TABLE_SIZE; index++ )
    {
        wan_flow_entry_ptr = &( wan_flow_table_ptr->entry[ index ] );

        RDD_DS_WAN_FLOW_ENTRY_INGRESS_FLOW_READ ( ingress_flow, wan_flow_entry_ptr );
        RDD_DS_WAN_FLOW_ENTRY_INGRESS_CLASSIFY_MODE_READ ( icm, wan_flow_entry_ptr );
        RDD_DS_WAN_FLOW_ENTRY_CPU_REASON_READ ( cpu_reason, wan_flow_entry_ptr );

        if ( icm )
        {
            bdmf_session_print ( session, "   %3d   |    %6d    |    Packet     |    %3d    \n", index, ingress_flow, cpu_reason );
        }
        else
        {
            bdmf_session_print ( session, "   %3d   |    %6d    |    Flow       |    %3d    \n", index, ingress_flow, cpu_reason );
        }
    }

    return ( 0 );
}


static int p_lilac_rdd_print_us_wan_flow ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    uint32_t                      index;
    uint32_t                      wan_port_id;
    RDD_WAN_CHANNEL_ID            wan_channel_id;
    BL_LILAC_RDD_TX_CRC_CALC_DTE  crc_calc;
    uint8_t                       pbits_to_queue_table_index;
    uint8_t                       traffic_class_to_queue_table_index;


    bdmf_session_print ( session, "WAN flow | WAN channel ID | WAN Port ID | CRC Calc | Pbits to Queue Index | Traffic class to Queue Index \n" );

    for ( index = 0; index < RDD_US_WAN_FLOW_TABLE_SIZE; index++ )
    {
        rdd_us_wan_flow_get ( index, &wan_channel_id, &wan_port_id, &crc_calc, &pbits_to_queue_table_index, &traffic_class_to_queue_table_index );
        bdmf_session_print ( session, "   %3d   |      %4d      |    %5d    |  %6d  |     %9d     |      %5d     \n", index, wan_channel_id, wan_port_id, crc_calc, pbits_to_queue_table_index, traffic_class_to_queue_table_index );
    }

    return ( 0 );
}


static int p_lilac_rdd_print_global_dscp_to_pbits ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_GLOBAL_DSCP_TO_PBITS_TABLE_DTS  *global_dscp_to_pbits_table_ptr;
    RDD_DSCP_TO_PBITS_ENTRY_DTS         *dscp_to_pbits_entry_ptr;
    uint32_t                            dscp_index;
    uint8_t                             pbits;

    global_dscp_to_pbits_table_ptr = ( RDD_GLOBAL_DSCP_TO_PBITS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + GLOBAL_DSCP_TO_PBITS_TABLE_ADDRESS );

    bdmf_session_print ( session, " DSCP - P-bits Table \n" );
    bdmf_session_print ( session, "---------------------\n" );

    for ( dscp_index = 0; dscp_index < RDD_GLOBAL_DSCP_TO_PBITS_TABLE_SIZE2; dscp_index++ )
    {
        dscp_to_pbits_entry_ptr = &( global_dscp_to_pbits_table_ptr->entry[ dscp_index ] );
        RDD_DSCP_TO_PBITS_ENTRY_PBITS_READ ( pbits, dscp_to_pbits_entry_ptr );

        bdmf_session_print ( session, "    %2d    |    %2d    \n", ( int )dscp_index, pbits );
    }
    return ( 0 );
}


static int p_lilac_rdd_print_dscp_to_pbits ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_DS_DSCP_TO_PBITS_TABLE_DTS  	*ds_dscp_to_pbits_table_ptr;
    RDD_US_DSCP_TO_PBITS_TABLE_DTS  	*us_dscp_to_pbits_table_ptr;
    RDD_DSCP_TO_PBITS_ENTRY_DTS     	*dscp_to_pbits_entry_ptr;
    uint32_t                            dscp_index;
    uint8_t                             pbits;
    uint8_t                             bridge_port;

    bridge_port = ( int )parm[ 0 ].value.unumber;

    if ( ( bridge_port > BL_LILAC_RDD_LAN4_BRIDGE_PORT ) && ( bridge_port < BL_LILAC_RDD_PCI_BRIDGE_PORT ) )
    {
        bdmf_session_print ( session, "INVALID PORT NUMBER!\n" );
        return ( 1 );
    }

    if ( bridge_port == BL_LILAC_RDD_WAN_BRIDGE_PORT )
    {
        ds_dscp_to_pbits_table_ptr = ( RDD_DS_DSCP_TO_PBITS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_DSCP_TO_PBITS_TABLE_ADDRESS );

        bdmf_session_print ( session, " Downstream DSCP - P-bits \n" );
        bdmf_session_print ( session, " Port Number: %d \n", bridge_port );
        bdmf_session_print ( session, "--------------------------\n" );

        for ( dscp_index = 0; dscp_index < RDD_DS_DSCP_TO_PBITS_TABLE_SIZE2; dscp_index++ )
        {
            dscp_to_pbits_entry_ptr = &( ds_dscp_to_pbits_table_ptr->entry[ dscp_index ] );
            RDD_DSCP_TO_PBITS_ENTRY_PBITS_READ ( pbits, dscp_to_pbits_entry_ptr );
            bdmf_session_print ( session, "     %3d     |     %3d     \n", ( int )dscp_index, pbits );
        }
    }
    else 
    {
        if ( bridge_port == BL_LILAC_RDD_PCI_BRIDGE_PORT )
        {
            bridge_port = 0;
        }

        us_dscp_to_pbits_table_ptr = ( RDD_US_DSCP_TO_PBITS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_DSCP_TO_PBITS_TABLE_ADDRESS );

        bdmf_session_print ( session, " Upstream DSCP - P-bits \n" );
        bdmf_session_print ( session, " Port Number: %d \n", bridge_port );
        bdmf_session_print ( session, "------------------------\n" );

        for ( dscp_index = 0; dscp_index < RDD_US_DSCP_TO_PBITS_TABLE_SIZE2; dscp_index++ )
        {
            dscp_to_pbits_entry_ptr = &( us_dscp_to_pbits_table_ptr->entry[ bridge_port ][ dscp_index ] );
            RDD_DSCP_TO_PBITS_ENTRY_PBITS_READ ( pbits, dscp_to_pbits_entry_ptr );
            bdmf_session_print ( session, "    %3d    |    %3d    \n", ( int )dscp_index, pbits );
        }
    }

    return ( 0 );
}


static int p_lilac_rdd_print_mac_table ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    BL_LILAC_RDD_BRIDGE_PORT_DTE       src_port;
    BL_LILAC_RDD_AGGREGATION_MODE_DTE  aggregation_flag;
    BL_LILAC_RDD_MAC_ENTRY_TYPE_DTE    entry_type;
    BL_LILAC_RDD_MAC_FWD_ACTION_DTE    sa_action;
	BL_LILAC_RDD_MAC_FWD_ACTION_DTE    da_action;
    RDD_MAC_PARAMS                     mac_params;
	BL_LILAC_RDD_ERROR_DTE             rdd_error;
    uint32_t                           mac_index;
    uint32_t                           index;
    uint8_t                            extension;
    bdmf_mac_t                         mac_addr;
    uint32_t                           aging;
    uint32_t                           skip;
    uint32_t                           valid;
    uint32_t                           counter = 0;

    bdmf_session_print ( session, " MAC Table\n" );
    bdmf_session_print ( session, "----------\n" );

    bdmf_session_print ( session, " index | source port | sa act. | da act. |    mac address    | aging | skip | valid | CAM | agg. | ext. | type\n" );

    for ( mac_index = 0; mac_index < 1024 + 32; mac_index++ )
    {
        rdd_error = rdd_mac_entry_get ( mac_index, &mac_params, &valid, &skip, &aging );
        memcpy ( mac_addr.b, mac_params.mac_addr.b, 6 );
        src_port = mac_params.bridge_port;
        entry_type = mac_params.entry_type;
        aggregation_flag = mac_params.aggregation_mode;
        extension = mac_params.extension_entry;
        sa_action = mac_params.sa_action;
        da_action = mac_params.da_action;

        if ( valid != 0 )
        {
            bdmf_session_print ( session, " %-7u ", ( unsigned )mac_index );
            switch ( src_port )
            {
            case BL_LILAC_RDD_WAN_BRIDGE_PORT :
                bdmf_session_print ( session, "%-13s ", "WAN" );
                break;
            case BL_LILAC_RDD_LAN0_BRIDGE_PORT :
                bdmf_session_print ( session, "%-13s ", "ETH0" );
                break;
            case BL_LILAC_RDD_LAN1_BRIDGE_PORT :
                bdmf_session_print ( session, "%-13s ", "ETH1" );
                break;
            case BL_LILAC_RDD_LAN2_BRIDGE_PORT :
                bdmf_session_print ( session, "%-13s ", "ETH2" );
                break;
            case BL_LILAC_RDD_LAN3_BRIDGE_PORT :
                bdmf_session_print ( session, "%-13s ", "ETH3" );
                break;
            case BL_LILAC_RDD_LAN4_BRIDGE_PORT :
                bdmf_session_print ( session, "%-13s ", "ETH4" );
                break;
            case BL_LILAC_RDD_WAN_ROUTER_PORT :
                bdmf_session_print ( session, "%-13s ", "WAN ROUTER" );
                break;
            case BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT :
                bdmf_session_print ( session, "%-13s ", "WAN IPTV" );
                break;
            case BL_LILAC_RDD_PCI_BRIDGE_PORT :
                bdmf_session_print ( session, "%-13s ", "PCI" );
                break;
            case BL_LILAC_RDD_WAN_QUASI_BRIDGE_PORT :
                bdmf_session_print ( session, "%-13s ", "WAN QUASI" );
                break;
            default :
                if ( src_port & 0x80 )
                {
                    bdmf_session_print ( session, "MC: 0x%-7X ", src_port );
                    break;
                }
                bdmf_session_print ( session, "%-13s ", "INVALID" );
                break;
            }

			switch ( sa_action )
			{
			case BL_LILAC_RDD_MAC_FWD_ACTION_FORWARD :
				bdmf_session_print ( session, "%-9s ", "FORWARD" );
				break;
			case BL_LILAC_RDD_MAC_FWD_ACTION_DROP :
				bdmf_session_print ( session, "%-9s ", "DROP" );
				break;
			case BL_LILAC_RDD_MAC_FWD_ACTION_CPU_TRAP0 :
				bdmf_session_print ( session, "%-9s ", "TRAP0" );
				break;
			case BL_LILAC_RDD_MAC_FWD_ACTION_CPU_TRAP1 :
				bdmf_session_print ( session, "%-9s ", "TRAP0" );
				break;
			case BL_LILAC_RDD_MAC_FWD_ACTION_CPU_TRAP2 :
				bdmf_session_print ( session, "%-9s ", "TRAP0" );
				break;
			case BL_LILAC_RDD_MAC_FWD_ACTION_CPU_TRAP3 :
				bdmf_session_print ( session, "%-9s ", "TRAP0" );
				break;
			case BL_LILAC_RDD_MAC_FWD_ACTION_RATE_LIMIT :
				bdmf_session_print ( session, "%-9s ", "RATE_LIM" );
				break;
			default :
                bdmf_session_print ( session, "%-9s ", "INVALID" );
                break;
			}

			switch ( da_action )
			{
			case BL_LILAC_RDD_MAC_FWD_ACTION_FORWARD :
				bdmf_session_print ( session, "%-9s ", "FORWARD" );
				break;
			case BL_LILAC_RDD_MAC_FWD_ACTION_DROP :
				bdmf_session_print ( session, "%-9s ", "DROP" );
				break;
			case BL_LILAC_RDD_MAC_FWD_ACTION_CPU_TRAP0 :
				bdmf_session_print ( session, "%-9s ", "TRAP0" );
				break;
			case BL_LILAC_RDD_MAC_FWD_ACTION_CPU_TRAP1 :
				bdmf_session_print ( session, "%-9s ", "TRAP0" );
				break;
			case BL_LILAC_RDD_MAC_FWD_ACTION_CPU_TRAP2 :
				bdmf_session_print ( session, "%-9s ", "TRAP0" );
				break;
			case BL_LILAC_RDD_MAC_FWD_ACTION_CPU_TRAP3 :
				bdmf_session_print ( session, "%-9s ", "TRAP0" );
				break;
			case BL_LILAC_RDD_MAC_FWD_ACTION_RATE_LIMIT :
				bdmf_session_print ( session, "%-9s ", "RATE_LIM" );
				break;
			default :
                bdmf_session_print ( session, "%-9s ", "INVALID" );
                break;
			}

            for ( index = 0; index < 6; index++ )
            {
                bdmf_session_print ( session, "%-2x ", mac_addr.b[ index ] );
            }

            bdmf_session_print ( session, "  %-7s ", ( aging != 0 ) ? "YES" : "NO"  );
            bdmf_session_print ( session, "%-6s ", ( skip != 0 ) ? "YES" : "NO"  );
            bdmf_session_print ( session, "%-7s", ( valid != 0 ) ? "YES" : "NO"  );
			bdmf_session_print ( session, "%-7s ", ( mac_index >= 1024 ) ? "YES" : "NO" );
            bdmf_session_print ( session, "%-7s", ( aggregation_flag != 0 ) ? "ON" : "OFF"  );
            bdmf_session_print ( session, "0x%02x ", extension );
            bdmf_session_print ( session, "%s\n", ( entry_type == BL_LILAC_RDD_STATIC_MAC_ADDRESS ) ? "STATIC" : "BRIDGE" );

            if ( skip == 0 )
            {
                counter++;
            }
        }
    }

    bdmf_session_print ( session, "\nTotal number of entries: %d\n", ( int )counter );

    return ( rdd_error );
}


static void p_lilac_rdd_print_iptv_l3_entry ( bdmf_session_handle      session,
                                              uint32_t                 xi_entry_index,
                                              RDD_IPTV_ENTRY_UNION     *xi_iptv_entry,
                                              rdpa_iptv_lookup_method  xi_classification_mode )
{
    bdmf_session_print ( session, " %-7u ", ( unsigned )xi_entry_index );

    bdmf_session_print ( session, "MC: 0x%X ", xi_iptv_entry->l3_entry_fields.egress_port_vector );

    bdmf_session_print ( session, "%-3u.", ( xi_iptv_entry->l3_entry_fields.src_ip.addr.ipv4 >> 24 ) & 0xFF );
    bdmf_session_print ( session, "%-3u.", ( xi_iptv_entry->l3_entry_fields.src_ip.addr.ipv4 >> 16 ) & 0xFF );
    bdmf_session_print ( session, "%-3u.", ( xi_iptv_entry->l3_entry_fields.src_ip.addr.ipv4 >>  8 ) & 0xFF );
    bdmf_session_print ( session, "%-3u", ( xi_iptv_entry->l3_entry_fields.src_ip.addr.ipv4 >>  0 ) & 0xFF );

    bdmf_session_print ( session, " " );

    bdmf_session_print ( session, "%-3u.", ( xi_iptv_entry->l3_entry_fields.dst_ip.addr.ipv4 >> 24 ) & 0xFF );
    bdmf_session_print ( session, "%-3u.", ( xi_iptv_entry->l3_entry_fields.dst_ip.addr.ipv4 >> 16 ) & 0xFF );
    bdmf_session_print ( session, "%-3u.", ( xi_iptv_entry->l3_entry_fields.dst_ip.addr.ipv4 >>  8 ) & 0xFF );
    bdmf_session_print ( session, "%-3u", ( xi_iptv_entry->l3_entry_fields.dst_ip.addr.ipv4 >>  0 ) & 0xFF );

    if ( xi_classification_mode == iptv_lookup_method_group_ip_src_ip_vid )
    {
         bdmf_session_print ( session, "0x%04x\n", xi_iptv_entry->l3_entry_fields.vid );
    }

    bdmf_session_print ( session, " " );

    bdmf_session_print ( session, "  %-7s ", ( xi_iptv_entry->l3_entry_fields.src_ip.addr.ipv4 == 0 ) ? "YES" : "NO" );
    bdmf_session_print ( session, "0x%04x ", xi_iptv_entry->l3_entry_fields.wlan_mcast_index );
    bdmf_session_print ( session, "0x%-17x\n", xi_iptv_entry->l3_entry_fields.ingress_classification_context );
}


static int p_lilac_rdd_print_iptv_table ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_IPTV_ENTRY_UNION                   iptv_entry;
    rdpa_iptv_lookup_method                classification_mode;
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS  *bridge_cfg_register;
    BL_LILAC_RDD_ERROR_DTE                 rdd_error;
    uint8_t                                layer_flag;
    uint32_t                               entry_index;
    uint32_t                               base_index;
    uint32_t                               provider;
    uint32_t                               i;
    uint32_t                               counter = 0;

    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );

    RDD_BRIDGE_CONFIGURATION_REGISTER_IPTV_CLASSIFICATION_METHOD_READ ( classification_mode, bridge_cfg_register );

    layer_flag = ( int )parm[ 0 ].value.unumber;

    if ( layer_flag == 0 )
    {
        bdmf_session_print ( session, " IPTV L2 Table\n" );
        bdmf_session_print ( session, "---------------\n" );
        bdmf_session_print ( session, " index | egress port | vlan id |   mac address   | SSID | IC context index\n" );

        if ( ( classification_mode != iptv_lookup_method_mac ) && ( classification_mode != iptv_lookup_method_mac_vid ) )
        {
            return ( BDMF_ERR_PARM );
        }

        for ( entry_index = 0; entry_index < RDD_IPTV_DDR_LOOKUP_TABLE_SIZE; entry_index++ )
        {
            rdd_error = rdd_iptv_entry_get ( entry_index, &iptv_entry );

            if ( rdd_error == BL_LILAC_RDD_OK )
            {
                bdmf_session_print ( session, " %-7u ", ( unsigned )entry_index );

                bdmf_session_print ( session, "MC: 0x%X ", iptv_entry.l2_entry_fields.egress_port_vector );

                bdmf_session_print ( session, "0x%-7x", ( unsigned )iptv_entry.l2_entry_fields.vid );

                for ( i = 0; i < 6; i++ )
                {
                    bdmf_session_print ( session, "%02x ", iptv_entry.l2_entry_fields.mac_addr.b[ i ] );
                }

                bdmf_session_print ( session, "0x%-6x", iptv_entry.l2_entry_fields.wlan_mcast_index );
                bdmf_session_print ( session, "0x%-17x\n", iptv_entry.l2_entry_fields.ingress_classification_context );

                counter++;
            }
        }
    }
    else
    {
        bdmf_session_print ( session, " IPTV L3 Table\n" );
        bdmf_session_print ( session, "---------------\n" );

        if ( ( classification_mode == iptv_lookup_method_group_ip_src_ip ) || ( classification_mode == iptv_lookup_method_group_ip ) )
        {
            bdmf_session_print ( session, " index | egress port |   source ip   |  destination ip  | any | ssid | IC context index\n" );
        }
        else if ( ( classification_mode == iptv_lookup_method_group_ip_src_ip_vid ) )
        {
            bdmf_session_print ( session, " index | egress port |   source ip   |  destination ip  | vlan id | any | ssid | IC context index\n" );
        }
        else
        {
            return ( BDMF_ERR_PARM );
        }

        for ( entry_index = 0; entry_index < RDD_IPTV_DDR_LOOKUP_TABLE_SIZE; entry_index++ )
        {
            rdd_error = rdd_iptv_entry_get ( entry_index, &iptv_entry );

            if ( rdd_error == BL_LILAC_RDD_OK )
            {
                p_lilac_rdd_print_iptv_l3_entry ( session, entry_index, &iptv_entry, classification_mode );
                counter++;
            }
        }

        if ( ( classification_mode == iptv_lookup_method_group_ip_src_ip ) || ( classification_mode == iptv_lookup_method_group_ip_src_ip_vid ) )
        {
            for ( base_index = 0; base_index < RDD_IPTV_DDR_LOOKUP_TABLE_SIZE; base_index++ )
            {
                for ( provider = 0; provider < LILAC_RDD_IPTV_NUMBER_OF_SERVICE_PROVIDERS; provider++ )
                {
                    entry_index = SSM_ENTRY_INDEX ( base_index, provider );

                    rdd_error = rdd_iptv_entry_get ( entry_index, &iptv_entry );

                    if ( rdd_error == BL_LILAC_RDD_OK )
                    {
                        p_lilac_rdd_print_iptv_l3_entry ( session, entry_index, &iptv_entry, classification_mode );
                        counter++;
                    }
                }
            }
        }
    }

    bdmf_session_print ( session, "\nTotal number of entries: %d\n", ( int )counter );

    return ( 0 );
}


static int p_lilac_rdd_print_upstream_pbits_to_qos ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_US_PBITS_TO_QOS_TABLE_DTS  *us_pbits_to_qos_table_ptr;
    RDD_US_QUEUE_ENTRY_DTS         *us_queue_entry_ptr;
    uint8_t                        wan_mapping_table_index;
    uint8_t                        pbits_index;
    uint8_t                        queue;
    uint8_t                        rate_controller;

    us_pbits_to_qos_table_ptr = RDD_US_PBITS_TO_QOS_TABLE_PTR();

    for ( wan_mapping_table_index = 0; wan_mapping_table_index < RDD_US_PBITS_TO_QOS_TABLE_SIZE; wan_mapping_table_index++ )
    {
        bdmf_session_print ( session, "  Table Index - %d   \n", wan_mapping_table_index );
        bdmf_session_print ( session, "---------------------\n");
        bdmf_session_print ( session, " P-bits | Queue | RC \n");
    
        for ( pbits_index = 0; pbits_index < RDD_US_PBITS_TO_QOS_TABLE_SIZE2; pbits_index++ )
        {
            us_queue_entry_ptr = &( us_pbits_to_qos_table_ptr->entry[ wan_mapping_table_index ][ pbits_index ] );

            RDD_US_QUEUE_ENTRY_QUEUE_READ ( queue, us_queue_entry_ptr );
            RDD_US_QUEUE_ENTRY_RATE_CONTROLLER_READ ( rate_controller, us_queue_entry_ptr );

            bdmf_session_print ( session, "   %2d   |   %d   | %2d \n", pbits_index, queue, rate_controller );
        }

        bdmf_session_print ( session, "\n" );
    }

    bdmf_session_print ( session, "\n" );

    return ( 0 );
}


static int p_lilac_rdd_print_downstream_pbits_to_qos ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_DS_PBITS_TO_QOS_TABLE_DTS  *ds_pbits_to_qos_table_ptr;
    RDD_PBITS_TO_QOS_ENTRY_DTS     *pbits_to_qos_entry_ptr;
    uint32_t                       table_index;
    uint32_t                       pbits_index;
    uint8_t                        qos;

    ds_pbits_to_qos_table_ptr = ( RDD_DS_PBITS_TO_QOS_TABLE_DTS * )(DEVICE_ADDRESS ( RUNNER_COMMON_0_OFFSET ) + DS_PBITS_TO_QOS_TABLE_ADDRESS );

    for ( table_index = 0; table_index < RDD_DS_PBITS_TO_QOS_TABLE_SIZE; table_index++ )
    {
        bdmf_session_print ( session, "   Table - %d   \n", ( int )table_index );
        bdmf_session_print ( session, "----------------\n");
        bdmf_session_print ( session, " P-bits |  QOS  \n");

        for ( pbits_index = 0; pbits_index < RDD_DS_PBITS_TO_QOS_TABLE_SIZE2; pbits_index++ )
        {
            pbits_to_qos_entry_ptr = ( RDD_PBITS_TO_QOS_ENTRY_DTS * )( &( ds_pbits_to_qos_table_ptr->entry[ table_index ][ pbits_index ] ) );

            RDD_PBITS_TO_QOS_ENTRY_QOS_READ ( qos, pbits_to_qos_entry_ptr );

            bdmf_session_print ( session, "   %2d   |   %2d   \n", ( int )pbits_index, qos );
        }

        bdmf_session_print ( session, "\n" );
    }

    return ( 0 );
}


static int p_lilac_rdd_print_us_connections_table ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_CONNECTION_TABLE_DTS  *lookup_table_ptr;
    int                       start_index;
    int                       number_of_connections;
    int                       index;

    start_index = ( int )parm[ 0 ].value.unumber;
    number_of_connections = ( int )parm[ 1 ].value.unumber;

    lookup_table_ptr = ( RDD_CONNECTION_TABLE_DTS * )UsConnectionTableBase;

    bdmf_session_print ( session, " Connections Table - Upstream\n" );
    bdmf_session_print ( session, "-----------------------------\n" );

    bdmf_session_print ( session, " index | valid | cntxt ind. | protocol | src. port | dest. port | src. ip         | dest. ip\n" );

    for ( index = start_index; index < ( start_index + number_of_connections ); index++ )
    {
        if ( index >= RDD_CONNECTION_TABLE_SIZE )
        {
            break;
        }

        f_lilac_rdd_print_us_connection_helper ( session, lookup_table_ptr, index, 1 );
    }

    return ( 0 );
}


static int p_lilac_rdd_print_ds_connections_table ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_CONNECTION_TABLE_DTS  *lookup_table_ptr;
    int                       start_index;
    int                       number_of_connections;
    int                       index;

    start_index = ( int )parm[ 0 ].value.unumber;
    number_of_connections = ( int )parm[ 1 ].value.unumber;

    lookup_table_ptr = ( RDD_CONNECTION_TABLE_DTS * )DsConnectionTableBase;

    bdmf_session_print ( session, " Connections Table - Downstream\n" );
    bdmf_session_print ( session, "-------------------------------\n" );

    bdmf_session_print ( session, " index | valid | cntxt ind. | protocol | src. port | dest. port |   src. ip   | dest. ip\n" );

    for ( index = start_index; index < ( start_index + number_of_connections ); index++ )
    {
        if ( index >= RDD_CONNECTION_TABLE_SIZE )
        {
            break;
        }

        f_lilac_rdd_print_ds_connection_helper ( session, lookup_table_ptr, index, 1 );
    }

    return ( 0 );
}


static int p_lilac_rdd_print_ds_connection ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_CONNECTION_TABLE_DTS  *lookup_table_ptr;
    int                       index;
    int                       context_index;

    index = ( int )parm[ 0 ].value.unumber;

    lookup_table_ptr = ( RDD_CONNECTION_TABLE_DTS * )DsConnectionTableBase;
    bdmf_session_print ( session, " Connection %6d (Downstream)\n", index );
    bdmf_session_print ( session, "-----------------------------------\n" );

    context_index = f_lilac_rdd_print_ds_connection_helper ( session, lookup_table_ptr, index, 0 );

    if( context_index >= 0 )
    {
        f_lilac_rdd_print_fc_context_helper ( session, context_index );
    }
    else
    {
        bdmf_session_print ( session, "Connection not found!\n" );
    }

    return ( 0 );
}


static int p_lilac_rdd_print_us_connection ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_CONNECTION_TABLE_DTS  *lookup_table_ptr;
    int                       index;
    int                       context_index;

    index = ( int )parm[ 0 ].value.unumber;

    lookup_table_ptr = ( RDD_CONNECTION_TABLE_DTS * )UsConnectionTableBase;
    bdmf_session_print ( session, " Connection %6d (Upstream)\n", index );
    bdmf_session_print ( session, "---------------------------------\n" );

    context_index = f_lilac_rdd_print_us_connection_helper ( session, lookup_table_ptr, index, 0 );

    if( context_index >= 0 )
    {
        f_lilac_rdd_print_fc_context_helper ( session, context_index );
    }
    else
    {
        bdmf_session_print ( session, "Connection not found!\n" );
    }

    return ( 0 );
}


static int p_lilac_rdd_print_context ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    uint32_t  context_index;
    uint32_t  direction;

    context_index = ( unsigned int )parm[ 0 ].value.unumber;
    direction = ( unsigned int )parm[ 1 ].value.unumber;

    /* Validate parameters */
    if ( ( context_index >= ( 1 << 14 ) ) ||
         ( direction > 1 ) )
    {
        bdmf_session_print ( session, "UT: Invalid parameters\n\n\r" );
        return ( BDMF_ERR_PARM );
    }

    bdmf_session_print ( session, " Context %d\n\r", ( int )context_index );
    bdmf_session_print ( session, "-------------------------\n\r" );

    f_lilac_rdd_print_fc_context_helper ( session, context_index );

    return ( 0 );
}


static int f_lilac_rdd_print_ds_connection_helper ( bdmf_session_handle session, RDD_CONNECTION_TABLE_DTS  *lookup_table_ptr, int  index, int  table_format_flag )
{
    RDD_CONNECTION_ENTRY_DTS  *lookup_entry_ptr;
    uint8_t                   valid;
    uint16_t                  context_index;
    uint8_t                   protocol;
    uint16_t                  src_port;
    uint16_t                  dst_port;
    bdmf_ip_t                 src_ip;
    bdmf_ip_t                 dst_ip;

    lookup_entry_ptr = ( RDD_CONNECTION_ENTRY_DTS * )( &( lookup_table_ptr->entry[ index ] ) );

    RDD_CONNECTION_ENTRY_VALID_READ ( valid, lookup_entry_ptr );
    RDD_CONNECTION_ENTRY_CONTEXT_INDEX_READ ( context_index, lookup_entry_ptr );
    RDD_CONNECTION_ENTRY_PROTOCOL_READ ( protocol, lookup_entry_ptr );
    RDD_CONNECTION_ENTRY_SRC_PORT_READ ( src_port, lookup_entry_ptr );
    RDD_CONNECTION_ENTRY_DST_PORT_READ ( dst_port, lookup_entry_ptr );
    RDD_CONNECTION_ENTRY_SRC_IP_READ ( src_ip.addr.ipv4, lookup_entry_ptr );
    RDD_CONNECTION_ENTRY_DST_IP_READ ( dst_ip.addr.ipv4, lookup_entry_ptr );

    if ( valid != 0 )
    {
        if ( table_format_flag == 0 )
        {	
            bdmf_session_print( session, "Valid:                   %6d\n", ( int ) valid );
            bdmf_session_print( session, "Context index:           %6d\n", ( int ) context_index );
            bdmf_session_print( session, "Protocol:                %6d\n", ( int ) protocol );
            bdmf_session_print( session, "Source port:             %6d\n", ( int ) src_port );
            bdmf_session_print( session, "Destination port:        %6d\n", ( int ) dst_port );
            bdmf_session_print( session, "Source IP:               %-3d.%-3d.%-3d.%-3d\n", ( int )( ( src_ip.addr.ipv4 >> 24 ) & 0xFF ), ( int )( ( src_ip.addr.ipv4 >> 16 ) & 0xFF ),
                                ( int )( ( src_ip.addr.ipv4 >> 8 ) & 0xFF), ( int )( src_ip.addr.ipv4 & 0xFF ) );
            bdmf_session_print( session, "Destination IP:          %-3d.%-3d.%-3d.%-3d\n", ( int )( ( dst_ip.addr.ipv4 >> 24 ) & 0xFF ), ( int )( ( dst_ip.addr.ipv4 >> 16 ) & 0xFF ),
								( int )( ( dst_ip.addr.ipv4 >> 8 ) & 0xFF), ( int )( dst_ip.addr.ipv4 & 0xFF ) );
        }
        else
        {
            bdmf_session_print( session, " %-6d ", ( int )index );
            bdmf_session_print( session, " %-6d ", ( int )valid );
            bdmf_session_print( session, " %-11d ", ( int )context_index );
            bdmf_session_print( session, " %-9d ", ( int )protocol );
            bdmf_session_print( session, " %-10d ", ( int )src_port );
            bdmf_session_print( session, " %-11d ", ( int )dst_port );
            bdmf_session_print( session, " %-3d.%-3d.%-3d.%-3d", ( int )( ( src_ip.addr.ipv4 >> 24 ) & 0xFF ), ( int )( ( src_ip.addr.ipv4 >> 16 ) & 0xFF ),
																 ( int )( ( src_ip.addr.ipv4 >> 8 ) & 0xFF), ( int )( src_ip.addr.ipv4 & 0xFF ) );
            bdmf_session_print( session, " %-3d.%-3d.%-3d.%-3d\n", ( int )( ( dst_ip.addr.ipv4 >> 24 ) & 0xFF ), ( int )( ( dst_ip.addr.ipv4 >> 16 ) & 0xFF ),
																   ( int )( ( dst_ip.addr.ipv4 >> 8 ) & 0xFF), ( int )( dst_ip.addr.ipv4 & 0xFF ) );
        }

        return ( context_index );
    }
    else
    {
        return ( -1 );
    }
}


static int f_lilac_rdd_print_us_connection_helper ( bdmf_session_handle session, RDD_CONNECTION_TABLE_DTS  *lookup_table_ptr, int  index, int  table_format_flag )
{
    RDD_CONNECTION_ENTRY_DTS  *lookup_entry_ptr;
    uint8_t                   valid;
    uint16_t                  context_index;
    uint8_t                   protocol;
    uint16_t                  src_port;
    uint16_t                  dst_port;
    bdmf_ip_t                 src_ip;
    bdmf_ip_t                 dst_ip;

    lookup_entry_ptr = ( RDD_CONNECTION_ENTRY_DTS * )( &( lookup_table_ptr->entry[ index ] ) );

    RDD_CONNECTION_ENTRY_VALID_READ ( valid, lookup_entry_ptr );
    RDD_CONNECTION_ENTRY_CONTEXT_INDEX_READ ( context_index, lookup_entry_ptr );
    RDD_CONNECTION_ENTRY_PROTOCOL_READ ( protocol, lookup_entry_ptr );
    RDD_CONNECTION_ENTRY_SRC_PORT_READ ( src_port, lookup_entry_ptr );
    RDD_CONNECTION_ENTRY_DST_PORT_READ ( dst_port, lookup_entry_ptr );
    RDD_CONNECTION_ENTRY_SRC_IP_READ ( src_ip.addr.ipv4, lookup_entry_ptr );
    RDD_CONNECTION_ENTRY_DST_IP_READ ( dst_ip.addr.ipv4, lookup_entry_ptr );

    if ( valid != 0 )
    {
        if ( table_format_flag == 0 )
        {
            bdmf_session_print ( session, "Valid:                   %6d\n", ( int )valid );
            bdmf_session_print ( session, "Context index:           %6d\n", ( int )context_index );
            bdmf_session_print ( session, "Protocol:                %6d\n", ( int )protocol );
            bdmf_session_print ( session, "Source port:             %6d\n", ( int )src_port );
            bdmf_session_print ( session, "Destination port:        %6d\n", ( int )dst_port );
            bdmf_session_print ( session, "Source IP:               %-3d.%-3d.%-3d.%-3d\n", ( int )( ( src_ip.addr.ipv4 >> 24 ) & 0xFF ), ( int )( ( src_ip.addr.ipv4 >> 16 ) & 0xFF ),
																							( int )( ( src_ip.addr.ipv4 >> 8 ) & 0xFF), ( int )( src_ip.addr.ipv4 & 0xFF ) );
            bdmf_session_print ( session, "Destination IP:          %-3d.%-3d.%-3d.%-3d\n", ( int )( ( dst_ip.addr.ipv4 >> 24 ) & 0xFF ), ( int )( ( dst_ip.addr.ipv4 >> 16 ) & 0xFF ),
																							( int )( ( dst_ip.addr.ipv4 >> 8 ) & 0xFF), ( int )( dst_ip.addr.ipv4 & 0xFF ) );
        }
        else
        {
            bdmf_session_print ( session, " %-6d ", ( int )index );
            bdmf_session_print ( session, " %-6d ", ( int )valid );
            bdmf_session_print ( session, " %-11d ", ( int )context_index );
            bdmf_session_print ( session, " %-9d ", ( int )protocol );
            bdmf_session_print ( session, " %-10d ", ( int )src_port );
            bdmf_session_print ( session, " %-11d ", ( int )dst_port );
            bdmf_session_print ( session, " %-3d.%-3d.%-3d.%-3d", ( int )( ( src_ip.addr.ipv4 >> 24 ) & 0xFF ), ( int )( ( src_ip.addr.ipv4 >> 16 ) & 0xFF ),
																  ( int )( ( src_ip.addr.ipv4 >> 8 ) & 0xFF), ( int )( src_ip.addr.ipv4 & 0xFF ) );
            bdmf_session_print ( session, " %-3d.%-3d.%-3d.%-3d\n", ( int )( ( dst_ip.addr.ipv4 >> 24 ) & 0xFF ), ( int )( ( dst_ip.addr.ipv4 >> 16 ) & 0xFF ),
																	( int )( ( dst_ip.addr.ipv4 >> 8 ) & 0xFF), ( int )( dst_ip.addr.ipv4 & 0xFF ) );
        }
		
        return ( context_index );
    }
    else
    {
        return ( -1 );
    }
}

void _rdd_fc_context_read_from_table_entry(rdd_fc_context_t *ctx, RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS *entry);
static int f_lilac_rdd_print_fc_context_helper (bdmf_session_handle session, int index)
{	
    rdd_fc_context_t context;
    RDD_CONTEXT_TABLE_DTS *context_table_ptr;
    RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS *context_entry_ptr;
    uint32_t i, ip_checksum_delta, l4_checksum_delta;
    uint8_t overflow;

    context_table_ptr = (RDD_CONTEXT_TABLE_DTS *)ContextTableBase;
    context_entry_ptr = (RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS *)(&(context_table_ptr->entry[index]));

    _rdd_fc_context_read_from_table_entry(&context, context_entry_ptr);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_IP_CHECKSUM_DELTA_READ(ip_checksum_delta, context_entry_ptr);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_LAYER4_CHECKSUM_DELTA_READ(l4_checksum_delta, context_entry_ptr);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_OVERFLOW_READ(overflow, context_entry_ptr);

    bdmf_session_print(session, "Valid packets counter:   %6d\n", (int)context.valid_cnt.packets);
    bdmf_session_print(session, "Forward action:          %6d\n", (int)context.fwd_action);
    bdmf_session_print(session, "CPU reason:              %6d\n", (int)context.trap_reason);
    bdmf_session_print(session, "DSCP value:              %6d\n", (int)context.dscp_value);
    bdmf_session_print(session, "ECN value:               %6d\n", (int)context.ecn_value);
    bdmf_session_print(session, "NAT port:                %6d\n", (int)context.nat_port);
    bdmf_session_print(session, "IP checksum delta:       %6d\n", (int)ip_checksum_delta);
    bdmf_session_print(session, "L4 checksum delta:       %6d\n", (int)l4_checksum_delta);
    bdmf_session_print(session, "NAT IP:                  %pI4\n", &context.nat_ip.addr.ipv4);
    bdmf_session_print(session, "Egress port:             %6d\n", (int)context.egress_port);
    bdmf_session_print(session, "Wifi ssid:               %6d\n", (int)context.wifi_ssid);
    bdmf_session_print(session, "WL metadata:             %6d\n", (int)context.wl_metadata);
    bdmf_session_print(session, "IP version:              %6s\n", ((context.ip_version == 0) ? "IPv4" : "IPv6"));
    bdmf_session_print(session, "Traffic class:           %6d\n", (int)context.traffic_class);
    bdmf_session_print(session, "WAN flow index:          %6d\n", (int)context.wan_flow_index);
    bdmf_session_print(session, "Rate controller:         %6d\n", (int)context.rate_controller);
    bdmf_session_print(session, "DS lite header index:    %6d\n", (int)context.ds_lite_hdr_index);
    bdmf_session_print(session, "Outer vid offset:        %6d\n", (int)context.ovid_offset);
    bdmf_session_print(session, "Policer ID:              %6d\n", (int)context.policer_id);
    bdmf_session_print(session, "Overflow:                %6d\n", (int)overflow);
    bdmf_session_print(session, "Outer pbit remap:        %6d\n", (int)context.opbit_action);
    bdmf_session_print(session, "Inner pbit remap:        %6d\n", (int)context.ipbit_action);
    bdmf_session_print(session, "L2 tags number:          %6d\n", (int)context.l2_hdr_number_of_tags);
    bdmf_session_print(session, "L2 offset:               %6d\n", (int)context.l2_hdr_offset);
    bdmf_session_print(session, "L2 size:                 %6d\n", (int)context.l2_hdr_size);

    if (context.service_queue_enabled) 
        bdmf_session_print(session, "Service queue:           %6d\n", (int)context.service_queue_id);
    else
        bdmf_session_print(session, "Service queue mode:      Disabled\n");

    bdmf_session_print(session, "Actions vector:          ");

    if (context.actions_vector != 0)
    {
        if (context.actions_vector & (1 << rdpa_fc_act_forward))
            bdmf_session_print(session, "forward ");

        if (context.actions_vector & (1 << rdpa_fc_act_ttl))
            bdmf_session_print(session, "| ttl ");

        if (context.actions_vector & (1 << rdpa_fc_act_policer))
            bdmf_session_print(session, "| policer ");

        if (context.actions_vector & (1 << rdpa_fc_act_dscp_remark))
            bdmf_session_print(session, "| dscp ");

        if (context.actions_vector & (1 << rdpa_fc_act_nat))
            bdmf_session_print(session, "| nat ");

        if (context.actions_vector & (1 << rdpa_fc_act_gre_remark))
            bdmf_session_print(session, "| gre ");

        if (context.actions_vector & (1 << rdpa_fc_act_opbit_remark))
            bdmf_session_print(session, "| opbit ");

        if (context.actions_vector & (1 << rdpa_fc_act_ipbit_remark))
            bdmf_session_print(session, "| ipbit ");

        if (context.actions_vector & (1 << rdpa_fc_act_dslite_tunnel))
            bdmf_session_print(session, "| dslite_tunnel ");
        if (context.actions_vector & (1 << rdpa_fc_act_gre_tunnel))
            bdmf_session_print(session, "| l2gre_tunnel ");
        if (context.actions_vector & (1 << rdpa_fc_act_pppoe))
            bdmf_session_print(session, "| pppoe ");

        bdmf_session_print(session, "\n");
    }
    else
    {
        bdmf_session_print(session, "0\n");
    }

    bdmf_session_print(session, "Connection direction:    %6s\n", ((context.conn_dir == 0) ? "DS" : "US"));
    bdmf_session_print(session, "Connection table index:  %6d\n", (int)context.conn_index);

    bdmf_session_print(session, "Layer 2 header:          ");

    for (i = 0; i < context.l2_hdr_size; i++)
        bdmf_session_print(session, "0x%02x ", (unsigned char)context.l2_header[i]);

    bdmf_session_print(session, "Drop eligibility flow:    %6d\n", (int)context.drop_eligibility);

    bdmf_session_print(session, "\n");

    return context.conn_index;
}

static int p_lilac_rdd_print_ds_context_entry_number ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    return p_lilac_rdd_print_context_entry_number_helper ( session, parm, 0 );
}


static int p_lilac_rdd_print_us_context_entry_number ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    return p_lilac_rdd_print_context_entry_number_helper ( session, parm, 1 );
}


static int p_lilac_rdd_print_context_entry_number_helper ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], int us_flag )
{
    RDD_CONNECTION_TABLE_DTS  *lookup_table_ptr;
    RDD_CONNECTION_ENTRY_DTS  *lookup_entry_ptr;
    int                       index;
    uint16_t                  context_index;
    uint8_t                   valid;
    uint8_t                   protocol;
    uint16_t                  src_port;
    uint16_t                  dst_port;
    bdmf_ip_t                 src_ip;
    bdmf_ip_t                 dst_ip;


    if ( us_flag == 0 )
    {
        lookup_table_ptr = ( RDD_CONNECTION_TABLE_DTS * )DsConnectionTableBase;
    }
    else
    {
        lookup_table_ptr = ( RDD_CONNECTION_TABLE_DTS * )UsConnectionTableBase;
    }

    for ( index = 0; index < RDD_CONNECTION_TABLE_SIZE; index++ )
    {
        lookup_entry_ptr = ( RDD_CONNECTION_ENTRY_DTS * )( &( lookup_table_ptr->entry[ index ] ) );

        RDD_CONNECTION_ENTRY_VALID_READ ( valid, lookup_entry_ptr );
        RDD_CONNECTION_ENTRY_CONTEXT_INDEX_READ ( context_index, lookup_entry_ptr );
        RDD_CONNECTION_ENTRY_PROTOCOL_READ ( protocol, lookup_entry_ptr );
        RDD_CONNECTION_ENTRY_SRC_PORT_READ ( src_port, lookup_entry_ptr );
        RDD_CONNECTION_ENTRY_DST_PORT_READ ( dst_port, lookup_entry_ptr );
        RDD_CONNECTION_ENTRY_SRC_IP_READ ( src_ip.addr.ipv4, lookup_entry_ptr );
        RDD_CONNECTION_ENTRY_DST_IP_READ ( dst_ip.addr.ipv4, lookup_entry_ptr );

        if ( ( valid == 1 ) && ( ( ( uint8_t )( parm[ 0 ].value.unumber ) ) == protocol ) &&
             ( ( ( uint16_t )( parm[ 1 ].value.unumber ) ) == src_port ) &&
             ( ( ( uint16_t )( parm[ 2 ].value.unumber ) ) == dst_port ) &&
             ( ( ( uint32_t )( parm[ 3 ].value.unumber ) ) == src_ip.addr.ipv4 ) &&
             ( ( ( uint32_t )( parm[ 4 ].value.unumber ) ) == dst_ip.addr.ipv4 ) )
        {
            bdmf_session_print ( session, " 5-Tuple found in connections table!\n Context index number is: %d\n", ( int )context_index );
            return ( 0 );
        }
    }

    bdmf_session_print ( session, " 5-Tuple not found\n" );

    return ( 0 );
}


static int p_lilac_rdd_print_connections_number ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_CONNECTION_TABLE_DTS  *lookup_table_ptr;
    RDD_CONNECTION_ENTRY_DTS  *lookup_entry_ptr;
    rdpa_traffic_dir          direction;
    uint32_t                  index;
    uint32_t                  connection_counter;
    uint8_t                   valid;

    direction = ( uint32_t )parm[ 0 ].value.unumber;
    connection_counter = 0;

    if ( direction == rdpa_dir_ds )
    {
        lookup_table_ptr = ( RDD_CONNECTION_TABLE_DTS * )DsConnectionTableBase;
    }
    else
    {
        lookup_table_ptr = ( RDD_CONNECTION_TABLE_DTS * )UsConnectionTableBase;
    }

    for ( index = 0; index < RDD_CONNECTION_TABLE_SIZE; index++ )
    {
        lookup_entry_ptr = ( RDD_CONNECTION_ENTRY_DTS * )( &( lookup_table_ptr->entry[ index ] ) );

        RDD_CONNECTION_ENTRY_VALID_READ ( valid, lookup_entry_ptr );

        if ( valid == 1 )
        {
            connection_counter++;
        }
    }

    bdmf_session_print ( session, "Number of %s connections: %u\n\r", direction ? "US" : "DS", ( unsigned )connection_counter );

    return ( 0 );
}


static int p_lilac_rdd_1588_mode_config ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    uint32_t  mode_1588_enable;

    mode_1588_enable = ( uint32_t )parm[ 0 ].value.unumber;

    rdd_1588_mode_config ( mode_1588_enable );

    return ( 0 );
}


static int p_lilac_rdd_print_1588_time_stamp ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
#if 0 
    uint32_t  time_stamp_0;
    uint32_t  time_stamp_1;
    uint32_t  time_stamp_2;

    rdd_1588_time_stamp_get ( &time_stamp_0, &time_stamp_1, &time_stamp_2 );

    bdmf_session_print ( session, " 1588 timestamp \n" );
    bdmf_session_print ( session, "----------------\n" );
    bdmf_session_print ( session, "Timestamp 0: 0x%08x\n", ( unsigned int )time_stamp_0 );
    bdmf_session_print ( session, "Timestamp 1: 0x%08x\n", ( unsigned int )time_stamp_1 );
    bdmf_session_print ( session, "Timestamp 2: 0x%08x\n", ( unsigned int )time_stamp_2 );
#endif
    return ( 0 );
}


static int p_lilac_rdd_upstream_padding_config ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    BL_LILAC_RDD_UPSTREAM_PADDING_MODE_DTE  upstream_padding_enable;
    BL_LILAC_RDD_UPSTREAM_PADDING_MODE_DTE  upstream_padding_cpu_enable;
    uint16_t                                max_size;

    upstream_padding_enable = ( BL_LILAC_RDD_UPSTREAM_PADDING_MODE_DTE )parm[ 0 ].value.unumber;
    upstream_padding_cpu_enable = ( BL_LILAC_RDD_UPSTREAM_PADDING_MODE_DTE )parm[ 1 ].value.unumber;
    max_size = ( uint16_t )parm[ 2 ].value.unumber;

    rdd_us_padding_config ( upstream_padding_enable, upstream_padding_cpu_enable, max_size );

    return ( 0 );
}


static int p_lilac_rdd_print_interrupt_vector ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    uint32_t  interrupt_number;
    uint8_t   sub_interrupt_vector;
    uint8_t   sub_interrupt_mask;

    interrupt_number = ( uint32_t )parm[ 0 ].value.unumber;

    rdd_interrupt_vector_get ( interrupt_number, &sub_interrupt_vector );
    rdd_interrupt_mask_get ( interrupt_number, &sub_interrupt_mask );

    bdmf_session_print ( session, "Sub-interrupt vector:  0x%02x\n", sub_interrupt_vector );
    bdmf_session_print ( session, "Sub-interrupt mask:    0x%02x\n", sub_interrupt_mask );

    return ( 0 );
}


static int p_lilac_rdd_tcont_byte_counter_read ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint32_t                tcont_id;
    uint32_t                tcont_counter;

    tcont_id = parm[ 0 ].value.unumber;

    rdd_error = rdd_wan_channel_byte_counter_read ( tcont_id, &tcont_counter );

    if ( rdd_error == BL_LILAC_RDD_OK )
    {
        bdmf_session_print ( session, "TCONT %d counter: %d\n", ( int )tcont_id, ( int )tcont_counter );
    }
    else
    {
        bdmf_session_print ( session, "bl_lilac_rdd_tcont_byte_counter_read retured error\n" );
    }

    return ( 0 );
}


static int p_lilac_rdd_iptv_mac_counter_read ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_IPTV_ENTRY_UNION    iptv_entry;
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint16_t                iptv_counter;
    uint32_t                iptv_entry_index;

    memcpy ( iptv_entry.l2_entry_fields.mac_addr.b, parm[ 0 ].value.mac, 6 );

    iptv_entry.l2_entry_fields.vid = parm[ 1 ].value.unumber;

    rdd_error = rdd_iptv_entry_search ( &iptv_entry, &iptv_entry_index );

    rdd_error = rdd_iptv_counter_get ( iptv_entry_index, &iptv_counter );

    if ( rdd_error == BL_LILAC_RDD_OK )
    {
        bdmf_session_print ( session, "IPTV counter:  %u\n", iptv_counter );
    }
    else
    {
        bdmf_session_print ( session, "bl_lilac_rdd_iptv_mac_counter_read retured error\n" );
    }

    return ( 0 );
}


static int p_lilac_rdd_iptv_layer3_counter_read ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_IPTV_ENTRY_UNION    iptv_entry;
    BL_LILAC_RDD_ERROR_DTE  rdd_error;
    uint32_t                iptv_entry_index;
    uint16_t                iptv_layer3_counter;

    iptv_entry.l3_entry_fields.src_ip.addr.ipv4 = parm[ 0 ].value.unumber;
    iptv_entry.l3_entry_fields.dst_ip.addr.ipv4 = parm[ 1 ].value.unumber;

    rdd_error = rdd_iptv_entry_search ( &iptv_entry, &iptv_entry_index );

    if ( rdd_error == BL_LILAC_RDD_OK )
    {
        rdd_error = rdd_iptv_counter_get ( iptv_entry_index, &iptv_layer3_counter );

        if ( rdd_error == BL_LILAC_RDD_OK )
        {
            bdmf_session_print ( session, "IPTV layer3 counter: %u\n", iptv_layer3_counter );
	}
	else
        {
            bdmf_session_print ( session, "bl_lilac_rdd_iptv_mac_counter_read retured error\n" );
	}
    }
    else
    {
        bdmf_session_print ( session, "bl_lilac_rdd_iptv_mac_counter_read retured error\n" );
    }

    return ( 0 );
}


static int p_lilac_rdd_flow_pm_counters_get ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    BL_LILAC_RDD_FLOW_PM_COUNTERS_DTE  flow_counters;
    BL_LILAC_RDD_ERROR_DTE             rdd_error;
    uint32_t                           direction, flow_id;

    flow_id = parm[ 0 ].value.unumber;
	direction = parm[ 1 ].value.unumber;

    rdd_error = rdd_flow_pm_counters_get ( flow_id, direction, LILAC_RDD_FALSE, &flow_counters );

    if ( rdd_error == BL_LILAC_RDD_OK )
    {
        bdmf_session_print ( session, "Flow ID %d PM  counters:\n", ( int )flow_id );
        bdmf_session_print ( session, "==============================\n" );

        if ( direction != 2 )
        {
            bdmf_session_print ( session, "Good RX bytes:    %-6u\n", ( unsigned )flow_counters.good_rx_bytes );
            bdmf_session_print ( session, "Good RX packets:  %-6u\n", ( unsigned )flow_counters.good_rx_packet );
            bdmf_session_print ( session, "RX drops:         %-6u\n", ( unsigned )flow_counters.error_rx_packets_discard );
        }
        if ( direction != 1 )
        {
            bdmf_session_print ( session, "Good TX bytes:    %-6u\n", ( unsigned )flow_counters.good_tx_bytes );
            bdmf_session_print ( session, "Good TX packets:  %-6u\n", ( unsigned )flow_counters.good_tx_packet );
            bdmf_session_print ( session, "TX drops:         %-6u\n", ( unsigned )flow_counters.error_tx_packets_discard );
        }
    }
    else
    {
        bdmf_session_print ( session, "bl_lilac_rdd_flow_pm_counters_get returned error\n" );
    }

    return ( 0 );
}


static int p_lilac_rdd_bridge_port_pm_counters_get ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    BL_LILAC_RDD_BRIDGE_PORT_PM_COUNTERS_DTE  pm_counters;
    BL_LILAC_RDD_ERROR_DTE                    rdd_error;
    uint32_t                                  bridge_port;

    bridge_port = parm[ 0 ].value.unumber;

    rdd_error = rdd_bridge_port_pm_counters_get ( bridge_port, 0, &pm_counters );

    if ( rdd_error == BL_LILAC_RDD_OK )
    {
        bdmf_session_print ( session, "   Bridge port PM counters:\n" );
        bdmf_session_print ( session, "==============================\n" );
        bdmf_session_print ( session, "bridge valid rx packets:    %-6u\n", ( unsigned )pm_counters.rx_valid );
        bdmf_session_print ( session, "bridge valid tx packets:    %-6u\n", ( unsigned )pm_counters.tx_valid );
        bdmf_session_print ( session, "bridge filtered packets:    %-6u\n", ( unsigned )pm_counters.bridge_filtered_packets );
        bdmf_session_print ( session, "bridge tx packets discard:  %-6u\n", ( unsigned )pm_counters.bridge_tx_packets_discard );
        bdmf_session_print ( session, "error rx bpm congestion:    %-6u\n", ( unsigned )pm_counters.error_rx_bpm_congestion );
    }
    else
    {
        bdmf_session_print ( session, "bl_lilac_rdd_bridge_port_pm_counters_get returned error\n" );
    }

    return ( 0 );
}


static int p_lilac_rdd_service_queue_counters_get ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    BL_LILAC_RDD_ERROR_DTE rdd_error;
    rdpa_stat_1way_t       stat = {};
    uint32_t               index;

    index = parm[ 0 ].value.unumber;

    rdd_error = rdd_service_queue_status_get(index, &stat);

    if ( rdd_error == BL_LILAC_RDD_OK )
    {
        bdmf_session_print ( session, "   Service queue counters:\n" );
        bdmf_session_print ( session, "==============================\n" );
        bdmf_session_print ( session, "good rx packet:     %-6u\n", ( unsigned )stat.passed.packets );
        bdmf_session_print ( session, "rx dropped packet:  %-6u\n", ( unsigned )stat.discarded.packets );
    }
    else
    {
        bdmf_session_print ( session, "rdd_service_queue_status_get retured error\n" );
    }

    return ( 0 );
}


static int p_lilac_rdd_various_counters_get ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    BL_LILAC_RDD_VARIOUS_COUNTERS_DTE  various_counters;
    BL_LILAC_RDD_ERROR_DTE             rdd_error;
    uint32_t                           direction, index;
    static uint8_t ingress_filters_arr[][ 16 ] = { "IGMP", "ICMPV6", "USER_0", "USER_1", "USER_2", "USER_3", "PPPOE_D", "PPPOE_S", "ARP", "1588", "802.1X", "802.1AG_CFM", "BROADCAST", "MULTICAST" };

    static uint8_t l4_filters_arr[][ 16 ] = { "ERROR", "EXCEPTION", "IP_FIRST_FRAG", "IP_FRAGMENT", "ICMP", "ESP", "GRE", "AH", "IPv6", "USER_DEFINED_0", "USER_DEFINED_1", "USER_DEFINED_2",
                                              "USER_DEFINED_3", "UNKNOWN", "EMPTY", "EMPTY" };


    direction = parm[ 0 ].value.unumber;

    rdd_error = rdd_various_counters_get ( direction, 0xffffffff, 0, &various_counters );

    if ( rdd_error == BL_LILAC_RDD_OK )
    {
        bdmf_session_print ( session, "   %-s counters:\n",  ( direction == 0 ) ? "Downstream" : "Upstream" );
        bdmf_session_print ( session, "==============================\n" );

        if ( direction == rdpa_dir_us )
        {
            bdmf_session_print ( session, "ACL L2 drop:                                %-6u\n", various_counters.acl_l2_drop );
            bdmf_session_print ( session, "ACL L3 drop:                                %-6u\n", various_counters.acl_l3_drop );
            bdmf_session_print ( session, "ACL OUI drop:                               %-6u\n", various_counters.acl_oui_drop );
            bdmf_session_print ( session, "Local switching congestion:                 %-6u\n", various_counters.local_switching_congestion );
            bdmf_session_print ( session, "EPON DDR queue drop:                        %-6u\n", various_counters.us_ddr_queue_drop );
            bdmf_session_print ( session, "DHD IH congestion:                          %-6u\n", various_counters.dhd_ih_congestion_drop );
            bdmf_session_print ( session, "DHD malloc failure drop:                    %-6u\n", various_counters.dhd_malloc_failed_drop );
        }
        else
        {
            bdmf_session_print ( session, "dst mac non router drop:                    %-6u\n", various_counters.dst_mac_non_router_drop );
            bdmf_session_print ( session, "firewall drop:                              %-6u\n", various_counters.firewall_drop );
            bdmf_session_print ( session, "invalid layer2 protocol drop:               %-6u\n", various_counters.invalid_layer2_protocol_drop );
            bdmf_session_print ( session, "IPTV layer 3 drop:                          %-6u\n", various_counters.iptv_layer3_drop );
            bdmf_session_print ( session, "DS policers drop:                           %-6u\n", various_counters.downstream_policers_drop );
            bdmf_session_print ( session, "EMAC loopback drop:                         %-6u\n", various_counters.emac_loopback_drop );
            bdmf_session_print ( session, "Dual Stack Lite congestion drop:            %-6u\n", various_counters.dual_stack_lite_congestion_drop );
            bdmf_session_print ( session, "Absolute Address List Overflow drop:        %-6u\n", various_counters.absolute_address_list_overflow_drop );
            bdmf_session_print ( session, "WLAN Mcast copy failure drop:               %-6u\n", various_counters.wlan_mcast_copy_failed_drop );
            bdmf_session_print ( session, "WLAN Mcast overflow (=queue full) drop:     %-6u\n", various_counters.wlan_mcast_overflow_drop );
            bdmf_session_print ( session, "WLAN Mcast tx configuration error drop:     %-6u\n", various_counters.wlan_mcast_drop );
            bdmf_session_print ( session, "SBPM ALLOC reply NACK:                      %-6u\n", various_counters.sbpm_alloc_reply_nack );
        }
        bdmf_session_print ( session, "ETH flow action drop:                       %-6u\n", various_counters.eth_flow_action_drop );
        bdmf_session_print ( session, "SA lookup failure drop:                     %-6u\n", various_counters.sa_lookup_failure_drop );
        bdmf_session_print ( session, "DA lookup failure drop:                     %-6u\n", various_counters.da_lookup_failure_drop );
        bdmf_session_print ( session, "SA action drop:                             %-6u\n", various_counters.sa_action_drop );
        bdmf_session_print ( session, "DA action drop:                             %-6u\n", various_counters.da_action_drop );
        bdmf_session_print ( session, "Forw. matrix disabled drop:                 %-6u\n", various_counters.forwarding_matrix_disabled_drop );
        bdmf_session_print ( session, "Connection action drop:                     %-6u\n", various_counters.connection_action_drop );
        bdmf_session_print ( session, "VLAN switching drop:                        %-6u\n", various_counters.vlan_switching_drop );

        for ( index = 0; index < BL_LILAC_RDD_INGRESS_FILTERS_NUMBER; index++ )
        {
            bdmf_session_print ( session, "%-16s (ingress filter %-2u) drop:  %-6u\n", ingress_filters_arr[ index ], ( unsigned )index, ( unsigned )various_counters.ingress_filters_drop[ index ] );
        }

        for ( index = 0; index <= RDD_LAYER4_FILTER_UNKNOWN; index++ )
        {
            bdmf_session_print ( session, "%-16s (layer4 filter %-2u) drop:   %-6u\n", l4_filters_arr[ index ], ( unsigned )index, ( unsigned )various_counters.layer4_filters_drop[ index ] );
        }

        bdmf_session_print ( session, "Header error drop:                          %-6u\n", ( unsigned )various_counters.ip_validation_filter_drop[ 0 ] );
        bdmf_session_print ( session, "IP fragments drop:                          %-6u\n", ( unsigned )various_counters.ip_validation_filter_drop[ 1 ] );
        bdmf_session_print ( session, "TPID detect drop:                           %-6u\n", ( unsigned )various_counters.tpid_detect_drop );
        bdmf_session_print ( session, "Invalid subnet IP drop:                     %-6u\n", ( unsigned )various_counters.invalid_subnet_ip_drop );
    }
    else
    {
        bdmf_session_print ( session, "bl_lilac_rdd_various_counters_get returned error\n" );
    }

    return ( 0 );
}

static int p_lilac_rdd_skb_debug_counters_get ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    uint8_t   *free_indexes_local_fifo_tail_ptr;
    uint32_t  *free_indexes_ddr_fifo_tail_ptr;
    uint32_t  value;

    bdmf_session_print ( session, "Sent abs packets: %d\n", ( int )g_cpu_tx_sent_abs_packets_counter );
    bdmf_session_print ( session, "Released SKB's: %d\n", ( int )g_cpu_tx_released_skb_counter );
    bdmf_session_print ( session, "No free SKB errors: %d\n", ( int )g_cpu_tx_no_free_skb_counter );
    bdmf_session_print ( session, "Occupied SKB's: %d\n", ( int )( LILAC_RDD_CPU_TX_SKB_LIMIT_DEFAULT - g_cpu_tx_skb_free_indexes_counter) );
    bdmf_session_print ( session, "Current SW's free skb idx = %d\n", ( int )( g_cpu_tx_skb_free_indexes_release_ptr) );
    bdmf_session_print ( session, "SKB's limit: %d\n", ( int )( LILAC_RDD_CPU_TX_SKB_LIMIT_DEFAULT ) ) ;
    bdmf_session_print ( session, "SKB First DDR entry = 0x%x\n" ,( int )( g_free_skb_indexes_fifo_table_physical_address ) );
    bdmf_session_print ( session, "SKB Last DDR entry = 0x%x\n" ,( int )( g_free_skb_indexes_fifo_table_physical_address_last_idx ) );
#if defined (CONFIG_DHD_RUNNER)
    bdmf_session_print ( session, "DHD CPU TX free/threshold: %d/%d\n" , (int)g_cpu_tx_dhd_free_counter, (int)g_cpu_tx_dhd_threshold);
    bdmf_session_print ( session, "DHD CPU TX over threshold counter: %d\n" , (int)g_cpu_tx_dhd_over_threshold_counter);
#endif

    free_indexes_ddr_fifo_tail_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + FREE_SKB_INDEXES_DDR_FIFO_TAIL_ADDRESS );
    MREAD_32( free_indexes_ddr_fifo_tail_ptr, value );
    bdmf_session_print ( session, "Current FW's DDR tail free skb idx = 0x%x (actual idx num = %d)\n", ( int )( value ), ( ( ( uint32_t )value - ( uint32_t )g_free_skb_indexes_fifo_table_physical_address ) / 2 ) );

    free_indexes_local_fifo_tail_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_FAST_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_PTR_ADDRESS );
    MREAD_8( free_indexes_local_fifo_tail_ptr, value );
    bdmf_session_print ( session, "Current FW's DS FAST free skb idx = %d\n", ( int )( value ) );

    free_indexes_local_fifo_tail_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PICO_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_PTR_ADDRESS );
    MREAD_8( free_indexes_local_fifo_tail_ptr, value );
    bdmf_session_print ( session, "Current FW's DS PICO free skb idx = %d\n", ( int )( value ) );

    free_indexes_local_fifo_tail_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_FAST_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_PTR_ADDRESS );
    MREAD_8( free_indexes_local_fifo_tail_ptr, value );
    bdmf_session_print ( session, "Current FW's US FAST free skb idx = %d\n", ( int )( value ) );

    free_indexes_local_fifo_tail_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PICO_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_PTR_ADDRESS );
    MREAD_8( free_indexes_local_fifo_tail_ptr, value );
    bdmf_session_print ( session, "Current FW's US PICO free skb idx = %d\n", ( int )( value ) );

    return ( 0 );
}


static int p_lilac_rdd_parallel_processing_debug_counters_get ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    BL_LILAC_RDD_VARIOUS_COUNTERS_DTE  various_counters;
    BL_LILAC_RDD_ERROR_DTE             rdd_error;

    rdd_error = rdd_various_counters_get ( 0, 0xffffffff, 0, &various_counters );

    if ( rdd_error == BL_LILAC_RDD_OK )
    {
        bdmf_session_print ( session, "Parallel processing no available slave      %-6u\n", various_counters.ds_parallel_processing_no_avialable_slave );
        bdmf_session_print ( session, "Parallel processing reorder slaves:         %-6u\n", various_counters.ds_parallel_processing_reorder_slaves );
    }
    else
    {
        bdmf_session_print ( session, "bl_lilac_rdd_various_counters_get returned error\n" );
    }

    return ( 0 );
}

#ifdef G9991
static int p_lilac_rdd_g9991_flow_counters_get ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
	BL_LILAC_RDD_ERROR_DTE rdd_error;
	RDD_G9991_PM_FLOW_COUNTERS_DTE counters;
	int i = ( uint8_t )parm[ 0 ].value.unumber;

    bdmf_session_print ( session, "G9991 counters, flow %d:\n", i );
    bdmf_session_print ( session, "------------------------------\n" );

	rdd_error = rdd_g9991_pm_flow_counters_get (i, LILAC_RDD_TRUE, &counters);
	if (!rdd_error)
	{
		bdmf_session_print ( session, "rx packets %d\nrx multicast %d\nrx broadcast %d\nrx bytes %d\ntx packets %d\ntx multicast %d\ntx broadcast %d\ntx bytes %d\n",
		    counters.rx_packets, counters.rx_mcast_packets, counters.rx_bcast_packets, counters.rx_bytes, counters.tx_packets, counters.tx_mcast_packets, counters.tx_bcast_packets, counters.tx_bytes);
	}

    return ( rdd_error );
}

static int p_lilac_rdd_g9991_debug_counters_get ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_G9991_PM_COUNTERS_DTE g9991_counters;
    BL_LILAC_RDD_ERROR_DTE    rdd_error;

    rdd_error = rdd_g9991_counters_get ( &g9991_counters );

    if ( rdd_error == BL_LILAC_RDD_OK )
    {
        bdmf_session_print ( session, "G999.1 DFC Frame Errors:       %-6u\n", g9991_counters.dfc_frame_error_counter );
        bdmf_session_print ( session, "G999.1 DFC Frames:             %-6u\n", g9991_counters.dfc_frame_counter );
        bdmf_session_print ( session, "G999.1 Data Frame Errors:      %-6u\n", g9991_counters.data_frame_error_counter );
        bdmf_session_print ( session, "G999.1 Illegal SID Errors:     %-6u\n", g9991_counters.illegal_sid_error_counter );
        bdmf_session_print ( session, "G999.1 Length Errors:          %-6u\n", g9991_counters.length_error_counter );
        bdmf_session_print ( session, "G999.1 Reassembly Error:       %-6u\n", g9991_counters.reassembly_error_counter );
        bdmf_session_print ( session, "G999.1 BBH Errors:             %-6u\n", g9991_counters.bbh_error_counter );
        bdmf_session_print ( session, "G999.1 Consequent Drop Errors: %-6u\n", g9991_counters.consequent_drop );
    }
    else
    {
        bdmf_session_print ( session, "p_lilac_rdd_g9991_debug_counters_get returned error\n" );
    }

    return ( 0 );
}
#endif

static int p_lilac_rdd_print_ingress_classification_context ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS  ds_context = {0};
    RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS  us_context = {0};
    RDD_VLAN_COMMAND_INDEX_ENTRY_DTS                 vlan_entry;
    RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS  *ds_ingress_classification_context_table_ptr;
    RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS  *ds_ingress_classification_context_entry_ptr;
    RDD_US_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS  *us_ingress_classification_context_table_ptr;
    RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS  *us_ingress_classification_context_entry_ptr;
    RDD_VLAN_COMMAND_INDEX_TABLE_DTS                 *vlan_cmd_idx_table_ptr;
    RDD_VLAN_COMMAND_INDEX_ENTRY_DTS                 *vlan_cmd_idx_entry_ptr;
    uint32_t  context_index;
    uint32_t  direction;
    uint32_t                                         dei_remark_enable;
    uint32_t                                         dei_value;
#if !defined(FIRMWARE_INIT) && !defined(RDD_BASIC)
    BL_LILAC_RDD_ERROR_DTE                           rdd_error;
    uint16_t                                         counter;
#endif

    context_index = ( unsigned int )parm[ 0 ].value.unumber;
    direction = ( unsigned int )parm[ 1 ].value.unumber;

    if ( direction == rdpa_dir_ds )
    {
        ds_ingress_classification_context_table_ptr = ( RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_ADDRESS );
        ds_ingress_classification_context_entry_ptr = &( ds_ingress_classification_context_table_ptr->entry[ context_index ] );

        vlan_cmd_idx_table_ptr = RDD_VLAN_COMMAND_INDEX_TABLE_PTR();
        vlan_cmd_idx_entry_ptr = &( vlan_cmd_idx_table_ptr->entry[ context_index ] );

        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OPBIT_REMARK_MODE_READ ( ds_context.opbit_remark_mode, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_IPBIT_REMARK_MODE_READ ( ds_context.ipbit_remark_mode, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OUTER_PBIT_READ ( ds_context.outer_pbit, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_INNER_PBIT_READ ( ds_context.inner_pbit, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_REMARKING_MODE_READ ( ds_context.dscp_remarking_mode, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_READ ( ds_context.dscp, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_ECN_READ ( ds_context.ecn, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_TRAFFIC_CLASS_READ ( ds_context.traffic_class, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_FORWARD_MODE_READ ( ds_context.forward_mode, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_EGRESS_PORT_READ ( ds_context.egress_port, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_QOS_MAPPING_MODE_READ ( ds_context.qos_mapping_mode, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SUBNET_ID_READ ( ds_context.subnet_id, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_WIFI_SSID_READ ( ds_context.wifi_ssid, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DROP_READ ( ds_context.drop, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CPU_READ ( ds_context.cpu, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_MODE_READ ( ds_context.policer_mode, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_ID_READ ( ds_context.policer_id, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_RATE_SHAPING_MODE_READ ( ds_context.rate_shaping_mode, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SERVICE_QUEUE_MODE_READ ( ds_context.service_queue_mode, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_REMARK_ENABLE_READ( dei_remark_enable, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_VALUE_READ( dei_value, ds_ingress_classification_context_entry_ptr );

        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH0_VLAN_COMMAND_ID_READ ( vlan_entry.eth0_vlan_command_id, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH1_VLAN_COMMAND_ID_READ ( vlan_entry.eth1_vlan_command_id, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH2_VLAN_COMMAND_ID_READ ( vlan_entry.eth2_vlan_command_id, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH3_VLAN_COMMAND_ID_READ ( vlan_entry.eth3_vlan_command_id, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH4_VLAN_COMMAND_ID_READ ( vlan_entry.eth4_vlan_command_id, vlan_cmd_idx_entry_ptr );
#ifndef G9991
        RDD_VLAN_COMMAND_INDEX_ENTRY_PCI0_VLAN_COMMAND_ID_READ ( vlan_entry.pci0_vlan_command_id, vlan_cmd_idx_entry_ptr );
#else
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH5_VLAN_COMMAND_ID_READ ( vlan_entry.eth5_vlan_command_id, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH6_VLAN_COMMAND_ID_READ ( vlan_entry.eth6_vlan_command_id, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH7_VLAN_COMMAND_ID_READ ( vlan_entry.eth7_vlan_command_id, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH8_VLAN_COMMAND_ID_READ ( vlan_entry.eth8_vlan_command_id, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH9_VLAN_COMMAND_ID_READ ( vlan_entry.eth9_vlan_command_id, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH10_VLAN_COMMAND_ID_READ ( vlan_entry.eth10_vlan_command_id, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH11_VLAN_COMMAND_ID_READ ( vlan_entry.eth11_vlan_command_id, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH12_VLAN_COMMAND_ID_READ ( vlan_entry.eth12_vlan_command_id, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH13_VLAN_COMMAND_ID_READ ( vlan_entry.eth13_vlan_command_id, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH14_VLAN_COMMAND_ID_READ ( vlan_entry.eth14_vlan_command_id, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH15_VLAN_COMMAND_ID_READ ( vlan_entry.eth15_vlan_command_id, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH16_VLAN_COMMAND_ID_READ ( vlan_entry.eth16_vlan_command_id, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH17_VLAN_COMMAND_ID_READ ( vlan_entry.eth17_vlan_command_id, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH18_VLAN_COMMAND_ID_READ ( vlan_entry.eth18_vlan_command_id, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH19_VLAN_COMMAND_ID_READ ( vlan_entry.eth19_vlan_command_id, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH20_VLAN_COMMAND_ID_READ ( vlan_entry.eth20_vlan_command_id, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH21_VLAN_COMMAND_ID_READ ( vlan_entry.eth21_vlan_command_id, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH22_VLAN_COMMAND_ID_READ ( vlan_entry.eth22_vlan_command_id, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH23_VLAN_COMMAND_ID_READ ( vlan_entry.eth23_vlan_command_id, vlan_cmd_idx_entry_ptr );
#endif

    }
    else
    {
        us_ingress_classification_context_table_ptr = ( RDD_US_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_CONTEXT_TABLE_ADDRESS );
        us_ingress_classification_context_entry_ptr = &( us_ingress_classification_context_table_ptr->entry[ context_index ] );

        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_TRAFFIC_CLASS_READ ( us_context.traffic_class, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_RATE_CONTROLLER_READ ( us_context.rate_controller, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_VLAN_CMD_INDEX_READ ( us_context.vlan_cmd_index, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_WAN_FLOW_READ ( us_context.wan_flow, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_QOS_MAPPING_MODE_READ ( us_context.qos_mapping_mode, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DROP_READ ( us_context.drop, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CPU_READ ( us_context.cpu, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OPBIT_REMARK_MODE_READ ( us_context.opbit_remark_mode, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_IPBIT_REMARK_MODE_READ ( us_context.ipbit_remark_mode, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OUTER_PBIT_READ ( us_context.outer_pbit, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_INNER_PBIT_READ ( us_context.inner_pbit, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_REMARKING_MODE_READ ( us_context.dscp_remarking_mode, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_READ ( us_context.dscp, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_ECN_READ ( us_context.ecn, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_MODE_READ ( us_context.policer_mode, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_ID_READ ( us_context.policer_id, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_QOS_RULE_MATCH_READ ( us_context.qos_rule_match, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_REMARK_ENABLE_READ( dei_remark_enable, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_VALUE_READ( dei_value, us_ingress_classification_context_entry_ptr );

    }

    if ( direction == rdpa_dir_ds )
    {
        bdmf_session_print ( session, "DS" );
    }
    else
    {
        bdmf_session_print ( session, "US" );
    }

    bdmf_session_print ( session, " Ingress Classification Context %d\n\r", ( int )context_index );
    bdmf_session_print ( session, "---------------------------------------\n\r" );   

    if ( direction == rdpa_dir_ds )
    {
        bdmf_session_print ( session, "Opbit remark mode:   %6d\n", ( int )ds_context.opbit_remark_mode );
        bdmf_session_print ( session, "Ipbit remark mode:   %6d\n", ( int )ds_context.ipbit_remark_mode );
        bdmf_session_print ( session, "CPU:                 %6d\n", ( int )ds_context.cpu );
        bdmf_session_print ( session, "DROP:                %6d\n", ( int )ds_context.drop );
        bdmf_session_print ( session, "Outer pbit:          %6d\n", ( int )ds_context.outer_pbit );
        bdmf_session_print ( session, "Inner pbit:          %6d\n", ( int )ds_context.inner_pbit );
        bdmf_session_print ( session, "DSCP remarking:      %6d\n", ( int )ds_context.dscp_remarking_mode );
        bdmf_session_print ( session, "DSCP value:          %6d\n", ( int )ds_context.dscp );
        bdmf_session_print ( session, "ECN value:           %6d\n", ( int )ds_context.ecn );
        bdmf_session_print ( session, "Traffic class:       %6d\n", ( int )ds_context.traffic_class );
        bdmf_session_print ( session, "QOS mapping mode:    %6d\n", ( int )ds_context.qos_mapping_mode ); 
        bdmf_session_print ( session, "Wifi ssid:           %6d\n", ( int )ds_context.wifi_ssid );
        bdmf_session_print ( session, "Egress port:         %6d\n", ( int )ds_context.egress_port );
        bdmf_session_print ( session, "Subnet ID:           %6d\n", ( int )ds_context.subnet_id );                 
        bdmf_session_print ( session, "Forward mode:        %6d\n", ( int )ds_context.forward_mode ); 
        bdmf_session_print ( session, "Policer mode:        %6d\n", ( int )ds_context.policer_mode ); 
        bdmf_session_print ( session, "Policer ID:          %6d\n", ( int )ds_context.policer_id ); 
        if ( ds_context.service_queue_mode )
        {
            bdmf_session_print ( session, "Service queue:      %6d\n", ( int )ds_context.service_queue );
        }

        bdmf_session_print ( session, "eth0_vlan_command:   %6d\n", ( signed char )vlan_entry.eth0_vlan_command_id );
        bdmf_session_print ( session, "eth1_vlan_command:   %6d\n", ( signed char )vlan_entry.eth1_vlan_command_id );
        bdmf_session_print ( session, "eth2_vlan_command:   %6d\n", ( signed char )vlan_entry.eth2_vlan_command_id );
        bdmf_session_print ( session, "eth3_vlan_command:   %6d\n", ( signed char )vlan_entry.eth3_vlan_command_id );
        bdmf_session_print ( session, "eth4_vlan_command:   %6d\n", ( signed char )vlan_entry.eth4_vlan_command_id );
#ifndef G9991
        bdmf_session_print ( session, "pci_vlan_command:    %6d\n", ( signed char )vlan_entry.pci0_vlan_command_id );
#else
        bdmf_session_print ( session, "eth5_vlan_command:   %6d\n", ( signed char )vlan_entry.eth5_vlan_command_id );
        bdmf_session_print ( session, "eth6_vlan_command:   %6d\n", ( signed char )vlan_entry.eth6_vlan_command_id );
        bdmf_session_print ( session, "eth7_vlan_command:   %6d\n", ( signed char )vlan_entry.eth7_vlan_command_id );
        bdmf_session_print ( session, "eth8_vlan_command:   %6d\n", ( signed char )vlan_entry.eth8_vlan_command_id );
        bdmf_session_print ( session, "eth9_vlan_command:   %6d\n", ( signed char )vlan_entry.eth9_vlan_command_id );
        bdmf_session_print ( session, "eth10_vlan_command:   %6d\n", ( signed char )vlan_entry.eth10_vlan_command_id );
        bdmf_session_print ( session, "eth11_vlan_command:   %6d\n", ( signed char )vlan_entry.eth11_vlan_command_id );
        bdmf_session_print ( session, "eth12_vlan_command:   %6d\n", ( signed char )vlan_entry.eth12_vlan_command_id );
        bdmf_session_print ( session, "eth13_vlan_command:   %6d\n", ( signed char )vlan_entry.eth13_vlan_command_id );
        bdmf_session_print ( session, "eth14_vlan_command:   %6d\n", ( signed char )vlan_entry.eth14_vlan_command_id );
        bdmf_session_print ( session, "eth15_vlan_command:   %6d\n", ( signed char )vlan_entry.eth15_vlan_command_id );
#endif
    }
    else
    {  
        bdmf_session_print ( session, "Opbit remark mode:   %6d\n", ( int )us_context.opbit_remark_mode );
        bdmf_session_print ( session, "Ipbit remark mode:   %6d\n", ( int )us_context.ipbit_remark_mode );
        bdmf_session_print ( session, "CPU:                 %6d\n", ( int )us_context.cpu );
        bdmf_session_print ( session, "DROP:                %6d\n", ( int )us_context.drop );
        bdmf_session_print ( session, "Outer pbit:          %6d\n", ( int )us_context.outer_pbit );
        bdmf_session_print ( session, "Inner pbit:          %6d\n", ( int )us_context.inner_pbit );
        bdmf_session_print ( session, "DSCP remarking:      %6d\n", ( int )us_context.dscp_remarking_mode );
        bdmf_session_print ( session, "DSCP value:          %6d\n", ( int )us_context.dscp );
        bdmf_session_print ( session, "ECN value:           %6d\n", ( int )us_context.ecn );
        bdmf_session_print ( session, "Traffic class:       %6d\n", ( int )us_context.traffic_class );
        bdmf_session_print ( session, "QOS mapping mode:    %6d\n", ( int )us_context.qos_mapping_mode ); 
        bdmf_session_print ( session, "WAN flow:            %6d\n", ( int )us_context.wan_flow );
        bdmf_session_print ( session, "Policer mode:        %6d\n", ( int )us_context.policer_mode );
        bdmf_session_print ( session, "Policer ID:          %6d\n", ( int )us_context.policer_id );
        bdmf_session_print ( session, "Rate controller:     %6d\n", ( int )us_context.rate_controller );
        bdmf_session_print ( session, "Qos rule match:      %6d\n", ( int )us_context.qos_rule_match );
        bdmf_session_print ( session, "US vlan command:     %6d\n", ( signed char )us_context.vlan_cmd_index );
    }

    if ( dei_remark_enable )
    {
        bdmf_session_print ( session, "Dei_command:         %s\n", ( dei_value )? "set":"clear" );
    }
    else
    {
        bdmf_session_print ( session, "Dei_command:         %s\n", "transparent" );
    }


#if !defined(FIRMWARE_INIT) && !defined(RDD_BASIC)
    rdd_error = rdd_ingress_classification_context_counter_read ( direction, context_index, &counter );

    if ( rdd_error == BL_LILAC_RDD_OK )
    {
        bdmf_session_print ( session, "Packet count:        %6d\n", ( int )counter );
    }
#endif

    return ( 0 );
}


static int p_lilac_rdd_print_ingress_classification_rule_cfgs ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS  *ds_rule_cfg_table_ptr;
    RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS  *us_rule_cfg_table_ptr;
    RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_DTS  *rule_cfg_entry_ptr;
    uint32_t	rule_cfg_id;
    uint8_t		rule_cfg_type;
    uint16_t	key_mask;
    uint8_t		lookup_mode;
    uint8_t		hit_action;
    uint8_t		miss_action;
    uint8_t		next_rule_cfg_id;
    uint16_t	next_group_id;
    uint32_t	direction;
    uint32_t    ingress_classification_rule_cfg_table_size;
    direction = ( unsigned int )parm[ 0 ].value.unumber;

    if ( direction == rdpa_dir_ds )
    {
        ds_rule_cfg_table_ptr = RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_PTR();
        ingress_classification_rule_cfg_table_size = RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE;
    }
    else
    {
        us_rule_cfg_table_ptr = RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_PTR();
        ingress_classification_rule_cfg_table_size = RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE;
    }

    bdmf_session_print ( session, "    id   |    mask   |  type  |  mode   |   hit_action   |  miss_action |  next_rule |  next_group  |  priority  \n" );

    for ( rule_cfg_id = 0; rule_cfg_id < ingress_classification_rule_cfg_table_size; rule_cfg_id++ )
    {
        if( g_ingress_classification_rule_cfg_table[ direction ].rule_cfg[ rule_cfg_id ].priority >= 0 )
        {
            if ( direction == rdpa_dir_ds )
            {
                rule_cfg_entry_ptr = &( ds_rule_cfg_table_ptr->entry[ rule_cfg_id ] );
            }
            else
            {
                rule_cfg_entry_ptr = &( us_rule_cfg_table_ptr->entry[ rule_cfg_id ] );
            }

            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_RULE_TYPE_READ ( rule_cfg_type, rule_cfg_entry_ptr );
            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_KEY_MASK_READ ( key_mask, rule_cfg_entry_ptr );
            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_LOOKUP_MODE_READ ( lookup_mode, rule_cfg_entry_ptr );
            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_HIT_ACTION_READ ( hit_action, rule_cfg_entry_ptr );
            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_MISS_ACTION_READ ( miss_action, rule_cfg_entry_ptr );
            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_RULE_CFG_ID_READ ( next_rule_cfg_id, rule_cfg_entry_ptr );
            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_GROUP_ID_READ ( next_group_id, rule_cfg_entry_ptr );

            bdmf_session_print ( session, "   %3u   |   0x%-5x  |  %3u   |  %-6u |    %-6u      |    %-6u    |   %-6u   |    %-6u    |", ( unsigned int )rule_cfg_id,
								 ( unsigned int )key_mask, ( unsigned int )rule_cfg_type, ( unsigned int )lookup_mode, ( unsigned int )hit_action,
								 ( unsigned int )miss_action, ( unsigned int )next_rule_cfg_id, ( unsigned int )next_group_id );

            bdmf_session_print ( session, "   %d   \n", g_ingress_classification_rule_cfg_table[ direction ].rule_cfg[ rule_cfg_id ].priority );
        }
    }

    bdmf_session_print ( session, "\n Head: %d\n", g_ingress_classification_rule_cfg_table[ direction ].first_rule_cfg_id );

    return ( 0 );
}


static int p_lilac_rdd_parallel_processing_context_cache_mode_set ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
#ifndef G9991
    uint8_t	 *ds_cache_mode_ptr;
    uint8_t	 *us_cache_mode_ptr;
    uint8_t	 mode;

    mode = ( unsigned int )parm[ 0 ].value.unumber;

    mode = ( !mode );

    ds_cache_mode_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PARALLEL_PROCESSING_CONTEXT_CACHE_MODE_ADDRESS );
    us_cache_mode_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PARALLEL_PROCESSING_CONTEXT_CACHE_MODE_ADDRESS );

    MWRITE_8( ds_cache_mode_ptr, mode );
    MWRITE_8( us_cache_mode_ptr, mode );

#endif
    return ( 0 );
}


static int p_lilac_rdd_print_general_information ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS  *bridge_cfg_register;
    uint8_t		                   debug_mode;

    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );
    RDD_BRIDGE_CONFIGURATION_REGISTER_DEBUG_MODE_READ ( debug_mode, bridge_cfg_register );

    bdmf_session_print ( session, " General Information:\n" );
    bdmf_session_print ( session, "======================\n" );
    bdmf_session_print ( session, " Debug mode:     %-6u\n", ( unsigned )debug_mode );

    return ( 0 );
}

static int p_lilac_rdd_print_pbits_to_wan_flow_table ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    uint8_t	                            table;
    uint8_t	                            wan_flow;
    RDD_US_PBITS_TO_WAN_FLOW_TABLE_DTS  *pbits_to_wan_flow_table_ptr;
    uint8_t                             *wan_flow_entry_ptr;
    uint8_t                             pbits;

    table = ( unsigned int )parm[ 0 ].value.unumber;

    bdmf_session_print ( session, " Printing Pbits To Wan Flow Table no. %d :\n" ,table);
    bdmf_session_print ( session, "===========================================\n" );

    pbits_to_wan_flow_table_ptr = RDD_US_PBITS_TO_WAN_FLOW_TABLE_PTR();

    bdmf_session_print ( session, " Pbits || Wan Flow\n");

    for ( pbits = 0 ; pbits < RDD_US_PBITS_TO_WAN_FLOW_TABLE_SIZE ; pbits++ )
    {
        wan_flow_entry_ptr = ( uint8_t * )&( pbits_to_wan_flow_table_ptr->entry[ table ][ pbits ] );
        MREAD_8 ( wan_flow_entry_ptr, wan_flow );
        
        bdmf_session_print ( session, "   %-1u   ||   %-3u   \n", pbits, wan_flow);
        
    }

    return ( 0 );
}


static int p_lilac_rdd_print_cpu_rx_interrupt_coalescing_information ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    RDD_INTERRUPT_COALESCING_CONFIG_TABLE_DTS *ic_table_ptr;
    RDD_INTERRUPT_COALESCING_CONFIG_DTS       *ic_entry_ptr;
    RDD_INTERRUPT_COALESCING_TIMER_CONFIG_DTS *ic_timer_config_table_ptr;
    uint16_t value16;
    uint16_t timeouts;
    uint32_t cntr;
    uint32_t first = 0 ,last = D_NUM_OF_RING_DESCRIPTORS;

    ic_table_ptr = ( RDD_INTERRUPT_COALESCING_CONFIG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + INTERRUPT_COALESCING_CONFIG_TABLE_ADDRESS );

    bdmf_session_print ( session, "CPU RX Interrupt Coalescing Information \n" );
    bdmf_session_print ( session, "------------------------------------------------------\n" );

    if (n_parms == 1 )
    {
        first = ( uint32_t )parm[ 0 ].value.unumber;
        last = first + 1;
    }

    for( cntr=first,ic_entry_ptr=&ic_table_ptr->entry[first];cntr < last;cntr++,ic_entry_ptr++ )
    {
        RDD_INTERRUPT_COALESCING_CONFIG_CONFIGURED_TIMEOUT_READ( value16, ic_entry_ptr );
        if(!value16) continue;

        bdmf_session_print ( session, "Interrupt coalescing values for CPU RX Ring Queue = %d:\n",cntr );
        bdmf_session_print ( session, "\tconfigured timeout (us)          = %d \n",value16 );
        RDD_INTERRUPT_COALESCING_CONFIG_CURRENT_TIMEOUT_READ( value16, ic_entry_ptr );
        bdmf_session_print ( session, "\tcurrent timeout (us)             = %d \n",value16 );
        RDD_INTERRUPT_COALESCING_CONFIG_CONFIGURED_MAX_PACKET_COUNT_READ( value16, ic_entry_ptr );
        bdmf_session_print ( session, "\tconfigured maximum packet count  = %d \n",value16 );
        RDD_INTERRUPT_COALESCING_CONFIG_CURRENT_PACKET_COUNT_READ( value16, ic_entry_ptr );
        bdmf_session_print ( session, "\tcurrent packet count             = %d \n",value16 );
        bdmf_session_print ( session, "\n" );

        rdd_2_bytes_counter_get ( CPU_RX_INTERRUPT_COALESCING_GROUP, (cntr << 2) + CPU_RX_INTCOL_TIMEOUTS_OFFSET, LILAC_RDD_TRUE, &timeouts );
        bdmf_session_print ( session, "\ttimeout interrupts               = %d \n",timeouts );
        rdd_2_bytes_counter_get ( CPU_RX_INTERRUPT_COALESCING_GROUP, (cntr << 2) + CPU_RX_INTCOL_MAXPKTS_OFFSET, LILAC_RDD_TRUE, &value16 );
        bdmf_session_print ( session, "\tpacket count reached interrupts  = %d \n",value16 );
        bdmf_session_print ( session, "\ttotal CPU RX interrupts          = %d \n",value16+timeouts );
        rdd_2_bytes_counter_get ( CPU_RX_INTERRUPT_COALESCING_GROUP, (cntr << 2) + CPU_RX_INTCOL_TOTALPKTS_OFFSET, LILAC_RDD_TRUE, &value16 );
        bdmf_session_print ( session, "\tpackets processed                = %d \n",value16 );
        bdmf_session_print ( session, "-------------------------------------------------------\n" );
        bdmf_session_print ( session, "\n" );

        ic_timer_config_table_ptr = ( RDD_INTERRUPT_COALESCING_TIMER_CONFIG_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + INTERRUPT_COALESCING_TIMER_CONFIG_TABLE_ADDRESS );
        RDD_INTERRUPT_COALESCING_TIMER_CONFIG_TIMER_PERIOD_READ( value16, ic_timer_config_table_ptr );
        bdmf_session_print ( session, "\tconfigured timer period (us)     = %d \n",value16 );
        RDD_INTERRUPT_COALESCING_TIMER_CONFIG_TIMER_ARMED_READ( value16, ic_timer_config_table_ptr );
        bdmf_session_print ( session, "\tcurrent timers armed bit mask    = 0x%4.4x \n",value16 );
        bdmf_session_print ( session, "\n\n" );
    }

    return 0;
}

extern unsigned int SEGMENTS_ADDRESSES[];
static char *seg_names[] = { "Private A", "Private B", "Common A", "Common B", "DDR", "PSRAM" };
extern TABLE_STRUCT RUNNER_TABLES[];

static unsigned int table_addr_get(TABLE_STRUCT *tbl, int entry_idx)
{
    DUMP_RUNNERREG_STRUCT *tbl_ctx;
    unsigned int addr;

    tbl_ctx = tbl->entries;
    addr = SEGMENTS_ADDRESSES[tbl->segment] + tbl_ctx->entries[entry_idx].starts;
    if (tbl->segment == COMMON_B_INDEX)
        addr -= 0x8000;
    if (tbl->segment == DDR_INDEX)
        addr += (uint32_t)g_runner_ddr_base_addr;
    else
        addr = (int)DEVICE_ADDRESS(addr);
    return addr;
}

static char *str_toupper(char *str)
{
    int i, len;

    len = strlen(str);
    for (i = 0; i < len; i++)
    {
        if (str[i] >= 'a' && str[i] <= 'z')
            str[i] = str[i] - 'a' + 'A';
    }
    return str;
}

static int p_lilac_rdd_print_tables_list(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    TABLE_STRUCT *tbl;
    DUMP_RUNNERREG_STRUCT *tbl_ctx;
    int i, j, n;
    char parm_val[256];

    if (n_parms)
    {
        strcpy(parm_val, parm[0].value.string);
        bdmf_session_print(session, "Param %s, parm_val %s\n", parm[0].value.string, parm_val);
        str_toupper(parm_val);
        bdmf_session_print(session, "parm_val after convert %s\n", parm_val);
    }

    bdmf_session_print(session, "List of Tables\n\n");
    bdmf_session_print(session, "%70s %8s %15s %12s %12s %12s\n", "Table Name", "Address", "Segment", "Entry Len",
        "Entry Types", "Size");
    bdmf_session_print(session, "---------------------------------------------------------------------");
    bdmf_session_print(session, "---------------------------------------------------------------------\n");
    for (i = 0, n = 0; i < NUMBER_OF_TABLES; i++)
    {
        tbl = &RUNNER_TABLES[i];
        if (n_parms)
        {
            /* Check if table name complies the pattern. If it doesn't, skip */
            if (!strstr(tbl->table_name, parm_val))
                continue;
        }
        tbl_ctx = tbl->entries;

        /* If this is a union, calc how many entry representations we have */
        for (j = 0; tbl_ctx->entries[j].callback; j++);

        bdmf_session_print(session, "%70s 0x%08x %15s %10d %10d          [%d]",
            tbl->table_name, table_addr_get(tbl, 0), seg_names[tbl->segment], tbl_ctx->length, j, tbl->size_rows);
        if (tbl->size_rows_d2)
            bdmf_session_print(session, "[%d]", tbl->size_rows_d2);
        if (tbl->size_rows_d3)
            bdmf_session_print(session, "[%d]", tbl->size_rows_d3);
        bdmf_session_print(session, "\n");
        n++;
    }
    bdmf_session_print(session, "\nTotal %d tables\n", n);

    return 0;
}

static int p_lilac_rdd_print_table_entries(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    TABLE_STRUCT *tbl;
    DUMP_RUNNERREG_STRUCT *tbl_ctx;
    int i, j;
    char parm_val[256];
    int start_entry, num_of_entries, is_addr;
    unsigned int tbl_addr, entry_addr;

    strcpy(parm_val, parm[0].value.string);
    str_toupper(parm_val);
    for (i = 0; i < NUMBER_OF_TABLES; i++)
    {
        tbl = &RUNNER_TABLES[i];
        if (!strcmp(tbl->table_name, parm_val))
            break;
    }
    if (i == NUMBER_OF_TABLES) /* Table not found */
    {
        bdmf_session_print(session, "Table %s not found\n", parm_val);
        return BDMF_ERR_PARM;
    }

    tbl_addr = table_addr_get(tbl, 0);
    tbl_ctx = tbl->entries;

    is_addr = parm[1].value.unumber;
    if (is_addr)
    {
        entry_addr = parm[3].value.unumber;
        start_entry = (entry_addr - tbl_addr) / tbl_ctx->length;
    }
    else
    {
        start_entry = parm[2].value.unumber;
        entry_addr = tbl_addr + start_entry * tbl_ctx->length;
    }
    num_of_entries = parm[4].value.unumber;

    for (i = 0; i < num_of_entries; i++, entry_addr += tbl_ctx->length)
    {
        bdmf_session_print(session, "Index %d, addr 0x%08x, size %d, value:\n", start_entry + i,
            entry_addr, tbl_ctx->length);
        bdmf_session_hexdump(session, (unsigned char *)entry_addr, 0, tbl_ctx->length);
        bdmf_session_print(session, "\n");
         /* It's possible that we have a union of entries. In this case, we want to print all possible
         * transformations.*/
        for (j = 0; tbl_ctx->entries[j].callback; j++)
        {
            tbl_ctx->entries[j].callback(session, (unsigned char *)entry_addr);
            bdmf_session_print(session, "\n");
        }
    }
    bdmf_session_print(session, "\n\n");
    return 0;
}

static int p_lilac_rdd_ack_prioritization_enable (bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t enable;

    enable = (unsigned int)parm[ 0 ].value.unumber;

    rdd_ack_prioritization_config(enable);

    return 0;
}

static int p_lilac_rdd_ack_packet_size_threshold_config (bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t ack_packet_size_threshold;

    ack_packet_size_threshold = (unsigned int)parm[ 0 ].value.unumber;

    rdd_ack_packet_size_threshold_config((uint8_t)ack_packet_size_threshold);

    return 0;
}

static int p_lilac_rdd_ack_packets_queue_index_config (bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t channel_id;
    uint32_t queue_num;
    uint32_t rate_controller;

    channel_id      = (unsigned int)parm[ 0 ].value.unumber;
    queue_num       = (unsigned int)parm[ 1 ].value.unumber;
    rate_controller = (unsigned int)parm[ 2 ].value.unumber;

    rdd_wan_ack_packets_queue_index_config((RDD_WAN_CHANNEL_ID)channel_id, (BL_LILAC_RDD_RATE_CONTROLLER_ID_DTE)rate_controller, (BL_LILAC_RDD_QUEUE_ID_DTE)queue_num);

    return 0;
}

static int32_t rdd_trace_enable(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    uint32_t enable;

    enable = parm[0].value.unumber;
    if (enable != 0 && enable != 1)
    {
        bdmf_session_print(session, "Bad argument %d (expected 0|1)\n", enable);
        return BDMF_ERR_PARM;
    }
    g_rdd_trace = enable;
    return 0;
}

#endif /* USE_BDMF_SHELL */
