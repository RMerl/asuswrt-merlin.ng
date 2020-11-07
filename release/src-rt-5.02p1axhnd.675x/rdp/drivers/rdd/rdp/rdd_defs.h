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

#ifndef _RDD_DEFS_H
#define _RDD_DEFS_H

#include "rdpa_types.h"
#include "rdpa_ip_class_basic.h"
#include "rdpa_cpu_basic.h"
#include "rdpa_iptv.h"
#include "rdd_data_structures_auto.h"

typedef enum
{
    RDD_CLUSTER_0 = 0,
    RDD_CLUSTER_1 = 1,
} rdd_cluster_t;

/*This type is strictly used as an enum. The values are irrelevant. There's mapping logic in rdd_tm/rdd_cpu.*/
typedef enum
{
    RDD_WAN_PHYSICAL_PORT_GPON = 0,
    RDD_WAN_PHYSICAL_PORT_ETH4 = 1,
    RDD_WAN_PHYSICAL_PORT_ETH5 = 2,
    RDD_WAN_PHYSICAL_PORT_ETH0 = 3,
    RDD_WAN_PHYSICAL_PORT_EPON = 4,
    RDD_WAN_PHYSICAL_PORT_ETH1 = 5,
    RDD_WAN_PHYSICAL_PORT_ETH2 = 6,
    RDD_WAN_PHYSICAL_PORT_ETH3 = 7,
} rdd_wan_physical_port_t;

#ifndef WL4908
/* XXX: Move rdd_emac_id_t definition to be project-specific. Can be auto-generated from XML? */
typedef enum
{
    RDD_EMAC_ID_START = 0,
    RDD_EMAC_ID_WIFI = RDD_EMAC_ID_START,
    RDD_EMAC_ID_LAN_START,
    RDD_EMAC_ID_0 = RDD_EMAC_ID_LAN_START,
    RDD_EMAC_ID_1,
    RDD_EMAC_ID_2,
    RDD_EMAC_ID_3,
    RDD_EMAC_ID_4,
    RDD_EMAC_ID_5,
    RDD_EMAC_ID_COUNT,
} rdd_emac_id_t;
#else
#define RDD_EMAC_ID_START RDD_EMAC_FIRST
#define RDD_EMAC_ID_LAN_START RDD_EMAC_ID_0
typedef rdd_rdd_emac rdd_emac_id_t;
#endif

typedef rdd_rdd_vport rdd_vport_id_t;

typedef uint32_t rdd_emac_id_vector_t;

static inline rdd_emac_id_vector_t rdd_emac_id_to_vector(rdd_emac_id_t emac, bdmf_boolean is_iptv)
{
    return 1LL << emac;
}
#define RDD_EMAC_PORT_TO_VECTOR(rdd_emac_id, is_iptv) rdd_emac_id_to_vector(rdd_emac_id, is_iptv)

typedef enum
{
    RDD_WAN_CHANNEL_UNASSIGNED = -1,
    RDD_WAN_CHANNEL_0 = 0,
    RDD_WAN_CHANNEL_1,
    RDD_WAN_CHANNEL_0_7_MAX = RDD_WAN_CHANNEL_1,
    RDD_WAN_CHANNEL_MAX = RDD_WAN_CHANNEL_1,
} rdd_wan_channel_id_t;

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
#define RDD_WAN0_CHANNEL_BASE               RDD_WAN_CHANNEL_1
#define RDD_WAN1_CHANNEL_BASE               RDD_WAN_CHANNEL_0

typedef enum {
    rdd_egress_phy_eth_lan = 0,
    rdd_egress_phy_eth_wan,
    rdd_egress_phy_gpon = rdd_egress_phy_eth_wan, /* Unused */
    rdd_egress_phy_dsl,
    rdd_egress_phy_wlan,
    rdd_egress_phy_max
} rdd_egress_phy_t;
#endif /*DSL*/

