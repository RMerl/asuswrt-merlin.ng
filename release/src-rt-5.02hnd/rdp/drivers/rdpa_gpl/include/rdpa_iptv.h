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

/** \defgroup mcast Multicast Management
 *   
 *  This section describes the objects responsible for the management of multicast traffic. 
 *  
 *  The IPTV Management object is responsible for packet replication. When a packet is needed to be replicated to a 
 *  Wi-Fi station by setting the egress_port field of the ::rdpa_ic_result_t structure, the WLAN multicast Flow Management 
 *  object must also be configured with the corresponding parameters. 
 *         
 */


/** \defgroup iptv IPTV Management
 *  \ingroup mcast       
 * APIs in this group are used for IPTV management
 * - IPTV lookup method
 * - Enable/disable multicast prefix filter (for GPON platforms)
 * - Add/Remove IPTV channels (per LAN port)
 * @{
 */

/** IPTV Multicast prefix filter method */
typedef enum
{
    rdpa_mcast_filter_method_none, /**< none multicast filter method */
    rdpa_mcast_filter_method_mac, /**< For mac multicast filter method */
    rdpa_mcast_filter_method_ip, /**< For IP multicast filter method */
} rdpa_mcast_filter_method;


/** L3 group address */
typedef struct
{
    bdmf_ip_t gr_ip;    /**< Multicast Group IP address */
    bdmf_ip_t src_ip;   /**< Multicast Source IP address. Can be specific IP (IGMPv3/MLDv2) or 0 (IGMPv2/MLDv1) */
} rdpa_iptv_l3_t;

/** Multicast group address */
typedef struct
{
    bdmf_mac_t mac;     /**< Multicast Group MAC Address, used for L2 multicast support */
    rdpa_iptv_l3_t l3;  /**< Multicast L3 address, used for L3 multicast support */
} rdpa_mcast_group_t;

/** IPTV channel key.\n
  * This type is used for APIes for add/delete channels.
  */
typedef struct
{
    rdpa_mcast_group_t mcast_group; /**< Multicast Group Address Can be either L2 or L3 */
    uint16_t vid;                   /**< VID used for multicast stream (channel). Can be specific VID or ANY */
} rdpa_iptv_channel_key_t;

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
typedef struct {
   uint32_t rx_valid_pkt; /**< Valid Received IPTV packets */
   uint32_t rx_crc_error_pkt; /**< Received packets with CRC error */
   uint32_t discard_pkt; /**< IPTV Discard Packets */    
} rdpa_iptv_stat_t;

typedef struct {
    bdmf_index channel_index; /**< Channel index */
    rdpa_if port; /**< Port */
} rdpa_channel_req_key_t;

/** @} end of iptv Doxygen group */

#endif /* _RDPA_IPTV_H_ */

