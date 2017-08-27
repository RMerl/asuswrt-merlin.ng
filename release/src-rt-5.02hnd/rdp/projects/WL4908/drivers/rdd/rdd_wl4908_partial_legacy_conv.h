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

#define rdd_bridge_port_t BL_LILAC_RDD_BRIDGE_PORT_DTE
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
#define rdd_emac_id_vector_t BL_LILAC_RDD_BRIDGE_PORT_VECTOR_DTE
#define rdd_service_queue_pm_counters_t RDD_SERVICE_QUEUE_PM_COUNTERS_DTE
#define rdd_ether_type_filter_t BL_LILAC_RDD_ETHER_TYPE_FILTER_NUMBER_DTE
#define rdd_ic_context_t rdd_ingress_classification_context_t 
#define rdd_vport_id_t BL_LILAC_RDD_BRIDGE_PORT_DTE
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

#endif