typedef enum
{
    RDD_RATE_CNTRL_ID_UNASSIGNED = -1,
    RDD_RATE_CNTRL_0  = 0,
    RDD_RATE_CNTRL_1,
    RDD_RATE_CNTRL_2,
    RDD_RATE_CNTRL_3,
    RDD_RATE_CNTRL_4,
    RDD_RATE_CNTRL_5,
    RDD_RATE_CNTRL_6,
    RDD_RATE_CNTRL_7,
    RDD_RATE_CNTRL_8,
    RDD_RATE_CNTRL_9,
    RDD_RATE_CNTRL_10,
    RDD_RATE_CNTRL_11,
    RDD_RATE_CNTRL_12,
    RDD_RATE_CNTRL_13,
    RDD_RATE_CNTRL_14,
    RDD_RATE_CNTRL_15,
    RDD_RATE_CNTRL_16,
    RDD_RATE_CNTRL_17,
    RDD_RATE_CNTRL_18,
    RDD_RATE_CNTRL_19,
    RDD_RATE_CNTRL_20,
    RDD_RATE_CNTRL_21,
    RDD_RATE_CNTRL_22,
    RDD_RATE_CNTRL_23,
    RDD_RATE_CNTRL_24,
    RDD_RATE_CNTRL_25,
    RDD_RATE_CNTRL_26,
    RDD_RATE_CNTRL_27,
    RDD_RATE_CNTRL_28,
    RDD_RATE_CNTRL_29,
    RDD_RATE_CNTRL_30,
    RDD_RATE_CNTRL_31,
    RDD_RATE_CNTRL_32,
    RDD_RATE_CNTRL_33,
    RDD_RATE_CNTRL_34,
    RDD_RATE_CNTRL_35,
    RDD_RATE_CNTRL_36,
    RDD_RATE_CNTRL_37,
    RDD_RATE_CNTRL_38,
    RDD_RATE_CNTRL_39,
    RDD_RATE_CNTRL_40,
    RDD_RATE_CNTRL_41,
    RDD_RATE_CNTRL_42,
    RDD_RATE_CNTRL_43,
    RDD_RATE_CNTRL_44,
    RDD_RATE_CNTRL_45,
    RDD_RATE_CNTRL_46,
    RDD_RATE_CNTRL_47,
    RDD_RATE_CNTRL_48,
    RDD_RATE_CNTRL_49,
    RDD_RATE_CNTRL_50,
    RDD_RATE_CNTRL_51,
    RDD_RATE_CNTRL_52,
    RDD_RATE_CNTRL_53,
    RDD_RATE_CNTRL_54,
    RDD_RATE_CNTRL_55,
    RDD_RATE_CNTRL_56,
    RDD_RATE_CNTRL_57,
    RDD_RATE_CNTRL_58,
    RDD_RATE_CNTRL_59,
    RDD_RATE_CNTRL_60,
    RDD_RATE_CNTRL_61,
    RDD_RATE_CNTRL_62,
    RDD_RATE_CNTRL_63,
    RDD_RATE_CNTRL_64,
    RDD_RATE_CNTRL_65,
    RDD_RATE_CNTRL_66,
    RDD_RATE_CNTRL_67,
    RDD_RATE_CNTRL_68,
    RDD_RATE_CNTRL_69,
    RDD_RATE_CNTRL_70,
    RDD_RATE_CNTRL_71,
    RDD_RATE_CNTRL_72,
    RDD_RATE_CNTRL_73,
    RDD_RATE_CNTRL_74,
    RDD_RATE_CNTRL_75,
    RDD_RATE_CNTRL_76,
    RDD_RATE_CNTRL_77,
    RDD_RATE_CNTRL_78,
    RDD_RATE_CNTRL_79,
    RDD_RATE_CNTRL_80,
    RDD_RATE_CNTRL_81,
    RDD_RATE_CNTRL_82,
    RDD_RATE_CNTRL_83,
    RDD_RATE_CNTRL_84,
    RDD_RATE_CNTRL_85,
    RDD_RATE_CNTRL_86,
    RDD_RATE_CNTRL_87,
    RDD_RATE_CNTRL_88,
    RDD_RATE_CNTRL_89,
    RDD_RATE_CNTRL_90,
    RDD_RATE_CNTRL_91,
    RDD_RATE_CNTRL_92,
    RDD_RATE_CNTRL_93,
    RDD_RATE_CNTRL_94,
    RDD_RATE_CNTRL_95,
    RDD_RATE_CNTRL_96,
    RDD_RATE_CNTRL_97,
    RDD_RATE_CNTRL_98,
    RDD_RATE_CNTRL_99,
    RDD_RATE_CNTRL_100,
    RDD_RATE_CNTRL_101,
    RDD_RATE_CNTRL_102,
    RDD_RATE_CNTRL_103,
    RDD_RATE_CNTRL_104,
    RDD_RATE_CNTRL_105,
    RDD_RATE_CNTRL_106,
    RDD_RATE_CNTRL_107,
    RDD_RATE_CNTRL_108,
    RDD_RATE_CNTRL_109,
    RDD_RATE_CNTRL_110,
    RDD_RATE_CNTRL_111,
    RDD_RATE_CNTRL_112,
    RDD_RATE_CNTRL_113,
    RDD_RATE_CNTRL_114,
    RDD_RATE_CNTRL_115,
    RDD_RATE_CNTRL_116,
    RDD_RATE_CNTRL_117,
    RDD_RATE_CNTRL_118,
    RDD_RATE_CNTRL_119,
    RDD_RATE_CNTRL_120,
    RDD_RATE_CNTRL_121,
    RDD_RATE_CNTRL_122,
    RDD_RATE_CNTRL_123,
    RDD_RATE_CNTRL_124,
    RDD_RATE_CNTRL_125,
    RDD_RATE_CNTRL_126,
    RDD_RATE_CNTRL_127,
} rdd_rate_cntrl_id_t;

