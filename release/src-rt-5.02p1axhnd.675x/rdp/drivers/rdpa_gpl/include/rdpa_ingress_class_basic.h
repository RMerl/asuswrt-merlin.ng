/*
 * <:copyright-BRCM:2013:DUAL/GPL:standard
 * 
 *    Copyright (c) 2013 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */

#ifndef _RDPA_INGRESS_CLASS_BASIC_H_
#define _RDPA_INGRESS_CLASS_BASIC_H_

#include "bdmf_interface.h"
#include "rdpa_cpu_basic.h"

/** \addtogroup ingress_class Ingress Classification
 *  Ingress classifier supports up to 16 classifiers per direction
 *
 * @{
 */

/* Actions of the optional actions vector */
typedef enum {
    /* 4 bytes fields */
    RDPA_IC_FIRST_4_BYTE_KEY = 0,
    RDPA_IC_SRC_IP = RDPA_IC_FIRST_4_BYTE_KEY,
    RDPA_IC_DST_IP,
    RDPA_IC_IPV6_FLOW_LABEL,
    /* Generic ic values used also to field id */
    RDPA_IC_GENERIC_1,
    RDPA_IC_GENERIC_L2 = RDPA_IC_GENERIC_1,
    RDPA_IC_GENERIC_2,
    RDPA_IC_GENERIC_L3 = RDPA_IC_GENERIC_2,
    RDPA_IC_GENERIC_MASK,
    RDPA_IC_GENERIC_L4 = RDPA_IC_GENERIC_MASK,
    RDPA_IC_GENERIC_2_MASK, 
    /* 2 bytes fields */
    RDPA_IC_FIRST_2_BYTE_KEY,
    RDPA_IC_OUTER_TPID = RDPA_IC_FIRST_2_BYTE_KEY,
    RDPA_IC_INNER_TPID,
    RDPA_IC_SRC_PORT,
    RDPA_IC_DST_PORT,
    RDPA_IC_OUTER_VID,
    RDPA_IC_INNER_VID,
    RDPA_IC_DST_MAC,
    RDPA_IC_SRC_MAC,
    RDPA_IC_ETHER_TYPE,
    /*1 byte fields*/
    RDPA_IC_FIRST_1_BYTE_KEY,
    RDPA_IC_IP_PROTOCOL = RDPA_IC_FIRST_1_BYTE_KEY,
#if defined(XRDP) || defined(BCM_XRDP)
    RDPA_IC_TOS,
#endif
    RDPA_IC_DSCP,
    RDPA_IC_SSID,
    RDPA_IC_INGRESS_PORT,
    RDPA_IC_OUTER_PBIT,
    RDPA_IC_INNER_PBIT,
    RDPA_IC_NUM_OF_VLANS,
    RDPA_IC_L3_PROTOCOL, 
    RDPA_IC_GEM_FLOW,
    RDPA_IC_NETWORK_LAYER,
    RDPA_IC_ANY,
    RDPA_IC_LAST_KEY,
} rdpa_ic_value;

