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

#ifndef _RDPA_UDPSPDTEST_H_
#define _RDPA_UDPSPDTEST_H_

/** \defgroup UDP Speed Test
 * This object is used to manage the UDP Speed Test \n
 * @{
 */

#include "rdpa_types.h"
#include "rdpa_spdtest_common.h"

/** Global configuration for UDP Speed Test mode.\n
 */
typedef struct
{
    rdpa_udpspdtest_proto_t proto; /*< Test protocol, should be configured before opening the first stream */
    uint32_t so_mark; /*< Socket mark per protocol, read-only */
} rdpa_udpspdtest_cfg_t;

/** UDP Speed Test Basic statistics (per-stream).\n
 */
typedef struct {
    bdmf_boolean running;      /**< Running/Stopped */
    uint64_t rx_packets;       /**< Number of packets received by the Analyzer */
    uint64_t rx_bytes;         /**< Number of bytes received by the Analyzer */
    uint32_t rx_time_usec;     /**< Receive Time in microseconds */
    uint64_t tx_packets;       /**< Number of packets transmitted by the Generator */
    uint32_t tx_discards;      /**< Number of packets discarded by the Generator */
    uint32_t tx_time_usec;     /**< Transmit Time in microseconds */
} rdpa_udpspdtest_basic_stat_t;

/** Iperf3 extention Statistics (per-stream).\n
 */
typedef struct
{
    uint64_t out_of_order_pkts; /**< Out-of-order packets */ 
    uint64_t error_cnt_pkts;    /**< Number of errors */
    uint32_t jitter;            /**< Detected Jitter */
    uint32_t tx_time_sec;       /**< TX time: seconds */
    uint32_t tx_time_usec;      /**< TX time: microseconds */
} rdpa_udpspdtest_iperf3_ext_stat_t;

/** UDP Speed Test Statistics.\n
 * */
typedef struct
{
    rdpa_udpspdtest_basic_stat_t basic; /**< Basic statistics, common to all tests */
    rdpa_udpspdtest_iperf3_ext_stat_t iperf3_ext; /**< Iperf3 extention statistics */
} rdpa_udpspdtest_stat_t;

/** @} end of udpspdtest Doxygen group. */

#endif /* _RDPA_UDPSPDTEST_H_ */