typedef enum
{
    RDD_TX_QUEUE_0 = 0,
    RDD_TX_QUEUE_1 = 1,
    RDD_TX_QUEUE_2 = 2,
    RDD_TX_QUEUE_3 = 3,
    RDD_TX_QUEUE_4 = 4,
    RDD_TX_QUEUE_5 = 5,
    RDD_TX_QUEUE_6 = 6,
    RDD_TX_QUEUE_7 = 7,
    RDD_TX_QUEUE_LAST = 7,
    RDD_TX_QUEUE_NUMBER = RDD_TX_QUEUE_LAST + 1,
} rdd_tx_queue_id_t;

typedef enum
{
    RDD_WAN_CHANNEL_SCHEDULE_PRIORITY = 0,
    RDD_WAN_CHANNEL_SCHEDULE_RATE_CONTROL = 1,
} rdd_wan_channel_schedule_t;

typedef enum
{
    RDD_PEAK_SCHEDULE_MODE_ROUND_ROBIN = 0,
    RDD_PEAK_SCHEDULE_MODE_STRICT_PRIORITY = 1,
} rdd_peak_schedule_mode_t;

typedef enum
{
    RDD_INTER_LAN_SCHEDULE_MODE_NORMAL = 0,
    RDD_INTER_LAN_SCHEDULE_MODE_STRICT_PRIORITY = 1,
    RDD_INTER_LAN_SCHEDULE_MODE_ROUND_ROBIN = 2,
} rdd_inter_lan_schedule_mode_t;

typedef struct
{
    uint32_t  rate;
    uint32_t  limit;
} rdd_rate_limit_params_t;

typedef struct
{
    uint32_t sustain_budget;
    rdd_rate_limit_params_t peak_budget;
    uint32_t peak_weight;
} rdd_rate_cntrl_params_t;

typedef enum
{
    RDD_RATE_LIMITER_PORT_0 = 0,
    RDD_RATE_LIMITER_PORT_1 = 1,
    RDD_RATE_LIMITER_PORT_2 = 2,
    RDD_RATE_LIMITER_PORT_3 = 3,
    RDD_RATE_LIMITER_PORT_4 = 4,
    RDD_RATE_LIMITER_PORT_5 = 5,
    RDD_RATE_LIMITER_PORT_6 = 6,
    RDD_RATE_LIMITER_PORT_7 = 7,
    RDD_RATE_LIMITER_PORT_8 = 8,
    RDD_RATE_LIMITER_PORT_9 = 9,
    RDD_RATE_LIMITER_PORT_10 = 10,
    RDD_RATE_LIMITER_PORT_11 = 11,
    RDD_RATE_LIMITER_PORT_12 = 12,
    RDD_RATE_LIMITER_PORT_13 = 13,
    RDD_RATE_LIMITER_PORT_14 = 14,
    RDD_RATE_LIMITER_PORT_15 = 15,

    RDD_RATE_LIMITER_PORT_LAST = 15,
    RDD_RATE_LIMITER_DISABLED = 16,
} rdd_rate_limiter_t;

typedef enum
{
    RDD_QUEUE_PROFILE_0 = 0,
    RDD_QUEUE_PROFILE_1,
    RDD_QUEUE_PROFILE_2,
    RDD_QUEUE_PROFILE_3,
    RDD_QUEUE_PROFILE_4,
    RDD_QUEUE_PROFILE_5,
    RDD_QUEUE_PROFILE_6,
    RDD_QUEUE_PROFILE_7,
    RDD_QUEUE_PROFILE_DISABLED = 8,
} rdd_queue_profile_id_t;

typedef struct
{
    uint16_t  min_threshold;
    uint16_t  max_threshold;
} rdd_prio_class_thresholds_t;

typedef struct
{
    rdd_prio_class_thresholds_t high_priority_class;
    rdd_prio_class_thresholds_t low_priority_class;
    uint32_t max_drop_probability;
    bdmf_boolean us_flow_control_mode; /* 0 for disabled, 1 for enabled */
} rdd_queue_profile_t;

typedef enum
{
    RDD_TPID_ID_0 = 0,
    RDD_TPID_ID_1,
    RDD_TPID_ID_2,
    RDD_TPID_ID_3,
    RDD_TPID_ID_4,
    RDD_TPID_ID_5,
    RDD_TPID_ID_6,
    RDD_TPID_ID_7,
} rdd_tpid_id_t;

