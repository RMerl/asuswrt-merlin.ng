/*
* <:copyright-BRCM:2012-2015:DUAL/GPL:standard
* 
*    Copyright (c) 2012-2015 Broadcom 
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

