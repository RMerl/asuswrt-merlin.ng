/*
   Copyright (c) 2014 Broadcom
   All Rights Reserved

    <:label-BRCM:2014:DUAL/GPL:standard
    
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

/*
 * rdpa_types.c
 *
 * RDPA types exposure to the management framework
 *
 *  Created on: Aug 16, 2012
 *      Author: igort
 */


#include "bdmf_dev.h"
#include "rdpa_types.h"
#include "rdpa_common.h"
#include "rdpa_ingress_class_basic.h"
#include "rdpa_filter.h"

/* rdpa_stat aggregate type */
struct bdmf_aggr_type rdpa_stat_aggr_type =
{
    .name = "rdpa_stat", .struct_name = "rdpa_stat_t",
    .help = "Packet and Byte Statistics",
    .fields = (struct bdmf_attr[]) {
        { .name = "packets", .help = "Packets", .size = sizeof(uint32_t), .flags = BDMF_ATTR_UNSIGNED,
          .type = bdmf_attr_number, .offset = offsetof(rdpa_stat_t, packets)
        },
        { .name = "bytes", .help = "Bytes", .size = sizeof(uint32_t), .flags = BDMF_ATTR_UNSIGNED,
          .type = bdmf_attr_number, .offset = offsetof(rdpa_stat_t, bytes)
        },
        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(rdpa_stat_aggr_type);

/* stat type table */
const bdmf_attr_enum_table_t rdpa_stat_type_enum_table =
{
    .type_name = "stat_type", .help = "selects stat type (packets only or packets+bytes)",
    .values =
    {
        {"packets_only", rdpa_stat_packets_only},
        {"packets_and_bytes", rdpa_stat_packets_and_bytes},
        {NULL, 0}
    }
};

/* rdpa_stat_1way aggregate type */
struct bdmf_aggr_type rdpa_stat_1_way_aggr_type =
{
    .name = "rdpa_stat_1way", .struct_name = "rdpa_stat_1way_t",
    .help = "Passed and Discarded Packet and Byte Statistics",
    .fields = (struct bdmf_attr[]) {
        { .name = "passed", .help = "Packets/bytes passed",
          .type = bdmf_attr_aggregate, .ts.aggr_type_name = "rdpa_stat",
          .offset = offsetof(rdpa_stat_1way_t, passed)
        },
        { .name = "discarded", .help = "Packets/bytes discarded",
          .type = bdmf_attr_aggregate, .ts.aggr_type_name = "rdpa_stat",
          .offset = offsetof(rdpa_stat_1way_t, discarded)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(rdpa_stat_1_way_aggr_type);

/* rdpa_stat_tx_rx aggregate type */
struct bdmf_aggr_type rdpa_stat_tx_rx_aggr_type = {
    .name = "rdpa_stat_tx_rx", .struct_name = "rdpa_stat_tx_rx_t",
    .help = "Passed and Discarded Packet and Byte Tx+Rx Statistics",
    .fields = (struct bdmf_attr[]) {
        { .name = "tx", .help = "Transmit",
          .type = bdmf_attr_aggregate, .ts.aggr_type_name = "rdpa_stat_1way",
          .offset = offsetof(rdpa_stat_tx_rx_t, tx)
        },
        { .name = "rx", .help = "Receive",
          .type = bdmf_attr_aggregate, .ts.aggr_type_name = "rdpa_stat_1way",
          .offset = offsetof(rdpa_stat_tx_rx_t, rx)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(rdpa_stat_tx_rx_aggr_type);

/* rdpa_stat_tx_rx_valid aggregate type */
struct bdmf_aggr_type rdpa_stat_tx_rx_valid_aggr_type = {
    .name = "rdpa_stat_tx_rx_valid", .struct_name = "rdpa_stat_tx_rx_valid_t",
    .help = "Passed Packet and Byte Tx+Rx Statistics",
    .fields = (struct bdmf_attr[]) {
        { .name = "tx", .help = "Transmit",
          .type = bdmf_attr_aggregate, .ts.aggr_type_name = "rdpa_stat",
          .offset = offsetof(rdpa_stat_tx_rx_valid_t, tx)
        },
        { .name = "rx", .help = "Receive",
          .type = bdmf_attr_aggregate, .ts.aggr_type_name = "rdpa_stat",
          .offset = offsetof(rdpa_stat_tx_rx_valid_t, rx)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(rdpa_stat_tx_rx_valid_aggr_type);

/* rdpa_dir_index aggregate type */
struct bdmf_aggr_type rdpa_dir_index_aggr_type =
{
    .name = "rdpa_dir_index", .struct_name = "rdpa_dir_index_t",
    .help = "Traffic direction + index",
    .fields = (struct bdmf_attr[]) {
        { .name = "dir", .help = "Direction", .size = sizeof(rdpa_traffic_dir),
          .type = bdmf_attr_enum, .offset = offsetof(rdpa_dir_index_t, dir),
          .ts.enum_table = &rdpa_traffic_dir_enum_table
        },
        { .name = "index", .help = "Bytes", .size = sizeof(bdmf_index),
          .type = bdmf_attr_number, .offset = offsetof(rdpa_dir_index_t, index)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(rdpa_dir_index_aggr_type);

const bdmf_attr_enum_table_t rdpa_if_enum_table =
{
    .type_name = "rdpa_if", .help = "Interface",
    .values = {
        /** WAN ports */
        {"wan0", rdpa_if_wan0},
        {"wan1", rdpa_if_wan1},
        {"wan2", rdpa_if_wan2},

        /** LAN ports */
        {"lan0", rdpa_if_lan0},
        {"lan1", rdpa_if_lan1},
        {"lan2", rdpa_if_lan2},
        {"lan3", rdpa_if_lan3},
        {"lan4", rdpa_if_lan4},
        {"lan5", rdpa_if_lan5},
        {"lan6", rdpa_if_lan6},
        {"lan7", rdpa_if_lan7},
#if !defined(BCM_DSL_RDP) && !defined(BCM63158)
        {"lan8", rdpa_if_lan8},
        {"lan9", rdpa_if_lan9},
        {"lan10", rdpa_if_lan10},
        {"lan11", rdpa_if_lan11},
        {"lan12", rdpa_if_lan12},
        {"lan13", rdpa_if_lan13},
        {"lan14", rdpa_if_lan14},
        {"lan15", rdpa_if_lan15},
        {"lan16", rdpa_if_lan16},
        {"lan17", rdpa_if_lan17},
        {"lan18", rdpa_if_lan18},
        {"lan19", rdpa_if_lan19},
        {"lan20", rdpa_if_lan20},
        {"lan21", rdpa_if_lan21},
#endif
#ifdef G9991
        {"lan22", rdpa_if_lan22},
        {"lan23", rdpa_if_lan23},
        {"lan24", rdpa_if_lan24},
        {"lan25", rdpa_if_lan25},
        {"lan26", rdpa_if_lan26},
        {"lan27", rdpa_if_lan27},
        {"lan28", rdpa_if_lan28},
        {"lan29", rdpa_if_lan29},
#endif

        /** Special ports */
        {"lag0", rdpa_if_lag0},
        {"lag1", rdpa_if_lag1},
        {"lag2", rdpa_if_lag2},
        {"lag3", rdpa_if_lag3},
        {"lag4", rdpa_if_lag4},

        /** Switch aggregate port */
        {"switch", rdpa_if_switch},

#ifndef XRDP
        /** WiFi physical ports */
        {"wlan0", rdpa_if_wlan0},
        {"wlan1", rdpa_if_wlan1},

        /** CPU (local termination) */
        {"cpu",  rdpa_if_cpu},

        /** WiFi logical ports (SSIDs) */
        {"ssid0", rdpa_if_ssid0},
        {"ssid1", rdpa_if_ssid1},
        {"ssid2", rdpa_if_ssid2},
        {"ssid3", rdpa_if_ssid3},
        {"ssid4", rdpa_if_ssid4},
        {"ssid5", rdpa_if_ssid5},
        {"ssid6", rdpa_if_ssid6},
        {"ssid7", rdpa_if_ssid7},
        {"ssid8", rdpa_if_ssid8},
        {"ssid9", rdpa_if_ssid9},
        {"ssid10", rdpa_if_ssid10},
        {"ssid11", rdpa_if_ssid11},
        {"ssid12", rdpa_if_ssid12},
        {"ssid13", rdpa_if_ssid13},
        {"ssid14", rdpa_if_ssid14},
        {"ssid15", rdpa_if_ssid15},
#else
        /** CPU (local termination) */
        {"cpu0", rdpa_if_cpu0},
        {"cpu1", rdpa_if_cpu1},
        {"cpu2", rdpa_if_cpu2},
        {"cpu3", rdpa_if_cpu3},
        {"wlan0", rdpa_if_wlan0},
        {"wlan1", rdpa_if_wlan1},
        {"wlan2", rdpa_if_wlan2},
#endif
        {"bond0", rdpa_if_bond0},
        {"bond1", rdpa_if_bond1},
        {"bond2", rdpa_if_bond2},

        {"any", rdpa_if_any},
        {"none", rdpa_if_none},
        {NULL, 0}
    }
};

const bdmf_attr_enum_table_t rdpa_lan_wan_if_enum_table =
{
    .type_name = "rdpa_if", .help = "Interface",
    .values = {
        /** WAN ports */
        {"wan0", rdpa_if_wan0},
        {"wan1", rdpa_if_wan1},
        {"wan2", rdpa_if_wan2},

        /** LAN ports */
        {"lan0", rdpa_if_lan0},
        {"lan1", rdpa_if_lan1},
        {"lan2", rdpa_if_lan2},
        {"lan3", rdpa_if_lan3},
        {"lan4", rdpa_if_lan4},
        {"lan5", rdpa_if_lan5},
        {"lan6", rdpa_if_lan6},
        {"lan7", rdpa_if_lan7},
#if !defined(BCM_DSL_RDP) && !defined(BCM63158)
        {"lan8", rdpa_if_lan8},
        {"lan9", rdpa_if_lan9},
        {"lan10", rdpa_if_lan10},
        {"lan11", rdpa_if_lan11},
        {"lan12", rdpa_if_lan12},
        {"lan13", rdpa_if_lan13},
        {"lan14", rdpa_if_lan14},
        {"lan15", rdpa_if_lan15},
        {"lan16", rdpa_if_lan16},
        {"lan17", rdpa_if_lan17},
        {"lan18", rdpa_if_lan18},
        {"lan19", rdpa_if_lan19},
        {"lan20", rdpa_if_lan20},
        {"lan21", rdpa_if_lan21},
#endif
#ifdef G9991
        {"lan22", rdpa_if_lan22},
        {"lan23", rdpa_if_lan23},
        {"lan24", rdpa_if_lan24},
        {"lan25", rdpa_if_lan25},
        {"lan26", rdpa_if_lan26},
        {"lan27", rdpa_if_lan27},
        {"lan28", rdpa_if_lan28},
        {"lan29", rdpa_if_lan29},
#endif
        {NULL, 0}
    }
};

const bdmf_attr_enum_table_t rdpa_lan_or_cpu_if_enum_table =
{
    .type_name = "rdpa_if", .help = "Interface",
    .values = {
        /** LAN ports */
        {"lan0", rdpa_if_lan0},
        {"lan1", rdpa_if_lan1},
        {"lan2", rdpa_if_lan2},
        {"lan3", rdpa_if_lan3},
        {"lan4", rdpa_if_lan4}, 

#ifndef XRDP
        /** WiFi logical ports (SSIDs) */
        {"ssid0", rdpa_if_ssid0},
        {"ssid1", rdpa_if_ssid1},
        {"ssid2", rdpa_if_ssid2},
        {"ssid3", rdpa_if_ssid3},
        {"ssid4", rdpa_if_ssid4},
        {"ssid5", rdpa_if_ssid5},
        {"ssid6", rdpa_if_ssid6},
        {"ssid7", rdpa_if_ssid7},
        {"ssid8", rdpa_if_ssid8},
        {"ssid9", rdpa_if_ssid9},
        {"ssid10", rdpa_if_ssid10},
        {"ssid11", rdpa_if_ssid11},
        {"ssid12", rdpa_if_ssid12},
        {"ssid13", rdpa_if_ssid13},
        {"ssid14", rdpa_if_ssid14},
        {"ssid15", rdpa_if_ssid15},
#else
        {"lan5", rdpa_if_lan5}, 
        {"lan6", rdpa_if_lan6}, 
        {"lan7", rdpa_if_lan7},
#if !defined(BCM_DSL_RDP) && !defined(BCM63158)
        {"lan8", rdpa_if_lan8}, 
        {"lan9", rdpa_if_lan9}, 
        {"lan10", rdpa_if_lan10}, 
        {"lan11", rdpa_if_lan11}, 
        {"lan12", rdpa_if_lan12}, 
        {"lan13", rdpa_if_lan13}, 
        {"lan14", rdpa_if_lan14}, 
        {"lan15", rdpa_if_lan15}, 
        {"lan16", rdpa_if_lan16}, 
        {"lan17", rdpa_if_lan17}, 
        {"lan18", rdpa_if_lan18}, 
        {"lan19", rdpa_if_lan19}, 
        {"lan20", rdpa_if_lan20}, 
        {"lan21", rdpa_if_lan21}, 
#endif
#ifdef G9991
        {"lan22", rdpa_if_lan22}, 
        {"lan23", rdpa_if_lan23}, 
        {"lan24", rdpa_if_lan24}, 
        {"lan25", rdpa_if_lan25}, 
        {"lan26", rdpa_if_lan26}, 
        {"lan27", rdpa_if_lan27}, 
        {"lan28", rdpa_if_lan28}, 
        {"lan29", rdpa_if_lan29}, 
#endif

        {"cpu0", rdpa_if_cpu0},
        {"cpu1", rdpa_if_cpu1},
        {"cpu2", rdpa_if_cpu2},
        {"cpu3", rdpa_if_cpu3},
        {"wlan0", rdpa_if_wlan0},
        {"wlan1", rdpa_if_wlan1},
        {"wlan2", rdpa_if_wlan2},
#endif
        {NULL, 0}
    }
};

const bdmf_attr_enum_table_t rdpa_lan_wan_wlan_if_enum_table =
{
    .type_name = "rdpa_if", .help = "Interface",
    .values = {
        /** WAN ports */
        {"wan0", rdpa_if_wan0},
        {"wan1", rdpa_if_wan1},
        {"wan2", rdpa_if_wan2},

        /** LAN ports */
        {"lan0", rdpa_if_lan0},
        {"lan1", rdpa_if_lan1},
        {"lan2", rdpa_if_lan2},
        {"lan3", rdpa_if_lan3},
        {"lan4", rdpa_if_lan4},
        {"lan5", rdpa_if_lan5},
        {"lan6", rdpa_if_lan6},
        {"lan7", rdpa_if_lan7},
#if !defined(BCM_DSL_RDP) && !defined(BCM63158)
        {"lan8", rdpa_if_lan8},
        {"lan9", rdpa_if_lan9},
        {"lan10", rdpa_if_lan10},
        {"lan11", rdpa_if_lan11},
        {"lan12", rdpa_if_lan12},
        {"lan13", rdpa_if_lan13},
        {"lan14", rdpa_if_lan14},
        {"lan15", rdpa_if_lan15},
        {"lan16", rdpa_if_lan16},
        {"lan17", rdpa_if_lan17},
        {"lan18", rdpa_if_lan18},
        {"lan19", rdpa_if_lan19},
        {"lan20", rdpa_if_lan20},
        {"lan21", rdpa_if_lan21},
#endif
#ifdef G9991
        {"lan22", rdpa_if_lan22},
        {"lan23", rdpa_if_lan23},
        {"lan24", rdpa_if_lan24},
        {"lan25", rdpa_if_lan25},
        {"lan26", rdpa_if_lan26},
        {"lan27", rdpa_if_lan27},
        {"lan28", rdpa_if_lan28},
        {"lan29", rdpa_if_lan29},
#endif

        /** Switch port */
        {"switch", rdpa_if_switch},

        /** CPU ports */
#ifdef XRDP
        {"cpu0", rdpa_if_cpu0},
        {"cpu1", rdpa_if_cpu1},
        {"cpu2", rdpa_if_cpu2},
        {"cpu3", rdpa_if_cpu3},
#endif

        /** WLAN ports */
        {"wlan0", rdpa_if_wlan0},
        {"wlan1", rdpa_if_wlan1},
#ifdef XRDP
        {"wlan2", rdpa_if_wlan2},
#endif
        {NULL, 0}
    }
};

const bdmf_attr_enum_table_t rdpa_wlan_ssid_enum_table =
{
    .type_name = "wlan_ssid", .help = "WLAN mcast SSID",
    .values = {
        {"ssid_0", rdpa_wlan_ssid0},
        {"ssid_1", rdpa_wlan_ssid1},
        {"ssid_2", rdpa_wlan_ssid2},
        {"ssid_3", rdpa_wlan_ssid3},
        {"ssid_4", rdpa_wlan_ssid4},
        {"ssid_5", rdpa_wlan_ssid5},
        {"ssid_6", rdpa_wlan_ssid6},
        {"ssid_7", rdpa_wlan_ssid7},
        {"ssid_8", rdpa_wlan_ssid8},
        {"ssid_9", rdpa_wlan_ssid9},
        {"ssid_10", rdpa_wlan_ssid10},
        {"ssid_11", rdpa_wlan_ssid11},
        {"ssid_12", rdpa_wlan_ssid12},
        {"ssid_13", rdpa_wlan_ssid13},
        {"ssid_14", rdpa_wlan_ssid14},
        {"ssid_15", rdpa_wlan_ssid15},
        {NULL, 0}
    }
};

const bdmf_attr_enum_table_t rdpa_emac_enum_table =
{
    .type_name = "rdpa_emac", .help = "EMAC",
    .values = {
        {"emac0", rdpa_emac0},
        {"emac1", rdpa_emac1},
        {"emac2", rdpa_emac2},
        {"emac3", rdpa_emac3},
        {"emac4", rdpa_emac4},
        {"emac5", rdpa_emac5},
        {"emac6", rdpa_emac6},
        {"emac7", rdpa_emac7},
        {"max", rdpa_emac__num_of},
        {"none", rdpa_emac_none},
        {NULL, 0}
    }
};

const bdmf_attr_enum_table_t rdpa_wan_emac_enum_table =
{
    .type_name = "rdpa_emac", .help = "WAN EMAC",
    .values = {
        {"none", rdpa_emac_none},
        {"emac0", rdpa_emac0},
        {"emac1", rdpa_emac1},
        {"emac2", rdpa_emac2},
        {"emac3", rdpa_emac3},
        {"emac4", rdpa_emac4},
        {"emac5", rdpa_emac5},
        {NULL, 0}
    }
};

const bdmf_attr_enum_table_t rdpa_wan_type_enum_table =
{
    .type_name = "rdpa_wan_type", .help = "WAN interface type",
    .values = {
        {"not set", rdpa_wan_none},
        {"gpon", rdpa_wan_gpon},
        {"epon", rdpa_wan_epon},
        {"gbe", rdpa_wan_gbe},
        {"dsl", rdpa_wan_dsl},
#ifdef XRDP
        {"xgpon", rdpa_wan_xgpon},
        {"xepon", rdpa_wan_xepon},
#endif
        {NULL, 0}
    }
};

const bdmf_attr_enum_table_t rdpa_speed_type_enum_table =
{
    .type_name = "rdpa_speed_type", .help = "Active Ethernet Speed",
    .values = {
        {"not set", rdpa_speed_none},
        {"100m", rdpa_speed_100m},
        {"1g", rdpa_speed_1g},
        {"2.5g", rdpa_speed_2_5g},
        {"5g", rdpa_speed_5g},
        {"10g", rdpa_speed_10g},
        {NULL, 0}
    }
};

const bdmf_attr_enum_table_t rdpa_forward_action_enum_table =
{
    .type_name = "rdpa_forward_action", .help = "Forwarding action",
    .values = {
        {"acl", rdpa_forward_action_none},    /* used only for acl flows */
        {"forward", rdpa_forward_action_forward},
        {"drop", rdpa_forward_action_drop},
        {"host", rdpa_forward_action_host},
        {"flood", rdpa_forward_action_flood}, /* used only for bridge DAL */
        {"skip", rdpa_forward_action_skip},   /* used for generic filter for increment counter action only \XRDP_LIMITED */
#ifdef XRDP
        {"drop_low_pri", rdpa_forward_action_drop_low_pri},   /* drop only if ingress filters pass \XRDP_LIMITED */
#endif        
        {NULL, 0}
    }
};

const bdmf_attr_enum_table_t rdpa_filter_action_enum_table =
{
    .type_name = "rdpa_filter_action", .help = "Filter action",
    .values = {
        {"allow", rdpa_filter_action_allow},
        {"deny", rdpa_filter_action_deny},
        {NULL, 0}
    }
};

const bdmf_attr_enum_table_t rdpa_traffic_dir_enum_table =
{
    .type_name = "rdpa_traffic_dir", .help = "Traffic direction",
    .values = {
        {"ds", rdpa_dir_ds},        /**< Downstream */
        {"us", rdpa_dir_us},        /**< Upstream */
        {NULL, 0}
    }
};

const bdmf_attr_enum_table_t rdpa_port_frame_allow_enum_table =
{
    .type_name = "rdpa_port_frame_allow", .help = "Eligible frame type on port",
    .values = {
        {"any", rdpa_port_allow_any},           /**< Allow tagged and untagged frames */
        {"tagged", rdpa_port_allow_tagged},     /**< Allow tagged frames only */
        {"untagged", rdpa_port_allow_untagged}, /**< Allow untagged frames only */
        {NULL, 0}
    }
};

/* qos_methods type enum values */
const bdmf_attr_enum_table_t rdpa_qos_method_enum_table =
{
    .type_name = "rdpa_qos_method", .help = "QoS mapping method",
    .values = {
        {"pbit", rdpa_qos_method_pbit},
        {"flow", rdpa_qos_method_flow},
        {NULL, 0}
    }
};

/* forw_mode type enum values */
const bdmf_attr_enum_table_t rdpa_forward_mode_enum_table =
{
    .type_name = "rdpa_forwarding_mode", .help = "Forwarding mode",
    .values = {
        {"packet", rdpa_forwarding_mode_pkt},
        {"flow", rdpa_forwarding_mode_flow},
        {NULL, 0}
    }
};

/* class_mode type enum values */
const bdmf_attr_enum_table_t rdpa_classify_mode_enum_table =
{
    .type_name = "rdpa_classify_mode", .help = "Classification mode",
    .values = {
        {"packet", rdpa_classify_mode_pkt},
        {"flow", rdpa_classify_mode_flow},
        {NULL, 0}
    }
};

/* discard_prty type enum values */
const bdmf_attr_enum_table_t rdpa_disc_prty_enum_table =
{
    .type_name = "rdpa_discard_prty", .help = "Discard priority",
    .values = {
        {"low", rdpa_discard_prty_low},
        {"high", rdpa_discard_prty_high},
        {NULL, 0}
    }
};

/* dest enum values */
const bdmf_attr_enum_table_t rdpa_flow_dest_enum_table =
{
    .type_name = "rdpa_flow_destination", .help = "Flow destination",
    .values = {
        {"not set", rdpa_flow_dest_none},
        {"iptv", rdpa_flow_dest_iptv},
        {"eth",  rdpa_flow_dest_eth},
        {"omci",  rdpa_flow_dest_omci},
        {NULL, 0}
    }
};

/* ip class mode enum values */
const bdmf_attr_enum_table_t rdpa_ip_class_method_enum_table =
{
    .type_name = "rdpa_ip_class_method", .help = "IP class operational mode",
    .values = {
        {"none", rdpa_method_none},
        {"fc", rdpa_method_fc},
        {"mixed", rdpa_method_mixed},
        {NULL, 0}
    }
};

/* ip version enum values */
const bdmf_attr_enum_table_t rdpa_ip_version_enum_table =
{
    .type_name = "ip_version", .help = "IP protocol version",
    .values = {
        {"other", RDPA_IC_L3_PROTOCOL_OTHER},
        {"ipv4", RDPA_IC_L3_PROTOCOL_IPV4},
        {"ipv6", RDPA_IC_L3_PROTOCOL_IPV6},
        {NULL, 0}
    }
};

/* ingress class result action vector */
const bdmf_attr_enum_table_t rdpa_ic_act_vect_enum_table =
{
    .type_name = "ic_action_vector", .help = "Vector of actions, relevant for ingress class only",
    .values =
    {
    	{"service_q", rdpa_ic_act_service_q},
        {"cpu_mirroring", rdpa_ic_act_cpu_mirroring},
        {"ttl", rdpa_ic_act_ttl},
        {NULL, 0}
    }
};

/* del command vector */
const bdmf_attr_enum_table_t rdpa_ic_dei_command_enum_table =
{
    .type_name = "dei_command", .help = "Set modification of dei bits in TCI",
    .values =
    {
        {"copy", RDPA_IC_DEI_COPY},
        {"clear", RDPA_IC_DEI_CLEAR},
        {"set", RDPA_IC_DEI_SET},
        {NULL, 0}
    }
};

/* del command vector */
const bdmf_attr_enum_table_t rdpa_bpm_buffer_size_enum_table =
{
    .type_name = "bpm_buffer_size", .help = "BPM buffer size",
    .values =
    {
        {"2K", RDPA_BPM_BUFFER_2K},
        {"2.5K", RDPA_BPM_BUFFER_2_5K},
        {"4K", RDPA_BPM_BUFFER_4K},
        {"16K", RDPA_BPM_BUFFER_16K},
        {NULL, 0}
    }
};

struct bdmf_aggr_type filter_ctrl_type =
{
    .name = "filter_ctrl", .struct_name = "rdpa_filter_ctrl_t",
    .help = "Filter control",
    .fields = (struct bdmf_attr[])
    {
        { .name = "enabled", .help = "Enabled", .type = bdmf_attr_boolean,
            .size = sizeof(bdmf_boolean),
            .offset = offsetof(rdpa_filter_ctrl_t, enabled)
        },
        { .name = "action", .help = "Action (drop/trap)", .type = bdmf_attr_enum,
            .ts.enum_table = &rdpa_forward_action_enum_table,
            .size = sizeof(rdpa_forward_action),
            .offset = offsetof(rdpa_filter_ctrl_t, action)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(filter_ctrl_type);

/* 'rdpa_filter': Enumeration table */
const bdmf_attr_enum_table_t rdpa_filter_enum_table =
{
    .type_name = "rdpa_filter", .help = "Filter",
    .values =
    {
        { "dhcp", RDPA_FILTER_DHCP },
        { "igmp", RDPA_FILTER_IGMP },
        { "mld", RDPA_FILTER_MLD },
        { "icmpv6", RDPA_FILTER_ICMPV6 },
        { "etype_udef_0", RDPA_FILTER_ETYPE_UDEF_0 },
        { "etype_udef_1", RDPA_FILTER_ETYPE_UDEF_1 },
        { "etype_udef_2", RDPA_FILTER_ETYPE_UDEF_2 },
        { "etype_udef_3", RDPA_FILTER_ETYPE_UDEF_3 },
        { "etype_pppoe_d", RDPA_FILTER_ETYPE_PPPOE_D },
        { "etype_pppoe_s", RDPA_FILTER_ETYPE_PPPOE_S },
        { "etype_arp", RDPA_FILTER_ETYPE_ARP },
        { "etype_802_1x", RDPA_FILTER_ETYPE_802_1X },
        { "etype_802_1ag_cfm", RDPA_FILTER_ETYPE_802_1AG_CFM },
        { "etype_ptp_1588", RDPA_FILTER_ETYPE_PTP_1588 },
        { "l4_ptp_1588", RDPA_FILTER_L4_PTP_1588 },
        { "mcast_ipv4", RDPA_FILTER_MCAST_IPV4 },
        { "mcast_ipv6", RDPA_FILTER_MCAST_IPV6 },
        { "mcast_l2", RDPA_FILTER_MCAST_L2 },
        { "mcast", RDPA_FILTER_MCAST },
        { "bcast", RDPA_FILTER_BCAST },
        { "oui", RDPA_FILTER_MAC_ADDR_OUI },
        { "hdr_err", RDPA_FILTER_HDR_ERR },
        { "ip_frag", RDPA_FILTER_IP_FRAG },
        { "tpid", RDPA_FILTER_TPID },
        { "mac_spoofing", RDPA_FILTER_MAC_SPOOFING },
        { "ip_mcast_control", RDPA_FILTER_IP_MCAST_CONTROL },
        { "l2cp", RDPA_FILTER_L2CP },
        { NULL, 0 }
    }
};

const bdmf_attr_enum_table_t rdpa_protocol_filters_table =
{
    .type_name = "protocol_filters", .help = "Protocol Filters",
    .values = {
        {"ipv4", rdpa_proto_filter_ipv4},
        {"ipv6", rdpa_proto_filter_ipv6},
        {"pppoe", rdpa_proto_filter_pppoe},
        {"non_ip", rdpa_proto_filter_non_ip},
        {"any", rdpa_proto_filter_any},
        {NULL, 0}
    }
};

/* rdpa_tc type enum values */
const bdmf_attr_enum_table_t rdpa_tc_enum_table =
{
    .type_name = "rdpa_cpu_tc", .help = "CPU Traffic classes",
    .values = {
        {"TC0", rdpa_cpu_tc0},
        {"TC1", rdpa_cpu_tc1},
        {"TC2", rdpa_cpu_tc2},
        {"TC3", rdpa_cpu_tc3},
        {"TC4", rdpa_cpu_tc4},
        {"TC5", rdpa_cpu_tc5},
        {"TC6", rdpa_cpu_tc6},
        {"TC7", rdpa_cpu_tc7},
        {NULL, 0}
    }
};