typedef enum
{
    RDD_VLAN_CMD_TRANSPARENT = 0,
    RDD_VLAN_CMD_ADD_TAG,
    RDD_VLAN_CMD_REMOVE_TAG,
    RDD_VLAN_CMD_REPLACE_TAG,
    RDD_VLAN_CMD_ADD_TWO_TAGS,
    RDD_VLAN_CMD_REMOVE_TWO_TAGS,
    RDD_VLAN_CMD_ADD_OUTER_TAG_REPLACE_INNER_TAG,
    RDD_VLAN_CMD_REMOVE_OUTER_TAG_REPLACE_INNER_TAG,
    RDD_VLAN_CMD_ADD_TAG_ALWAYS,
    RDD_VLAN_CMD_REMOVE_TAG_ALWAYS,
    RDD_VLAN_CMD_REPLACE_OUTER_TAG_REPLACE_INNER_TAG,
    RDD_VLAN_CMD_REMOVE_OUTER_TAG_COPY,
    RDD_VLAN_CMD_ADD_3RD_TAG,
    RDD_MAX_VLAN_CMD,
} rdd_bridge_vlan_cmd_t;

typedef enum
{
    RDD_PBITS_CMD_TRANSPARENT,
    RDD_PBITS_CMD_COPY,
    RDD_PBITS_CMD_CONFIGURED,
    RDD_PBITS_CMD_REMAP,
    RDD_MAX_PBITS_CMD,
} rdd_bridge_pbits_cmd_t;

typedef struct
{
    uint32_t vlan_command_id;
    rdd_bridge_vlan_cmd_t vlan_command;
    rdd_bridge_pbits_cmd_t pbits_command;
    uint16_t outer_vid;
    uint16_t inner_vid;
    uint8_t outer_pbits;
    uint8_t inner_pbits;
    bdmf_boolean outer_tpid_overwrite_enable;
    bdmf_boolean inner_tpid_overwrite_enable;
    rdd_tpid_id_t outer_tpid_id;
    rdd_tpid_id_t inner_tpid_id;
} rdd_vlan_cmd_param_t;

/* XXX: Temporary, to be defined as enum */
typedef uint32_t rdd_port_profile_t;

typedef unsigned long long rdd_vport_vector_t;
typedef struct
{
    uint16_t vid;
    rdd_vport_vector_t isolation_mode_port_vector;
    rdd_vport_vector_t aggregation_mode_port_vector;
    uint16_t aggregation_vid_index;
} rdd_lan_vid_cfg_t;

/* XXX: Get rid of this enum, use auto-generated instead */
typedef enum
{
    RDD_FILTER_IGMP = 0,
    RDD_FILTER_ICMPV6,
    RDD_FILTER_UDEF_0,
    RDD_FILTER_UDEF_1,
    RDD_FILTER_UDEF_2,
    RDD_FILTER_UDEF_3,
    RDD_FILTER_PPPOE_D,
    RDD_FILTER_PPPOE_S,
    RDD_FILTER_ARP,
    RDD_FILTER_1588,
    RDD_FILTER_802_1X,
    RDD_FILTER_802_1AG_CFM,
    RDD_FILTER_BROADCAST,
    RDD_FILTER_MULTICAST,
    RDD_FILTER_LAST,
} rdd_ingress_filter_t;

typedef enum
{
    RDD_LAYER4_FILTER_ERROR = 0,
    RDD_LAYER4_FILTER_EXCEPTION,
    RDD_LAYER4_FILTER_IP_FIRST_FRAGMENT,
    RDD_LAYER4_FILTER_IP_FRAGMENT,
    RDD_LAYER4_FILTER_GRE,
    RDD_LAYER4_FILTER_LAYER3_IPV4,
    RDD_LAYER4_FILTER_LAYER3_IPV6,
    RDD_LAYER4_FILTER_ICMP,
    RDD_LAYER4_FILTER_ESP,
    RDD_LAYER4_FILTER_AH,
    RDD_LAYER4_FILTER_IPV6,
    RDD_LAYER4_FILTER_USER_DEFINED_0,
    RDD_LAYER4_FILTER_USER_DEFINED_1,
    RDD_LAYER4_FILTER_USER_DEFINED_2,
    RDD_LAYER4_FILTER_USER_DEFINED_3,
    RDD_LAYER4_FILTER_UNKNOWN,
} rdd_l4_filter_idx_t;

typedef enum
{
    RDD_FILTER_ACTION_CPU_TRAP = 1,
    RDD_FILTER_ACTION_DROP,
} rdd_ingress_filter_action_t;

typedef enum
{
    RDD_ETHER_TYPE_FILTER_USER_0 = 2,
    RDD_ETHER_TYPE_FILTER_USER_1,
    RDD_ETHER_TYPE_FILTER_USER_2,
    RDD_ETHER_TYPE_FILTER_USER_3,
    RDD_ETHER_TYPE_FILTER_PPPOE_D,
    RDD_ETHER_TYPE_FILTER_PPPOE_S,
    RDD_ETHER_TYPE_FILTER_ARP,
    RDD_ETHER_TYPE_FILTER_1588,
    RDD_ETHER_TYPE_FILTER_802_1X,
    RDD_ETHER_TYPE_FILTER_802_1AG_CFM,
} rdd_ether_type_filter_t;

