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


#ifndef _RDPA_IPTV_H_
#define _RDPA_IPTV_H_

#include "rdpa_types.h"
#include "rdpa_ingress_class.h"

#if !defined(BCM_DSL_XRDP) && !defined(BCM_DSL_RDP) && !defined(RDP_UFC)
#include "rdpa_ip_class.h"
#else
#include "rdpa_ucast.h"
#endif

#include "rdpa_mcast_basic.h"

/** \defgroup iptv IPTV Management
 *  APIs in this group are used for IPTV management
 * - IPTV lookup method
 * - Enable/disable multicast prefix filter (for GPON platforms)
 * - Add/Remove IPTV channels (per LAN port)
 * @{
 */


/** IPTV channel request info.\n
 * This is underlying type for iptv_channel_request aggregate
 */
typedef struct
{
    rdpa_iptv_channel_key_t key; /**< IPTV channel key, identifies single channel in JOIN/LEAVE requests */
    rdpa_ic_result_t mcast_result; /**< Multicast stream (channel) classification result */
    uint16_t  wlan_mcast_index; /**< Index in WLAN multicast clients table >*/ 
} rdpa_iptv_channel_request_t;

/** IPTV channel info.\n
  * This is underlying type for iptv_channel aggregate
  */
typedef struct
{
    rdpa_iptv_channel_key_t key; /**< IPTV channel key, identifies single channel in JOIN/LEAVE requests */
    rdpa_ports ports; /**< LAN ports mask represents the ports currently watching the channel */ 
    rdpa_ic_result_t mcast_result; /**< Multicast stream (channel) classification result */
    uint16_t  wlan_mcast_index; /**< Index in WLAN multicast clients table >*/ 
} rdpa_iptv_channel_t;

/** IPTV Global statistics */
typedef struct
{
    uint32_t rx_valid_pkt; /**< Valid Received IPTV packets */
    uint32_t rx_valid_bytes; /**< Valid Received IPTV bytes */
    uint32_t rx_crc_error_pkt; /**< Received packets with CRC error */
    uint32_t discard_pkt; /**< IPTV Discard Packets */
    uint32_t iptv_lkp_miss_drop;           /**< Drop due to IPTV Hash lookup miss (DA \ DIP) */
    uint32_t iptv_src_ip_vid_lkp_miss_drop;     /**< Drop due to IPTV channel Source IP \ VID lookup miss */
    uint32_t iptv_invalid_ctx_entry_drop;       /**< Drop due to IPTV channel invalid ctx entry */
    uint32_t iptv_fpm_sbpm_alloc_nack_drop;     /**< Drop due to exhaustion of FPM buffers */
    uint32_t iptv_first_repl_disp_nack_drop;    /**< Drop due to unavilable dispatcher buffer - first replication */
    uint32_t iptv_exception_drop;               /**< Drop due to IPTV exception */
    uint32_t iptv_other_repl_disp_nack_drop;    /**< Drop due to unavilable dispatcher buffer - other replication */
    uint32_t discard_bytes;                     /**< IPTV total discard packets length in bytes */
#ifdef BCM6858
    uint32_t iptv_congestion_drop;              /**< IPTV congestion drops */
#endif
    uint32_t iptv_fpm_below_threshold_drop;     /**< Drop due to FPM buffers below threshold */
} rdpa_iptv_stat_t;

/** IPTV Chanel key */
typedef struct {
    bdmf_index channel_index; /**< Channel index */
    rdpa_if port; /**< Port */
} rdpa_channel_req_key_t;

#define RDPA_MAX_IPTV_CHANNELS 1024
#define RDPA_MAX_IPTV_FLOWS 1024

#if defined(BCM_PON_XRDP) && !defined(RDP_UFC)
/** FC Multicast flow definition (key + result).\n
 */
typedef struct {
    uint32_t  hw_flow_id;            /**< Multicast flow HW flow ID */
    rdpa_fc_mcast_flow_key_t key;    /**< FC Multicast flow key */
    rdpa_ip_flow_result_t result;    /**< FC Multicast flow result */
} rdpa_fc_mcast_flow_t;
#endif

/** @} end of iptv Doxygen group */

/** HW flow modes */
typedef enum
{
	rdpa_iptv_flow_mode_single, /**< Single flow mode */
	rdpa_iptv_flow_mode_multi, /**< Multi-flow mode */
} rdpa_iptv_flow_mode_t;

#endif /* _RDPA_IPTV_H_ */

