/*
* <:copyright-BRCM:2014:DUAL/GPL:standard
*
*    Copyright (c) 2014 Broadcom
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

#ifndef _RDPA_UDPSPDTEST_H_
#define _RDPA_UDPSPDTEST_H_

/** \defgroup UDP Speed Test
 * This object is used to manage the UDP Speed Test \n
 * @{
 */

#include "rdpa_types.h"
#include "rdpa_spdtest_common.h"

typedef enum
{
    rdpa_udpspdtest_proto_basic,
    rdpa_udpspdtest_proto_iperf3,
} rdpa_udpspdtest_proto_t;

#define RDPA_SPDTEST_SO_MARK_PREFIX    0xFEFEFC00 /* Include TCP and UDP socket mark */
#define RDPA_TCPSPDTEST_SO_MARK        0xFEFEFD00
#define RDPA_UDPSPDTEST_SO_MARK_PREFIX 0xFEFEFE00
#define RDPA_UDPSPDTEST_SO_MARK_BASIC  0xFEFEFE10
#define RDPA_UDPSPDTEST_SO_MARK_IPERF3 0xFEFEFE20
#define RDPA_UDPSPDTEST_SO_MARK_LAST   0xFEFEFE30

#define RDPA_UDPSPDTEST_DEF_MBS 2000

/** Global configuration for UDP Speed Test mode.\n
 */
typedef struct
{
    rdpa_udpspdtest_proto_t proto; /*< Test protocol, should be configured before opening the first stream */
    uint8_t num_of_streams; /*< Number of total opened streams, read-only */
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

/** UDP Speed Test RX parameters.\n
 */
typedef struct {
    bdmf_ip_t local_ip_addr;   /**< Local IP Address */
    uint16_t local_port_nbr;   /**< Local UDP Port Number */
    bdmf_ip_t remote_ip_addr;  /**< Remote IP Address */
    uint16_t remote_port_nbr;  /**< Remote UDP Port Number */
    bdmf_index us_flow_index;  /**< Analyzer US Unicast Flow Index */
    bdmf_index ds_flow_index;  /**< Analyzer DS Unicast Flow Index */
} rdpa_udpspdtest_rx_params_t;

/** UDP Speed Test TX parameters.\n
 */
typedef struct {
    uint32_t kbps;           /**< Transmit Rate (Kbps) */
    uint32_t mbs;            /**< Maximum Burst Size (bytes) */
    uint64_t total_packets_to_send; /**< Total number of packets to send (optional) */
    bdmf_boolean iperf3_64bit_counters; /**< Iperf3 configuration: use 64-bit counters in UDP test packets */
} rdpa_udpspdtest_tx_params_t;

/** @} end of udpspdtest Doxygen group. */

#endif /* _RDPA_UDPSPDTEST_H_ */