typedef struct
{
    uint16_t invalid_layer2_protocol_drop;
    uint16_t firewall_drop;
    uint16_t acl_oui_drop;
    uint16_t acl_l2_drop;
    uint16_t acl_l3_drop;
    uint16_t dst_mac_non_router_drop;
    uint16_t eth_flow_action_drop;
    uint16_t sa_lookup_failure_drop;
    uint16_t da_lookup_failure_drop;
    uint16_t sa_action_drop;
    uint16_t da_action_drop;
    uint16_t forwarding_matrix_disabled_drop;
    uint16_t connection_action_drop;
    uint16_t iptv_layer3_drop;
    uint16_t local_switching_congestion;
    uint16_t vlan_switching_drop;
    uint16_t downstream_policers_drop;
    uint16_t layer4_filters_drop[16];
    uint16_t ingress_filters_drop[RDD_FILTER_LAST];
    uint16_t ip_validation_filter_drop[2];
    uint16_t emac_loopback_drop;
    uint16_t tpid_detect_drop;
    uint16_t dual_stack_lite_congestion_drop;
    uint16_t invalid_subnet_ip_drop;
    uint16_t us_ddr_queue_drop;
    uint16_t ds_parallel_processing_no_avialable_slave;
    uint16_t ds_parallel_processing_reorder_slaves;
} rdd_various_counters_t;

typedef enum
{
    INVALID_L2_PROTO_DROP_COUNTER_MASK           = 0x1,
    FIREWALL_DROP_COUNTER_MASK                   = 0x2,
    ACL_OUI_DROP_COUNTER_MASK                    = 0x4,
    ACL_L2_DROP_COUNTER_MASK                     = 0x8,
    ACL_L3_DROP_COUNTER_MASK                     = 0x10,
    DST_MAC_NON_ROUTER_DROP_COUNTER_MASK         = 0x20,
    ETHERNET_FLOW_ACTION_DROP_COUNTER_MASK       = 0x40,
    SA_LOOKUP_FAILURE_DROP_COUNTER_MASK          = 0x80,
    DA_LOOKUP_FAILURE_DROP_COUNTER_MASK          = 0x100,
    SA_ACTION_DROP_COUNTER_MASK                  = 0x200,
    DA_ACTION_DROP_COUNTER_MASK                  = 0x400,
    FORWARDING_MATRIX_DISABLED_DROP_COUNTER_MASK = 0x800,
    CONNECTION_ACTION_DROP_COUNTER_MASK          = 0x1000,
    IPTV_L3_DROP_COUNTER_MASK                    = 0x2000,
    LOCAL_SWITCHING_CONGESTION_COUNTER_MASK      = 0x4000,
    VLAN_SWITCHING_DROP_COUNTER_MASK             = 0x8000,
    DOWNSTREAM_POLICERS_DROP_COUNTER_MASK        = 0x10000,
    L4_FILTERS_DROP_COUNTER_MASK                 = 0x20000,
    INGRESS_FILTERS_DROP_COUNTER_MASK            = 0x40000,
    IP_VALIDATION_FILTER_DROP_COUNTER_MASK       = 0x80000,
    EMAC_LOOPBACK_DROP_COUNTER_MASK              = 0x100000,
    TPID_DETECT_DROP_COUNTER_MASK                = 0x200000,
    DUAL_STACK_LITE_CONGESTION_DROP_COUNTER_MASK = 0x400000,
    INVALID_SUBNET_IP_DROP_COUNTER_MASK          = 0x800000,
    EPON_DDR_QUEUEU_DROP_COUNTER_MASK            = 0x800000,
} rdd_various_counters_mask_t;

typedef enum
{
    RDD_SUBNET_FLOW_CACHE = 0,
    RDD_SUBNET_BRIDGE,
    RDD_SUBNET_BRIDGE_IPTV,
    RDD_SUBNET_LAN,
} rdd_subnet_id_t;

typedef enum
{
    RDD_DEI_CMD_TRANSPARENT = 0,
    RDD_DEI_CMD_CLEAR,
    RDD_DEI_CMD_SET,
} rdd_dei_cmd_t;

