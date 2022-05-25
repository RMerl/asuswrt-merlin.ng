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

#ifndef _RDD_WL4908_PARTIAL_LEGACY_CONF_H
#define _RDD_WL4908_PARTIAL_LEGACY_CONF_H

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

typedef uint32_t rdd_port_profile_t;

typedef unsigned long long rdd_vport_vector_t;

#define RDD_EMAC_ID_START RDD_EMAC_FIRST
#define RDD_EMAC_ID_LAN_START RDD_EMAC_ID_0

#define rdd_rate_limit_params_t RDD_RATE_LIMIT_PARAMS
#define rdd_lan_vid_cfg_t RDD_LAN_VID_PARAMS
#define rdd_rate_limiter_t RDD_RATE_LIMITER_ID_DTE
#define rdd_inter_lan_schedule_mode_t BL_LILAC_RDD_INTER_LAN_SCHEDULING_MODE_DTE
#define rdd_wan_channel_id_t RDD_WAN_CHANNEL_ID 
#define rdd_rate_cntrl_id_t BL_LILAC_RDD_RATE_CONTROLLER_ID_DTE
#define rdd_subnet_id_t BL_LILAC_RDD_SUBNET_ID_DTE 
#define rdd_ic_lkp_mode_t rdd_ingress_classification_lookup_mode
#define rdd_full_fc_acceleration_mode_t rdd_full_fc_acceleration_mode
#define rdd_mac_params_t RDD_MAC_PARAMS
#define rdd_service_queue_pm_counters_t RDD_SERVICE_QUEUE_PM_COUNTERS_DTE
#define rdd_ether_type_filter_t BL_LILAC_RDD_ETHER_TYPE_FILTER_NUMBER_DTE
#define rdd_ic_context_t rdd_ingress_classification_context_t 
#define rdd_us_wan_flow_cfg rdd_us_wan_flow_config
#define rdd_wan_channel_schedule_t RDD_WAN_CHANNEL_SCHEDULE
#define rdd_peak_schedule_mode_t RDD_US_PEAK_SCHEDULING_MODE
#define rdd_wan_channel_cfg rdd_wan_channel_set
#define rdd_rate_cntrl_params_t RDD_RATE_CONTROLLER_PARAMS
#define rdd_rate_cntrl_cfg rdd_rate_controller_config
#define rdd_rate_cntrl_remove rdd_rate_controller_remove
#define rdd_rate_cntrl_modify rdd_rate_controller_modify
#define rdd_wan_tx_queue_cfg rdd_wan_tx_queue_config
#define rdd_eth_tx_ddr_queue_addr_cfg rdd_eth_tx_ddr_queue_addr_config
#define rdd_queue_profile_t RDD_QUEUE_PROFILE
#define rdd_wan_channel_rate_limiter_cfg rdd_wan_channel_rate_limiter_config
#define rdd_us_overall_rate_limiter_cfg rdd_us_overall_rate_limiter_config
#define rdd_queue_profile_cfg rdd_queue_profile_config
#define rdd_vlan_cmd_param_t rdd_vlan_command_params 
#define rdd_vport_pm_counters_t BL_LILAC_RDD_BRIDGE_PORT_PM_COUNTERS_DTE
#define rdd_cpu_rx_meter BL_LILAC_RDD_CPU_METER_DTE 
#define rdd_drop_precedence_cfg rdd_drop_precedence_config
#define rdd_various_counters_t BL_LILAC_RDD_VARIOUS_COUNTERS_DTE
#define rdd_mtu_cfg rdd_mtu_config
#define rdd_subnet_pm_counters_t BL_LILAC_RDD_SUBNET_PM_COUNTERS_DTE

