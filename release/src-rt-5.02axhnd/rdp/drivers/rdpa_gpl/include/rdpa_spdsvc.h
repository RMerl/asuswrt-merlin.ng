/*
* <:copyright-BRCM:2014:DUAL/GPL:standard
* 
*    Copyright (c) 2014 Broadcom 
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

#ifndef _RDPA_SPDSVC_H_
#define _RDPA_SPDSVC_H_

/** \defgroup spdsvc Speed Service
 * This object is used to manage the Runner Speed Service Generator \n
 * and Analyzer modules.\n
 * @{
 */

/** Speed Service Generator.\n
 * Contains the configuration parameters \n
 * required by the Runner Speed Service Generator.\n
 */
typedef struct {
    uint32_t kbps;           /**< Transmit Rate (Kbps) */
    uint32_t mbs;            /**< Maximum Burst Size (bytes)*/
    uint32_t copies;         /**< Stream Packet copies */
    uint32_t total_length;   /**< Total Stream Packet length */
    uint32_t test_time_ms;   /**< Total test time (ms) */
} rdpa_spdsvc_generator_t;

/** Speed Service Analyzer.\n
 * Contains the configuration parameters \n
 * required by the Runner Speed Service Analyzer.\n
 */
typedef struct {
    bdmf_ip_t local_ip_addr;   /**< Local IP Address */
    uint16_t local_port_nbr;   /**< Local UDP Port Number */
    bdmf_ip_t remote_ip_addr;  /**< Remote IP Address */
    uint16_t remote_port_nbr;  /**< Remote UDP Port Number */
    bdmf_index us_flow_index;  /**< Analyzer US Unicast Flow Index */
    bdmf_index ds_flow_index;  /**< Analyzer DS Unicast Flow Index */
} rdpa_spdsvc_analyzer_t;

/** Speed Service Result.\n
 * The Speed Service Result contains the results provided by the \n
 * Runner Generator and Analyzer at the end of each test run.\n
 */
typedef struct {
    uint8_t running;           /**< 0: Test done; 1: Test in progress */
    uint32_t rx_packets;       /**< Number of packets received by the Analyzer */
    uint32_t rx_bytes;         /**< Number of bytes received by the Analyzer */
    uint32_t rx_time_usec;     /**< Receive Time in microseconds */
    uint32_t tx_packets;       /**< Number of packets transmitted by the Generator */
    uint32_t tx_discards;      /**< Number of packets discarded by the Generator */
} rdpa_spdsvc_result_t;

/** @} end of spdsvc Doxygen group. */

#endif /* _RDPA_SPDSVC_H_ */