typedef struct
{
    rdpa_qos_method qos_method; /**< QoS classification method flow / pbit */
    uint8_t wan_flow;  /**< WAN flow : Gem Flow or LLID */
    rdpa_forward_action action; /**< forward/drop/cpu */
    bdmf_index policer; /**< Policer ID */
    rdpa_forwarding_mode forw_mode;  /** < flow/pkt based */
    bdmf_boolean opbit_remark; /**< enable outer pbit remark */
    rdpa_pbit opbit_val;	/**< outer pbit remark value */
    bdmf_boolean ipbit_remark; /**< enable inner pbit remark */
    rdpa_pbit ipbit_val; /**< inner pbit remark value */
    bdmf_boolean dscp_remark; /**< enable dscp remark */
    rdpa_dscp dscp_val; /**< dscp remark value */
    uint8_t ecn_val;
    rdd_vport_id_t egress_port;
    uint8_t wifi_ssid;
    rdd_subnet_id_t subnet_id;
    bdmf_index rate_shaper;
    uint8_t rate_controller_id;
    uint8_t priority;
    uint8_t wan_flow_mapping_table;
    rdpa_qos_method wan_flow_mapping_mode;
    bdmf_boolean service_queue_mode;
    uint8_t service_queue;
    bdmf_boolean qos_rule_wan_flow_overrun; /**< enable overrun wan flow value by qos rule   */
    rdd_dei_cmd_t dei_command;
    bdmf_boolean                  cpu_mirroring;
    union {
        struct {
            uint8_t eth0_vlan_command;
            uint8_t eth1_vlan_command;
            uint8_t eth2_vlan_command;
            uint8_t eth3_vlan_command;
            uint8_t eth4_vlan_command;
            uint8_t eth5_vlan_command;
            uint8_t pci_vlan_command;
        } ds_vlan_command;
        uint8_t  us_vlan_command;
    } vlan_command_id;
#define us_vlan_cmd vlan_command_id.us_vlan_command
#define ds_vlan_cmd vlan_command_id.ds_vlan_command
#define ds_eth0_vlan_cmd vlan_command_id.ds_vlan_command.eth0_vlan_command
#define ds_eth1_vlan_cmd vlan_command_id.ds_vlan_command.eth1_vlan_command
#define ds_eth2_vlan_cmd vlan_command_id.ds_vlan_command.eth2_vlan_command
#define ds_eth3_vlan_cmd vlan_command_id.ds_vlan_command.eth3_vlan_command
#define ds_eth4_vlan_cmd vlan_command_id.ds_vlan_command.eth4_vlan_command
#define ds_pci_vlan_cmd vlan_command_id.ds_vlan_command.pci_vlan_command
#define ds_eth5_vlan_cmd vlan_command_id.ds_vlan_command.eth5_vlan_command
#define ds_eth6_vlan_cmd vlan_command_id.ds_vlan_command.eth6_vlan_command
#define ds_eth7_vlan_cmd vlan_command_id.ds_vlan_command.eth7_vlan_command
#define ds_eth8_vlan_cmd vlan_command_id.ds_vlan_command.eth8_vlan_command
#define ds_eth9_vlan_cmd vlan_command_id.ds_vlan_command.eth9_vlan_command
#define ds_eth10_vlan_cmd vlan_command_id.ds_vlan_command.eth10_vlan_command
#define ds_eth11_vlan_cmd vlan_command_id.ds_vlan_command.eth11_vlan_command
#define ds_eth12_vlan_cmd vlan_command_id.ds_vlan_command.eth12_vlan_command
#define ds_eth13_vlan_cmd vlan_command_id.ds_vlan_command.eth13_vlan_command
#define ds_eth14_vlan_cmd vlan_command_id.ds_vlan_command.eth14_vlan_command
#define ds_eth15_vlan_cmd vlan_command_id.ds_vlan_command.eth15_vlan_command
} rdd_ic_context_t;

typedef enum
{
    RDD_IC_LKP_MODE_IH = 0,
    RDD_IC_LKP_MODE_OPTIMIZED,
    RDD_IC_LKP_MODE_SHORT,
    RDD_IC_LKP_MODE_LONG,
} rdd_ic_lkp_mode_t;

typedef enum
{
    RDD_FC_FWD_ACTION_CPU = 0,
    RDD_FC_FWD_ACTION_DROP,
} rdd_fc_fwd_action_t;

typedef enum
{
    RDD_FULL_FC_ACCELERATION_NON_IP = 0,
    RDD_FULL_FC_ACCELERATION_MCAST_IP,
} rdd_full_fc_acceleration_mode_t;

typedef enum
{
    RDD_MAC_FWD_ACTION_FORWARD = 0,
    RDD_MAC_FWD_ACTION_DROP,
    RDD_MAC_FWD_ACTION_CPU_TRAP0,
    RDD_MAC_FWD_ACTION_CPU_TRAP1,
    RDD_MAC_FWD_ACTION_CPU_TRAP2,
    RDD_MAC_FWD_ACTION_CPU_TRAP3,
    RDD_MAC_FWD_ACTION_RATE_LIMIT,
} rdd_mac_fwd_action_t;

typedef struct
{
    bdmf_mac_t mac_addr;
    uint16_t vlan_id;
    rdd_vport_id_t bridge_port;
    bdmf_boolean entry_type; /* 0 for static, 1 for dynamic */
    bdmf_boolean aggregation_mode; /* 0 for disabled, 1 for enabled */
    uint8_t extension_entry;
    rdd_mac_fwd_action_t sa_action;
    rdd_mac_fwd_action_t da_action;
} rdd_mac_params_t;

typedef enum
{
    RDD_FLOW_PM_COUNTERS_DS = 1,
    RDD_FLOW_PM_COUNTERS_US,
    RDD_FLOW_PM_COUNTERS_BOTH,
} rdd_flow_pm_counters_type_t;

