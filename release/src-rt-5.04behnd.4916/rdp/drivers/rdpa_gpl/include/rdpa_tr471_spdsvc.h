/*
* <:copyright-BRCM:2022:DUAL/GPL:standard
*
*    Copyright (c) 2022 Broadcom
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

typedef void (*tr471_tx_complete_callback_t)(unsigned long param);

#endif /* _RDPA_TR471_SPDSVC_H_ */