#define RDD_FC_FWD_ACTION_CPU RDD_FLOW_CACHE_FORWARD_ACTION_CPU
#define RDD_FC_FWD_ACTION_DROP RDD_FLOW_CACHE_FORWARD_ACTION_DROP
#define RDD_ETHER_TYPE_FILTER_USER_0 BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_0 
#define RDD_ETHER_TYPE_FILTER_USER_1 BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_1
#define RDD_ETHER_TYPE_FILTER_USER_2 BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_2
#define RDD_ETHER_TYPE_FILTER_USER_3 BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_3
#define RDD_ETHER_TYPE_FILTER_PPPOE_D BL_LILAC_RDD_ETHER_TYPE_FILTER_PPPOE_D
#define RDD_ETHER_TYPE_FILTER_PPPOE_S BL_LILAC_RDD_ETHER_TYPE_FILTER_PPPOE_S
#define RDD_ETHER_TYPE_FILTER_ARP BL_LILAC_RDD_ETHER_TYPE_FILTER_ARP
#define RDD_ETHER_TYPE_FILTER_1588 BL_LILAC_RDD_ETHER_TYPE_FILTER_1588
#define RDD_ETHER_TYPE_FILTER_802_1X BL_LILAC_RDD_ETHER_TYPE_FILTER_802_1X
#define RDD_ETHER_TYPE_FILTER_802_1AG_CFM BL_LILAC_RDD_ETHER_TYPE_FILTER_802_1AG_CFM
#define RDD_RATE_LIMITER_DISABLED RDD_RATE_LIMITER_IDLE
#define RDD_RATE_LIMITER_PORT_0 RDD_RATE_LIMITER_EMAC_0
#define RDD_PEAK_SCHEDULE_MODE_ROUND_ROBIN RDD_US_PEAK_SCHEDULING_MODE_ROUND_ROBIN
#define RDD_PEAK_SCHEDULE_MODE_STRICT_PRIORITY RDD_US_PEAK_SCHEDULING_MODE_STRICT_PRIORITY
#define RDD_PEAK_SCHEDULE_MODE_STRICT_PRIORITY RDD_US_PEAK_SCHEDULING_MODE_STRICT_PRIORITY
#define RDD_SUBNET_FLOW_CACHE BL_LILAC_RDD_SUBNET_FLOW_CACHE
#define RDD_SUBNET_BRIDGE BL_LILAC_RDD_SUBNET_BRIDGE
#define RDD_PACKET_HEADROOM_OFFSET LILAC_RDD_PACKET_DDR_OFFSET 
#define RDD_PACKET_HEADROOM_OFFSET LILAC_RDD_PACKET_DDR_OFFSET 
#define CPU_RX_METER_DISABLE BL_LILAC_RDD_CPU_METER_DISABLE
#define INVALID_L2_PROTO_DROP_COUNTER_MASK INVALID_LAYER2_PROTOCOL_DROP_COUNTER_MASK
#define IPTV_L3_DROP_COUNTER_MASK IPTV_LAYER3_DROP_COUNTER_MASK
#define RDD_RUNNER_PACKET_BUFFER_SIZE LILAC_RDD_RUNNER_PACKET_BUFFER_SIZE

#define rdpa_wan_type2rdd_egress_phy(src) (rdd_egress_phy_eth_wan)
#define rdd_egress_phy2rdpa_wan_type(src) (rdpa_wan_gbe)

#define rdd_subnet_id_t BL_LILAC_RDD_SUBNET_ID_DTE 
#define RDD_SUBNET_FLOW_CACHE BL_LILAC_RDD_SUBNET_FLOW_CACHE
#define RDD_SUBNET_BRIDGE BL_LILAC_RDD_SUBNET_BRIDGE
#define RDD_SUBNET_BRIDGE_IPTV BL_LILAC_RDD_SUBNET_BRIDGE_IPTV
#define RDD_SUBNET_LAN BL_LILAC_RDD_SUBNET_LAN

/* PORT */

/* Ingress classification */
#define RDD_US_IC_RULE_CFG_TABLE_SIZE RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE
#define RDD_DS_IC_RULE_CFG_TABLE_SIZE RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE
#define RDD_US_IC_CONTEXT_TABLE_SIZE RDD_US_INGRESS_CLASSIFICATION_CONTEXT_TABLE_SIZE
#define RDD_DS_IC_CONTEXT_TABLE_SIZE RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_SIZE

#define rdd_ic_context_t rdd_ingress_classification_context_t 
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

#define rdd_us_ic_default_flows_cfg rdd_us_ingress_classification_default_flows_config

#define rdd_ic_context_cfg rdd_ingress_classification_context_config
#define rdd_ic_context_get rdd_ingress_classification_context_get
#define rdd_ic_context_counter_read rdd_ingress_classification_context_counter_read

#define rdd_ic_rule_cfg_add rdd_ingress_classification_rule_cfg_add 
#define rdd_ic_rule_cfg_delete rdd_ingress_classification_rule_cfg_delete
#define rdd_ic_rule_cfg_modify rdd_ingress_classification_rule_cfg_modify

#define rdd_ic_rule_add rdd_ingress_classification_rule_add
#define rdd_ic_rule_delete rdd_ingress_classification_rule_delete