typedef struct
{
    uint32_t good_rx_packet;
    uint32_t good_rx_bytes;
    uint32_t good_tx_packet;
    uint32_t good_tx_bytes;
    uint16_t error_rx_packets_discard;
    uint16_t error_tx_packets_discard;
} rdd_flow_pm_counters_t;

typedef struct
{
    uint32_t good_tx_packet;
    uint16_t error_tx_packets_discard;
} rdd_service_queue_pm_counters_t;

typedef struct
{
    uint32_t rx_valid;
    uint32_t tx_valid;
    uint16_t error_rx_bpm_congestion;
    uint16_t bridge_filtered_packets;
    uint16_t bridge_tx_packets_discard;
} rdd_vport_pm_counters_t;

typedef struct
{
    uint16_t wlan_mcast_index;
    uint16_t egress_port_vector;
    uint8_t ic_context;
    rdpa_iptv_channel_key_t key;
#define mc_key_vid key.vid
#define mc_key_mac key.mcast_group.mac
#define mc_key_gr_ip key.mcast_group.l3.gr_ip
#define mc_key_src_ip key.mcast_group.l3.src_ip
#define mc_wlan_idx wlan_mcast_index
#define mc_egress_port_vector egress_port_vector
#define mc_ic_context ic_context
} rdd_iptv_entry_t;

typedef enum
{
    rdd_cpu_tx_mode_full         = 0,
    rdd_cpu_tx_mode_interworking = 1,
    rdd_cpu_tx_mode_egress_enq   = 2,
} rdd_cpu_tx_mode_t;

typedef enum
{
    rdd_host_buffer   = 0,
    rdd_runner_buffer = 1,
} rdd_buffer_type_t;

typedef struct
{
    rdpa_traffic_dir traffic_dir;
    rdd_cpu_tx_mode_t mode;
    rdd_buffer_type_t buffer_type;

    uint8_t wifi_ssid;
    uint16_t wan_flow;
    rdpa_discard_prty  drop_precedence;

    union
    {
        struct
        {
            rdd_emac_id_t  emac_id;
            rdd_tx_queue_id_t queue_id;
            bdmf_boolean en_1588;
        } ds;

        struct
        {
            rdd_wan_channel_id_t wan_channel;
            rdd_rate_cntrl_id_t rate_controller;
            uint32_t queue;
            rdd_vport_id_t src_bridge_port;
        } us;
    } direction;
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    int is_spdsvc_setup_packet;
#endif
} rdd_cpu_tx_args_t;

