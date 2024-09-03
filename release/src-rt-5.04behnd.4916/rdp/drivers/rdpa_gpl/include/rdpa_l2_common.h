/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
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
    bdmf_object_handle port_ingress_obj;  /**< RDPA ingress Interface */
#if defined(BCM_DSL_RDP)
    uint8_t ingress_radio_idx;
#endif
                        /* rdpa fw support up to 256 flows */
    uint8_t wan_flow;   /**< WAN Flow, used if ingress port is wan (e.g. gem_flow), ignored otherwise \XRDP_LIMITED */
    uint8_t client_idx;  /**< Client index used for  L2 flooding, Zeroed otherwise \XRDP_LIMITED */
    uint8_t tcp_pure_ack;/**< TCP pure ack flow */
    uint8_t is_llc_snap; /**< LLC_SNAP flag */
    uint8_t is_ctx_ext;
#if defined(CONFIG_RUNNER_FPI)
    rdpa_fpi_mode_t fpi_mode;
#endif
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
