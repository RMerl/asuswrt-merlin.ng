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

#ifndef _RDPA_L2_COMMON_H_
#define _RDPA_L2_COMMON_H_

#include <bdmf_interface.h>

/** \addtogroup l2_class
 * @{
 */

/** L2 flow key.\n
 * This key is used to classify traffic.\n
 */
typedef struct {
    bdmf_mac_t src_mac;  /**< Source MAC address */
    bdmf_mac_t dst_mac;  /**< Destination MAC address */
    uint32_t vtag0;      /**< VLAN tag 0 */
    uint32_t vtag1;      /**< VLAN tag 1 */
    uint8_t vtag_num;    /**< Number of vlan tags */
    uint16_t eth_type;   /**< Ether Type */
    uint8_t tos;         /**< ToS */
    rdpa_traffic_dir dir;/**< Traffic direction */
    rdpa_if ingress_if;  /**< Ingress RDPA interface */
    uint8_t lookup_port; /**< Ingress bridge port \RDP_LIMITED */
    uint16_t wan_flow;   /**< WAN Flow, used f ingress port is wan (e.g. gem_flow), ignored otherwise \XRDP_LIMITED */
    uint8_t tcp_pure_ack;/**< TCP pure ack flow */
} rdpa_l2_flow_key_t;

typedef enum
{
    rdpa_l2_flow_key_exclude_ecn,
    rdpa_l2_flow_key_exclude_dscp,
    rdpa_l2_flow_key_exclude_dei,
} rdpa_l2_flow_key_exclude_t;

/** Excluded fields from L2 flow lookup key, combined to mask. */
typedef enum
{
    rdpa_l2_flow_key_exclude_ecn_field = (1 << rdpa_l2_flow_key_exclude_ecn),    /**< Exclude ECN field from the key */
    rdpa_l2_flow_key_exclude_dscp_field = (1 << rdpa_l2_flow_key_exclude_dscp),  /**< Exclude DSCP field from the key */
    rdpa_l2_flow_key_exclude_dei_field = (1 << rdpa_l2_flow_key_exclude_dei),    /**< Exclude DEI field from the key */
} rdpa_l2_flow_key_exlcude_field_t;

typedef uint32_t rdpa_l2_flow_key_exclude_fields_t; /**< Mask of \ref rdpa_l2_flow_key_exclude_fields_t (excluded fields) */

/** @} end of l2_class Doxygen group. */

#endif /* _RDPA_L2_COMMON_H_ */