#define SET_PRIVATE_MEM_GEN_PTR(dir, table_ptr, table_name, type) \
    do { \
        if (dir == rdpa_dir_ds) \
            table_ptr = (type *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_##table_name##_ADDRESS); \
        else \
            table_ptr = (type *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_##table_name##_ADDRESS); \
    } while (0)

#define SET_DTS_PRIVATE_MEM_GEN_PTR(dir, table_ptr, table_name) \
    SET_PRIVATE_MEM_GEN_PTR(dir, table_ptr, table_name, RDD_##table_name##_DTS)

#define SET_COMMON_MEM_GEN_PTR(dir, table_ptr, table_name, type) \
    do { \
        if (dir == rdpa_dir_ds) \
            table_ptr = (type *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + DS_##table_name##_ADDRESS - sizeof(RUNNER_COMMON)); \
        else \
            table_ptr = (type *)(DEVICE_ADDRESS(RUNNER_COMMON_1_OFFSET) + US_##table_name##_ADDRESS - sizeof(RUNNER_COMMON)); \
    } while (0)

#define SET_DTS_COMMON_MEM_GEN_PTR(dir, table_ptr, table_name) \
    SET_COMMON_MEM_GEN_PTR(dir, table_ptr, table_name, RDD_##table_name##_DTS)

#ifndef BIT_MASK
#define BIT_MASK(a)  (1 << (a))
#endif

typedef enum
{
    RDD_MAC_TABLE_SIZE_32 = 0,
    RDD_MAC_TABLE_SIZE_64,
    RDD_MAC_TABLE_SIZE_128,
    RDD_MAC_TABLE_SIZE_256,
    RDD_MAC_TABLE_SIZE_512,
    RDD_MAC_TABLE_SIZE_1024,
    RDD_MAC_TABLE_SIZE_2048,
    RDD_MAC_TABLE_SIZE_4096,
} rdd_mac_table_size_t;

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
typedef enum
{
    BL_LILAC_RDD_WAN0_BRIDGE_PORT           = 0, /* DSL WAN */
    BL_LILAC_RDD_WAN1_BRIDGE_PORT           = 1, /* ETH WAN */
    BL_LILAC_RDD_LAN0_BRIDGE_PORT           = 2,
    BL_LILAC_RDD_LAN1_BRIDGE_PORT           = 3,
    BL_LILAC_RDD_LAN2_BRIDGE_PORT           = 4,
    BL_LILAC_RDD_LAN3_BRIDGE_PORT           = 5,
    BL_LILAC_RDD_LAN4_BRIDGE_PORT           = 6,
    BL_LILAC_RDD_LAN5_BRIDGE_PORT           = 7,
    BL_LILAC_RDD_LAN6_BRIDGE_PORT           = 8,
    BL_LILAC_RDD_LAN7_BRIDGE_PORT           = 9,
    BL_LILAC_RDD_WAN_ROUTER_PORT            = 17,
    BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT       = 18,
    BL_LILAC_RDD_PCI_BRIDGE_PORT            = 19,
    BL_LILAC_RDD_VIRTUAL_BRIDGE_PORT        = 10,
    BL_LILAC_RDD_CPU_BRIDGE_PORT            = 12,
    BL_LILAC_RDD_WAN_QUASI_BRIDGE_PORT      = 15,
    BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT = 0x10,
    BL_LILAC_RDD_MULTICAST_LAN1_BRIDGE_PORT = 0x20,
    BL_LILAC_RDD_MULTICAST_LAN2_BRIDGE_PORT = 0x40,
    BL_LILAC_RDD_MULTICAST_LAN3_BRIDGE_PORT = 0x80,
    BL_LILAC_RDD_MULTICAST_LAN4_BRIDGE_PORT = 0x100,
    BL_LILAC_RDD_MULTICAST_PCI_BRIDGE_PORT  = 0x200,
} rdd_bridge_port_t;
#define BL_LILAC_RDD_WAN_BRIDGE_PORT BL_LILAC_RDD_WAN0_BRIDGE_PORT
#else
/* XXX: TO BE REMOVED. Everywhere to be replaced with rdd_vport_t or rdd_port_profile_t */
typedef enum
{
    BL_LILAC_RDD_WAN_BRIDGE_PORT            = 0,
    BL_LILAC_RDD_LAN0_BRIDGE_PORT           = 1,
    BL_LILAC_RDD_LAN1_BRIDGE_PORT           = 2,
    BL_LILAC_RDD_LAN2_BRIDGE_PORT           = 3,
    BL_LILAC_RDD_LAN3_BRIDGE_PORT           = 4,
    BL_LILAC_RDD_LAN4_BRIDGE_PORT           = 5,
    BL_LILAC_RDD_LAN5_BRIDGE_PORT           = 6,
    BL_LILAC_RDD_LAN6_BRIDGE_PORT           = 7,
    BL_LILAC_RDD_LAN7_BRIDGE_PORT           = 8,
    BL_LILAC_RDD_WAN_ROUTER_PORT            = 9,
    BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT       = 10,
    BL_LILAC_RDD_PCI_BRIDGE_PORT            = 11,
    BL_LILAC_RDD_G9991_BRIDGE_PORT          = 12,
    BL_LILAC_RDD_VIRTUAL_BRIDGE_PORT        = 13,
    BL_LILAC_RDD_CPU_BRIDGE_PORT            = 14,
    BL_LILAC_RDD_WAN_QUASI_BRIDGE_PORT      = 15,
    BL_LILAC_RDD_MULTICAST_LAN0_BRIDGE_PORT = 0x10,
    BL_LILAC_RDD_MULTICAST_LAN1_BRIDGE_PORT = 0x20,
    BL_LILAC_RDD_MULTICAST_LAN2_BRIDGE_PORT = 0x40,
    BL_LILAC_RDD_MULTICAST_LAN3_BRIDGE_PORT = 0x80,
    BL_LILAC_RDD_MULTICAST_LAN4_BRIDGE_PORT = 0x100,
    BL_LILAC_RDD_MULTICAST_PCI_BRIDGE_PORT  = 0x200,
} rdd_bridge_port_t;
#endif

typedef enum
{
    RDD_CHIP_REVISION_A0 = 0,
    RDD_CHIP_REVISION_B0 = 1,
} rdd_chip_revision_t;

#ifdef WL4908
typedef enum
{
    RDD_CPU_METER_0       = 0,
    RDD_CPU_METER_1       = 1,
    RDD_CPU_METER_2       = 2,
    RDD_CPU_METER_3       = 3,
    RDD_CPU_METER_4       = 4,
    RDD_CPU_METER_5       = 5,
    RDD_CPU_METER_6       = 6,
    RDD_CPU_METER_7       = 7,
    RDD_CPU_METER_8       = 8,
    RDD_CPU_METER_9       = 9,
    RDD_CPU_METER_10      = 10,
    RDD_CPU_METER_11      = 11,
    RDD_CPU_METER_12      = 12,
    RDD_CPU_METER_13      = 13,
    RDD_CPU_METER_14      = 14,
    RDD_CPU_METER_15      = 15,
    RDD_CPU_METER_DISABLE = 16,
} rdd_cpu_meter_t;
#endif
#endif