/** Ingress classification rule mask fields bitmask */
typedef enum
{
    RDPA_IC_MASK_SRC_IP          = (1 << RDPA_IC_SRC_IP),          /**< Source IP address */
    RDPA_IC_MASK_DST_IP          = (1 << RDPA_IC_DST_IP),          /**< Destination IP address */
    RDPA_IC_MASK_IPV6_FLOW_LABEL = (1 << RDPA_IC_IPV6_FLOW_LABEL), /**< IPv6 Flow Label field */
    RDPA_IC_MASK_GENERIC_1       = (1 << RDPA_IC_GENERIC_1),       /**< Generic key 0 (type L2/L3/L4, offset, mask */
    RDPA_IC_MASK_GENERIC_2       = (1 << RDPA_IC_GENERIC_2),       /**< Generic key 1 (type L2/L3/L4, offset, mask */
    RDPA_IC_MASK_GENERIC_MASK    = (1 << RDPA_IC_GENERIC_MASK),    /**< Generic key 0 mask (per flow)*/
    RDPA_IC_MASK_GENERIC_2_MASK  = (1 << RDPA_IC_GENERIC_2_MASK),  /**< Generic key 1 mask (per flow)*/
    RDPA_IC_MASK_OUTER_TPID      = (1 << RDPA_IC_OUTER_TPID),      /**< Outer TPID */
    RDPA_IC_MASK_INNER_TPID      = (1 << RDPA_IC_INNER_TPID),      /**< Inner TPID */
    RDPA_IC_MASK_SRC_PORT        = (1 << RDPA_IC_SRC_PORT),        /**< Source port */
    RDPA_IC_MASK_DST_PORT        = (1 << RDPA_IC_DST_PORT),        /**< Destination port */
    RDPA_IC_MASK_OUTER_VID       = (1 << RDPA_IC_OUTER_VID),       /**< Outer VID */
    RDPA_IC_MASK_INNER_VID       = (1 << RDPA_IC_INNER_VID),       /**< Inner VID */
    RDPA_IC_MASK_DST_MAC         = (1 << RDPA_IC_DST_MAC),         /**< Destination MAC address */
    RDPA_IC_MASK_SRC_MAC         = (1 << RDPA_IC_SRC_MAC),         /**< Source MAC address */
    RDPA_IC_MASK_ETHER_TYPE      = (1 << RDPA_IC_ETHER_TYPE),      /**< Ether Type */
    RDPA_IC_MASK_IP_PROTOCOL     = (1 << RDPA_IC_IP_PROTOCOL),     /**< Protocol */
#if defined(XRDP) || defined(BCM_XRDP)
    RDPA_IC_MASK_TOS             = (1 << RDPA_IC_TOS),             /**< IP Type of Service */
#endif
    RDPA_IC_MASK_DSCP            = (1 << RDPA_IC_DSCP),            /* IP Differentiated Services Code Point */
    RDPA_IC_MASK_SSID            = (1 << RDPA_IC_SSID),            /**< Ingress SSID */
    RDPA_IC_MASK_INGRESS_PORT    = (1 << RDPA_IC_INGRESS_PORT),    /**< LAN port */
    RDPA_IC_MASK_OUTER_PBIT      = (1 << RDPA_IC_OUTER_PBIT),      /**< Outer PBIT */
    RDPA_IC_MASK_INNER_PBIT      = (1 << RDPA_IC_INNER_PBIT),      /**< Inner PBIT */
    RDPA_IC_MASK_NUM_OF_VLANS    = (1 << RDPA_IC_NUM_OF_VLANS),    /**< Number of VLAN in packet */
    RDPA_IC_MASK_L3_PROTOCOL     = (1 << RDPA_IC_L3_PROTOCOL),     /**< L3 Protocol (Other-0, IPv4-1, IPv6-2) */
    RDPA_IC_MASK_GEM_FLOW        = (1 << RDPA_IC_GEM_FLOW),        /**< GEM / LLID */
    RDPA_IC_MASK_NETWORK_LAYER   = (1 << RDPA_IC_NETWORK_LAYER),   /**< 1 do on L3 / 0 do on L2 */
    RDPA_IC_MASK_ANY             = (1 << RDPA_IC_ANY),             /**< Match All packets */
} rdpa_ic_fields;

/** Ingress classification rule type */
typedef enum
{
    RDPA_IC_TYPE_ACL = 1, /**< Classification type ACL */
    RDPA_IC_TYPE_FLOW,    /**< Classification type Flow */
    RDPA_IC_TYPE_QOS,     /**< Classification type QoS */
    RDPA_IC_TYPE_GENERIC_FILTER, /**< Classification type Generic Filter */
    RDPA_IC_TYPE_IP_FLOW = RDPA_IC_TYPE_GENERIC_FILTER,
    RDPA_IC_TYPE_NUM = RDPA_IC_TYPE_GENERIC_FILTER,
} rdpa_ic_type;

