/*
* <:copyright-BRCM:2012-2015:DUAL/GPL:standard
* 
*    Copyright (c) 2012-2015 Broadcom 
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

#ifndef _RDPA_CAPWAP_H_
#define _RDPA_CAPWAP_H_

#include <bdmf_interface.h>
#include "rdd_runner_defs.h"

/** CAPWAP configuration.\n
 */
typedef struct
{
    uint16_t ac_port;                         /**< UDP source port of AC */
    bdmf_ip_t ac_ip;                          /**< IP address of CAPWAP access controller */
    bdmf_ip_t ap_ip;                          /**< IP address of the access point */
    bdmf_mac_t ap_mac_address;                /**< MAC address of the access point */
} rdpa_capwap_cfg_t;

/** CAPWAP reassembly configuration.\n
 */
typedef struct
{
    bdmf_boolean ip_v4_window_check;          /**< 1=verify IPv4 Packet Id of last fragment */
                                              /**< is within a window of first fragment */
    uint16_t receive_frame_buffer_size;       /**< largest reassembled packet size */
} rdpa_capwap_reassembly_cfg_t;

/** CAPWAP reassembly statistics.\n
 */
typedef struct {
    uint32_t invalid_headers;
    uint32_t aborts;
    uint32_t fragments_received;
    uint32_t fragments_evicted;
    uint32_t unfragmented_packets;
    uint32_t middle_fragments;
    uint32_t first_fragments;
    uint32_t last_fragments;
    uint32_t packets_not_in_window;
    uint32_t packets_reassembled;
    uint32_t invalid_fragment;
    uint32_t reassembled_packet_too_big;
} rdpa_capwap_reassembly_stats_t;

/** CAPWAP reassembly active context entries.\n
 */
typedef struct {
    uint16_t entry0;
    uint16_t entry1;
    uint16_t entry2;
    uint16_t entry3;
} rdpa_capwap_reassembly_contexts_t;

/** CAPWAP fragmentation configuration.\n
 */
typedef struct
{
    uint16_t    max_frame_size;               /**< max size that can be sent to egress port */
} rdpa_capwap_fragmentation_cfg_t;

/** CAPWAP fragmentation statistics.\n
 */
typedef struct {
    uint32_t upstream_packets;
    uint32_t invalid_ethtype_or_ip_header;
    uint32_t invalid_protocol;
    uint32_t invalid_capwap_version_type;
    uint32_t congestion;
    uint32_t middle_fragments;
    uint32_t first_fragments;
    uint32_t last_fragments;
} rdpa_capwap_fragmentation_stats_t;

/** @} end of ip_class Doxygen group. */

#endif /* _RDPA_CAPWAP_H_ */