#define rdd_ic_generic_rule_cfg rdd_ingress_classification_generic_rule_cfg
#define RDD_VLAN_COMMAND_SKIP LILAC_RDD_VLAN_COMMAND_SKIP

#define rdd_ic_lkp_mode_t rdd_ingress_classification_lookup_mode
#define RDD_IC_LKP_MODE_IH rdd_ingress_classification_lookup_mode_ih
#define RDD_IC_LKP_MODE_OPTIMIZED rdd_ingress_classification_lookup_mode_optimized
#define RDD_IC_LKP_MODE_SHORT rdd_ingress_classification_lookup_mode_short
#define RDD_IC_LKP_MODE_LONG rdd_ingress_classification_lookup_mode_long

/* Ingress Filter */
#define rdd_local_switching_filters_cfg rdd_local_switching_filters_config
#define rdd_tpid_detect_filter_value_cfg rdd_tpid_detect_filter_value_config

#define rdd_ingress_filter_action_t BL_LILAC_RDD_FILTER_ACTION_DTE
#define RDD_FILTER_ACTION_CPU_TRAP BL_LILAC_RDD_FILTER_ACTION_CPU_TRAP
#define RDD_FILTER_ACTION_DROP BL_LILAC_RDD_FILTER_ACTION_DROP

#define rdd_ether_type_filter_t BL_LILAC_RDD_ETHER_TYPE_FILTER_NUMBER_DTE
#define RDD_ETHER_TYPE_FILTER_USER_0 BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_0 
#define RDD_ETHER_TYPE_FILTER_USER_1 BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_1
#define RDD_ETHER_TYPE_FILTER_USER_2 BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_2
#define RDD_ETHER_TYPE_FILTER_USER_3 BL_LILAC_RDD_ETHER_TYPE_FILTER_USER_3
#define RDD_ETHER_TYPE_FILTER_PPPOE_D BL_LILAC_RDD_ETHER_TYPE_FILTER_PPPOE_D
#define RDD_ETHER_TYPE_FILTER_PPPOE_S BL_LILAC_RDD_ETHER_TYPE_FILTER_PPPOE_S
#define RDD_ETHER_TYPE_FILTER_ARP BL_LILAC_RDD_ETHER_TYPE_FILTER_ARP
#define RDD_ETHER_TYPE_FILTER_1588 BL_LILAC_RDD_ETHER_TYPE_FILTER_1588
#define RDD_ETHER_TYPE_FILTER_802_1X BL_LILAC_RDD_ETHER_TYPE_FILTER_802_1X
#define RDD_ETHER_TYPE_FILTER_802_1AG_CFM BL_LILAC_RDD_ETHER_TYPE_FILTER_802_1AG_CFM

#define rdpa_filter2rdd_etype_filter rdpa_filter_to_rdd_etype_filter

#define rdd_ingress_filter_t BL_LILAC_RDD_INGRESS_FILTER_DTE 
#define RDD_FILTER_IGMP BL_LILAC_RDD_IGMP_FILTER_NUMBER
#define RDD_FILTER_ICMPV6 BL_LILAC_RDD_ICMPV6_FILTER_NUMBER
#define RDD_FILTER_UDEF_0 BL_LILAC_RDD_USER_0_FILTER_NUMBER
#define RDD_FILTER_UDEF_1 BL_LILAC_RDD_USER_1_FILTER_NUMBER
#define RDD_FILTER_UDEF_2 BL_LILAC_RDD_USER_2_FILTER_NUMBER
#define RDD_FILTER_UDEF_3 BL_LILAC_RDD_USER_3_FILTER_NUMBER
#define RDD_FILTER_PPPOE_D BL_LILAC_RDD_PPPOE_D_FILTER_NUMBER
#define RDD_FILTER_PPPOE_S BL_LILAC_RDD_PPPOE_S_FILTER_NUMBER
#define RDD_FILTER_ARP BL_LILAC_RDD_ARP_FILTER_NUMBER
#define RDD_FILTER_1588 BL_LILAC_RDD_1588_FILTER_NUMBER
#define RDD_FILTER_802_1X BL_LILAC_RDD_802_1X_FILTER_NUMBER
#define RDD_FILTER_802_1AG_CFM BL_LILAC_RDD_802_1AG_CFM_FILTER_NUMBER
#define RDD_FILTER_BROADCAST BL_LILAC_RDD_BROADCAST_FILTER_NUMBER
#define RDD_FILTER_MULTICAST BL_LILAC_RDD_MULTICAST_FILTER_NUMBER
#define RDD_FILTER_LAST BL_LILAC_RDD_INGRESS_FILTERS_NUMBER