/** Ingress classification rule type */
typedef enum
{
    RDPA_IC_L3_PROTOCOL_OTHER = 0, /**< IC L3 Protocol field = Other */
    RDPA_IC_L3_PROTOCOL_IPV4  = 1, /**< IC L3 Protocol field = IPv4 */
    RDPA_IC_L3_PROTOCOL_IPV6  = 2  /**< IC L3 Protocol field = IPv6 */
} rdpa_ic_l3_protocol;

/** Generic field configuration */
typedef struct
{
    rdpa_offset_t type; /**< Packet offset type ::rdpa_offset_t */
    uint32_t offset; /**< Packet offset, must be 2-bytes aligned */ 
    uint32_t mask; /* 4-byte key binary mask */
} rdpa_ic_gen_rule_cfg_t;

/** Ingress classification dei command */
typedef enum 
{
    RDPA_IC_DEI_COPY  = 0,
    RDPA_IC_DEI_CLEAR = 1,
    RDPA_IC_DEI_SET   = 2
} rdpa_ic_dei_command;

/** force flow type */
typedef enum
{
    RDPA_L3 = 0,         /**< Force L3 type */
    RDPA_L2 = 1,         /**< Force L2 type */
} rdpa_network_layer_type_t;

/** traffic level */
typedef enum
{
    RDPA_ALL_TRAFFIC = 0,          /**< All traffic */
    RDPA_FLOW_MISSED_TRAFFIC = 1,  /**< Only Flow missed traffic */
    RDPA_NUM_OF_FILTER_LOCATIONS = 2
} rdpa_filter_location_t;

/** Ingress classification flow key 
 * This key is used to classify traffic.\n
 */
typedef struct
{
    bdmf_ip_t                   src_ip;           /**< Source ipv4/ipv6 IP */
    bdmf_ip_t                   src_ip_mask;      /**< Source ipv4/ipv6 IP mask \XRDP_LIMITED */
    bdmf_ip_t                   dst_ip;           /**< Destination ipv4/ipv6 IP */
    bdmf_ip_t                   dst_ip_mask;      /**< Destination ipv4/ipv6 IP mask \XRDP_LIMITED */
    uint16_t                    src_port;         /**< Source port */
    uint16_t                    src_port_mask;    /**< Source port mask \XRDP_LIMITED */
    uint16_t                    dst_port;         /**< Destination port */
    uint16_t                    dst_port_mask;    /**< Destination port mask \XRDP_LIMITED */
    uint8_t                     protocol;         /**< IP protocols. For example, UDP(17) */
    uint16_t                    outer_vid;        /**< Outer VID */
    uint16_t                    inner_vid;        /**< Inner VID */
    bdmf_mac_t                  dst_mac;          /**< DA MAC address */
    bdmf_mac_t                  dst_mac_mask;     /**< DA MAC address mask \XRDP_LIMITED */
    bdmf_mac_t                  src_mac;          /**< SA MAC address */
    bdmf_mac_t                  src_mac_mask;     /**< SA MAC address mask \XRDP_LIMITED */
    uint16_t                    etype;            /**< Ethernet type */
    uint8_t                     tos;              /**< TOS val \XRDP_LIMITED */
    uint8_t                     tos_mask;         /**< TOS val mask \XRDP_LIMITED */
    uint8_t                     dscp;             /*   DSCP val */
    uint8_t                     ssid;             /**< Wi-Fi SSID */
    rdpa_if                     ingress_port;     /**< US - ingress port index */
    uint8_t                     outer_pbits;      /**< Outer PBIT */
    uint8_t                     outer_pbits_mask; /**< Outer PBIT mask \XRDP_LIMITED */
    uint8_t                     inner_pbits;      /**< Inner PBIT */
    uint8_t                     inner_pbits_mask; /**< Inner PBIT mask \XRDP_LIMITED */
    uint8_t                     number_of_vlans;  /**< Number of VLANs */
    uint32_t                    ipv6_flow_label;  /**< IPv6 Flow Label field */
    uint16_t                    outer_tpid;       /**< Outer TPID */
    uint16_t                    inner_tpid;       /**< Inner TPID */
    rdpa_ic_l3_protocol         l3_protocol;      /**< L3 protocol (other, IPv4, IPv6) */
    uint32_t                    generic_key_1;    /**< Key for first generic field matching ::rdpa_ic_gen_rule_cfg_t */
    uint32_t                    generic_key_2;    /**< Key for second generic field matching ::rdpa_ic_gen_rule_cfg_t */
    uint8_t                     gem_flow;         /**< DS- GEM or LLID index */
    uint32_t                    generic_mask;     /**< Mask per flow for generic key 1 \XRDP_LIMITED */
    uint32_t                    generic_mask_2;   /**< Mask per flow for generic key 2 \XRDP_LIMITED */
    rdpa_network_layer_type_t   network_layer;    /**< Run generic on l3 or l2 \XRDP_LIMITED */
} rdpa_ic_key_t;

