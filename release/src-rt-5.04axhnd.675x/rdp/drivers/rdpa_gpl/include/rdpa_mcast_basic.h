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

#ifndef _RDPA_MCAST_BASIC_H_
#define _RDPA_MCAST_BASIC_H_

/** \addtogroup iptv
 * @{
 */


/** L3 group address */
typedef struct
{
    bdmf_ip_t gr_ip;    /**< Multicast Group IP address */
    bdmf_ip_t src_ip;   /**< Multicast Source IP address. Can be specific IP (IGMPv3/MLDv2) or 0 (IGMPv2/MLDv1) */
} rdpa_iptv_l3_t;

typedef enum
{
    rdpa_mcast_flow_key_exclude_pbit,
    rdpa_mcast_flow_key_exclude_dei,
    rdpa_mcast_flow_key_exclude_etype,
} rdpa_mcast_flow_key_exclude_t;

/** Excluded fields from mcast flow lookup key, combined to mask. */
typedef enum
{
    rdpa_mcast_flow_key_exclude_pbit_field = (1 << rdpa_mcast_flow_key_exclude_pbit),    /**< Exclude PBIT field from the key */
    rdpa_mcast_flow_key_exclude_dei_field = (1 << rdpa_mcast_flow_key_exclude_dei),  /**< Exclude DEI field from the key */
    rdpa_mcast_flow_key_exclude_etype_field = (1 << rdpa_mcast_flow_key_exclude_etype),    /**< Exclude ETYPE field from the key */
} rdpa_mcast_flow_key_exclude_field_t;

/** IPTV Multicast prefix filter method */
typedef enum
{
    rdpa_mcast_filter_method_none, /**< none multicast filter method */
    rdpa_mcast_filter_method_mac, /**< For mac multicast filter method */
    rdpa_mcast_filter_method_ip, /**< For IP multicast filter method */
    rdpa_mcast_filter_method_mac_and_ip /**< For mac and IP together multicast filter method \XRDP_LIMITED*/
} rdpa_mcast_filter_method;

/** IPTV entries lookup method */
typedef enum
{
    iptv_lookup_method_mac, /**< Perform IPTV entry lookup by MAC address (L2) */
    iptv_lookup_method_mac_vid, /**< Perform IPTV entry lookup by MAC address and VID (L2) */
    iptv_lookup_method_group_ip, /**< Perform IPTV entry lookup by Multicast Group IP address (IGMPv2/MLDv1) */
    iptv_lookup_method_group_ip_src_ip, /**< Perform IPTV entry lookup by Multicast Group IP and Source IP
                                             addresses (IGMPv3/MLDv2). Source IP address is optional. */
    iptv_lookup_method_group_ip_src_ip_vid /**< Perform IPTV entry lookup by Multicast Group IP and Source IP
                                                addresses and VID. Source IP address is optional. */
} rdpa_iptv_lookup_method;

#define RDPA_MCAST_MULTI_FLOW_MAX_CLIENTS 256 /* number of clients should be multiple of 32, otherwiise it requires changes in code */
#define RDPA_MASTER_MC_FLOW_CACHE_CONTEXT_ENTRY_CLIENT_IDX_VECTOR_NUMBER (RDPA_MCAST_MULTI_FLOW_MAX_CLIENTS/8/sizeof(uint32_t))

/** Multicast group address */
typedef struct
{
    bdmf_mac_t mac;     /**< Multicast Group MAC Address, used for L2 multicast support */
    rdpa_iptv_l3_t l3;  /**< Multicast L3 address, used for L3 multicast support */
} rdpa_mcast_group_t;

/** IPTV channel key.\n
  * This type is used for API's for add/delete channels.
  */
typedef struct rdpa_iptv_channel_key
{
    rdpa_mcast_group_t mcast_group; /**< Multicast Group Address Can be either L2 or L3 */
    uint16_t vid;                   /**< VID used for multicast stream (channel). */
#if defined(CONFIG_RUNNER_IPTV_LKUP_KEY_INCLUDE_SRC_PORT)
    rdpa_if rx_if;                  /**< Received Interface (For Multi Wan only)*/
#endif
#if defined(BCM_DSL_XRDP) || defined(RDP_UFC)
    uint16_t inner_vid;
    uint8_t num_vlan_tags;
#endif
} rdpa_iptv_channel_key_t;

/** FC Multicast flow key.\n
 * The flow key is used to classify Multicast traffic for single replication.\n
 * - Setting a flow Key inner or outer VLAN ID to 0xFFFF indicates
 * that the corresponding VLAN IDs of Rx packets should be ignored
 * when trying to match Rx packets to the respective flow (wildcard).
 */
typedef struct {
    rdpa_iptv_channel_key_t key; /**< Channel Key */
    uint16_t entry_idx; /**< Client entry index */
    uint8_t num_vlan_tags;  /**< Number of VLAN Tags */
    uint32_t outer_vlan; /**< Outer VLAN (VID + Pbit) */
    uint32_t inner_vlan; /**< Inner VLAN (VID + Pbit) */
    uint16_t etype; /**< Ether type */
    uint8_t tos; /**< desired TOS fields (DSCP/TOS) */
    rdpa_if rx_if;          /**< Received Interface */
} rdpa_fc_mcast_flow_key_t;

/** @} end of mcast Doxygen group. */

#endif /* _RDPA_MCAST_BASIC_H_ */
