/*
* <:copyright-BRCM:2022:DUAL/GPL:standard
* 
*    Copyright (c) 2022 Broadcom 
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

#ifndef _RDPA_TR471_SPDSVC_H_
#define _RDPA_TR471_SPDSVC_H_

/** \defgroup spdsvc Speed Service
 * This object is used to manage the Runner TR-471 Speed Service Transmit module \n
 * @{
 */

#include "rdpa_spdtest_common.h"

/** TR-471 Speed Service transmit.\n
 * Contains the configuration parameters \n
 */
typedef struct {
    uint32_t total_burst_size;       /**< number of packets to send */
    uint32_t full_length_burst_size; /**< number of full length packets to send */
    uint32_t payload_length;         /**< UDP payload length w/o header */
    uint32_t addon_length;           /**< UDP payload length of runt packets w/o header */
    uint32_t packet_length;          /**< packet length */
    uint8_t  is_first_burst;         /**< indicates first burst */
    uint8_t  hw_stream_idx;          /**< stream index */
    void *packet_ptr;                /**< pointer to packet data starting at IP header */
} rdpa_tr471_spdsvc_tx_start_t;

/** TR-471 Speed Service packet identification.\n
 */
typedef struct {
    bdmf_ip_t src_ipaddr;            /**< source IP address */
    bdmf_ip_t dst_ipaddr;            /**< destination IP address */
    uint16_t src_port;               /**< source UDP port */
    uint16_t dst_port;               /**< destination UDP port */
} rdpa_tr471_spdsvc_rx_pkt_id_t;

/** @} end of spdsvc Doxygen group. */

#define TR471_MAX_NUM_OF_HW_STREAMS 4

typedef void (*tr471_tx_complete_callback_t)(unsigned long param);

#endif /* _RDPA_TR471_SPDSVC_H_ */