/** Actions of the optional actions vector */
typedef enum
{
    rdpa_ic_act_none, /**< action none */
    rdpa_ic_act_service_q, /**< action service queue */
    rdpa_ic_act_cpu_mirroring, /**< action cpu_mirroring */
    rdpa_ic_act_ttl  /**< action ttl \XRDP_LIMITED */
}
rdpa_ic_action;

/** Vector of \ref rdpa_ic_action_vector_t "actions". All configured actions are applied on the flow entry */
typedef uint16_t rdpa_ic_action_vec_t;

/** Ingress classification flow result */
typedef struct
{
    rdpa_qos_method qos_method; /**< QoS classification method flow / pbit */
    uint8_t wan_flow;  /**< WAN flow : Gem Flow or LLID */
    rdpa_forward_action  action; /**< forward/drop/cpu */
    bdmf_object_handle policer; /**< Policer object */
    rdpa_forwarding_mode forw_mode;  /**< flow/packet based */
    rdpa_if egress_port; /**< Egress port */
    uint8_t ssid; /**< SSID, in use when egress port is WLAN */
    uint32_t queue_id; /**< Egress queue ID if forwarding is flow based. Traffic class if forwarding is packet based */
    bdmf_object_handle vlan_action; /**< VLAN action, can be overwritten by VLAN action per egress port */
    bdmf_boolean opbit_remark; /**< Enable outer PBIT remark */
    rdpa_pbit opbit_val;	/**< Outer PBIT remark value */
    bdmf_boolean ipbit_remark; /**< Enable inner PBIT remark */
    rdpa_pbit ipbit_val; /**< Inner PBIT remark value */
    bdmf_boolean dscp_remark; /**< Enable DSCP remark */
    rdpa_dscp dscp_val; /**< DSCP remark value */
    uint8_t ecn_val;
    bdmf_object_handle pbit_to_gem_table; /**< pBit to GEM Mapping table handle. Set null for flow based mapping */
    rdpa_ic_action_vec_t action_vec; /**< Enabled action to be performed on the matched flow. */
    bdmf_index service_q_id; /**< Service queue ID, for none: service_q_id=-1 */
    rdpa_ic_dei_command dei_command; /**< Set the DEI bits modification **/
    rdpa_cpu_reason trap_reason; /**< Trap reason */
    bdmf_boolean include_mcast; /**< Include mcast flow flag */
    bdmf_boolean loopback; /**< Enable WAN loopback \XRDP_LIMITED */
    bdmf_boolean disable_stat; /**< Disable stat - true/false  \XRDP_LIMITED */
    rdpa_stat_type stat_type; /**< in case disable_stat==0, select packets only or packets + bytes  \XRDP_LIMITED */
} rdpa_ic_result_t;

/** @} end of ingress_classification Doxygen group. */

#define RDPA_IC_PHY_DS_PQ_MAX    4
#endif /* _RDPA_INGRESS_CLASS_BASIC_H_ */