/* VLAN Action */
#define rdd_tpid_id_t rdd_tpid_id
#define RDD_TPID_ID_7 rdd_tpid_id_7 

#define rdd_bridge_vlan_cmd_t rdd_bridge_vlan_command
#define RDD_VLAN_CMD_TRANSPARENT rdd_vlan_command_transparent
#define RDD_VLAN_CMD_ADD_TAG rdd_vlan_command_add_tag
#define RDD_VLAN_CMD_REMOVE_TAG rdd_vlan_command_remove_tag 
#define RDD_VLAN_CMD_REPLACE_TAG rdd_vlan_command_replace_tag
#define RDD_VLAN_CMD_ADD_TWO_TAGS rdd_vlan_command_add_two_tags
#define RDD_VLAN_CMD_REMOVE_TWO_TAGS rdd_vlan_command_remove_two_tags
#define RDD_VLAN_CMD_ADD_OUTER_TAG_REPLACE_INNER_TAG rdd_vlan_command_add_outer_tag_replace_inner_tag
#define RDD_VLAN_CMD_REMOVE_OUTER_TAG_REPLACE_INNER_TAG rdd_vlan_command_remove_outer_tag_replace_inner_tag
#define RDD_VLAN_CMD_ADD_TAG_ALWAYS rdd_vlan_command_add_tag_always
#define RDD_VLAN_CMD_REMOVE_TAG_ALWAYS rdd_vlan_command_remove_tag_always
#define RDD_VLAN_CMD_REPLACE_OUTER_TAG_REPLACE_INNER_TAG rdd_vlan_command_replace_outer_tag_replace_inner_tag
#define RDD_VLAN_CMD_REMOVE_OUTER_TAG_COPY rdd_vlan_command_remove_outer_tag_copy
#define RDD_VLAN_CMD_ADD_3RD_TAG rdd_vlan_command_add_3rd_tag
#define RDD_MAX_VLAN_CMD rdd_max_vlan_command

#define rdd_bridge_pbits_cmd_t rdd_bridge_pbits_command
#define RDD_PBITS_CMD_TRANSPARENT rdd_pbits_command_transparent
#define RDD_PBITS_CMD_COPY rdd_pbits_command_copy
#define RDD_PBITS_CMD_CONFIGURED rdd_pbits_command_configured
#define RDD_PBITS_CMD_REMAP rdd_pbits_command_remap
#define RDD_MAX_PBITS_CMD rdd_max_pbits_command

#define rdd_vlan_cmd_param_t rdd_vlan_command_params 

#define rdd_tpid_overwrite_table_cfg rdd_tpid_overwrite_table_config
#define rdd_vlan_cmd_cfg rdd_vlan_command_config

/* Bridge */
#define rdd_bridge_port_t BL_LILAC_RDD_BRIDGE_PORT_DTE
#define rdd_vport_id_t BL_LILAC_RDD_BRIDGE_PORT_DTE

#define rdd_mac_fwd_action_t BL_LILAC_RDD_MAC_FWD_ACTION_DTE
#define RDD_MAC_FWD_ACTION_FORWARD BL_LILAC_RDD_MAC_FWD_ACTION_FORWARD
#define RDD_MAC_FWD_ACTION_DROP BL_LILAC_RDD_MAC_FWD_ACTION_DROP
#define RDD_MAC_FWD_ACTION_CPU_TRAP0 BL_LILAC_RDD_MAC_FWD_ACTION_CPU_TRAP0
#define RDD_MAC_FWD_ACTION_CPU_TRAP1 BL_LILAC_RDD_MAC_FWD_ACTION_CPU_TRAP1
#define RDD_MAC_FWD_ACTION_CPU_TRAP2 BL_LILAC_RDD_MAC_FWD_ACTION_CPU_TRAP2
#define RDD_MAC_FWD_ACTION_CPU_TRAP3 BL_LILAC_RDD_MAC_FWD_ACTION_CPU_TRAP3
#define RDD_MAC_FWD_ACTION_RATE_LIMIT BL_LILAC_RDD_MAC_FWD_ACTION_RATE_LIMIT 

/* XXX: Temporary, due to missing EPON/LLID separation */
#ifndef _RDPA_EPON_H
static inline rdpa_epon_mode _rdpa_epon_mode_get(void)
{
    return rdpa_epon_ctc;
}

static inline bdmf_boolean is_rdpa_epon_ctc_or_cuc_mode(void)
{
    return 1;
}
#endif

#endif

